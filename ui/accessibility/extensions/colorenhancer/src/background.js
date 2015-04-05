// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * Adds filter script and css to all existing tabs.
 *
 * TODO(wnwen): Verify content scripts are not being injected multiple times.
 */
function injectContentScripts() {
  chrome.windows.getAll({'populate': true}, function(windows) {
    for (var i = 0; i < windows.length; i++) {
      var tabs = windows[i].tabs;
      for (var j = 0; j < tabs.length; j++) {
        var url = tabs[j].url;
        if (isDisallowedUrl(url)) {
          continue;
        }
        chrome.tabs.insertCSS(
            tabs[j].id,
            {file: 'res/cvd.css'});
        chrome.tabs.executeScript(
            tabs[j].id,
            {file: 'src/common.js'});
        chrome.tabs.executeScript(
            tabs[j].id,
            {file: 'src/cvd.js'});
      }
    }
  });
}

/**
 * Updates all existing tabs with config values.
 */
function updateTabs() {
  chrome.windows.getAll({'populate': true}, function(windows) {
    for (var i = 0; i < windows.length; i++) {
      var tabs = windows[i].tabs;
      for (var j = 0; j < tabs.length; j++) {
        var url = tabs[j].url;
        if (isDisallowedUrl(url)) {
          continue;
        }
        debugPrint('sending  to ' + siteFromUrl(url) + ' ' +
            getSiteDelta(siteFromUrl(url)) + ',' +
            getSiteSeverity(siteFromUrl(url)));
        var msg = {
          'delta': getSiteDelta(siteFromUrl(url)),
          'severity': getSiteSeverity(siteFromUrl(url)),
          'type': getDefaultType(),
          'simulate': getDefaultSimulate()
        };
        chrome.tabs.sendRequest(tabs[j].id, msg);
      }
    }
  });
}

/**
 * Initial extension loading.
 */
(function initialize() {
  injectContentScripts();
  updateTabs();

  chrome.extension.onRequest.addListener(
      function(request, sender, sendResponse) {
        if (request['init']) {
          var delta = getDefaultDelta();
          if (sender.tab) {
            delta = getSiteDelta(siteFromUrl(sender.tab.url));
          }

          var severity = getDefaultSeverity();
          if (sender.tab) {
            severity = getSiteSeverity(siteFromUrl(sender.tab.url));
          }

          var type = getDefaultType();
          var simulate = getDefaultSimulate();

          var msg = {
            'delta': delta,
            'severity': severity,
            'type': getDefaultType(),
            'simulate': getDefaultSimulate()
          };
          sendResponse(msg);
        }
      });

  document.addEventListener('storage', function(evt) {
    updateTabs();
  }, false);
})();
