// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QGuiApplication>
#include <QScreen>

#include <QTextStream>

#include "screenlistmodel.hpp"

namespace pulse {

ScreenListModel::ScreenListModel(QObject* parent)
    : QAbstractListModel(parent)
{
    auto* app = qApp;
    connect(app, &QGuiApplication::screenAdded, this, &ScreenListModel::screensChanged);
    connect(app, &QGuiApplication::screenRemoved, this, &ScreenListModel::screensChanged);
    connect(app, &QGuiApplication::primaryScreenChanged, this, &ScreenListModel::screensChanged);
}

int ScreenListModel::rowCount(const QModelIndex&) const
{
    return QGuiApplication::screens().size();
}

QVariant ScreenListModel::data(const QModelIndex& index, int role) const
{
    const auto screenList = QGuiApplication::screens();
    Q_ASSERT(index.isValid());
    Q_ASSERT(index.row() <= screenList.size());

    if (role == Qt::DisplayRole) {
        auto* screen = screenList.at(index.row());
        QString description;
        QTextStream str(&description);
        str << '"' << screen->name() << "\" " << screen->size().width() << 'x'
            << screen->size().height() << ", " << screen->logicalDotsPerInch() << "DPI";
        return description;
    }

    return {};
}

QScreen* ScreenListModel::screen(const QModelIndex& index) const
{
    return QGuiApplication::screens().value(index.row());
}

void ScreenListModel::screensChanged()
{
    beginResetModel();
    endResetModel();
}

}
