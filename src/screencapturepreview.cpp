// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QMediaCaptureSession>
#include <QScreenCapture>
#include <QGraphicsScene>
#include <QGraphicsVideoItem>
#include <QGraphicsView>
#include <QResizeEvent>

#include <QAction>
#include <QDir>
#include <QFileDialog>
#include <QFrame>
#include <QGridLayout>
#include <QGuiApplication>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMediaFormat>
#include <QMediaRecorder>
#include <QMessageBox>
#include <QPushButton>
#include <QTimer>
#include <QUrl>

#include "regionselectionoverlay.hpp"
#include "screencapturepreview.hpp"
#include "screenlistmodel.hpp"
#include "windowlistmodel.hpp"

namespace pulse {

ScreenCapturePreview::ScreenCapturePreview(QWidget *parent)
    : QWidget(parent),
      screenListView(new QListView(this)),
      windowListView(new QListView(this)),
      screenCapture(new QScreenCapture(this)),
      windowCapture(new QWindowCapture(this)),
      mediaCaptureSession(new QMediaCaptureSession(this)),
      graphicsScene(new QGraphicsScene(this)),
      graphicsVideoItem(new QGraphicsVideoItem ()),
      graphicsView(new QGraphicsView(graphicsScene, this)),
      gridLayout(new QGridLayout(this)),
      startStopButton(new QPushButton(this)),
      screenLabel(new QLabel(tr("Select screen to capture:"), this)),
      windowLabel(new QLabel(tr("Select window to capture:"), this)),
      videoWidgetLabel(new QLabel(tr("Capture output:"), this)),
      mediaRecorder(new QMediaRecorder(this)),
      recordButton(new QPushButton(tr("Record"), this)),
      selectRegionButton(new QPushButton(tr("Select Region…"), this)),
      regionLabel(new QLabel(tr("No region selected"), this))
{
    // Get lists of screens and windows:
    screenList = new ScreenListModel(this);
    windowList = new WindowListModel(this);

    // Setup QScreenCapture with initial source:

    mediaCaptureSession->setScreenCapture(screenCapture);
    mediaCaptureSession->setWindowCapture(windowCapture);

    graphicsScene->addItem(graphicsVideoItem);
    graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    graphicsView->setFrameStyle(QFrame::NoFrame);
    graphicsView->setAlignment(Qt::AlignCenter);
    graphicsView->setBackgroundBrush(Qt::black);

    mediaCaptureSession->setVideoOutput(graphicsVideoItem);

    // All three (fileFormat, videoCodec, audioCodec) must be specified so that
    // QMediaRecorder::record() does not re-resolve the format and override our
    // choices. If any field is unspecified, Qt calls resolveFormat(RequiresVideo)
    // internally and will pick whatever it deems "best" — on Linux that resolves
    // to hevc_vaapi, which fails when the VAAPI driver lacks an HEVC profile.
    //
    // On Linux, Qt's bundled FFmpeg omits libx264 (GPL), so H.264 software
    // encoding is unavailable. MPEG-4 Part 2 ("mpeg4" muxer) is always present
    // as a pure-software path. AAC is built into Qt's FFmpeg on all platforms.
    QMediaFormat format;
    format.setFileFormat(QMediaFormat::MPEG4);
    format.setAudioCodec(QMediaFormat::AudioCodec::AAC);
#ifdef Q_OS_LINUX
    // Use MPEG-4 Part 2 — guaranteed software encoder, no VAAPI required.
    format.setVideoCodec(QMediaFormat::VideoCodec::MPEG4);
#else
    format.setVideoCodec(QMediaFormat::VideoCodec::H264);
#endif
    mediaRecorder->setMediaFormat(format);
    mediaCaptureSession->setRecorder(mediaRecorder);

    // Setup UI:
    screenListView->setModel(screenList);
    windowListView->setModel(windowList);

    auto updateAction = new QAction(tr("Update Windows List"), this);
    connect(updateAction, &QAction::triggered, windowList, &WindowListModel::populate);
    windowListView->addAction(updateAction);
    windowListView->setContextMenuPolicy(Qt::ActionsContextMenu);

    gridLayout->addWidget(screenLabel, 0, 0);
    gridLayout->addWidget(screenListView, 1, 0);
    gridLayout->addWidget(windowLabel, 2, 0);
    gridLayout->addWidget(windowListView, 3, 0);
    gridLayout->addWidget(startStopButton, 4, 0);
    gridLayout->addWidget(recordButton, 5, 0);
    gridLayout->addWidget(selectRegionButton, 6, 0);
    gridLayout->addWidget(regionLabel, 7, 0);
    gridLayout->addWidget(videoWidgetLabel, 0, 1);
    gridLayout->addWidget(graphicsView, 1, 1, 7, 1);

    gridLayout->setColumnStretch(1, 1);
    gridLayout->setRowStretch(1, 1);
    gridLayout->setColumnMinimumWidth(0, 400);
    gridLayout->setColumnMinimumWidth(1, 400);
    gridLayout->setRowMinimumHeight(3, 1);

    connect(screenListView->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            &ScreenCapturePreview::onCurrentScreenSelectionChanged);
    connect(windowListView->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            &ScreenCapturePreview::onCurrentWindowSelectionChanged);
    connect(startStopButton, &QPushButton::clicked, this,
            &ScreenCapturePreview::onStartStopButtonClicked);
    connect(screenCapture, &QScreenCapture::errorChanged, this,
            &ScreenCapturePreview::onScreenCaptureErrorChanged, Qt::QueuedConnection);
    connect(windowCapture, &QWindowCapture::errorChanged, this,
            &ScreenCapturePreview::onWindowCaptureErrorChanged, Qt::QueuedConnection);
    connect(recordButton, &QPushButton::clicked, this,
            &ScreenCapturePreview::onRecordButtonClicked);
    connect(mediaRecorder, &QMediaRecorder::errorChanged, this, [this]() {
        if (mediaRecorder->error() != QMediaRecorder::NoError)
            QMessageBox::warning(this, tr("QMediaRecorder: Error occurred"),
                                 mediaRecorder->errorString());
    }, Qt::QueuedConnection);
    connect(mediaRecorder, &QMediaRecorder::recorderStateChanged, this,
            [this](QMediaRecorder::RecorderState state) {
                recordButton->setText(state == QMediaRecorder::RecordingState
                                          ? tr("Stop Recording")
                                          : tr("Record"));
            });

    connect(graphicsVideoItem, &QGraphicsVideoItem::nativeSizeChanged, this,
            [this]() { fitVideoToView(); });
    connect(selectRegionButton, &QPushButton::clicked, this,
            &ScreenCapturePreview::onSelectRegionClicked);

    updateActive(SourceType::Screen, true);
}

void ScreenCapturePreview::onSelectRegionClicked()
{
    hide();
    // Brief delay so the window visually disappears before overlays paint
    QTimer::singleShot(80, this, [this]() {
        const auto screens = QGuiApplication::screens();
        for (QScreen *screen : screens) {
            auto *overlay = new RegionSelectionOverlay(screen);
            connect(overlay, &RegionSelectionOverlay::regionSelected,
                    this, &ScreenCapturePreview::onRegionSelected);
            connect(overlay, &RegionSelectionOverlay::selectionCancelled,
                    this, &ScreenCapturePreview::onSelectionCancelled);
            _overlays.append(overlay);
        }
        if (!_overlays.isEmpty())
            _overlays.first()->setFocus();
    });
}

void ScreenCapturePreview::onRegionSelected(QRect globalRect)
{
    _selectedRegion = globalRect;
    qDeleteAll(_overlays);
    _overlays.clear();
    show();
    regionLabel->setText(tr("Region: %1×%2 at (%3, %4)")
                             .arg(_selectedRegion.width())
                             .arg(_selectedRegion.height())
                             .arg(_selectedRegion.x())
                             .arg(_selectedRegion.y()));
}

void ScreenCapturePreview::onSelectionCancelled()
{
    qDeleteAll(_overlays);
    _overlays.clear();
    _selectedRegion = QRect();
    regionLabel->setText(tr("No region selected"));
    show();
}

ScreenCapturePreview::~ScreenCapturePreview() = default;

void ScreenCapturePreview::onCurrentScreenSelectionChanged(QItemSelection selection)
{
    if (auto indexes = selection.indexes(); !indexes.empty()) {
        screenCapture->setScreen(screenList->screen(indexes.front()));
        updateActive(SourceType::Screen, isActive());

        windowListView->clearSelection();
    } else {
        screenCapture->setScreen(nullptr);
    }
}

void ScreenCapturePreview::onCurrentWindowSelectionChanged(QItemSelection selection)
{
    if (auto indexes = selection.indexes(); !indexes.empty()) {
        auto window = windowList->window(indexes.front());
        if (!window.isValid()) {
            const auto questionResult = QMessageBox::question(
                    this, tr("Invalid window"),
                    tr("The window is no longer valid. Update the list of windows?"));
            if (questionResult == QMessageBox::Yes) {
                updateActive(SourceType::Window, false);

                windowListView->clearSelection();
                windowList->populate();
                return;
            }
        }

        windowCapture->setWindow(window);
        updateActive(SourceType::Window, isActive());

        screenListView->clearSelection();
    } else {
        windowCapture->setWindow({});
    }
}

void ScreenCapturePreview::onWindowCaptureErrorChanged()
{
    if (windowCapture->error() == QWindowCapture::NoError)
        return;

    QMessageBox::warning(this, tr("QWindowCapture: Error occurred"), windowCapture->errorString());
}

void ScreenCapturePreview::onScreenCaptureErrorChanged()
{
    if (screenCapture->error() == QScreenCapture::NoError)
        return;

    QMessageBox::warning(this, tr("QScreenCapture: Error occurred"), screenCapture->errorString());
}

void ScreenCapturePreview::onStartStopButtonClicked()
{
    updateActive(sourceType, !isActive());
}

void ScreenCapturePreview::updateStartStopButtonText()
{
    switch (sourceType) {
    case SourceType::Window:
        startStopButton->setText(isActive() ? tr("Stop window capture")
                                            : tr("Start window capture"));
        break;
    case SourceType::Screen:
        startStopButton->setText(isActive() ? tr("Stop screen capture")
                                            : tr("Start screen capture"));
        break;
    }
}

void ScreenCapturePreview::updateActive(SourceType sourceType, bool active)
{
    this->sourceType = sourceType;

    screenCapture->setActive(active && sourceType == SourceType::Screen);
    windowCapture->setActive(active && sourceType == SourceType::Window);

    updateStartStopButtonText();
}

bool ScreenCapturePreview::isActive() const
{
    switch (sourceType) {
    case SourceType::Window:
        return windowCapture->isActive();
    case SourceType::Screen:
        return screenCapture->isActive();
    default:
        return false;
    }
}

void ScreenCapturePreview::onRecordButtonClicked()
{
    if (mediaRecorder->recorderState() == QMediaRecorder::RecordingState) {
        mediaRecorder->stop();
        return;
    }

    const QString filePath = QFileDialog::getSaveFileName(
        this, tr("Save Recording"),
        QDir::homePath() + "/recording.mp4",
        tr("MP4 Video (*.mp4)"));
    if (filePath.isEmpty())
        return;

    mediaRecorder->setOutputLocation(QUrl::fromLocalFile(filePath));
    mediaRecorder->record();
}

void ScreenCapturePreview::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    fitVideoToView();
}

void ScreenCapturePreview::fitVideoToView()
{
    const QSizeF native = graphicsVideoItem->nativeSize();
    if (native.isEmpty())
        return;
    const QSizeF viewSize = graphicsView->viewport()->size();
    const QSizeF fitted = native.scaled(viewSize, Qt::KeepAspectRatio);
    graphicsVideoItem->setSize(fitted);
    graphicsVideoItem->setPos((viewSize.width() - fitted.width()) / 2.0,
                              (viewSize.height() - fitted.height()) / 2.0);
    graphicsScene->setSceneRect(QRectF(QPointF(), viewSize));
}

}
