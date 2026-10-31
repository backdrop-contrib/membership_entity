<?php

/**
 * @file
 * Page callbacks for membership_entity_hierarchy.
 */

/**
 * Page callback: add a sub-member to a membership.
 *
 * Opens the ordinary membership add form with "Belongs to" already set to the
 * membership the action was followed from, so that adding a member to a
 * household or association starts from that household or association rather
 * than from a list of every membership on the site. Where more than one
 * membership type exists the type is chosen first, exactly as the generic add
 * page does.
 *
 * @param MembershipEntity $parent
 *   The membership the new sub-member will belong to.
 * @param string $type_url_str
 *   The membership type to create, with underscores replaced by dashes.
 *   Omitted on the first step when there is more than one type.
 *
 * @return array|string
 *   A render array, or the built add form.
 */
function membership_entity_hierarchy_add_sub_member_page(MembershipEntity $parent, $type_url_str = NULL) {
  // The add form and its handlers live with membership_entity.
  module_load_include('inc', 'membership_entity', 'membership_entity.pages');

  $bundles = membership_entity_get_bundles();
  if (empty($bundles)) {
    return array(
      '#markup' => t('There are no membership types, so a sub-member cannot be created.'),
    );
  }

  $type = NULL;
  if (isset($type_url_str)) {
    $candidate = str_replace('-', '_', $type_url_str);
    if (!isset($bundles[$candidate])) {
      return MENU_NOT_FOUND;
    }
    $type = $candidate;
  }
  elseif (count($bundles) == 1) {
    // Nothing to choose, so do not ask.
    $type = key($bundles);
  }

  if (isset($type)) {
    // t() has already sanitized the label into the string, so the title must
    // not be run through check_plain() a second time.
    backdrop_set_title(t('Add sub-member to @parent', array(
      '@parent' => membership_entity_parent_label($parent->mid),
    )), PASS_THROUGH);

    $membership = membership_entity_create(array(
      'type' => $type,
      'pid' => $parent->mid,
    ));

    return backdrop_get_form('membership_entity_edit_form', $membership, $type);
  }

  $items = array();
  foreach ($bundles as $bundle => $bundle_type) {
    $items[] = l($bundle_type->label, 'membership/' . $parent->mid . '/add-sub-member/' . str_replace('_', '-', $bundle));
  }

  return array(
    'help' => array(
      '#prefix' => '<p>',
      '#suffix' => '</p>',
      '#markup' => t('Which type of membership should the new sub-member of @parent hold?', array(
        '@parent' => membership_entity_parent_label($parent->mid),
      )),
    ),
    'types' => array(
      '#theme' => 'item_list',
      '#items' => $items,
    ),
  );
}
