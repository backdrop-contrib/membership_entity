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

  // Whether a sub-member has been brought onto the term the membership above
  // them is paying for. Lives on the membership table rather than the term
  // table because it is a fact about the member, answered from their terms.
  $data['membership_entity']['renewal_state'] = array(
    'title' => t('Renewal state'),
    'help' => t('Whether the membership is covered as far forward as the membership it belongs to. Intended for a roster of sub-members being marked off at renewal.'),
    'field' => array(
      'handler' => 'views_handler_field_membership_renewal_state',
    ),
  );

  // When the membership's cover runs out, or a marker for "due for renewal",
  // for mailing lists. One value per membership however many terms it holds.
  $data['membership_entity']['expiry'] = array(
    'title' => t('Expiry'),
    'help' => t('The furthest end date of the membership\'s terms, or a marker when it falls in the next few months.'),
    'field' => array(
      'handler' => 'views_handler_field_membership_expiry',
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
