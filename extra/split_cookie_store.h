// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_EXTRA_SPLIT_COOKIE_STORE_H_
#define SEATURTLE_EXTRA_SPLIT_COOKIE_STORE_H_

#include <string>

#include "base/synchronization/lock.h"
#include "base/containers/hash_tables.h"
#include "net/cookies/cookie_store.h"

namespace base {
class FilePath;
}

namespace net {
class CookieStore;
}  // namespace net

namespace seaturtle {

namespace jni {
class  UpdateFavoritesResponse;
}

class SplitCookieStore : public net::CookieStore {
 public:
  SplitCookieStore(const base::FilePath& path);

  virtual void SetCookieWithOptionsAsync(
      const GURL& url,
      const std::string& cookie_line,
      const net::CookieOptions& options,
      const net::CookieStore::SetCookiesCallback& callback) OVERRIDE;

  virtual void GetCookiesWithOptionsAsync(
      const GURL& url,
      const net::CookieOptions& options,
      const net::CookieStore::GetCookiesCallback& callback) OVERRIDE;

  virtual void GetAllCookiesForURLAsync(
      const GURL& url,
      const net::CookieStore::GetCookieListCallback& callback) OVERRIDE;

  virtual void DeleteCookieAsync(const GURL& url,
                                 const std::string& cookie_name,
                                 const base::Closure& callback) OVERRIDE;

  virtual void DeleteAllCreatedBetweenAsync(
      const base::Time& delete_begin,
      const base::Time& delete_end,
      const net::CookieStore::DeleteCallback& callback) OVERRIDE;

  virtual void DeleteAllCreatedBetweenForHostAsync(
      const base::Time delete_begin,
      const base::Time delete_end,
      const GURL& url,
      const net::CookieStore::DeleteCallback& callback) OVERRIDE;

  virtual void DeleteSessionCookiesAsync(
      const net::CookieStore::DeleteCallback&) OVERRIDE;

  // Returns the underlying CookieMonster.
  virtual net::CookieMonster* GetCookieMonster() OVERRIDE;

  void UpdateFavorites(const jni::UpdateFavoritesResponse& update);

 protected:
  friend class base::RefCountedThreadSafe<SplitCookieStore>;
  virtual ~SplitCookieStore();

 private:
  static GURL GetSiteForURL(const GURL& url);

  net::CookieStore* ForURL(const GURL& url);

  void PersistantDeleteCookieAsync(const GURL& url,
                                   const std::string& cookie_name,
                                   const base::Closure& callback);

  void PersistantDeleteAllCreatedBetweenAsync(
      const base::Time& delete_begin,
      const base::Time& delete_end,
      const net::CookieStore::DeleteCallback& callback,
      int already_deleted);

  void PersistantDeleteAllCreatedBetweenForHostAsync(
      const base::Time delete_begin,
      const base::Time delete_end,
      const GURL& url,
      const net::CookieStore::DeleteCallback& callback,
      int already_deleted);

  void PersistantDeleteSessionCookiesAsync(
      const net::CookieStore::DeleteCallback&,
      int already_deleted);

  scoped_refptr<net::CookieStore> persistant_;
  scoped_refptr<net::CookieStore> transient_;

  base::Lock lock_;
  base::hash_set<std::string> favorites_;

  DISALLOW_COPY_AND_ASSIGN(SplitCookieStore);
};

}  // namespace seaturtle

#endif  // SEATURTLE_EXTRA_SPLIT_COOKIE_STORE_H_
