// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QtGlobal>

QT_USE_NAMESPACE

namespace pulse {

/** 
    Causes a window appear on the current Space instead of triggering a Space
    transition. Must be called after the widget's native window is created (i.e.
    after show()/showFullScreen()).

    @param windowId Native window identifier for the widget window.
*/
void applyOverlayWindowBehavior(quintptr windowId);

}
