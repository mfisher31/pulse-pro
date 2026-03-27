// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QApplication>

#include "screencapturepreview.hpp"

int main(int argc, char *argv[])
{
    QApplication app (argc, argv);
    ScreenCapturePreview screenCapturePreview;
    screenCapturePreview.show();
    return app.exec();
}
