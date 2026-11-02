<?php

/**
 * @file
 * Settings page for renewal reminders.
 */

/**
 * Form builder: renewal reminder settings.
 */
function membership_entity_reminder_settings_form($form, &$form_state) {
  $config = config('membership_entity_reminder.settings');
  $today = membership_entity_reminder_today();

  $form['enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Send renewal reminders'),
    '#default_value' => $config->get('enabled'),
    '#description' => t('Reminders go out on cron. Set the wording below and check the list of reminders due today before turning this on.'),
  );

  $form['start_date'] = array(
    '#type' => 'textfield',
    '#title' => t('Send reminders falling due on or after'),
    '#default_value' => $config->get('start_date') ? $config->get('start_date') : $today,
    '#size' => 12,
    '#description' => t('YYYY-MM-DD. A reminder whose date came before this is never sent, so turning reminders on does not send everyone already inside a reminder period at once. Move it back to catch up on reminders a previous system would have sent.'),
  );

  $form['schedule'] = array(
    '#type' => 'fieldset',
    '#title' => t('When'),
  );
  $form['schedule']['days_before'] = array(
    '#type' => 'textfield',
    '#title' => t('Days before expiry'),
    '#default_value' => implode(', ', (array) $config->get('days_before')),
    '#description' => t('Separate with commas, e.g. %example. Leave empty for none.', array('%example' => '30, 7, 1')),
  );
  $form['schedule']['days_after'] = array(
    '#type' => 'textfield',
    '#title' => t('Days after expiry'),
    '#default_value' => implode(', ', (array) $config->get('days_after')),
    '#description' => t('For members who have not renewed, e.g. %example. Leave empty for none.', array('%example' => '14')),
  );
  $form['schedule']['send_hour'] = array(
    '#type' => 'select',
    '#title' => t('Not before'),
    '#options' => backdrop_map_assoc(range(0, 23), 'membership_entity_reminder_hour_label'),
    '#default_value' => (int) $config->get('send_hour'),
    '#description' => t('Site time. Reminders go on the first cron run after this hour.'),
  );

  $bundles = membership_entity_get_bundles();
  if (count($bundles) > 1) {
    $options = array();
    foreach ($bundles as $name => $bundle) {
      $options[$name] = check_plain($bundle->label);
    }
    $form['schedule']['types'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Membership types'),
      '#options' => $options,
      '#default_value' => (array) $config->get('types'),
      '#description' => t('None ticked means all.'),
    );
  }

  $form['from'] = array(
    '#type' => 'textfield',
    '#title' => t('Replies go to'),
    '#default_value' => $config->get('from'),
    '#description' => t('For example the membership secretary. Leave empty for the site e-mail address. Reminders are always sent from the site address, which mail servers trust.'),
  );

  $token_link = theme('token_tree_link', array(
    'token_types' => array('membership-reminder', 'user'),
  ));
  foreach (array('before' => t('Before expiry'), 'after' => t('After expiry')) as $key => $title) {
    $form[$key] = array(
      '#type' => 'fieldset',
      '#title' => t('Message: @title', array('@title' => $title)),
      '#description' => $token_link,
    );
    $form[$key][$key . '_subject'] = array(
      '#type' => 'textfield',
      '#title' => t('Subject'),
      '#default_value' => $config->get($key . '_subject'),
      '#maxlength' => 255,
    );
    $form[$key][$key . '_body'] = array(
      '#type' => 'textarea',
      '#title' => t('Body'),
      '#default_value' => $config->get($key . '_body'),
      '#rows' => 15,
    );
  }

  $form['summary_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Daily summary'),
  );
  $form['summary_settings']['summary'] = array(
    '#type' => 'checkbox',
    '#title' => t('E-mail a summary of the reminders sent each day'),
    '#default_value' => $config->get('summary'),
    '#description' => t('Sent only on days when reminders went out. It lists members with no e-mail address separately, to be reminded by post.'),
  );
  $form['summary_settings']['summary_to'] = array(
    '#type' => 'textfield',
    '#title' => t('Send the summary to'),
    '#default_value' => $config->get('summary_to'),
    '#description' => t('One or more addresses, separated by commas.'),
    '#states' => array(
      'visible' => array(':input[name="summary"]' => array('checked' => TRUE)),
    ),
  );

  $form['test'] = array(
    '#type' => 'fieldset',
    '#title' => t('Test the messages'),
    '#description' => t('Sends both messages, as they would be for this member, to the address below. Nothing is sent to the member and nothing is recorded. Save any changes first.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['test']['test_member_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Member number'),
    '#size' => 12,
  );
  $form['test']['test_mail'] = array(
    '#type' => 'textfield',
    '#title' => t('Send test to'),
    '#default_value' => $GLOBALS['user']->mail,
    '#description' => t('Not saved.'),
  );
  $form['test']['send_test'] = array(
    '#type' => 'submit',
    '#value' => t('Send test e-mails'),
    '#submit' => array('membership_entity_reminder_test_submit'),
    '#validate' => array('membership_entity_reminder_test_validate'),
    '#limit_validation_errors' => array(array('test_member_id'), array('test_mail')),
  );

  $form['due'] = array(
    '#type' => 'fieldset',
    '#title' => t('Due today'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['due']['list'] = membership_entity_reminder_due_table($today);

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  return $form;
}

/**
 * Labels an hour for the "Not before" select.
 *
 * @param int $hour
 *   0 to 23.
 *
 * @return string
 *   For example "08:00".
 */
function membership_entity_reminder_hour_label($hour) {
  return sprintf('%02d:00', $hour);
}

/**
 * The reminders that would go out today with the saved settings.
 *
 * @param string $today
 *   Today, Y-m-d.
 *
 * @return array
 *   A render array.
 */
function membership_entity_reminder_due_table($today) {
  $rows = array();
  foreach (membership_entity_reminder_due($today) as $mid => $item) {
    $rows[] = array(
      l($item['membership']->member_id, 'membership/' . $mid),
      check_plain(membership_entity_member_name($item['membership'])),
      membership_entity_reminder_format_date($item['expiry']),
      membership_entity_reminder_offset_label($item['days']),
    );
  }

  return array(
    '#theme' => 'table',
    '#header' => array(t('Member'), t('Name'), t('Expires'), t('Reminder')),
    '#rows' => $rows,
    '#empty' => t('No reminders are due today.'),
    '#prefix' => '<p>' . t('With the saved settings, whether or not reminders are turned on. Already-sent reminders are not listed.') . '</p>',
  );
}

/**
 * Validate handler for the settings form.
 */
function membership_entity_reminder_settings_form_validate($form, &$form_state) {
  $values = $form_state['values'];

  $date = DateTime::createFromFormat('!Y-m-d', $values['start_date']);
  if (!$date || $date->format('Y-m-d') !== $values['start_date']) {
    form_set_error('start_date', t('Enter the start date as YYYY-MM-DD.'));
  }

  foreach (array('days_before', 'days_after') as $key) {
    $days = membership_entity_reminder_parse_days($values[$key]);
    if ($days === FALSE) {
      form_set_error($key, t('Enter whole numbers of days, separated by commas.'));
    }
    else {
      form_set_value($form['schedule'][$key], $days, $form_state);
    }
  }

  if ($values['from'] !== '' && !valid_email_address(trim($values['from']))) {
    form_set_error('from', t('%mail is not a valid e-mail address.', array('%mail' => $values['from'])));
  }

  if ($values['summary'] && trim($values['summary_to']) === '') {
    form_set_error('summary_to', t('Enter where to send the summary.'));
  }
  foreach (array_filter(array_map('trim', explode(',', $values['summary_to']))) as $mail) {
    if (!valid_email_address($mail)) {
      form_set_error('summary_to', t('%mail is not a valid e-mail address.', array('%mail' => $mail)));
    }
  }
}

/**
 * Turns "30, 7, 1" into a list of days.
 *
 * @param string $value
 *   Comma-separated whole numbers.
 *
 * @return int[]|false
 *   Distinct days, largest first, or FALSE if anything is not a whole number.
 */
function membership_entity_reminder_parse_days($value) {
  $days = array();
  foreach (array_filter(array_map('trim', explode(',', $value)), 'strlen') as $part) {
    if (!ctype_digit($part) || (int) $part < 1) {
      return FALSE;
    }
    $days[] = (int) $part;
  }

  $days = array_unique($days);
  rsort($days);
  return $days;
}

/**
 * Submit handler for the settings form.
 */
function membership_entity_reminder_settings_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  $config = config('membership_entity_reminder.settings');

  foreach (array('enabled', 'summary') as $key) {
    $config->set($key, (bool) $values[$key]);
  }
  foreach (array('start_date', 'from', 'summary_to', 'before_subject', 'before_body', 'after_subject', 'after_body') as $key) {
    $config->set($key, trim($values[$key]));
  }
  $config->set('days_before', $values['days_before']);
  $config->set('days_after', $values['days_after']);
  $config->set('send_hour', (int) $values['send_hour']);
  $config->set('types', isset($values['types']) ? array_values(array_filter($values['types'])) : array());
  $config->save();

  backdrop_set_message(t('The configuration options have been saved.'));
}

/**
 * Validate handler for the test button.
 */
function membership_entity_reminder_test_validate($form, &$form_state) {
  $member_id = trim($form_state['values']['test_member_id']);
  $membership = $member_id === '' ? FALSE : membership_entity_load_by_member_id($member_id);
  if (!$membership) {
    form_set_error('test_member_id', t('No membership has that number.'));
  }
  else {
    $form_state['membership_entity_reminder_test'] = $membership;
  }

  if (!valid_email_address(trim($form_state['values']['test_mail']))) {
    form_set_error('test_mail', t('Enter the address to send the test to.'));
  }
}

/**
 * Submit handler for the test button: both messages, to the address given.
 */
function membership_entity_reminder_test_submit($form, &$form_state) {
  global $user;
  $to = trim($form_state['values']['test_mail']);
  $membership = $form_state['membership_entity_reminder_test'];
  $account = !empty($membership->uid) ? user_load($membership->uid) : FALSE;

  $expiry = membership_entity_term_get_expiration_date($membership);
  $expiry = $expiry ? $expiry->format('Y-m-d') : membership_entity_reminder_today();
  $reply = config_get('membership_entity_reminder.settings', 'from');

  foreach (array('before' => -7, 'after' => 14) as $key => $days) {
    backdrop_mail('membership_entity_reminder', $key, $to, user_preferred_language($user), array(
      'membership' => $membership,
      'account' => $account ? $account : $user,
      'expiry' => $expiry,
      'days' => $days,
    ), $reply ? $reply : NULL);
  }

  backdrop_set_message(t('Test reminders for %member_id sent to %mail.', array(
    '%member_id' => $membership->member_id,
    '%mail' => $to,
  )));
}
