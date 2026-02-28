/*
 * SPDX-FileCopyrightText: 2007 Zack Rusin <zack@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef TEST_CORE_H
#define TEST_CORE_H

#include <QObject>

class SonnetCoreTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCore();
    void testCore2();
};

#endif
