<?php

/**
 * @file
 * Definition of views_handler_field_membership_term_datetime.
 */

/**
 * Field handler for membership term dates.
 */
class views_handler_field_membership_term_datetime extends views_handler_field_date { // @codingStandardsIgnoreLine, Views override compliant

  /**
   * {@inheritdoc}
   */
  public function construct() {
    parent::construct();
    $this->additional_fields['mid'] = 'mid';
    $this->additional_fields['timezone'] = 'timezone';
  }

  /**
   * {@inheritdoc}
   */
  public function option_definition() { // @codingStandardsIgnoreLine, Views override compliant
    $options = parent::option_definition();
    $options['end_of_cover'] = array('default' => FALSE, 'bool' => TRUE);
    return $options;
  }

  /**
   * {@inheritdoc}
   */
  public function options_form(&$form, &$form_state) { // @codingStandardsIgnoreLine, Views override compliant
    parent::options_form($form, $form_state);

    $form['timezone'] = array(
      '#type' => 'value',
      '#value' => 'UTC',
    );

    $form['end_of_cover'] = array(
      '#type' => 'checkbox',
      '#title' => t('Show the last moment of cover rather than the boundary'),
      '#default_value' => $this->options['end_of_cover'],
      '#description' => t('A term end date is stored as the instant cover stops, which is also the instant the next term starts — so a year from 1 April 2026 is stored as 1 April 2027, 12:00am. Displayed under a heading like "End date" that reads as though the member is covered on that day, when the last day they are actually covered is 31 March. Tick this to show 31 March 2027, 11:59pm instead. Display only: the stored value is unchanged, because renewals chain from it.'),
    );
  }

  /**
   * {@inheritdoc}
   */
  public function get_value($values, $field = NULL) { // @codingStandardsIgnoreLine, Views override compliant
    $value = parent::get_value($values, $field);
    if (empty($field) || $field == $this->real_field) {
      // A lifetime term has no end date at all — MembershipEntityTerm::save()
      // sets it to NULL deliberately. Constructing a date from that throws
      // "The DateTime object has not been correctly initialized by its
      // constructor", which took out the terms View and every lifetime
      // member's own membership page. Returning NULL lets
      // views_handler_field_date::render() emit its empty text instead.
      if (empty($value)) {
        return NULL;
      }

      $date = new BackdropDateTime($value, 'UTC');
      $timestamp = $date->format('U');

      // Step back to the last second of cover. Done here rather than in the
      // stored value because membership_entity_term_membership_entity_term_start_date_alter()
      // starts a renewal term at the previous term's end: shifting the stored
      // value would move every renewal back a second, compounding each time.
      if (!empty($this->options['end_of_cover'])) {
        $timestamp -= 1;
      }

      return $timestamp;
    }
    return $value;
  }

  /**
   * {@inheritdoc}
   */
  public function render($values) {
    // Load the membership object and settings.
    if ($mid = $this->get_value($values, 'mid')) {
      $type = db_query("SELECT type FROM {membership_entity} WHERE mid = :mid", array(':mid' => $mid))->fetchColumn();
      $bundle_settings = membership_entity_get_bundle_settings($type);

      // Force timezone to respect membership type settings.
      $this->options['timezone'] = date_get_timezone($bundle_settings['tz_handling'], $this->get_value($values, 'timezone'));
      return parent::render($values);
    }
  }

}
