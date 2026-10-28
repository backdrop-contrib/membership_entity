<?php

/**
 * @file
 * Membership Entity API Documentation.
 *
 * This file contains no working PHP code; it exists to provide additional
 * documentation for doxygen as well as to document hooks in the standard
 * Backdrop manner.
 */

/**
 * Control access to memberships.
 *
 * @param string $op
 *   The membership operation.
 * @param optional|MembershipEntity $membership
 *   The membership object. NULL if $op is 'join'.
 * @param object $account
 *   The loaded user account object. Defaults to logged in user.
 *
 * @return bool
 *   TRUE if $account as permission for $op.
 */
function hook_membership_entity_access($op, $membership = NULL, $account = NULL) {
  $user = isset($account) ? $account : $GLOBALS['user'];

  // Join page access.
  if ($op == 'join') {
    $type = str_replace('-', '_', $membership);
    // Check if a membership already exists for this user.
    $exists = (bool) db_query('SELECT mid FROM {membership_entity} WHERE uid = :uid AND uid <> 0 AND type = :type', array(':uid' => $user->uid, ':type' => $type))->rowCount();
    if (!$exists && user_access("$type join", $user)) {
      return TRUE;
    }
    return FALSE;
  }

  if (isset($membership) && $type_name = $membership->type) {
    if (user_access("$type_name $op any membership", $user)) {
      return TRUE;
    }

    if (user_access("$type_name $op own membership", $user) && $membership->uid == $user->uid) {
      return TRUE;
    }

    // Secondary member access.
    $results = db_query('SELECT mid FROM {membership_entity_secondary_member} WHERE uid = :uid', array(':uid' => $user->uid))->fetchAllAssoc('mid');
    if (!empty($results) && in_array($membership->mid, array_keys($results))) {
      // Can secondary members edit the membership?
      $bundle_settings = membership_entity_get_bundle_settings($type_name);
      if ($op == 'edit' && $bundle_settings['all_edit'] && user_access("$type edit own membership", $user)) {
        return TRUE;
      }

      // Can secondary members view the membership?
      if ($op == 'view' && user_access("$type view own membership", $user)) {
        return TRUE;
      }
    }
  }
}

/**
 * Define membership operations.
 *
 * @return array
 *   An array of operations with a label and and callback.
 */
function hook_membership_entity_operations() {
  return array(
    'delete' => array(
      'label' => t('Delete selected memberships'),
      'callback' => NULL,
    ),
  );
}

/**
 * Respond to a membership being detached from its parent.
 *
 * Fired when a membership that belonged to a household or association has its
 * parent cleared. The membership is on its own from this point: nothing gates
 * its access any longer, so anything the parent was providing on its behalf
 * has to be reconsidered here.
 *
 * membership_entity_term implements this to give the membership a term of its
 * own, copied from the parent's current term, where the parent was paying for
 * it. Without that the membership would have no term at all and would never
 * expire.
 *
 * @param MembershipEntity $membership
 *   The membership that has just been detached, already saved.
 * @param int $old_pid
 *   The membership id of the parent it was detached from.
 */
function hook_membership_entity_uncoupled($membership, $old_pid) {
  watchdog('example', 'Membership @id left membership @parent.', array(
    '@id' => $membership->member_id,
    '@parent' => $old_pid,
  ));
}

/**
 * Alter the status that governs a membership's access.
 *
 * The effective status is the membership's own status, unless it belongs to a
 * parent membership that is not itself active — in which case the parent's
 * status is returned instead, so that revoking an organisation's membership
 * revokes everyone beneath it.
 *
 * @param int $status
 *   The resolved status, one of the MEMBERSHIP_ENTITY_* constants.
 * @param MembershipEntity $membership
 *   The membership being resolved.
 */
function hook_membership_entity_effective_status_alter(&$status, $membership) {
  // Let honorary members keep access whatever their association is doing.
  if (!empty($membership->field_honorary[LANGUAGE_NONE][0]['value'])) {
    $status = MEMBERSHIP_ENTITY_ACTIVE;
  }
}
