/*
 * test_dialog.h
 *
 * SPDX-FileCopyrightText: 2004 Zack Rusin <zack@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef TEST_DIALOG_H
#define TEST_DIALOG_H

#include "dialog.h"

#include <QObject>

class TestDialog : public QObject
{
    Q_OBJECT
public:
    TestDialog();

public Q_SLOTS:
    void check(const QString &buffer);
    void doneChecking(const QString &);
};

#endif
