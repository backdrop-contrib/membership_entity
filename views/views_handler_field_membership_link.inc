<?php

/**
 * @file
 * Definition of views_handler_field_membership_link.
 */

/**
 * Field handler to show a link to the membership.
 */
class views_handler_field_membership_link extends views_handler_field_entity { // @codingStandardsIgnoreLine, Views override compliant

  /**
   * Store Entity Information array returned by entity_get_info().
   *
   * @var array
   */
  public $entityInfo;

  /**
   * Operation beig performed.
   *
   * @var array
   */
  public $op;

  /**
   * {@inheritdoc}
   */
  public function init(&$view, &$options) {
    parent::init($view, $options);
    $this->entityInfo = entity_get_info($this->entity_type);
    $this->op = $this->definition['op'];

    // Add the primary key field.
    $key = $this->entityInfo['entity keys']['id'];
    $this->additional_fields[$key] = $key;
  }

  /**
   * {@inheritdoc}
   */
  public function option_definition() { // @codingStandardsIgnoreLine, Views override compliant
    $options = parent::option_definition();
    $options['text'] = array('default' => $this->op, 'translatable' => TRUE);
    return $options;
  }

  /**
   * {@inheritdoc}
   */
  public function options_form(&$form, &$form_state) { // @codingStandardsIgnoreLine, Views override compliant
    $form['text'] = array(
      '#type' => 'textfield',
      '#title' => t('Text to display'),
      '#default_value' => $this->options['text'],
    );
    parent::options_form($form, $form_state);

    // The path is set by render_link function so don't allow to set it.
    $form['alter']['path'] = array('#access' => FALSE);
    $form['alter']['external'] = array('#access' => FALSE);
  }

  /**
   * {@inheritdoc}
   */
  public function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

  /**
   * {@inheritdoc}
   */
  public function render($values) { // @codingStandardsIgnoreLine, Views override compliant
    if ($entity = $this->get_value($values)) {
      return $this->render_link($entity);
    }
  }

  /**
   * Build a membership link.
   *
   * @param Entity $entity
   *   The membership entity object.
   *
   * @return string
   *   HTML for the membership link.
   */
  public function render_link(Entity $entity) { // @codingStandardsIgnoreLine, Views override compliant
    if ($this->link_access($entity)) {
      $this->options['alter']['make_link'] = TRUE;
      $uri = entity_uri($this->entity_type, $entity);
      $this->options['alter']['path'] = $uri['path'];
      if ($this->op != 'view') {
        $this->options['alter']['path'] .= "/$this->op";
        $this->options['alter']['query'] = backdrop_get_destination();
      }
      $text = !empty($this->options['text']) ? $this->options['text'] : $this->op;
      return $text;
    }
  }

  /**
   * Whether the current user may follow the link, by the route's own rules.
   *
   * Not entity_access(): in Backdrop it accepts only create, view, update and
   * delete, so 'edit', 'renew', 'clone' and 'export' were always refused, and
   * none of these entity classes overrides access(), so view and delete were
   * granted to everybody. The entity type's own access callback is what the
   * routes use. Renew uses the renew page's check, which also knows about
   * sub-memberships and lifetime terms.
   *
   * @param Entity $entity
   *   The entity the link is for.
   *
   * @return bool
   *   TRUE if the link should be shown.
   */
  protected function link_access(Entity $entity) {
    if ($this->op == 'renew' && $entity instanceof MembershipEntity) {
      return membership_entity_term_access_renew($entity);
    }
    $callback = isset($this->entityInfo['access callback']) ? $this->entityInfo['access callback'] : NULL;
    if ($callback && function_exists($callback)) {
      return (bool) $callback($this->op, $entity);
    }
    return entity_access($this->op, $this->entity_type, $entity);
  }

}
