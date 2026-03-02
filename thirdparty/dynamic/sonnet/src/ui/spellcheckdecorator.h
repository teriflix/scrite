/*
 * spellcheckdecorator.h
 *
 * SPDX-FileCopyrightText: 2013 Aurélien Gâteau <agateau@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef SPELLCHECKDECORATOR_H
#define SPELLCHECKDECORATOR_H

#include <QObject>

#include <memory>

#include "sonnetui_export.h"

class QTextEdit;
class QPlainTextEdit;

namespace Sonnet
{
class SpellCheckDecoratorPrivate;
class Highlighter;

/*!
 * \class Sonnet::SpellCheckDecorator
 * \inheaderfile Sonnet/SpellCheckDecorator
 * \inmodule KF6Sonnet
 *
 * \brief Connects a Sonnet::Highlighter to a QTextEdit extending the context menu
 * of the text edit with spell check suggestions.
 * \since 5.0
 */

class SONNETUI_EXPORT SpellCheckDecorator : public QObject
{
    Q_OBJECT
public:
    /*!
     * Creates a spell-check decorator.
     *
     * \a textEdit the QTextEdit in need of spell-checking. It also is used as the QObject parent for the decorator.
     */
    explicit SpellCheckDecorator(QTextEdit *textEdit);

    /*!
     * Creates a spell-check decorator.
     *
     * \a textEdit the QPlainTextEdit in need of spell-checking. It also is used as the QObject parent for the decorator.
     * \since 5.12
     */
    explicit SpellCheckDecorator(QPlainTextEdit *textEdit);

    ~SpellCheckDecorator() override;

    /*!
     * Set a custom highlighter on the decorator.
     *
     * SpellCheckDecorator does not take ownership of the new highlighter,
     * and you must manually delete the old highlighter.
     */
    void setHighlighter(Highlighter *highlighter);

    /*!
     * Returns the hightlighter used by the decorator
     */
    Highlighter *highlighter() const;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

    /*!
     * Returns true if the spell checking should be enabled for a given block of text
     * The default implementation always returns true.
     */
    virtual bool isSpellCheckingEnabledForBlock(const QString &textBlock) const;

private:
    friend SpellCheckDecoratorPrivate;
    const std::unique_ptr<SpellCheckDecoratorPrivate> d;

    Q_DISABLE_COPY(SpellCheckDecorator)
};
}

#endif /* SPELLCHECKDECORATOR_H */
