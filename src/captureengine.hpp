// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QMediaRecorder>
#include <QObject>
#include <QRect>

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
        Saves the most recently received video frame as an image.

        Performs a one-shot GPU→CPU download via QVideoFrame::toImage().
        No-op if no frame has been received yet. The output format is inferred
        from the file extension (.png, .jpg, etc.).

        @param filePath  Absolute path for the output image file.
        @param cropRect  If valid, the saved image is cropped to this rectangle
                         before writing.
        @note Do not call in a hot path — toImage() forces a GPU→CPU download.
    */
    void writeVideoFrame(const QString& filePath, const QRect& cropRect = {});

    /**
        Grabs the full display via CGDisplayCreateImage and saves it as an image.

        Uses QScreen::grabWindow(0) which is inherently cursor-free on macOS.
        Falls back to the primary screen if no capture screen is set.
        The output format is inferred from the file extension (.png, .jpg, etc.).

        @param filePath  Absolute path for the output image file.
        @param cropRect  Optional crop in global logical coordinates (as returned by
                         QWidget::mapToGlobal). The method translates to screen-local
                         coordinates and scales by devicePixelRatio before cropping,
                         so callers do not need to account for HiDPI.
    */
    void writeSnapshot(const QString& filePath, const QRect& cropRect = {});

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
        Returns the screen currently set as the capture source, or nullptr if
        the source type is Window or no screen has been set.
    */
    QScreen* currentScreen() const;

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

    /** Emitted by takeSnapshotOnNextFrame() after the file has been saved. */
    void snapshotTaken(const QString& filePath);

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
