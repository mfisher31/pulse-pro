// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QSystemTrayIcon;
QT_END_NAMESPACE

QT_USE_NAMESPACE

namespace pulse {

class CaptureEngine;

/**
    Manages the system tray icon and its context menu.

    Provides quick access to snapshot, recording, and window controls without
    requiring the main window to be visible. Actions that need application-level
    coordination (snapshot, show window) are exposed as signals; the engine is
    called directly for recording state changes.
*/
class TrayController : public QObject
{
    Q_OBJECT

public:
    explicit TrayController(CaptureEngine* engine, QObject* parent = nullptr);
    ~TrayController() override;

signals:
    /** Emitted when the user requests a fullscreen snapshot from the tray. */
    void snapshotRequested();

    /** Emitted when the user toggles recording via the tray menu. */
    void recordToggleRequested();

    /** Emitted when the user requests the main window be shown. */
    void showWindowRequested();

private:
    CaptureEngine* _engine = nullptr;
    QSystemTrayIcon* _trayIcon = nullptr;
    QMenu* _contextMenu = nullptr;
    QAction* _recordAction = nullptr;
};

} // namespace pulse
