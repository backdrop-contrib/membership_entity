<?php

/**
 * @file
 * Contains the sub-member renewal state field handler.
 *
 * It lives in Term, not Hierarchy, because the Expiry field extends it and
 * must work without Hierarchy enabled. Hierarchy registers the field itself.
 */

/**
 * Field handler saying whether a sub-member is on the parent's current term.
 *
 * The question a roster is worked through to answer is "has this person been
 * marked off yet", and neither of the columns that already exist answers it.
 * Stored status still reads active until the expiry cron catches up, so on the
 * day the list arrives every row looks identical; and a cover date on its own
 * means nothing without the parent's date to hold it against, which is the
 * comparison this saves the operator making two hundred times by eye.
 */
class views_handler_field_membership_renewal_state extends views_handler_field { // @codingStandardsIgnoreLine, views handler naming

  /**
   * Furthest active cover for every membership on the page, keyed by mid.
   *
   * Each entry is an array of 'end' and 'lifetime'.
   *
   * @var array
   */
  protected $cover = array();

  /**
   * Whether pre_render() has run and populated the cover.
   *
   * @var bool
   */
  protected $prepared = FALSE;

  /**
   * {@inheritdoc}
   */
  public function query() {
    $this->ensure_my_table();
    $this->add_additional_fields(array(
      'mid' => 'mid',
      'pid' => 'pid',
    ));
  }

  /**
   * {@inheritdoc}
   */
  public function option_definition() { // @codingStandardsIgnoreLine, views handler naming
    $options = parent::option_definition();
    $options['date_format'] = array('default' => 'custom');
    $options['custom_date_format'] = array('default' => 'j M Y');
    return $options;
  }

  /**
   * {@inheritdoc}
   */
  public function options_form(&$form, &$form_state) { // @codingStandardsIgnoreLine, views handler naming
    parent::options_form($form, $form_state);

    $form['custom_date_format'] = array(
      '#type' => 'textfield',
      '#title' => t('Date format'),
      '#default_value' => $this->options['custom_date_format'],
      '#description' => t('A PHP date format string used for the cover dates shown alongside the state.'),
    );
  }

  /**
   * Fetch the cover for every membership on the page, and their parents.
   *
   * Two queries for the whole listing rather than two per row. An association
   * roster is exactly the case where the difference is felt, since it is the
   * only listing on the site with no natural upper bound.
   */
  public function pre_render(&$values) { // @codingStandardsIgnoreLine, views handler naming
    $mids = array();
    foreach ($values as $row) {
      foreach (array('mid', 'pid') as $key) {
        $id = $this->get_value($row, $key);
        if (!empty($id)) {
          $mids[$id] = $id;
        }
      }
    }

    $this->prepared = TRUE;

    if (empty($mids)) {
      return;
    }

    $this->cover = $this->loadCover($mids);
  }

  /**
   * The furthest active cover held by each of a set of memberships.
   *
   * Deliberately the furthest rather than the current one: a membership renewed
   * before its date ran out holds two active terms at once, and the current one
   * is then the earlier of the two. Reading that would show an association that
   * has just renewed as still sitting on last year's date, and every sub-member
   * beneath it as renewed when none of them had been.
   *
   * @param array $mids
   *   The membership ids to look up.
   *
   * @return array
   *   Arrays of 'end' and 'lifetime', keyed by membership id. Memberships with
   *   no term at all are absent.
   */
  protected function loadCover(array $mids) {
    $cover = array();

    // Expired terms count towards the date deliberately. Once the cron has
    // been round, a member who has not renewed holds nothing but expired
    // terms, and reporting "no cover" of somebody who was covered until April
    // throws away the one piece of information that says whether they are a
    // lapsed member or somebody who was never on the books. An expired term
    // cannot make anybody look renewed, because it only becomes expired once
    // its end date has passed and the date being measured against is in the
    // future.
    //
    // Cancelled terms are excluded. Cancelling is a decision to end cover
    // early, so a cancelled term's end date is a date that was never reached
    // and would read here as cover that still stands.
    //
    // A lifetime term is only honoured while active, for the same reason.
    $result = db_query('SELECT mid, MAX(end) AS furthest, MAX(CASE WHEN term = :lifetime AND status = :active THEN 1 ELSE 0 END) AS lifetime FROM {membership_entity_term} WHERE mid IN (:mids) AND status <> :cancelled GROUP BY mid', array(
      ':mids' => $mids,
      ':active' => MEMBERSHIP_ENTITY_ACTIVE,
      ':cancelled' => MEMBERSHIP_ENTITY_CANCELLED,
      ':lifetime' => 'lifetime',
    ));

    foreach ($result as $row) {
      $cover[$row->mid] = array(
        'end' => $row->furthest,
        'lifetime' => (bool) $row->lifetime,
      );
    }

    return $cover;
  }

  /**
   * {@inheritdoc}
   */
  public function render($values) {
    $mid = $this->get_value($values, 'mid');
    $pid = $this->get_value($values, 'pid');

    // Not every render path calls pre_render(). Falling back costs a query,
    // which is better than reporting everybody as unrenewed.
    if (!$this->prepared) {
      $this->cover = $this->loadCover(array_filter(array($mid, $pid)));
    }

    $own = isset($this->cover[$mid]) ? $this->cover[$mid] : NULL;

    if (!empty($own['lifetime'])) {
      return $this->state('lifetime', t('Lifetime'));
    }

    // With nobody above paying, there is no date to measure against and the
    // only honest thing to show is the cover itself.
    $parent = (!empty($pid) && isset($this->cover[$pid])) ? $this->cover[$pid] : NULL;
    if (empty($parent) || empty($parent['end'])) {
      return empty($own['end'])
        ? $this->state('none', t('No cover'))
        : $this->state('unknown', t('Covered to @date', array('@date' => $this->formatDate($own['end']))));
    }

    if (!empty($own['end']) && strtotime($own['end']) >= strtotime($parent['end'])) {
      return $this->state('renewed', t('Renewed to @date', array(
        '@date' => $this->formatDate($own['end']),
      )));
    }

    return empty($own['end'])
      ? $this->state('due', t('Not renewed — no cover'))
      : $this->state('due', t('Not renewed — cover ended @date', array(
        '@date' => $this->formatDate($own['end']),
      )));
  }

  /**
   * Wrap a state in a class, so a theme can colour the column.
   *
   * @param string $state
   *   A machine name for the state.
   * @param string $text
   *   The already sanitised text to show.
   *
   * @return string
   *   Rendered markup.
   */
  protected function state($state, $text) {
    return '<span class="membership-renewal-state membership-renewal-state-' . check_plain($state) . '">' . $text . '</span>';
  }

  /**
   * Render one of the stored datetime values.
   *
   * @param string $value
   *   A datetime string as held in the term table.
   *
   * @return string
   *   The formatted date.
   */
  protected function formatDate($value) {
    return format_date(strtotime($value), 'custom', $this->options['custom_date_format'], 'UTC');
  }

}
