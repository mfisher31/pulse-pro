// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>
#include <QString>

QT_USE_NAMESPACE

namespace pulse {

class CaptureEngine;
class MainWindow;
class TrayController;

/**
    Top-level application controller.

    Owns CaptureEngine, MainWindow, and TrayController, wiring them together
    so that actions from both the tray and the main window share a single
    capture pipeline. This is the entry point for application-level logic;
    main.cpp constructs this and calls start().

    @note MainWindow is not a QObject child of AppController because QWidget
          top-level windows must not have a non-widget parent. Its lifetime
          is managed explicitly via the destructor.
*/
class AppController : public QObject
{
    Q_OBJECT

public:
    explicit AppController(QObject* parent = nullptr);
    ~AppController() override;

    /** Shows the main window and makes the tray icon visible. */
    void start();

    /** @returns The application-level capture engine. */
    CaptureEngine* engine() const;

private:
    void takeSnapshot();
    void toggleRecording();
    QString nextSnapshotPath() const;
    QString nextRecordingPath() const;

private:
    CaptureEngine* _engine = nullptr;
    MainWindow* _window = nullptr;
    TrayController* _tray = nullptr;
};

} // namespace pulse
