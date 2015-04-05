// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This module implements helper objects for the dialog, newwindow, and
// permissionrequest <webview> events.

var MessagingNatives = requireNative('messaging_natives');
var WebViewConstants = require('webViewConstants').WebViewConstants;
var WebViewInternal = require('webViewInternal').WebViewInternal;

var PERMISSION_TYPES = ['media',
                        'geolocation',
                        'pointerLock',
                        'download',
                        'loadplugin',
                        'filesystem',
                        'fullscreen'];

// -----------------------------------------------------------------------------
// WebViewActionRequest object.

// Default partial implementation of a webview action request.
function WebViewActionRequest(webViewImpl, event, webViewEvent, interfaceName) {
  this.webViewImpl = webViewImpl;
  this.event = event;
  this.webViewEvent = webViewEvent;
  this.interfaceName = interfaceName;
  this.guestInstanceId = this.webViewImpl.guest.getId();
  this.requestId = event.requestId;
  this.actionTaken = false;
}

// Performs the default action for the request.
WebViewActionRequest.prototype.defaultAction = function() {
  // Do nothing if the action has already been taken or the requester is
  // already gone (in which case its guestInstanceId will be stale).
  if (this.actionTaken ||
      this.guestInstanceId != this.webViewImpl.guest.getId()) {
    return;
  }

  this.actionTaken = true;
  WebViewInternal.setPermission(this.guestInstanceId, this.requestId,
                                'default', '', function(allowed) {
    if (allowed) {
      return;
    }
    this.showWarningMessage();
  }.bind(this));
};

// Called to handle the action request's event.
WebViewActionRequest.prototype.handleActionRequestEvent = function() {
  // Construct the interface object and attach it to |webViewEvent|.
  var request = this.getInterfaceObject();
  this.webViewEvent[this.interfaceName] = request;

  var defaultPrevented = !this.webViewImpl.dispatchEvent(this.webViewEvent);
  // Set |webViewEvent| to null to break the circular reference to |request| so
  // that the garbage collector can eventually collect it.
  this.webViewEvent = null;
  if (this.actionTaken) {
    return;
  }

  if (defaultPrevented) {
    // Track the lifetime of |request| with the garbage collector.
    MessagingNatives.BindToGC(request, this.defaultAction.bind(this));
  } else {
    this.defaultAction();
  }
};

// Displays a warning message when an action request is blocked by default.
WebViewActionRequest.prototype.showWarningMessage = function() {
  window.console.warn(this.WARNING_MSG_REQUEST_BLOCKED);
};

// This function ensures that each action is taken at most once.
WebViewActionRequest.prototype.validateCall = function() {
  if (this.actionTaken) {
    throw new Error(this.ERROR_MSG_ACTION_ALREADY_TAKEN);
  }
  this.actionTaken = true;
};

// The following are implemented by the specific action request.

// Returns the interface object for this action request.
WebViewActionRequest.prototype.getInterfaceObject = undefined;

// Error/warning messages.
WebViewActionRequest.prototype.ERROR_MSG_ACTION_ALREADY_TAKEN = undefined;
WebViewActionRequest.prototype.WARNING_MSG_REQUEST_BLOCKED = undefined;

// -----------------------------------------------------------------------------
// Dialog object.

// Represents a dialog box request (e.g. alert()).
function Dialog(webViewImpl, event, webViewEvent) {
  WebViewActionRequest.call(this, webViewImpl, event, webViewEvent, 'dialog');

  this.handleActionRequestEvent();
}

Dialog.prototype.__proto__ = WebViewActionRequest.prototype;

Dialog.prototype.getInterfaceObject = function() {
  return {
    ok: function(user_input) {
      this.validateCall();
      user_input = user_input || '';
      WebViewInternal.setPermission(
          this.guestInstanceId, this.requestId, 'allow', user_input);
    }.bind(this),
    cancel: function() {
      this.validateCall();
      WebViewInternal.setPermission(
          this.guestInstanceId, this.requestId, 'deny');
    }.bind(this)
  };
};

Dialog.prototype.showWarningMessage = function() {
  var VOWELS = ['a', 'e', 'i', 'o', 'u'];
  var dialogType = this.event.messageType;
  var article = (VOWELS.indexOf(dialogType.charAt(0)) >= 0) ? 'An' : 'A';
  this.WARNING_MSG_REQUEST_BLOCKED = this.WARNING_MSG_REQUEST_BLOCKED.
      replace('%1', article).replace('%2', dialogType);
  window.console.warn(this.WARNING_MSG_REQUEST_BLOCKED);
};

Dialog.prototype.ERROR_MSG_ACTION_ALREADY_TAKEN =
    WebViewConstants.ERROR_MSG_DIALOG_ACTION_ALREADY_TAKEN;
Dialog.prototype.WARNING_MSG_REQUEST_BLOCKED =
    WebViewConstants.WARNING_MSG_DIALOG_REQUEST_BLOCKED;

// -----------------------------------------------------------------------------
// NewWindow object.

// Represents a new window request.
function NewWindow(webViewImpl, event, webViewEvent) {
  WebViewActionRequest.call(this, webViewImpl, event, webViewEvent, 'window');

  this.handleActionRequestEvent();
}

NewWindow.prototype.__proto__ = WebViewActionRequest.prototype;

NewWindow.prototype.getInterfaceObject = function() {
  return {
    attach: function(webview) {
      this.validateCall();
      if (!webview || !webview.tagName || webview.tagName != 'WEBVIEW') {
        throw new Error(ERROR_MSG_WEBVIEW_EXPECTED);
      }

      var webViewImpl = privates(webview).internal;
      // Update the partition.
      if (this.event.partition) {
        webViewImpl.onAttach(this.event.partition);
      }

      var attached = webViewImpl.attachWindow(this.event.windowId);
      if (!attached) {
        window.console.error(ERROR_MSG_NEWWINDOW_UNABLE_TO_ATTACH);
      }

      if (this.guestInstanceId != this.webViewImpl.guest.getId()) {
        // If the opener is already gone, then its guestInstanceId will be
        // stale.
        return;
      }

      // If the object being passed into attach is not a valid <webview>
      // then we will fail and it will be treated as if the new window
      // was rejected. The permission API plumbing is used here to clean
      // up the state created for the new window if attaching fails.
      WebViewInternal.setPermission(this.guestInstanceId, this.requestId,
                                    attached ? 'allow' : 'deny');
    }.bind(this),
    discard: function() {
      this.validateCall();
      if (!this.guestInstanceId) {
        // If the opener is already gone, then we won't have its
        // guestInstanceId.
        return;
      }
      WebViewInternal.setPermission(
          this.guestInstanceId, this.requestId, 'deny');
    }.bind(this)
  };
};

NewWindow.prototype.ERROR_MSG_ACTION_ALREADY_TAKEN =
    WebViewConstants.ERROR_MSG_NEWWINDOW_ACTION_ALREADY_TAKEN;
NewWindow.prototype.WARNING_MSG_REQUEST_BLOCKED =
    WebViewConstants.WARNING_MSG_NEWWINDOW_REQUEST_BLOCKED;

// -----------------------------------------------------------------------------
// PermissionRequest object.

// Represents a permission request (e.g. to access the filesystem).
function PermissionRequest(webViewImpl, event, webViewEvent) {
  WebViewActionRequest.call(this, webViewImpl, event, webViewEvent, 'request');

  if (!this.validPermissionCheck()) {
    return;
  }

  this.handleActionRequestEvent();
}

PermissionRequest.prototype.__proto__ = WebViewActionRequest.prototype;

PermissionRequest.prototype.getInterfaceObject = function() {
  return {
    allow: function() {
      this.validateCall();
      WebViewInternal.setPermission(
          this.guestInstanceId, this.requestId, 'allow');
    }.bind(this),
    deny: function() {
      this.validateCall();
      WebViewInternal.setPermission(
          this.guestInstanceId, this.requestId, 'deny');
    }.bind(this)
  };
};

PermissionRequest.prototype.showWarningMessage = function() {
  window.console.warn(
      this.WARNING_MSG_REQUEST_BLOCKED.replace('%1', this.event.permission));
};

// Checks that the requested permission is valid. Returns true if valid.
PermissionRequest.prototype.validPermissionCheck = function() {
  if (PERMISSION_TYPES.indexOf(this.event.permission) < 0) {
    // The permission type is not allowed. Trigger the default response.
    this.defaultAction();
    return false;
  }
  return true;
};

PermissionRequest.prototype.ERROR_MSG_ACTION_ALREADY_TAKEN =
    WebViewConstants.ERROR_MSG_PERMISSION_ACTION_ALREADY_TAKEN;
PermissionRequest.prototype.WARNING_MSG_REQUEST_BLOCKED =
    WebViewConstants.WARNING_MSG_PERMISSION_REQUEST_BLOCKED;

// -----------------------------------------------------------------------------

// FullscreenPermissionRequest object.

// Represents a fullscreen permission request.
function FullscreenPermissionRequest(webViewImpl, event, webViewEvent) {
  PermissionRequest.call(this, webViewImpl, event, webViewEvent);
}

FullscreenPermissionRequest.prototype.__proto__ = PermissionRequest.prototype;

FullscreenPermissionRequest.prototype.getInterfaceObject = function() {
  return {
    allow: function() {
      this.validateCall();
      WebViewInternal.setPermission(
          this.guestInstanceId, this.requestId, 'allow');
      // Now make the <webview> element go fullscreen.
      this.webViewImpl.makeElementFullscreen();
    }.bind(this),
    deny: function() {
      this.validateCall();
      WebViewInternal.setPermission(
          this.guestInstanceId, this.requestId, 'deny');
    }.bind(this)
  };
};

var WebViewActionRequests = {
  WebViewActionRequest: WebViewActionRequest,
  Dialog: Dialog,
  NewWindow: NewWindow,
  PermissionRequest: PermissionRequest,
  FullscreenPermissionRequest: FullscreenPermissionRequest
};

// Exports.
exports.WebViewActionRequests = WebViewActionRequests;