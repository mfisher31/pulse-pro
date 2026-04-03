// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QGuiApplication>
#include <QSignalSpy>
#include <QTest>

#include "regionselectionoverlay.hpp"
#include "regionselectionoverlay_test.hpp"

using pulse::RegionSelectionOverlay;

void RegionSelectionOverlayTest::escapeKey_emitsSelectionCancelled()
{
    auto* overlay = new RegionSelectionOverlay(QGuiApplication::primaryScreen());
    QApplication::processEvents();

    QSignalSpy spy(overlay, &RegionSelectionOverlay::selectionCancelled);
    QTest::keyClick(overlay, Qt::Key_Escape);

    QCOMPARE(spy.count(), 1);
    delete overlay;
}

void RegionSelectionOverlayTest::tinyDrag_emitsSelectionCancelled()
{
    auto* overlay = new RegionSelectionOverlay(QGuiApplication::primaryScreen());
    QApplication::processEvents();

    QSignalSpy spy(overlay, &RegionSelectionOverlay::selectionCancelled);
    QTest::mousePress(overlay, Qt::LeftButton, Qt::NoModifier, QPoint(100, 100));
    QTest::mouseRelease(overlay, Qt::LeftButton, Qt::NoModifier, QPoint(102, 101));

    QCOMPARE(spy.count(), 1);
    delete overlay;
}

void RegionSelectionOverlayTest::validDrag_emitsRegionSelected()
{
    auto* overlay = new RegionSelectionOverlay(QGuiApplication::primaryScreen());
    QApplication::processEvents();

    QSignalSpy spy(overlay, &RegionSelectionOverlay::regionSelected);
    QTest::mousePress(overlay, Qt::LeftButton, Qt::NoModifier, QPoint(100, 100));
    QTest::mouseMove(overlay, QPoint(300, 250));
    QTest::mouseRelease(overlay, Qt::LeftButton, Qt::NoModifier, QPoint(300, 250));

    QCOMPARE(spy.count(), 1);
    const QRect region = spy.at(0).at(0).toRect();
    QVERIFY(region.isValid());

    delete overlay;
}
