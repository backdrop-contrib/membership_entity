<?php

/**
 * @file
 * Contains the broken member_id plugin class.
 */

/**
 * A broken implementation of MembershipEntityMemberId.
 */
class MembershipEntityMemberIdBroken extends MembershipEntityMemberIdAbstract {

  /**
   * {@inheritdoc}
   */
  public function next(MembershipEntity $membership) {}

  /**
   * {@inheritdoc}
   */
  public function settingsForm(array &$form_state) {
    $form['member_id_broken'] = array(
      '#markup' => t('The selected member_id handler is broken.'),
    );
    return $form;
  }

}
