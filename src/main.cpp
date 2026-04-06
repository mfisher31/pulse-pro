// Copyright (C) 2026 Medical Informatics Engineering.
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QApplication>
#include <QSettings>

#include "appcontroller.hpp"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("Medical Informatics Engineering");
    app.setOrganizationDomain("mieweb.org");
    app.setApplicationName("PulsePro");
    app.setApplicationVersion("0.1.0");
    app.setQuitOnLastWindowClosed(false);

    pulse::AppController controller;
    controller.start();

    return app.exec();
}
