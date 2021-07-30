<?php
/**
 * @file
 * Path integration.
 */

/**
 * Implements hook_path_info().
 */
function membership_entity_path_info() {

  $info['membership_entity'] = array(
    'entity_type' => 'membership_entity',
    'label' => t('Membership'),
    'pattern description' => t('Default path pattern (applies to all membership types with blank patterns below.)'),
    'pattern default' => 'membership/[membership_entity:mid]',
    'type delete callback' => 'membership_entity_path_type_delete_callback',
    'batch update callback' => 'membership_entity_path_bulk_update_batch_process',
    'batch file' => 'membership_entity.path.inc',
    'batch file path' => backdrop_get_path('module', 'membership_entity'),
    'source prefix' => 'membership/',
  );

  // Get membership types.
  foreach (membership_entity_get_bundles() as $machine_name => $type) {
    $info['membership_entity']['pattern items'][$machine_name] = t('Pattern for all %label paths', array('%label' => $type->label));
  }

  return $info;
}

/**
 * Delete all paths associated with a membership type when that type is deleted.
 *
 * @param array $types
 */
function membership_entity_path_type_delete_callback($types) {
  $query = db_select('membership_entity', 'me');
  $query->addField('me', 'mid');
  $query->condition('me.type', $types, 'IN');
  $mids = $query->execute()->fetchCol();

  $sources = array();
  foreach ($mids as $mid) {
    $sources[] = 'membership/' . $mid;
  }

  $query = db_delete('url_alias');
  $query->condition('source', $sources, 'IN');
  $query->execute();

}

/**
 * Batch processing callback; Generate or update aliases for membership entities..
 */
function membership_entity_path_bulk_update_batch_process(&$context) {

  if (!isset($context['sandbox']['current'])) {
    $context['sandbox']['count'] = 0;
    $context['sandbox']['current'] = 0;
  }

  $query = db_select('membership_entity', 'me');
  $query->addField('me', 'mid');
  $query->condition('me.mid', $context['sandbox']['current'], '>');
  $query->orderBy('mer.mid');
  $query->addTag('path_bulk_update');
  $query->addMetaData('entity', 'membership_entity');
  if (!empty($context['choices'])) {
    $query->condition('me.type', $context['choices'], 'IN');
  }
  $query->leftJoin('url_alias', 'ua', "CONCAT('membership/', me.mid) = ua.source");
  if ($context['op'] == 'update') {
    $query->isNotNull('ua.source');
  }
  if ($context['op'] == 'generate') {
    $query->isNull('ua.source');
  }

  // Get the total amount of items to process.
  if (!isset($context['sandbox']['total'])) {
    $context['sandbox']['total'] = $query->countQuery()->execute()->fetchField();

    // If there are no memberships to update, the stop immediately.
    if (!$context['sandbox']['total']) {
      $context['finished'] = 1;
      return;
    }
  }

  $query->range(0, 25);
  $mids = $query->execute()->fetchCol();

  module_load_include('inc', 'path');
  path_verbose_suspend();

  $memberships = membership_entity_load_multiple($mids);
  foreach ($memberships as $membership) {
    if (path_save_automatic_entity_alias($membership)) {
      $context['results']['total']['membership'] += 1;
    }
  }

  path_verbose_resume();

  $context['sandbox']['count'] += count($mids);
  $context['sandbox']['current'] = max($mids);
  $context['message'] = t('Updated alias for membership @mid.', array('@mid' => end($mids)));

  if ($context['sandbox']['count'] != $context['sandbox']['total']) {
    $context['finished'] = $context['sandbox']['count'] / $context['sandbox']['total'];
  }

}

function membership_entity_path_delete($membership) {

  $query = db_delete('url_alias');
  $query->condition('source', "membership/{$membership->mid}");
  $query->execute();

}
