<?php

/**
 * @file
 * Provides entity metadata integration.
 */

/**
 * Provides metadata for membership terms.
 */
class MembershipEntityTermMetadataController extends EntityDefaultMetadataController {

  /**
   * {@inheritdoc}
   */
  public function entityPropertyInfo() {
    $info = parent::entityPropertyInfo();
    $properties = &$info[$this->type]['properties'];

    // Remove unnecessary fields.
    unset($properties['url']);
    unset($properties['membership_entity_type']);

    unset($properties['mid']);
    $properties['membership'] = array(
      'label' => t('Membership'),
      'type' => 'membership_entity',
      'description' => t('The membership for this term.'),
      'getter callback' => 'entity_property_getter_method',
      'setter callback' => 'entity_property_setter_method',
      'setter permission' => 'administer memberships',
      'required' => TRUE,
      'schema field' => 'mid',
    );

    $properties['term'] = array(
      'label' => t('Term length'),
      'description' => t('The membership term length.'),
    ) + $properties['term'];

    $properties['status'] = array(
      'label' => t('Term Status'),
      'description' => t('The membership term status.'),
      'options list' => 'membership_entity_get_status_list',
      'setter callback' => 'entity_property_verbatim_set',
    ) + $properties['status'];

    $properties['start'] = array(
      'type' => 'date',
      'label' => t('Start date'),
      'description' => t('The date the membership term begins.'),
      'required' => TRUE,
      'schema field' => 'start',
    );

    $properties['end'] = array(
      'type' => 'date',
      'label' => t('End date'),
      'description' => t('The instant the membership term stops, which is also the instant a renewal term begins — a year from 1 April 2026 ends on 1 April 2027, 12:00am. It is a boundary, not the last day of cover: the member is covered for the whole of 31 March and no part of 1 April. Label it "Renewal date" where that reads better, or use the term date field\'s "last moment of cover" option to show 31 March instead.'),
      'required' => TRUE,
      'schema field' => 'end',
    );

    $properties['current'] = array(
      'type' => 'boolean',
      'label' => t('Current'),
      'description' => t('A boolean indicating whether this is the current term.'),
      'getter callback' => 'membership_entity_term_get_properties',
      'computed' => TRUE,
      'entity views field' => TRUE,
    );

    $properties['created'] = array(
      'type' => 'date',
      'label' => t('Date created'),
      'description' => t('The date the membership term was created.'),
      'setter callback' => 'entity_property_verbatim_set',
      'setter permission' => 'administer memberships',
    ) + $properties['created'];

    $properties['changed'] = array(
      'type' => 'date',
      'label' => t('Date changed'),
      'description' => t('The date the membership term was most recently updated.'),
    ) + $properties['changed'];

    return $info;
  }

}
