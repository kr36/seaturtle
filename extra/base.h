// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SEATURTLE_EXTRA_BASE_H_
#define SEATURTLE_EXTRA_BASE_H_

#include "base/logging.h"
#include "content/public/browser/browser_thread.h"

#if defined(SEATURTLE_DEBUG_BUILD)
#define STLOG() \
  LOG(INFO)
#else // defined(SEATURTLE_DEBUG_BUILD)
#define STLOG() \
  DLOG(INFO)
#endif

// TODO(cy) Add release versions of this
#define STDCHECK(condition) \
  CHECK(condition)

// TODO(cy) Add release versions of this
#define STNOTREACHED() \
  CHECK(false)

// TODO(cy) Add release versions of this
#define STDCHECK_ON_THREAD(thread_identifier)                      \
  (CHECK(::content::BrowserThread::CurrentlyOn(\
      ::content::BrowserThread::thread_identifier)) \
   << ::content::BrowserThread::GetDCheckCurrentlyOnErrorMessage(   \
          ::content::BrowserThread::thread_identifier))

#endif  // SEATURTLE_EXTRA_BASE_H_
