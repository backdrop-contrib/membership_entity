<?php

/**
 * @file
 * Provides entity views integration.
 */

/**
 * Provides views integration for memberships.
 */
class MembershipEntityViewsController extends EntityDefaultViewsController {

  /**
   * Overrides EntityDefaultViewsController::views_data().
   */
  public function views_data() { // @codingStandardsIgnoreLine, EntityDefaultViewsController override compliant
    $info = parent::views_data();
    $data = &$info['membership_entity'];

    $data['primary_member']['title'] = t('Primary member');
    $data['primary_member']['relationship']['label'] = t('Primary member');
    $data['primary_member']['field'] = [
      'type' => 'user',
      'handler' => 'views_handler_field_membership_primary_member',
    ];

    $data['secondary_members'] = [
      'title' => t('Secondary members'),
      'help' => t('Secondary members that belong to the membership.'),
      'field' => [
        'handler' => 'views_handler_field_membership_secondary_member',
      ],
    ];

    $data['url'] = [
      'title' => t('Link'),
      'help' => t('A link to view the membership.'),
      'field' => [
        'op' => 'view',
        'handler' => 'views_handler_field_membership_link',
      ],
    ];

    $data['edit'] = [
      'title' => t('Edit'),
      'help' => t('A link to edit the membership.'),
      'field' => [
        'op' => 'edit',
        'handler' => 'views_handler_field_membership_link',
      ],
    ];

    $data['delete'] = [
      'title' => t('Delete'),
      'help' => t('A link to delete the membership.'),
      'field' => [
        'op' => 'delete',
        'handler' => 'views_handler_field_membership_link',
      ],
    ];

    $info['membership_entity_secondary_member'] = [
      'table' => [
        'group' => t('Secondary members'),
        'join' => [
          'membership_entity' => [
            'left_field' => 'mid',
            'field' => 'mid',
          ],
        ],
      ],
    ];

    $info['membership_entity_secondary_member']['uid'] = [
      'title' => t('User'),
      'help' => t('Secondary members that belong to the membership.'),
      'relationship' => [
        'label' => t('Secondary member'),
        'handler' => 'views_handler_relationship',
        'base' => 'users',
        'base field' => 'uid',
        'relationship field' => 'uid',
      ],
    ];

    $info['membership_entity_secondary_member']['weight'] = [
      'title' => t('Weight'),
      'help' => t('The secondary member weight field.'),
      'field' => [
        'handler' => 'views_handler_field_numeric',
      ],
      'filter' => [
        'handler' => 'views_handler_filter_numeric',
      ],
      'sort' => [
        'handler' => 'views_handler_sort',
      ],
    ];

    $user = &$info['users'];
    $user['membership_entity']['title'] = t('Primary membership');
    $user['membership_entity']['relationship']['label'] = t('Primary membership');

    return $info;
  }

}

class EntityDefaultViewsController {

  protected $type, $info, $relationships;

  public function __construct($type) {
    $this->type = $type;
    $this->info = entity_get_info($type);
  }

  /**
 * Defines the result for hook_views_data().
 */
public function views_data() {
    $data = array();
    $this->relationships = array();

    if (!empty($this->info['base table'])) {
      $table = $this->info['base table'];
      // Define the base group of this table. Fields that don't
      // have a group defined will go into this field by default.
      $data[$table]['table']['group']  = backdrop_ucfirst($this->info['label']);
      $data[$table]['table']['entity type'] = $this->type;

      // If the plural label isn't available, use the regular label.
      $label = isset($this->info['plural label']) ? $this->info['plural label'] : $this->info['label'];
      $data[$table]['table']['base'] = array(
        'field' => $this->info['entity keys']['id'],
        'access query tag' => $this->type . '_access',
        'title' => backdrop_ucfirst($label),
        'help' => isset($this->info['description']) ? $this->info['description'] : '',
      );
      $data[$table]['table']['entity type'] = $this->type;
      $data[$table] += $this->schema_fields();

      // Add in any reverse-relationships which have been determined.
      $data += $this->relationships;
    }
    if (!empty($this->info['revision table']) && !empty($this->info['entity keys']['revision'])) {
      $revision_table = $this->info['revision table'];

      $data[$table]['table']['default_relationship'] = array(
        $revision_table => array(
          'table' => $revision_table,
          'field' => $this->info['entity keys']['revision'],
        ),
      );

      // Define the base group of this table. Fields that don't
      // have a group defined will go into this field by default.
      $data[$revision_table]['table']['group']  = backdrop_ucfirst($this->info['label']) . ' ' . t('Revisions');
      $data[$revision_table]['table']['entity type'] = $this->type;

      // If the plural label isn't available, use the regular label.
      $label = isset($this->info['plural label']) ? $this->info['plural label'] : $this->info['label'];
      $data[$revision_table]['table']['base'] = array(
        'field' => $this->info['entity keys']['revision'],
        'access query tag' => $this->type . '_access',
        'title' => backdrop_ucfirst($label) . ' ' . t('Revisions'),
        'help' => (isset($this->info['description']) ? $this->info['description'] . ' ' : '') . t('Revisions'),
      );
      $data[$revision_table]['table']['entity type'] = $this->type;
      $data[$revision_table] += $this->schema_revision_fields();

      // Add in any reverse-relationships which have been determined.
      $data += $this->relationships;

      // For other base tables, explain how we join.
      $data[$revision_table]['table']['join'] = array(
        // Directly links to base table.
        $table => array(
          'left_field' => $this->info['entity keys']['revision'],
          'field' => $this->info['entity keys']['revision'],
        ),
      );
      $data[$revision_table]['table']['default_relationship'] = array(
        $table => array(
          'table' => $table,
          'field' => $this->info['entity keys']['id'],
        ),
      );
    }
    return $data;
  }

  /**
   * Try to come up with some views fields with the help of the schema and
   * the entity property information.
   */
  protected function schema_fields() {
    $schema = backdrop_get_schema($this->info['base table']);
    $properties = $this->entity_get_property_info($this->type) + array('properties' => array());
    $data = array();

    foreach ($properties['properties'] as $name => $property_info) {
      if (isset($property_info['schema field']) && isset($schema['fields'][$property_info['schema field']])) {
        if ($views_info = $this->map_from_schema_info($name, $schema['fields'][$property_info['schema field']], $property_info)) {
          $data[$name] = $views_info;
        }
      }
    }
    return $data;
  }

  /**
   * Try to come up with some views fields with the help of the revision schema
   * and the entity property information.
   */
  protected function schema_revision_fields() {
    $data = array();
    if (!empty($this->info['revision table'])) {
      $schema = backdrop_get_schema($this->info['revision table']);
      $properties = $this->entity_get_property_info($this->type) + array('properties' => array());

      foreach ($properties['properties'] as $name => $property_info) {
        if (isset($property_info['schema field']) && isset($schema['fields'][$property_info['schema field']])) {
          if ($views_info = $this->map_from_schema_info($name, $schema['fields'][$property_info['schema field']], $property_info)) {
            $data[$name] = $views_info;
          }
        }
      }
    }
    return $data;
  }

  /**
   * Comes up with views information based on the given schema and property
   * info.
   */
  protected function map_from_schema_info($property_name, $schema_field_info, $property_info) {
    $type = isset($property_info['type']) ? $property_info['type'] : 'text';
    $views_field_name = $property_info['schema field'];

    $return = array();

    if (!empty($schema_field_info['serialize'])) {
      return FALSE;
    }

    $description = array(
      'title' => $property_info['label'],
      'help' => isset($property_info['description']) ? $property_info['description'] : NULL,
    );

    // Add in relationships to related entities.
    if (($info = entity_get_info($type)) && !empty($info['base table'])) {

      // Prepare reversed relationship data.
      $label_lowercase = backdrop_strtolower($this->info['label'][0]) . backdrop_substr($this->info['label'], 1);
      $property_label_lowercase = backdrop_strtolower($property_info['label'][0]) . backdrop_substr($property_info['label'], 1);

      // We name the field of the first reverse-relationship just with the
      // base table to be backward compatible, for subsequents relationships we
      // append the views field name in order to get a unique name.
      $name = !isset($this->relationships[$info['base table']][$this->info['base table']]) ? $this->info['base table'] : $this->info['base table'] . '_' . $views_field_name;
      $this->relationships[$info['base table']][$name] = array(
        'title' => $this->info['label'],
        'help' => t("Associated @label via the @label's @property.", array('@label' => $label_lowercase, '@property' => $property_label_lowercase)),
        'relationship' => array(
          'label' => $this->info['label'],
          'handler' => $this->getRelationshipHandlerClass($this->type, $type),
          'base' => $this->info['base table'],
          'base field' => $views_field_name,
          'relationship field' => isset($info['entity keys']['name']) ? $info['entity keys']['name'] : $info['entity keys']['id'],
        ),
      );

      $return['relationship'] = array(
        'label' => backdrop_ucfirst($info['label']),
        'handler' => $this->getRelationshipHandlerClass($type, $this->type),
        'base' => $info['base table'],
        'base field' => isset($info['entity keys']['name']) ? $info['entity keys']['name'] : $info['entity keys']['id'],
        'relationship field' => $views_field_name,
      );

      // Add in direct field/filters/sorts for the id itself too.
      $type = isset($info['entity keys']['name']) ? 'token' : 'integer';
      // Append the views-field-name to the title if it is different to the
      // property name.
      if ($property_name != $views_field_name) {
        $description['title'] .= ' ' . $views_field_name;
      }
    }

    switch ($type) {
      case 'token':
      case 'text':
        $return += $description + array(
            'field' => array(
              'real field' => $views_field_name,
              'handler' => 'views_handler_field',
              'click sortable' => TRUE,
            ),
            'sort' => array(
              'real field' => $views_field_name,
              'handler' => 'views_handler_sort',
            ),
            'filter' => array(
              'real field' => $views_field_name,
              'handler' => 'views_handler_filter_string',
            ),
            'argument' => array(
              'real field' => $views_field_name,
              'handler' => 'views_handler_argument_string',
            ),
          );
        break;

      case 'decimal':
      case 'integer':
        $return += $description + array(
            'field' => array(
              'real field' => $views_field_name,
              'handler' => 'views_handler_field_numeric',
              'click sortable' => TRUE,
              'float' => ($type == 'decimal'),
            ),
            'sort' => array(
              'real field' => $views_field_name,
              'handler' => 'views_handler_sort',
            ),
            'filter' => array(
              'real field' => $views_field_name,
              'handler' => 'views_handler_filter_numeric',
            ),
            'argument' => array(
              'real field' => $views_field_name,
              'handler' => 'views_handler_argument_numeric',
            ),
          );
        break;

      case 'date':
        $return += $description + array(
            'field' => array(
              'real field' => $views_field_name,
              'handler' => 'views_handler_field_date',
              'click sortable' => TRUE,
            ),
            'sort' => array(
              'real field' => $views_field_name,
              'handler' => 'views_handler_sort_date',
            ),
            'filter' => array(
              'real field' => $views_field_name,
              'handler' => 'views_handler_filter_date',
            ),
            'argument' => array(
              'real field' => $views_field_name,
              'handler' => 'views_handler_argument_date',
            ),
          );
        break;

      case 'duration':
        $return += $description + array(
            'field' => array(
              'real field' => $views_field_name,
              'handler' => 'entity_views_handler_field_duration',
              'click sortable' => TRUE,
            ),
            'sort' => array(
              'real field' => $views_field_name,
              'handler' => 'views_handler_sort',
            ),
            'filter' => array(
              'real field' => $views_field_name,
              'handler' => 'views_handler_filter_numeric',
            ),
            'argument' => array(
              'real field' => $views_field_name,
              'handler' => 'views_handler_argument_numeric',
            ),
          );
        break;

      case 'uri':
        $return += $description + array(
            'field' => array(
              'real field' => $views_field_name,
              'handler' => 'views_handler_field_url',
              'click sortable' => TRUE,
            ),
            'sort' => array(
              'real field' => $views_field_name,
              'handler' => 'views_handler_sort',
            ),
            'filter' => array(
              'real field' => $views_field_name,
              'handler' => 'views_handler_filter_string',
            ),
            'argument' => array(
              'real field' => $views_field_name,
              'handler' => 'views_handler_argument_string',
            ),
          );
        break;

      case 'boolean':
        $return += $description + array(
            'field' => array(
              'real field' => $views_field_name,
              'handler' => 'views_handler_field_boolean',
              'click sortable' => TRUE,
            ),
            'sort' => array(
              'real field' => $views_field_name,
              'handler' => 'views_handler_sort',
            ),
            'filter' => array(
              'real field' => $views_field_name,
              'handler' => 'views_handler_filter_boolean_operator',
            ),
            'argument' => array(
              'real field' => $views_field_name,
              'handler' => 'views_handler_argument_string',
            ),
          );
        break;
    }

    // If there is an options list callback, add to the filter and field.
    if (isset($return['filter']) && !empty($property_info['options list'])) {
      $return['filter']['handler'] = 'views_handler_filter_in_operator';
      $return['filter']['options callback'] = array('EntityDefaultViewsController', 'optionsListCallback');
      $return['filter']['options arguments'] = array($this->type, $property_name, 'view');
    }
    // @todo: This class_exists is needed until views 3.2.
    if (isset($return['field']) && !empty($property_info['options list']) && class_exists('views_handler_field_machine_name')) {
      $return['field']['handler'] = 'views_handler_field_machine_name';
      $return['field']['options callback'] = array('EntityDefaultViewsController', 'optionsListCallback');
      $return['field']['options arguments'] = array($this->type, $property_name, 'view');
    }
    return $return;
  }

  /**
   * Determines the handler to use for a relationship to an entity type.
   *
   * @param $entity_type
   *   The entity type to join to.
   * @param $left_type
   *   The data type from which to join.
   */
  function getRelationshipHandlerClass($entity_type, $left_type) {
    // Look for an entity type which is used as bundle for the given entity
    // type. If there is one, allow filtering the relation by bundle by using
    // our own handler.
    foreach (entity_get_info() as $type => $info) {
      // In case we already join from the bundle entity we do not need to filter
      // by bundle entity any more, so we stay with the general handler.
      if (!empty($info['bundle of']) && $info['bundle of'] == $entity_type && $type != $left_type) {
        return 'entity_views_handler_relationship_by_bundle';
      }
    }
    return 'views_handler_relationship';
  }

  /**
   * A callback returning property options, suitable to be used as views options callback.
   */
  public static function optionsListCallback($type, $selector, $op = 'view') {
    $wrapper = entity_metadata_wrapper($type, NULL);
    $parts = explode(':', $selector);
    foreach ($parts as $part) {
      $wrapper = $wrapper->get($part);
    }
    return $wrapper->optionsList($op);
  }

  /**
   * Get the entity property info array of an entity type.
   *
   * @param $entity_type
   *   The entity type, e.g. node, for which the info shall be returned, or NULL
   *   to return an array with info about all types.
   *
   * @see hook_entity_property_info()
   * @see hook_entity_property_info_alter()
   */
  protected function entity_get_property_info($entity_type = NULL) {
    // Use the advanced backdrop_static() pattern, since this is called very often.
    static $backdrop_static_fast;
    if (!isset($backdrop_static_fast)) {
      $backdrop_static_fast['info'] = &backdrop_static(__FUNCTION__);
    }
    $info = &$backdrop_static_fast['info'];

    // hook_entity_property_info() includes translated strings, so each language
    // is cached separately.
    $langcode = $GLOBALS['language']->langcode;

    if (empty($info)) {
      if ($cache = cache_get("entity_property_info:$langcode")) {
        $info = $cache->data;
      }
      else {
        $info = module_invoke_all('entity_property_info');
        // Let other modules alter the entity info.
        backdrop_alter('entity_property_info', $info);
        cache_set("entity_property_info:$langcode", $info);
      }
    }
    return empty($entity_type) ? $info : (isset($info[$entity_type]) ? $info[$entity_type] : array());
  }

}
