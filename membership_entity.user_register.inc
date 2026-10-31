<?php

/**
 * @file
 * Contains overrides of core register form methods for use on the join form.
 *
 * In order to include registration options on the join form for anonymous
 * users we need to add form elements from the user module. Due to the
 * structure of form arrays we cannot reuse these methods directly.
 */

/**
 * Overrides user_account_form() for the join process.
 *
 * @see membership_entity_join_form()
 */
function membership_entity_user_account_form(&$form, &$form_state) {
  // Account information.
  $form['user']['account'] = array(
    '#type' => 'container',
    '#weight' => -10,
    // Add account elements to $form_state['values']['user'].
    '#parents' => array('user'),
  );

  $form['user']['account']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Username'),
    '#maxlength' => USERNAME_MAX_LENGTH,
    '#description' => t('Spaces are allowed; punctuation is not allowed except for periods, hyphens, apostrophes, and underscores.'),
    '#required' => TRUE,
    '#attributes' => array('class' => array('username')),
  );

  $form['user']['account']['mail'] = array(
    '#type' => 'textfield',
    '#title' => t('E-mail address'),
    '#maxlength' => EMAIL_MAX_LENGTH,
    '#description' => t('A valid e-mail address. All e-mails from the system will be sent to this address. The e-mail address is not made public and will only be used if you wish to receive a new password or wish to receive certain news or notifications by e-mail.'),
    '#required' => TRUE,
  );

  // Display password field only when user is allowed to assign a password
  // during registration.
  if (!config_get('system.core', 'user_email_verification')) {
    $form['user']['account']['pass'] = array(
      '#type' => 'password_confirm',
      '#size' => 25,
      '#description' => t('Provide a password for the new account in both fields.'),
      '#required' => TRUE,
    );
  }

  $form['user']['account']['status'] = array(
    '#type' => 'value',
    '#value' => config_get('system.core', 'user_register') == USER_REGISTER_VISITORS,
  );
}

/**
 * Overrides user_account_form_validate() for the join process.
 */
function membership_entity_user_account_form_validate($form, &$form_state) {
  $values = &$form_state['values']['user'];
  $account = $form['#user'];

  // The account is a not-yet-saved user entity, so its uid is NULL. Cast it,
  // as core's user_account_form_validate() does: 'uid <> NULL' is never true in
  // SQL, which would make both uniqueness checks below silently pass.
  $uid = (int) $account->uid;

  // Validate username.
  if (isset($values['name'])) {
    if ($error = user_validate_name($values['name'])) {
      form_set_error('user][name', $error);
    }
    elseif ((bool) db_select('users')->fields('users', array('uid'))->condition('uid', $uid, '<>')->condition('name', db_like($values['name']), 'LIKE')->range(0, 1)->execute()->fetchField()) {
      form_set_error('user][name', t('The name %name is already taken.', array('%name' => $values['name'])));
    }
  }

  // Trim whitespace from mail, to prevent confusing 'e-mail not valid'
  // warnings often caused by cutting and pasting.
  $mail = trim($values['mail']);
  form_set_value($form['user']['account']['mail'], $mail, $form_state);

  // Validate the e-mail address. Backdrop has no user_validate_mail(); that was
  // a Drupal 7 function, and valid_email_address() is the replacement.
  if (empty($mail)) {
    form_set_error('user][mail', t('You must enter an e-mail address.'));
  }
  elseif (!valid_email_address($mail)) {
    form_set_error('user][mail', t('The e-mail address %mail is not valid.', array('%mail' => $mail)));
  }
  elseif ((bool) db_select('users')->fields('users', array('uid'))->condition('uid', $uid, '<>')->condition('mail', db_like($mail), 'LIKE')->range(0, 1)->execute()->fetchField()) {
    form_set_error('user][mail', t('The e-mail address %email is already registered. <a href="@password">Have you forgotten your password?</a>', array('%email' => $mail, '@password' => url('user/password'))));
  }
}
