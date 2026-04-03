// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QItemSelection>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QComboBox;
class QGraphicsScene;
class QGraphicsVideoItem;
class QGraphicsView;
class QGridLayout;
class QLabel;
class QListView;
class QPushButton;
QT_END_NAMESPACE

QT_USE_NAMESPACE

namespace pulse {

class CaptureEngine;
class RegionSelectionOverlay;
class ScreenListModel;
class WindowListModel;

/**
    Main application window for Pulse Pro.

    Owns the display pipeline (QGraphicsScene, QGraphicsVideoItem, QGraphicsView)
    and all UI controls. Delegates capture source management, audio, and recording
    to CaptureEngine.
*/
class ScreenCapturePreview : public QWidget
{
    Q_OBJECT

public:
    explicit ScreenCapturePreview(QWidget* parent = nullptr);
    ~ScreenCapturePreview() override;

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onCurrentScreenSelectionChanged(QItemSelection selection);
    void onCurrentWindowSelectionChanged(QItemSelection selection);
    void onStartStopButtonClicked();
    void onRecordButtonClicked();
    void onSelectRegionClicked();
    void onRegionSelected(QRect globalRect);
    void onSelectionCancelled();
    void onAudioDeviceChanged(int index);

private:
    void updateStartStopButtonText();
    void fitVideoToView();
    void populateAudioDevices();

private:
    CaptureEngine* _engine = nullptr;
    ScreenListModel* _screenList = nullptr;
    WindowListModel* _windowList = nullptr;
    QListView* _screenListView = nullptr;
    QListView* _windowListView = nullptr;
    QGraphicsScene* _graphicsScene = nullptr;
    QGraphicsVideoItem* _graphicsVideoItem = nullptr;
    QGraphicsView* _graphicsView = nullptr;
    QGridLayout* _gridLayout = nullptr;
    QPushButton* _startStopButton = nullptr;
    QPushButton* _recordButton = nullptr;
    QPushButton* _selectRegionButton = nullptr;
    QLabel* _regionLabel = nullptr;
    QLabel* _screenLabel = nullptr;
    QLabel* _windowLabel = nullptr;
    QLabel* _videoWidgetLabel = nullptr;
    QLabel* _audioLabel = nullptr;
    QComboBox* _audioDeviceCombo = nullptr;
    QRect _selectedRegion;
    QList<RegionSelectionOverlay*> _overlays;
};

} // namespace pulse
