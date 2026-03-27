// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QWindowCapture>

#include "windowlistmodel.hpp"

WindowListModel::WindowListModel(QObject *parent)
    : QAbstractListModel(parent), _windows(QWindowCapture::capturableWindows())
{
}

int WindowListModel::rowCount(const QModelIndex &) const
{
    return _windows.size();
}

QVariant WindowListModel::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(index.isValid());
    Q_ASSERT(index.row() <= _windows.size());

    if (role == Qt::DisplayRole) {
        auto window = _windows.at(index.row());
        return window.description();
    }

    return {};
}

QCapturableWindow WindowListModel::window(const QModelIndex &index) const
{
    return _windows.at(index.row());
}

void WindowListModel::populate()
{
    beginResetModel();
    _windows = QWindowCapture::capturableWindows();
    endResetModel();
}
