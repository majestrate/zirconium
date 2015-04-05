// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_WEB_CONTENTS_WEB_CONTENTS_VIEW_GUEST_H_
#define CONTENT_BROWSER_WEB_CONTENTS_WEB_CONTENTS_VIEW_GUEST_H_

#include <vector>

#include "base/memory/scoped_ptr.h"
#include "content/browser/renderer_host/render_view_host_delegate_view.h"
#include "content/browser/web_contents/web_contents_view.h"
#include "content/common/content_export.h"
#include "content/common/drag_event_source_info.h"

namespace content {

class WebContents;
class WebContentsImpl;
class BrowserPluginGuest;

class WebContentsViewGuest : public WebContentsView,
                             public RenderViewHostDelegateView {
 public:
  // The corresponding WebContentsImpl is passed in the constructor, and manages
  // our lifetime. This doesn't need to be the case, but is this way currently
  // because that's what was easiest when they were split.
  // WebContentsViewGuest always has a backing platform dependent view,
  // |platform_view|.
  WebContentsViewGuest(WebContentsImpl* web_contents,
                       BrowserPluginGuest* guest,
                       scoped_ptr<WebContentsView> platform_view,
                       RenderViewHostDelegateView* platform_view_delegate_view);
  ~WebContentsViewGuest() override;

  WebContents* web_contents();

  void OnGuestAttached(WebContentsView* parent_view);

  void OnGuestDetached(WebContentsView* old_parent_view);

  // Converts the guest specific coordinates in |params| to embedder specific
  // ones.
  ContextMenuParams ConvertContextMenuParams(
      const ContextMenuParams& params) const;

  // WebContentsView implementation --------------------------------------------
  gfx::NativeView GetNativeView() const override;
  gfx::NativeView GetContentNativeView() const override;
  gfx::NativeWindow GetTopLevelNativeWindow() const override;
  void GetContainerBounds(gfx::Rect* out) const override;
  void SizeContents(const gfx::Size& size) override;
  void Focus() override;
  void SetInitialFocus() override;
  void StoreFocus() override;
  void RestoreFocus() override;
  DropData* GetDropData() const override;
  gfx::Rect GetViewBounds() const override;
  void CreateView(const gfx::Size& initial_size,
                  gfx::NativeView context) override;
  RenderWidgetHostViewBase* CreateViewForWidget(
      RenderWidgetHost* render_widget_host,
      bool is_guest_view_hack) override;
  RenderWidgetHostViewBase* CreateViewForPopupWidget(
      RenderWidgetHost* render_widget_host) override;
  void SetPageTitle(const base::string16& title) override;
  void RenderViewCreated(RenderViewHost* host) override;
  void RenderViewSwappedIn(RenderViewHost* host) override;
  void SetOverscrollControllerEnabled(bool enabled) override;
#if defined(OS_MACOSX)
  void SetAllowOtherViews(bool allow) override;
  bool GetAllowOtherViews() const override;
  bool IsEventTracking() const override;
  void CloseTabAfterEventTracking() override;
#endif

  // Backend implementation of RenderViewHostDelegateView.
  void ShowContextMenu(RenderFrameHost* render_frame_host,
                       const ContextMenuParams& params) override;
  void StartDragging(const DropData& drop_data,
                     blink::WebDragOperationsMask allowed_ops,
                     const gfx::ImageSkia& image,
                     const gfx::Vector2d& image_offset,
                     const DragEventSourceInfo& event_info) override;
  void UpdateDragCursor(blink::WebDragOperation operation) override;
  void GotFocus() override;
  void TakeFocus(bool reverse) override;

 private:
  // The WebContentsImpl whose contents we display.
  WebContentsImpl* web_contents_;
  BrowserPluginGuest* guest_;
  // The platform dependent view backing this WebContentsView.
  // Calls to this WebContentsViewGuest are forwarded to |platform_view_|.
  scoped_ptr<WebContentsView> platform_view_;
  gfx::Size size_;

  // Delegate view for guest's platform view.
  RenderViewHostDelegateView* platform_view_delegate_view_;

  DISALLOW_COPY_AND_ASSIGN(WebContentsViewGuest);
};

}  // namespace content

#endif  // CONTENT_BROWSER_WEB_CONTENTS_WEB_CONTENTS_VIEW_GUEST_H_
