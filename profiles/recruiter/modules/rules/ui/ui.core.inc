<?php

/**
 * @file Contains core ui functions.
 */

/**
 * Plugin UI Interface.
 */
interface RulesPluginUIInterface {

  /**
   * Adds the whole configuration form of this rules configuration. For rule
   * elements that are part of a configuration this method just adds the
   * elements configuration form.
   *
   * @param $form
   *   The form array where to add the form.
   * @param $form_state
   *   The current form state.
   * @param $options
   *   An optional array of options with the known keys:
   *    - 'show settings': Whether to include the 'settings' fieldset for
   *      editing configuration settings like the label or categories. Defaults
   *      to FALSE.
   *    - 'button': Whether a submit button should be added. Defaults to FALSE.
   *    - 'init': Whether the element is about to be configured the first time
   *      and the configuration is about to be initialized. Defaults to FALSE.
   *    - 'restrict plugins: May be used to restrict the list of rules plugins
   *      that may be added to this configuration. For that set an array of
   *      valid plugins. Note that conditions and actions are always valid, so
   *      just set an empty array for just allowing those.
   *    - 'restrict conditions': Optionally set an array of condition names to
   *      restrict the conditions that are available for adding.
   *    - 'restrict actions': Optionally set an array of action names to
   *      restrict the actions that are available to for adding.
   *    - 'restrict events': Optionally set an array of event names to restrict
   *      the events that are available for adding.
   */
  public function form(&$form, &$form_state, $options = array());

  /**
   * Validate the configuration form of this rule element.
   *
   * @param $form
   * @param $form_state
   */
  public function form_validate($form, &$form_state);

  /**
   * Submit the configuration form of this rule element. This makes sure to
   * put the updated configuration in the form state. For saving changes
   * permanently, just call $config->save() afterwards.
   *
   * @param $form
   * @param $form_state
   */
  public function form_submit($form, &$form_state);

  /**
   * Returns a structured array for rendering this element in overviews.
   */
  public function buildContent();

  /**
   * Returns the help text for editing this plugin.
   */
  public function help();

  /**
   * Returns ui operations for this element.
   */
  public function operations();

}

/**
 * Helper object for mapping elements to ids.
 */
class RulesElementMap {

  /**
   * @var RulesPlugin
   */
  protected $configuration;
  protected $index = array();
  protected $counter = 0;

  public function __construct(RulesPlugin $config) {
    $this->configuration = $config->root();
  }

  /**
   * Makes sure each element has an assigned id.
   */
  public function index() {
    foreach ($this->getUnIndexedElements($this->configuration) as $element) {
      $id = &$element->property('elementId');
      $id = ++$this->counter;
      $this->index[$id] = $element;
    }
  }

  protected function getUnIndexedElements($element, &$unindexed = array()) {
    // Remember unindexed elements.
    $id = $element->property('elementId');
    if (!isset($id)) {
      $unindexed[] = $element;
    }
    else {
      // Make sure $this->counter refers to the highest id.
      if ($id > $this->counter) {
        $this->counter = $id;
      }
      $this->index[$id] = $element;
    }
    // Recurse down the tree.
    if ($element instanceof RulesContainerPlugin) {
      foreach ($element as $child) {
        $this->getUnIndexedElements($child, $unindexed);
      }
    }
    return $unindexed;
  }

  /**
   * Looks up the element with the given id.
   */
  public function lookup($id) {
    if (!$this->index) {
      $this->index();
    }
    return isset($this->index[$id]) ? $this->index[$id] : FALSE;
  }
}

/**
 * Faces UI extender for all kind of Rules plugins. Provides various useful
 * methods for any rules UI.
 */
class RulesPluginUI extends FacesExtender implements RulesPluginUIInterface {

  /**
   * @var RulesPlugin
   */
  protected $element;

  /**
   * The base path determines where a Rules overview UI lives. All forms that
   * want to display Rules (overview) forms need to set this variable. This is
   * necessary in order to get correct operation links, paths, redirects, bread
   * crumbs etc. for the form() and overviewTable() methods.
   *
   * @see RulesUIController
   * @see rules_admin_reaction_overview()
   * @see rules_admin_components_overview()
   */
  public static $basePath = NULL;

  /**
   * Provide $this->element to make the code more meaningful.
   */
  public function __construct(FacesExtendable $object) {
    parent::__construct($object);
    $this->element = $object;
  }

  /**
   * Implements RulesPluginUIInterface.
   * Generates the element edit form.
   *
   * Note: Make sure that you set RulesPluginUI::$basePath before using this
   * method, otherwise paths, links, redirects etc. won't be correct.
   */
  public function form(&$form, &$form_state, $options = array()) {
    self::formDefaults($form, $form_state);
    $form_state += array('rules_element' => $this->element);
    // Make sure the element has been processed.
    $this->element->processSettings();

    // Add the help to the top of the form.
    $help = $this->element->help();
    $form['help'] = is_array($help) ? $help : array('#markup' => $help);

    // We use $form_state['element_settings'] to store the settings of both
    // parameter modes. That way one can switch between the parameter modes
    // without loosing the settings of those.
    $form_state += array('element_settings' => $this->element->settings);
    $settings = $this->element->settings + $form_state['element_settings'];

    $form['parameter'] = array(
      '#tree' => TRUE,
    );

    foreach ($this->element->pluginParameterInfo() as $name => $parameter) {
      if ($parameter['type'] == 'hidden') {
        continue;
      }

      $form['parameter'][$name] = array(
        '#type' => 'fieldset',
        '#title' => check_plain($parameter['label']),
        '#description' => filter_xss(isset($parameter['description']) ? $parameter['description'] : ''),
      );

      // Init the parameter input mode.
      $form_state['parameter_mode'][$name] = !isset($form_state['parameter_mode'][$name]) ? NULL : $form_state['parameter_mode'][$name];
      $form['parameter'][$name] += $this->getParameterForm($name, $parameter, $settings, $form_state['parameter_mode'][$name]);
    }

    // Provide a form for editing the label and name of provided variables.
    $settings = $this->element->settings;
    foreach ($this->element->pluginProvidesVariables() as $var_name => $var_info) {
      $form['provides'][$var_name] = array(
        '#type' => 'fieldset',
        '#title' => check_plain($var_info['label']),
        '#description' => filter_xss(isset($var_info['description']) ? $var_info['description'] : ''),
      );
      $form['provides'][$var_name]['label'] = array(
        '#type' => 'textfield',
        '#title' => t('Variable label'),
        '#default_value' => isset($settings[$var_name . ':label']) ? $settings[$var_name . ':label'] : $var_info['label'],
      );
      $form['provides'][$var_name]['var'] = array(
        '#type' => 'textfield',
        '#title' => t('Variable name'),
        '#default_value' => isset($settings[$var_name . ':var']) ? $settings[$var_name . ':var'] : $var_name,
        '#description' => t('The variable name must contain only lowercase letters, numbers, and underscores and must be unique in the current scope.'),
        '#element_validate' => array('rules_ui_element_machine_name_validate'),
      );
    }
    if (!empty($form['provides'])) {
      $help = '<div class="description">' . t('Adjust the names and labels of provided variables, but note that renaming of already utilizied variables invalidates the existing uses.') . '</div>';
      $form['provides'] += array(
        '#tree' => TRUE,
        '#prefix' => '<h4 class="rules-form-heading">' . t('Provided variables') . '</h4>' . $help,
      );
    }

    // Add settings form, if specified.
    if (!empty($options['show settings'])) {
      $this->settingsForm($form, $form_state);
    }
    // Add submit button, if specified.
    if (!empty($options['button'])) {
      $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Save'),
        '#weight' => 10,
      );
    }
  }

  /**
   * Actually generates the parameter form for the given data type.
   */
  protected function getParameterForm($name, $info, $settings, &$mode) {
    $class = $this->getDataTypeClass($info['type']);
    $supports_input_mode = in_array('RulesDataDirectInputFormInterface', class_implements($class));

    // Init the mode.
    if (!isset($mode)) {
      if (isset($settings[$name . ':select'])) {
        $mode = 'selector';
      }
      elseif (isset($settings[$name]) && $supports_input_mode) {
        $mode = 'input';
      }
      elseif (isset($info['restriction'])) {
        $mode = $info['restriction'];
      }
      else {
        $mode = call_user_func(array($class, 'getDefaultMode'));
      }
    }

    // Add the parameter form.
    if ($mode == 'input' && $supports_input_mode) {
      $form['settings'] = call_user_func(array($class, 'inputForm'), $name, $info, $settings, $this->element);
    }
    else {
      $form['settings'] = call_user_func(array($class, 'selectionForm'), $name, $info, $settings, $this->element);
    }

    // Add a link for switching the input mode when JS is enabled and a button
    // to switch it without javascript, in case switching is possible.
    if ($supports_input_mode && empty($info['restriction'])) {
      $value = $mode == 'selector' ? t('Switch to the direct input mode') : t('Switch to data selection');

      $form['switch_button'] = array(
        '#type' => 'submit',
        '#name' => 'param_' . $name,
        '#attributes' => array('class' => array('rules-switch-button')),
        '#parameter' => $name,
        '#value' => $value,
        '#submit' => array('rules_ui_parameter_replace_submit'),
        '#ajax' => rules_ui_form_default_ajax('none'),
        // Do not validate!
        '#limit_validation_errors' => array(),
      );
    }
    return $form;
  }

  /**
   * Implements RulesPluginUIInterface.
   */
  public function form_validate($form, &$form_state) {
    $this->form_extract_values($form, $form_state);

    if (isset($form_state['values']['provides'])) {
      $vars = $this->element->availableVariables();
      foreach ($form_state['values']['provides'] as $name => $values) {
        if (isset($vars[$values['var']])) {
          form_error($form['provides'][$name]['var'], t('The variable name %name is already taken.', array('%name' => $values['var'])));
        }
      }
    }

    // Enforce processing of the settings to ensure access for any applied input
    // evaluators and processors is checked now.
    $this->element->processSettings(TRUE, TRUE);

    if (!$this->element->access()) {
      form_set_error(NULL, t('Access violation!'));
    }
    try {
      $this->element->integrityCheck();
    }
    catch (RulesException $e) {
      form_set_error(implode('][', $e->keys), t('The integrity check failed with the following error message: "!message"', array('!message' => $e->getMessage())));
    }
    if (!empty($form['settings'])) {
      $this->settingsFormValidate($form, $form_state);
    }
  }

  /**
   * Applies the values of the form to the element.
   */
  public function form_extract_values($form, &$form_state) {
    $this->element->settings = array();
    if (isset($form_state['values']['parameter'])) {
      foreach ($form_state['values']['parameter'] as $name => $values) {
        $this->element->settings += $values['settings'];
      }
    }
    if (isset($form_state['values']['provides'])) {
      foreach ($form_state['values']['provides'] as $name => $values) {
        $this->element->settings[$name . ':label'] = $values['label'];
        $this->element->settings[$name . ':var'] = $values['var'];
      }
    }
    if (!empty($form['settings'])) {
      $this->settingsFormExtractValues($form, $form_state);
    }
  }

  /**
   * Implements RulesPluginUIInterface.
   */
  public function form_submit($form, &$form_state) {
    if (!empty($form['settings'])) {
      $this->settingsFormSubmit($form, $form_state);
    }
    $this->element->save();
    drupal_set_message(t('Your changes have been saved.'));
  }

  /**
   * Adds the configuration settings form (label, tags, description, ..).
   */
  public function settingsForm(&$form, &$form_state) {
    // Add the settings in a separate fieldset below.
    $form['settings'] = array(
      '#type' => 'fieldset',
      '#title' => t('Settings'),
      '#collapsible' => TRUE,
      '#collapsed' => empty($form_state['values']['vars']['more']),
      '#weight' => 5,
    );
    $form['settings']['label'] = array(
      '#type' => 'textfield',
      '#title' => t('Name'),
      '#default_value' => $this->element->label(),
      '#required' => TRUE,
      '#weight' => -5,
    );
    if (!empty($this->element->module) && !empty($this->element->name) && $this->element->module == 'rules' && strpos($this->element->name, 'rules_') === 0) {
      // Remove the Rules module prefix from the machine name.
      $machine_name = substr($this->element->name, strlen($this->element->module) + 1);
    }
    else {
      $machine_name = $this->element->name;
    }
    $form['settings']['name'] = array(
      '#type' => 'machine_name',
      '#default_value' => isset($machine_name) ? $machine_name : '',
      '#maxlength' => 32,
      '#disabled' => entity_has_status('rules_config', $this->element, ENTITY_IN_CODE) && !(isset($form_state['op']) && $form_state['op'] == 'clone'),
      '#machine_name' => array(
        'exists' => 'rules_config_load',
        'source' => array('settings', 'label'),
      ),
      '#required' => TRUE,
      '#description' => t('The machine-readable name of this configuration is used by rules internally to identify the configuration. This name must contain only lowercase letters, numbers, and underscores and must be unique.'),
    );

    // Show a form for editing variables for components.
    if (($plugin_info = $this->element->pluginInfo()) && !empty($plugin_info['component'])) {
      $description = $this->element->hasStatus(ENTITY_IN_CODE) ? t('Variables that are passed to the component when it is invoked. They can not be edited for configurations that are provided in code.') : t('Specify the variables to be passed to the component when it is invoked. For each variable you have to specify a certain data type, a label and a unique machine readable name containing only lowercase alphanumeric characters and underscores.');
      $form['settings']['vars'] = array(
        '#prefix' => '<div id="rules-component-variables">',
        '#suffix' => '</div>',
        '#tree' => TRUE,
        '#element_validate' => array('rules_ui_element_variable_form_validate'),
        '#theme' => 'rules_ui_variable_form',
        '#title' => t('Variables'),
        '#description' => $description,
        // Variables can not be edited on configurations in code.
        '#disabled' => $this->element->hasStatus(ENTITY_IN_CODE),
      );
      $weight = 0;
      foreach ($this->element->componentVariables() as $name => $var_info) {
        if (!(isset($var_info['parameter']) && $var_info['parameter'] == FALSE)) {
          $form['settings']['vars']['items'][$name] = RulesPluginUI::getVariableForm($name, $var_info);
          $form['settings']['vars']['items'][$name]['weight']['#default_value'] = $weight++;
        }
      }
      // Always add three empty forms.
      for ($i = 0; $i < 3; $i++) {
        $form['settings']['vars']['items'][$i] = RulesPluginUI::getVariableForm();
        $form['settings']['vars']['items'][$i]['weight']['#default_value'] = $weight++;
      }
      $form['settings']['vars']['more'] = array(
        '#type' => 'submit',
        '#value' => t('Add more'),
        // Enable AJAX once #756762 is fixed.
        // '#ajax' => rules_ui_form_default_ajax('none'),
        '#limit_validation_errors' => array(array('vars')),
        '#submit' => array('rules_form_submit_rebuild'),
      );
    }

    // TODO: Attach field form thus description and tags.
  }

  public function settingsFormExtractValues($form, &$form_state) {
    $this->element->label = $form_state['values']['label'];

    // If the name was changed we have to redirect to the URL that contains
    // the new name, instead of rebuilding on the old URL with the old name
    if ($form['settings']['name']['#default_value'] != $form_state['values']['name']) {
      $module = isset($this->element->module) ? $this->element->module : 'rules';
      $this->element->name = $module . '_' . $form_state['values']['name'];
      $form_state['redirect'] = RulesPluginUI::path($this->element->name);
    }

    if (isset($form_state['values']['vars']['items'])) {
      $vars = &$this->element->componentVariables();
      // Clear the variables and re-add all variables based on the form values.
      // However keep non-parameter variables as they don't appear in the form.
      foreach ($vars as $name => $info) {
        if (!isset($info['parameter']) || $info['parameter']) {
          unset($vars[$name]);
        }
      }
      usort($form_state['values']['vars']['items'], 'rules_element_sort_helper');
      foreach ($form_state['values']['vars']['items'] as $item) {
        if ($item['type'] && $item['name'] && $item['label']) {
          $vars[$item['name']] = array('label' => $item['label'], 'type' => $item['type']);
        }
      }
      // Disable FAPI persistence for the variable form so renumbering works.
      unset($form_state['input']['vars']);
    }
  }

  public function settingsFormValidate($form, &$form_state) {
    if ($form['settings']['name']['#default_value'] != $form_state['values']['name'] && rules_config_load($this->element->name)) {
      form_set_error('name', t('The machine-readable name %name is already taken.', array('%name' => $form_state['values']['name'])));
    }
  }

  public function settingsFormSubmit($form, &$form_state) {

  }

  /**
   * Returns the form for configuring the info of a single variable.
   */
  public function getVariableForm($name = '', $info = array()) {
    $form['type'] = array(
      '#type' => 'select',
      '#options' => array(0 => '--') + RulesPluginUI::getOptions('data_info'),
      '#default_value' => isset($info['type']) ? $info['type'] : 0,
    );
    $form['label'] = array(
      '#type' => 'textfield',
      '#size' => 40,
      '#default_value' => isset($info['label']) ? $info['label'] : '',
    );
    $form['name'] = array(
      '#type' => 'textfield',
      '#size' => 40,
      '#default_value' => $name,
      '#element_validate' => array('rules_ui_element_machine_name_validate'),
    );
    // Just set the weight #default_value for the returned form.
    $form['weight'] = array(
      '#type' => 'weight',
    );
    return $form;
  }

  /**
   * Returns the name of class for the given data type.
   *
   * @param $data_type
   *  The name of the data typ
   */
  public function getDataTypeClass($data_type) {
    $cache = rules_get_cache();
    $data_info = $cache['data_info'];
    return (is_string($data_type) && isset($data_info[$data_type]['ui class'])) ? $data_info[$data_type]['ui class'] : 'RulesDataUI';
  }

  /**
   * Implements RulesPluginUIInterface.
   * Show a preview of the configuration settings.
   */
  public function buildContent() {
    // Make sure the element has been processed.
    $this->element->processSettings();

    $config_name = $this->element->root()->name;
    $content['label'] = array(
      '#type' => 'link',
      '#title' => $this->element->label(),
      '#href' => $this->element->isRoot() ? RulesPluginUI::path($config_name) : RulesPluginUI::path($config_name, 'edit', $this->element),
      '#prefix' => '<div class="rules-element-label">',
      '#suffix' => '</div>'
    );
    // Put the elements below in a "description" div.
    $content['description'] = array(
      '#prefix' => '<div class="description">',
    );
    $content['description']['parameter'] = array(
      '#caption' => t('Parameter'),
      '#theme' => 'rules_content_group',
    );
    foreach ($this->element->pluginParameterInfo() as $name => $parameter) {
      $element = array();
      if (isset($this->element->settings[$name . ':select'])) {
        $element['content'] = array(
         '#markup' => '[' . $this->element->settings[$name . ':select'] . ']',
        );
      }
      elseif (isset($this->element->settings[$name]) && (!isset($parameter['default value']) || $parameter['default value'] != $this->element->settings[$name])) {
        $method = empty($parameter['options list']) ? 'render' : 'renderOptionsLabel';
        $element = call_user_func(array($this->getDataTypeClass($parameter['type']), $method), $this->element->settings[$name], $name, $parameter, $this->element);
      }
      // Only add parameters that are really configured / not default.
      if ($element) {
        $content['description']['parameter'][$name] = array(
          '#theme' => 'rules_parameter_configuration',
          '#info' => $parameter,
        ) + $element;
      }
    }
    foreach ($this->element->providesVariables() as $name => $var_info) {
      $content['description']['provides'][$name] = array(
        '#theme' => 'rules_provides_variable_view',
        '#info' => $var_info,
        '#name' => $name,
      );
    }
    if (!empty($content['description']['provides'])) {
      $content['description']['provides'] += array(
        '#caption' => t('Provides variables'),
        '#theme' => 'rules_content_group',
      );
    }

    $content['#suffix'] = '</div>';
    $content['#type'] = 'container';
    $content['#attributes']['class'][] = 'rules-element-content';
    return $content;
  }

  /**
   * Implements RulesPluginUIInterface.
   */
  public function operations() {
    $name = $this->element->root()->name;
    $render = array(
      '#theme' => 'links__rules',
    );
    $render['#attributes']['class'][] = 'rules-operations';
    $render['#attributes']['class'][] = 'action-links';
    $render['#links']['edit'] = array(
      'title' => t('edit'),
      'href' => RulesPluginUI::path($name, 'edit', $this->element),
    );
    $render['#links']['delete'] = array(
      'title' => t('delete'),
      'href' => RulesPluginUI::path($name, 'delete', $this->element),
    );
    return $render;
  }

  /**
   * Implements RulesPluginUIInterface.
   */
  public function help() {}


  /**
   * Deprecated by the controllers overviewTable() method.
   */
  public static function overviewTable($conditions = array(), $options = array()) {
    return rules_ui()->overviewTable($conditions, $options);
  }

  /**
   * Generates a path using the given operation for the element with the given
   * id of the configuration with the given name.
   */
  public static function path($name, $op = NULL, RulesPlugin $element = NULL, $parameter = FALSE) {
    $element_id = isset($element) ? $element->elementId() : FALSE;
    $base_path = isset(self::$basePath) ? self::$basePath : 'admin/config/workflow/rules';
    return implode('/', array_filter(array($base_path . '/manage', $name, $op, $element_id, $parameter)));
  }

  /**
   * Determines the default redirect target for an edited/deleted element. This
   * is a parent element which is either a rule or the configuration root.
   */
  public static function defaultRedirect(RulesPlugin $element) {
    while (!$element->isRoot()) {
      if ($element instanceof Rule) {
        return self::path($element->root()->name, 'edit', $element);
      }
      $element = $element->parentElement();
    }
    return self::path($element->name);
  }

  /**
   * Returns an array of options to use with a select for the items specified
   * in the given hook.
   *
   * @param $hook
   *   The hook to use to fetch data, or an array of data to use.
   */
  public static function getOptions($hook) {
    $sorted_data = array();
    $ungrouped = array();
    $data = !is_array($hook) ? rules_fetch_data($hook) : $hook;
    foreach ($data as $name => $info) {
      if (!empty($info['group'])) {
        $sorted_data[drupal_ucfirst($info['group'])][$name] = drupal_ucfirst($info['label']);
      }
      else {
        $ungrouped[$name] = drupal_ucfirst($info['label']);
      }
    }
    asort($ungrouped);
    foreach ($sorted_data as $key => $choices) {
      asort($choices);
      $sorted_data[$key] = $choices;
    }
    ksort($sorted_data);
    // Always move the 'Components' group down it it exists.
    if (isset($sorted_data[t('Components')])) {
      $copy = $sorted_data[t('Components')];
      unset($sorted_data[t('Components')]);
      $sorted_data[t('Components')] = $copy;
    }
    return $ungrouped + $sorted_data;
  }

  public static function formDefaults(&$form, &$form_state) {
    form_load_include($form_state, 'inc', 'rules', 'ui/ui.forms');
    // Add our own css.
    $form['#attached']['css'][] = drupal_get_path('module', 'rules') . '/ui/rules.ui.css';

    // Specify the wrapper div used by #ajax.
    $form['#prefix'] = '<div id="rules-form-wrapper">';
    $form['#suffix'] = '</div>';

    // Preserve the base path in the form state. The after build handler will
    // set self::$basePath again for cached forms.
    if (isset(self::$basePath)) {
      $form_state['_rules_base_path'] = RulesPluginUI::$basePath;
      $form['#after_build'][] = 'rules_form_after_build_restore_base_path';
    }
  }
}

/**
 * UI for abstract plugins (conditions & actions).
 */
class RulesAbstractPluginUI extends RulesPluginUI {

  /**
   * Overridden to invoke the abstract plugins form alter callback and to add
   * the negation checkbox for conditions.
   */
  public function form(&$form, &$form_state, $options = array()) {
    parent::form($form, $form_state, $options);

    if ($this->element instanceof RulesCondition) {
      $form['negate'] = array(
        '#title' => t('Negate'),
        '#type' => 'checkbox',
        '#description' => t('If checked, the condition result is negated such that it returns TRUE if it evaluates to FALSE.'),
        '#default_value' => $this->element->isNegated(),
        '#weight' => 5,
      );
    }
    $this->element->call('form_alter', array(&$form, &$form_state, $options));
  }

  public function form_extract_values($form, &$form_state) {
    parent::form_extract_values($form, $form_state);
    if ($this->element instanceof RulesCondition && isset($form_state['values']['negate'])) {
      $this->element->negate($form_state['values']['negate']);
    }
  }
}

/**
 * UI for Rules Container.
 */
class RulesContainerPluginUI extends RulesPluginUI {

  /**
   * Generates a table for editing the contained elements.
   */
  public function form(&$form, &$form_state, $options = array(), $iterator = NULL) {
    parent::form($form, $form_state, $options);
    $form['elements'] = array(
      // Hide during creation or for embedded elements.
      '#access' => empty($options['init']) && $this->element->isRoot(),
      '#tree' => TRUE,
      '#theme' => 'rules_elements',
      '#empty' => t('None'),
      '#caption' => t('Elements')
    );
    $form['elements']['#attributes']['class'][] = 'rules-container-plugin';

    // Recurse over all element childrens or use the provided iterator.
    $iterator = isset($iterator) ? $iterator : new RecursiveIteratorIterator($this->element, RecursiveIteratorIterator::SELF_FIRST);
    $root_depth = $this->element->depth();
    foreach ($iterator as $key => $child) {
      $id = $child->elementId();

      // Do not render rules as container element when displayed in a rule set.
      $is_container = $child instanceof RulesContainerPlugin && !($child instanceof Rule);
      $form['elements'][$id] = array(
        '#depth' => $child->depth() - $root_depth - 1,
        '#container' => $is_container,
      );
      $form['elements'][$id]['label'] = $child->buildContent();
      $form['elements'][$id]['weight'] = array(
        '#type' => 'weight',
        '#default_value' => $child->weight,
        '#delta' => 20,
      );
      $form['elements'][$id]['parent_id'] = array(
        '#type' => 'hidden',
        // If another iterator is passed in, the childs parent may not equal
        // the current element. Thus ask the child for its parent.
        '#default_value' => $child->parentElement()->elementId(),
      );
      $form['elements'][$id]['element_id'] = array(
        '#type' => 'hidden',
        '#default_value' => $id,
      );
      $form['elements'][$id]['operations'] = $child->operations();
    }

    // Alter the submit button label.
    if (!empty($options['button']) && !empty($options['init'])) {
      $form['submit']['#value'] = t('Continue');
    }
    elseif (!empty($options['button']) && $this->element->isRoot()) {
      $form['submit']['#value'] = t('Save changes');
    }
  }

  /**
   * Applies the values of the form to the given rule configuration.
   */
  public function form_extract_values($form, &$form_state) {
    parent::form_extract_values($form, $form_state);
    // Get the right form values.
    $values = $form_state['values'];
    foreach ($form['#parents'] as $key) {
      $values = isset($values[$key]) ? $values[$key] : NULL;
    }
    // Now apply the new hierarchy.
    if (isset($values['elements'])) {
      foreach ($values['elements'] as $id => $data) {
        $child = $this->element->elementMap()->lookup($id);
        $child->weight = $data['weight'];
        $parent = $this->element->elementMap()->lookup($data['parent_id']);
        $child->setParent($parent ? $parent : $this->element);
      }
      $this->element->sortChildren(TRUE);
    }
  }

  public function operations() {
    $ops = parent::operations();
    $add_ops = self::addOperations();
    $ops['#links'] += $add_ops['#links'];
    return $ops;
  }

  /**
   * Gets the Add-* operations for the given element.
   */
  public function addOperations() {
    $name = $this->element->root()->name;
    $render = array(
      '#theme' => 'links__rules',
    );
    $render['#attributes']['class'][] = 'rules-operations-add';
    $render['#attributes']['class'][] = 'action-links';
    foreach (rules_fetch_data('plugin_info') as $plugin => $info) {
      if (!empty($info['embeddable']) && $this->element instanceof $info['embeddable']) {
        $render['#links']['add_' . $plugin] = array(
          'title' => t('Add !name', array('!name' => $plugin)),
          'href' => RulesPluginUI::path($name, 'add', $this->element, $plugin),
        );
      }
    }
    return $render;
  }


  public function buildContent() {
    $content = parent::buildContent();
    // Don't link the title for embedded container plugins, except for rules.
    if (!$this->element->isRoot() && !($this->element instanceof Rule)) {
      $content['label']['#type'] = 'markup';
      $content['label']['#markup'] = check_plain($content['label']['#title']);
      unset($content['label']['#title']);
    }
    elseif ($this->element->isRoot()) {
      $content['description']['settings'] = array(
        '#theme' => 'rules_content_group',
        '#weight' => -4,
        'machine_name' => array(
          '#markup' => t('Machine name: ') . $this->element->name,
        ),
        'weight' => array(
          '#access' => $this->element instanceof RulesTriggerableInterface,
          '#markup' => t('Weight: ') . $this->element->weight,
        ),
      );
      if ($vars = $this->element->componentVariables()) {
        $content['description']['variables'] = array(
          '#caption' => t('Variables'),
          '#theme' => 'rules_content_group',
        );
        foreach ($vars as $name => $info) {
          $content['description']['variables'][$name] = array(
            '#theme' => 'rules_provides_variable_view',
            '#info' => $info,
            '#name' => $name,
          );
        }
      }
    }
    return $content;
  }
}

/**
 * UI for Rules condition container.
 */
class RulesConditionContainerUI extends RulesContainerPluginUI {

  public function form(&$form, &$form_state, $options = array(), $iterator = NULL) {
    parent::form($form, $form_state, $options, $iterator);
    // Add the add-* operation links.
    $form['elements']['#add'] = self::addOperations();
    $form['elements']['#attributes']['class'][] = 'rules-condition-container';
    $form['elements']['#caption'] = t('Conditions');

    // By default skip
    if (!empty($options['init']) && !$this->element->isRoot()) {
      $config = $this->element->root();
      $form['init_help'] = array(
        '#type' => 'container',
        '#id' => 'rules-plugin-add-help',
        'content' => array(
          '#markup' => t('You are about to add a new @plugin to the @config-plugin %label.', array('@plugin' => $this->element->plugin(), '@config-plugin' => $config->plugin(), '%label' => $config->label())),
        ),
      );
    }
    $form['negate'] = array(
      '#title' => t('Negate'),
      '#type' => 'checkbox',
      '#description' => t('If checked, the condition result is negated such that it returns TRUE if it evaluates to FALSE.'),
      '#default_value' => $this->element->isNegated(),
      '#weight' => 5,
    );
  }

  public function form_extract_values($form, &$form_state) {
    parent::form_extract_values($form, $form_state);
    if (isset($form_state['values']['negate'])) {
      $this->element->negate($form_state['values']['negate']);
    }
  }
}

/**
 * UI for Rules action container.
 */
class RulesActionContainerUI extends RulesContainerPluginUI {

  public function form(&$form, &$form_state, $options = array(), $iterator = NULL) {
    parent::form($form,  $form_state, $options, $iterator);
    // Add the add-* operation links.
    $form['elements']['#add'] = self::addOperations();
    $form['elements']['#attributes']['class'][] = 'rules-action-container';
    $form['elements']['#caption'] = t('Actions');
  }

  public function settingsForm(&$form, &$form_state) {
    parent::settingsForm($form, $form_state);

    $vars = $this->element->call('stateVariables', array(TRUE));
    $vars = array_diff_key($vars, RulesState::defaultVariables());
    $form['settings']['component_provides'] = array(
      '#access' => !empty($vars),
      '#type' => 'checkboxes',
      '#options' => rules_extract_property($vars, 'label'),
      '#default_value' => $this->element->componentProvidesVariables(),
      '#title' => t('Provided variables'),
      '#description' => $this->element->hasStatus(ENTITY_IN_CODE) ? t('Variables that are provided to the caller. They can not be edited for configurations that are provided in code.') : t('Select the variables that should be provided to the caller.'),
      // Variables can not be edited on configurations in code.
      '#disabled' => $this->element->hasStatus(ENTITY_IN_CODE),
    );
  }

  public function settingsFormExtractValues($form, &$form_state) {
    parent::settingsFormExtractValues($form, $form_state);

    if (isset($form_state['values']['component_provides'])) {
      $vars = &$this->element->componentProvidesVariables();
      $vars = array_keys(array_filter($form_state['values']['component_provides']));
    }
  }
}
