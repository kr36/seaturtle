// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/shell/renderer/shell_content_renderer_client.h"

#include "content/public/renderer/render_view.h"
#include "seaturtle/shell/renderer/shell_render_view_observer.h"
#include "seaturtle/shell/renderer/shell_render_frame_observer.h"

#include "third_party/WebKit/public/web/WebDocument.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/platform/WebURLError.h"

#include "seaturtle/extra/base.h"
#include "seaturtle/jni/jni_bridge.h"

using content::RenderView;
using content::RenderFrame;

namespace seaturtle {

void ShellContentRendererClient::RenderViewCreated(RenderView* render_view) {
  STLOG() << "render view created";
  new ShellRenderViewObserver(render_view);
}

void ShellContentRendererClient::RenderFrameCreated(RenderFrame* render_frame) {
  STLOG() << "render frame created";
  new ShellRenderFrameObserver(render_frame);
}

bool ShellContentRendererClient::HasErrorPage(int http_status_code,
    std::string* error_domain) {
  STLOG() << "has error page: " << http_status_code;
  *error_domain = "seaturtle-error";
  return true;
}

void ShellContentRendererClient::GetNavigationErrorStrings(
      RenderView* render_view,
      blink::WebFrame* frame,
      const blink::WebURLRequest& failed_request,
      const blink::WebURLError& error,
      std::string* error_html,
      base::string16* error_description) { 
  if (error_html != NULL) {
    jni::Params req;
    req.set_type(jni::Params::RENDER);
    jni::Render* r = req.mutable_render();
    r->set_type(jni::Render::ERROR);
    jni::Render::Error* re = r->mutable_error();
    re->set_url(GURL(failed_request.url()).spec());
    re->set_error_domain(error.domain.utf8());
    re->set_reason(error.reason);
    re->set_localized_description(error.localizedDescription.utf8());

    jni::Params resp;
    jni::Invoke(req, &resp);
    STDCHECK(resp.type() == jni::Params::RENDER_RESPONSE);
    error_html->assign(resp.render_response());
  }
}

}  // namespace seaturtle
