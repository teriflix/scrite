/*
 * test_highlighter.h
 *
 * SPDX-FileCopyrightText: 2006 Laurent Montel <montel@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KTESTSPELL_H
#define KTESTSPELL_H

#include "highlighter.h"
#include <QTextEdit>

class QContextMenuEvent;
class TestSpell : public QTextEdit
{
    Q_OBJECT
public:
    TestSpell();
public Q_SLOTS:
    void slotActivate();

protected:
    void contextMenuEvent(QContextMenuEvent *) override;
    Sonnet::Highlighter *hl;
};
#endif
