// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/shell/browser/shell.h"

#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/web_contents.h"
#include "url/gurl.h"

#include "net/base/url_util.h"
#include "net/url_request/url_request.h"
#include "net/ssl/ssl_cipher_suite_names.h"
#include "net/ssl/ssl_connection_status_flags.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/resource_request_info.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/common/security_style.h"
#include "content/public/common/ssl_status.h"
#include "content/public/browser/navigation_details.h"
#include "content/public/browser/site_instance.h"
#include "content/public/common/favicon_url.h"
#include "content/public/common/web_preferences.h"

#include "seaturtle/extra/base.h"
#include "seaturtle/extra/settings.h"
#include "seaturtle/extra/shell_messages.h"
#include "seaturtle/extra/constants.h"
#include "seaturtle/extra/gfx_util.h"
#include "seaturtle/block/blocker.h"
#include "seaturtle/jni/jni_bridge.h"
#include "seaturtle/shell/browser/shell_browser_context.h"
#include "seaturtle/shell/browser/shell_content_browser_client.h"
#include "seaturtle/shell/browser/shell_javascript_dialog_manager.h"

using net::URLRequest;
using content::NavigationController;
using content::WebContents;
using content::JavaScriptDialogManager;
using content::NavigationEntry;
using content::ResourceRequestInfo;
using content::RenderProcessHost;
using content::RenderFrameHost;
using content::RenderViewHost;
using content::OpenURLParams;
using content::SiteInstance;
using content::WebPreferences;
using seaturtle::jni::Params;
using seaturtle::jni::LogEvent_Type;
using seaturtle::jni::ShellCommand;
using seaturtle::jni::ShellCommandResponse;
using seaturtle::jni::LaunchShellResponse;

namespace seaturtle {

// static
std::vector<Shell*> Shell::all_shells_;

Shell::Shell(WebContents* web_contents) :
  content::WebContentsObserver(web_contents),
  wc_(web_contents),
  si_(web_contents->GetSiteInstance()) {
  all_shells_.push_back(this);
}

Shell::~Shell() {
  SendCommand(ShellCommand::CLEANUP);
  for (size_t i = 0; i < all_shells_.size(); ++i) {
    if (all_shells_[i] == this) {
      all_shells_.erase(all_shells_.begin() + i);
      break;
    }
  }
}

// static
Params* Shell::LaunchShell(intptr_t parent) {
  ST_SETTINGS(settings);
  linked_ptr<ShellBrowserContext> owned_sbc;
  ShellBrowserContext* sbc;
  if (settings->state().single_context()) {
    sbc = ShellBrowserContext::Get();
  } else {
    owned_sbc.reset(new ShellBrowserContext());
    sbc = owned_sbc.get();
  }
  // Shell::CreateNewWindow
  // TODO(cy) is this how we really want to handle site instance?
  WebContents::CreateParams create_params(sbc, NULL);
  create_params.routing_id = MSG_ROUTING_NONE;
  Params* ret = CreateShell(
      WebContents::Create(create_params), parent, owned_sbc);
  return ret;
}

// static
Params* Shell::CreateShell(WebContents* web_contents,
    intptr_t parent, linked_ptr<ShellBrowserContext> sbc) {
  Params* ret = new Params();
  ret->set_type(Params::LAUNCH_SHELL_RESPONSE);
  LaunchShellResponse* resp = ret->mutable_launch_shell_response();
  // Shell::CreateShell
  Shell* shell = new Shell(web_contents);
  shell->parent_shell_ = parent;
  shell->sbc_ = sbc;
  resp->set_native_shell(reinterpret_cast<intptr_t>(shell));
  web_contents->SetDelegate(shell);
  resp->set_native_contents(reinterpret_cast<intptr_t>(web_contents));
  return ret;
}


// static
Shell* Shell::ForWebContents(const WebContents* wc) {
  for (size_t i = 0; i < all_shells_.size(); ++i) {
    if (all_shells_[i]->web_contents() == wc) {
      return all_shells_[i];
    }
  }
  return NULL;
}

// static
/*void Shell::UpdateAllRenderers() {
  // Instead see if we can override something that forces a new render
  // on the next page load. This will be usefull for transiet pref changes
  // (load without blocking).
  //Blocker* blocker = Blocker::Singleton();
  for (size_t i = 0; i < all_shells_.size(); ++i) {
    RenderProcessHost* rph =
      all_shells_[i]->web_contents()->GetRenderProcessHost();
    if (rph != NULL) {
      bool success = rph->FastShutdownIfPossible();
      STDCHECK(success);
    }
  }
}*/

// static
Shell* Shell::ForRenderProcessAndView(int render_process_id,
    int render_view_id) {
  RenderViewHost* rvh = RenderViewHost::FromID(
      render_process_id, render_view_id);
  if (rvh == NULL) {
    STLOG() << "missing render view host " << render_process_id
      << " " << render_view_id;
    return NULL;
  }
  return Shell::ForRenderViewHost(rvh);
}

// static
Shell* Shell::ForRenderProcessAndFrame(int render_process_id,
    int render_frame_id) {
  RenderFrameHost* rfh = RenderFrameHost::FromID(render_process_id,
      render_frame_id);
  if (rfh == NULL) {
    STLOG() << "missing render frame host " << render_process_id << " "
      << render_frame_id;
    return NULL;
  }
  return Shell::ForWebContents(WebContents::FromRenderFrameHost(rfh));
}

// static
Shell* Shell::ForURLRequest(const net::URLRequest& request) {
  const ResourceRequestInfo* rri = ResourceRequestInfo::ForRequest(&request);
  if (rri == NULL) {
    // STNOTREACHED() << "missing resource request info " << request.url();
    // TODO(cy) starting hitting this on wired.com. Since moving to Chrome 38.
    // The check fails when disqus tries to use a web socket.
    // Why is the network delegate getting these?
    // Have websockets been somehow reenabled?
    return NULL;
  }
  return ForRenderProcessAndView(rri->GetChildID(), rri->GetRouteID());
}

// static
Shell* Shell::ForRenderViewHost(const RenderViewHost* rvh) {
  return Shell::ForWebContents(WebContents::FromRenderViewHost(rvh));
}

// static
Shell* Shell::ForPtr(intptr_t ptr) {
  Shell* ret = reinterpret_cast<Shell*>(ptr);
  for (size_t i = 0; i < all_shells_.size(); ++i) {
    if (all_shells_[i] == ret) {
      return ret;
    }
  }
  return NULL;
}

// static
jni::Params* Shell::DispatchShellCommand(const jni::ShellCommand& sc) {
  Shell* s = ForPtr(sc.native_shell());
  // If this ever fires, return a code to java and dump the stack.
  CHECK(s != NULL) << "no shell for native address";
  return s->HandleShellCommand(sc);
}

// static
void Shell::UpdateAllWebPreferences() {
  for (size_t i = 0; i < all_shells_.size(); ++i) {
    all_shells_[i]->UpdateWebPreferences();
  }
}


ShellJavaScriptDialogManager* Shell::GetShellJavaScriptDialogManager() {
  if (jdm_.get() == NULL) {
    jdm_.reset(new ShellJavaScriptDialogManager());
  }
  return jdm_.get();
}

JavaScriptDialogManager* Shell::GetJavaScriptDialogManager() {
  return GetShellJavaScriptDialogManager();
}

void Shell::LoadingStateChanged(WebContents* source,
        bool to_different_document) {
  ShellCommand sc;
  sc.set_is_loading(source->IsLoading());
  SendCommand(ShellCommand::IS_LOADING, sc);
}

void Shell::LoadProgressChanged(WebContents* source, double progress) {
  ShellCommand sc;
  sc.set_progress(progress);
  SendCommand(ShellCommand::PROGRESS, sc);
}

void Shell::CloseContents(WebContents* source) {
  Close();
}

void Shell::Close() {
  SendCommand(ShellCommand::HIDE);
  delete this;
}

ShellCommandResponse* Shell::GetSSLStatus() {
  ShellCommandResponse* scr = new ShellCommandResponse();
  jni::SSLStatus* ss = scr->mutable_ssl_status();
  ss->set_is_insecure(true);
  // TODO(cy) we could probably send up the ssl state /w the current url,
  // just once when the url change is triggered?
  content::NavigationEntry* ne = wc_->GetController().GetLastCommittedEntry();
  if (ne == NULL) {
    ss->set_is_insecure(false);
    return scr;
  }
  if (ne->GetURL().scheme().compare(kKryptonScheme) == 0) {
    ss->set_is_insecure(false);
    return scr;
  }
  // TODO(cy) use constants and report to user whats going on here.
  const content::SSLStatus& stat = ne->GetSSL();
  if (stat.security_style != content::SECURITY_STYLE_AUTHENTICATED) {
    return scr;
  }
  if (stat.security_bits < 128) {
    return scr;
  }
  uint16 cipher_suite =
      net::SSLConnectionStatusToCipherSuite(stat.connection_status);
  if (!cipher_suite) {
    return scr;
  }
  int ssl_version = net::SSLConnectionStatusToVersion(stat.connection_status);
  if (ssl_version >= net::SSL_CONNECTION_VERSION_TLS1) {
    ss->set_is_insecure(false);
  }
  return scr;
}


ShellCommandResponse* Shell::GetVisibleUrl() {
  ShellCommandResponse* scr = new ShellCommandResponse();
  content::NavigationEntry* last = wc_->GetController().GetLastCommittedEntry();
  content::NavigationEntry* visible = wc_->GetController().GetVisibleEntry();
  if (last != visible && visible != NULL) {
    scr->set_url(visible->GetURL().spec());
  }
  return scr;
}

ShellCommandResponse* Shell::GetSSLDetails() {
  // TODO(cy) SSLStatus.content_status
  // TODO(cy) using criteria above report insecure stuff.
  ShellCommandResponse* scr = new ShellCommandResponse();
  jni::SSLDetails* sd = scr->mutable_ssl_details();
  content::NavigationEntry* ne = wc_->GetController().GetLastCommittedEntry();
  if (ne != NULL) {
    const content::SSLStatus& ssl = ne->GetSSL();
    sd->set_bits(ssl.security_bits);
    uint16 cipher_suite =
        net::SSLConnectionStatusToCipherSuite(ssl.connection_status);
    if (ssl.security_bits > 0 && cipher_suite) {
      int ssl_version =
        net::SSLConnectionStatusToVersion(ssl.connection_status);
      const char* ssl_version_str;
      net::SSLVersionToString(&ssl_version_str, ssl_version);
      const char *key_exchange, *cipher, *mac;
      bool is_aead;
      net::SSLCipherSuiteToStrings(&key_exchange, &cipher, &mac,
          &is_aead, cipher_suite);
      if (ssl_version_str) {
        sd->set_ssl_version(ssl_version_str);
      }
      if (key_exchange) {
        sd->set_key_exchange(key_exchange);
      }
      if (cipher) {
        sd->set_cipher(cipher);
      }
      if (mac) {
        sd->set_mac(mac);
      }
      sd->set_is_aead(is_aead);
    }
  }
  return scr;
}

void Shell::Reload(bool as_desktop) {
  NavigationEntry* active = wc_->GetController().GetActiveEntry();
  if (as_desktop && active != NULL) {
    Load(active->GetURL().spec(), true);
  } else {
    wc_->GetController().Reload(true);
  }
}

void Shell::Load(const std::string& surl, bool as_desktop) {
  GURL url(surl);
  if (!url.is_valid()) {
    url = net::AppendQueryParameter(GURL(kBadUrlUrl),
        kBadUrlParam, surl);
  }
  NavigationController::LoadURLParams params(url);
  if (as_desktop) {
    params.override_user_agent = NavigationController::UA_OVERRIDE_TRUE;
    wc_->SetUserAgentOverride(kUserAgentDesktop);
  } else {
    params.override_user_agent = NavigationController::UA_OVERRIDE_FALSE;
    wc_->SetUserAgentOverride("");
  }
  wc_->GetController().LoadURLWithParams(params);
}

jni::Params* Shell::HandleShellCommand(const ShellCommand& sc) {
  ShellCommandResponse* resp = NULL;
  switch (sc.command()) {
    case ShellCommand::CLOSE:
      Close();
      break;
    case ShellCommand::GET_PARENT:
      resp = new ShellCommandResponse();
      resp->set_parent_shell(parent_shell_);
      break;
    case ShellCommand::LOAD_URL:
      Load(sc.url(), sc.override_user_agent());
      break;
    case ShellCommand::RELOAD:
      Reload(sc.override_user_agent());
      break;
    case ShellCommand::GET_SSL_STATUS:
      resp = GetSSLStatus();
      break;
    case ShellCommand::GET_SSL_DETAILS:
      resp = GetSSLDetails();
      break;
    case ShellCommand::GET_LOG:
      resp = GetLog();
      break;
    case ShellCommand::GET_VISIBLE_URL:
      resp = GetVisibleUrl();
      break;
    case ShellCommand::DOWNLOAD_FAVICON:
      DownloadFavicon();
      break;
    default:
      // if this ever fires, return a code to java and dump the stack.
      STNOTREACHED() << "unknown shell command " << sc.command();
  }
  if (resp == NULL) {
    return NULL;
  }
  Params* p = new Params();
  p->set_type(Params::SHELL_COMMAND_RESPONSE);
  p->set_allocated_shell_command_response(resp);
  return p;
}

void Shell::AddNewContents(WebContents* source,
                           WebContents* new_contents,
                           WindowOpenDisposition disposition,
                           const gfx::Rect& initial_pos,
                           bool user_gesture,
                           bool* was_blocked) {
  // TODO(cy) this function is confusing as hell.
  // user_gesture is always true when
  // called with non numm was_blocked....
  // It looks like popupblocking can only occure on 'shouldcreatewebcontents'
  // override. Maybe use in conjunction with HandleGesture Begin/End
  // STLOG() << "AddNewContents user gesture: " << user_gesture
  //  << " was_blocked == NULL: " << (was_blocked == NULL);
  /*
  bool blocked = was_blocked != NULL && !user_gesture;
  SendBlockerEvent(BlockerEvent::POPUP, blocked,
      new_contents->GetURL().spec());
  if (blocked) {
    STLOG() << "AddNewContents blocked popup";
    *was_blocked = blocked;
    return;
  }
  */

  // TODO(cy) ugh... sort this out. Make it so we can open new contents
  // in our a separate context?
  linked_ptr<ShellBrowserContext> owned_sbc;
  {
    ST_SETTINGS(settings);
    if (!settings->state().single_context()) {
      owned_sbc = sbc_;
    }
  }

  Params* p = CreateShell(
      new_contents, reinterpret_cast<intptr_t>(this), owned_sbc);
  jni::Invoke(*p);
  delete p;
  SendCommand(ShellCommand::NOTIFY_POPUP_OPENED);
}

WebContents* Shell::OpenURLFromTab(WebContents* source,
                                   const OpenURLParams& params) {
  // TODO(cy)
  // This is verbatim from shell.cc. not sure wtf the deal is here.
  // CURRENT_TAB is the only one we implement for now.
  if (params.disposition != CURRENT_TAB) {
    STNOTREACHED() << "unexpected disposition: " << params.disposition;
  }
  NavigationController::LoadURLParams load_url_params(params.url);
  load_url_params.referrer = params.referrer;
  load_url_params.frame_tree_node_id = params.frame_tree_node_id;
  load_url_params.transition_type = params.transition;
  load_url_params.extra_headers = params.extra_headers;
  load_url_params.should_replace_current_entry =
      params.should_replace_current_entry;

  if (params.transferred_global_request_id != content::GlobalRequestID()) {
    load_url_params.is_renderer_initiated = params.is_renderer_initiated;
    load_url_params.transferred_global_request_id =
        params.transferred_global_request_id;
  } else if (params.is_renderer_initiated) {
    load_url_params.is_renderer_initiated = true;
  }

  source->GetController().LoadURLWithParams(load_url_params);
  return source;
}

bool Shell::OnMessageReceived(const IPC::Message& message,
                              RenderFrameHost* rfh) {
  // For some reason the compiler doesn't like IPC_BEGIN_MAP_WITH_PARAM macro
  /*bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(Shell, message)
  IPC_MESSAGE_HANDLER_GENERIC(SeaturtleMsg_NotifyDidCreateDocumentElement,
        OnNotifyDidCreateDocumentElement(rfh))
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;*/
  if (message.type() == SeaturtleMsg_NotifyDidCreateDocumentElement::ID) {
    OnNotifyDidCreateDocumentElement(rfh);
    return true;
  }
  return false;
}

void Shell::OnNotifyDidCreateDocumentElement(
    RenderFrameHost* rfh) {
  STLOG() << "on notify did create document element";
  {
    ST_SETTINGS(s);
    if (!s->filters().inject_css()) {
      return;
    }
  }
  Blocker* b = Blocker::Singleton();
  if (!b->IsWhitelisted(rfh->GetLastCommittedURL())) {
    rfh->Send(new SeaturtleMsg_InjectCssInFrame(rfh->GetRoutingID()));
  }
}

void Shell::DidNavigateMainFrame(const content::LoadCommittedDetails& details,
      const content::FrameNavigateParams& params) {
  if (details.is_navigation_to_different_page()) {
    base::AutoLock lock(log_lock_);
    log_.Clear();
  }
  ShellCommand sc;
  sc.set_url(params.url.spec());
  SendCommand(ShellCommand::UPDATE_VISIBLE_URL, sc);

  favicon_candidate_.reset(NULL);
}

void Shell::DidFinishLoad(content::RenderFrameHost* rfh,
                          const GURL& validated_url) {
  if (wc_->GetMainFrame() != rfh) {
    return;
  }
  ShellCommand sc;
  sc.mutable_did_finish_load()->set_url(validated_url.spec());
  sc.mutable_did_finish_load()->set_title(
      base::UTF16ToUTF8(wc_->GetTitle()));
  SendCommand(ShellCommand::DID_FINISH_LOAD, sc);
}

/*void Shell::TitleWasSet(NavigationEntry* entry, bool explicit_set) {
  if (!explicit_set || entry->GetPageType() != content::PAGE_TYPE_NORMAL) {
    return;
  }
  STLOG() << "title set: " << entry->GetTitle();
  Params p;
  p.set_type(Params::TITLE_UPDATE);
  TitleUpdate* tu = p.mutable_title_update();
  tu->set_url(entry->GetURL().spec());
  tu->set_title(base::UTF16ToUTF8(entry->GetTitle()));
  jni::Invoke(p);
}*/

void Shell::LogEvent(LogEvent_Type t, bool is_blocked,
    const std::string& desc) {
  {
    base::AutoLock lock(log_lock_);
    jni::LogEvent* le = log_.add_event();
    le->set_type(t);
    le->set_is_blocked(is_blocked);
    le->set_description(desc);
  }
  SendCommand(ShellCommand::NOTIFY_LOG_EVENT);
}

ShellCommandResponse* Shell::GetLog() {
  ShellCommandResponse* scr = new ShellCommandResponse();
  base::AutoLock lock(log_lock_);
  scr->mutable_log()->MergeFrom(log_);
  return scr;
}


void Shell::SendCommand(ShellCommand::Command c,
    const ShellCommand& sc) {
  Params p;
  p.set_type(Params::SHELL_COMMAND);
  ShellCommand* s = p.mutable_shell_command();
  s->MergeFrom(sc);
  s->set_native_shell(reinterpret_cast<intptr_t>(this));
  s->set_command(c);
  jni::Invoke(p);
}

void Shell::DidUpdateFaviconURL(
    const std::vector<content::FaviconURL>& candidates) {
  STLOG() << "did update favicon url";
  const GURL* best_candidate = NULL;
  int best_size_diff = INT_MAX;
  bool best_is_touch = false;
  // Note sometimes sizes don't come back. Use reddit as one test case.
  for (size_t i = 0; i < candidates.size(); i++) {
    const content::FaviconURL& f = candidates[i];
    STLOG() << "favicon candidate: " << f.icon_type << " "
      << f.icon_url;
    if (f.icon_type == content::FaviconURL::INVALID_ICON) {
      continue;
    }
    bool current_is_touch = f.icon_type == content::FaviconURL::TOUCH_ICON ||
        f.icon_type == content::FaviconURL::TOUCH_PRECOMPOSED_ICON;
    if (best_candidate == NULL) {
      best_candidate = &f.icon_url;
      best_is_touch = current_is_touch;
      continue;
    }
    if (!best_is_touch && current_is_touch) {
      best_candidate = &f.icon_url;
      best_is_touch = current_is_touch;
      continue;
    }
    if (best_is_touch && !current_is_touch) {
      continue;
    }
    for (size_t j = 0; j < f.icon_sizes.size(); j++) {
      const gfx::Size& s = f.icon_sizes[j];
      STLOG() << "favicon size: " << s.width() << "x" << s.height();
      int current_size_diff = abs(kIdealFaviconSize -
          ((s.width() + s.height()) / 2));
      if (current_size_diff < best_size_diff) {
        best_candidate = &f.icon_url;
        best_size_diff = current_size_diff;
        best_is_touch = current_is_touch;
      }
    }
  }
  if (best_candidate != NULL) {
    favicon_candidate_.reset(new GURL(*best_candidate));
  }
}

void Shell::DownloadFavicon() {
  STLOG() << "starting favicon download";
  GURL site_url = wc_->GetURL();
  GURL icon_url;
  if (favicon_candidate_.get() == NULL) {
    STLOG() << "no favicon candiate for page!";
    return;
  } else {
    // TODO(cy) I think we should be refcouting favicon_candidate_ to avoid
    // a NULL race.
    icon_url = GURL(*favicon_candidate_.get());
  }
  STLOG() << "downloading favicon: " << icon_url;
  WebContents::ImageDownloadCallback idc =
    base::Bind(&Shell::FaviconDownloaded, site_url);
  wc_->DownloadImage(icon_url, true, kIdealFaviconSize, idc);
}

void Shell::UpdateWebPreferences() {
  RenderViewHost* rvh = wc_->GetRenderViewHost();
  if (rvh == NULL) {
    STNOTREACHED() << "could not update prefs";
  }
  WebPreferences wp = rvh->GetWebkitPreferences();
  seaturtle::UpdateWebPreferences(&wp);
  rvh->UpdateWebkitPreferences(wp);
}

// static
void Shell::FaviconDownloaded(const GURL& site_url, int id, int status,
    const GURL& image_url, const std::vector<SkBitmap>& images,
    const std::vector<gfx::Size>& sizes) {
  STLOG() << "favicon downloaded! " << images.size();
  if (images.size() < 1) {
    return;
  }
  Params p;
  p.set_type(Params::FAVICON_DOWNLOADED);
  if (!SkBitmapToProto(images[0],
        p.mutable_favicon_downloaded()->mutable_image())) {
    STLOG() << "could not convert favicon";
  }
  p.mutable_favicon_downloaded()->set_url(site_url.spec());
  jni::Invoke(p);
}

}  // namespace seaturtle
