<?php

/**
 * @file
 * Containse the MembershipEntityController class.
 */

/**
 * Entity controller class for membership entities.
 */
class MembershipEntityController extends EntityPlusController {

  /**
   * Overrides EntityPlusController::create().
   */
  public function create(array $values = array()) {
    // Add some defaults.
    $values += array(
      'mid' => 0,
      'uid' => 0,
      'pid' => 0,
      'member_id' => '',
      'type' => 'membership',
      'created' => REQUEST_TIME,
      'status' => MEMBERSHIP_ENTITY_ACTIVE,
      'secondary_members' => array(),
    );
    $values = parent::create($values);

    // Allow modules to add stuff to the default membership.
    $this->invoke('create', $values);
    return $values;
  }

  /**
   * Overrides EntityPlusController::load().
   */
  public function load($mids = array(), $conditions = array()) {
    $memberships = parent::load($mids, $conditions);

    if (empty($memberships)) {
      // Early exit if membership loading failed.
      return array();
    }

    // Load secondary members.
    foreach ($memberships as $membership) {
      $membership->secondary_members = array();
      $secondary_members = db_query('SELECT uid, weight FROM {membership_entity_secondary_member} WHERE mid = :mid ORDER BY weight', array(
        ':mid' => $membership->mid,
      ));
      foreach ($secondary_members as $member) {
        $membership->secondary_members[$member->weight] = $member->uid;
      }
    }
    return $memberships;
  }

  /**
   * Load a single membership by member id.
   *
   * @param string $member_id
   *   The unique membership member id.
   *
   * @return MembershipEntity
   *   The membership object identified by member id.
   */
  public function loadByMemberID($member_id) { // @codingStandardsIgnoreLine
    $result = $this->load(FALSE, array('member_id' => $member_id));
    return reset($result);
  }

  /**
   * Load all memberships for a given user.
   *
   * @param object|int $account
   *   A user account object or uid.
   * @param string $bundle
   *   The membership bundle to load.
   *
   * @return MembershipEntity[] | FALSE
   *   An array of all memberships for the given user account, keyed by
   *   bundle. If $bundle is provided, a single membership object if it exists,
   *   or FLASE otherwise.
   */
  public function loadByUser($account, $bundle = NULL) {
    // Support uid or entire user object.
    $uid = is_object($account) ? $account->uid : $account;
    $memberships = $this->load(FALSE, array('uid' => $uid));

    if (empty($memberships)) {
      // Early exist if no memberships found.
      return array();
    }

    // Keyed by membership id, not by bundle. Keying by bundle silently discarded
    // every membership but the last whenever a user held more than one of the
    // same type — which is routine wherever a lapsed member rejoins and is
    // issued a second membership rather than having the first reinstated. The
    // losses were not cosmetic: hook_user_delete() deletes only what this
    // returns, so cancelling such a user left an orphaned membership row
    // pointing at a uid that no longer existed.
    $return = array();
    foreach ($memberships as $membership) {
      $return[$membership->mid] = $membership;
    }

    // Filtering by bundle still yields a single membership, as callers expect.
    // Where a user holds several of one type the most recent is the live one.
    if (!empty($bundle)) {
      $match = FALSE;
      foreach ($return as $membership) {
        if ($membership->type == $bundle) {
          $match = $membership;
        }
      }
      return $match;
    }

    return $return;
  }

  /**
   * Overrides EntityPlusController::save().
   */
  public function save($membership, DatabaseTransaction $transaction = NULL) {
    $transaction = isset($transaction) ? $transaction : db_transaction();
    $data = membership_entity_get_bundle_settings($membership->type);
    $original_status = NULL;
    $original_pid = 0;

    try {
      $roles = array();

      // Roles follow the effective status, not the stored one, so that a child
      // membership under a lapsed parent grants nothing even while its own
      // status remains active.
      $effective = membership_entity_effective_status($membership);
      $op = ($effective == MEMBERSHIP_ENTITY_ACTIVE) ? 'add' : 'remove';

      // Add or remove old roles depending on membership status.
      $roles[$membership->uid] = array(
        'rid' => $data['primary_role'],
        'op' => $op,
      );
      foreach ($membership->secondary_members as $uid) {
        $roles[$uid] = array(
          'rid' => $data['secondary_role'],
          'op' => $op,
        );
      }

      // Remove old roles.
      if (!empty($membership->mid)) {
        $params = array(':mid' => $membership->mid);
        $original = db_query('SELECT uid, status, pid FROM {membership_entity} WHERE mid = :mid', $params)->fetchObject();
        $original_uid = $original ? $original->uid : NULL;
        $original_status = $original ? (int) $original->status : NULL;
        $original_pid = $original ? (int) $original->pid : 0;
        if ($membership->uid != $original_uid) {
          $roles[$original_uid] = array(
            'rid' => $data['primary_role'],
            'op' => 'remove',
          );
        }

        $original_secondary = array_keys(db_query('SELECT uid FROM {membership_entity_secondary_member} WHERE mid = :mid', $params)->fetchAllAssoc('uid'));
        foreach (array_diff($original_secondary, $membership->secondary_members) as $uid) {
          $roles[$uid] = array(
            'rid' => $data['secondary_role'],
            'op' => 'remove',
          );
        }
      }
      _membership_entity_update_roles($roles);
    }
    catch (Exception $e) {
      $transaction->rollback();
      watchdog_exception('membership_entity', $e);
      throw $e;
    }

    // Save the membership.
    $status = parent::save($membership, $transaction);

    try {
      // Update secondary members.
      db_delete('membership_entity_secondary_member')->condition('mid', $membership->mid)->execute();
      if (!empty($membership->secondary_members)) {
        $query = db_insert('membership_entity_secondary_member')
          ->fields(array('mid', 'uid', 'weight'));
        foreach ($membership->secondary_members as $weight => $uid) {
          $query->values(array(
            'mid' => $membership->mid,
            'uid' => $uid,
            'weight' => $weight,
          ));
        }
        $query->execute();
      }

      // A change of status here changes what every child membership grants, so
      // reapply their roles. Their own stored statuses are left alone.
      if (isset($original_status) && $original_status != (int) $membership->status) {
        _membership_entity_propagate_to_children($membership);
      }

      // A membership that has just been detached from its parent is on its own
      // from here. Where the parent was paying for it, it has no term of its
      // own and would otherwise never expire, so modules are given the chance
      // to put that right — see membership_entity_term_membership_entity_uncoupled().
      if (!empty($original_pid) && empty($membership->pid)) {
        module_invoke_all('membership_entity_uncoupled', $membership, $original_pid);
      }
    }
    catch (Exception $e) {
      $transaction->rollback();
      watchdog_exception('membership_entity', $e);
      throw $e;
    }

    return $status;
  }

  /**
   * Overrides EntityPlusController::delete().
   */
  public function delete($mids, DatabaseTransaction $transaction = NULL) {
    $memberships = $mids ? $this->load($mids) : FALSE;
    if (!$memberships) {
      // Early exit if no memberships were loaded.
      return;
    }

    try {
      // Remove roles.
      $roles = array();
      foreach ($memberships as $membership) {
        $bundle_settings = membership_entity_get_bundle_settings($membership->type);
        $roles[$membership->uid] = array(
          'rid' => $bundle_settings['primary_role'],
          'op' => 'remove',
        );
        foreach ($membership->secondary_members as $uid) {
          $roles[$uid] = array(
            'rid' => $bundle_settings['secondary_role'],
            'op' => 'remove',
          );
        }
      }
      _membership_entity_update_roles($roles);

      // Delete secondary members.
      db_delete('membership_entity_secondary_member')
        ->condition('mid', $mids, 'IN')
        ->execute();
    }
    catch (Exception $e) {
      if (isset($transaction)) {
        $transaction->rollback();
      }
      watchdog_exception($this->entityType, $e);
      throw $e;
    }

    // Finish deleting the memberships.
    parent::delete($mids, $transaction);
  }

}
