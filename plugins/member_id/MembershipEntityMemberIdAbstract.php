<?php

/**
 * @file
 * Contains the abstract member_id plugin class.
 */

/**
 * An abstract implementation of MembershipEntityMemberId.
 */
abstract class MembershipEntityMemberIdAbstract implements MembershipEntityMemberIdInterface {
  /**
   * The settings options.
   *
   * @var array
   */
  protected $settings;

  /**
   * Default constructor.
   *
   * @param array $settings
   *   An array of default plugin settings.
   */
  public function __construct(array $settings) {
    $this->settings = $settings;
  }

  /**
   * {@inheritdoc}
   */
  public function next(MembershipEntity $membership) {}

  /**
   * {@inheritdoc}
   */
  public function settingsForm(array &$form_state) {
    return array();
  }

  /**
   * {@inheritdoc}
   */
  public function validateSettings(array &$element, array &$form_state) {}

}
