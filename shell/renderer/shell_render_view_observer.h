// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_SHELL_RENDERER_SHELL_RENDER_VIEW_OBSERVER_H_
#define SEATURTLE_SHELL_RENDERER_SHELL_RENDER_VIEW_OBSERVER_H_

#include "content/public/renderer/render_view_observer.h"

namespace blink {
class WebLocalFrame;
}

namespace content {
class RenderView;
}

namespace seaturtle {

class ShellRenderViewObserver : public ::content::RenderViewObserver {
 public:
  explicit ShellRenderViewObserver(::content::RenderView* render_view)
      : ::content::RenderViewObserver(render_view) {}
  virtual ~ShellRenderViewObserver() {}

  virtual void DidCreateDocumentElement(blink::WebLocalFrame* frame) OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(ShellRenderViewObserver);
};

}  // namespace seaturtle

#endif  // SEATURTLE_SHELL_RENDERER_SHELL_RENDER_VIEW_OBSERVER_H_
