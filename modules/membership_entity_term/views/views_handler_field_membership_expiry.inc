<?php

/**
 * @file
 * Contains the membership expiry field handler.
 */

/**
 * Field handler for when a membership's cover runs out.
 *
 * Shows the furthest end date of the membership's terms, or a marker when that
 * date falls in the next few calendar months: a mailing list sent to a printer
 * needs to say which members get a renewal form in the envelope, and the date
 * alone leaves every printer to work that out. The furthest end, not the
 * current term's, for the reason given on loadCover().
 */
class views_handler_field_membership_expiry extends views_handler_field_membership_renewal_state { // @codingStandardsIgnoreLine, views handler naming

  /**
   * {@inheritdoc}
   */
  public function option_definition() { // @codingStandardsIgnoreLine, views handler naming
    $options = parent::option_definition();
    $options['custom_date_format'] = array('default' => 'd/m/Y');
    $options['output'] = array('default' => 'date');
    $options['months'] = array('default' => 2);
    $options['marker'] = array('default' => 'E');
    $options['lifetime_text'] = array('default' => 'Lifetime');
    return $options;
  }

  /**
   * {@inheritdoc}
   */
  public function options_form(&$form, &$form_state) { // @codingStandardsIgnoreLine, views handler naming
    parent::options_form($form, $form_state);

    $form['output'] = array(
      '#type' => 'radios',
      '#title' => t('Show'),
      '#options' => array(
        'date' => t('The expiry date'),
        'marker' => t('A marker when the membership is due for renewal'),
      ),
      '#default_value' => $this->options['output'],
    );
    $form['custom_date_format']['#states'] = array(
      'visible' => array(':input[name="options[output]"]' => array('value' => 'date')),
    );
    $form['lifetime_text'] = array(
      '#type' => 'textfield',
      '#title' => t('Text for lifetime memberships'),
      '#default_value' => $this->options['lifetime_text'],
      '#states' => array(
        'visible' => array(':input[name="options[output]"]' => array('value' => 'date')),
      ),
    );
    $form['months'] = array(
      '#type' => 'number',
      '#title' => t('Due for renewal: expiring in the next'),
      '#field_suffix' => t('calendar months'),
      '#min' => 1,
      '#default_value' => $this->options['months'],
      '#description' => t('Counted from next month: 2 in July means expiring in August or September. The current month is left out, as its members were due on the previous mailing.'),
      '#states' => array(
        'visible' => array(':input[name="options[output]"]' => array('value' => 'marker')),
      ),
    );
    $form['marker'] = array(
      '#type' => 'textfield',
      '#title' => t('Marker'),
      '#size' => 10,
      '#default_value' => $this->options['marker'],
      '#description' => t('Shown for memberships due for renewal; others are left blank.'),
      '#states' => array(
        'visible' => array(':input[name="options[output]"]' => array('value' => 'marker')),
      ),
    );
  }

  /**
   * {@inheritdoc}
   */
  public function render($values) {
    $mid = $this->get_value($values, 'mid');

    // Not every render path calls pre_render().
    if (!$this->prepared) {
      $this->cover = $this->loadCover(array_filter(array($mid)));
    }

    $own = isset($this->cover[$mid]) ? $this->cover[$mid] : NULL;
    if (empty($own) || (empty($own['end']) && empty($own['lifetime']))) {
      return '';
    }

    if ($this->options['output'] === 'marker') {
      return (empty($own['lifetime']) && $this->isDue($own['end'])) ? $this->sanitize_value($this->options['marker']) : '';
    }

    return !empty($own['lifetime']) ? $this->sanitize_value($this->options['lifetime_text']) : $this->formatDate($own['end']);
  }

  /**
   * Whether an end date falls in the renewal window.
   *
   * Compares year and month together, so a December mailing catches the
   * following January and February.
   *
   * @param string $end
   *   A datetime string as held in the term table.
   *
   * @return bool
   *   TRUE when the date is in one of the next 'months' calendar months.
   */
  protected function isDue($end) {
    $expiry = (int) format_date(strtotime($end), 'custom', 'Y', 'UTC') * 12 + (int) format_date(strtotime($end), 'custom', 'n', 'UTC');
    $now = (int) format_date(REQUEST_TIME, 'custom', 'Y') * 12 + (int) format_date(REQUEST_TIME, 'custom', 'n');
    $ahead = $expiry - $now;
    return $ahead >= 1 && $ahead <= (int) $this->options['months'];
  }

}
