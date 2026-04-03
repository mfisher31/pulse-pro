// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>

class WindowListModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void rowCount_nonNegative();
    void data_nonDisplayRole_returnsInvalidVariant();
    void populate_emitsModelReset();
};
