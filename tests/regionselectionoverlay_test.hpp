// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>

class RegionSelectionOverlayTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void escapeKey_emitsSelectionCancelled();
    void tinyDrag_emitsSelectionCancelled();
    void validDrag_emitsRegionSelected();
};
