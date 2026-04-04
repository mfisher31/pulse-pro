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

/**
    Enables or disables the native window's open/close animation.

    Set to false before calling QWidget::hide() or QWidget::show() to suppress
    the macOS fade animation, then restore to true afterwards. Qt retains
    correct visibility state because it still owns the hide/show calls.

    No-op on non-macOS platforms.

    @param windowId Native window identifier for the widget window.
    @param enabled  Pass false to suppress animation, true to restore it.
*/
void setWindowAnimationEnabled(quintptr windowId, bool enabled);

/**
    Hides the system mouse cursor.

    Calls are reference-counted by the OS — each hideMouseCursor() must be
    paired with exactly one showMouseCursor().

    No-op on non-macOS platforms (Qt's QGuiApplication::setOverrideCursor
    can be used there instead).
*/
void hideMouseCursor();

/**
    Restores the system mouse cursor hidden by hideMouseCursor().
*/
void showMouseCursor();

}
