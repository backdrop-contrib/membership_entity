<?php

/**
 * Implements hook_views_data_alter().
 */
function membership_entity_term_views_data_alter(&$data) {
  $data['views_entity_membership_entity_term']['current']['field']['output formats'] = array(
    'current' => array(t('Current'), ''),
  );

  // Add membership fields.
  $data['membership_entity']['table']['join'] = array(
    'membership_entity_term' => array(
      'left_field' => 'mid',
      'field' => 'mid',
    ),
  );

  $data['membership_entity']['membership_entity_term_representative'] = array(
    'relationship' => array(
      'title' => t('Representative membership term'),
      'label' => t('Representative term'),
      'help' => t('Obtains a single representative term for each membership, according to a chosen sort criterion.'),
      'handler' => 'views_handler_relationship_term_groupwise_max',
      'relationship field' => 'mid',
      'outer field' => 'membership_entity.mid',
      'argument table' => 'membership_entity',
      'argument field' => 'mid',
      'base' => 'membership_entity_term',
      'field' => 'id',
    ),
  );

  $data['membership_entity']['renew'] = array(
    'title' => t('Renew'),
    'help' => t('A link to renew the membership.'),
    'field' => array(
      'op' => 'renew',
      'handler' => 'views_handler_field_membership_link',
    ),
  );
}
