// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/sessions/ios/ios_serialized_navigation_builder.h"

#include "components/sessions/serialized_navigation_entry.h"
#include "ios/web/public/favicon_status.h"
#include "ios/web/public/navigation_item.h"
#include "ios/web/public/referrer.h"

namespace sessions {

// static
SerializedNavigationEntry
IOSSerializedNavigationBuilder::FromNavigationItem(
    int index, const web::NavigationItem& item) {
  SerializedNavigationEntry navigation;
  navigation.index_ = index;
  navigation.unique_id_ = item.GetUniqueID();
  navigation.referrer_url_ = item.GetReferrer().url;
  navigation.referrer_policy_ = item.GetReferrer().policy;
  navigation.virtual_url_ = item.GetVirtualURL();
  navigation.title_ = item.GetTitle();
  navigation.transition_type_ = item.GetTransitionType();
  navigation.timestamp_ = item.GetTimestamp();
  if (item.GetFavicon().valid)
    navigation.favicon_url_ = item.GetFavicon().url;

  return navigation;
}

// static
scoped_ptr<web::NavigationItem>
IOSSerializedNavigationBuilder::ToNavigationItem(
    const SerializedNavigationEntry* navigation, int page_id) {
  scoped_ptr<web::NavigationItem> item(web::NavigationItem::Create());

  item->SetURL(navigation->virtual_url_);
  item->SetReferrer(web::Referrer(
      navigation->referrer_url_,
      static_cast<web::ReferrerPolicy>(navigation->referrer_policy_)));
  item->SetTitle(navigation->title_);
  item->SetPageID(page_id);
  item->SetTransitionType(ui::PAGE_TRANSITION_RELOAD);
  item->SetTimestamp(navigation->timestamp_);

  if (navigation->favicon_url_.is_valid()) {
    item->GetFavicon().url = navigation->favicon_url_;
  }

  return item.Pass();
}

}  // namespace sessions
