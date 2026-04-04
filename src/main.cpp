// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QApplication>

#include "mainwindow.hpp"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    pulse::MainWindow window;
    window.show();
    return app.exec();
}
