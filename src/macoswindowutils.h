// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QtGlobal>

#ifdef Q_OS_MACOS

namespace pulse {
/** 
    Makes a window appear on the current Space instead of triggering a Space
    transition. Must be called after the widget's native window is created (i.e.
    after show()/showFullScreen()).
*/
void applyOverlayWindowBehavior(quintptr windowId);
}

#endif // Q_OS_MACOS
