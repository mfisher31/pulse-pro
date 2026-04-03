// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>

class ScreenListModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void rowCount_atLeastOneScreen();
    void data_displayRole_returnsNonEmptyString();
    void data_nonDisplayRole_returnsInvalidVariant();
    void screen_validIndex_returnsNonNull();
};
