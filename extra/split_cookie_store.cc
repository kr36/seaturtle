// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/extra/split_cookie_store.h"

#include "base/stl_util.h"
#include "net/base/registry_controlled_domains/registry_controlled_domain.h"
#include "content/public/browser/cookie_store_factory.h"
#include "url/gurl.h"

#include "seaturtle/extra/base.h"
#include "seaturtle/jni/jni_bridge.h"

using content::CookieStoreConfig;
using net::CookieOptions;
using net::CookieStore;
using net::CookieMonster;

namespace seaturtle {

namespace {

void DeleteComplete(const CookieStore::DeleteCallback& finished, int a, int b) {
  return finished.Run(a + b);
}

}  // namespace

SplitCookieStore::SplitCookieStore(const base::FilePath& path) {
  CookieStoreConfig p_config(
      path,
      CookieStoreConfig::RESTORED_SESSION_COOKIES,
      NULL, NULL);
  STLOG() << "initializing persistant cookie store in " << path.value();
  persistant_ = content::CreateCookieStore(p_config);
  transient_ = content::CreateCookieStore(CookieStoreConfig());
  jni::Params req;
  jni::Params resp;
  req.set_type(jni::Params::UPDATE_FAVORITES_REQUEST);
  jni::Invoke(req, &resp);
  UpdateFavorites(resp.update_favorites_response());
}

SplitCookieStore::~SplitCookieStore() {
}

CookieStore* SplitCookieStore::ForURL(const GURL& url) {
  std::string site_host = GetSiteForURL(url).host();
  bool is_persistant;
  {
    base::AutoLock lock(lock_);
    is_persistant = ContainsKey(favorites_, site_host);
  }
  if (is_persistant) {
    STLOG() << "persistant cookie: " << site_host <<
      " (" << url << ")";
  } else {
    STLOG() << "transient cookie: " << site_host <<
      " (" << url << ")";
  }
  if (is_persistant) {
    return persistant_.get();
  }
  return transient_.get();
}

void SplitCookieStore::UpdateFavorites(
    const jni::UpdateFavoritesResponse& update) {
  base::AutoLock lock(lock_);
  favorites_.clear();
  for (int i = 0; i < update.url_size(); ++i) {
    const std::string& url = update.url(i);
    GURL gurl(url);
    if (!gurl.is_valid()) {
      STNOTREACHED() << "invalid favorite url: " << url;
      continue;
    }
    GURL site = GetSiteForURL(gurl);
    STLOG() << "adding favorite: " << site.host() << " (" << url << ")";
    favorites_.insert(site.host());
  }
  // TODO(cy) Cookie dance, move cookies between persistant_ and transient_
  // as nessecary.
}

void SplitCookieStore::SetCookieWithOptionsAsync(
      const GURL& url,
      const std::string& cookie_line,
      const CookieOptions& options,
      const CookieStore::SetCookiesCallback& callback) {
  STLOG() << "SetCookieWithOptionsAsync " << url;
  ForURL(url)->SetCookieWithOptionsAsync(
      url, cookie_line, options, callback);
}

void SplitCookieStore::GetCookiesWithOptionsAsync(
      const GURL& url,
      const CookieOptions& options,
      const CookieStore::GetCookiesCallback& callback) {
  ForURL(url)->GetCookiesWithOptionsAsync(
      url, options, callback);
}

void SplitCookieStore::GetAllCookiesForURLAsync(
      const GURL& url,
      const CookieStore::GetCookieListCallback& callback) {
  ForURL(url)->GetAllCookiesForURLAsync(url, callback);
}

void SplitCookieStore::DeleteCookieAsync(const GURL& url,
    const std::string& cookie_name,
    const base::Closure& finished) {
  base::Closure cb =
    base::Bind(
        &SplitCookieStore::PersistantDeleteCookieAsync, this,
        url, cookie_name, finished);
  transient_->DeleteCookieAsync(url, cookie_name, cb);
}

void SplitCookieStore::PersistantDeleteCookieAsync(const GURL& url,
    const std::string& cookie_name,
    const base::Closure& finished) {
  persistant_->DeleteCookieAsync(url, cookie_name, finished);
}

void SplitCookieStore::DeleteAllCreatedBetweenAsync(
    const base::Time& delete_begin,
    const base::Time& delete_end,
    const CookieStore::DeleteCallback& finished) {
  CookieStore::DeleteCallback cb =
    base::Bind(
        &SplitCookieStore::PersistantDeleteAllCreatedBetweenAsync, this,
        delete_begin, delete_end, finished);
  transient_->DeleteAllCreatedBetweenAsync(
      delete_begin, delete_end, cb);
}

void SplitCookieStore::PersistantDeleteAllCreatedBetweenAsync(
    const base::Time& delete_begin,
    const base::Time& delete_end,
    const CookieStore::DeleteCallback& finished,
    int already_deleted) {
  CookieStore::DeleteCallback cb =
    base::Bind(&DeleteComplete, finished, already_deleted);
  persistant_->DeleteAllCreatedBetweenAsync(
      delete_begin, delete_end, cb);
}

void SplitCookieStore::DeleteAllCreatedBetweenForHostAsync(
    const base::Time delete_begin,
    const base::Time delete_end,
    const GURL& url,
    const CookieStore::DeleteCallback& finished) {
  CookieStore::DeleteCallback cb =
    base::Bind(
        &SplitCookieStore::PersistantDeleteAllCreatedBetweenForHostAsync, this,
        delete_begin, delete_end, url, finished);
  transient_->DeleteAllCreatedBetweenForHostAsync(
      delete_begin, delete_end, url, cb);
}

void SplitCookieStore::PersistantDeleteAllCreatedBetweenForHostAsync(
    const base::Time delete_begin,
    const base::Time delete_end,
    const GURL& url,
    const CookieStore::DeleteCallback& finished,
    int already_deleted) {
  CookieStore::DeleteCallback cb =
    base::Bind(&DeleteComplete, finished, already_deleted);
  persistant_->DeleteAllCreatedBetweenForHostAsync(
      delete_begin, delete_end, url, cb);
}

void SplitCookieStore::DeleteSessionCookiesAsync(
    const CookieStore::DeleteCallback& finished) {
  CookieStore::DeleteCallback cb =
    base::Bind(&SplitCookieStore::PersistantDeleteSessionCookiesAsync, this,
    finished);
  transient_->DeleteSessionCookiesAsync(cb);
}

void SplitCookieStore::PersistantDeleteSessionCookiesAsync(
    const CookieStore::DeleteCallback& finished,
    int already_deleted) {
  CookieStore::DeleteCallback cb =
    base::Bind(&DeleteComplete, finished, already_deleted);
  persistant_->DeleteSessionCookiesAsync(cb);
}

CookieMonster* SplitCookieStore::GetCookieMonster() {
  // STNOTREACHED();
  // Strangely this is getting called by media_resource_getter
  return NULL;
}

// static
GURL SplitCookieStore::GetSiteForURL(const GURL& url) {
  // WATCH this was stolen from site_instance_impl, keep an eye on it.

  // URLs with no host should have an empty site.
  GURL site;

  // TODO(creis): For many protocols, we should just treat the scheme as the
  // site, since there is no host.  e.g., file:, about:, chrome:

  // If the url has a host, then determine the site.
  if (url.has_host()) {
    // Only keep the scheme and registered domain as given by GetOrigin.  This
    // may also include a port, which we need to drop.
    site = url.GetOrigin();

    // Remove port, if any.
    if (site.has_port()) {
      GURL::Replacements rep;
      rep.ClearPort();
      site = site.ReplaceComponents(rep);
    }

    // If this URL has a registered domain, we only want to remember that part.
    std::string domain =
        net::registry_controlled_domains::GetDomainAndRegistry(
            url,
            net::registry_controlled_domains::INCLUDE_PRIVATE_REGISTRIES);
    if (!domain.empty()) {
      GURL::Replacements rep;
      rep.SetHostStr(domain);
      site = site.ReplaceComponents(rep);
    }
  }
  return site;
}

}  // namespace seaturtle
