<?php

/**
 * @file
 * A one-box search across several fields, matched field by field.
 */

/**
 * Filter handler: match the text in any of the chosen fields.
 *
 * Core's "Combine fields filter" joins the fields with CONCAT_WS() and matches
 * the result. MySQL refuses that when the columns have different collations
 * (membership_entity.member_id and users.name commonly do, depending on when
 * each table was created): "Illegal mix of collations ... for operation
 * 'like'". Views then shows an empty result, so the search silently finds
 * nothing. Matching each field on its own and ORing the results never mixes
 * collations, and for "contains" it finds the same rows.
 *
 * Only the "contains" operator is offered, which is what a search box needs.
 */
class views_handler_filter_membership_search extends views_handler_filter_combine {

  /**
   * {@inheritdoc}
   */
  function operators() {
    $operators = parent::operators();
    return array('contains' => $operators['contains']);
  }

  /**
   * {@inheritdoc}
   */
  function query() {
    $this->view->_build('field');
    $value = trim((string) $this->value);
    if ($value === '') {
      return;
    }

    $conditions = array();
    $placeholder = $this->placeholder();
    foreach ($this->options['fields'] as $id) {
      // Field access checks may have removed this handler.
      if (!isset($this->view->field[$id])) {
        continue;
      }
      $field = $this->view->field[$id];
      $field->ensure_my_table();
      if (!empty($field->table_alias) && !empty($field->real_field)) {
        $conditions[] = "$field->table_alias.$field->real_field LIKE $placeholder";
      }
    }

    if ($conditions) {
      $this->query->add_where_expression($this->options['group'], '(' . implode(' OR ', $conditions) . ')', array(
        $placeholder => '%' . db_like($value) . '%',
      ));
    }
  }

}
