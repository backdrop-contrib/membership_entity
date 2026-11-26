<?php

/**
 * @file
 * Contains the membership expiry filter handler.
 */

/**
 * Filter handler: memberships expiring within the next number of days.
 *
 * The Expiry field works out each membership's cover after the query has run,
 * so it cannot be filtered on. This filter does the same sum in the query: the
 * furthest end date of the membership's terms, cancelled terms left out, and
 * memberships with an active lifetime term never match. The reasons for those
 * choices are given on views_handler_field_membership_renewal_state::loadCover().
 *
 * Days only, counted from when the view is run: "60" on 26 November means
 * expiring by 25 January, the same window as the Expiry field's marker.
 */
class views_handler_filter_membership_expiry extends views_handler_filter {

  /**
   * Keeps the exposed number as given, not wrapped in an array, and makes an
   * empty exposed box mean "do not filter".
   *
   * @var bool
   */
  public $always_multiple = TRUE; // @codingStandardsIgnoreLine, views handler naming

  /**
   * {@inheritdoc}
   */
  public function option_definition() { // @codingStandardsIgnoreLine, views handler naming
    $options = parent::option_definition();
    $options['operator']['default'] = 'within';
    $options['value']['default'] = 60;
    $options['include_past'] = array('default' => FALSE, 'bool' => TRUE);
    return $options;
  }

  /**
   * {@inheritdoc}
   */
  public function operator_options($which = 'title') { // @codingStandardsIgnoreLine, views handler naming
    return array('within' => t('Expiring within'));
  }

  /**
   * {@inheritdoc}
   */
  public function value_form(&$form, &$form_state) { // @codingStandardsIgnoreLine, views handler naming
    $form['value'] = array(
      '#type' => 'number',
      '#title' => t('Expiring within'),
      '#field_suffix' => t('days'),
      '#min' => 1,
      '#size' => 5,
      '#default_value' => $this->value,
      '#description' => empty($form_state['exposed']) ? t('Counted from when the view is run. When the filter is exposed, this is the number the box starts with.') : '',
    );
  }

  /**
   * {@inheritdoc}
   */
  public function options_form(&$form, &$form_state) { // @codingStandardsIgnoreLine, views handler naming
    parent::options_form($form, $form_state);
    $form['include_past'] = array(
      '#type' => 'checkbox',
      '#title' => t('Include memberships whose expiry has already passed'),
      '#description' => t('Off: only memberships expiring from now on. On: also those whose cover has already run out, e.g. active members the expiry cron has not reached yet, or lapsed members to chase.'),
      '#default_value' => $this->options['include_past'],
    );
  }

  /**
   * {@inheritdoc}
   */
  public function admin_summary() { // @codingStandardsIgnoreLine, views handler naming
    if ($this->is_a_group()) {
      return t('grouped');
    }
    if (!empty($this->options['exposed'])) {
      return t('exposed');
    }
    return format_plural((int) $this->value, 'within 1 day', 'within @count days');
  }

  /**
   * {@inheritdoc}
   */
  public function query() {
    $days = is_numeric($this->value) ? (int) $this->value : 0;
    if ($days < 1) {
      return;
    }

    // Term end dates are held in UTC.
    $this->ensure_my_table();
    $to = $this->placeholder();
    $cancelled = $this->placeholder();
    $active = $this->placeholder();
    $lifetime = $this->placeholder();
    $args = array(
      $to => gmdate('Y-m-d H:i:s', REQUEST_TIME + $days * 86400),
      $cancelled => MEMBERSHIP_ENTITY_CANCELLED,
      $active => MEMBERSHIP_ENTITY_ACTIVE,
      $lifetime => 'lifetime',
    );
    $having = "MAX(t.end) <= $to";
    if (empty($this->options['include_past'])) {
      $from = $this->placeholder();
      $args[$from] = gmdate('Y-m-d H:i:s', REQUEST_TIME);
      $having .= " AND MAX(t.end) >= $from";
    }
    $having .= " AND MAX(CASE WHEN t.term = $lifetime AND t.status = $active THEN 1 ELSE 0 END) = 0";

    $this->query->add_where_expression($this->options['group'], "$this->table_alias.mid IN (SELECT t.mid FROM {membership_entity_term} t WHERE t.status <> $cancelled GROUP BY t.mid HAVING $having)", $args);
  }

}
