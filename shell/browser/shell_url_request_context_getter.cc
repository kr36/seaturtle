// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/shell/browser/shell_url_request_context_getter.h"

#include "base/file_util.h"
#include "base/logging.h"
#include "base/lazy_instance.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/threading/sequenced_worker_pool.h"
#include "base/threading/worker_pool.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/cookie_store_factory.h"
#include "content/public/common/url_constants.h"
#include "net/base/cache_type.h"
#include "net/cert/cert_verifier.h"
#include "net/cookies/cookie_monster.h"
#include "net/dns/host_resolver.h"
#include "net/dns/mapped_host_resolver.h"
#include "net/http/http_auth_handler_factory.h"
#include "net/http/http_cache.h"
#include "net/http/http_network_session.h"
#include "net/http/http_server_properties_impl.h"
#include "net/http/transport_security_state.h"
#include "net/http/transport_security_persister.h"
#include "net/proxy/proxy_service.h"
#include "net/url_request/static_http_user_agent_settings.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_storage.h"
#include "net/url_request/url_request_job_factory_impl.h"
#include "net/base/sdch_manager.h"
#include "net/disk_cache/disk_cache.h"

#include "seaturtle/extra/base.h"
#include "seaturtle/extra/settings.h"
#include "seaturtle/extra/constants.h"
#include "seaturtle/extra/net_log.h"
#include "seaturtle/extra/proxy_config_service.h"
#include "seaturtle/shell/browser/shell_content_browser_client.h"
#include "seaturtle/shell/browser/shell_network_delegate.h"
#include "seaturtle/extra/seaturtle_protocol_handler.h"
#include "seaturtle/extra/ssl_config_service.h"
#include "seaturtle/extra/process_cache.h"
#include "seaturtle/extra/split_cookie_store.h"

using net::SdchManager;
using net::HttpAuthHandlerFactory;
using net::HttpServerProperties;
using net::HttpUserAgentSettings;
using net::URLRequestJobFactory;
using net::NetworkDelegate;
using net::HostResolver;
using net::CookieStore;
using net::TransportSecurityState;
using net::TransportSecurityPersister;
using net::ProxyService;
using net::HttpCache;
using content::BrowserThread;
using seaturtle::settings::AllSettings;

namespace seaturtle {

namespace {

// TODO(cy) without the StorageContext dance, we only construct the
// urlrequestcontext getter once, (or once per browser context in the case of
// krypton x), so this isn't really nessecary. Maybe get rid of it.
class SharedState {
 public:
  scoped_ptr<NetworkDelegate> network_delegate_;
  NetworkDelegate* GetNetworkDelegate() {
    if (network_delegate_.get() == NULL) {
      network_delegate_.reset(new ShellNetworkDelegate());
    }
    return network_delegate_.get();
  }

  scoped_refptr<SplitCookieStore> cookie_store_;
  SplitCookieStore* GetCookieStore(const base::FilePath& path) {
    if (cookie_store_.get() == NULL) {
      STDCHECK(!path.empty());
      base::FilePath cookie_path = path.Append(
          FILE_PATH_LITERAL(kStateDir)).Append(
          FILE_PATH_LITERAL(kCookieStore));
      STLOG() << "initializing persistant cookie store in "
        << cookie_path.value();
      cookie_store_ = new SplitCookieStore(cookie_path);
    }
    return cookie_store_.get();
  }


  SplitCookieStore* MaybeGetCookieStore() {
    return cookie_store_.get();
  }

  scoped_ptr<TransportSecurityState> tss_;
  scoped_ptr<TransportSecurityPersister> tsp_;
  TransportSecurityState* GetTransportSecurityState(
      const base::FilePath& path) {
    STDCHECK(!path.empty());
    if (tss_.get() == NULL) {
      tss_.reset(new TransportSecurityState());
      base::FilePath hsts_path = path.Append(
          FILE_PATH_LITERAL(kStateDir)).Append(
          FILE_PATH_LITERAL(kHstsDir));
      STLOG() << "initializing hsts db: " << hsts_path.value();
      bool success = base::CreateDirectory(hsts_path);
      STDCHECK(success);
      // TODO(cy) keep a close eye on hsts. The latest version disables
      // public key pins, also turns itself off after ~10weeks.
      // TODO(cy) trim old entries from persisted db.
      tsp_.reset(new TransportSecurityPersister(
                  tss_.get(),
                  hsts_path,
                  BrowserThread::GetMessageLoopProxyForThread(
                    BrowserThread::FILE).get(), false));  // readonly
    }
    return tss_.get();
  }

  TransportSecurityState* MaybeGetTransportSecurityState() {
    return tss_.get();
  }

  scoped_ptr<HttpCache> http_cache_;
  HttpCache* GetHttpCache(const base::FilePath& path,
      int cache_size, const net::HttpNetworkSession::Params& params) {
      STDCHECK(!path.empty());
    if (http_cache_.get() == NULL) {
      base::FilePath cache_path = path.Append(
          FILE_PATH_LITERAL(kStateDir)).Append(
          FILE_PATH_LITERAL(kCacheDir));
      STLOG() << "initializing disk cache " << cache_size << " "
        << cache_path.value();
      net::HttpCache::BackendFactory* main_backend =
          new net::HttpCache::DefaultBackend(
              net::DISK_CACHE,
              // WATCH keep an eye on what aw_settings et al uses.
              net::CACHE_BACKEND_SIMPLE,
              cache_path,
              cache_size,
              BrowserThread::GetMessageLoopProxyForThread(
                BrowserThread::CACHE).get());
      http_cache_.reset(new net::HttpCache(params, main_backend));
    }
    return http_cache_.get();
  }


  HttpCache* MaybeGetHttpCache() {
    return http_cache_.get();
  }

/*    scoped_ptr<ServerBoundCertService> cert_service_;
  ServerBoundCertService* GetCertService() {
    if (cert_service_.get() == NULL) {
      cert_service_.reset(new net::ServerBoundCertService(
        new net::DefaultServerBoundCertStore(NULL),
        base::WorkerPool::GetTaskRunner(true)));
    }
    return cert_service_.get();
  }*/

  scoped_refptr<SSLConfigService> ssl_config_service_;
  SSLConfigService* GetSSLConfigService() {
    if (ssl_config_service_.get() == NULL) {
      ssl_config_service_ = new seaturtle::SSLConfigService();
    }
    return ssl_config_service_.get();
  }

  scoped_ptr<HostResolver> host_resolver_;
  HostResolver* GetHostResolver() {
    if (host_resolver_.get() == NULL) {
      host_resolver_ =  HostResolver::CreateDefaultResolver(NetLog::Get());
    }
    return host_resolver_.get();
  }

  scoped_ptr<HttpAuthHandlerFactory> auth_factory_;
  HttpAuthHandlerFactory* GetAuthFactory(HostResolver* host_resolver) {
    if (auth_factory_.get() == NULL) {
      auth_factory_.reset(
          HttpAuthHandlerFactory::CreateDefault(host_resolver));
    }
    return auth_factory_.get();
  }

  scoped_ptr<HttpServerProperties> hsp_;
  base::WeakPtr<HttpServerProperties> GetHttpServerProperties() {
    if (hsp_.get() == NULL) {
      hsp_.reset(new net::HttpServerPropertiesImpl());
    }
    return hsp_->GetWeakPtr();
  }

  scoped_ptr<ProxyService> proxy_service_;
  ProxyService* GetProxyService() {
    if (proxy_service_.get() == NULL) {
      proxy_service_.reset(ProxyService::CreateWithoutProxyResolver(
          new ProxyConfigService(), NetLog::Get()));
    }
    return proxy_service_.get();
  }

  scoped_ptr<HttpUserAgentSettings> ua_settings_;
  HttpUserAgentSettings* GetHttpUserAgentSettings() {
    if (ua_settings_.get() == NULL) {
      ua_settings_.reset(new net::StaticHttpUserAgentSettings(
        net::HttpUtil::GenerateAcceptLanguageHeader(
            ProcessCache::Singleton()->GetAcceptLangs()),
        std::string()));
    }
    return ua_settings_.get();
  }

  scoped_ptr<URLRequestJobFactory> urjf_;
  URLRequestJobFactory* GetJobFactory() {
    if (urjf_.get() == NULL) {
      net::URLRequestJobFactoryImpl* urjf =
          new net::URLRequestJobFactoryImpl();
      CHECK(urjf->SetProtocolHandler(kKryptonScheme,
        new SeaturtleProtocolHandler()));
      urjf_.reset(urjf);
    }
    return urjf_.get();
  }

  scoped_ptr<SdchManager> sdch_;
  SdchManager* GetSdch() {
    if (sdch_.get() == NULL) {
      sdch_.reset(new SdchManager());
    }
    return sdch_.get();
  }
};

base::LazyInstance<SharedState> shared_singleton = LAZY_INSTANCE_INITIALIZER;

}  // namespace


ShellURLRequestContextGetter::ShellURLRequestContextGetter() {
  // This is mostly for the fetcher.
  STDCHECK_ON_THREAD(IO);
}

ShellURLRequestContextGetter::ShellURLRequestContextGetter(
    const base::FilePath& base_path)
    : base_path_(base_path) {
  STDCHECK_ON_THREAD(UI);
}

ShellURLRequestContextGetter::~ShellURLRequestContextGetter() {
  STLOG() << "context getter destroyed";
}

// static
SplitCookieStore* ShellURLRequestContextGetter::GetSplitCookieStore() {
  return shared_singleton.Pointer()->GetCookieStore(base::FilePath());
}

net::URLRequestContext* ShellURLRequestContextGetter::GetURLRequestContext() {
  STDCHECK_ON_THREAD(IO);
  STLOG() << "GetURLRequestContext";
 
  if (!url_request_context_) {
    STLOG() << "creating new request context: " << base_path_.value();
    ST_SETTINGS(settings);
    SharedState* shared = shared_singleton.Pointer();
    url_request_context_.reset(new net::URLRequestContext());
    net::HttpNetworkSession::Params params;

    //  BEGIN ALWAYS SHARED STATE.
    url_request_context_->set_net_log(NetLog::Get());
    params.net_log = url_request_context_->net_log();

    url_request_context_->set_network_delegate(shared->GetNetworkDelegate());
    params.network_delegate =
        url_request_context_->network_delegate();

    url_request_context_->set_ssl_config_service(
        shared->GetSSLConfigService());
    params.ssl_config_service =
        url_request_context_->ssl_config_service();

    url_request_context_->set_cert_verifier(net::CertVerifier::CreateDefault());
    params.cert_verifier =
        url_request_context_->cert_verifier();

    url_request_context_->set_host_resolver(shared->GetHostResolver());
    params.host_resolver =
        url_request_context_->host_resolver();

    url_request_context_->set_http_auth_handler_factory(
        shared->GetAuthFactory(url_request_context_->host_resolver()));
    params.http_auth_handler_factory =
        url_request_context_->http_auth_handler_factory();

    url_request_context_->set_http_server_properties(
        shared->GetHttpServerProperties());
    params.http_server_properties =
        url_request_context_->http_server_properties();

    url_request_context_->set_proxy_service(
        shared->GetProxyService());
    params.proxy_service =
        url_request_context_->proxy_service();

    url_request_context_->set_http_user_agent_settings(
        shared->GetHttpUserAgentSettings());

    url_request_context_->set_job_factory(shared->GetJobFactory());

    // END ALWAYS SHARED STATE.
    // BEGIN EXTRA PARAMETER CONFIGURATION.

    params.ignore_certificate_errors = false;
    if (settings->network().spdy_enabled()) {
      STLOG() << "SPDY enabled";
      // TODO(sgurun) remove once crbug.com/329681 is fixed.
      params.next_protos = net::NextProtosSpdy31();
      params.use_alternate_protocols = true;
    } else {
      STLOG() << "SPDY disabled";
    }

    // END EXTRA PARAMETER CONFIGURATION.

    storage_.reset(
        new net::URLRequestContextStorage(url_request_context_.get()));

    // BEGIN COOKIE STORE.

    if (!base_path_.empty() &&
        settings->state().single_context() &&
        settings->state().allow_persistant_cookies()) {
      STLOG() << "using persistant cookies";
      url_request_context_->set_cookie_store(
          shared->GetCookieStore(base_path_));
    } else {
      STLOG() << "using non-persistant cookies";
      storage_->set_cookie_store(
          content::CreateCookieStore(content::CookieStoreConfig()));
    }

    // END COOKIE STORE.

    if (!base_path_.empty() &&
        settings->state().single_context() &&
        settings->state().persist_cache()) {
      STLOG() << "using persistant cache / hsts / sdch";
      url_request_context_->set_transport_security_state(
          shared->GetTransportSecurityState(base_path_));
      params.transport_security_state =
          url_request_context_->transport_security_state();


      int disk_cache_size = settings->state().disk_cache_size_mb()
        * 1024 * 1024;
      url_request_context_->set_http_transaction_factory(
          shared->GetHttpCache(base_path_, disk_cache_size, params));


      url_request_context_->set_sdch_manager(shared->GetSdch());
    } else {
      STLOG() << "using non-persistant cache / hsts / sdch";
      storage_->set_transport_security_state(new TransportSecurityState);
      params.transport_security_state =
          url_request_context_->transport_security_state();


      net::HttpCache::BackendFactory* main_backend =
        net::HttpCache::DefaultBackend::InMemory(0);
      net::HttpCache* main_cache = new net::HttpCache(
          params, main_backend);
      storage_->set_http_transaction_factory(main_cache);

      // TODO(cy) this exists in the latest version.
      // storage_->set_sdch_manager(new SdchManager());
      sdch_.reset(new SdchManager());
      url_request_context_->set_sdch_manager(sdch_.get());
    }
  }

  return url_request_context_.get();
}

scoped_refptr<base::SingleThreadTaskRunner>
    ShellURLRequestContextGetter::GetNetworkTaskRunner() const {
  return BrowserThread::GetMessageLoopProxyForThread(BrowserThread::IO);
}

HostResolver* ShellURLRequestContextGetter::host_resolver() {
  return url_request_context_->host_resolver();
}

void CookiesCleared(int cleared) {
  // TODO(cy) do something better with the result
  LOG(INFO) << "cleared cookies: " << cleared;
}

void CacheCleared(int cleared) {
  // TODO(cy) do something better with the result
  LOG(INFO) << "cleared cache: " << cleared;
}

// static
void ShellURLRequestContextGetter::ClearData() {
  SharedState* shared = shared_singleton.Pointer();
  SplitCookieStore* scs = shared->MaybeGetCookieStore();
  if (scs != NULL) {
    CookieStore::DeleteCallback cb = base::Bind(&CookiesCleared);
    scs->DeleteAllCreatedBetweenAsync(base::Time(), base::Time::Max(), cb);
  }
  HttpCache* hc = shared->MaybeGetHttpCache();
  if (hc != NULL) {
    disk_cache::Backend* backend = hc->GetCurrentBackend();
    if (backend != NULL) {
      disk_cache::Backend::CompletionCallback cb = base::Bind(&CacheCleared);
      backend->DoomAllEntries(cb);
    }
  }
  TransportSecurityState* tss = shared->MaybeGetTransportSecurityState();
  if (tss != NULL) {
    tss->DeleteAllDynamicDataSince(base::Time());
  }
  // TODO(cy) FINISH ME
}

}  // namespace seaturtle

