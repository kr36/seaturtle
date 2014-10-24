// Copyright 2014 Kr36 LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Multiply-included file, no traditional include guard.
#include <string>
#include <vector>

#include "ipc/ipc_message_macros.h"
#include "ipc/ipc_platform_file.h"

#define IPC_MESSAGE_START SeaturtleMsgStart

//IPC_MESSAGE_CONTROL1(SeaturtleMsg_UpdateBlockingRules,
//                     std::string /* rule list content */)

// IPC_MESSAGE_ROUTED0(SeaturtleMsg_FrameHasBlockedRequest)

IPC_MESSAGE_ROUTED0(SeaturtleMsg_NotifyDidCreateDocumentElement)
IPC_MESSAGE_ROUTED0(SeaturtleMsg_InjectCssInFrame)
