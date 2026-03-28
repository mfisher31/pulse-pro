// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SCREENCAPTUREPREVIEW_H
#define SCREENCAPTUREPREVIEW_H

#include <QItemSelection>
#include <QScreenCapture>
#include <QWidget>
#include <QWindowCapture>

class RegionSelectionOverlay;
class ScreenListModel;
class WindowListModel;

QT_BEGIN_NAMESPACE
class QGraphicsScene;
class QGraphicsVideoItem;
class QGraphicsView;
class QGridLayout;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QListView;
class QMediaCaptureSession;
class QMediaRecorder;
class QPushButton;
class QTimer;
QT_END_NAMESPACE

QT_USE_NAMESPACE

class ScreenCapturePreview : public QWidget
{
    Q_OBJECT

public:
    explicit ScreenCapturePreview(QWidget *parent = nullptr);
    ~ScreenCapturePreview() override;

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onCurrentScreenSelectionChanged(QItemSelection index);
    void onCurrentWindowSelectionChanged(QItemSelection index);
    void onWindowCaptureErrorChanged();
    void onScreenCaptureErrorChanged();
    void onStartStopButtonClicked();
    void onRecordButtonClicked();
    void onSelectRegionClicked();
    void onRegionSelected(QRect globalRect);
    void onSelectionCancelled();

private:
    enum class SourceType { Screen, Window };

    void updateActive(SourceType sourceType, bool active);
    void updateStartStopButtonText();
    void fitVideoToView();
    bool isActive() const;

private:
    ScreenListModel *screenList = nullptr;
    WindowListModel *windowList = nullptr;
    QListView *screenListView = nullptr;
    QListView *windowListView = nullptr;
    QScreenCapture *screenCapture = nullptr;
    QWindowCapture *windowCapture = nullptr;
    QMediaCaptureSession *mediaCaptureSession = nullptr;
    QGraphicsScene *graphicsScene = nullptr;
    QGraphicsVideoItem *graphicsVideoItem = nullptr;
    QGraphicsView *graphicsView = nullptr;
    QGridLayout *gridLayout = nullptr;
    QPushButton *startStopButton = nullptr;
    QPushButton *recordButton = nullptr;
    QPushButton *selectRegionButton = nullptr;
    QLabel *regionLabel = nullptr;
    QMediaRecorder *mediaRecorder = nullptr;
    QLabel *screenLabel = nullptr;
    QLabel *windowLabel = nullptr;
    QLabel *videoWidgetLabel = nullptr;
    SourceType sourceType = SourceType::Screen;
    QRect _selectedRegion;
    QList<RegionSelectionOverlay *> _overlays;
};

#endif // SCREENCAPTUREPREVIEW_H
