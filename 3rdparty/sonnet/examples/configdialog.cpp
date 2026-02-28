/**
 * test_configdialog.cpp
 *
 * SPDX-FileCopyrightText: 2004 Zack Rusin <zack@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "configdialog.h"
#include "speller.h"

#include <QApplication>

using namespace Sonnet;

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    ConfigDialog *dialog = new ConfigDialog(nullptr);

    dialog->show();

    return app.exec();
}
