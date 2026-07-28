<?php

/**
 * @file
 * Contains the MembershipEntity class.
 */

/**
 * Defines a membership entity.
 */
class MembershipEntity extends Entity {
  /**
   * The primary identifier for a membership.
   *
   * @var int
   */
  public $mid;

  /**
   * A user enterable unique member id.
   *
   * @var int
   */
  public $member_id; // @codingStandardsIgnoreLine

  /**
   * The type (bundle) of membership.
   *
   * @var string
   */
  public $type;

  /**
   * The primary member.
   *
   * @var int
   */
  public $uid;

  /**
   * An array of secondary members.
   *
   * @var array
   */
  public $secondary_members; // @codingStandardsIgnoreLine

  /**
   * Integer code indicating the membership status.
   *
   * @var int
   *
   * @see membership_entity_get_status_list().
   */
  public $status;

  /**
   * The Unix timestamp when the membership was created.
   *
   * @var int
   */
  public $created;

  /**
   * The Unix timestamp when the membership was most recently saved.
   *
   * @var int
   */
  public $changed;

  /**
   * Build a new membership entity.
   */
  public function __construct($values = array()) {
    return parent::__construct($values);
  }

  /**
   * Returns the entity identifier (the entity's machine name or numeric ID).
   *
   * Left unimplemented by the Backdrop port. Core's Entity::isNew() is
   * '!empty($this->is_new) || !$this->id()', so a NULL identifier made every
   * saved membership report itself as new for the rest of its life.
   *
   * @return int|null
   *   The membership id, or NULL if the membership has not been saved.
   */
  public function id() {
    return $this->mid;
  }

  /**
   * Returns the type of the entity.
   *
   * @return
   *   The type of the entity.
   */
  public function entityType() {
    return 'membership_entity';
  }

  /**
   * Returns the bundle of the entity.
   *
   * @return
   *   The bundle of the entity. Defaults to the entity type if the entity type
   *   does not make use of different bundles.
   */
  public function bundle() {
    return $this->type ?? $this->entityType();
  }

  /**
   * Returns the label of the entity.
   *
   * The member id, as hook_entity_info() declares in its 'entity keys'. There
   * is no $label property on a membership; reading one returned NULL and
   * warned under PHP 8.
   *
   * @return string
   *   The unique member id.
   */
  public function label() {
    return $this->member_id;
  }

  /**
   * Returns the URI elements of the entity.
   *
   * Left unimplemented by the Backdrop port, while defaultUri() below was
   * written and then never reached. Backdrop's entity_uri() is simply
   * 'return $entity->uri();' — there is no 'uri callback' indirection as there
   * was in the Drupal 7 Entity API — so this returning NULL broke the links in
   * the shipped Views and made path aliases save with an empty source.
   *
   * @return array
   *   An array containing the 'path' and 'options' keys used to build the URI
   *   of the entity, and matching the signature of url().
   */
  public function uri() {
    return $this->defaultUri();
  }

  /**
   * Returns the primary member.
   */
  public function primaryMember() {
    return user_load($this->uid);
  }

  /**
   * Set the primary member for the membership.
   *
   * @param object|int $account
   *   The loaded account object or member uid.
   */
  public function setPrimaryMember($account) {
    $this->uid = is_object($account) ? $account->uid : $account;
  }

  /**
   * Returns the full url to the membership.
   */
  public function url() {
    $uri = $this->uri();
    return url($uri['path'], $uri);
  }

  /**
   * Returns the Backdrop path to the membership.
   */
  public function path() {
    $uri = $this->uri();
    return $uri['path'];
  }

  /**
   * Overrides Entity::defaultUri().
   */
  public function defaultUri() {
    return array(
      'path' => 'membership/' . $this->mid,
      'options' => array(),
    );
  }

  /**
   * Overrides Entity::save().
   */
  public function save() {
    if (empty($this->created)) {
      $this->created = REQUEST_TIME;
    }
    $this->changed = REQUEST_TIME;
    return parent::save();
  }

}
