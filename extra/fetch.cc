// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/extra/fetch.h"

#include "base/atomic_sequence_num.h"
#include "net/base/load_flags.h"
#include "content/public/browser/browser_thread.h"

#include "seaturtle/extra/base.h"
#include "seaturtle/jni/jni_bridge.h"
#include "seaturtle/shell/browser/shell_url_request_context_getter.h"
#include "seaturtle/shell/browser/shell_browser_context.h"

using content::BrowserThread;
using std::string;
using net::URLFetcher;
using net::URLRequest;
using seaturtle::jni::Params;

namespace seaturtle {

namespace {
base::StaticAtomicSequenceNumber seq;

const char kSeaturtleAutocompleteRequest[] = "seaturtle-autocomplete-request";

class AutocompleteRequestUserData : public base::SupportsUserData::Data {
 public:
  static net::URLFetcher::CreateDataCallback CreateCallback() {
    return base::Bind(&AutocompleteRequestUserData::Create);
  }

 private:
  static base::SupportsUserData::Data* Create() {
    return new AutocompleteRequestUserData();
  }
};

scoped_refptr<net::URLRequestContextGetter> fetch_getter;

}  // namespace

Fetcher::Fetcher(const string& url) : url_(url) {
  id_ = seq.GetNext();
}

// static
void Fetcher::Init(Fetcher* f) {
  STDCHECK_ON_THREAD(IO);
  f->url_fetcher_.reset(net::URLFetcher::Create(GURL(f->url_),
      net::URLFetcher::GET, f));

  if (fetch_getter.get() == NULL) {
    fetch_getter = new ShellURLRequestContextGetter();
  }
  f->url_fetcher_->SetRequestContext(fetch_getter.get());

  //url_fetcher_->AddExtraRequestHeader("Accept: application/json");
  f->url_fetcher_->SetLoadFlags(net::LOAD_DO_NOT_SEND_COOKIES |
                             net::LOAD_DO_NOT_SAVE_COOKIES |
                             net::LOAD_DISABLE_CACHE |
                             net::LOAD_DO_NOT_SEND_AUTH_DATA);
  f->url_fetcher_->SetURLRequestUserData(kSeaturtleAutocompleteRequest,
      AutocompleteRequestUserData::CreateCallback());
  f->url_fetcher_->Start();
}

void Fetcher::OnURLFetchComplete(const net::URLFetcher* source) {
  STDCHECK_ON_THREAD(IO);
  Params p;
  p.set_type(Params::HTTP_FETCH_COMPLETE);
  p.mutable_http_fetch_complete()->set_id(id_);
  p.mutable_http_fetch_complete()->set_response(source->GetResponseCode());
  bool result = source->GetResponseAsString(
      p.mutable_http_fetch_complete()->mutable_content());
  STDCHECK(result);
  jni::Invoke(p);
  delete this;
}

Fetcher::~Fetcher() {}

// static
Params* Fetcher::Fetch(const std::string& url) {
  Fetcher* f = new Fetcher(url);
  bool result = BrowserThread::PostTask(
    BrowserThread::IO,
    FROM_HERE,
    base::Bind(&Fetcher::Init, f));
  STDCHECK(result);
  Params* p = new Params();
  p->set_type(Params::HTTP_FETCH_RESPONSE);
  p->set_http_fetch_response(f->id_);
  return p;
}

// static
bool Fetcher::IsAutocompleteRequest(const URLRequest& req) {
  return req.GetUserData(kSeaturtleAutocompleteRequest) != NULL;
}

}  // namespace seaturtle
