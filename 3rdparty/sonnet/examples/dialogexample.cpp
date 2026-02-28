// krazy:excludeall=spelling
/**
 * test_dialog.cpp
 *
 * SPDX-FileCopyrightText: 2004 Zack Rusin <zack@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "dialogexample.h"

#include "backgroundchecker.h"

#include <QApplication>
#include <QDebug>
using namespace Sonnet;

TestDialog::TestDialog()
    : QObject(nullptr)
{
}

//@@snippet_begin(dictionary_combo_box_example)
void TestDialog::check(const QString &buffer)
{
    Sonnet::Dialog *dlg = new Sonnet::Dialog(new BackgroundChecker(this), nullptr);
    connect(dlg, &Dialog::spellCheckDone, this, &TestDialog::doneChecking);
    dlg->setBuffer(buffer);
    dlg->show();
}

//@@snippet_end

void TestDialog::doneChecking(const QString &buf)
{
    qDebug() << "Done with :" << buf;
    qApp->quit();
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    TestDialog test;
    test.check(
        QString::fromLatin1("This is a sample buffer. Whih this thingg will "
                            "be checkin for misstakes. Whih, Enviroment, govermant. Whih."));

    return app.exec();
}

#include "moc_dialogexample.cpp"
