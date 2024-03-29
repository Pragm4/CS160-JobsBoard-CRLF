<?php

/**
 * Implementation of hook_views_default_views().
 */
function message_example_views_default_views() {
  $views = array();

  // Exported view: message_example
  $view = new view;
  $view->name = 'message_example';
  $view->description = '';
  $view->tag = '';
  $view->base_table = 'message';
  $view->human_name = 'Message example';
  $view->core = 7;
  $view->api_version = '3.0-alpha1';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

  /* Display: Defaults */
  $handler = $view->new_display('default', 'Defaults', 'default');
  $handler->display->display_options['access']['type'] = 'none';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['pager']['type'] = 'full';
  $handler->display->display_options['style_plugin'] = 'default';
  $handler->display->display_options['row_plugin'] = 'fields';
  $handler->display->display_options['row_options']['inline'] = array(
    'render' => 'render',
    'name' => 'name',
  );
  $handler->display->display_options['row_options']['hide_empty'] = 0;
  /* Empty text: Global: Text area */
  $handler->display->display_options['empty']['area']['id'] = 'area';
  $handler->display->display_options['empty']['area']['table'] = 'views';
  $handler->display->display_options['empty']['area']['field'] = 'area';
  $handler->display->display_options['empty']['area']['empty'] = FALSE;
  $handler->display->display_options['empty']['area']['content'] = 'No messages created yet.';
  /* Relationship: Message: User */
  $handler->display->display_options['relationships']['uid']['id'] = 'uid';
  $handler->display->display_options['relationships']['uid']['table'] = 'message';
  $handler->display->display_options['relationships']['uid']['field'] = 'uid';
  $handler->display->display_options['relationships']['uid']['required'] = 0;
  /* Field: User: Name */
  $handler->display->display_options['fields']['name']['id'] = 'name';
  $handler->display->display_options['fields']['name']['table'] = 'users';
  $handler->display->display_options['fields']['name']['field'] = 'name';
  $handler->display->display_options['fields']['name']['relationship'] = 'uid';
  $handler->display->display_options['fields']['name']['label'] = '';
  $handler->display->display_options['fields']['name']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['name']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['name']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['name']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['name']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['name']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['name']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['name']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['name']['alter']['html'] = 0;
  $handler->display->display_options['fields']['name']['element_label_colon'] = 1;
  $handler->display->display_options['fields']['name']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['name']['hide_empty'] = 0;
  $handler->display->display_options['fields']['name']['empty_zero'] = 0;
  $handler->display->display_options['fields']['name']['link_to_user'] = 1;
  $handler->display->display_options['fields']['name']['overwrite_anonymous'] = 0;
  /* Field: Message: Show message */
  $handler->display->display_options['fields']['render']['id'] = 'render';
  $handler->display->display_options['fields']['render']['table'] = 'message';
  $handler->display->display_options['fields']['render']['field'] = 'render';
  $handler->display->display_options['fields']['render']['label'] = '';
  $handler->display->display_options['fields']['render']['alter']['alter_text'] = 0;
  $handler->display->display_options['fields']['render']['alter']['make_link'] = 0;
  $handler->display->display_options['fields']['render']['alter']['absolute'] = 0;
  $handler->display->display_options['fields']['render']['alter']['trim'] = 0;
  $handler->display->display_options['fields']['render']['alter']['nl2br'] = 0;
  $handler->display->display_options['fields']['render']['alter']['word_boundary'] = 1;
  $handler->display->display_options['fields']['render']['alter']['ellipsis'] = 1;
  $handler->display->display_options['fields']['render']['alter']['strip_tags'] = 0;
  $handler->display->display_options['fields']['render']['alter']['html'] = 0;
  $handler->display->display_options['fields']['render']['element_label_colon'] = 1;
  $handler->display->display_options['fields']['render']['element_default_classes'] = 1;
  $handler->display->display_options['fields']['render']['hide_empty'] = 0;
  $handler->display->display_options['fields']['render']['empty_zero'] = 0;
  $handler->display->display_options['fields']['render']['skip_access'] = 1;
  /* Sort criterion: Message: Message unique ID */
  $handler->display->display_options['sorts']['mid']['id'] = 'mid';
  $handler->display->display_options['sorts']['mid']['table'] = 'message';
  $handler->display->display_options['sorts']['mid']['field'] = 'mid';
  $handler->display->display_options['sorts']['mid']['order'] = 'DESC';
  /* Argument: Fields: field_node_ref (field_node_ref) - nid */
  $handler->display->display_options['arguments']['field_node_ref_nid']['id'] = 'field_node_ref_nid';
  $handler->display->display_options['arguments']['field_node_ref_nid']['table'] = 'field_data_field_node_ref';
  $handler->display->display_options['arguments']['field_node_ref_nid']['field'] = 'field_node_ref_nid';
  $handler->display->display_options['arguments']['field_node_ref_nid']['style_plugin'] = 'default_summary';
  $handler->display->display_options['arguments']['field_node_ref_nid']['default_argument_type'] = 'flag_user_flaggings';
  $handler->display->display_options['arguments']['field_node_ref_nid']['default_argument_options']['flag_name'] = TRUE;
  $handler->display->display_options['arguments']['field_node_ref_nid']['validate_fail'] = 'ignore';
  $handler->display->display_options['arguments']['field_node_ref_nid']['break_phrase'] = 0;
  $handler->display->display_options['arguments']['field_node_ref_nid']['not'] = 0;

  /* Display: Watched node content */
  $handler = $view->new_display('panel_pane', 'Watched node content', 'panel_pane_1');
  $handler->display->display_options['display_description'] = 'Node content a user flagged.';
  $handler->display->display_options['defaults']['arguments'] = FALSE;
  /* Argument: Fields: field_node_ref (field_node_ref) - nid */
  $handler->display->display_options['arguments']['field_node_ref_nid']['id'] = 'field_node_ref_nid';
  $handler->display->display_options['arguments']['field_node_ref_nid']['table'] = 'field_data_field_node_ref';
  $handler->display->display_options['arguments']['field_node_ref_nid']['field'] = 'field_node_ref_nid';
  $handler->display->display_options['arguments']['field_node_ref_nid']['default_action'] = 'default';
  $handler->display->display_options['arguments']['field_node_ref_nid']['style_plugin'] = 'default_summary';
  $handler->display->display_options['arguments']['field_node_ref_nid']['default_argument_type'] = 'flag_user_flaggings';
  $handler->display->display_options['arguments']['field_node_ref_nid']['default_argument_options']['flag_name'] = TRUE;
  $handler->display->display_options['arguments']['field_node_ref_nid']['break_phrase'] = 1;
  $handler->display->display_options['arguments']['field_node_ref_nid']['not'] = 0;

  /* Display: Watched user content */
  $handler = $view->new_display('panel_pane', 'Watched user content', 'panel_pane_2');
  $handler->display->display_options['display_description'] = 'User content a user flagged.';
  $handler->display->display_options['defaults']['arguments'] = FALSE;
  /* Argument: Fields: field_node_ref (field_node_ref) - nid */
  $handler->display->display_options['arguments']['field_node_ref_nid']['id'] = 'field_node_ref_nid';
  $handler->display->display_options['arguments']['field_node_ref_nid']['table'] = 'field_data_field_node_ref';
  $handler->display->display_options['arguments']['field_node_ref_nid']['field'] = 'field_node_ref_nid';
  $handler->display->display_options['arguments']['field_node_ref_nid']['default_action'] = 'default';
  $handler->display->display_options['arguments']['field_node_ref_nid']['style_plugin'] = 'default_summary';
  $handler->display->display_options['arguments']['field_node_ref_nid']['default_argument_type'] = 'flag_user_flaggings';
  $handler->display->display_options['arguments']['field_node_ref_nid']['default_argument_options']['flag_name'] = TRUE;
  $handler->display->display_options['arguments']['field_node_ref_nid']['break_phrase'] = 1;
  $handler->display->display_options['arguments']['field_node_ref_nid']['not'] = 0;
  $translatables['message_example'] = array(
    t('Defaults'),
    t('more'),
    t('Apply'),
    t('Reset'),
    t('Sort By'),
    t('Asc'),
    t('Desc'),
    t('Items per page'),
    t('- All -'),
    t('Offset'),
    t('No messages created yet.'),
    t('Message user'),
    t('All'),
    t('Watched node content'),
    t('Node content a user flagged.'),
    t('View panes'),
    t('Watched user content'),
    t('User content a user flagged.'),
  );

  $views[$view->name] = $view;

  return $views;
}
