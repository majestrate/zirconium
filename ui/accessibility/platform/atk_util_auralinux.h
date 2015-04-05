// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_ACCESSIBILITY_AX_UTIL_AURALINUX_H_
#define UI_ACCESSIBILITY_AX_UTIL_AURALINUX_H_

#include "base/memory/singleton.h"
#include "ui/accessibility/ax_export.h"

namespace ui {

// This singleton class initializes ATK (accessibility toolkit) and
// registers an implementation of the AtkUtil class, a global class that
// every accessible application needs to register once.
class AtkUtilAuraLinux {
 public:
  // Get the single instance of this class.
  static AtkUtilAuraLinux* GetInstance();

  AtkUtilAuraLinux();
  virtual ~AtkUtilAuraLinux();

 private:
  friend struct DefaultSingletonTraits<AtkUtilAuraLinux>;
};

}  // namespace ui

#endif  // UI_ACCESSIBILITY_AX_UTIL_AURALINUX_H_
