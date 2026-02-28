/*
 * backgroundchecker.h
 *
 * SPDX-FileCopyrightText: 2004 Zack Rusin <zack@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef SONNET_BACKGROUNDCHECKER_H
#define SONNET_BACKGROUNDCHECKER_H

#include "speller.h"

#include "sonnetcore_export.h"

#include <QObject>

#include <memory>

/*!
 * \namespace Sonnet
 * \inmodule SonnetCore
 */
namespace Sonnet
{
class BackgroundCheckerPrivate;
class Speller;

/*!
 * \class Sonnet::BackgroundChecker
 * \inheaderfile Sonnet/BackgroundChecker
 * \inmodule SonnetCore
 *
 * \brief class used for spell checking in the background.
 *
 * BackgroundChecker is used to perform spell checking without
 * blocking the application. You can use it as is by calling
 * the checkText function or subclass it and reimplement
 * getMoreText function.
 *
 * The misspelling signal is emitted whenever a misspelled word
 * is found. The background checker stops right before emitting
 * the signal. So the parent has to call continueChecking function
 * to resume the checking.
 *
 * The done() signal is emitted when whole text is spell checked.
 */
class SONNETCORE_EXPORT BackgroundChecker : public QObject
{
    Q_OBJECT
public:
    /*!
     */
    explicit BackgroundChecker(QObject *parent = nullptr);
    /*!
     */
    explicit BackgroundChecker(const Speller &speller, QObject *parent = nullptr);
    ~BackgroundChecker() override;

    /*!
     * This method is used to spell check static text.
     * It automatically invokes start().
     *
     * Use fetchMoreText() with start() to spell check a stream.
     */
    void setText(const QString &text);
    /*!
     */
    [[nodiscard]] QString text() const;

    /*!
     */
    [[nodiscard]] QString currentContext() const;

    /*!
     */
    Speller speller() const;
    /*!
     */
    void setSpeller(const Speller &speller);

    /*!
     */
    bool checkWord(const QString &word);
    /*!
     */
    QStringList suggest(const QString &word) const;
    /*!
     */
    bool addWordToPersonal(const QString &word);

    /*!
     * This method is used to add a word to the session of the
     * speller currently set in BackgroundChecker.
     *
     * \since 5.55
     */
    bool addWordToSession(const QString &word);

    /*!
     * Returns whether the automatic language detection is disabled,
     * overriding the Sonnet settings.
     *
     * Returns true if the automatic language detection is disabled
     * \since 5.71
     */
    bool autoDetectLanguageDisabled() const;

    /*!
     * Sets whether to disable the automatic language detection.
     *
     * \a autoDetectDisabled if true, the language will not be
     * detected automatically by the spell checker, even if the option
     * is enabled in the Sonnet settings.
     * \since 5.71
     */
    void setAutoDetectLanguageDisabled(bool autoDetectDisabled);

public Q_SLOTS:
    /*!
     */
    virtual void start();
    /*!
     */
    virtual void stop();
    /*!
     */
    void replace(int start, const QString &oldText, const QString &newText);
    /*!
     */
    void changeLanguage(const QString &lang);

    /*!
     * After emitting misspelling signal the background
     * checker stops. The catcher is responsible for calling
     * continueChecking function to resume checking.
     */
    virtual void continueChecking();

Q_SIGNALS:
    /*!
     * Emitted whenever a misspelled word is found
     */
    void misspelling(const QString &word, int start);

    /*!
     * Emitted after the whole text has been spell checked.
     */
    void done();

protected:
    /*!
     * This function is called to get the text to spell check.
     * It will be called continuesly until it returns QString()
     * in which case the done() signal is emitted.
     * \note the start parameter in misspelling() is not a combined
     * position but a position in the last string returned
     * by fetchMoreText. You need to store the state in the derivatives.
     */
    [[nodiscard]] virtual QString fetchMoreText();

    /*!
     * This function will be called whenever the background checker
     * will be finished text which it got from fetchMoreText.
     */
    virtual void finishedCurrentFeed();

protected Q_SLOTS:
    /*!
     */
    void slotEngineDone();

private:
    std::unique_ptr<BackgroundCheckerPrivate> const d;
};
}

#endif
