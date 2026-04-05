// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QAudioOutput>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsVideoItem>
#include <QGraphicsView>
#include <QMediaPlayer>
#include <QPixmap>
#include <QVideoSink>

#include "sourcedisplay.hpp"

namespace pulse {

SourceDisplay::SourceDisplay(QWidget* parent)
    : GraphicsDisplay(parent)
    , _videoItem(new QGraphicsVideoItem())
    , _pixmapItem(new QGraphicsPixmapItem())
    , _player(new QMediaPlayer(this))
    , _audioOutput(new QAudioOutput(this))
{
    _player->setAudioOutput(_audioOutput);
    _player->setVideoOutput(_videoItem->videoSink());

    connect(_videoItem, &QGraphicsVideoItem::nativeSizeChanged, this, [this]() {
        fitContentToView();
    });
    connect(_player,
            &QMediaPlayer::playbackStateChanged,
            this,
            &SourceDisplay::playbackStateChanged);
    connect(_player, &QMediaPlayer::errorOccurred, this, [this]() {
        emit errorOccurred(_player->errorString());
    });
}

SourceDisplay::~SourceDisplay()
{
    // Items not currently in the scene are not owned by it; delete explicitly.
    if (!_videoItem->scene())
         delete _videoItem;
    if (!_pixmapItem->scene())
        delete _pixmapItem;
}

SourceDisplay::SourceType SourceDisplay::sourceType() const
{
    return _sourceType;
}

void SourceDisplay::setVideoSource(const QString& filePath)
{
    clearSceneItems();
    _sourceType = SourceType::Video;
    scene()->addItem(_videoItem);
    _player->setSource(QUrl::fromLocalFile(filePath));
    _player->play();
}

void SourceDisplay::setImageSource(const QString& filePath)
{
    const QPixmap pixmap(filePath);
    if (pixmap.isNull())
        return;

    clearSceneItems();
    _sourceType = SourceType::Image;
    _pixmapItem->setPixmap(pixmap);
    scene()->addItem(_pixmapItem);
    fitContentToView();
}

void SourceDisplay::clearSource()
{
    _player->stop();
    _player->setSource({});
    clearSceneItems();
    _sourceType = SourceType::None;
}

void SourceDisplay::fitContentToView()
{
    const QSizeF viewSize = view()->viewport()->size();

    switch (_sourceType) {
    case SourceType::Video: {
        const QSizeF native = _videoItem->nativeSize();
        if (native.isEmpty())
            return;
        const QSizeF fitted = native.scaled(viewSize, Qt::KeepAspectRatio);
        _videoItem->setSize(fitted);
        _videoItem->setPos((viewSize.width() - fitted.width()) / 2.0,
                           (viewSize.height() - fitted.height()) / 2.0);
        break;
    }
    case SourceType::Image: {
        const QSizeF native = _pixmapItem->pixmap().size();
        if (native.isEmpty())
            return;
        const QSizeF fitted = native.scaled(viewSize, Qt::KeepAspectRatio);
        const double scale = fitted.width() / native.width();
        _pixmapItem->setScale(scale);
        _pixmapItem->setPos((viewSize.width() - fitted.width()) / 2.0,
                            (viewSize.height() - fitted.height()) / 2.0);
        break;
    }
    case SourceType::None:
        break;
    }
}

void SourceDisplay::clearSceneItems()
{
    // Remove without deleting — items are owned by this class, not the scene.
    scene()->removeItem(_videoItem);
    scene()->removeItem(_pixmapItem);
}

} // namespace pulse
