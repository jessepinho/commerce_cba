<?php

/**
 * @file
 * Class for handling Checkout by Amazon IPNs.
 *
 * This class is a drastically cleaned-up version of Amazon's sample code for
 * handling Instant Order Processing Notifications.
 */

class CommerceCbaIpn {

  var $type;
  var $data;

  /**
   * Validate the IPN data.
   *
   * @return
   *   TRUE if the IPN data validates; FALSE otherwise.
   */
  function validate() {
    // If the basic data is not included, there's a problem.
    if (empty($_POST) || !isset($_POST['NotificationType'], $_POST['NotificationData'])) {
      watchdog('commerce_cba', '$_POST data missing.');
      return FALSE;
    }

    // Validate the notification type.
    $valid_types = array(
      'NewOrderNotification',
      'OrderCancelledNotification',
      'OrderReadyToShipNotification',
    );
    if (!in_array($_POST['NotificationType'], $valid_types)) {
      watchdog('commerce_cba', 'NotificationType was invalid.');
      return FALSE;
    }
    $this->type = $_POST['NotificationType'];

    $this->data = simplexml_load_string($_POST['NotificationData']);
    if (!$this->data) {
      watchdog('commerce_cba', 'simplexml_load_string() returned FALSE.');
      return FALSE;
    }

    return TRUE;
  }

  /**
   * Authenticate the IPN data.
   *
   * @return
   *   TRUE if the IPN data authenticates; FALSE otherwise.
   */
  function authenticate($secretKey) {
    // Don't bother authenticating if the cart doesn't need to be signed.
    if (empty($_POST['Signature'])) {
      return TRUE;
    }
    // If it should be signed, make sure all the necessary parameters have been
    // passed in.
    elseif (!isset(
      $_POST['UUID'],
      $_POST['Timestamp'],
      $_POST['AWSAccessKeyId']
    )) {
      watchdog('commerce_cba', 'Signed cart missing required parameters.');
      return FALSE;
    }

    // Validate the timestamp. It should be within the last 15 minutes, per
    // Amazon's API instructions.
    if (strtotime($_POST['Timestamp']) <= (REQUEST_TIME - (60 * 15))) {
      watchdog('commerce_cba', 'Timestamp not within last 15 minutes.');
      return FALSE;
    }

    // Validate the signature.
    $signature = $_POST['UUID'] . $_POST['Timestamp'];
    $signature = base64_encode(pack('H*', hash_hmac('sha1', $signature, $secretKey)));
	  if ($signature != $_POST['Signature']) {
      watchdog('commerce_cba', 'Signatures did not match.<br />Local signature: @local<br />Remote signature: @remote', array('@local' => $signature, '@remote' => $_POST['Signature']));
	    return FALSE;
	  }
	  return TRUE;
  }

}
