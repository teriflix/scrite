/*
 * backgroundchecker.cpp
 *
 * SPDX-FileCopyrightText: 2004 Zack Rusin <zack@kde.org>
 * SPDX-FileCopyrightText: 2009 Jakub Stachowski <qbast@go2.pl>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "backgroundchecker.h"
#include "backgroundchecker_p.h"

#include "core_debug.h"

using namespace Sonnet;

void BackgroundCheckerPrivate::start()
{
    sentenceOffset = -1;
    continueChecking();
}

void BackgroundCheckerPrivate::continueChecking()
{
    metaObject()->invokeMethod(this, "checkNext", Qt::QueuedConnection);
}

void BackgroundCheckerPrivate::checkNext()
{
    do {
        // go over current sentence
        while (sentenceOffset != -1 && words.hasNext()) {
            Token word = words.next();
            if (!words.isSpellcheckable()) {
                continue;
            }

            // ok, this is valid word, do something
            if (currentDict.isMisspelled(word.toString())) {
                lastMisspelled = word;
                Q_EMIT misspelling(word.toString(), word.position() + sentenceOffset);
                return;
            }
        }
        // current sentence done, grab next suitable

        sentenceOffset = -1;
        const bool autodetectLanguage = currentDict.testAttribute(Speller::AutoDetectLanguage);
        const bool ignoreUpperCase = !currentDict.testAttribute(Speller::CheckUppercase);
        while (mainTokenizer.hasNext()) {
            Token sentence = mainTokenizer.next();
            if (autodetectLanguage && !autoDetectLanguageDisabled) {
                if (!mainTokenizer.isSpellcheckable()) {
                    continue;
                }
                // FIXME: find best from family en -> en_US, en_GB, ... ?
                currentDict.setLanguage(mainTokenizer.language());
            }
            sentenceOffset = sentence.position();
            words.setBuffer(sentence.toString());
            words.setIgnoreUppercase(ignoreUpperCase);
            break;
        }
    } while (sentenceOffset != -1);
    Q_EMIT done();
}

BackgroundChecker::BackgroundChecker(QObject *parent)
    : QObject(parent)
    , d(new BackgroundCheckerPrivate)
{
    connect(d.get(), &BackgroundCheckerPrivate::misspelling, this, &BackgroundChecker::misspelling);
    connect(d.get(), &BackgroundCheckerPrivate::done, this, &BackgroundChecker::slotEngineDone);
}

BackgroundChecker::BackgroundChecker(const Speller &speller, QObject *parent)
    : QObject(parent)
    , d(new BackgroundCheckerPrivate)
{
    d->currentDict = speller;
    connect(d.get(), &BackgroundCheckerPrivate::misspelling, this, &BackgroundChecker::misspelling);
    connect(d.get(), &BackgroundCheckerPrivate::done, this, &BackgroundChecker::slotEngineDone);
}

BackgroundChecker::~BackgroundChecker() = default;

void BackgroundChecker::setText(const QString &text)
{
    d->mainTokenizer.setBuffer(text);
    d->start();
}

void BackgroundChecker::start()
{
    // ## what if d->currentText.isEmpty()?

    // TODO: carry state from last buffer
    d->mainTokenizer.setBuffer(fetchMoreText());
    d->start();
}

void BackgroundChecker::stop()
{
    //    d->stop();
}

QString BackgroundChecker::fetchMoreText()
{
    return QString();
}

void BackgroundChecker::finishedCurrentFeed()
{
}

bool BackgroundChecker::autoDetectLanguageDisabled() const
{
    return d->autoDetectLanguageDisabled;
}

void BackgroundChecker::setAutoDetectLanguageDisabled(bool autoDetectDisabled)
{
    d->autoDetectLanguageDisabled = autoDetectDisabled;
}

void BackgroundChecker::setSpeller(const Speller &speller)
{
    d->currentDict = speller;
}

Speller BackgroundChecker::speller() const
{
    return d->currentDict;
}

bool BackgroundChecker::checkWord(const QString &word)
{
    return d->currentDict.isCorrect(word);
}

bool BackgroundChecker::addWordToPersonal(const QString &word)
{
    return d->currentDict.addToPersonal(word);
}

bool BackgroundChecker::addWordToSession(const QString &word)
{
    return d->currentDict.addToSession(word);
}

QStringList BackgroundChecker::suggest(const QString &word) const
{
    return d->currentDict.suggest(word);
}

void BackgroundChecker::changeLanguage(const QString &lang)
{
    // this sets language only for current sentence
    d->currentDict.setLanguage(lang);
}

void BackgroundChecker::continueChecking()
{
    d->continueChecking();
}

void BackgroundChecker::slotEngineDone()
{
    finishedCurrentFeed();
    const QString currentText = fetchMoreText();

    if (currentText.isNull()) {
        Q_EMIT done();
    } else {
        d->mainTokenizer.setBuffer(currentText);
        d->start();
    }
}

QString BackgroundChecker::text() const
{
    return d->mainTokenizer.buffer();
}

QString BackgroundChecker::currentContext() const
{
    int len = 60;
    // we don't want the expression underneath casted to an unsigned int
    // which would cause it to always evaluate to false
    int currentPosition = d->lastMisspelled.position() + d->sentenceOffset;
    bool begin = ((currentPosition - len / 2) <= 0) ? true : false;

    QString buffer = d->mainTokenizer.buffer();
    buffer.replace(currentPosition, d->lastMisspelled.length(), QStringLiteral("<b>%1</b>").arg(d->lastMisspelled.toString()));

    QString context;
    if (begin) {
        context = QStringLiteral("%1...").arg(buffer.mid(0, len));
    } else {
        context = QStringLiteral("...%1...").arg(buffer.mid(currentPosition - 20, len));
    }

    context.replace(QLatin1Char('\n'), QLatin1Char(' '));

    return context;
}

void Sonnet::BackgroundChecker::replace(int start, const QString &oldText, const QString &newText)
{
    // FIXME: here we assume that replacement is in current fragment. So 'words' has
    // to be adjusted and sentenceOffset does not
    d->words.replace(start - (d->sentenceOffset), oldText.length(), newText);
    d->mainTokenizer.replace(start, oldText.length(), newText);
}

#include "moc_backgroundchecker.cpp"
#include "moc_backgroundchecker_p.cpp"
