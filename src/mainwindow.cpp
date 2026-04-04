// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QAction>
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QGuiApplication>
#include <QMenu>
#include <QMenuBar>
#include <QPixmap>
#include <QScreen>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>

#include "captureengine.hpp"
#include "mainwindow.hpp"
#include "regionselectionoverlay.hpp"
#include "screencapturepreview.hpp"
#include "windowutil.hpp"

namespace pulse {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , _engine(new CaptureEngine(this))
    , _preview(new ScreenCapturePreview(_engine, this))
    , _snapshotButton(new QToolButton(this))
    , _countdownTimer(new QTimer(this))
{
    setWindowTitle(tr("Pulse Pro"));
    setCentralWidget(_preview);

    _countdownTimer->setInterval(1000);
    connect(_countdownTimer, &QTimer::timeout, this, [this]() {
        --_countdownRemaining;
        if (_countdownRemaining > 0) {
            statusBar()->showMessage(tr("Snapshot in %1…").arg(_countdownRemaining));
        } else {
            _countdownTimer->stop();
            statusBar()->showMessage(tr("Ready"));
            executeSnapshot();
        }
    });

    // --- Snapshot mode actions ---
    auto* fullscreenAction = new QAction(tr("Fullscreen"), this);
    auto* timedAction = new QAction(tr("After 5 Second Delay"), this);
    auto* selectionAction = new QAction(tr("Selection"), this);

    connect(fullscreenAction, &QAction::triggered, this, [this]() {
        takeSnapshotWithMode(SnapshotMode::Fullscreen);
    });
    connect(timedAction, &QAction::triggered, this, [this]() {
        takeSnapshotWithMode(SnapshotMode::Timed);
    });
    connect(selectionAction, &QAction::triggered, this, [this]() {
        takeSnapshotWithMode(SnapshotMode::Selection);
    });

    auto* snapshotMenu = new QMenu(this);
    snapshotMenu->addAction(fullscreenAction);
    snapshotMenu->addAction(timedAction);
    snapshotMenu->addAction(selectionAction);

    _snapshotButton->setText(tr("Snapshot"));
    _snapshotButton->setPopupMode(QToolButton::MenuButtonPopup);
    _snapshotButton->setMenu(snapshotMenu);
    _snapshotButton->setDefaultAction(fullscreenAction);
    connect(_snapshotButton, &QToolButton::clicked, this, [this]() {
        takeSnapshotWithMode(_snapshotMode);
    });

    // --- Toolbar ---
    auto* toolbar = addToolBar(tr("Main"));
    toolbar->setMovable(false);
    toolbar->addWidget(_snapshotButton);

    auto* recordAction = new QAction(tr("Record"), this);
    auto* settingsAction = new QAction(tr("Settings"), this);
    toolbar->addAction(recordAction);
    toolbar->addSeparator();
    toolbar->addAction(settingsAction);

    // --- Menu bar ---
    auto* fileMenu = menuBar()->addMenu(tr("File"));
    fileMenu->addAction(fullscreenAction);
    fileMenu->addAction(timedAction);
    fileMenu->addAction(selectionAction);
    fileMenu->addSeparator();
    fileMenu->addAction(recordAction);
    fileMenu->addSeparator();
    auto* quitAction = new QAction(tr("Quit"), this);
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
    fileMenu->addAction(quitAction);

    auto* editMenu = menuBar()->addMenu(tr("Edit"));
    editMenu->addAction(settingsAction);

    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::takeSnapshotWithMode(SnapshotMode mode)
{
    _snapshotMode = mode;

    switch (mode) {
    case SnapshotMode::Fullscreen:
        setWindowAnimationEnabled(winId(), false);
        hide();
        QTimer::singleShot(100, this, [this]() {
            executeSnapshot();
            show();
            setWindowAnimationEnabled(winId(), true);
        });
        break;

    case SnapshotMode::Timed:
        _countdownRemaining = 5;
        statusBar()->showMessage(tr("Snapshot in 5…"));
        _countdownTimer->start();
        break;

    case SnapshotMode::Selection:
        hide();
        QTimer::singleShot(80, this, [this]() {
            for (QScreen* screen : QGuiApplication::screens()) {
                auto* overlay = new RegionSelectionOverlay(screen);
                connect(overlay,
                        &RegionSelectionOverlay::regionSelected,
                        this,
                        &MainWindow::onRegionSelected);
                connect(overlay,
                        &RegionSelectionOverlay::selectionCancelled,
                        this,
                        &MainWindow::onSelectionCancelled);
                _overlays.append(overlay);
            }
            if (!_overlays.isEmpty())
                _overlays.first()->setFocus();
        });
        break;
    }
}

void MainWindow::onRegionSelected(QRect globalRect)
{
    for (auto* overlay : _overlays) {
        overlay->setVisible(false);
        overlay->hide();
    }
    qDeleteAll(_overlays);
    _overlays.clear();

    // Wait for the compositor to repaint the screen without the overlay before
    // grabbing — otherwise the blue selection border bleeds into the capture.
    QTimer::singleShot(100, this, [this, globalRect]() {
        executeSnapshot(globalRect);
        show();
    });
}

void MainWindow::onSelectionCancelled()
{
    qDeleteAll(_overlays);
    _overlays.clear();
    show();
}

void MainWindow::executeSnapshot(const QRect& cropRect)
{
    _engine->writeSnapshot (nextSnapshotPath(), cropRect);
}

QString MainWindow::nextSnapshotPath() const
{
    const QDir dir(QDir::homePath() + "/Desktop/Snapshots");
    if (!dir.exists())
        dir.mkpath(".");
    const QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
    return dir.filePath("snapshot-" + timestamp + ".png");
}

CaptureEngine* MainWindow::engine() const
{
    return _engine;
}

} // namespace pulse
