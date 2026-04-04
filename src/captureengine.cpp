// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QAudioDevice>
#include <QAudioInput>
#include <QCapturableWindow>
#include <QMediaCaptureSession>
#include <QMediaFormat>
#include <QScreen>
#include <QScreenCapture>
#include <QUrl>
#include <QVideoFrame>
#include <QVideoSink>
#include <QWindowCapture>

#include "captureengine.hpp"

namespace pulse {

CaptureEngine::CaptureEngine(QObject* parent)
    : QObject(parent)
    , _screenCapture(new QScreenCapture(this))
    , _windowCapture(new QWindowCapture(this))
    , _captureSession(new QMediaCaptureSession(this))
    , _recorder(new QMediaRecorder(this))
    , _audioInput(new QAudioInput(this))
    , _videoSink(new QVideoSink(this))
{
    _captureSession->setScreenCapture(_screenCapture);
    _captureSession->setWindowCapture(_windowCapture);
    _captureSession->setRecorder(_recorder);
    _captureSession->setVideoOutput(_videoSink);
    // Audio is disabled by default; wire it in via setAudioDevice() / setAudioEnabled().

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
    _recorder->setMediaFormat(format);

    connect(_screenCapture, &QScreenCapture::errorChanged, this, [this]() {
        if (_screenCapture->error() != QScreenCapture::NoError)
            emit errorOccurred(tr("QScreenCapture: %1").arg(_screenCapture->errorString()));
    }, Qt::QueuedConnection);

    connect(_windowCapture, &QWindowCapture::errorChanged, this, [this]() {
        if (_windowCapture->error() != QWindowCapture::NoError)
            emit errorOccurred(tr("QWindowCapture: %1").arg(_windowCapture->errorString()));
    }, Qt::QueuedConnection);

    connect(_recorder, &QMediaRecorder::errorChanged, this, [this]() {
        if (_recorder->error() != QMediaRecorder::NoError)
            emit errorOccurred(tr("QMediaRecorder: %1").arg(_recorder->errorString()));
    }, Qt::QueuedConnection);

    connect(_recorder,
            &QMediaRecorder::recorderStateChanged,
            this,
            &CaptureEngine::recorderStateChanged);
}

void CaptureEngine::setVideoOutput(QVideoSink* sink)
{
    // Forward every frame the session delivers to the downstream display sink.
    connect(_videoSink, &QVideoSink::videoFrameChanged, sink, &QVideoSink::setVideoFrame);
}

void CaptureEngine::takeSnapshot(const QString& filePath, const QRect& cropRect)
{
    const QVideoFrame frame = _videoSink->videoFrame();
    if (!frame.isValid())
        return;
    // toImage() is a one-shot GPU→CPU download; acceptable outside hot paths.
    QImage image = frame.toImage();
    if (cropRect.isValid())
        image = image.copy(cropRect);
    image.save(filePath);
}

void CaptureEngine::takeSnapshotOnNextFrame(const QString& filePath, const QRect& cropRect)
{
    // Qt::SingleShotConnection auto-disconnects after the first delivery.
    connect(_videoSink, &QVideoSink::videoFrameChanged, this,
            [this, filePath, cropRect](const QVideoFrame& frame) {
                if (!frame.isValid())
                    return;
                QImage image = frame.toImage();
                if (cropRect.isValid())
                    image = image.copy(cropRect);
                image.save(filePath);
                emit snapshotTaken(filePath);
            }, Qt::SingleShotConnection);
}

void CaptureEngine::setScreen(QScreen* screen)
{
    _screenCapture->setScreen(screen);
}

void CaptureEngine::setWindow(const QCapturableWindow& window)
{
    _windowCapture->setWindow(window);
}

void CaptureEngine::setActive(SourceType sourceType, bool active)
{
    _sourceType = sourceType;
    _screenCapture->setActive(active && _sourceType == SourceType::Screen);
    _windowCapture->setActive(active && _sourceType == SourceType::Window);
    emit activeChanged();
}

bool CaptureEngine::isActive() const
{
    switch (_sourceType) {
    case SourceType::Window:
        return _windowCapture->isActive();
    case SourceType::Screen:
        return _screenCapture->isActive();
    default:
        return false;
    }
}

CaptureEngine::SourceType CaptureEngine::sourceType() const
{
    return _sourceType;
}

QScreen* CaptureEngine::currentScreen() const
{
    return _screenCapture->screen();
}

void CaptureEngine::setAudioEnabled(bool enabled)
{
    _captureSession->setAudioInput(enabled ? _audioInput : nullptr);
}

void CaptureEngine::setAudioDevice(const QAudioDevice& device)
{
    _audioInput->setDevice(device);
    _captureSession->setAudioInput(_audioInput);
}

void CaptureEngine::startRecording(const QString& filePath)
{
    _recorder->setOutputLocation(QUrl::fromLocalFile(filePath));
    _recorder->record();
}

void CaptureEngine::stopRecording()
{
    _recorder->stop();
}

QMediaRecorder::RecorderState CaptureEngine::recorderState() const
{
    return _recorder->recorderState();
}

} // namespace pulse
