// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/apps/native_app_window_frame_view_mac.h"

#import <Cocoa/Cocoa.h>

#import "ui/gfx/mac/coordinate_conversion.h"
#include "ui/views/widget/widget.h"

NativeAppWindowFrameViewMac::NativeAppWindowFrameViewMac(views::Widget* frame)
    : views::NativeFrameView(frame) {
}

NativeAppWindowFrameViewMac::~NativeAppWindowFrameViewMac() {
}

gfx::Rect NativeAppWindowFrameViewMac::GetWindowBoundsForClientBounds(
    const gfx::Rect& client_bounds) const {
  NSWindow* ns_window = GetWidget()->GetNativeWindow();
  gfx::Rect window_bounds = gfx::ScreenRectFromNSRect([ns_window
      frameRectForContentRect:gfx::ScreenRectToNSRect(client_bounds)]);
  // Enforce minimum size (1, 1) in case that |client_bounds| is passed with
  // empty size.
  if (window_bounds.IsEmpty())
    window_bounds.set_size(gfx::Size(1, 1));
  return window_bounds;
}
