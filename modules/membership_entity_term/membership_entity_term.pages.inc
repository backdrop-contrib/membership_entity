<?php

/**
 * @file
 * Page callbacks for the membership_entity_term module.
 */

/**
 * Add term options to the membership settings form.
 */
function _membership_entity_term_settings_form(&$form, &$form_state, $data) {
  // Make date timezone options available.
  form_load_include($form_state, 'inc', 'date', 'date.admin');

  // Add a grace period.
  $grace_options = array(
    'none' => t('None'),
    '1 day' => t('1 day'),
    '2 days' => t('2 days'),
    '3 days' => t('3 days'),
    '1 week' => t('1 week'),
    '2 weeks' => t('2 weeks'),
    '3 weeks' => t('3 weeks'),
    '1 month' => t('1 month'),
    '2 months' => t('2 months'),
    '3 months' => t('3 months'),
    '6 months' => t('6 months'),
    'other' => t('Other'),
  );

  // A club whose year runs to a fixed date charges the same whenever somebody
  // joins, and renews everybody together. Offered as the last day of cover
  // rather than as the instant cover stops, because 31 March is the date a
  // member recognises and 1 April is the one that gets entered by mistake.
  $fixed = explode('-', $data['fixed_renewal_date']);
  $fixed_day = isset($fixed[0]) ? (int) $fixed[0] : 0;
  $fixed_month = isset($fixed[1]) ? (int) $fixed[1] : 0;

  $form['membership_settings']['fixed_renewal'] = array(
    '#type' => 'container',
    '#tree' => FALSE,
  );

  $form['membership_settings']['fixed_renewal']['fixed_renewal_enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('All memberships of this type renew on the same date each year'),
    '#default_value' => !empty($data['fixed_renewal_date']),
    '#description' => t('Leave this off for terms that simply run for their length from the day they start. Turn it on for a membership year: every term then ends on the date below, whenever it began, so somebody joining part way through is covered until the next renewal rather than for a full term.'),
  );

  $form['membership_settings']['fixed_renewal']['fixed_renewal_day'] = array(
    '#type' => 'select',
    '#title' => t('Last day of cover'),
    '#options' => backdrop_map_assoc(range(1, 31)),
    '#default_value' => $fixed_day ? $fixed_day : 31,
    '#states' => array(
      'visible' => array(
        ':input[name="fixed_renewal_enabled"]' => array('checked' => TRUE),
      ),
    ),
  );

  $form['membership_settings']['fixed_renewal']['fixed_renewal_month'] = array(
    '#type' => 'select',
    '#title' => t('Month'),
    '#options' => array(
      1 => t('January'), 2 => t('February'), 3 => t('March'),
      4 => t('April'), 5 => t('May'), 6 => t('June'),
      7 => t('July'), 8 => t('August'), 9 => t('September'),
      10 => t('October'), 11 => t('November'), 12 => t('December'),
    ),
    '#default_value' => $fixed_month ? $fixed_month : 3,
    '#description' => t('The last day members are covered. Cover stops at midnight afterwards, so a year ending 31 March is stored as 1 April — which is also the day the next term begins.'),
    '#states' => array(
      'visible' => array(
        ':input[name="fixed_renewal_enabled"]' => array('checked' => TRUE),
      ),
    ),
  );

  $form['#validate'][] = '_membership_entity_term_settings_fixed_renewal_validate';

  // Which view draws the term ledger on a membership page. Offered as a setting
  // because the shipped one is a starting point, not a decision: an
  // organisation that wants its ledger to read as a history rather than a table
  // should be able to build that view and point this at it.
  $ledger_options = array();
  foreach (views_get_all_views() as $view_name => $view) {
    if ($view->base_table != 'membership_entity_term' || !empty($view->disabled)) {
      continue;
    }
    foreach ($view->display as $display_id => $display) {
      $ledger_options[$view_name . ':' . $display_id] = t('@view — @display', array(
        '@view' => $view->human_name ? $view->human_name : $view_name,
        '@display' => $display->display_title,
      ));
    }
  }

  if (!empty($ledger_options)) {
    $form['membership_settings']['term_list_view'] = array(
      '#type' => 'select',
      '#title' => t('Term list'),
      '#options' => $ledger_options,
      '#default_value' => !empty($data['term_list_view']) ? $data['term_list_view'] : 'membership_entity_terms:term_list',
      '#description' => t('The view used to list this membership type\'s terms on its page. Where it appears, and whether it appears at all, is set under Manage Display.'),
    );
  }

  $form['membership_settings']['grace_period'] = array(
    '#type' => 'select',
    '#title' => t('Grace period'),
    '#description' => t('Allow a grace period for membership renewals. There will be no lapse in membership terms if a membership is renewed within the grace period.'),
    '#options' => $grace_options,
    '#default_value' => in_array($data['grace_period'], array_keys($grace_options)) ? $data['grace_period'] : 'other',
  );

  $form['membership_settings']['custom_grace_period'] = array(
    '#type' => 'textfield',
    '#title' => t('Other grace period'),
    '#size' => 32,
    '#description' => t('Enter a grace period including a number and unit (eg. 15 days).'),
    '#default_value' => $data['grace_period'],
    '#states' => array(
      'visible' => array(
        ':input[name="grace_period"]' => array('value' => 'other'),
      ),
      'required' => array(
        ':input[name="grace_period"]' => array('value' => 'other'),
      ),
    ),
  );

  // Add membership term lengths.
  if (!isset($form_state['membership_entity_term']['available_term_lengths'])) {
    $form_state['membership_entity_term']['available_term_lengths'] = $data['available_term_lengths'];
  }

  $form['term_lengths'] = array(
    '#type' => 'fieldset',
    '#title' => t('Membership Term Lengths'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#tree' => TRUE,
  );

  $form['term_lengths']['tz_handling'] = array(
    '#type' => 'select',
    '#title' => t('Timezone handling'),
    '#description' => t('Select the timezone handling method for term dates. See <a href="@url">time zone handling</a> for details.', array(
      '@url' => url('https://www.backdrop.org/node/1455578', array('external' => TRUE)),
    )),
    '#default_value' => $data['tz_handling'],
    '#options' => date_timezone_handling_options(),
  );

  $form['term_lengths']['timezone_db'] = array(
    '#type' => 'hidden',
    '#value' => $data['timezone_db'],
  );

  $wrapper_id = 'available-term-lengths-wrapper';
  $form['term_lengths']['available_term_lengths'] = array(
    '#theme' => 'membership_entity_term_available_term_lengths_element',
    '#id' => 'membership-entity-term-lengths',
    '#prefix' => '<div id="' . $wrapper_id . '">',
    '#suffix' => '</div>',
    '#element_validate' => array('membership_entity_term_length_element_validate'),
    '#description' => t('Select the term lengths that will be available for this membership type.'),
  );

  if (!empty($form_state['membership_entity_term']['available_term_lengths'])) {
    foreach ($form_state['membership_entity_term']['available_term_lengths'] as $delta => $term_length) {
      $form['term_lengths']['available_term_lengths'][$delta] = array(
        'term_length' => array(
          '#type' => 'value',
          '#value' => $term_length,
        ),
        'example' => array(
          '#markup' => ($term_length == 'lifetime') ? t('N/A') : format_date(strtotime($term_length), 'custom', 'Y-m-d H:i:s O'),
        ),
        'weight' => array(
          '#type' => 'weight',
          '#title' => t('Weight for term length @term_length', array('@term_length' => $term_length)),
          '#title_display' => 'invisible',
          '#default_value' => $delta,
        ),
        'remove' => array(
          '#type' => 'submit',
          '#value' => t('Remove'),
          '#name' => 'available_term_lengths_remove_' . $delta,
          '#delta' => $delta,
          '#submit' => array('membership_entity_term_length_remove'),
          '#limit_validation_errors' => array(),
          '#ajax' => array(
            'callback' => 'membership_entity_term_length_ajax_callback',
            'wrapper' => $wrapper_id,
            'effect' => 'fade',
          ),
        ),
      );
    }
  }

  $form['term_lengths']['term_length_options'] = array(
    '#type' => 'select',
    '#title' => t('Term length'),
    '#options' => array(
      '1 day' => t('1 day'),
      '1 week' => t('1 week'),
      '1 month' => t('1 month'),
      '3 months' => t('3 months'),
      '6 months' => t('6 months'),
      '1 year' => t('1 year'),
      '2 years' => t('2 years'),
      '3 years' => t('3 years'),
      '4 years' => t('4 years'),
      '5 years' => t('5 years'),
      '10 years' => t('10 years'),
      'lifetime' => t('Lifetime'),
      'other' => t('Other'),
    ),
  );

  $form['term_lengths']['custom_term_length'] = array(
    '#type' => 'textfield',
    '#title' => t('Other term length'),
    '#size' => 32,
    '#description' => t('Enter a term length including number and unit (eg. 15 years).'),
    '#states' => array(
      'visible' => array(
        ':input[name="term_lengths[term_length_options]"]' => array('value' => 'other'),
      ),
      'required' => array(
        ':input[name="term_lengths[term_length_options]"]' => array('value' => 'other'),
      ),
    ),
  );

  $form['term_lengths']['add'] = array(
    '#type' => 'submit',
    '#value' => t('Add term length'),
    '#validate' => array('membership_entity_term_length_add_validate'),
    '#submit' => array('membership_entity_term_length_add'),
    '#limit_validation_errors' => array(
      array('term_lengths', 'term_length_options'),
      array('term_lengths', 'custom_term_length'),
    ),
    '#ajax' => array(
      'callback' => 'membership_entity_term_length_ajax_callback',
      'wrapper' => $wrapper_id,
      'effect' => 'fade',
    ),
  );

  $form['#validate'][] = 'membership_entity_term_settings_form_validate';
}

/**
 * Ajax callback to update term lengths.
 */
function membership_entity_term_length_ajax_callback($form, $form_state) {
  return $form['term_lengths']['available_term_lengths'];
}

/**
 * Validate handler to add a term length.
 */
function membership_entity_term_length_add_validate($form, &$form_state) {
  $values = &$form_state['values']['term_lengths'];
  $term_length = $values['term_length_options'];

  if (empty($term_length)) {
    form_set_error('term_lengths][term_length_options', t('Please select a a term length.'));
  }

  // Validate a custom term length.
  if ($term_length == 'other') {
    $term_length = trim($values['custom_term_length']);
    if (!preg_match(MEMBERSHIP_ENTITY_TERM_LENGTH_REGEX, $term_length)) {
      form_set_error('term_lengths][custom_term_length', t('%term_length is not a valid term length. Please enter term lengths as a number and unit (eg. 15 years)', array(
        '%term_length' => $term_length,
      )));
    }
  }

  // Check for duplicates.
  if (in_array($term_length, $form_state['membership_entity_term']['available_term_lengths'])) {
    form_set_error('term_lengths][term_length_options', t('The term length %term_length has already been added.', array(
      '%term_length' => $term_length,
    )));
  }
}

/**
 * Submit handler to add a term length.
 */
function membership_entity_term_length_add($form, &$form_state) {
  $values = $form_state['values']['term_lengths'];
  $term = ($values['term_length_options'] != 'other') ? $values['term_length_options'] : $term = $values['custom_term_length'];
  $form_state['membership_entity_term']['available_term_lengths'][] = check_plain($term);
  $form_state['rebuild'] = TRUE;
}

/**
 * Submit handler to remove a term length.
 */
function membership_entity_term_length_remove($form, &$form_state) {
  $button = $form_state['triggering_element'];
  if (isset($button['#delta'])) {
    unset($form_state['membership_entity_term']['available_term_lengths'][$button['#delta']]);
  }
  $form_state['rebuild'] = TRUE;
}

/**
 * Form validation handler for membership_entity_settings_form().
 */
function membership_entity_term_settings_form_validate($form, &$form_state) {
  $values = &$form_state['values'];

  // Save grace period.
  if ($values['grace_period'] == 'other') {
    form_set_value($form['membership_settings']['grace_period'], $values['custom_grace_period'], $form_state);
  }

  // Save and flatten timezone settings.
  $values['tz_handling'] = $values['term_lengths']['tz_handling'];
  if ($values['tz_handling'] == 'none') {
    $values['timezone_db'] = '';
  }
  else {
    $values['timezone_db'] = date_get_timezone_db($values['tz_handling']);
  }

  // Flatten available term lengths.
  if (!empty($values['term_lengths']['available_term_lengths'])) {
    $values['available_term_lengths'] = $values['term_lengths']['available_term_lengths'];
  }

  // Remove nested values.
  unset($values['term_lengths']);
}

/**
 * Element validate callback for the available term lengths field.
 */
function membership_entity_term_length_element_validate($element, &$form_state, $form) {
  $items = element_children($element);

  // Make sure at least one term length is selected.
  if (empty($items)) {
    form_error($element, t('You must add at least one term length.'));
    return;
  }

  // Prepare available term_lengths.
  $term_lengths = array();
  foreach ($items as $delta) {
    $term_lengths[$element[$delta]['weight']['#value']] = $element[$delta]['term_length']['#value'];
  }
  ksort($term_lengths);
  form_set_value($element, $term_lengths, $form_state);
}

/**
 * Validate handler: fold the fixed renewal date into its stored form.
 *
 * The form asks three questions — on or off, a day and a month — and the
 * setting is one string, "DD-MM", empty when the feature is off. Also rejects a
 * date that does not exist, since the day and month selects cannot know about
 * each other.
 *
 * @see _membership_entity_term_settings_form()
 */
function _membership_entity_term_settings_fixed_renewal_validate($form, &$form_state) {
  $values = &$form_state['values'];

  if (empty($values['fixed_renewal_enabled'])) {
    $values['fixed_renewal_date'] = '';
    return;
  }

  $day = (int) $values['fixed_renewal_day'];
  $month = (int) $values['fixed_renewal_month'];

  // 31 February and 31 September do not exist, and checkdate is the cheapest
  // way to say so. A leap year is used deliberately, so that 29 February is
  // allowed to be chosen — it simply falls back to the 28th in other years.
  if (!checkdate($month, $day, 2024)) {
    form_set_error('fixed_renewal_day', t('%day %month is not a date. Choose a day that exists in that month.', array(
      '%day' => $day,
      '%month' => $form['membership_settings']['fixed_renewal']['fixed_renewal_month']['#options'][$month],
    )));
    return;
  }

  $values['fixed_renewal_date'] = $day . '-' . $month;
}

/**
 * Add term length options to the membership_entity_form.
 */
function membership_entity_term_create_form(&$form, &$form_state) {
  $membership = $form['#membership_entity'];
  $bundle_settings = $form['#bundle_settings'];

  // Membership status is now determined by terms.
  unset($form['membership']['status']);

  if (!empty($membership->is_new)) {
    $form['#membership_entity_term'] = $term = membership_entity_term_create(array(
      'membership_entity_type' => $membership->type,
    ));

    // Where the parent is already fixed — which is the whole of the "add
    // sub-member" route — and that parent pays for its sub-members, every term
    // control on this form is decoration: the submit handler discards whatever
    // is entered and copies the parent's term down instead. Say what the term
    // will be rather than asking for one that cannot be honoured.
    //
    // The generic add form is deliberately left alone. There the parent is
    // chosen from a select of memberships whose types may differ in this
    // setting, so whether the term is inherited is not known until the form
    // comes back.
    if (!empty($membership->pid) && membership_entity_child_inherits_terms($membership)) {
      _membership_entity_term_inherited_notice($form, $membership);
      $form['actions']['submit']['#submit'][] = 'membership_entity_term_create_form_submit';
      return;
    }

    // Add membership term fields.
    membership_entity_term_form($form, $form_state);
    $form['#validate'][] = 'membership_entity_term_form_validate';

    // Hide fields where the 'membership_entity_join_form' setting is turned
    // off.
    foreach (field_info_instances('membership_entity_term', $term->membership_entity_type) as $field_name => $instance) {
      if (empty($instance['settings']['membership_entity_join_form'])) {
        $form['membership_term'][$field_name]['#access'] = FALSE;
      }
    }

    $form['actions']['submit']['#submit'][] = 'membership_entity_term_create_form_submit';
  }
}

/**
 * State the term a sub-member will be given, in place of asking for one.
 *
 * Shown where the membership above pays for everyone beneath it. The date comes
 * from the same lookup the submit handler uses, so what is promised here is
 * what is written.
 *
 * @param array $form
 *   The membership form, altered in place.
 * @param MembershipEntity $membership
 *   The sub-member being created, whose parent is already fixed.
 */
function _membership_entity_term_inherited_notice(&$form, MembershipEntity $membership) {
  $parent_term = membership_entity_term_get_current($membership->pid);
  $parent = membership_entity_parent_label($membership->pid);

  if ($parent_term) {
    $message = t('This membership is paid for by @parent, so it takes that membership’s term and will run to @date. It is renewed whenever @parent is.', array(
      '@parent' => $parent,
      '@date' => format_date(strtotime($parent_term->end), 'custom', 'j F Y'),
    ));
    $type = 'status';
  }
  else {
    // Worth saying before the form is submitted rather than after: the person
    // filling it in may want to renew the membership above first.
    $message = t('This membership is paid for by @parent, but @parent has no current term, so this member will start with none. They will be covered from the next time @parent is renewed.', array(
      '@parent' => $parent,
    ));
    $type = 'warning';
  }

  $form['membership_term_inherited'] = array(
    '#type' => 'item',
    '#title' => t('Term'),
    '#markup' => '<div class="messages ' . $type . '">' . $message . '</div>',
    '#weight' => 20,
  );
}

/**
 * Submit handler for creating new terms.
 */
function membership_entity_term_create_form_submit($form, &$form_state) {
  $membership = &$form_state['membership_entity'];
  $term = $form['#membership_entity_term'];

  // Where the chosen parent pays for all of its sub-members, the term is not the
  // one entered on this form: it is the parent's, copied down so that the two
  // run to the same date. The parent is only known once the form has been
  // submitted, which is why any term length entered above is discarded here
  // rather than hidden earlier.
  //
  // The sub-member does get a term of their own, rather than none at all. It is
  // what gives them a joining date, a renewal history, and a lapse that can be
  // seen and explained — a membership with no terms sits at active for ever and
  // leaves no record of any of it.
  if (membership_entity_child_inherits_terms($membership)) {
    if ($copied = membership_entity_term_copy_from_parent($membership, $membership->pid)) {
      $membership->terms[$copied->id] = $copied;
      $form_state['membership_entity_term'] = $copied;
      backdrop_set_message(t('This membership is paid for by the membership it belongs to, and has been given a term running to the same date, @date.', array(
        '@date' => format_date(strtotime($copied->end), 'custom', 'j F Y'),
      )));
    }
    else {
      backdrop_set_message(t('This membership is paid for by the membership it belongs to, but that membership has no current term, so no term could be given. It will be covered from the next time the membership above it is renewed.'), 'warning');
    }
    return;
  }

  form_state_values_clean($form_state);
  _membership_entity_form_submit_build_entity('membership_entity_term', $term, $form['membership_term'], $form_state);

  // Add the new membership term.
  $term->mid = $membership->mid;

  $status = membership_entity_term_save($term);
  $membership->terms[$term->id] = $term;
  $form_state['membership_entity_term'] = $term;
}

/**
 * Form builder to edit membership terms.
 */
function membership_entity_term_edit($form, &$form_state, $membership, $term) {
  $form['#membership_entity'] = $membership;
  $form['#membership_entity_term'] = $term;
  $form['#entity_type'] = 'membership_entity_term';
  $form['#attributes']['class'][] = 'membership-entity-term-edit-form';
  $bundle_settings = $form['#bundle_settings'] = membership_entity_get_bundle_settings($membership->type);

  if (empty($form['#bundle_settings'])) {
    return '';
  }

  membership_entity_term_form($form, $form_state);
  $form['#validate'][] = 'membership_entity_term_form_validate';

  // Add the buttons.
  $form['actions'] = array(
    '#type' => 'actions',
    '#weight' => 9999,
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Save'),
      '#submit' => array('membership_entity_term_edit_form_submit'),
    ),
    'delete' => array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#limit_validation_errors' => array(),
      '#submit' => array('membership_entity_term_edit_form_delete'),
    ),
  );

  return $form;
}

/**
 * Submit the membership entity term edit form.
 */
function membership_entity_term_edit_form_submit($form, &$form_state) {
  $term = $form['#membership_entity_term'];

  form_state_values_clean($form_state);
  _membership_entity_form_submit_build_entity('membership_entity_term', $term, $form['membership_term'], $form_state);

  $status = membership_entity_term_save($term);

  // Add the saved term object to $form_state for other modules.
  $form_state['membership_entity_term'] = $term;
  switch ($status) {
    case SAVED_UPDATED:
      backdrop_set_message(t('The membership term has been updated.'));
      $form_state['redirect'] = 'membership/' . $term->mid;
      break;
  }
}

/**
 * Helper to add default and field api membership entity term fields.
 */
function membership_entity_term_form(&$form, &$form_state) {
  $membership = $form['#membership_entity'];
  $term = $form['#membership_entity_term'];
  $bundle_settings = $form['#bundle_settings'];

  $form['#membership_entity_term_form'] = TRUE;

  $form['membership_term'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('membership-entity-term-info'),
    ),
    '#parents' => array('membership_term'),
    '#tree' => TRUE,
  );

  // Basic information.
  foreach (array('id', 'mid', 'membership_entity_type') as $key) {
    $form['membership_term'][$key] = array(
      '#type' => 'value',
      '#value' => isset($term->{$key}) ? $term->{$key} : NULL,
    );
  }

  // Allow other modules to override the start date.
  $start = new BackdropDateTime($term->start, 'UTC');
  backdrop_alter('membership_entity_term_start_date', $start, $term);

  // Convert to appropriate timezone for display.
  $timezone = date_get_timezone($bundle_settings['tz_handling'], $term->timezone);
  $start->setTimezone(new DateTimeZone($timezone));

  $form['membership_term']['start'] = array(
    '#type' => 'date_select',
    '#title' => t('Term start date'),
    '#description' => t('Select the new membership term start date. The term end date will be calculated automatically based on the start date, term length, and any modifiers.'),
    '#date_format' => DATE_FORMAT_DATETIME,
    '#default_value' => date_format_date($start, 'custom', DATE_FORMAT_DATETIME),
    '#date_timezone' => $timezone,
    '#date_text_parts' => array('year'),
    '#date_increment' => 1,
    '#date_year_range' => '-10:+10',
    '#date_label_position' => 'above',
    '#access' => user_access('administer membership terms'),
  );

  // The date popup widget doesn't work correctly when #access is FALSE.
  if (module_exists('date_popup') && $form['membership_term']['start']['#access']) {
    $form['membership_term']['start']['#type'] = 'date_popup';
  }

  if ($bundle_settings['tz_handling'] == 'date') {
    $form['membership_term']['timezone'] = array(
      '#type' => 'date_timezone',
      '#theme_wrappers' => array('date_timezone'),
      '#default_value' => $timezone,
      '#attributes' => array('class' => array('date-no-float')),
      '#date_label_position' => 'above',
      '#access' => user_access('administer membership terms'),
    );
  }
  elseif ($bundle_settings['tz_handling'] != 'none') {
    $form['membership_term']['timezone'] = array(
      '#type' => 'hidden',
      '#value' => 'UTC',
    );
  }

  // A membership type whose term settings have never been saved offers no term
  // lengths, which leaves a required radios element with no options — a form
  // that cannot be submitted and gives no clue why. Say what is wrong, and to
  // whom it is worth saying it.
  $options = !empty($bundle_settings['available_term_lengths'])
    ? backdrop_map_assoc($bundle_settings['available_term_lengths'])
    : array();

  if (empty($options)) {
    if (user_access('administer membership types')) {
      backdrop_set_message(t('No term lengths have been configured for this membership type, so it cannot be joined or renewed yet. <a href="!url">Add at least one term length</a>.', array(
        '!url' => url('admin/memberships/types/manage/' . $membership->type),
      )), 'error');
    }
    else {
      backdrop_set_message(t('This membership type is not available at the moment. Please contact the site administrator.'), 'error');
    }
  }

  $form['membership_term']['term'] = array(
    '#type' => 'radios',
    '#title' => t('Term length'),
    '#description' => t('Select the membership term length.'),
    '#default_value' => !empty($term->term) ? $term->term : (count($options) <= 1 ? reset($options) : NULL),
    '#required' => TRUE,
    '#options' => $options,
  );

  $form['membership_term']['status'] = array(
    '#type' => 'radios',
    '#title' => t('Status'),
    '#options' => membership_entity_get_status_list(),
    '#default_value' => $term->status,
    '#required' => TRUE,
    '#access' => user_access('administer membership terms'),
  );

  // Add term modifiers.
  if (!isset($form_state['membership_term']['modifiers'])) {
    $form_state['membership_term']['modifiers'] = $term->modifiers;
  }

  $form['membership_term']['modifiers'] = array(
    '#type' => 'fieldset',
    '#title' => t('Term Modifiers'),
    '#description' => t('Modifiers are used to extend or reduce the term length by a set amount of time. eg. +1 month.'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#access' => user_access('administer membership terms'),
    '#tree' => TRUE,
    '#states' => array(
      'invisible' => array(
        ':input[name="membership_term[term]"]' => array('value' => 'lifetime'),
      ),
    ),
  );

  $wrapper_id = 'term-modifiers-wrapper';
  $form['membership_term']['modifiers']['list'] = array(
    '#theme' => 'membership_entity_term_modifiers_element',
    '#id' => 'membership-entity-term-modifiers',
    '#prefix' => '<div id="' . $wrapper_id . '">',
    '#suffix' => '</div>',
  );

  foreach ($form_state['membership_term']['modifiers'] as $delta => $modifier) {
    $form['membership_term']['modifiers']['list'][$delta] = array(
      'value' => array(
        '#type' => 'value',
        '#value' => $modifier,
      ),
      'remove' => array(
        '#type' => 'submit',
        '#value' => t('Remove'),
        '#name' => 'modifiers_remove_' . $delta,
        '#delta' => $delta,
        '#submit' => array('membership_entity_term_modifier_remove'),
        '#limit_validation_errors' => array(),
        '#ajax' => array(
          'callback' => 'membership_entity_term_modifier_ajax_callback',
          'wrapper' => $wrapper_id,
          'effect' => 'fade',
        ),
      ),
    );
  }

  $form['membership_term']['modifiers']['options'] = array(
    '#type' => 'select',
    '#title' => t('Select Modifier'),
    '#options' => array(
      '+1 day' => t('+1 day'),
      '+1 week' => t('+1 week'),
      '+1 month' => t('+1 month'),
      '+3 months' => t('+3 months'),
      '+6 months' => t('+6 months'),
      'other' => t('Other'),
    ),
  );

  $form['membership_term']['modifiers']['custom'] = array(
    '#type' => 'textfield',
    '#title' => t('Other modifier'),
    '#size' => 32,
    '#description' => t('Enter a term modifier as a sign (+-), number, and unit (eg. +1 month).'),
    '#states' => array(
      'visible' => array(
        ':input[name="membership_term[modifiers][options]"]' => array('value' => 'other'),
      ),
      'required' => array(
        ':input[name="membership_term[modifiers][options]"]' => array('value' => 'other'),
      ),
    ),
  );

  $form['membership_term']['modifiers']['add'] = array(
    '#type' => 'submit',
    '#value' => t('Add modifier'),
    '#validate' => array('membership_entity_term_modifier_add_validate'),
    '#submit' => array('membership_entity_term_modifier_add'),
    '#limit_validation_errors' => array(array('membership_term', 'modifiers')),
    '#ajax' => array(
      'callback' => 'membership_entity_term_modifier_ajax_callback',
      'wrapper' => $wrapper_id,
      'effect' => 'fade',
    ),
  );

  // Add field UI items.
  field_attach_form('membership_entity_term', $term, $form['membership_term'], $form_state);
  $form['membership_term']['#pre_render'][] = '_membership_entity_extra_fields_pre_render';

  // Allow other modules to alter the form.
  backdrop_alter('membership_entity_term_form', $form['membership_term'], $form_state, $term);
}

/**
 * Validate the membership entity term form.
 *
 * @see membership_entity_form_term()
 */
function membership_entity_term_form_validate($form, &$form_state) {
  $values = &$form_state['values']['membership_term'];

  // Prepare available term modifiers.
  if (empty($values['modifiers']['list']) || $values['term'] == 'lifetime') {
    form_set_value($form['membership_term']['modifiers'], array(), $form_state);
  }
  else {
    $modifiers = array_map('_membership_entity_term_modifier_array_map', $values['modifiers']['list']);
    form_set_value($form['membership_term']['modifiers'], $modifiers, $form_state);
  }

  // Handle timezone conversion before saving.
  $start = new BackdropDateTime($values['start'], $form['membership_term']['start']['#date_timezone']);
  $start->setTimezone(new DateTimeZone($values['timezone']));
  form_set_value($form['membership_term']['start'], date_format_date($start, 'custom', DATE_FORMAT_DATETIME), $form_state);

  _membership_entity_form_field_validate('membership_entity_term', $form['membership_term'], $form_state);
}

/**
 * Ajax callback to update term modifiers.
 */
function membership_entity_term_modifier_ajax_callback($form, $form_state) {
  return $form['membership_term']['modifiers']['list'];
}

/**
 * Validate handler to add a term modifier.
 */
function membership_entity_term_modifier_add_validate($form, &$form_state) {
  $values = &$form_state['values']['membership_term'];
  $modifier = $values['modifiers']['options'];

  // Validate a custom term length.
  if ($modifier == 'other') {
    $modifier = trim($values['modifiers']['custom']);
    // Multiple intervals in one string (eg. +1 month 1 day) are not supported.
    if (!preg_match(MEMBERSHIP_ENTITY_TERM_MODIFIER_REGEX, $modifier)) {
      form_set_error('membership_term][modifiers][custom', t('%modifier is not a valid term modifier. Please enter modifiers as a sign (+-), number, and unit (eg. +1 month)', array(
        '%modifier' => $modifier,
      )));
    }
  }
}

/**
 * Submit handler to add a term modifier.
 */
function membership_entity_term_modifier_add($form, &$form_state) {
  $values = $form_state['values']['membership_term'];
  $modifier = ($values['modifiers']['options'] != 'other') ? $values['modifiers']['options'] : $values['modifiers']['custom'];
  $form_state['membership_term']['modifiers'][] = check_plain($modifier);
  $form_state['rebuild'] = TRUE;
}

/**
 * Submit handler to remove a term modifier.
 */
function membership_entity_term_modifier_remove($form, &$form_state) {
  $button = $form_state['triggering_element'];
  if (isset($button['#delta'])) {
    unset($form_state['membership_term']['modifiers'][$button['#delta']]);
  }
  $form_state['rebuild'] = TRUE;
}

/**
 * Array map callback to return a modifier value.
 */
function _membership_entity_term_modifier_array_map($a) {
  return $a['value'];
}

/**
 * Menu callback: Renew an existing membership.
 */
function membership_entity_term_renew($form, &$form_state, $membership, $bundle = 'membership') {
  backdrop_set_title(t('Renew membership @member_id', array('@member_id' => $membership->member_id)));
  $form['#membership_entity'] = $membership;
  $form['#membership_entity']->is_renewal = TRUE;
  $form['#bundle_settings'] = $bundle_settings = membership_entity_get_bundle_settings($membership->type);
  if (empty($bundle_settings)) {
    return '';
  }

  // Add membership fields.
  form_load_include($form_state, 'inc', 'membership_entity', 'membership_entity.pages');
  membership_entity_form($form, $form_state);
  $form['#validate'][] = 'membership_entity_form_validate';

  // Hide membership fields where the 'membership_entity_term_renew_form'
  // setting is turned off.
  foreach (field_info_instances('membership_entity', $membership->type) as $field_name => $instance) {
    if (empty($instance['settings']['membership_entity_term_renew_form'])) {
      $form[$field_name]['#access'] = FALSE;
    }
  }

  // Membership status is now determined by terms.
  unset($form['membership']['status']);

  $form['#membership_entity_term'] = $term = membership_entity_term_create(array(
    'is_renewal' => TRUE,
    'mid' => $membership->mid,
    'membership_entity_type' => $membership->type,
  ));

  // Add membership term fields.
  membership_entity_term_form($form, $form_state);
  $form['membership_term']['start']['#title'] = t('Renewal start date');
  $form['membership_term']['term']['#title'] = t('Renewal term length');
  $form['#validate'][] = 'membership_entity_term_form_validate';

  // Hide term fields where the 'membership_entity_term_renew_form'
  // setting is turned off.
  foreach (field_info_instances('membership_entity_term', $term->membership_entity_type) as $field_name => $instance) {
    if (empty($instance['settings']['membership_entity_term_renew_form'])) {
      $form['membership_term'][$field_name]['#access'] = FALSE;
    }
  }

  $form['actions'] = array(
    '#type' => 'actions',
    '#weight' => 9999,
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Renew'),
      '#submit' => array(
        'membership_entity_edit_form_submit',
        'membership_entity_term_renew_submit',
      ),
    ),
  );

  // Check for a pending renewal.
  $latest_term = end($membership->terms);
  if ($latest_term && $latest_term->status == MEMBERSHIP_ENTITY_PENDING) {
    backdrop_set_message(t('Your membership term is currently pending. You will not be able to renew until your term is activated. If you feel that you have received this message in error, please contact a site administrator.'), 'warning');
    $form['actions']['submit']['#value'] .= " " . t('(disabled)');
    $form['actions']['submit']['#attributes']['disabled'] = 'disabled';
    $form['actions']['submit']['#validate'] = array('membership_entity_term_form_disabled_validate');
  }

  return $form;
}

/**
 * Validation callback to prevent renewal if a term is already pending.
 */
function membership_entity_term_form_disabled_validate($form, &$form_state) {
  form_set_error('submit', t('Your membership term is currently pending. You will not be able to renew until your term is activated. If you feel that you have received this message in error, please contact a site administrator.'));
}

/**
 * Submit handler to renew a membership.
 */
function membership_entity_term_renew_submit($form, &$form_state) {
  $membership = &$form_state['membership_entity'];
  $term = $form['#membership_entity_term'];

  form_state_values_clean($form_state);
  _membership_entity_form_submit_build_entity('membership_entity_term', $term, $form['membership_term'], $form_state);
  $status = membership_entity_term_save($term);

  // Add the saved term object to $form_state for other modules.
  $form_state['membership_entity_term'] = $term;
  switch ($status) {
    case SAVED_NEW:
      backdrop_set_message(t('The membership has been renewed.'));
      $form_state['redirect'] = 'membership/' . $term->mid;
      break;
  }

  $membership->terms[$term->id] = $term;
  $form_state['membership_entity_term'] = $term;
}

/**
 * Submit handler to delete a membership term.
 */
function membership_entity_term_edit_form_delete($form, &$form_state) {
  $destination = array();
  if (isset($_GET['destination'])) {
    $destination = backdrop_get_destination();
    unset($_GET['destination']);
  }
  $term = $form['#membership_entity_term'];
  $form_state['redirect'] = array(
    "membership/$term->mid/term/$term->id/delete",
    array('query' => $destination),
  );
}

/**
 * Display a confirmation page before deleting a membership term.
 */
function membership_entity_term_delete_confirm($form, &$form_state, $membership, $term) {
  $form_state['membership'] = $membership;
  $form_state['term'] = $term;
  $form['#submit'][] = 'membership_entity_term_delete_confirm_submit';
  return confirm_form($form, t('Are you sure you want to delete this membership term?'), "membership/$term->mid/term/$term->id/edit", t('This action cannot be undone.'), t('Delete'), t('Cancel'));
}

/**
 * Submit the membership term delete confirm form.
 */
function membership_entity_term_delete_confirm_submit($form, &$form_state) {
  $membership = $form_state['membership'];
  $term = $form_state['term'];
  if ($form_state['values']['confirm']) {
    membership_entity_term_delete($term->id);
    backdrop_set_message(t('Deleted 1 membership term.'));
  }
  $form_state['redirect'] = 'membership/' . $membership->mid;
}

/**
 * Page Callback: Show a generic renew page.
 */
function membership_entity_term_renew_page($bundle = 'membership') {
  // Order by mid descending and take the first, so a member who holds more
  // than one membership of this type is offered the newest. The query was
  // unordered and took current(), i.e. the lowest mid — which handed a lapsed
  // member who had rejoined their dead membership to renew rather than the
  // live one.
  $efq = new EntityFieldQuery();
  $result = $efq->entityCondition('entity_type', 'membership_entity')
    ->entityCondition('bundle', $bundle)
    ->propertyCondition('uid', $GLOBALS['user']->uid)
    ->propertyOrderBy('mid', 'DESC')
    ->execute();
  $membership = !empty($result['membership_entity']) ? membership_entity_load(current(array_keys($result['membership_entity']))) : FALSE;
  if (!empty($membership)) {
    if (membership_entity_term_access('renew', $membership)) {
      return backdrop_get_form('membership_entity_term_renew', $membership, $bundle);
    }
    else {
      $output['access_denied'] = array(
        '#prefix' => '<p>',
        '#suffix' => '</p>',
        '#markup' => t('You do not have access to renew this membership. Please contact a site administrator for more information.'),
      );
    }
  }

  // Show a login message if no membership was found.
  if (empty($membership)) {
    $output['no_membership'] = array(
      '#prefix' => '<p>',
      '#suffix' => '</p>',
      '#markup' => user_is_anonymous() ? t('You must log in before you can renew your membership.') : t('There are no memberships associated with your account.'),
    );
  }

  $output['actions'] = array(
    '#theme_wrappers' => array('container'),
    '#attributes' => array('class' => array('container-inline')),
  );

  if (user_is_anonymous()) {
    $output['actions']['login'] = array(
      '#type' => 'link',
      '#title' => t('Log in'),
      '#href' => 'user/login',
      '#options' => array(
        'query' => backdrop_get_destination(),
      ),
      '#attributes' => array('class' => array('button')),
    );
  }

  if (membership_entity_access('join', $bundle)) {
    $output['actions']['join'] = array(
      '#type' => 'link',
      '#title' => t('Join'),
      '#href' => 'user/join',
      '#attributes' => array('class' => array('button')),
    );
  }

  return $output;
}
