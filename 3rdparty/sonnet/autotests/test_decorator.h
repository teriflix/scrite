/*
 * SPDX-FileCopyrightText: 2025 Espen Sandøy Hustad <espen@ehustad.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef TEST_DECORATOR_H
#define TEST_DECORATOR_H

#include <QObject>

class SonnetDecoratorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testDestroyPlainTextDecorator();
    void testDestroyTextDecorator();
    void testDestroyPlainTextEdit();
    void testDestroyTextEdit();
};

#endif
