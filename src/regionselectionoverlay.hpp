// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef REGIONSELECTIONOVERLAY_H
#define REGIONSELECTIONOVERLAY_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QScreen;
QT_END_NAMESPACE

QT_USE_NAMESPACE

class RegionSelectionOverlay : public QWidget
{
    Q_OBJECT

public:
    explicit RegionSelectionOverlay(QScreen *screen, QWidget *parent = nullptr);

signals:
    void regionSelected(QRect globalRect);
    void selectionCancelled();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    QPoint _dragStart;
    QRect _selection;
    bool _dragging = false;
};

#endif // REGIONSELECTIONOVERLAY_H
