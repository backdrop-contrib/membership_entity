<?php

/**
 * @file
 * Page callbacks for membership entities.
 */

/**
 * Form builder: membership settings form.
 */
function membership_entity_settings_form($form, &$form_state, $data = array()) {
  $form['#attached'] = array(
    'css' => array(backdrop_get_path('module', 'membership_entity') . '/css/membership_entity_admin.css'),
  );

  if (empty($data)) {
    $data = membership_entity_get_bundle_settings();
  }
  $form['#data'] = $data;

  $form['membership_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Membership options'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $member_id_plugins = membership_entity_get_member_id();
  $options = array();
  foreach ($member_id_plugins as $id => $plugin) {
    $options[$id] = '<div class="membership-entity-radio-label">' . check_plain($plugin['title']) . '<br /><small>' . check_plain($plugin['description']) . '</small></div>';
  }
  $wrapper_id = 'membership-entity-settings-wrapper';

  $default_format = !empty($form_state['membership_entity']['member_id_format']) ? $form_state['membership_entity']['member_id_format'] : $data['member_id_format'];
  $form['membership_settings']['member_id_format'] = array(
    '#type' => 'radios',
    '#title' => t('Site id number format'),
    '#options' => $options,
    '#default_value' => $default_format,
    '#required' => TRUE,
    '#element_validate' => array('membership_entity_member_id_format_validate'),
    '#limit_validation_errors' => array(),
    '#ajax' => array(
      'callback' => 'membership_entity_settings_ajax',
      'wrapper' => $wrapper_id,
      'effect' => 'fade',
    ),
  );

  $form['membership_settings']['member_id_settings'] = array(
    '#type' => 'container',
    '#tree' => TRUE,
    '#element_validate' => array('membership_entity_member_id_settings_validate'),
    '#attributes' => array(
      'id' => $wrapper_id,
      'class' => array('membership-entity-member-id-settings'),
    ),
  );

  $handler = _membership_entity_member_id_get_instance($default_format, $data['member_id_settings']);
  $settings_form = $handler->settingsForm($form_state);
  if (!empty($settings_form)) {
    $form['membership_settings']['member_id_settings']['#type'] = 'fieldset';
    $form['membership_settings']['member_id_settings']['#title'] = t('Site id number settings');
    $form['membership_settings']['member_id_settings'] += $settings_form;
  }

  $form['membership_settings']['member_id_submit'] = array(
    '#type' => 'submit',
    '#name' => 'member_id_submit',
    '#value' => t('Change plugin'),
    '#limit_validation_errors' => array(),
    '#attributes' => array('class' => array('js-hide')),
    '#submit' => array('membership_entity_settings_ajax_submit'),
  );

  $form['user_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Member options'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $roles = user_roles(TRUE);
  $form['user_settings']['primary_role'] = array(
    '#title' => t('Primary member role'),
    '#type' => 'select',
    '#options' => $roles,
    '#default_value' => $data['primary_role'],
    '#required' => TRUE,
    '#description' => t('Select the role that will be assigned to the primary member when the membership is active.'),
  );

  $form['user_settings']['show_on_profile'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display membership information on the user profile.'),
    '#default_value' => $data['show_on_profile'],
  );

  $form['user_settings']['multiple_memberships'] = array(
    '#type' => 'checkbox',
    '#title' => t('One person may hold more than one of these at once'),
    '#default_value' => !empty($data['multiple_memberships']),
    '#description' => t('Leave this off for an ordinary subscription, where holding two at the same time makes no sense. Turn it on where somebody can genuinely belong to more than one at once — a member of two affiliated associations, or of two branches.'),
  );

  $form['rejoining'] = array(
    '#type' => 'fieldset',
    '#title' => t('Rejoining'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#description' => t('What happens when somebody who has lapsed comes back.'),
  );

  $form['rejoining']['rejoin_policy'] = array(
    '#type' => 'radios',
    '#title' => t('When a lapsed member rejoins'),
    '#options' => array(
      'reinstate' => t('Give them their old membership back'),
      'new' => t('Give them a new membership'),
    ),
    '#default_value' => isset($data['rejoin_policy']) ? $data['rejoin_policy'] : 'reinstate',
    '#description' => t('Reinstating keeps the member number they had, along with their joining date and their whole term history — the gap simply shows in it, which is usually what an organisation wants to be able to see. Issuing a new membership starts them again with a new number, and leaves the old record in place as history.<br />A member number can only belong to one membership, so keeping the number and starting a new record are not both possible.'),
  );

  $form['retention'] = array(
    '#type' => 'fieldset',
    '#title' => t('Keeping records'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['retention']['retention_period'] = array(
    '#type' => 'select',
    '#title' => t('Keep a lapsed membership for'),
    '#options' => membership_entity_retention_periods(),
    '#default_value' => isset($data['retention_period']) ? $data['retention_period'] : '1 year',
    '#description' => t('How long a membership of this type is kept after it lapses. A year suits an organisation that chases a lapsed member once or twice and then lets them go. Choose longer where a membership record is also an insurance or audit record — a block insurance scheme, for instance, usually has to be evidenced for years afterwards.<br />Nothing is ever deleted automatically. This decides when an administrator is <em>allowed</em> to delete, and stops a record being lost as a side effect of deleting the member\'s user account.'),
  );


  // Secondary members are on the way out. Sub-members do everything they did
  // and give the person a record of their own, so the settings are gathered
  // here, out of the way, rather than sitting among the ones still worth
  // reaching for. Nothing is removed: a site already using them must be able to
  // see and change what it has.
  $in_use = ((int) $data['cardinality']) !== 1;

  $form['secondary_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Secondary members (deprecated)'),
    '#collapsible' => TRUE,
    '#collapsed' => !$in_use,
    '#description' => t('A secondary member shares one membership with the primary member. They hold a user account and a role, but no member id, fields, terms, renewal or page of their own, and nothing can be recorded against them individually. <strong>Prefer sub-members</strong>, under "Sub-members" below: a sub-member has their own membership record and everything that comes with it, and can still be paid for by the membership above them.'),
  );

  if ($in_use) {
    $form['secondary_settings']['deprecated'] = array(
      '#type' => 'markup',
      '#markup' => '<div class="messages warning">' . t('This membership type still allows secondary members. To move away from them, give each secondary member their own membership with its "Belongs to" set to this one, then set the number below to 1. Existing secondary members are never removed automatically, and they keep their role until they are cleared from the membership form.') . '</div>',
      '#weight' => -1,
    );
  }

  $form['secondary_settings']['cardinality'] = array(
    '#type' => 'select',
    '#title' => t('Number of users sharing one membership'),
    '#options' => array(
      FIELD_CARDINALITY_UNLIMITED => t('Unlimited'),
      // Spelled out because this doubles as the switch that turns secondary
      // members off, and "1" alone does not read that way.
      '1' => t('1 — primary member only, no secondary members'),
      '2' => t('2'),
      '3' => t('3'),
      '4' => t('4'),
      '5' => t('5'),
      '6' => t('6'),
      '7' => t('7'),
      '8' => t('8'),
      '9' => t('9'),
      '10' => t('10'),
    ),
    '#default_value' => $data['cardinality'],
    '#description' => t('How many user accounts may share a single membership of this type, counting the primary member. 1 — the default — removes secondary members from the membership form altogether.'),
  );

  $form['secondary_settings']['secondary_role'] = array(
    '#title' => t('Secondary member role'),
    '#type' => 'select',
    '#options' => array('primary' => 'Same as primary member') + $roles,
    '#default_value' => ($data['secondary_role'] == $data['primary_role']) ? 'primary' : $data['secondary_role'],
    '#description' => t('Select the role that will be assigned to secondary members when the membership is active.'),
    '#states' => array(
      'invisible' => array(
        ':input[name="cardinality"]' => array('value' => '1'),
      ),
    ),
  );

  $form['secondary_settings']['all_edit'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow secondary members to edit the membership.'),
    '#default_value' => $data['all_edit'],
    '#description' => t('If not checked, only the primary member will be able to edit membership fields.'),
    '#states' => array(
      'invisible' => array(
        ':input[name="cardinality"]' => array('value' => '1'),
      ),
    ),
  );

  $form['hierarchy_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Sub-members'),
    '#collapsible' => TRUE,
    '#collapsed' => empty($data['max_children']),
    '#description' => t('A membership of this type may hold other memberships beneath it — the members of a household, or of an affiliated association. Each sub-member keeps their own membership record, member id and page, and loses access whenever this membership is not active.'),
  );

  $form['hierarchy_settings']['max_children'] = array(
    '#type' => 'select',
    '#title' => t('Maximum sub-members'),
    '#options' => array(
      '0' => t('None — this membership type cannot have sub-members'),
      FIELD_CARDINALITY_UNLIMITED => t('Unlimited'),
    ) + backdrop_map_assoc(range(1, 20)),
    '#default_value' => $data['max_children'],
    '#description' => t('Set to none for an ordinary single membership. A household might allow 5; an association, unlimited. This is separate from the number of users per membership above: users share this one membership, whereas sub-members each hold a membership of their own.'),
  );

  $form['hierarchy_settings']['child_terms'] = array(
    '#type' => 'radios',
    '#title' => t('Who pays'),
    '#options' => array(
      'inherit' => t('One payment covers everyone — renewing this membership renews every sub-member with it.'),
      'roster' => t('One payment covers everyone, but the membership supplies a list — sub-members are renewed individually from it.'),
      'own' => t('Each sub-member renews separately — and still loses access if this membership lapses.'),
    ),
    '#default_value' => $data['child_terms'],
    '#states' => array(
      'invisible' => array(
        ':input[name="max_children"]' => array('value' => '0'),
      ),
    ),
  );

  $form['#validate'][] = 'membership_entity_settings_form_validate';
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
    '#submit' => array('membership_entity_settings_form_submit'),
  );

  // Allow other modules to add settings to the form.
  backdrop_alter('membership_entity_settings', $form, $form_state, $data);
  return $form;
}

/**
 * Settings that apply across every membership type.
 *
 * @see membership_entity_menu()
 */
function membership_entity_global_settings_form($form, &$form_state) {
  $config = config('membership_entity.settings');

  $form['postal'] = array(
    '#type' => 'fieldset',
    '#title' => t('Postal members'),
    '#description' => t('Backdrop requires a unique e-mail address on every user account, but real clubs have members who deal with them entirely by post and have no address to give. With this enabled, such a member can be given an account carrying a generated address that can never be delivered to, so that they can still hold a membership, appear in listings and be counted.'),
  );

  $form['postal']['postal_members'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow members with no e-mail address'),
    '#default_value' => $config->get('postal_members'),
  );

  $form['postal']['postal_mail_domain'] = array(
    '#type' => 'textfield',
    '#title' => t('Domain for generated addresses'),
    '#default_value' => $config->get('postal_mail_domain'),
    '#placeholder' => membership_entity_postal_mail_domain(),
    '#description' => t('Leave empty to use %default. Use a domain that can never receive mail — the reserved .invalid suffix guarantees it, so nothing is delivered and nothing bounces. A domain you actually own would collect these messages or reject them noisily.', array(
      '%default' => membership_entity_postal_mail_domain(),
    )),
    '#states' => array(
      'visible' => array(
        ':input[name="postal_members"]' => array('checked' => TRUE),
      ),
    ),
  );

  $form['pages'] = array(
    '#type' => 'fieldset',
    '#title' => t('Join and renew pages'),
    '#description' => t('The built-in join and renew forms create memberships and terms without taking payment. To send members somewhere that does, such as a shop product, enter its path here. The Join and Renew links, and every page under them, then redirect there.'),
  );

  $form['pages']['join_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Join page'),
    '#default_value' => $config->get('join_path'),
    '#description' => t('Replaces %path. A site path such as %example, or a full URL. Leave empty to use the built-in form.', array(
      '%path' => 'user/join',
      '%example' => 'membership-application',
    )),
  );

  $form['pages']['renew_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Renew page'),
    '#default_value' => $config->get('renew_path'),
    '#description' => t('Replaces %path and each membership\'s Renew link. Users with %permission still reach the renew form from the membership itself, to record renewals paid offline. Leave empty to use the built-in form.', array(
      '%path' => 'user/renew',
      '%permission' => t('Administer membership terms'),
    )),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  return $form;
}

/**
 * Validate handler for the global settings form.
 *
 * @see membership_entity_global_settings_form()
 */
function membership_entity_global_settings_form_validate($form, &$form_state) {
  $domain = trim($form_state['values']['postal_mail_domain']);

  // Catch a domain that cannot produce a valid address before it is saved and
  // every postal account creation starts failing.
  if ($domain !== '' && !valid_email_address('noreply+test@' . $domain)) {
    form_set_error('postal_mail_domain', t('%domain does not produce a valid e-mail address.', array(
      '%domain' => $domain,
    )));
  }

  foreach (array('join_path' => 'user/join', 'renew_path' => 'user/renew') as $key => $own_path) {
    $path = trim($form_state['values'][$key]);
    if ($path === '' || url_is_external($path)) {
      continue;
    }
    $normal = backdrop_get_normal_path($path);
    if (!backdrop_valid_path($normal)) {
      form_set_error($key, t('%path is not a page on this site.', array('%path' => $path)));
    }
    // Pointing a page at itself, or below itself, would redirect for ever.
    elseif (preg_match('#^(user/join|user/renew)(/|$)#', $normal)) {
      form_set_error($key, t('%path would redirect to itself.', array('%path' => $path)));
    }
  }
}

/**
 * Submit handler for the global settings form.
 *
 * @see membership_entity_global_settings_form()
 */
function membership_entity_global_settings_form_submit($form, &$form_state) {
  config('membership_entity.settings')
    ->set('postal_members', (bool) $form_state['values']['postal_members'])
    ->set('postal_mail_domain', trim($form_state['values']['postal_mail_domain']))
    ->set('join_path', trim($form_state['values']['join_path']))
    ->set('renew_path', trim($form_state['values']['renew_path']))
    ->save();

  backdrop_set_message(t('The configuration options have been saved.'));
}

/**
 * Element validate callback for the member id format.
 */
function membership_entity_member_id_format_validate(&$element, &$form_state, $form) {
  $trigger = $form_state['triggering_element'];
  if ($trigger['#name'] == 'member_id_format') {
    $form_state['membership_entity']['member_id_format'] = $trigger['#value'];
  }
}

/**
 * Element validate callback or member id settings.
 */
function membership_entity_member_id_settings_validate(&$element, &$form_state, $form) {
  $values = $form_state['values'];
  if ($instance = _membership_entity_member_id_get_instance($values['member_id_format'], $values['member_id_settings'])) {
    $instance->validateSettings($element, $form_state);
  }
}

/**
 * Ajax callback for the settings form.
 */
function membership_entity_settings_ajax($form, &$form_state) {
  return $form['membership_settings']['member_id_settings'];
}

/**
 * Ajax callback for the settings form.
 */
function membership_entity_settings_ajax_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
}

/**
 * Validate handler for the settings form.
 */
function membership_entity_settings_form_validate($form, &$form_state) {
  $values = &$form_state['values'];
  if (isset($values['secondary_role']) && $values['secondary_role'] == 'primary') {
    form_set_value($form['secondary_settings']['secondary_role'], $values['primary_role'], $form_state);
  }

  // Make sure member_id_settings are set even if the chosen plugin does not
  // define a settings form.
  if (empty($values['member_id_settings'])) {
    $values['member_id_settings'] = array();
  }
}

/**
 * Form submission handler for membership_entity_settings_form().
 */
function membership_entity_settings_form_submit($form, &$form_state) {
  // Remove extra fields and save the values.
  form_state_values_clean($form_state);
  config_set('membership_entity.settings', 'membership_entity_settings', $form_state['values']);
  backdrop_set_message(t('The configuration options have been saved'));
}

/**
 * Menu callback: Edit a membership.
 */
function membership_entity_edit($membership) {
  backdrop_set_title(t('Edit membership: @id', array('@id' => $membership->member_id)));
  return backdrop_get_form('membership_entity_edit_form', $membership);
}

/**
 * Form builder to create/edit membership entities.
 *
 * @see membership_entity_form_validate()
 * @see membership_entity_form_submit()
 */
function membership_entity_edit_form($form, &$form_state, $membership = NULL, $type = 'membership') {
  if (empty($membership)) {
    $membership = membership_entity_create(array('type' => $type));
  }

  $form['#attributes']['class'][] = 'membership-entity-edit-form';
  if (!empty($membership->type)) {
    $form['#attributes']['class'][] = 'membership-entity-' . $membership->type . '-edit-form';
  }

  // Add the membership entity form fields.
  $form['#membership_entity'] = $membership;
  $form['#bundle_settings'] = membership_entity_get_bundle_settings($membership->type);
  if (empty($form['#bundle_settings']['status'])) {
    backdrop_set_message(t('Missing configuration settings for membership type %type. Please visit the <a href="!url">settings page</a> to configure the membership type.', array(
      '%type' => $membership->type,
      '!url' => url("admin/memberships/types/manage/$membership->type"),
    )), 'error');

    return '';
  }

  membership_entity_form($form, $form_state);
  $form['#validate'][] = 'membership_entity_form_validate';

  // Add the buttons.
  $form['actions'] = array(
    '#type' => 'actions',
    '#weight' => 9999,
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Save'),
      '#submit' => array('membership_entity_edit_form_submit'),
    ),
  );

  // The account has to exist before the membership that hangs on it is saved.
  if (isset($form['membership']['new_account'])) {
    array_unshift($form['actions']['submit']['#submit'], 'membership_entity_create_member_account_submit');
  }

  if (!empty($membership->mid) && membership_entity_access('delete', $membership)) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#limit_validation_errors' => array(),
      '#submit' => array('membership_entity_edit_form_delete'),
    );
  }

  // Add type specific submit handlers.
  if (function_exists($membership->type . '_membership_entity_edit_form_submit')) {
    $form['#submit'][] = $membership->type . '_membership_entity_edit_form_submit';
  }

  return $form;
}

/**
 * Page callback: add a sub-member to a membership.
 *
 * Opens the ordinary membership add form with "Belongs to" already set to the
 * membership the action was followed from, so that adding a member to a
 * household or association starts from that household or association rather
 * than from a list of every membership on the site. Where more than one
 * membership type exists the type is chosen first, exactly as the generic add
 * page does.
 *
 * @param MembershipEntity $parent
 *   The membership the new sub-member will belong to.
 * @param string $type_url_str
 *   The membership type to create, with underscores replaced by dashes.
 *   Omitted on the first step when there is more than one type.
 *
 * @return array|string
 *   A render array, or the built add form.
 */
function membership_entity_add_sub_member_page(MembershipEntity $parent, $type_url_str = NULL) {
  $bundles = membership_entity_get_bundles();
  if (empty($bundles)) {
    return array(
      '#markup' => t('There are no membership types, so a sub-member cannot be created.'),
    );
  }

  $type = NULL;
  if (isset($type_url_str)) {
    $candidate = str_replace('-', '_', $type_url_str);
    if (!isset($bundles[$candidate])) {
      return MENU_NOT_FOUND;
    }
    $type = $candidate;
  }
  elseif (count($bundles) == 1) {
    // Nothing to choose, so do not ask.
    $type = key($bundles);
  }

  if (isset($type)) {
    // t() has already sanitized the label into the string, so the title must
    // not be run through check_plain() a second time.
    backdrop_set_title(t('Add sub-member to @parent', array(
      '@parent' => membership_entity_parent_label($parent->mid),
    )), PASS_THROUGH);

    $membership = membership_entity_create(array(
      'type' => $type,
      'pid' => $parent->mid,
    ));

    return backdrop_get_form('membership_entity_edit_form', $membership, $type);
  }

  $items = array();
  foreach ($bundles as $bundle => $bundle_type) {
    $items[] = l($bundle_type->label, 'membership/' . $parent->mid . '/add-sub-member/' . str_replace('_', '-', $bundle));
  }

  return array(
    'help' => array(
      '#prefix' => '<p>',
      '#suffix' => '</p>',
      '#markup' => t('Which type of membership should the new sub-member of @parent hold?', array(
        '@parent' => membership_entity_parent_label($parent->mid),
      )),
    ),
    'types' => array(
      '#theme' => 'item_list',
      '#items' => $items,
    ),
  );
}

/**
 * Page callback: the join form, or a redirect for somebody coming back.
 *
 * A lapsed member rejoining is not filling in a new membership — they already
 * have one, with their number, their joining date and their whole history on
 * it. Reinstating them is renewing that record, so send them to the renew form
 * rather than building a second membership beside the first.
 *
 * @param string $bundle
 *   The membership type being joined.
 *
 * @return array
 *   The join form.
 */
function membership_entity_join_page($bundle = 'membership') {
  if (user_is_logged_in() && $reinstate = membership_entity_membership_to_reinstate($GLOBALS['user'], $bundle)) {
    if (membership_entity_term_access_renew($reinstate)) {
      backdrop_set_message(t('Welcome back. You already hold membership @member_id, so renewing it here keeps your membership number and your history rather than starting you again.', array(
        '@member_id' => $reinstate->member_id,
      )));
      backdrop_goto('membership/' . $reinstate->mid . '/renew');
    }
  }

  return backdrop_get_form('membership_entity_join_form', $bundle);
}

/**
 * Form builder for the join form.
 */
function membership_entity_join_form($form, &$form_state, $bundle = 'membership') {
  $form['#attributes']['class'][] = 'membership-entity-join-form';

  $form['#membership_entity'] = membership_entity_create(array(
    'uid' => $GLOBALS['user']->uid,
    'type' => $bundle,
  ));

  $form['#bundle_settings'] = membership_entity_get_bundle_settings($bundle);
  if (empty($form['#bundle_settings'])) {
    return '';
  }

  membership_entity_form($form, $form_state);
  $form['#validate'][] = 'membership_entity_form_validate';

  // Hide fields where the 'membership_entity_join_form' setting is turned off.
  foreach (field_info_instances('membership_entity', $bundle) as $field_name => $instance) {
    if (empty($instance['settings']['membership_entity_join_form'])) {
      $form[$field_name]['#access'] = FALSE;
    }
  }

  // Design decision: Do not allow secondary members during a new join.
  // Secondary members may be added after the primary member is verified via
  // email or payment.
  if (isset($form['membership']['secondary_members'])) {
    $form['membership']['secondary_members']['#access'] = FALSE;
  }

  $form['actions'] = array(
    '#type' => 'actions',
    '#weight' => 9999,
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Join'),
      '#submit' => array('membership_entity_edit_form_submit'),
    ),
  );

  // Add the registration form for anonymous users.
  if (!user_is_logged_in()) {
    // Build an empty user entity to register into, exactly as core's
    // user_register_form() does. backdrop_anonymous_user() returns an
    // AnonymousUser, which core defines as "not an entity ... it cannot be
    // saved to the database" and which has no save() method at all, so the
    // submit handler could never have created an account from it.
    $form['#user'] = entity_create('user', array());

    $form['user'] = array(
      '#type' => 'container',
      '#parents' => array('user'),
      '#tree' => TRUE,
      '#pre_render' => array('_membership_entity_extra_fields_pre_render'),
      '#weight' => -10,
      '#attributes' => array(
        'class' => array(
          'user-info-from-cookie',
          'membership-entity-user-info',
        ),
      ),
      '#attached' => array(
        'library' => array(
          array('system', 'jquery.cookie'),
        ),
      ),
    );

    $form['user']['login'] = array(
      '#prefix' => '<p>',
      '#suffix' => '</p>',
      '#markup' => t('Click <a href="@url">here to log in</a> if you already have an account.', array(
        '@url' => url('user/login', array('query' => backdrop_get_destination())),
      )),
      '#weight' => -9999,
    );

    // Start with the default user account fields.
    form_load_include($form_state, 'inc', 'membership_entity', 'membership_entity.user_register');
    membership_entity_user_account_form($form, $form_state);

    // Attach field widgets, and hide the ones where the 'user_register_form'
    // setting is not on. No language argument: entity_language() is a Drupal 7
    // Entity API function that Backdrop does not provide, and field_attach_form()
    // resolves the language itself when none is given.
    field_attach_form('user', $form['#user'], $form['user'], $form_state);
    foreach (field_info_instances('user', 'user') as $field_name => $instance) {
      if (empty($instance['settings']['user_register_form'])) {
        $form['user'][$field_name]['#access'] = FALSE;
      }
    }

    // The e-mail address is required, but a postal member has none to give and
    // one is generated for them during validation. Required is enforced while
    // validation recurses through the children, before any handler below can
    // run, so the flag has to be lifted on the built form instead.
    $form['#membership_entity_postal_parents'] = array('user');
    $form['#after_build'][] = 'membership_entity_postal_required_after_build';

    // User account validate handlers need to run first.
    array_unshift($form['#validate'], 'membership_entity_user_join_register_validate');
    array_unshift($form['#validate'], 'membership_entity_user_account_form_validate');

    // User account submit handler needs to run first.
    array_unshift($form['actions']['submit']['#submit'], 'membership_entity_user_join_register_submit');
  }

  return $form;
}

/**
 * Helper to add default and field api membership entity fields.
 */
function membership_entity_form(&$form, &$form_state) {
  $membership = $form['#membership_entity'];
  $bundle_settings = $form['#bundle_settings'];

  $form['#entity_type'] = 'membership_entity';
  $form['#bundle'] = $membership->type;
  $form['#membership_entity_form'] = TRUE;

  // Basic information. 'created' is not here: it is offered as a date below,
  // since the joining date is a fact about the membership rather than a
  // timestamp nobody should touch.
  foreach (array('mid', 'type', 'entity_type') as $key) {
    $form[$key] = array(
      '#type' => 'value',
      '#value' => isset($membership->{$key}) ? $membership->{$key} : NULL,
    );
  }
  $form['changed'] = array(
    '#type' => 'hidden',
    '#default_value' => isset($membership->changed) ? $membership->changed : NULL,
  );

  $form['membership'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('membership-entity-membership-info'),
    ),
  );

  if (!empty($membership->member_id)) {
    if (user_access('administer memberships')) {
      $schema = backdrop_get_schema('membership_entity');
      $form['membership']['member_id'] = array(
        '#type' => 'textfield',
        // Named for what it is. This is the site's own identifier, generated
        // from one sequence shared by every membership type, so it is unique
        // across the site and cannot express "association A's member 001" —
        // the organisation's own number belongs in a field with the "This
        // field is a membership number" setting, which is scoped per parent.
        '#title' => t('Site id number'),
        '#description' => t('An administrative number the site gives every membership, unique across all membership types. It is not a membership number: the organisation’s own number belongs in a field marked as a membership number, which is scoped per association and may repeat between them.'),
        '#default_value' => $membership->member_id,
        '#required' => TRUE,
        '#maxlength' => $schema['fields']['member_id']['length'],
      );
    }
    else {
      $form['membership']['member_id'] = array(
        '#type' => 'value',
        '#value' => $membership->member_id,
      );

      $form['membership']['member_id_display'] = array(
        '#type' => 'markup',
        '#markup' => t('Site id number: %id', array('%id' => $membership->member_id)),
      );
    }
  }

  // Somebody being added to a household or an association very often has no
  // account yet, and until now there was no way to give them one from here:
  // both member selects match existing accounts only. Offer to create it.
  $may_create_account = membership_entity_may_create_member_account($membership);
  $may_choose_account = user_access('administer memberships');

  if ($may_create_account && $may_choose_account) {
    $form['membership']['member_source'] = array(
      '#type' => 'radios',
      '#title' => t('Primary member'),
      '#options' => array(
        'existing' => t('An existing user account'),
        'new' => t('Someone new — create their account now'),
      ),
      // Arriving from a parent's "Add sub-member" action means adding somebody
      // to a household or an association, and most of those people have no
      // account yet — that is the case the whole feature exists for. Start on
      // the branch they are most likely to want. The generic add form, where an
      // administrator is usually recording a membership for somebody who has
      // already registered, still starts on the existing account.
      '#default_value' => !empty($membership->pid) ? 'new' : 'existing',
      '#weight' => -1,
    );
  }

  $form['membership']['uid'] = array(
    '#type' => 'membership_entity_member_select',
    '#title' => t('Primary member'),
    '#access' => $may_choose_account,
    // Where creating an account is on offer this field may legitimately be left
    // empty, so requiredness is judged in the validate handler instead. Marking
    // it #required would reject the form before the choice is even read: an
    // element's required check runs while validation descends through the form,
    // ahead of every form level handler.
    '#required' => !$may_create_account,
    '#default_value' => !empty($membership->uid) ? array($membership->uid) : array(),
  );

  if ($may_create_account) {
    if ($may_choose_account) {
      // The member select is a composite element that renders no wrapper of its
      // own, and backdrop_process_states() registers a state against the
      // element's HTML id — so without a tag carrying that id there is nothing
      // for the JavaScript to show or hide, and the field stays on screen
      // whichever branch is chosen. A container gives it one.
      //
      // The class is not decoration. states.js hides an element by walking UP
      // from it to the nearest '.form-item, .form-submit, .form-wrapper'
      // (core/misc/states.js ~530), and theme_container() only adds
      // 'form-wrapper' automatically to elements whose #type is literally
      // 'container' (core/includes/form.inc ~3704). A bare wrapper round this
      // element therefore matches nothing, and the walk carries on up to the
      // whole membership container — so hiding this one field hid every field
      // on the form.
      $form['membership']['uid']['#theme_wrappers'] = array('container');
      $form['membership']['uid']['#attributes'] = array(
        'class' => array('form-wrapper', 'membership-entity-existing-account'),
      );
      $form['membership']['uid']['#states'] = array(
        'visible' => array(
          ':input[name="member_source"]' => array('value' => 'existing'),
        ),
      );

      // The radios above are the label for this choice, and the field's own
      // "Select an existing account" labels the input. A third heading in
      // between just repeats them.
      $form['membership']['uid']['#title'] = '';
    }

    _membership_entity_new_account_elements($form, $may_choose_account);
  }

  $cardinality = (int) $bundle_settings['cardinality'];
  $allows_secondary = ($cardinality > 1 || $cardinality == FIELD_CARDINALITY_UNLIMITED);

  // A membership can still be holding secondary members after its type has been
  // set back to one user — the rows survive, and the controller keeps granting
  // them the secondary role from them. Keep the field on screen in that case,
  // sized so they can only be removed, or they would be invisible and
  // unremovable while still carrying access.
  // Only ever on an edit form. Secondary members are deprecated, so there is no
  // reason to offer them on a membership being created, and a membership being
  // created has none to show — leaving nothing but the notice telling the
  // person not to use the thing they are being offered.
  $existing_secondary = count($membership->secondary_members);
  if (!empty($membership->mid) && ($allows_secondary || $existing_secondary)) {
    $form['membership']['secondary_members'] = array(
      '#type' => 'membership_entity_member_select',
      '#title' => format_plural($cardinality, t('Secondary member'), t('Secondary members')),
      // Administrators only, for the same reason the primary member select is.
      // The field matches against every account on the site, so offering it to
      // a household head or an association secretary would let them search the
      // whole membership — including other associations' members — and attach
      // any of them to a membership, granting site access with it. Neither is
      // theirs to do. Their own way of adding somebody is the sub-member form,
      // which creates that person's account rather than finding one.
      '#access' => user_access('administer memberships'),
      '#cardinality' => $allows_secondary
        ? (($cardinality == FIELD_CARDINALITY_UNLIMITED) ? $cardinality : $cardinality - 1)
        : $existing_secondary,
      '#default_value' => $membership->secondary_members,
      // Spelled out because the site has two ways to attach people and they are
      // easily confused. Someone added here shares this one membership and has
      // no record of their own; someone who needs their own membership id,
      // fields or renewal is a sub-member, added by creating a membership for
      // them and setting its "Belongs to" to this one.
      '#description' => $allows_secondary
        ? t('<strong>Deprecated — prefer sub-members.</strong> People added here share <em>this</em> membership. They do hold a user account and a role, but no membership record, member id, fields, terms or renewal of their own, so nothing can be recorded against them individually. To give someone their own record, add them as a sub-member instead: their account can be created at the same time, even if they have no e-mail address.')
        : t('This membership type no longer allows secondary members, but this membership still has @count. They continue to hold the secondary member role until removed here. Clear them, and give anyone who should keep access their own membership with its "Belongs to" set to this one.', array(
          '@count' => $existing_secondary,
        )),
    );

    if (!$allows_secondary) {
      $form['membership']['secondary_members']['#prefix'] = '<div class="messages warning">'
        . t('Secondary members are turned off for this membership type. The @count listed below are left over and should be removed.', array('@count' => $existing_secondary))
        . '</div>';
    }
  }

  // A membership that has sub-members of its own cannot also sit beneath
  // another — memberships are limited to two levels.
  $has_children = !empty($membership->mid) && membership_entity_child_count($membership->mid);

  // Arrived from the "Add sub-member" action on a parent's page: the parent is
  // decided by where the administrator was standing, so it is shown rather
  // than offered. Only a new membership can be pre-filled this way; editing an
  // existing one always goes through the selector below.
  if (empty($membership->mid) && !empty($membership->pid)) {
    $form['membership']['pid'] = array(
      '#type' => 'value',
      '#value' => $membership->pid,
    );
    $form['membership']['pid_display'] = array(
      '#type' => 'item',
      '#title' => t('Belongs to'),
      '#markup' => check_plain(membership_entity_parent_label($membership->pid)),
      '#description' => t('This member is being added to that membership. To attach them elsewhere, save this membership and then edit it.'),
    );
  }
  elseif (!$has_children && user_access('administer memberships')) {
    if ($parents = membership_entity_eligible_parents($membership)) {
      $form['membership']['pid'] = array(
        '#type' => 'select',
        '#title' => t('Belongs to'),
        '#options' => array(0 => t('- None -')) + $parents,
        '#default_value' => !empty($membership->pid) ? $membership->pid : 0,
        '#description' => t('The household or association this membership belongs to. While that membership is not active this member has no access, whatever their own status says. Changing it moves the member; setting it to none uncouples them onto their own membership.'),
      );
    }
  }
  elseif (!$has_children) {
    // A household head or association secretary may only ever attach to their
    // own membership. They can let a member go, but moving somebody into a
    // different association is an administrator's decision — otherwise one
    // secretary could push members onto another's list.
    $sub_op = empty($membership->mid) ? 'create' : 'edit';
    $own = membership_entity_managed_parents($sub_op);

    if (!empty($membership->pid) && membership_entity_manages_sub_membership('detach', $membership)) {
      $current = membership_entity_managed_parents('edit');
      $options = array(0 => t('- Remove from this membership -'));
      if (isset($current[$membership->pid])) {
        $options[$membership->pid] = $current[$membership->pid];
      }

      $form['membership']['pid'] = array(
        '#type' => 'select',
        '#title' => t('Belongs to'),
        '#options' => $options,
        '#default_value' => $membership->pid,
        '#description' => t('Removing this member leaves their own membership and user account untouched — they simply no longer belong to yours. Moving them to a different membership is done by a site administrator.'),
      );
    }
    elseif ($sub_op == 'create' && count($own) == 1) {
      // Nothing to choose, so do not offer a choice: fix it to the one
      // membership they administer.
      $form['membership']['pid'] = array(
        '#type' => 'value',
        '#value' => key($own),
      );
      $form['membership']['pid_display'] = array(
        '#type' => 'item',
        '#title' => t('Belongs to'),
        '#markup' => check_plain(reset($own)),
      );
    }
    elseif ($sub_op == 'create' && count($own) > 1) {
      $form['membership']['pid'] = array(
        '#type' => 'select',
        '#title' => t('Belongs to'),
        '#options' => $own,
        '#default_value' => !empty($membership->pid) ? $membership->pid : key($own),
        '#required' => TRUE,
      );
    }
  }

  // The date they first joined, which is not derivable from the terms: on this
  // site 23 of 68 imported memberships have a created date years earlier than
  // their earliest surviving term. Presented the way core presents a node's
  // authored-on date, because that is exactly what it is.
  $form['membership']['created'] = array(
    // html_date is core's HTML5 date input: a real date picker in the browser,
    // still typable for a date twenty years ago, and no dependency on the date
    // module — which the base module does not have and should not gain for
    // one field.
    '#type' => 'html_date',
    '#title' => t('Joined'),
    '#default_value' => !empty($membership->created) ? format_date($membership->created, 'custom', DATE_FORMAT_DATE) : '',
    '#description' => t('The date this membership began, which is not necessarily the date of its earliest recorded term. Leave it empty to use today.'),
    '#access' => user_access('administer memberships'),
    '#weight' => -5,
  );

  $form['membership']['status'] = array(
    '#type' => 'radios',
    '#title' => t('Status'),
    '#options' => membership_entity_get_status_list(),
    '#default_value' => $membership->status,
    '#required' => TRUE,
    '#access' => user_access('administer memberships'),
  );

  $form['membership']['no_contact'] = array(
    '#type' => 'checkbox',
    '#title' => t('Do not contact this member'),
    '#default_value' => !empty($membership->no_contact),
    '#description' => t('Stops everything the site would otherwise send them, renewal reminders included. Set it when a member has died, or has asked to be left alone. The record itself is kept — this is about not writing to them, not about ending their membership.'),
    '#access' => membership_entity_access('edit', $membership),
  );

  // Add field UI items.
  field_attach_form('membership_entity', $membership, $form, $form_state);

  // Allow other modules to alter the form.
  backdrop_alter('membership_entity_form', $form, $form_state, $membership);
}

/**
 * Build the fields for creating the primary member's account on this form.
 *
 * Kept deliberately small: a username, an address and the postal option. Roles,
 * status and the password are not offered, because the person filling this in
 * may be an association secretary rather than a site administrator. The account
 * is created as an ordinary authenticated user with a random password, and the
 * member sets their own by asking for a new one.
 *
 * @param array $form
 *   The membership form, altered in place.
 * @param bool $conditional
 *   Whether the fields are one branch of a choice, and so must be hidden until
 *   that branch is picked, rather than the only way to name the member.
 */
function _membership_entity_new_account_elements(&$form, $conditional) {
  $form['membership']['new_account'] = array(
    '#type' => 'container',
    '#tree' => TRUE,
    '#attributes' => array('class' => array('membership-entity-new-account')),
  );

  if ($conditional) {
    $form['membership']['new_account']['#states'] = array(
      'visible' => array(
        ':input[name="member_source"]' => array('value' => 'new'),
      ),
    );
  }
  else {
    $form['membership']['new_account']['#type'] = 'fieldset';
    $form['membership']['new_account']['#title'] = t('Primary member');
    $form['membership']['new_account']['#description'] = t('The person this membership is for. An account is created for them so that the membership has an owner.');
  }

  $form['membership']['new_account']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Username'),
    '#maxlength' => USERNAME_MAX_LENGTH,
    '#description' => t('Spaces are allowed; punctuation is not allowed except for periods, hyphens, apostrophes, and underscores.'),
  );

  $form['membership']['new_account']['mail'] = array(
    '#type' => 'textfield',
    '#title' => t('E-mail address'),
    '#maxlength' => EMAIL_MAX_LENGTH,
  );

  // A member who deals with the organisation entirely by post has no address to
  // give, and Backdrop requires one. Generate an undeliverable one instead.
  if (config_get('membership_entity.settings', 'postal_members')) {
    $form['membership']['new_account']['postal'] = array(
      '#type' => 'checkbox',
      '#title' => t('No e-mail address (postal member)'),
      '#description' => t('Generates an undeliverable address at %domain. They will not be able to sign in, which is intended: the account exists so they can hold a membership.', array(
        '%domain' => membership_entity_postal_mail_domain(),
      )),
    );
    $form['membership']['new_account']['mail']['#states'] = array(
      'invisible' => array(
        ':input[name="new_account[postal]"]' => array('checked' => TRUE),
      ),
    );
  }
}

/**
 * Validate the account being created for a new membership's primary member.
 *
 * Mirrors the checks core makes on the registration form. Nothing here relies
 * on #required, because the fields are one branch of a choice and the other
 * branch leaves them legitimately empty.
 *
 * @param array $form
 *   The membership form.
 * @param array $form_state
 *   The current state of the form, whose values are altered in place: a postal
 *   member's generated address is written back so the submit handler can use
 *   it.
 */
function _membership_entity_new_account_validate($form, &$form_state) {
  $values = $form_state['values']['new_account'];
  $element = $form['membership']['new_account'];
  $postal = !empty($values['postal']);

  $name = trim($values['name']);
  form_set_value($element['name'], $name, $form_state);

  if ($name === '') {
    form_set_error('new_account][name', t('You must enter a username for the new member.'));
  }
  elseif ($error = user_validate_name($name)) {
    form_set_error('new_account][name', $error);
  }
  elseif ((bool) db_query_range('SELECT uid FROM {users} WHERE name = :name', 0, 1, array(':name' => $name))->fetchField()) {
    form_set_error('new_account][name', t('The name %name is already taken.', array('%name' => $name)));
  }

  // A postal member's address is derived from the username, so it can only be
  // built once the name is known to be good and unique.
  if ($postal) {
    if (!form_get_errors()) {
      form_set_value($element['mail'], membership_entity_generate_postal_mail($name), $form_state);
    }
    return;
  }

  $mail = trim($values['mail']);
  form_set_value($element['mail'], $mail, $form_state);

  if ($mail === '') {
    form_set_error('new_account][mail', t('You must enter an e-mail address.'));
  }
  elseif (!valid_email_address($mail)) {
    form_set_error('new_account][mail', t('The e-mail address %mail is not valid.', array('%mail' => $mail)));
  }
  elseif ((bool) db_query_range('SELECT uid FROM {users} WHERE mail = :mail', 0, 1, array(':mail' => $mail))->fetchField()) {
    form_set_error('new_account][mail', t('The e-mail address %mail is already registered.', array('%mail' => $mail)));
  }
}

/**
 * Submit handler: create the primary member's account before the membership.
 *
 * Runs ahead of membership_entity_edit_form_submit(), which needs a uid to hang
 * the membership on.
 *
 * @see membership_entity_edit_form()
 */
function membership_entity_create_member_account_submit($form, &$form_state) {
  if (!_membership_entity_creating_account($form_state)) {
    return;
  }

  $values = $form_state['values']['new_account'];

  // Backdrop's user_save() takes an entity; the Drupal 7 (NULL, $edit) form
  // does not exist. Roles are left alone deliberately - membership status is
  // what grants the member role, through the controller.
  $account = entity_create('user', array(
    'name' => $values['name'],
    'mail' => $values['mail'],
    'pass' => user_password(32),
    'init' => $values['mail'],
    'status' => 1,
  ));
  $account->save();

  if (empty($account->uid)) {
    form_set_error('new_account][name', t('The account could not be created, so no membership was saved.'));
    $form_state['rebuild'] = TRUE;
    return;
  }

  $form_state['values']['uid'] = $account->uid;
  $form_state['membership_entity_new_account'] = $account;

  watchdog('membership_entity', 'Created account %name while adding a membership.', array(
    '%name' => $account->name,
  ), WATCHDOG_NOTICE, l(t('edit'), 'user/' . $account->uid . '/edit'));

  backdrop_set_message(t('Created an account for %name.', array('%name' => format_username($account))));
}

/**
 * Whether this submission is creating the primary member's account.
 *
 * @param array $form_state
 *   The current state of the form.
 *
 * @return bool
 *   TRUE when the new account branch is the one in use.
 */
function _membership_entity_creating_account($form_state) {
  if (!isset($form_state['values']['new_account'])) {
    return FALSE;
  }

  // Where no choice was offered the fields are the only way to name the member,
  // so their presence is the answer.
  return !isset($form_state['values']['member_source'])
    || $form_state['values']['member_source'] == 'new';
}

/**
 * Retained memberships held by a set of accounts.
 *
 * @param array $accounts
 *   User objects, keyed by uid.
 *
 * @return array
 *   Retained memberships, keyed by mid.
 */
function _membership_entity_retained_for_accounts(array $accounts) {
  $retained = array();

  foreach ($accounts as $account) {
    if (empty($account->uid)) {
      continue;
    }
    foreach (membership_entity_load_by_user($account->uid) as $membership) {
      if (membership_entity_is_retained($membership)) {
        $retained[$membership->mid] = $membership;
      }
    }
  }

  return $retained;
}

/**
 * Take the deleting options off an account cancellation form.
 *
 * Disabling the account is better than deleting it in every respect that
 * matters to a kept record: the address, the e-mail and any profile fields stay
 * where they are, so the membership still knows who it belonged to without
 * anything having to be copied out of the account first. It also leaves the
 * person able to be reinstated rather than re-created, which is what keeping
 * their member number on a rejoin depends on.
 *
 * @param array $form
 *   The cancellation form, altered in place.
 * @param array $retained
 *   The memberships that are being kept.
 * @param string $explanation
 *   Text saying which accounts are affected.
 */
function _membership_entity_restrict_cancel_methods(&$form, array $retained, $explanation) {
  foreach (array('user_cancel_reassign', 'user_cancel_delete') as $method) {
    if (isset($form['user_cancel_method'][$method])) {
      unset($form['user_cancel_method'][$method]);
    }
  }

  // Whatever the site default is, it may have been one of the two just removed,
  // which would leave nothing selected.
  if (isset($form['user_cancel_method']['user_cancel_block'])) {
    $form['user_cancel_method']['user_cancel_block']['#default_value'] = 'user_cancel_block';
  }

  $reasons = array();
  foreach ($retained as $membership) {
    $reasons[] = membership_entity_retention_reason($membership);
  }

  $form['membership_entity_retained'] = array(
    '#theme_wrappers' => array('container'),
    '#attributes' => array('class' => array('messages', 'warning')),
    '#markup' => $explanation . theme('item_list', array('items' => $reasons)),
    '#weight' => -10,
  );

  $form['#validate'][] = '_membership_entity_cancel_method_validate';
}

/**
 * Validate handler: refuse a cancellation method that deletes the account.
 *
 * The options are taken off the form, but a radio can be posted around, so the
 * rule is enforced here as well. Restricting what is offered is presentation;
 * this is the part that holds.
 */
function _membership_entity_cancel_method_validate($form, &$form_state) {
  if (!in_array($form_state['values']['user_cancel_method'], array('user_cancel_reassign', 'user_cancel_delete'))) {
    return;
  }

  form_set_error('user_cancel_method', t('These accounts hold membership records that are being kept, so they cannot be deleted. Disable the account instead — the record then keeps the address and contact details it was made with. An administrator with permission to delete retained memberships can remove the records first if they are genuinely to go.'));
}

/**
 * Implements hook_form_FORM_ID_alter() for user_cancel_confirm_form().
 *
 * Deleting an account would either destroy a membership record that is being
 * kept, or strand it with nobody to link to. Offer disabling the account
 * instead, which keeps everything the record needs.
 */
function membership_entity_form_user_cancel_confirm_form_alter(&$form, &$form_state) {
  if (empty($form['_account']['#value'])) {
    return;
  }

  $account = $form['_account']['#value'];
  $retained = _membership_entity_retained_for_accounts(array($account->uid => $account));
  if (empty($retained)) {
    return;
  }

  _membership_entity_restrict_cancel_methods($form, $retained, t('This account holds membership records that are being kept, so it cannot be deleted. Disabling it keeps the account details the records were made with, and the person can be reinstated later.'));
}

/**
 * Implements hook_form_FORM_ID_alter() for user_multiple_cancel_confirm().
 *
 * @see membership_entity_form_user_cancel_confirm_form_alter()
 */
function membership_entity_form_user_multiple_cancel_confirm_alter(&$form, &$form_state) {
  if (empty($form_state['accounts'])) {
    return;
  }

  $retained = _membership_entity_retained_for_accounts($form_state['accounts']);
  if (empty($retained)) {
    return;
  }

  _membership_entity_restrict_cancel_methods($form, $retained, format_plural(count($retained),
    'One of these accounts holds a membership record that is being kept, so none of them can be deleted in one go. Disabling them keeps the account details the records were made with.',
    'These accounts hold @count membership records that are being kept, so they cannot be deleted. Disabling them keeps the account details the records were made with.'));
}

/**
 * Validate the membership entity form.
 *
 * @see membership_entity_form()
 */
function membership_entity_form_validate($form, &$form_state) {
  $values = &$form_state['values'];
  $membership = $form['#membership_entity'];

  // A membership must belong to somebody. Where creating the account is on
  // offer the primary member field is not marked required, so whichever branch
  // was chosen has to be checked here instead.
  if (isset($form['membership']['new_account'])) {
    if (_membership_entity_creating_account($form_state)) {
      _membership_entity_new_account_validate($form, $form_state);
    }
    elseif (empty($values['uid'])) {
      form_set_error('uid', t('Choose the member this membership is for, or create an account for them.'));
    }
  }

  // The joining date arrives as a Y-m-d string and has to be a timestamp by the
  // time the entity is built. core's html_date_validate() has already rejected
  // anything that is not a real date, during the same recursion that runs
  // before this handler.
  if (isset($values['created'])) {
    $entered = trim($values['created']);

    if ($entered === '') {
      form_set_value($form['membership']['created'], !empty($membership->created) ? $membership->created : REQUEST_TIME, $form_state);
    }
    else {
      // Built in the site's timezone rather than the server's, and with the
      // time of day the record already had. Two things depend on that: a date
      // near midnight would otherwise land on the wrong day, and a membership
      // saved for some unrelated reason would silently have its stored moment
      // moved to 00:00.
      $timezone = new DateTimeZone(backdrop_get_user_timezone());
      $date = DateTime::createFromFormat(DATE_FORMAT_DATE, $entered, $timezone);

      if ($date === FALSE) {
        form_set_error('created', t('%date is not a date this membership could have begun on.', array('%date' => $entered)));
      }
      else {
        $time = !empty($membership->created)
          ? explode(':', format_date($membership->created, 'custom', 'H:i:s'))
          : array(0, 0, 0);
        $date->setTime((int) $time[0], (int) $time[1], (int) $time[2]);

        form_set_value($form['membership']['created'], $date->getTimestamp(), $form_state);
      }
    }
  }

  if (!empty($values['member_id'])) {
    // The site id number must be unique across every membership type.
    if ((bool) db_query_range('SELECT mid FROM {membership_entity} WHERE mid <> :mid AND member_id = :member_id', 0, 1, array(':mid' => $membership->mid, ':member_id' => $values['member_id']))->fetchField()) {
      form_set_error('member_id', t('Site id number %member_id is already in use.', array(
        '%member_id' => $values['member_id'],
      )));
    }
  }

  // Flatten the primary member field and make sure there is not already a
  // membership for this user.
  if (!empty($values['uid']) && $uid = reset($values['uid'])) {
    if ($uid != $membership->uid) {
      $mid = membership_entity_conflicting_membership($uid, $membership->type, !empty($membership->mid) ? $membership->mid : 0);

      if ($mid) {
        form_set_error('uid', t('A current membership already exists for %name. Did you want to <a href="!url">renew</a> it? This membership type does not allow one person to hold two at once.', array(
          '%name' => membership_entity_member_name(membership_entity_load($mid)),
          '!url' => url('membership/' . $mid . '/renew'),
        )));
      }
    }
    form_set_value($form['membership']['uid'], $uid, $form_state);
  }

  if (!empty($values['secondary_members'])) {
    // Make sure secondary members are unique.
    form_set_value($form['membership']['secondary_members'], array_values(array_unique($values['secondary_members'])), $form_state);

    // Don't allow the primary member as a secondary member.
    foreach ($values['secondary_members'] as $delta => $value) {
      if ($values['uid'] == $value) {
        form_set_error("secondary_members][$delta][value", t('%name is the primary member.', array(
          '%name' => format_username(user_load($values['uid'])),
        )));
      }
    }
  }

  // Anyone without full membership administration may only attach to their own
  // membership. Enforced here as well as in the form: the select can be posted
  // around, so restricting the options is presentation, not security.
  if (isset($values['pid']) && !user_access('administer memberships')) {
    $submitted = (int) $values['pid'];

    // A membership that has not been saved yet has no original parent, however
    // the form was seeded — the "Add sub-member" action pre-fills pid, and that
    // must still be checked rather than waved through as "unchanged".
    $original = (!empty($membership->mid) && !empty($membership->pid)) ? (int) $membership->pid : 0;

    if ($submitted !== $original) {
      if ($submitted === 0) {
        // Letting a member go.
        if (!membership_entity_manages_sub_membership('detach', $membership)) {
          form_set_error('pid', t('You do not have permission to remove this member.'));
        }
      }
      else {
        // Attaching, whether new or moved. Moving between parents is barred
        // outright — a member who has left one membership is re-attached by an
        // administrator, so that nobody can push members onto another list.
        $op = empty($membership->mid) ? 'create' : 'edit';
        $allowed = membership_entity_managed_parents($op);

        if ($original !== 0) {
          form_set_error('pid', t('You cannot move this member to a different membership. Remove them, and a site administrator can attach them elsewhere.'));
        }
        elseif (!isset($allowed[$submitted])) {
          form_set_error('pid', t('You do not have permission to attach a member to that membership.'));
        }
      }
    }
  }

  // Validate the parent membership. The select only offers eligible parents,
  // but a membership may have gained sub-members or filled up since the form
  // was built, and the value can be posted directly.
  if (isset($values['pid']) && !empty($values['pid'])) {
    if (!empty($membership->mid) && $values['pid'] == $membership->mid) {
      form_set_error('pid', t('A membership cannot belong to itself.'));
    }
    elseif (!empty($membership->mid) && membership_entity_child_count($membership->mid)) {
      form_set_error('pid', t('This membership has sub-members of its own, so it cannot also belong to another membership.'));
    }
    elseif ($parent = membership_entity_load($values['pid'])) {
      $parent_settings = membership_entity_get_bundle_settings($parent->type);
      $max = $parent_settings['max_children'];

      if (!empty($parent->pid)) {
        form_set_error('pid', t('%id already belongs to another membership, so it cannot take sub-members itself.', array(
          '%id' => $parent->member_id,
        )));
      }
      elseif (empty($max)) {
        form_set_error('pid', t('%id is of a type that does not allow sub-members.', array(
          '%id' => $parent->member_id,
        )));
      }
      elseif ($max != FIELD_CARDINALITY_UNLIMITED && $values['pid'] != $membership->pid
        && membership_entity_child_count($parent->mid) >= $max) {
        form_set_error('pid', t('%id already has its maximum of @max sub-members.', array(
          '%id' => $parent->member_id,
          '@max' => $max,
        )));
      }
      // Moving a member who renews separately under a parent that pays for
      // everyone leaves their own terms in place, where they can still expire
      // the member even though the parent is paid up. Worth saying, not worth
      // destroying term history over.
      elseif ($parent_settings['child_terms'] == 'inherit' && !empty($membership->terms)) {
        backdrop_set_message(t('%id pays for all of its sub-members, but this membership has terms of its own. Those terms still apply and can expire this member. Review them if that is not what you want.', array(
          '%id' => $parent->member_id,
        )), 'warning');
      }
    }
    else {
      form_set_error('pid', t('The selected parent membership no longer exists.'));
    }
  }

  // Validate custom fields.
  entity_form_field_validate('membership_entity', $form, $form_state);
}

/**
 * Validate handler for the join form registration.
 */
function membership_entity_user_join_register_validate($form, &$form_state) {
  _membership_entity_form_field_validate('user', $form['user'], $form_state);
}

/**
 * Submit handler for the join form registration.
 *
 * @see user_register_submit()
 */
function membership_entity_user_join_register_submit($form, &$form_state) {
  $values = &$form_state['values']['user'];

  $pass = !config_get('system.core', 'user_email_verification') ? $pass = $values['pass'] : user_password();

  // A postal member's address can never receive anything, so a password they
  // could use is a door with no key and no way to ask for one. Give the account
  // a long random one instead: undeliverable and unenterable together.
  if (!empty($values['postal'])) {
    $pass = user_password(32);
  }

  // Remove unneeded values.
  form_state_values_clean($form_state);

  $values['pass'] = $pass;
  $values['init'] = $values['mail'];

  $account = $form['#user'];
  _membership_entity_form_submit_build_entity('user', $account, $form['user'], $form_state);

  // Backdrop's user_save() takes a single argument and simply calls
  // $account->save(); the Drupal 7 $edit array has no equivalent and was being
  // discarded. Save the entity directly and judge the outcome by its status,
  // as core's user_register_submit() does.
  $status = $account->save();

  // Terminate if an error occurred while saving the account.
  if ($status != SAVED_NEW) {
    backdrop_set_message(t('Error saving user account.'), 'error');
    $form_state['redirect'] = '';
    return;
  }
  $form_state['user'] = $account;
  $form_state['values']['uid'] = $values['uid'] = $account->uid;

  watchdog('membership_entity', 'New user registration with membership: %name (%email).', array(
    '%name' => $values['name'],
    '%email' => $values['mail'],
  ), WATCHDOG_NOTICE, l(t('edit'), 'user/' . $account->uid . '/edit'));

  $account->password = $pass;
  // No e-mail verification required; log in user immediately.
  if (!config_get('system.core', 'user_email_verification') && $account->status) {
    _user_mail_notify('register_no_approval_required', $account);
    $form_state['uid'] = $account->uid;
    user_login_submit(array(), $form_state);
    backdrop_set_message(t('Registration successful. You are now logged in.'));
    $form_state['redirect'] = '';
  }
  // No administrator approval required.
  elseif ($account->status) {
    _user_mail_notify('register_no_approval_required', $account);
    backdrop_set_message(t('A welcome message with further instructions has been sent to your e-mail address.'));
    $form_state['redirect'] = '';
  }
  else {
    _user_mail_notify('register_pending_approval', $account);
    backdrop_set_message(t('Thank you for applying for an account. Your account is currently pending approval by the site administrator.<br />In the meantime, a welcome message with further instructions has been sent to your e-mail address.'));
    $form_state['redirect'] = '';
  }
}

/**
 * Submit the membership entity form.
 *
 * @see membership_entity_form()
 * @see membership_entity_form_validate()
 */
function membership_entity_edit_form_submit($form, &$form_state) {
  $membership = $form['#membership_entity'];

  // Remove extra stuff.
  form_state_values_clean($form_state);
  entity_form_submit_build_entity('membership_entity', $membership, $form, $form_state);
  $status = membership_entity_save($membership);

  // Add the saved membership object to $form_state for other modules.
  $form_state['membership_entity'] = $membership;

  $t_args = array('%member_id' => $membership->member_id);
  switch ($status) {
    case SAVED_NEW:
      if (user_access('administer memberships')) {
        backdrop_set_message(t('Membership %member_id has been created.', $t_args));
        $form_state['redirect'] = 'membership/' . $membership->mid;
      }
      elseif (user_is_logged_in()) {
        backdrop_set_message(t('Your membership has been created.'));
        $form_state['redirect'] = 'user';
      }
      break;

    case SAVED_UPDATED:
      backdrop_set_message(t('Membership %member_id has been updated.', $t_args));
      $form_state['redirect'] = 'membership/' . $membership->mid;
      break;
  }
}

/**
 * Submit handler to delete a membership.
 */
function membership_entity_edit_form_delete($form, &$form_state) {
  $destination = array();
  if (isset($_GET['destination'])) {
    $destination = backdrop_get_destination();
    unset($_GET['destination']);
  }
  $membership = $form['#membership_entity'];
  $form_state['redirect'] = array(
    'membership/' . $membership->mid . '/delete',
    array('query' => $destination),
  );
}

/**
 * Dispaly a confirmation page before delteing a membership.
 */
function membership_entity_delete_confirm($form, &$form_state, $membership) {
  // Deleting a membership that still has sub-members would strand them against
  // a parent that no longer exists, leaving their access governed by nothing.
  // They have to be moved or removed first — a decision only an administrator
  // can make.
  if ($children = membership_entity_child_count($membership->mid)) {
    backdrop_set_title(t('This membership cannot be deleted yet'));

    $form['blocked'] = array(
      '#theme_wrappers' => array('container'),
      '#markup' => format_plural($children,
        'Membership %member_id still has 1 sub-member. Move or delete that membership first.',
        'Membership %member_id still has @count sub-members. Move or delete those memberships first.',
        array('%member_id' => $membership->member_id)),
    );

    $form['actions'] = array('#type' => 'actions');
    $form['actions']['back'] = array(
      '#type' => 'link',
      '#title' => t('Back to the membership'),
      '#href' => 'membership/' . $membership->mid,
    );

    return $form;
  }

  // Membership records are commonly kept for a fixed period after they lapse,
  // and for an organisation running a block insurance scheme they are insurance
  // records. Removing one early is a deliberate act, not a tidy-up.
  if (membership_entity_is_retained($membership) && !user_access('delete retained memberships')) {
    backdrop_set_title(t('This membership cannot be deleted yet'));

    $form['blocked'] = array(
      '#theme_wrappers' => array('container'),
      '#markup' => membership_entity_retention_reason($membership) . ' '
        . t('The period is set on the <a href="!url">membership type</a>.', array(
          '!url' => url('admin/memberships/types/manage/' . $membership->type),
        )),
    );

    $form['actions'] = array('#type' => 'actions');
    $form['actions']['back'] = array(
      '#type' => 'link',
      '#title' => t('Back to the membership'),
      '#href' => 'membership/' . $membership->mid,
    );

    return $form;
  }

  $form_state['memberships'] = array(
    $membership->mid => $membership,
  );

  $form['membership'] = array(
    '#theme_wrappers' => array('container'),
    '#attributes' => array(),
    '#markup' => t('You are about to delete membership %member_id.', array(
      '%member_id' => $membership->member_id,
    )),
  );

  $form['#submit'][] = 'membership_entity_multiple_delete_confirm_submit';
  return confirm_form($form, t('Are you sure you want to delete this membership?'), 'membership/' . $membership->mid, t('This action cannot be undone.'), t('Delete'), t('Cancel'));
}

/**
 * Submit the memberships delete confirm form.
 */
function membership_entity_multiple_delete_confirm_submit($form, &$form_state) {
  $memberships = $form_state['memberships'];
  if ($form_state['values']['confirm']) {
    membership_entity_delete_multiple(array_keys($memberships));
    $count = count($memberships);
    backdrop_set_message(format_plural($count, 'Deleted 1 membership.', 'Deleted @count memberships.'));
  }
  $form_state['redirect'] = 'admin/memberships';
}
