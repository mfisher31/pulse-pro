// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QTest>

#include "screenlistmodel.hpp"
#include "screenlistmodel_test.hpp"

using pulse::ScreenListModel;

void ScreenListModelTest::rowCount_atLeastOneScreen()
{
    ScreenListModel model;
    QVERIFY(model.rowCount() >= 1);
}

void ScreenListModelTest::data_displayRole_returnsNonEmptyString()
{
    ScreenListModel model;
    const QModelIndex idx = model.index(0);
    const QVariant value = model.data(idx, Qt::DisplayRole);
    QVERIFY(value.isValid());
    QVERIFY(!value.toString().isEmpty());
}

void ScreenListModelTest::data_nonDisplayRole_returnsInvalidVariant()
{
    ScreenListModel model;
    const QModelIndex idx = model.index(0);
    QVERIFY(!model.data(idx, Qt::UserRole).isValid());
}

void ScreenListModelTest::screen_validIndex_returnsNonNull()
{
    ScreenListModel model;
    QScreen* screen = model.screen(model.index(0));
    QVERIFY(screen != nullptr);
}
