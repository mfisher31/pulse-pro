// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QMediaPlayer>

#include "graphicsdisplay.hpp"

QT_BEGIN_NAMESPACE
class QAudioOutput;
class QGraphicsPixmapItem;
class QGraphicsVideoItem;
QT_END_NAMESPACE

QT_USE_NAMESPACE

namespace pulse {

/**
    Media source display widget.

    Extends GraphicsDisplay with support for displaying video files and static
    images as scene items. The active source is swapped at runtime via
    setVideoSource() or setImageSource() without recreating the scene.

    Custom source types not yet backed by a QGraphicsItem can be layered into
    the scene directly via the inherited scene() accessor.

    @note For live capture preview (screen/window), use ScreenCapturePreview.
*/
class SourceDisplay : public GraphicsDisplay
{
    Q_OBJECT

public:
    /** Identifies which kind of source is currently loaded. */
    enum class SourceType
    {
        None,   ///< No source loaded.
        Video,  ///< Video file driven by QMediaPlayer.
        Image,  ///< Static image displayed via QGraphicsPixmapItem.
    };

    explicit SourceDisplay(QWidget* parent = nullptr);
    ~SourceDisplay() override;

    /** @returns The type of the currently loaded source. */
    SourceType sourceType() const;

    /**
        Loads and displays a video file. Replaces any current source.

        Playback begins immediately.

        @param filePath Absolute path to the video file (MP4, MOV, etc.).
    */
    void setVideoSource(const QString& filePath);

    /**
        Loads and displays a static image. Replaces any current source.

        @param filePath Absolute path to the image file (PNG, JPEG, TIFF, etc.).
    */
    void setImageSource(const QString& filePath);

    /** Clears the current source and resets the scene to black. */
    void clearSource();

signals:
    /** Emitted when the media player's playback state changes. */
    void playbackStateChanged(QMediaPlayer::PlaybackState state);

    /** Emitted if the media player reports an error. */
    void errorOccurred(const QString& message);

protected:
    void fitContentToView() override;

private:
    void clearSceneItems();

private:
    QGraphicsVideoItem* _videoItem = nullptr;
    QGraphicsPixmapItem* _pixmapItem = nullptr;
    QMediaPlayer* _player = nullptr;
    QAudioOutput* _audioOutput = nullptr;
    SourceType _sourceType = SourceType::None;
};

} // namespace pulse
