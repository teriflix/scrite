/*
 * SPDX-FileCopyrightText: 2006 David Faure <faure@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef TEST_FILTER_H
#define TEST_FILTER_H

#include <QObject>

class SonnetFilterTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testLatin();
    void testIndic();
    void testSentence();
};

#endif
