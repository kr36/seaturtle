// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_EXTRA_FETCH_H_
#define SEATURTLE_EXTRA_FETCH_H_

#include <string>

#include "net/url_request/url_fetcher.h"
#include "net/url_request/url_fetcher_delegate.h"

namespace seaturtle {

namespace jni {
class Params;
}

class Fetcher : public net::URLFetcherDelegate {
 public:
  static jni::Params* Fetch(const std::string& url);
  static void Init(Fetcher* f);
  static bool IsAutocompleteRequest(const net::URLRequest& req);

  virtual ~Fetcher();
  virtual void OnURLFetchComplete(const net::URLFetcher* source) OVERRIDE;

 private:
  explicit Fetcher(const std::string& url);

  int id_;
  std::string url_;
  scoped_ptr<net::URLFetcher> url_fetcher_;

  DISALLOW_COPY_AND_ASSIGN(Fetcher);
};


}  // namespace seaturtle

#endif  // SEATURTLE_EXTRA_FETCH_H_
