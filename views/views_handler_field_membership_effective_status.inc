<?php

/**
 * @file
 * Contains the effective status field handler.
 */

/**
 * Field handler to display the status that actually governs access.
 *
 * A membership beneath a lapsed household or association grants nothing, even
 * though its own stored status is still active. This field shows what a member
 * actually has, which the plain status column cannot.
 */
class views_handler_field_membership_effective_status extends views_handler_field { // @codingStandardsIgnoreLine, views handler naming

  /**
   * Parent statuses for this result set, keyed by parent mid.
   *
   * @var array
   */
  protected $parent_status = array(); // @codingStandardsIgnoreLine, views handler naming

  /**
   * Whether pre_render() has run and populated the parent statuses.
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
      'status' => 'status',
    ));
  }

  /**
   * Fetch every parent status needed by this page in a single query.
   */
  public function pre_render(&$values) { // @codingStandardsIgnoreLine, views handler naming
    $pids = array();
    foreach ($values as $row) {
      $pid = $this->get_value($row, 'pid');
      if (!empty($pid)) {
        $pids[$pid] = $pid;
      }
    }

    $this->prepared = TRUE;

    if (empty($pids)) {
      return;
    }

    $result = db_query('SELECT mid, status FROM {membership_entity} WHERE mid IN (:mids)', array(
      ':mids' => $pids,
    ));
    foreach ($result as $row) {
      $this->parent_status[$row->mid] = (int) $row->status;
    }
  }

  /**
   * {@inheritdoc}
   */
  public function render($values) {
    $status = (int) $this->get_value($values, 'status');
    $pid = $this->get_value($values, 'pid');

    // Not every render path calls pre_render(). Falling back to a direct
    // lookup costs a query per row, which is far better than reporting a
    // lapsed member as active.
    if (!$this->prepared && !empty($pid) && !isset($this->parent_status[$pid])) {
      $this->parent_status[$pid] = (int) db_query('SELECT status FROM {membership_entity} WHERE mid = :mid', array(
        ':mid' => $pid,
      ))->fetchField();
    }

    // Mirrors membership_entity_effective_status(): only an otherwise active
    // membership can be blocked by its parent, and the parent's own status is
    // shown so the reason stays visible.
    if (!empty($pid) && $status == MEMBERSHIP_ENTITY_ACTIVE && isset($this->parent_status[$pid])) {
      if ($this->parent_status[$pid] != MEMBERSHIP_ENTITY_ACTIVE) {
        $status = $this->parent_status[$pid];
      }
    }

    $list = membership_entity_get_status_list();
    return isset($list[$status]) ? check_plain($list[$status]) : '';
  }

}
