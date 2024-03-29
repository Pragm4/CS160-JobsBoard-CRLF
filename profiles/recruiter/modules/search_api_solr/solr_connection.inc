<?php

/**
 * A few custom rewrites to the Apache_Solr_Service class, to allow providing
 * HTTP authentication and using this module without turning "allow_url_fopen"
 * on.
 *
 * Stolen from the apachesolr module for the most part.
 */
class SearchApiSolrConnection extends Apache_Solr_Service {

  /**
   * Authentication string (username + password) for HTTP authentication.
   */
  protected $http_auth;

  /**
   * Constructs a Solr connection with an optional HTTP user and password.
   *
   * @param array $options
   *   An array containing construction arguments.
   */
  public function __construct(array $options) {
    $options += array(
      'host' => 'localhost',
      'port' => 8983,
      'path' => '',
      'http_user' => NULL,
      'http_pass' => NULL,
    );
    parent::__construct($options['host'], $options['port'], $options['path']);
    if ($options['http_user'] && $options['http_pass']) {
      $this->http_auth = 'Basic ' . base64_encode($options['http_user'] . ':' . $options['http_pass']);
    }
    // Since /ping otherwise complains about missing default field.
    $this->_pingUrl .= '?q=id:1';
  }

  /**
   * Central method for making a get operation against this Solr Server.
   *
   * @see Apache_Solr_Service::_sendRawGet()
   */
  protected function _sendRawGet($url, $timeout = FALSE) {
    // Little "hack" to allow filter-only queries
    // Since "*:*" doesn't work with the dismax query handler, we mustn't set
    // "q", to let "q.alt" kick in. However, Apache_Solr_Service::search() will
    // always add "q", even if it is empty. Therefore, we delete empty "q"
    // parameters here.
    $url = preg_replace('/([?&])q=(&|$)/', '$1', $url);

    list($data, $headers) = $this->_makeHttpRequest($url, 'GET', array(), '', $timeout);
    $response = new Apache_Solr_Response($data, $headers, $this->_createDocuments, $this->_collapseSingleValueArrays);
    $code = (int) $response->getHttpStatus();
    if ($code != 200) {
      $message = $response->getHttpStatusMessage();
      if ($code >= 400 && $code != 403 && $code != 404) {
        // Add details, like Solr's exception message.
        $message .= $response->getRawResponse();
      }
      throw new Exception('"' . $code . '" Status: ' . $message);
    }
    return $response;
  }

  /**
   * Central method for making a post operation against this Solr Server.
   *
   * @see Apache_Solr_Service::_sendRawPost()
   */
  protected function _sendRawPost($url, $rawPost, $timeout = FALSE, $contentType = 'text/xml; charset=UTF-8') {
    $request_headers = array('Content-Type' => $contentType);
    list($data, $headers) = $this->_makeHttpRequest($url, 'POST', $request_headers, $rawPost, $timeout);
    $response = new Apache_Solr_Response($data, $headers, $this->_createDocuments, $this->_collapseSingleValueArrays);
    $code = (int) $response->getHttpStatus();
    if ($code != 200) {
      $message = $response->getHttpStatusMessage();
      if ($code >= 400 && $code != 403 && $code != 404) {
        // Add details, like Solr's exception message.
        $message .= $response->getRawResponse();
      }
      throw new Exception('"' . $code . '" Status: ' . $message);
    }
    return $response;
  }


  /**
   * Call the /admin/ping servlet, to test the connection to the server.
   *
   * @param $timeout
   *   maximum time to wait for ping in seconds, -1 for unlimited (default 2).
   * @return
   *   (float) seconds taken to ping the server, FALSE if timeout occurs.
   */
  public function ping($timeout = 2) {
    $start = microtime(TRUE);

    if ($timeout <= 0.0) {
      $timeout = -1;
    }
    // Attempt a HEAD request to the solr ping url.
    list($data, $headers) = $this->_makeHttpRequest($this->_pingUrl, 'HEAD', array(), NULL, $timeout);
    $response = new Apache_Solr_Response($data, $headers);

    if ($response->getHttpStatus() == 200) {
      // Add 0.1 ms to the ping time so we never return 0.0.
      return microtime(TRUE) - $start + 0.0001;
    }
    else {
      return FALSE;
    }
  }

  /**
   * Helper method for making an HTTP request, without using stupid stuff like
   * file_get_contents().
   */
  protected function _makeHttpRequest($url, $method = 'GET', $headers = array(), $content = '', $timeout = FALSE) {
    $options = array(
      'headers' => $headers,
      'method' => $method,
      'data' => $content,
    );

    if ($this->http_auth) {
      $options['headers']['Authorization'] = $this->http_auth;
    }
    if ($timeout) {
      $options['timeout'] = $timeout;
    }

    $result = drupal_http_request($url, $options);

    if (!isset($result->code) || $result->code < 0) {
      $result->code = 0;
      $result->status_message = 'Request failed';
      $result->protocol = 'HTTP/1.0';
    }
    // Additional information may be in the error property.
    if (isset($result->error)) {
      $result->status_message .= ': ' . check_plain($result->error);
    }

    if (!isset($result->data)) {
      $result->data = '';
    }
    // The headers have to be reformatted for the response class.
    $headers[] = "{$result->protocol} {$result->code} {$result->status_message}";
    if (isset($result->headers)) {
      foreach ($result->headers as $name => $value) {
        $headers[] = "$name: $value";
      }
    }
    return array($result->data, $headers);
  }

  /**
   * Convenience function for escaping a field name.
   *
   * Since field names can only contain one special character, ":", there is no
   * need to use the complete escape() method.
   *
   * @param string $value
   *   The field name to escape.
   *
   * @return string
   *   An escaped string suitable for passing to Solr.
   */
  static public function escapeFieldName($value) {
    $value = str_replace(':', '\:', $value);
    return $value;
  }

  /**
   * Convenience function for creating phrase syntax from a value.
   *
   * @param string $value
   *   The string to convert into a Solr phrase value.
   *
   * @return string
   *   A quoted string suitable for passing to Solr.
   */
  static public function phrase($value) {
    $value = str_replace("\\", "\\\\", $value);
    $value = str_replace('"', '\"', $value);
    return '"' . $value . '"';
  }

}
