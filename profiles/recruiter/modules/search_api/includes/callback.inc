<?php

/**
 * Interface representing a Search API data-alter callback.
 */
interface SearchApiAlterCallbackInterface {

  /**
   * Construct a data-alter callback.
   *
   * @param SearchApiIndex $index
   *   The index whose items will be altered.
   * @param array $options
   *   The callback options set for this index.
   */
  public function __construct(SearchApiIndex $index, array $options = array());

  /**
   * Display a form for configuring this callback.
   * Since forcing users to specify options for disabled callbacks makes no
   * sense, none of the form elements should have the '#required' attribute set.
   *
   * @return array
   *   A form array for configuring this callback, or FALSE if no configuration
   *   is possible.
   */
  public function configurationForm();

  /**
   * Validation callback for the form returned by configurationForm().
   *
   * @param array $form
   *   The form returned by configurationForm().
   * @param array $values
   *   The part of the $form_state['values'] array corresponding to this form.
   * @param array $form_state
   *   The complete form state.
   */
  public function configurationFormValidate(array $form, array &$values, array &$form_state);

  /**
   * Submit callback for the form returned by configurationForm().
   *
   * This method should both return the new options and set them internally.
   *
   * @param array $form
   *   The form returned by configurationForm().
   * @param array $values
   *   The part of the $form_state['values'] array corresponding to this form.
   * @param array $form_state
   *   The complete form state.
   *
   * @return array
   *   The new options array for this callback.
   */
  public function configurationFormSubmit(array $form, array &$values, array &$form_state);

  /**
   * Alter items before indexing.
   *
   * Items which are removed from the array won't be indexed, but will be marked
   * as clean for future indexing. This could for instance be used to implement
   * some sort of access filter for security purposes (e.g., don't index
   * unpublished nodes or comments).
   *
   * @param array $items
   *   An array of entities to be altered, keyed by entity ID.
   */
  public function alterItems(array &$items);

  /**
   * Declare the properties that are (or can be) added to items with this
   * callback. If a property with this name already exists for an entity it
   * will be overridden, so keep a clear namespace by prefixing the properties
   * with the module name if this is not desired.
   *
   * @see hook_entity_property_info()
   *
   * @return array
   *   Information about all additional properties, as specified by
   *   hook_entity_property_info() (only the inner "properties" array).
   */
  public function propertyInfo();

}

/**
 * Abstract base class for data-alter callbacks, implementing most methods with
 * sensible defaults.
 * Extending classes will at least have to implement the alterItems() method to
 * make this work. If that method adds additional fields to the items,
 * propertyInfo() has to be overridden, too.
 */
abstract class SearchApiAbstractAlterCallback implements SearchApiAlterCallbackInterface {

  /**
   * The index whose items will be altered.
   *
   * @var SearchApiIndex
   */
  protected $index;

  /**
   * The configuration options for this callback, if it has any.
   *
   * @var array
   */
  protected $options;

  /**
   * Construct a data-alter callback.
   *
   * @param SearchApiIndex $index
   *   The index whose items will be altered.
   * @param array $options
   *   The callback options set for this index.
   */
  public function __construct(SearchApiIndex $index, array $options = array()) {
    $this->index = $index;
    $this->options = $options;
  }

  /**
   * Display a form for configuring this callback.
   * Since forcing users to specify options for disabled callbacks makes no
   * sense, none of the form elements should have the '#required' attribute set.
   *
   * @return array
   *   A form array for configuring this callback, or FALSE if no configuration
   *   is possible.
   */
  public function configurationForm() {
    return array();
  }

  /**
   * Validation callback for the form returned by configurationForm().
   *
   * @param array $form
   *   The form returned by configurationForm().
   * @param array $values
   *   The part of the $form_state['values'] array corresponding to this form.
   * @param array $form_state
   *   The complete form state.
   */
  public function configurationFormValidate(array $form, array &$values, array &$form_state) { }

  /**
   * Submit callback for the form returned by configurationForm().
   *
   * This method should both return the new options and set them internally.
   *
   * @param array $form
   *   The form returned by configurationForm().
   * @param array $values
   *   The part of the $form_state['values'] array corresponding to this form.
   * @param array $form_state
   *   The complete form state.
   *
   * @return array
   *   The new options array for this callback.
   */
  public function configurationFormSubmit(array $form, array &$values, array &$form_state) {
    $this->options = $values;
    return $values;
  }

  /**
   * Declare the properties that are (or can be) added to items with this
   * callback. If a property with this name already exists for an entity it
   * will be overridden, so keep a clear namespace by prefixing the properties
   * with the module name if this is not desired.
   *
   * @see hook_entity_property_info()
   *
   * @return array
   *   Information about all additional properties, as specified by
   *   hook_entity_property_info() (only the inner "properties" array).
   */
  public function propertyInfo() {
    return array();
  }

}
