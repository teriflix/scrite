/*  This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2004 Zack Rusin <zack@kde.org>
    SPDX-FileCopyrightText: 2006 Jacob R Rideout <kde@jacobrideout.net>
    SPDX-FileCopyrightText: 2009 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QList>
#include <QString>

#include "textbreaks_p.h"
#include "tokenizer_p.h"

namespace Sonnet
{
class BreakTokenizerPrivate
{
public:
    enum Type {
        Words,
        Sentences,
    };

    BreakTokenizerPrivate(Type s)
        : breakFinder(new TextBreaks)
        , itemPosition(-1)
        , cacheValid(false)
        , type(s)
    {
    }

    ~BreakTokenizerPrivate()
    {
        delete breakFinder;
    }

    TextBreaks::Positions breaks() const;
    void invalidate();
    void shiftBreaks(int from, int offset);
    void replace(int pos, int len, const QString &newWord);

    TextBreaks *const breakFinder;
    QString buffer;

    int itemPosition = -1;
    mutable bool cacheValid;
    Token last;
    const Type type;
    bool inAddress = false;
    bool ignoreUppercase = false;

    bool hasNext() const;
    Token next();
    void setBuffer(const QString &b)
    {
        invalidate();
        buffer = b;
    }

private:
    void regenerateCache() const;
    mutable TextBreaks::Positions cachedBreaks;
};

void BreakTokenizerPrivate::invalidate()
{
    cacheValid = false;
    itemPosition = -1;
}

bool BreakTokenizerPrivate::hasNext() const
{
    if (itemPosition >= (breaks().size() - 1)) {
        return false;
    }

    return true;
}

TextBreaks::Positions BreakTokenizerPrivate::breaks() const
{
    if (!cacheValid) {
        regenerateCache();
    }

    return cachedBreaks;
}

void BreakTokenizerPrivate::shiftBreaks(int from, int offset)
{
    for (int i = 0; i < cachedBreaks.size(); i++) {
        if (cachedBreaks[i].start > from) {
            cachedBreaks[i].start = cachedBreaks[i].start - offset;
        }
    }
}

void BreakTokenizerPrivate::regenerateCache() const
{
    if (!breakFinder || buffer.isEmpty()) {
        cachedBreaks = TextBreaks::Positions();
    }

    if (breakFinder) {
        breakFinder->setText(buffer);

        if (type == Sentences) {
            cachedBreaks = breakFinder->sentenceBreaks();
        } else if (type == Words) {
            cachedBreaks = breakFinder->wordBreaks();
        }
    }

    cacheValid = true;
}

Token BreakTokenizerPrivate::next()
{
    Token block;

    if (!hasNext()) {
        last = block;
        return block;
    }

    itemPosition++;

    const TextBreaks::Positions breaks = this->breaks();
    const TextBreaks::Position &textBreak = breaks.at(itemPosition);
    QStringView token = QStringView(buffer).mid(textBreak.start, textBreak.length);
    last = {token, textBreak.start};
    return last;
}

void BreakTokenizerPrivate::replace(int pos, int len, const QString &newWord)
{
    buffer.replace(pos, len, newWord);
    int offset = len - newWord.length();
    if (cacheValid) {
        shiftBreaks(pos, offset);
    }
}

/*-----------------------------------------------------------*/

WordTokenizer::WordTokenizer(const QString &buffer)
    : d(new BreakTokenizerPrivate(BreakTokenizerPrivate::Words))
{
    setBuffer(buffer);
}

WordTokenizer::~WordTokenizer() = default;

bool WordTokenizer::hasNext() const
{
    return d->hasNext();
}

void WordTokenizer::setBuffer(const QString &buffer)
{
    d->setBuffer(buffer);
}

Token WordTokenizer::next()
{
    Token n = d->next();

    // end of address of url?
    if (d->inAddress && n.position() > 0 && d->buffer[n.position() - 1].isSpace()) {
        d->inAddress = false;
    }

    // check if this word starts an email address of url
    if (!d->inAddress || hasNext()) {
        const int pos = n.position() + n.length();
        if ((pos < d->buffer.length()) && d->buffer[pos] == QLatin1Char('@')) {
            d->inAddress = true;
        }
        if ((pos + 2 < d->buffer.length()) && d->buffer[pos] == QLatin1Char(':') && d->buffer[pos + 1] == QLatin1Char('/')
            && d->buffer[pos + 2] == QLatin1Char('/')) {
            d->inAddress = true;
        }
    }
    return n;
}

QString WordTokenizer::buffer() const
{
    return d->buffer;
}

bool WordTokenizer::isUppercase(QStringView word) const
{
    for (int i = 0; i < word.length(); ++i) {
        if (word.at(i).isLetter() && !word.at(i).isUpper()) {
            return false;
        }
    }
    return true;
}

void WordTokenizer::setIgnoreUppercase(bool val)
{
    d->ignoreUppercase = val;
}

void WordTokenizer::replace(int pos, int len, const QString &newWord)
{
    d->replace(pos, len, newWord);
}

bool WordTokenizer::isSpellcheckable() const
{
    if (d->last.isNull() || d->last.isEmpty()) {
        return false;
    }
    if (!d->last.at(0).isLetter()) {
        return false;
    }
    if (d->inAddress) {
        return false;
    }
    if (d->ignoreUppercase && isUppercase(d->last.token)) {
        return false;
    }
    return true;
}

/* --------------------------------------------------------------------*/

SentenceTokenizer::SentenceTokenizer(const QString &buffer)
    : d(new BreakTokenizerPrivate(BreakTokenizerPrivate::Sentences))
{
    setBuffer(buffer);
}

SentenceTokenizer::~SentenceTokenizer() = default;

bool SentenceTokenizer::hasNext() const
{
    return d->hasNext();
}

void SentenceTokenizer::setBuffer(const QString &buffer)
{
    d->setBuffer(buffer);
}

Token SentenceTokenizer::next()
{
    return d->next();
}

QString SentenceTokenizer::buffer() const
{
    return d->buffer;
}

void SentenceTokenizer::replace(int pos, int len, const QString &newWord)
{
    d->replace(pos, len, newWord);
}
}
