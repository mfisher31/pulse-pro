// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QAction>
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QStandardPaths>
#include <QGuiApplication>
#include <QMenu>
#include <QMenuBar>
#include <QMimeDatabase>
#include <QPixmap>
#include <QScreen>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

#include "captureengine.hpp"
#include "testwindow.hpp"
#include "regionselectionoverlay.hpp"
#include "screencapturepreview.hpp"
#include "sourcedisplay.hpp"
#include "windowutil.hpp"

namespace pulse {

TestWindow::TestWindow(CaptureEngine* engine, QWidget* parent)
    : QMainWindow(parent)
    , _engine(engine)
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
    auto* openAction = new QAction(tr("Open…"), this);
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &TestWindow::openMediaFile);
    fileMenu->addAction(openAction);
    fileMenu->addSeparator();
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

void TestWindow::takeSnapshotWithMode(SnapshotMode mode)
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
                        &TestWindow::onRegionSelected);
                connect(overlay,
                        &RegionSelectionOverlay::selectionCancelled,
                        this,
                        &TestWindow::onSelectionCancelled);
                _overlays.append(overlay);
            }
            if (!_overlays.isEmpty())
                _overlays.first()->setFocus();
        });
        break;
    }
}

void TestWindow::onRegionSelected(QRect globalRect)
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

void TestWindow::onSelectionCancelled()
{
    qDeleteAll(_overlays);
    _overlays.clear();
    show();
}

void TestWindow::executeSnapshot(const QRect& cropRect)
{
    _engine->writeSnapshot (nextSnapshotPath(), cropRect);
}

QString TestWindow::nextSnapshotPath() const
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    const QDir dir(base + "/Snapshots");
    if (!dir.exists())
        dir.mkpath(".");
    const QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
    return dir.filePath("snapshot-" + timestamp + ".png");
}

CaptureEngine* TestWindow::engine() const
{
    return _engine;
}

void TestWindow::openMediaFile()
{
    static const QString filter = tr(
        "Media Files (*.mp4 *.mov *.m4v *.avi *.mkv *.png *.jpg *.jpeg *.tiff *.tif *.bmp *.gif);;"
        "Video Files (*.mp4 *.mov *.m4v *.avi *.mkv);;"
        "Image Files (*.png *.jpg *.jpeg *.tiff *.tif *.bmp *.gif);;"
        "All Files (*)");

    const QString filePath = QFileDialog::getOpenFileName(this, tr("Open Media"), QDir::homePath(), filter);
    if (filePath.isEmpty())
        return;

    if (!_mediaWindow) {
        _mediaWindow = new QWidget(nullptr, Qt::Window);
        _mediaWindow->setAttribute(Qt::WA_DeleteOnClose);
        _mediaWindow->setWindowTitle(tr("Media Preview"));
        _mediaWindow->resize(800, 600);
        _sourceDisplay = new SourceDisplay(_mediaWindow);
        auto* layout = new QVBoxLayout(_mediaWindow);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(_sourceDisplay);
        connect(_mediaWindow, &QWidget::destroyed, this, [this]() {
            _mediaWindow = nullptr;
            _sourceDisplay = nullptr;
        });
    }

    const QMimeDatabase mimeDb;
    const QString mimeType = mimeDb.mimeTypeForFile(filePath).name();

    if (mimeType.startsWith("video/"))
        _sourceDisplay->setVideoSource(filePath);
    else
        _sourceDisplay->setImageSource(filePath);

    _mediaWindow->setWindowTitle(tr("Media Preview — %1").arg(QFileInfo(filePath).fileName()));
    _mediaWindow->show();
    _mediaWindow->raise();
    _mediaWindow->resize(640, 360);

}

} // namespace pulse
