// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QAction>
#include <QApplication>
#include <QMediaRecorder>
#include <QMenu>
#include <QStyle>
#include <QSystemTrayIcon>

#include "captureengine.hpp"
#include "traycontroller.hpp"

namespace pulse {

TrayController::TrayController(CaptureEngine* engine, QObject* parent)
    : QObject(parent)
    , _engine(engine)
    , _trayIcon(new QSystemTrayIcon(this))
    , _contextMenu(new QMenu())
    , _recordAction(new QAction(tr("Record"), this))
{
    // TODO: Replace with the application icon once assets are available
    _trayIcon->setIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));
    _trayIcon->setToolTip(tr("Pulse Pro"));

    auto* snapshotAction = new QAction(tr("Snapshot"), this);
    auto* showAction = new QAction(tr("Show Window"), this);
    auto* quitAction = new QAction(tr("Quit"), this);

    _contextMenu->addAction(snapshotAction);
    _contextMenu->addAction(_recordAction);
    _contextMenu->addSeparator();
    _contextMenu->addAction(showAction);
    _contextMenu->addSeparator();
    _contextMenu->addAction(quitAction);

    _trayIcon->setContextMenu(_contextMenu);

    connect(snapshotAction, &QAction::triggered, this, &TrayController::snapshotRequested);
    connect(_recordAction, &QAction::triggered, this, &TrayController::recordToggleRequested);
    connect(showAction, &QAction::triggered, this, &TrayController::showWindowRequested);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    connect(_engine,
            &CaptureEngine::recorderStateChanged,
            this,
            [this](QMediaRecorder::RecorderState state) {
                _recordAction->setText(state == QMediaRecorder::RecordingState
                                           ? tr("Stop Recording")
                                           : tr("Record"));
            });

    // Double-clicking the tray icon also shows the main window
    connect(_trayIcon,
            &QSystemTrayIcon::activated,
            this,
            [this](QSystemTrayIcon::ActivationReason reason) {
                if (reason == QSystemTrayIcon::DoubleClick)
                    emit showWindowRequested();
            });

    _trayIcon->show();
}

TrayController::~TrayController()
{
    delete _contextMenu;
}

} // namespace pulse
