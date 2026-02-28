/*
 * highlighter.h
 *
 * SPDX-FileCopyrightText: 2004 Zack Rusin <zack@kde.org>
 * SPDX-FileCopyrightText: 2013 Martin Sandsmark <martin.sandsmark@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef SONNET_HIGHLIGHTER_H
#define SONNET_HIGHLIGHTER_H

#include "sonnetui_export.h"
#include <QStringList>
#include <QSyntaxHighlighter>

#include <memory>

class QTextEdit;
class QPlainTextEdit;

namespace Sonnet
{
class HighlighterPrivate;

/*!
 * \class Sonnet::Highlighter
 * \inheaderfile Sonnet/Highlighter
 * \inmodule SonnetUi
 * \brief The sonnet Highlighter.
 *
 * Used for drawing pretty red lines in text fields
 */
class SONNETUI_EXPORT Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    /*!
     */
    explicit Highlighter(QTextEdit *textEdit, const QColor &col = QColor());

    /*!
     * Highlighter.
     *
     * \a col define spellchecking color
     *
     * \since 5.12
     */
    explicit Highlighter(QPlainTextEdit *textEdit, const QColor &col = QColor());
    ~Highlighter() override;

    /*!
     * Returns whether a spell checking backend with support for the
     * currentLanguage() was found.
     *
     * Returns true if spell checking is supported for the current language.
     */
    [[nodiscard]] bool spellCheckerFound() const;

    /*!
     * Returns the language code for the current language.
     */
    [[nodiscard]] QString currentLanguage() const;

    /*!
     * \brief Enable/Disable spell checking.
     *
     * If \a active is true then spell checking is enabled; otherwise it
     * is disabled. Note that you have to disable automatic (de)activation
     * with \l setAutomatic() before you change the state of spell
     * checking if you want to persistently enable/disable spell
     * checking.
     *
     * \a active if true, then spell checking is enabled
     *
     * \sa isActive(), setAutomatic()
     */
    void setActive(bool active);

    /*!
     * Returns the state of spell checking.
     *
     * Returns true if spell checking is active
     *
     * \sa setActive()
     */
    [[nodiscard]] bool isActive() const;

    /*!
     * Returns the state of the automatic disabling of spell checking.
     *
     * Returns true if spell checking is automatically disabled if there's
     * too many errors
     */
    [[nodiscard]] bool automatic() const;

    /*!
     * Sets whether to automatically disable spell checking if there's too
     * many errors.
     *
     * \a automatic if true, spell checking will be disabled if there's
     * a significant amount of errors.
     */
    void setAutomatic(bool automatic);

    /*!
     * Returns whether the automatic language detection is disabled,
     * overriding the Sonnet settings.
     *
     * Returns true if the automatic language detection is disabled
     * \since 5.71
     */
    [[nodiscard]] bool autoDetectLanguageDisabled() const;

    /*!
     * Sets whether to disable the automatic language detection.
     *
     * \a autoDetectDisabled if true, the language will not be
     * detected automatically by the spell checker, even if the option
     * is enabled in the Sonnet settings.
     * \since 5.71
     */
    void setAutoDetectLanguageDisabled(bool autoDetectDisabled);

    /*!
     * Adds the given word permanently to the dictionary. It will never
     * be marked as misspelled again, even after restarting the application.
     *
     * \a word the word which will be added to the dictionary
     * \since 4.1
     */
    void addWordToDictionary(const QString &word);

    /*!
     * Ignores the given word. This word will not be marked misspelled for
     * this session. It will again be marked as misspelled when creating
     * new highlighters.
     *
     * \a word the word which will be ignored
     * \since 4.1
     */
    void ignoreWord(const QString &word);

    /*!
     * Returns a list of suggested replacements for the given misspelled word.
     * If the word is not misspelled, the list will be empty.
     *
     * \a word the misspelled word
     *
     * \a max at most this many suggestions will be returned. If this is
     *            -1, as many suggestions as the spell backend supports will
     *            be returned.
     *
     * Returns a list of suggested replacements for the word
     * \since 4.1
     */
    QStringList suggestionsForWord(const QString &word, int max = 10);

    /*!
     * Returns a list of suggested replacements for the given misspelled word.
     * If the word is not misspelled, the list will be empty.
     *
     * \a word the misspelled word
     *
     * \a cursor the cursor pointing to the beginning of that word. This is used
     *               to determine the language to use, when AutoDetectLanguage is enabled.
     *
     * \a max at most this many suggestions will be returned. If this is
     *            -1, as many suggestions as the spell backend supports will
     *            be returned.
     *
     * Returns a list of suggested replacements for the word
     * \since 5.42
     */
    QStringList suggestionsForWord(const QString &word, const QTextCursor &cursor, int max = 10);

    /*!
     * Checks if a given word is marked as misspelled by the highlighter.
     *
     * \a word the word to be checked
     *
     * Returns true if the given word is misspelled.
     * \since 4.1
     */
    bool isWordMisspelled(const QString &word);

    /*!
     * Sets the color in which the highlighter underlines misspelled words.
     * \since 4.2
     */
    void setMisspelledColor(const QColor &color);

    /*!
     * Return true if checker is enabled by default
     * \since 4.5
     */
    bool checkerEnabledByDefault() const;

    /*!
     * Set a new \l QTextDocument for this highlighter to operate on.
     *
     * \a document the new document to operate on.
     */
    void setDocument(QTextDocument *document);

Q_SIGNALS:

    /*!
     * Emitted when as-you-type spell checking is enabled or disabled.
     *
     * \a description is a i18n description of the new state,
     *        with an optional reason
     */
    void activeChanged(const QString &description);

protected:
    void highlightBlock(const QString &text) override;
    /*!
     */
    virtual void setMisspelled(int start, int count);
    /*!
     */
    virtual void unsetMisspelled(int start, int count);

    bool eventFilter(QObject *o, QEvent *e) override;
    /*!
     */
    bool intraWordEditing() const;
    /*!
     */
    void setIntraWordEditing(bool editing);

public Q_SLOTS:
    /*!
     * Set language to use for spell checking.
     *
     * \a language the language code for the new language to use.
     */
    void setCurrentLanguage(const QString &language);

    /*!
     * Run auto detection, disabling spell checking if too many errors are found.
     */
    void slotAutoDetection();

    /*!
     * Force a new highlighting.
     */
    void slotRehighlight();

private Q_SLOTS:
    SONNETUI_NO_EXPORT void contentsChange(int pos, int added, int removed);

private:
    std::unique_ptr<HighlighterPrivate> const d;
    Q_DISABLE_COPY(Highlighter)
};
}

#endif
