// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/common/mojo/keep_alive.mojom.h"
#include "extensions/renderer/api_test_base.h"
#include "grit/extensions_renderer_resources.h"
#include "third_party/mojo/src/mojo/public/cpp/bindings/interface_impl.h"

// A test launcher for tests for the stash client defined in
// extensions/test/data/keep_alive_client_unittest.js.

namespace extensions {
namespace {

// A KeepAlive implementation that calls provided callbacks on creation and
// destruction.
class TestKeepAlive : public mojo::InterfaceImpl<KeepAlive> {
 public:
  explicit TestKeepAlive(const base::Closure& on_destruction)
      : on_destruction_(on_destruction) {}

  ~TestKeepAlive() override { on_destruction_.Run(); }

  static void Create(const base::Closure& on_creation,
                     const base::Closure& on_destruction,
                     mojo::InterfaceRequest<KeepAlive> keep_alive) {
    mojo::BindToRequest(new TestKeepAlive(on_destruction), &keep_alive);
    on_creation.Run();
  }

 private:
  const base::Closure on_destruction_;
};

}  // namespace

class KeepAliveClientTest : public ApiTestBase {
 public:
  KeepAliveClientTest() {}

  void SetUp() override {
    ApiTestBase::SetUp();
    service_provider()->AddService(
        base::Bind(&TestKeepAlive::Create,
                   base::Bind(&KeepAliveClientTest::KeepAliveCreated,
                              base::Unretained(this)),
                   base::Bind(&KeepAliveClientTest::KeepAliveDestroyed,
                              base::Unretained(this))));
    created_keep_alive_ = false;
    destroyed_keep_alive_ = false;
  }

  void WaitForKeepAlive() {
    // Wait for a keep-alive to be created and destroyed.
    while (!created_keep_alive_ || !destroyed_keep_alive_) {
      base::RunLoop run_loop;
      stop_run_loop_ = run_loop.QuitClosure();
      run_loop.Run();
    }
    EXPECT_TRUE(created_keep_alive_);
    EXPECT_TRUE(destroyed_keep_alive_);
  }

 private:
  void KeepAliveCreated() {
    created_keep_alive_ = true;
    if (!stop_run_loop_.is_null())
      stop_run_loop_.Run();
  }
  void KeepAliveDestroyed() {
    destroyed_keep_alive_ = true;
    if (!stop_run_loop_.is_null())
      stop_run_loop_.Run();
  }

  bool created_keep_alive_;
  bool destroyed_keep_alive_;
  base::Closure stop_run_loop_;

  DISALLOW_COPY_AND_ASSIGN(KeepAliveClientTest);
};

TEST_F(KeepAliveClientTest, KeepAliveWithSuccessfulCall) {
  RunTest("keep_alive_client_unittest.js", "testKeepAliveWithSuccessfulCall");
  WaitForKeepAlive();
}

TEST_F(KeepAliveClientTest, KeepAliveWithError) {
  RunTest("keep_alive_client_unittest.js", "testKeepAliveWithError");
  WaitForKeepAlive();
}

}  // namespace extensions
