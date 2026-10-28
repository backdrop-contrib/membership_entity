<?php

/**
 * @file
 * Provides entity metadata integration.
 */

/**
 * Provides metadata for memberships.
 */
class MembershipEntityMetadataController extends EntityDefaultMetadataController {

  /**
   * Overrides EntityDefaultMetadataController::entityPropertyInfo().
   */
  public function entityPropertyInfo() {
    $info = parent::entityPropertyInfo();
    $properties = &$info[$this->type]['properties'];

    $properties['mid'] = array(
      'label' => t('Internal id'),
      'description' => t('The unique membership identifier used internally and in URLs.'),
      'required' => TRUE,
    ) + $properties['mid'];

    $properties['member_id'] = array(
      'label' => t('Site id number'),
      'description' => t('The administrative number this site assigns the membership, unique across all membership types. Not the organisation’s own membership number.'),
      'required' => TRUE,
    ) + $properties['member_id'];

    $properties['type'] = array(
      'label' => t('Type'),
      'description' => t('The membership type.'),
      'required' => TRUE,
    ) + $properties['type'];

    unset($properties['uid']);
    $properties['primary_member'] = array(
      'label' => t('Primary member'),
      'type' => 'user',
      'description' => t('The primary member on the membership.'),
      'getter callback' => 'entity_plus_property_getter_method',
      'setter callback' => 'entity_plus_property_setter_method',
      'setter permission' => 'administer memberships',
      'required' => TRUE,
      'schema field' => 'uid',
    );

    // Typed as a membership rather than left as a raw id so that it chains:
    // [membership_entity:pid:member-id] lets the token member id generator
    // derive a sub-member's id from its parent's — 00042-00001 rather than
    // consuming a top level number of its own.
    $properties['pid'] = array(
      'label' => t('Parent membership'),
      'type' => 'membership_entity',
      'description' => t('The household or association membership this one belongs to.'),
      'getter callback' => 'membership_entity_metadata_get_parent',
      'setter callback' => 'entity_plus_property_verbatim_set',
      'setter permission' => 'administer memberships',
      'schema field' => 'pid',
    );

    $properties['status'] = array(
      'label' => t('Membership status'),
      'description' => t('The membership status.'),
      'options list' => 'membership_entity_get_status_list',
    ) + $properties['status'];

    $properties['created'] = array(
      'type' => 'date',
      'label' => t('Date created'),
      'description' => t('The date the membership was created.'),
      'setter callback' => 'entity_plus_property_verbatim_set',
      'setter permission' => 'administer memberships',
    ) + $properties['created'];

    $properties['changed'] = array(
      'type' => 'date',
      'label' => t('Date changed'),
      'description' => t('The date the membership was most recently updated.'),
    ) + $properties['changed'];

    unset($properties['url']);
    return $info;
  }

}
