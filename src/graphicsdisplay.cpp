// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QFrame>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QResizeEvent>
#include <QVBoxLayout>

#include "graphicsdisplay.hpp"

namespace pulse {

GraphicsDisplay::GraphicsDisplay(QWidget* parent)
    : QWidget(parent)
    , _scene(new QGraphicsScene(this))
    , _view(new QGraphicsView(_scene, this))
{
    _view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _view->setFrameStyle(QFrame::NoFrame);
    _view->setAlignment(Qt::AlignCenter);
    _view->setBackgroundBrush(Qt::black);
    _view->setRenderHint(QPainter::Antialiasing, false);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(_view);
}

GraphicsDisplay::~GraphicsDisplay() = default;

QGraphicsScene* GraphicsDisplay::scene() const
{
    return _scene;
}

QGraphicsView* GraphicsDisplay::view() const
{
    return _view;
}

void GraphicsDisplay::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    const QSizeF viewSize = _view->viewport()->size();
    _scene->setSceneRect(QRectF(QPointF(), viewSize));
    fitContentToView();
}

} // namespace pulse
