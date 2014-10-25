// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/android/jni_android.h"
#include "base/android/jni_registrar.h"
#include "base/android/library_loader/library_loader_hooks.h"
#include "base/basictypes.h"
#include "base/debug/debugger.h"
#include "base/logging.h"
#include "content/public/app/content_main.h"
#include "content/public/browser/android/compositor.h"
#include "seaturtle/shell/app/shell_main_delegate.h"
#include "seaturtle/app/android/library_loader_hooks.h"

// This is called by the VM when the shared library is first loaded.
JNI_EXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
  base::android::SetLibraryLoadedHook(&seaturtle::LibraryLoaded);

  base::android::InitVM(vm);
  JNIEnv* env = base::android::AttachCurrentThread();

  if (!base::android::RegisterLibraryLoaderEntryHook(env))
    return -1;

  content::Compositor::Initialize();
  content::SetContentMainDelegate(new seaturtle::ShellMainDelegate());

  return JNI_VERSION_1_4;
}
