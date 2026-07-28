<?php

/**
 * @file
 * Contains the MembershipEntityType class.
 */

/**
 * Defines a membership type.
 */
class MembershipEntityType extends Entity {

  public $id;
  public $label;
  public $type;
  public $description;
  public $weight;
  public $data;

  /**
   * Default constructor for membership types.
   *
   * @param optional|array $values
   *   An array of default values.
   */
  public function __construct($values = array()) {
    parent::__construct($values, 'membership_entity_type');
  }

  /**
   * Returns the entity identifier (the entity's machine name or numeric ID).
   *
   * @return
   *   The identifier of the entity, or NULL if the entity does not yet have
   *   an identifier.
   */
  public function id() {
    return $this->id;
  }

  /**
   * Returns the type of the entity.
   *
   * @return
   *   The type of the entity.
   */
  public function entityType() {
    return 'membership_entity_type';
  }

  /**
   * Returns the bundle of the entity.
   *
   * @return
   *   The bundle of the entity. Defaults to the entity type if the entity type
   *   does not make use of different bundles.
   */
  public function bundle() {
    return $this->type;
  }

  /**
   * Returns the label of the entity.
   *
   * @return
   *   The label of the entity, or NULL if there is no label defined.
   */
  public function label() {
    return $this->label;
  }

  /**
   * Returns the full url to the membership type.
   *
   * @param optional|string $context
   *   String containing context action.
   *
   * @return string
   *   URL to action from context, or default URL.
   */
  public function url($context = NULL) {
    $uri = $this->uri($context);
    return url($uri['path'], $uri);
  }

  /**
   * Returns the Backdrop path to the membership type.
   *
   * @param optional|string $context
   *   String containing context action.
   *
   * @return string
   *   URI to action from context, or default URI.
   */
  public function path($context = NULL) {
    $uri = $this->uri($context);
    return $uri['path'];
  }

  /**
   * {@inheritdoc}
   *
   * The context is honoured directly rather than being gated on an
   * entityInfo['uri callback'] check. Backdrop's core Entity class has no
   * entityInfo property — that belonged to the Drupal 7 Entity API base class —
   * so the guard was always false and every context fell through to the admin
   * management path, sending joins to membership type management instead of the
   * join form.
   *
   * @see https://github.com/backdrop-contrib/membership_entity/issues/4
   */
  public function uri($context = NULL) {
    switch ($context) {
      case 'join':
        return $this->joinUri();

      case 'admin':
        return $this->adminUri();

      default:
        return $this->defaultUri();
    }
  }

  /**
   * Return a path the membership type join form.
   *
   * @return array
   *   Array containing the path string, keyed by "path".
   */
  public function joinUri() {
    $type_url_str = str_replace('_', '-', $this->type);
    return array(
      'path' => 'user/join/' . $type_url_str,
      'options' => array(),
    );
  }

  /**
   * Return a path the membership type admin create form.
   *
   * @return array
   *   Array containing the path string, keyed by "path".
   */
  public function adminUri() {
    $type_url_str = str_replace('_', '-', $this->type);
    return array(
      'path' => 'admin/memberships/add/' . $type_url_str,
      'options' => array(),
    );
  }

  /**
   * {@inheritdoc}
   */
  public function defaultUri() {
    return array(
      'path' => 'admin/memberships/types/manage/' . $this->type,
      'options' => array(),
    );
  }

}
