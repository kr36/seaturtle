// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "seaturtle/extra/seaturtle_protocol_handler.h"

#include <algorithm>
#include <string>

#include "base/bind.h"
#include "base/memory/weak_ptr.h"
#include "net/base/io_buffer.h"
#include "net/url_request/url_request.h"
#include "net/url_request/url_request_job.h"

#include "seaturtle/extra/base.h"
#include "seaturtle/jni/jni_bridge.h"

using net::URLRequest;
using net::URLRequestJob;
using net::NetworkDelegate;

namespace seaturtle {

using jni::Params;
using jni::Render;

namespace {

class URLRequestSeaturtleJob : public URLRequestJob {
 public:
  URLRequestSeaturtleJob(URLRequest* request,
      NetworkDelegate* network_delegate)
      : URLRequestJob(request, network_delegate),
        weak_factory_(this),
        bytes_already_read_(0) {}

  virtual void Start() OVERRIDE {
    base::MessageLoop::current()->PostTask(
      FROM_HERE,
      base::Bind(&URLRequestSeaturtleJob::StartAsync,
                weak_factory_.GetWeakPtr()));
  };
  virtual bool GetMimeType(std::string* mime_type) const OVERRIDE {
    *mime_type = "text/html";
    return true;
  }
  virtual bool ReadRawData(
      net::IOBuffer* buf, int buf_size, int* bytes_read) OVERRIDE {
    STDCHECK(buf_size > 0);
    STDCHECK(bytes_read);
    STDCHECK(resp_.get() != NULL);
    int resp_length = resp_->render_response().size();
    if (bytes_already_read_ < resp_length) {
      *bytes_read = std::min(buf_size, resp_length);
      strncpy(buf->data(), resp_->render_response().data(), *bytes_read);
      bytes_already_read_ += *bytes_read;
    } else {
      *bytes_read = 0;
    }
    return true;
  }

 protected:
  virtual ~URLRequestSeaturtleJob() {}

 private:
  void StartAsync() {
    Params p;
    p.set_type(Params::RENDER);
    p.mutable_render()->set_type(Render::SCHEME);
    p.mutable_render()->mutable_scheme()->set_url(request()->url().spec());

    resp_.reset(new Params());
    jni::Invoke(p, resp_.get());
    STDCHECK(resp_->type() == Params::RENDER_RESPONSE);
    NotifyHeadersComplete();
  }

  base::WeakPtrFactory<URLRequestSeaturtleJob> weak_factory_;
  int bytes_already_read_;
  scoped_ptr<Params> resp_;
};

}  // namespace

SeaturtleProtocolHandler::SeaturtleProtocolHandler() {
}

URLRequestJob* SeaturtleProtocolHandler::MaybeCreateJob(
    URLRequest* request, NetworkDelegate* network_delegate) const {
  return new URLRequestSeaturtleJob(request, network_delegate);
}

bool SeaturtleProtocolHandler::IsSafeRedirectTarget(
    const GURL& location) const {
  return false;
}

}  // namespace seaturtle
