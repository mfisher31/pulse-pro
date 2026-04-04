// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QMainWindow>

QT_USE_NAMESPACE

namespace pulse {

class CaptureEngine;
class ScreenCapturePreview;

/**
    Top-level application window.

    Owns CaptureEngine and passes it to ScreenCapturePreview. Toolbar and menu
    actions wire directly to the engine so that window-level controls (record,
    stop, settings) do not need to go through the preview widget.
*/
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

    /** @returns The application-level capture engine. */
    CaptureEngine* engine() const;

private:
    CaptureEngine* _engine = nullptr;
    ScreenCapturePreview* _preview = nullptr;
};

} // namespace pulse
