// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_SHELL_RENDERER_SHELL_CONTENT_RENDERER_CLIENT_H_
#define EXTENSIONS_SHELL_RENDERER_SHELL_CONTENT_RENDERER_CLIENT_H_

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/renderer/content_renderer_client.h"

namespace extensions {

class Dispatcher;
class DispatcherDelegate;
class ExtensionsClient;
class ShellExtensionsRendererClient;
class ShellRendererMainDelegate;

// Renderer initialization and runtime support for app_shell.
class ShellContentRendererClient : public content::ContentRendererClient {
 public:
  ShellContentRendererClient();
  ~ShellContentRendererClient() override;

  // content::ContentRendererClient implementation:
  void RenderThreadStarted() override;
  void RenderFrameCreated(content::RenderFrame* render_frame) override;
  void RenderViewCreated(content::RenderView* render_view) override;
  bool OverrideCreatePlugin(content::RenderFrame* render_frame,
                            blink::WebLocalFrame* frame,
                            const blink::WebPluginParams& params,
                            blink::WebPlugin** plugin) override;
  blink::WebPlugin* CreatePluginReplacement(
      content::RenderFrame* render_frame,
      const base::FilePath& plugin_path) override;
  bool ShouldForwardToGuestContainer(const IPC::Message& msg) override;
  bool WillSendRequest(blink::WebFrame* frame,
                       ui::PageTransition transition_type,
                       const GURL& url,
                       const GURL& first_party_for_cookies,
                       GURL* new_url) override;
  const void* CreatePPAPIInterface(const std::string& interface_name) override;
  bool IsExternalPepperPlugin(const std::string& module_name) override;
  bool ShouldEnableSiteIsolationPolicy() const override;
  content::BrowserPluginDelegate* CreateBrowserPluginDelegate(
      content::RenderFrame* render_frame,
      const std::string& mime_type,
      const GURL& original_url) override;

 protected:
  // app_shell embedders may need custom extensions client interfaces.
  // This class takes ownership of the returned object.
  virtual ExtensionsClient* CreateExtensionsClient();

 private:
  scoped_ptr<ExtensionsClient> extensions_client_;
  scoped_ptr<ShellExtensionsRendererClient> extensions_renderer_client_;
  scoped_ptr<DispatcherDelegate> extension_dispatcher_delegate_;
  scoped_ptr<Dispatcher> extension_dispatcher_;

  DISALLOW_COPY_AND_ASSIGN(ShellContentRendererClient);
};

}  // namespace extensions

#endif  // EXTENSIONS_SHELL_RENDERER_SHELL_CONTENT_RENDERER_CLIENT_H_
