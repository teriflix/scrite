/*
 * backgroundchecker_p.h
 *
 * SPDX-FileCopyrightText: 2009 Jakub Stachowski <qbast@go2.pl>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef SONNET_BACKGROUNDCHECKER_P_H
#define SONNET_BACKGROUNDCHECKER_P_H

#include "backgroundchecker.h"
#include "languagefilter_p.h"
#include "speller.h"
#include "tokenizer_p.h"

#include <QObject>

namespace Sonnet
{

class BackgroundCheckerPrivate : public QObject
{
    Q_OBJECT
public:
    /*!
     */
    BackgroundCheckerPrivate()
        : mainTokenizer(new SentenceTokenizer)
        , sentenceOffset(-1)
    {
        autoDetectLanguageDisabled = false;
    }

    /*!
     */
    void start();
    /*!
     */
    void continueChecking();

    LanguageFilter mainTokenizer;
    WordTokenizer words;
    Token lastMisspelled;
    Speller currentDict;
    int sentenceOffset;
    bool autoDetectLanguageDisabled;

private Q_SLOTS:
    /*!
     */
    void checkNext();
Q_SIGNALS:
    /*!
     */
    void misspelling(const QString &, int);
    /*!
     */
    void done();
};

}

#endif
