/*  This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2009 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ABSTRACTTOKENIZER_H
#define ABSTRACTTOKENIZER_H

#include "sonnetcore_export.h"
#include <QString>

#include <memory>

namespace Sonnet
{
struct Token {
    QStringView token = nullptr;
    int positionInBuffer = -1;

    QString toString() const
    {
        return token.toString();
    }

    Q_DECL_CONSTEXPR int length() const
    {
        return token.size();
    }

    /*
     * position in buffer of which the token is a view.
     */
    Q_DECL_CONSTEXPR int position() const
    {
        return positionInBuffer;
    }

    Q_DECL_CONSTEXPR bool isNull() const
    {
        return token.isNull();
    }

    Q_DECL_CONSTEXPR bool isEmpty() const
    {
        return token.isEmpty();
    }

    Q_DECL_CONSTEXPR QChar at(qsizetype n) const
    {
        return token.at(n);
    }
};

/*!
 * \internal
 * AbstractTokenizer breaks text into smaller pieces - words, sentences, paragraphs.
 *
 * AbstractTokenizer is an abstract class that must be subclassed to be used. It provides API modelled
 * after Java-style iterators. During tokenization buffer can be modified using provided replace() method.
 */
class AbstractTokenizer
{
public:
    virtual ~AbstractTokenizer()
    {
    }

    /*!
     * Sets text to tokenize. It also resets tokenizer state.
     */
    virtual void setBuffer(const QString &buffer = QString()) = 0;
    /*!
     * Returns true if there is another token available.
     * Returns true if another token is available, false if not.
     */
    virtual bool hasNext() const = 0;

    /*!
     * Returns next token or null QString if there is none
     */
    virtual Token next() = 0;

    /*! Returns content of currently tokenized buffer*/
    virtual QString buffer() const = 0;

    /*!
     * Replace part of text in current buffer. Always use this function instead of directly
     * changing data in underlying buffer or tokenizer's internal state may become inconsistent.
     */
    virtual void replace(int position, int len, const QString &newWord) = 0;
};

class BreakTokenizerPrivate;

/*!
 * \brief WordTokenizer splits supplied buffer into individual words.
 *
 * WordTokenizer splits buffer into words according to rules from Unicode standard 5.1.
 * If purpose is to check spelling, use isSpellcheckable() to determine if current word should be
 * checked or ignored.
 *
 * Usage example:
 *
 * \code
 * WordTokenizer t(buffer);
 * Speller sp;
 * while (t.hasNext()) {
 *     Token word=t.next();
 *     if (!t.isSpellcheckable()) continue;
 *     qDebug() << word.toString() << " " << sp.isCorrect(word.toString());
 * }
 * \endcode
 *
 * This example checks spelling of given buffer.
 * \since 4.3
 * \internal
 */
class SONNETCORE_EXPORT WordTokenizer : public AbstractTokenizer
{
public:
    /*!
     * Constructor for word tokenizer
     * \a buffer
     */
    WordTokenizer(const QString &buffer = QString());
    ~WordTokenizer() override;

    void setBuffer(const QString &buffer) override;
    bool hasNext() const override;
    Token next() override;
    QString buffer() const override;
    void replace(int position, int len, const QString &newWord) override;

    /*! Returns true if this word should be spell checked. This ignores email addresses, URLs and other things according to configuration */
    bool isSpellcheckable() const;

    /*! If ignore uppercase is true, then any word containing only uppercase letters will be considered unsuitable for spell check */
    void setIgnoreUppercase(bool val);

private:
    SONNETCORE_NO_EXPORT bool isUppercase(QStringView word) const;

private:
    std::unique_ptr<BreakTokenizerPrivate> const d;
};

/*!
 * \internal
 *
 * \brief SentenceTokenizer splits supplied buffer into individual sentences.
 *
 * SentenceTokenizer splits buffer into sentences according to rules from Unicode standard 5.1.
 * \since 4.3
 */
class SONNETCORE_EXPORT SentenceTokenizer : public AbstractTokenizer
{
public:
    /*!
     */
    SentenceTokenizer(const QString &buffer = QString());
    ~SentenceTokenizer() override;
    void setBuffer(const QString &buffer) override;
    bool hasNext() const override;
    Token next() override;
    QString buffer() const override;
    void replace(int position, int len, const QString &newWord) override;

private:
    std::unique_ptr<BreakTokenizerPrivate> const d;
};
}
#endif
