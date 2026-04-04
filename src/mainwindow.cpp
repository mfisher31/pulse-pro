// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QAction>
#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>

#include "captureengine.hpp"
#include "mainwindow.hpp"
#include "screencapturepreview.hpp"

namespace pulse {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , _engine(new CaptureEngine(this))
    , _preview(new ScreenCapturePreview(_engine, this))
{
    setWindowTitle(tr("Pulse Pro"));
    setCentralWidget(_preview);

    // Toolbar
    auto* toolbar = addToolBar(tr("Main"));
    toolbar->setMovable(false);

    // Placeholder actions — wire to _preview as features are added
    auto* newSessionAction = new QAction(tr("Snapshot"), this);
    auto* openAction = new QAction(tr("Record"), this);
    auto* settingsAction = new QAction(tr("Settings"), this);

    toolbar->addAction(newSessionAction);
    toolbar->addAction(openAction);
    toolbar->addSeparator();
    toolbar->addAction(settingsAction);

    // Menu bar
    auto* fileMenu = menuBar()->addMenu(tr("File"));
    fileMenu->addAction(newSessionAction);
    fileMenu->addAction(openAction);
    fileMenu->addSeparator();
    auto* quitAction = new QAction(tr("Quit"), this);
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
    fileMenu->addAction(quitAction);

    auto* editMenu = menuBar()->addMenu(tr("Edit"));
    editMenu->addAction(settingsAction);

    statusBar()->showMessage(tr("Ready"));
}

CaptureEngine* MainWindow::engine() const
{
    return _engine;
}

} // namespace pulse
