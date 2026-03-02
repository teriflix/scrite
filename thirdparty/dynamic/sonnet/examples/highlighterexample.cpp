/**
 * test_highlighter.cpp
 *
 * SPDX-FileCopyrightText: 2004 Zack Rusin <zack@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "highlighterexample.h"

#include <QAction>
#include <QApplication>
#include <QContextMenuEvent>
#include <QDebug>
#include <QMenu>

TestSpell::TestSpell()
    : QTextEdit()
{
    hl = new Sonnet::Highlighter(this);
}

void TestSpell::contextMenuEvent(QContextMenuEvent *e)
{
    qDebug() << "TestSpell::contextMenuEvent";
    QMenu *popup = createStandardContextMenu();
    QMenu *subMenu = new QMenu(popup);
    subMenu->setTitle(QStringLiteral("Text highlighting"));
    connect(subMenu, SIGNAL(triggered(QAction *)), this, SLOT(slotActivate()));
    QAction *action = new QAction(QStringLiteral("active or not"), popup);
    popup->addSeparator();
    popup->addMenu(subMenu);
    subMenu->addAction(action);
    popup->exec(e->globalPos());
    delete popup;
}

void TestSpell::slotActivate()
{
    qDebug() << "Activate or not highlight :";
    hl->setActive(!hl->isActive());
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QTextEdit *test = new TestSpell();
    test->show();

    return app.exec();
}

#include "moc_highlighterexample.cpp"
