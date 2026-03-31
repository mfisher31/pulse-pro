// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#import <AppKit/AppKit.h>

#include "windowutil.hpp"

namespace pulse {

void applyOverlayWindowBehavior(quintptr windowId)
{
    NSView* view = reinterpret_cast<NSView*>(windowId);
    NSWindow* window = [view window];
    if (!window)
        return;

    // clang-format off
    window.collectionBehavior = NSWindowCollectionBehaviorCanJoinAllSpaces
                              |  NSWindowCollectionBehaviorStationary 
                              |  NSWindowCollectionBehaviorFullScreenAuxiliary
                              |  NSWindowCollectionBehaviorIgnoresCycle;
    // clang-format on
}

}
