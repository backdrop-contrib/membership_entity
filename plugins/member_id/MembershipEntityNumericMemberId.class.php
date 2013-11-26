<?php

/**
 * @file
 * Contains the MembershipEntityNumericMemberId class.
 */

class MembershipEntityNumericMemberId extends MembershipEntityMemberIdAbstract {
  /**
   * Returns the next available member id.
   */
  public function next() {
    $settings = $this->settings;
    $length = $this->settings['length'];
    $member_id = variable_get('membership_entity_next_member_id', 0);
    variable_set('membership_entity_next_member_id', ++$member_id);
    return str_pad($member_id, $length, '0', STR_PAD_LEFT);
  }

  /**
   * Builds the settings form for the member id.
   */
  public function settingsForm(&$form_state) {
    $settings = $this->settings;
    $form['length'] = array(
      '#type' => 'textfield',
      '#title' => t('Member ID Length'),
      '#description' => t("The minimum number of digits for the member id. Numeric ids with fewer digits will be padded with '0's (eg. 00001)."),
      '#size' => 5,
      '#default_value' => !empty($settings['length']) ? $settings['length'] : 5,
      '#element_validate' => array('element_validate_integer_positive'),
    );
    return $form;
  }
}