// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_TEST_PLUGIN_PLUGIN_REQUEST_READ_TEST_H_
#define CONTENT_TEST_PLUGIN_PLUGIN_REQUEST_READ_TEST_H_

#include <string>
#include <vector>

#include "base/compiler_specific.h"
#include "content/test/plugin/plugin_test.h"

namespace NPAPIClient {

// Tests whether the browser correctly handles single range requests from NPAPI
// plugins.
class PluginRequestReadTest : public PluginTest {
 public:
  PluginRequestReadTest(NPP id, NPNetscapeFuncs* host_functions);
  ~PluginRequestReadTest() override;

  //
  // NPAPI Functions
  //
  NPError New(uint16 mode,
              int16 argc,
              const char* argn[],
              const char* argv[],
              NPSavedData* saved) override;
  NPError SetWindow(NPWindow* window) override;
  NPError NewStream(NPMIMEType type,
                    NPStream* stream,
                    NPBool seekable,
                    uint16* stream_type) override;
  NPError DestroyStream(NPStream* stream, NPError reason) override;
  int32 WriteReady(NPStream* stream) override;
  int32 Write(NPStream* stream, int32 offset, int32 len, void* buffer) override;

 private:
  // Tracks ranges, which we requested, but for which we did not get response.
  std::vector<NPByteRange> requested_ranges_;
  std::string url_to_request_;
  bool tests_started_;
  bool read_requested_;
};

}  // namespace NPAPIClient

#endif  // CONTENT_TEST_PLUGIN_PLUGIN_REQUEST_READ_TEST_H_

