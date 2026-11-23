<?php

/**
 * @file
 * Views integration for membership_entity_hierarchy.
 */

/**
 * Implements hook_views_data_alter().
 */
function membership_entity_hierarchy_views_data_alter(&$data) {
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

  // One search box over member id and name. Core's combine filter fails on
  // mixed collations; see views_handler_filter_membership_search.
  $data['views']['membership_search'] = array(
    'title' => t('Search fields (membership)'),
    'help' => t('Matches the text in any of the chosen fields, field by field. Use instead of "Combine fields filter" where the fields come from different tables.'),
    'filter' => array(
      'handler' => 'views_handler_filter_membership_search',
    ),
  );
}
