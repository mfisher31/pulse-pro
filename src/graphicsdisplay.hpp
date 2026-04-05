// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QWidget>

QT_BEGIN_NAMESPACE
class QGraphicsScene;
class QGraphicsView;
QT_END_NAMESPACE

QT_USE_NAMESPACE

namespace pulse {

/**
    Base widget providing a GPU-resident QGraphicsScene/QGraphicsView display surface.

    Handles view configuration, layout, and resize propagation. Subclasses
    implement fitContentToView() to reposition and scale their scene items
    whenever the viewport changes. The scene rect is updated automatically
    before fitContentToView() is called.

    Subclasses access the underlying view via the protected view() accessor
    (e.g. to read viewport size inside fitContentToView()).
*/
class GraphicsDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit GraphicsDisplay(QWidget* parent = nullptr);
    ~GraphicsDisplay() override;

    /**
        @returns The QGraphicsScene owned by this widget.

        Callers may add custom QGraphicsItem layers directly to the scene.
    */
    QGraphicsScene* scene() const;

protected:
    /**
        @returns The QGraphicsView owned by this widget.

        Intended for subclasses that need to query viewport geometry inside
        fitContentToView().
    */
    QGraphicsView* view() const;

    /**
        Called after every viewport resize with the scene rect already updated.

        Subclasses should reposition and scale their scene items to match the
        new viewport dimensions.
    */
    virtual void fitContentToView() = 0;

    void resizeEvent(QResizeEvent* event) override;

private:
    QGraphicsScene* _scene = nullptr;
    QGraphicsView* _view = nullptr;
};

} // namespace pulse
