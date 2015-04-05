// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Changes to analytics reporting structures can have disruptive effects on the
// analytics history of Files.app (e.g. making it hard or impossible to detect
// trending).
//
// In general, treat changes to analytics like histogram changes, i.e. make
// additive changes, don't remove or rename existing Dimensions, Events, Labels,
// etc.
//
// Changes to this file will need to be reviewed by someone familiar with the
// analytics system.

// namespace
var metrics = metrics || metricsBase;

/** @enum {string} */
metrics.Categories = {
  ACQUISITION: 'Acquisition',
  INTERNALS: 'Internals'
};

/**
 * The values of these enums come from the analytics console.
 * @private @enum {number}
 */
metrics.Dimension_ = {
  CONSUMER_TYPE: 1,
  SESSION_TYPE: 2,
  MACHINE_USE: 3
};

/**
 * Enumeration of known FSPs used to qualify "provided" extensions
 * "screens" on analytics. All FSPs NOT present in this list
 * will be reported to analytics as 'provided-unknown'.
 *
 * NOTE: When an unknown provider is encountered, a separate event will be
 * sent to analytics with the id. Consulation of that event will provided
 * an indication when an extension is popular enough to be added to the
 * whitelist.
 *
 * @enum {string}
 */
metrics.FileSystemProviders = {
  oedeeodfidgoollimchfdnbmhcpnklnd: 'ZipUnpacker'
};

/**
 * Returns a new "screen" name for a provided file system type.
 * @param {string|undefined} extensionId The FSP provider extension ID.
 * @param {string} defaultName
 * @return {string} Name or undefined if extension is unrecognized.
 */
metrics.getFileSystemProviderName = function(extensionId, defaultName) {
  return metrics.FileSystemProviders[extensionId] || defaultName;
};

/**
 * @enum {!analytics.EventBuilder.Dimension}
 */
metrics.Dimensions = {
  CONSUMER_TYPE_READER: {
    index: metrics.Dimension_.CONSUMER_TYPE,
    value: 'Non-import'
  },
  CONSUMER_TYPE_IMPORTER: {
    index: metrics.Dimension_.CONSUMER_TYPE,
    value: 'Import'
  },
  SESSION_TYPE_NON_IMPORT: {
    index: metrics.Dimension_.SESSION_TYPE,
    value: 'Non-import'
  },
  SESSION_TYPE_IMPORT: {
    index: metrics.Dimension_.SESSION_TYPE,
    value: 'Import'
  },
  MACHINE_USE_SINGLE: {
    index: metrics.Dimension_.MACHINE_USE,
    value: 'Single'
  },
  MACHINE_USE_MULTIPLE: {
    index: metrics.Dimension_.MACHINE_USE,
    value: 'Multiple'
  }
};

// namespace
metrics.event = metrics.event || {};

/**
 * Base event builders for files app.
 * @private @enum {!analytics.EventBuilder}
 */
metrics.event.Builders_ = {
  IMPORT: analytics.EventBuilder.builder()
      .category(metrics.Categories.ACQUISITION),
  INTERNALS: analytics.EventBuilder.builder()
      .category(metrics.Categories.INTERNALS)
};

/** @enum {!analytics.EventBuilder} */
metrics.ImportEvents = {
  DEVICE_YANKED: metrics.event.Builders_.IMPORT
      .action('Device Yanked'),

  ERRORS: metrics.event.Builders_.IMPORT
      .action('Import Error Count'),

  FILES_DEDUPLICATED: metrics.event.Builders_.IMPORT
      .action('Files Deduplicated'),

  FILES_IMPORTED: metrics.event.Builders_.IMPORT
      .action('Files Imported'),

  HISTORY_LOADED: metrics.event.Builders_.IMPORT
      .action('History Loaded'),

  IMPORT_CANCELLED: metrics.event.Builders_.IMPORT
      .action('Import Cancelled'),

  MEGABYTES_IMPORTED: metrics.event.Builders_.IMPORT
      .action('Megabytes Imported'),

  STARTED: metrics.event.Builders_.IMPORT
      .action('Import Started')
      .dimension(metrics.Dimensions.SESSION_TYPE_IMPORT)
      .dimension(metrics.Dimensions.CONSUMER_TYPE_IMPORTER)
};

/** @enum {!analytics.EventBuilder} */
metrics.Internals = {
  UNRECOGNIZED_FILE_SYSTEM_PROVIDER: metrics.event.Builders_.INTERNALS
      .action('Unrecognized File System Provider')
};

// namespace
metrics.timing = metrics.timing || {};

/** @enum {string} */
metrics.timing.Variables = {
  COMPUTE_HASH: 'Compute Content Hash',
  SEARCH_BY_HASH: 'Search By Hash',
  HISTORY_LOAD: 'History Load'
};