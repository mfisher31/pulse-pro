// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QGraphicsScene>
#include <QGraphicsVideoItem>
#include <QGraphicsView>
#include <QResizeEvent>

#include <QAction>
#include <QAudioDevice>
#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QFrame>
#include <QGridLayout>
#include <QGuiApplication>
#include <QLabel>
#include <QListView>
#include <QMediaDevices>
#include <QMediaRecorder>
#include <QMessageBox>
#include <QPushButton>
#include <QTimer>

#include "captureengine.hpp"
#include "regionselectionoverlay.hpp"
#include "screencapturepreview.hpp"
#include "screenlistmodel.hpp"
#include "windowlistmodel.hpp"

namespace pulse {

ScreenCapturePreview::ScreenCapturePreview(CaptureEngine* engine, QWidget* parent)
    : QWidget(parent)
    , _engine(engine)
    , _screenList(new ScreenListModel(this))
    , _windowList(new WindowListModel(this))
    , _screenListView(new QListView(this))
    , _windowListView(new QListView(this))
    , _graphicsScene(new QGraphicsScene(this))
    , _graphicsVideoItem(new QGraphicsVideoItem())
    , _graphicsView(new QGraphicsView(_graphicsScene, this))
    , _gridLayout(new QGridLayout(this))
    , _startStopButton(new QPushButton(this))
    , _recordButton(new QPushButton(tr("Record"), this))
    , _selectRegionButton(new QPushButton(tr("Select Region…"), this))
    , _screenLabel(new QLabel(tr("Select screen to capture:"), this))
    , _windowLabel(new QLabel(tr("Select window to capture:"), this))
    , _videoWidgetLabel(new QLabel(tr("Capture output:"), this))
    , _audioLabel(new QLabel(tr("Audio input:"), this))
    , _audioDeviceCombo(new QComboBox(this))
    , _regionLabel(new QLabel(tr("No region selected"), this))
{
    _engine->setVideoOutput(_graphicsVideoItem);

    _graphicsScene->addItem(_graphicsVideoItem);
    _graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _graphicsView->setFrameStyle(QFrame::NoFrame);
    _graphicsView->setAlignment(Qt::AlignCenter);
    _graphicsView->setBackgroundBrush(Qt::black);

    populateAudioDevices();

    _screenListView->setModel(_screenList);
    _windowListView->setModel(_windowList);

    auto* updateAction = new QAction(tr("Update Windows List"), this);
    connect(updateAction, &QAction::triggered, _windowList, &WindowListModel::populate);
    _windowListView->addAction(updateAction);
    _windowListView->setContextMenuPolicy(Qt::ActionsContextMenu);

    _gridLayout->addWidget(_screenLabel, 0, 0);
    _gridLayout->addWidget(_screenListView, 1, 0);
    _gridLayout->addWidget(_windowLabel, 2, 0);
    _gridLayout->addWidget(_windowListView, 3, 0);
    _gridLayout->addWidget(_startStopButton, 4, 0);
    _gridLayout->addWidget(_recordButton, 5, 0);
    _gridLayout->addWidget(_selectRegionButton, 6, 0);
    _gridLayout->addWidget(_regionLabel, 7, 0);
    _gridLayout->addWidget(_audioLabel, 8, 0);
    _gridLayout->addWidget(_audioDeviceCombo, 9, 0);
    _gridLayout->addWidget(_videoWidgetLabel, 0, 1);
    _gridLayout->addWidget(_graphicsView, 1, 1, 9, 1);

    _gridLayout->setColumnStretch(1, 1);
    _gridLayout->setRowStretch(1, 1);
    _gridLayout->setColumnMinimumWidth(0, 400);
    _gridLayout->setColumnMinimumWidth(1, 400);
    _gridLayout->setRowMinimumHeight(3, 1);

    connect(_screenListView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &ScreenCapturePreview::onCurrentScreenSelectionChanged);
    connect(_windowListView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &ScreenCapturePreview::onCurrentWindowSelectionChanged);
    connect(_startStopButton,
            &QPushButton::clicked,
            this,
            &ScreenCapturePreview::onStartStopButtonClicked);
    connect(_recordButton,
            &QPushButton::clicked,
            this,
            &ScreenCapturePreview::onRecordButtonClicked);
    connect(_selectRegionButton,
            &QPushButton::clicked,
            this,
            &ScreenCapturePreview::onSelectRegionClicked);
    connect(_audioDeviceCombo,
            &QComboBox::currentIndexChanged,
            this,
            &ScreenCapturePreview::onAudioDeviceChanged);

    connect(_engine, &CaptureEngine::activeChanged, this, [this]() {
        updateStartStopButtonText();
    });
    connect(_engine,
            &CaptureEngine::recorderStateChanged,
            this,
            [this](QMediaRecorder::RecorderState state) {
                _recordButton->setText(state == QMediaRecorder::RecordingState
                                           ? tr("Stop Recording")
                                           : tr("Record"));
            });
    connect(_engine, &CaptureEngine::errorOccurred, this, [this](const QString& message) {
        QMessageBox::warning(this, tr("Capture Error"), message);
    });

    connect(_graphicsVideoItem, &QGraphicsVideoItem::nativeSizeChanged, this, [this]() {
        fitVideoToView();
    });

    _engine->setActive(CaptureEngine::SourceType::Screen, true);
}

ScreenCapturePreview::~ScreenCapturePreview() = default;

void ScreenCapturePreview::populateAudioDevices()
{
    _audioDeviceCombo->addItem(tr("No audio"));
    const auto devices = QMediaDevices::audioInputs();
    for (const QAudioDevice& device : devices)
        _audioDeviceCombo->addItem(device.description(), QVariant::fromValue(device));
}

void ScreenCapturePreview::onAudioDeviceChanged(int index)
{
    if (index <= 0) {
        _engine->setAudioEnabled(false);
        return;
    }
    const auto device = _audioDeviceCombo->itemData(index).value<QAudioDevice>();
    _engine->setAudioDevice(device);
}

void ScreenCapturePreview::onCurrentScreenSelectionChanged(QItemSelection selection)
{
    if (auto indexes = selection.indexes(); !indexes.empty()) {
        _engine->setScreen(_screenList->screen(indexes.front()));
        _engine->setActive(CaptureEngine::SourceType::Screen, _engine->isActive());

        _windowListView->clearSelection();
    } else {
        _engine->setScreen(nullptr);
    }
}

void ScreenCapturePreview::onCurrentWindowSelectionChanged(QItemSelection selection)
{
    if (auto indexes = selection.indexes(); !indexes.empty()) {
        auto window = _windowList->window(indexes.front());
        if (!window.isValid()) {
            const auto questionResult = QMessageBox::question(
                this,
                tr("Invalid window"),
                tr("The window is no longer valid. Update the list of windows?"));
            if (questionResult == QMessageBox::Yes) {
                _engine->setActive(CaptureEngine::SourceType::Window, false);

                _windowListView->clearSelection();
                _windowList->populate();
                return;
            }
        }

        _engine->setWindow(window);
        _engine->setActive(CaptureEngine::SourceType::Window, _engine->isActive());

        _screenListView->clearSelection();
    } else {
        _engine->setWindow({});
    }
}

void ScreenCapturePreview::onStartStopButtonClicked()
{
    _engine->setActive(_engine->sourceType(), !_engine->isActive());
}

void ScreenCapturePreview::onRecordButtonClicked()
{
    if (_engine->recorderState() == QMediaRecorder::RecordingState) {
        _engine->stopRecording();
        return;
    }

    const QString filePath = QFileDialog::getSaveFileName(
        this, tr("Save Recording"), QDir::homePath() + "/recording.mp4", tr("MP4 Video (*.mp4)"));
    if (filePath.isEmpty())
        return;

    _engine->startRecording(filePath);
}

void ScreenCapturePreview::onSelectRegionClicked()
{
    hide();
    // Brief delay so the window visually disappears before overlays paint
    QTimer::singleShot(80, this, [this]() {
        const auto screens = QGuiApplication::screens();
        for (QScreen* screen : screens) {
            auto* overlay = new RegionSelectionOverlay(screen);
            connect(overlay,
                    &RegionSelectionOverlay::regionSelected,
                    this,
                    &ScreenCapturePreview::onRegionSelected);
            connect(overlay,
                    &RegionSelectionOverlay::selectionCancelled,
                    this,
                    &ScreenCapturePreview::onSelectionCancelled);
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
    _regionLabel->setText(tr("Region: %1×%2 at (%3, %4)")
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
    _regionLabel->setText(tr("No region selected"));
    show();
}

void ScreenCapturePreview::updateStartStopButtonText()
{
    switch (_engine->sourceType()) {
    case CaptureEngine::SourceType::Window:
        _startStopButton->setText(_engine->isActive() ? tr("Stop window capture")
                                                      : tr("Start window capture"));
        break;
    case CaptureEngine::SourceType::Screen:
        _startStopButton->setText(_engine->isActive() ? tr("Stop screen capture")
                                                      : tr("Start screen capture"));
        break;
    }
}

void ScreenCapturePreview::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    fitVideoToView();
}

void ScreenCapturePreview::fitVideoToView()
{
    const QSizeF native = _graphicsVideoItem->nativeSize();
    if (native.isEmpty())
        return;
    const QSizeF viewSize = _graphicsView->viewport()->size();
    const QSizeF fitted = native.scaled(viewSize, Qt::KeepAspectRatio);
    _graphicsVideoItem->setSize(fitted);
    _graphicsVideoItem->setPos((viewSize.width() - fitted.width()) / 2.0,
                               (viewSize.height() - fitted.height()) / 2.0);
    _graphicsScene->setSceneRect(QRectF(QPointF(), viewSize));
}

} // namespace pulse

