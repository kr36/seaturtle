// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/shell/renderer/shell_render_view_observer.h"

#include "content/public/renderer/render_view.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_view_observer.h"
#include "third_party/WebKit/public/web/WebLocalFrame.h"

#include "seaturtle/extra/base.h"
#include "seaturtle/extra/shell_messages.h"

using content::RenderFrame;
using blink::WebLocalFrame;

namespace seaturtle {

void ShellRenderViewObserver::DidCreateDocumentElement(WebLocalFrame* frame) {
  STLOG() << "did create document element";
  Send(new SeaturtleMsg_NotifyDidCreateDocumentElement(
        RenderFrame::FromWebFrame(frame)->GetRoutingID()));
}

}  // namespace seaturtle
