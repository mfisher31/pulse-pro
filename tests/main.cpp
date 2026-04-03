// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QApplication>
#include <QTest>

#include "regionselectionoverlay_test.hpp"
#include "screenlistmodel_test.hpp"
#include "windowlistmodel_test.hpp"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    int result = 0;

    ScreenListModelTest screenListModelTest;
    result |= QTest::qExec(&screenListModelTest, argc, argv);

    WindowListModelTest windowListModelTest;
    result |= QTest::qExec(&windowListModelTest, argc, argv);

    RegionSelectionOverlayTest regionSelectionOverlayTest;
    result |= QTest::qExec(&regionSelectionOverlayTest, argc, argv);

    return result;
}
