<?php

/**
 * CTOOLS Compatability functions
 */

/**
 * Get a class from a plugin, if it exists. If the plugin is not already
 * loaded, try ctools_plugin_load_class() instead.
 *
 * @param $plugin_definition
 *   The loaded plugin type.
 * @param $class_name
 *   The identifier of the class. For example, 'handler'.
 *
 * @return string
 *   The actual name of the class to call, or NULL if the class does not exist.
 */
function ctools_plugin_get_class($plugin_definition, $class_name) {
  // If cached the .inc file may not have been loaded. require_once is quite safe
  // and fast so it's okay to keep calling it.
  if (isset($plugin_definition['file'])) {
    // Plugins that are loaded from info files have the info file as
    // $plugin['file'].  Don't try to run those.
    $info = ctools_plugin_get_info($plugin_definition['plugin module'], $plugin_definition['plugin type']);
    if (empty($info['info file'])) {
      require_once BACKDROP_ROOT . '/' . $plugin_definition['path'] . '/' . $plugin_definition['file'];
    }
  }

  $return = FALSE;
  if (!isset($plugin_definition[$class_name])) {
    return;
  }
  elseif (is_string($plugin_definition[$class_name])) {
    // Plugin uses the string form shorthand.
    $return = $plugin_definition[$class_name];
  }
  elseif (isset($plugin_definition[$class_name]['class'])) {
    // Plugin uses the verbose array form.
    $return = $plugin_definition[$class_name]['class'];
  }
  // @todo consider adding an else {watchdog(...)} here

  return ($return && class_exists($return)) ? $return : NULL;
}

/**
 * Ask a module for info about a particular plugin type.
 */
function ctools_plugin_get_info($module, $type) {
  $all_info = ctools_plugin_get_plugin_type_info();
  return isset($all_info[$module][$type]) ? $all_info[$module][$type] : array();
}

/**
 * Return the full list of plugin type info for all plugin types registered in
 * the current system.
 *
 * This function manages its own cache getting/setting, and should always be
 * used as the way to initially populate the list of plugin types. Make sure you
 * call this function to properly populate the ctools_plugin_type_info static
 * variable.
 *
 * @return array
 *   A multilevel array of plugin type info, the outer array keyed on module
 *   name and each inner array keyed on plugin type name.
 */
function ctools_plugin_get_plugin_type_info($flush = FALSE) {
  static $backdrop_static_fast;
  if (!isset($backdrop_static_fast)) {
    $backdrop_static_fast['info_loaded'] = &backdrop_static('ctools_plugin_type_info_loaded', FALSE);
    $backdrop_static_fast['all_type_info'] = &backdrop_static('ctools_plugin_type_info', array());
  }
  $info_loaded = &$backdrop_static_fast['info_loaded'];
  $all_type_info = &$backdrop_static_fast['all_type_info'];

  // Only trigger info loading once.
  if ($info_loaded && !$flush) {
    return $all_type_info;
  }
  $info_loaded = TRUE;

  $cache = cache_get('ctools_plugin_type_info');
  if (!empty($cache->data) && !$flush) {
    // Plugin type info cache is warm, use it.
    $all_type_info = $cache->data;
  }
  else {
    // Cache expired, refill it.
    foreach (module_implements('ctools_plugin_type') as $module) {
      $module_infos = array();
      $function = $module . '_ctools_plugin_type';
      $module_infos = $function();

      foreach ($module_infos as $plugin_type_name => $plugin_type_info) {
        // Apply defaults. Array addition will not overwrite pre-existing keys.
        $plugin_type_info += array(
          'module' => $module,
          'type' => $plugin_type_name,
          'cache' => FALSE,
          'cache table' => 'cache',
          'classes' => array(),
          'use hooks' => FALSE,
          'defaults' => array(),
          'process' => '',
          'alterable' => TRUE,
          'extension' => 'inc',
          'info file' => FALSE,
          'hook' => $module . '_' . $plugin_type_name,
          'load themes' => FALSE,
        );
        $all_type_info[$module][$plugin_type_name] = $plugin_type_info;
      }
    }
    cache_set('ctools_plugin_type_info', $all_type_info);
  }

  return $all_type_info;
}
