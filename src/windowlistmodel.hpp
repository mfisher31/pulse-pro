// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QCapturableWindow>

QT_USE_NAMESPACE

class WindowListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit WindowListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QCapturableWindow window(const QModelIndex &index) const;

public Q_SLOTS:
    void populate();

private:
    QList<QCapturableWindow> _windows;
};
