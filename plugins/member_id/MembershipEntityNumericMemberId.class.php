<?php

/**
 * @file
 * Contains the MembershipEntityNumericMemberId class.
 */

/**
 * MembershipEntityNumericMemberId class.
 */
class MembershipEntityNumericMemberId extends MembershipEntityMemberIdAbstract {

  /**
   * {@inheritdoc}
   */
  public function next(MembershipEntity $membership) {
    $settings = $this->settings;
    $length = !empty($settings['length']) ? $settings['length'] : 5;

    // Reading the counter and writing it back is two steps; without the lock,
    // two people joining at once draw the same number and the second save
    // fails on the unique key.
    while (!lock_acquire('membership_entity_member_id')) {
      lock_wait('membership_entity_member_id');
    }

    // The counter holds the last number issued. A starting number lets a site
    // leave a gap — imported members below it, members who joined here above —
    // and never takes the counter backwards.
    $member_id = (int) state_get('membership_entity_next_member_id', 0);
    if (!empty($settings['start'])) {
      $member_id = max($member_id, (int) $settings['start'] - 1);
    }

    // Members imported with their existing numbers never advanced the counter,
    // so it can sit far below numbers in use. Carry on above the highest, not
    // into the gaps below it: those are numbers lapsed members once held.
    $member_id = max($member_id, $this->highestInUse());

    // Belt and braces for padded and unpadded forms of the same number.
    do {
      $member_id++;
      $candidate = str_pad($member_id, $length, '0', STR_PAD_LEFT);
      $taken = db_query('SELECT 1 FROM {membership_entity} WHERE member_id IN (:ids)', array(
        ':ids' => array($candidate, (string) $member_id),
      ))->fetchField();
    } while ($taken);

    state_set('membership_entity_next_member_id', $member_id);
    lock_release('membership_entity_member_id');

    return $candidate;
  }

  /**
   * {@inheritdoc}
   */
  public function settingsForm(array &$form_state) {
    $settings = $this->settings;
    $form['length'] = array(
      '#type' => 'number',
      '#title' => t('Site id number length'),
      '#description' => t("The minimum number of digits for the member id. Numeric ids with fewer digits will be padded with '0's (eg. 00001)."),
      '#min' => 1,
      '#required' => TRUE,
      '#default_value' => !empty($settings['length']) ? $settings['length'] : 5,
    );

    $highest = $this->highestInUse();
    $last = state_get('membership_entity_next_member_id', 0);
    $form['start'] = array(
      '#type' => 'number',
      '#title' => t('Number new memberships from'),
      '#description' => t('Optional. New memberships are numbered from here. Without it they carry on above the highest number in use, never into gaps below it. Useful after importing members, so that everyone who joined on this site is recognisable by number. The highest number in use is %highest; the last number issued here was %last. Shared by every membership type that uses site id numbers.', array(
        '%highest' => $highest ? $highest : t('none'),
        '%last' => $last ? $last : t('none'),
      )),
      '#min' => 1,
      '#default_value' => !empty($settings['start']) ? $settings['start'] : '',
    );

    return $form;
  }

  /**
   * The highest all-digit member id held by any membership.
   *
   * @return int
   *   The number, or 0 where there is none.
   */
  protected function highestInUse() {
    return (int) db_query("SELECT MAX(CAST(member_id AS UNSIGNED)) FROM {membership_entity} WHERE member_id REGEXP '^[0-9]+$'")->fetchField();
  }

  /**
   * {@inheritdoc}
   */
  public function validateSettings(array &$element, array &$form_state) {
    $schema = backdrop_get_schema('membership_entity');
    if ($element['length']['#value'] > $schema['fields']['member_id']['length']) {
      form_error($element['length'], t('Site id number length cannot exceed %max.', array(
        '%max' => $schema['fields']['member_id']['length'],
      )));
    }

    // A start at or below numbers already issued would not be where new
    // members begin, which defeats the point of choosing one.
    $start = trim($element['start']['#value']);
    $highest = $this->highestInUse();
    if ($start !== '' && (int) $start <= $highest) {
      form_error($element['start'], t('Numbers up to %highest are already in use. Choose a higher starting number, or leave it empty.', array(
        '%highest' => $highest,
      )));
    }
  }

}
