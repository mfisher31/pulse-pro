// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QMediaRecorder>
#include <QObject>

QT_BEGIN_NAMESPACE
class QAudioDevice;
class QAudioInput;
class QCapturableWindow;
class QMediaCaptureSession;
class QScreen;
class QScreenCapture;
class QVideoSink;
class QWindowCapture;
QT_END_NAMESPACE

QT_USE_NAMESPACE

namespace pulse {

/**
    Manages the screen/window capture pipeline, audio input, and media recording.

    Owns QScreenCapture, QWindowCapture, QMediaCaptureSession, QMediaRecorder,
    and QAudioInput. The display-side video output is injected via setVideoOutput()
    so that the engine remains independent of any particular widget hierarchy.

    @note Audio is disabled by default. Call setAudioDevice() to enable it.
*/
class CaptureEngine : public QObject
{
    Q_OBJECT

public:
    /** Identifies the active capture source. */
    enum class SourceType
    {
        Screen,
        Window
    };

    explicit CaptureEngine(QObject* parent = nullptr);

    /**
        Connects a downstream video sink to receive frames from the capture session.

        The engine inserts its own QVideoSink between the session and the
        downstream sink so it can hold the last frame for snapshot use.

        @param sink The target sink (e.g. QGraphicsVideoItem::videoSink()).
    */
    void setVideoOutput(QVideoSink* sink);

    /**
        Captures the most recently received video frame and saves it as an image.

        The output format is inferred from the file extension (.png, .jpg, etc.).
        Safe to call at any time; no-op if no frame has been received yet.

        @param filePath Absolute path for the output image file.
    */
    void takeSnapshot(const QString& filePath);

    /**
        Sets the screen to capture when source type is Screen.

        @param screen The target screen, or nullptr to clear.
    */
    void setScreen(QScreen* screen);

    /**
        Sets the window to capture when source type is Window.

        @param window The target capturable window.
    */
    void setWindow(const QCapturableWindow& window);

    /**
        Activates or deactivates capture for the specified source type.

        @param sourceType Which source to control.
        @param active     Whether to start (true) or stop (false) capture.
    */
    void setActive(SourceType sourceType, bool active);

    /** @returns Whether the current source type is actively capturing. */
    bool isActive() const;

    /** @returns The currently selected source type. */
    SourceType sourceType() const;

    /**
        Enables or disables audio input in the capture session.

        @param enabled Pass false for video-only recording.
    */
    void setAudioEnabled(bool enabled);

    /**
        Selects the audio input device and enables audio in the session.

        @param device The device to use. Obtain candidates from QMediaDevices::audioInputs().
    */
    void setAudioDevice(const QAudioDevice& device);

    /**
        Starts recording to the specified file path.

        @param filePath Absolute path for the output MP4 file.
    */
    void startRecording(const QString& filePath);

    /** Stops an in-progress recording. */
    void stopRecording();

    /** @returns The current recorder state. */
    QMediaRecorder::RecorderState recorderState() const;

signals:
    /** Emitted whenever the active/inactive state of the current capture source changes. */
    void activeChanged();

    /** Emitted when the recorder transitions between states. */
    void recorderStateChanged(QMediaRecorder::RecorderState state);

    /** Emitted when any capture or recorder error occurs. */
    void errorOccurred(const QString& message);

private:
    QScreenCapture* _screenCapture = nullptr;
    QWindowCapture* _windowCapture = nullptr;
    QMediaCaptureSession* _captureSession = nullptr;
    QMediaRecorder* _recorder = nullptr;
    QAudioInput* _audioInput = nullptr;
    QVideoSink* _videoSink = nullptr;
    SourceType _sourceType = SourceType::Screen;
};

} // namespace pulse
