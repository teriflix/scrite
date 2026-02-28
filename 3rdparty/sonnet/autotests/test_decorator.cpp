/*
 * SPDX-FileCopyrightText: 2025 Espen Sandøy Hustad <espen@ehustad.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "test_decorator.h"

#include <QPlainTextEdit>
#include <QScopedPointer>
#include <QTest>
#include <QTextEdit>

#include <Sonnet/SpellCheckDecorator>

QTEST_MAIN(SonnetDecoratorTest)

/*
 * This unit test was added because of https://bugs.kde.org/show_bug.cgi?id=492444
 * The main purpose is to test that there are no segfaults on object destruction
 */
void SonnetDecoratorTest::testDestroyTextDecorator()
{
    QScopedPointer<QTextEdit, QScopedPointerDeleteLater> parent(new QTextEdit(nullptr));
    Sonnet::SpellCheckDecorator *decorator = new Sonnet::SpellCheckDecorator(parent.get());
    decorator->deleteLater();
}

void SonnetDecoratorTest::testDestroyPlainTextDecorator()
{
    QScopedPointer<QPlainTextEdit, QScopedPointerDeleteLater> parent(new QPlainTextEdit(nullptr));
    Sonnet::SpellCheckDecorator *decorator = new Sonnet::SpellCheckDecorator(parent.get());
    decorator->deleteLater();
}

// NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
void SonnetDecoratorTest::testDestroyTextEdit()
{
    QScopedPointer<QTextEdit, QScopedPointerDeleteLater> parent(new QTextEdit(nullptr));
    Sonnet::SpellCheckDecorator *decorator = new Sonnet::SpellCheckDecorator(parent.get());
    Q_UNUSED(decorator)
}

void SonnetDecoratorTest::testDestroyPlainTextEdit()
{
    QScopedPointer<QPlainTextEdit, QScopedPointerDeleteLater> parent(new QPlainTextEdit(nullptr));
    Sonnet::SpellCheckDecorator *decorator = new Sonnet::SpellCheckDecorator(parent.get());
    Q_UNUSED(decorator)
}
// NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)
