// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QDateTime>
#include <QDir>
#include <QMediaRecorder>
#include <QStandardPaths>
#include <QTimer>

#include "appcontroller.hpp"
#include "captureengine.hpp"
#include "mainwindow.hpp"
#include "traycontroller.hpp"

namespace pulse {

AppController::AppController(QObject* parent)
    : QObject(parent)
    , _engine(new CaptureEngine(this))
    , _window(new MainWindow(_engine))
    , _tray(new TrayController(_engine, this))
{
    connect(_tray, &TrayController::snapshotRequested, this, &AppController::takeSnapshot);
    connect(_tray, &TrayController::recordToggleRequested, this, &AppController::toggleRecording);
    connect(_tray, &TrayController::showWindowRequested, this, [this]() {
        _window->show();
        _window->raise();
        _window->activateWindow();
    });
}

AppController::~AppController()
{
    delete _window;
}

void AppController::start()
{
    _window->show();
}

CaptureEngine* AppController::engine() const
{
    return _engine;
}

void AppController::takeSnapshot()
{
    const bool windowVisible = _window->isVisible();
    if (windowVisible)
        _window->hide();

    // Brief delay so the compositor repaints before the grab
    QTimer::singleShot(100, this, [this, windowVisible]() {
        _engine->writeSnapshot(nextSnapshotPath());
        if (windowVisible)
            _window->show();
    });
}

void AppController::toggleRecording()
{
    if (_engine->recorderState() == QMediaRecorder::RecordingState) {
        _engine->stopRecording();
    } else {
        _engine->startRecording(nextRecordingPath());
    }
}

QString AppController::nextSnapshotPath() const
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    const QDir dir(base + "/Snapshots");
    if (!dir.exists())
        dir.mkpath(".");
    const QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
    return dir.filePath("snapshot-" + timestamp + ".png");
}

QString AppController::nextRecordingPath() const
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
    const QDir dir(base + "/Recordings");
    if (!dir.exists())
        dir.mkpath(".");
    const QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
    return dir.filePath("movie-" + timestamp + ".mp4");
}

} // namespace pulse
