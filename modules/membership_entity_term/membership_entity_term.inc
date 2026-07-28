<?php

/**
 * @file
 * Contains the MembershipEntity class.
 */

/**
 * Defines a membership entity.
 */
class MembershipEntityTerm extends Entity {
  /**
   * The primary term identifier.
   *
   * @var int
   */
  public $id;

  /**
   * The membership identifier.
   *
   * @var int
   */
  public $mid;

  /**
   * The human readable term length. eg. 1 year.
   *
   * @var string
   */
  public $term;

  /**
   * An array term length modifiers. eg. +1 month.
   *
   * @var array
   */
  public $modifiers;

  /**
   * The date when this term starts.
   *
   * @var string
   */
  public $start;

  /**
   * The date when this term ends.
   *
   * @var string
   */
  public $end;

  /**
   * The Unix timestamp when the membership term was created.
   *
   * @var int
   */
  public $created;

  /**
   * The Unix timestamp when the membership term was most recently saved.
   *
   * @var int
   */
  public $changed;

  /**
   * Build a new membership entity term.
   */
  public function __construct($values = array()) {
    return parent::__construct($values, 'membership_entity_term');
  }

  /**
   * Returns the Membership Entity.
   */
  public function membership() {
    return membership_entity_load($this->mid);
  }

  /**
   * Sets the Membership Entity.
   *
   * @param mixed $membership
   *   The Membership Entity object or the Membership Entity id (mid).
   */
  public function setMembership($membership) {
    $this->mid = is_object($membership) ? $membership->mid : $membership;
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
    return 'membership_entity_term';
  }

  /**
   * Returns the bundle of the entity.
   *
   * @return
   *   The bundle of the entity. Defaults to the entity type if the entity type
   *   does not make use of different bundles.
   */
  public function bundle() {
    return $this->entityType();
  }

  /**
   * Returns the label of the entity.
   *
   * Backdrop's entity_label() calls this method directly and ignores the
   * 'label callback' declared in hook_entity_info(), so delegate to that
   * callback here. There is no $label property on a term; reading one returned
   * NULL and warned under PHP 8.
   *
   * @return string
   *   The member id, term length and start date.
   */
  public function label() {
    return membership_entity_term_label($this, $this->entityType());
  }

  /**
   * Returns the URI elements of the entity.
   *
   * Left unimplemented by the Backdrop port, while defaultUri() below was
   * written and then never reached — see MembershipEntity::uri() for the same
   * defect and why it matters.
   *
   * @return array
   *   An array containing the 'path' and 'options' keys used to build the URI
   *   of the entity, and matching the signature of url().
   */
  public function uri() {
    return $this->defaultUri();
  }

  /**
   * Returns the full url to the membership term.
   */
  public function url() {
    $uri = $this->uri();
    return url($uri['path'], $uri);
  }

  /**
   * Returns the Backdrop path to the membership term.
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
      'path' => "membership/$this->mid/term/$this->id",
      'options' => array(),
    );
  }

  /**
   * Overrides Entity::save().
   */
  public function save() {
    // Check for valid term length that is understood by DateTime::modify().
    if (preg_match(MEMBERSHIP_ENTITY_TERM_LENGTH_REGEX, $this->term)) {
      $this->calculateEndDate();
    }
    elseif ($this->term == 'lifetime') {
      $this->end = NULL;
    }

    // Update created and changed dates.
    if (empty($this->created)) {
      $this->created = REQUEST_TIME;
    }
    $this->changed = REQUEST_TIME;

    return parent::save();
  }

  /**
   * Calculate the term end date.
   */
  protected function calculateEndDate() {
    // Add the term length.
    $end = new BackdropDateTime($this->start, new DateTimezone($this->timezone));
    $end = _membership_entity_term_modify_date($end, $this->term);

    // Add modifiers.
    foreach ($this->modifiers as $modifier) {
      $end = _membership_entity_term_modify_date($end, $modifier);
    }

    // Allow other modules to override the end date.
    backdrop_alter('membership_entity_term_end_date', $end, $this);

    $this->end = date_format_date($end, 'custom', DATE_FORMAT_DATETIME);
  }

}
