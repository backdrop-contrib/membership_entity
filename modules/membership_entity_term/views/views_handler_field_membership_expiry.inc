<?php

/**
 * @file
 * Contains the membership expiry field handler.
 */

/**
 * Field handler for when a membership's cover runs out.
 *
 * Shows the furthest end date of the membership's terms, or a marker when that
 * date falls within the next number of days: a mailing list sent to a printer
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
    $options['days'] = array('default' => 60);
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
    $form['days'] = array(
      '#type' => 'number',
      '#title' => t('Due for renewal: expiring within'),
      '#field_suffix' => t('days'),
      '#min' => 1,
      '#default_value' => $this->options['days'],
      '#description' => t('Counted from when the view is run: 60 on 26 November means expiring by 25 January, including any still listed whose end date has already passed.'),
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
   * Anything ending up to that many days from now counts, including ends that
   * have already passed while the membership is still listed (the expiry cron
   * has not reached it yet). Same window as the Expiry filter.
   *
   * @param string $end
   *   A datetime string as held in the term table (UTC).
   *
   * @return bool
   *   TRUE when the date is no more than 'days' days from now.
   */
  protected function isDue($end) {
    $days = (int) $this->options['days'];
    return strtotime($end . ' UTC') <= REQUEST_TIME + ($days > 0 ? $days : 60) * 86400;
  }

}
