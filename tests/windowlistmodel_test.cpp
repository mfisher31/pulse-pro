// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QSignalSpy>
#include <QTest>

#include "windowlistmodel.hpp"
#include "windowlistmodel_test.hpp"

using pulse::WindowListModel;

void WindowListModelTest::rowCount_nonNegative()
{
    WindowListModel model;
    QVERIFY(model.rowCount() >= 0);
}

void WindowListModelTest::data_nonDisplayRole_returnsInvalidVariant()
{
    WindowListModel model;
    if (model.rowCount() == 0)
        QSKIP("No capturable windows available in this environment");

    const QModelIndex idx = model.index(0);
    QVERIFY(!model.data(idx, Qt::UserRole).isValid());
}

void WindowListModelTest::populate_emitsModelReset()
{
    WindowListModel model;
    QSignalSpy spy(&model, &WindowListModel::modelReset);
    model.populate();
    QCOMPARE(spy.count(), 1);
}
