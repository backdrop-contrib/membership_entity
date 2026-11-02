<?php

/**
 * @file
 * Tokens for renewal reminder e-mails.
 */

/**
 * Implements hook_token_info().
 */
function membership_entity_reminder_token_info() {
  $types['membership-reminder'] = array(
    'name' => t('Renewal reminder'),
    'description' => t('The membership a renewal reminder is about.'),
    'needs-data' => 'membership-reminder',
  );

  $tokens['member-name'] = array(
    'name' => t('Member name'),
    'description' => t('The name of the member being reminded.'),
  );
  $tokens['member-id'] = array(
    'name' => t('Member number'),
    'description' => t('The membership number.'),
  );
  $tokens['expiry-date'] = array(
    'name' => t('Expiry date'),
    'description' => t('The date the membership expires or expired, e.g. 2 November 2026. Add a format for another style, e.g. [membership-reminder:expiry-date:custom:d/m/Y].'),
    'type' => 'date',
  );
  $tokens['days'] = array(
    'name' => t('Days'),
    'description' => t('How many days until, or since, the expiry date.'),
  );
  $tokens['renew-url'] = array(
    'name' => t('Renew URL'),
    'description' => t('The renew page: the one set at Memberships > Settings, or the built-in form.'),
  );
  $tokens['membership-url'] = array(
    'name' => t('Membership URL'),
    'description' => t("The membership's own page."),
  );

  return array(
    'types' => $types,
    'tokens' => array('membership-reminder' => $tokens),
  );
}

/**
 * Implements hook_tokens().
 */
function membership_entity_reminder_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $replacements = array();
  if ($type != 'membership-reminder' || empty($data['membership-reminder']['membership'])) {
    return $replacements;
  }

  $reminder = $data['membership-reminder'];
  $membership = $reminder['membership'];
  $sanitize = !empty($options['sanitize']);
  $url_options = array('absolute' => TRUE);
  if (isset($options['language'])) {
    $url_options['language'] = $options['language'];
  }

  foreach ($tokens as $name => $original) {
    switch ($name) {
      case 'member-name':
        $value = membership_entity_member_name($membership);
        $replacements[$original] = $sanitize ? check_plain($value) : $value;
        break;

      case 'member-id':
        $replacements[$original] = $sanitize ? check_plain($membership->member_id) : $membership->member_id;
        break;

      case 'expiry-date':
        $replacements[$original] = membership_entity_reminder_format_date($reminder['expiry']);
        break;

      // Counted from today, not the reminder's offset: a reminder that went
      // out late must not overstate the time left.
      case 'days':
        $utc = new DateTimeZone('UTC');
        $today = new DateTime(membership_entity_reminder_today(), $utc);
        $replacements[$original] = $today->diff(new DateTime($reminder['expiry'], $utc))->days;
        break;

      case 'renew-url':
        $path = config_get('membership_entity.settings', 'renew_path');
        $replacements[$original] = url($path ? $path : 'user/renew', $url_options);
        break;

      case 'membership-url':
        $replacements[$original] = url('membership/' . $membership->mid, $url_options);
        break;
    }
  }

  if ($date_tokens = token_find_with_prefix($tokens, 'expiry-date')) {
    $replacements += token_generate('date', $date_tokens, array('date' => membership_entity_reminder_timestamp($reminder['expiry'])), $options);
  }

  return $replacements;
}
