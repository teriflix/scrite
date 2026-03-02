/*  This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2009 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QString>

#include "guesslanguage.h"
#include "languagefilter_p.h"
#include "loader_p.h"
#include "settingsimpl_p.h"
#include "speller.h"

namespace Sonnet
{
#define MIN_RELIABILITY 0.1
#define MAX_ITEMS 5

class LanguageFilterPrivate
{
public:
    LanguageFilterPrivate(AbstractTokenizer *s)
        : source(s)
    {
        gl.setLimits(MAX_ITEMS, MIN_RELIABILITY);
    }

    ~LanguageFilterPrivate()
    {
        delete source;
    }

    QString mainLanguage() const;

    AbstractTokenizer *source = nullptr;
    Token lastToken;

    mutable QString lastLanguage;
    mutable QString cachedMainLanguage;
    QString prevLanguage;

    GuessLanguage gl;
    Speller sp;
};

QString LanguageFilterPrivate::mainLanguage() const
{
    if (cachedMainLanguage.isNull()) {
        cachedMainLanguage = gl.identify(source->buffer(), QStringList(Loader::openLoader()->settings()->defaultLanguage()));
    }
    return cachedMainLanguage;
}

/* -----------------------------------------------------------------*/

LanguageFilter::LanguageFilter(AbstractTokenizer *source)
    : d(new LanguageFilterPrivate(source))
{
    d->prevLanguage = Loader::openLoader()->settings()->defaultLanguage();
}

LanguageFilter::LanguageFilter(const LanguageFilter &other)
    : d(new LanguageFilterPrivate(other.d->source))
{
    d->lastToken = other.d->lastToken;
    d->lastLanguage = other.d->lastLanguage;
    d->cachedMainLanguage = other.d->cachedMainLanguage;
    d->prevLanguage = other.d->prevLanguage;
}

LanguageFilter::~LanguageFilter() = default;

bool LanguageFilter::hasNext() const
{
    return d->source->hasNext();
}

void LanguageFilter::setBuffer(const QString &buffer)
{
    d->cachedMainLanguage = QString();
    d->source->setBuffer(buffer);
}

Token LanguageFilter::next()
{
    d->lastToken = d->source->next();
    d->prevLanguage = d->lastLanguage;
    d->lastLanguage = QString();
    return d->lastToken;
}

QString LanguageFilter::language() const
{
    if (d->lastLanguage.isNull()) {
        d->lastLanguage = d->gl.identify(d->lastToken.toString(), QStringList() << d->prevLanguage << Loader::openLoader()->settings()->defaultLanguage());
    }
    const QStringList available = d->sp.availableLanguages();

    // FIXME: do something a little more smart here
    if (!available.contains(d->lastLanguage)) {
        for (const QString &lang : available) {
            if (lang.startsWith(d->lastLanguage)) {
                d->lastLanguage = lang;
                break;
            }
        }
    }

    return d->lastLanguage;
}

bool LanguageFilter::isSpellcheckable() const
{
    const QString &lastlang = language();
    if (lastlang.isEmpty()) {
        return false;
    }

    if (d->sp.availableLanguages().contains(lastlang)) {
        return true;
    }

    return false;
}

QString LanguageFilter::buffer() const
{
    return d->source->buffer();
}

void LanguageFilter::replace(int position, int len, const QString &newWord)
{
    d->source->replace(position, len, newWord);
    // FIXME: fix lastToken
    // uncache language for current token - it may have changed
    d->lastLanguage = QString();
}
}
