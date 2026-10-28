<?php

/**
 * @file
 * Contains the child count field handler.
 */

/**
 * Field handler to display how many memberships belong to a membership.
 */
class views_handler_field_membership_child_count extends views_handler_field_numeric { // @codingStandardsIgnoreLine, views handler naming

  /**
   * The child counts for this result set, keyed by parent mid.
   *
   * @var array
   */
  protected $counts = array();

  /**
   * Whether pre_render() has run and populated the counts.
   *
   * @var bool
   */
  protected $prepared = FALSE;

  /**
   * {@inheritdoc}
   */
  public function query() {
    $this->ensure_my_table();
    $this->add_additional_fields(array('mid' => 'mid'));
  }

  /**
   * Count the children of every membership on the page in a single query.
   *
   * Doing this per row would issue one query per membership, which an
   * association listing of any size would feel.
   */
  public function pre_render(&$values) { // @codingStandardsIgnoreLine, views handler naming
    $mids = array();
    foreach ($values as $row) {
      $mid = $this->get_value($row, 'mid');
      if (!empty($mid)) {
        $mids[$mid] = $mid;
      }
    }

    if (empty($mids)) {
      return;
    }

    $result = db_query('SELECT pid, COUNT(mid) AS total FROM {membership_entity} WHERE pid IN (:pids) GROUP BY pid', array(
      ':pids' => $mids,
    ));
    foreach ($result as $row) {
      $this->counts[$row->pid] = $row->total;
    }

    $this->prepared = TRUE;
  }

  /**
   * {@inheritdoc}
   */
  public function render($values) {
    $mid = $this->get_value($values, 'mid');

    // Not every render path calls pre_render(). Without this the field would
    // quietly report zero sub-members for everyone, which reads as fact rather
    // than as a failure.
    if (!$this->prepared) {
      return parent::render((object) array(
        $this->field_alias => membership_entity_child_count($mid),
      ));
    }

    $count = isset($this->counts[$mid]) ? $this->counts[$mid] : 0;

    return parent::render((object) array(
      $this->field_alias => $count,
    ));
  }

}
