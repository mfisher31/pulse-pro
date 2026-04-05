// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QMainWindow>
#include <QRect>

QT_BEGIN_NAMESPACE
class QTimer;
class QToolButton;
QT_END_NAMESPACE

QT_USE_NAMESPACE

namespace pulse {

class CaptureEngine;
class RegionSelectionOverlay;
class ScreenCapturePreview;
class SourceDisplay;

/**
    Top-level application window.

    Receives a CaptureEngine reference and passes it to ScreenCapturePreview.
    Toolbar and menu actions wire directly to the engine so that window-level
    controls (record, stop, settings) do not need to go through the preview
    widget. The engine is not owned by this window — its lifetime is managed
    by AppController.
*/
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /** Identifies which snapshot mode the toolbar button currently uses. */
    enum class SnapshotMode
    {
        Fullscreen,
        Timed,
        Selection
    };

    /**
        Constructs the main window.

        @param engine The application-level capture engine. Not owned by this window.
        @param parent Optional parent widget.
    */
    explicit MainWindow(CaptureEngine* engine, QWidget* parent = nullptr);

    /** @returns The application-level capture engine. */
    CaptureEngine* engine() const;

private:
    void takeSnapshotWithMode(SnapshotMode mode);
    void executeSnapshot(const QRect& cropRect = {});
    QString nextSnapshotPath() const;

    void onRegionSelected(QRect globalRect);
    void onSelectionCancelled();
    void openMediaFile();

private:
    CaptureEngine* _engine = nullptr;
    ScreenCapturePreview* _preview = nullptr;
    QToolButton* _snapshotButton = nullptr;
    QTimer* _countdownTimer = nullptr;
    SnapshotMode _snapshotMode = SnapshotMode::Fullscreen;
    QList<RegionSelectionOverlay*> _overlays;
    int _countdownRemaining = 0;
    QWidget* _mediaWindow = nullptr;
    SourceDisplay* _sourceDisplay = nullptr;
};

} // namespace pulse
