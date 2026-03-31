// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QGuiApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>

#include "regionselectionoverlay.hpp"

#ifdef Q_OS_MACOS
#include "macoswindowutils.h"
#endif

namespace pulse {

RegionSelectionOverlay::RegionSelectionOverlay(QScreen* screen, QWidget* parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setCursor(Qt::CrossCursor);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setGeometry(screen->geometry());
    show();

#ifdef Q_OS_MACOS
    pulse::applyOverlayWindowBehavior(static_cast<quintptr>(winId()));
#endif
}

void RegionSelectionOverlay::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    // Dark scrim over entire overlay
    painter.fillRect(rect(), QColor(0, 0, 0, 140));

    if (!_selection.isEmpty()) {
        // Punch a transparent hole through the scrim where the selection is
        painter.setCompositionMode(QPainter::CompositionMode_Clear);
        painter.fillRect(_selection, Qt::transparent);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

        // Selection border
        painter.setPen(QPen(QColor(80, 200, 255), 1));
        painter.drawRect(_selection.adjusted(0, 0, -1, -1));
    }

    // Crosshair lines from current cursor position to edges
    const QPoint cursor = mapFromGlobal(QCursor::pos());
    painter.setPen(QPen(QColor(255, 255, 255, 100), 1));
    painter.drawLine(0, cursor.y(), width(), cursor.y());
    painter.drawLine(cursor.x(), 0, cursor.x(), height());
}

void RegionSelectionOverlay::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
        return;
    _dragStart = event->pos();
    _selection = QRect();
    _dragging = true;
    update();
}

void RegionSelectionOverlay::mouseMoveEvent(QMouseEvent* event)
{
    if (_dragging)
        _selection = QRect(_dragStart, event->pos()).normalized();
    update();
}

void RegionSelectionOverlay::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton || !_dragging)
        return;
    _dragging = false;

    _selection = QRect(_dragStart, event->pos()).normalized();

    if (_selection.width() < 4 || _selection.height() < 4) {
        // Treat tiny drags as accidental clicks — cancel
        emit selectionCancelled();
        return;
    }

    // Convert local coords to global screen coordinates
    const QRect globalRect
        = QRect(mapToGlobal(_selection.topLeft()), mapToGlobal(_selection.bottomRight()));
    emit regionSelected(globalRect);
}

void RegionSelectionOverlay::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
        emit selectionCancelled();
    else
        QWidget::keyPressEvent(event);
}

}
