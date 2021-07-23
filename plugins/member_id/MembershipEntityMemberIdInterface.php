<?php

/**
 * @file
 * Contains the member_id plugin interface.
 */

/**
 * Additional member_id formats for a membership type.
 */
interface MembershipEntityMemberIdInterface {

  /**
   * Determines the next available member id.
   *
   * @param MembershipEntity $membership
   *   The membership object used for tokens.
   *
   * @return string
   *   The next available member id.
   */
  public function next(MembershipEntity $membership);

  /**
   * Builds the settings form this handler.
   *
   * @param array $form_state
   *   The Form API form_state array.
   *
   * @return array
   *   The Form API form array.
   *
   * @todo determine if form_state parameter is actually needed, since it
   * is not used within the method.
   */
  public function settingsForm(array &$form_state);

  /**
   * Validates the plugin settings form.
   *
   * @param array $element
   *   Form API element array.
   * @param array $form_state
   *   Form API form_state array.
   */
  public function validateSettings(array &$element, array &$form_state);

}
