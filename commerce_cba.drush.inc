<?php

/**
 * @file
 * Drush command file.
 */

function drush_commerce_cba_post_sql_sync($source = NULL, $destination = NULL) {
  $settings = array();
  $rule = rules_config_load('commerce_payment_cba');
  foreach ($rule->actions() as $action) {
    if (!empty($action->settings['payment_method']['settings']['mode'])) {
      $action->settings['payment_method']['settings']['mode'] = 'sandbox';
    }
  }
  entity_save('rules_config', $rule);
}
