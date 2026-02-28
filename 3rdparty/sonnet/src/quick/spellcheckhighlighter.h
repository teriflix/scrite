/*
 * SPDX-FileCopyrightText: 2004 Zack Rusin <zack@kde.org>
 * SPDX-FileCopyrightText: 2013 Martin Sandsmark <martin.sandsmark@kde.org>
 * SPDX-FileCopyrightText: 2013 Aurélien Gâteau <agateau@kde.org>
 * SPDX-FileCopyrightText: 2020 Christian Mollekopf <mollekopf@kolabsystems.com>
 * SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

// TODO KF6 create AbstractSpellcheckHighlighter and make the QtQuick and QtWidget inherit
// from it.

#include <QQuickTextDocument>
#include <QSyntaxHighlighter>

class HighlighterPrivate;

/*!
 * \qmltype SpellcheckHighlighter
 * \inqmlmodule org.kde.sonnet
 *
 * \brief The Sonnet Highlighter class, used for drawing red lines in text fields
 * when detecting spelling mistakes.
 *
 * SpellcheckHighlighter is adapted for QML applications. In usual Kirigami/QQC2-desktop-style
 * applications, this can be used directly by adding \c {Kirigami.SpellCheck.enabled: true} on
 * a TextArea.
 *
 * On other QML applications, you can add the SpellcheckHighlighter as a child of a TextArea.
 *
 * \note TextField is not supported, as it lacks QTextDocument API that Sonnet relies on.
 *
 * \qml
 * TextArea {
 *     id: textArea
 *     Sonnet.SpellcheckHighlighter {
 *         id: spellcheckhighlighter
 *         document: textArea.textDocument
 *         cursorPosition: textArea.cursorPosition
 *         selectionStart: textArea.selectionStart
 *         selectionEnd: textArea.selectionEnd
 *         misspelledColor: Kirigami.Theme.negativeTextColor
 *         active: true
 *
 *         onChangeCursorPosition: {
 *             textArea.cursorPosition = start;
 *             textArea.moveCursorSelection(end, TextEdit.SelectCharacters);
 *         }
 *     }
 * }
 * \endqml
 *
 * Additionally SpellcheckHighlighter provides some convenient methods to create
 * a context menu with suggestions.
 *
 * \since 5.88
 */
class SpellcheckHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
    QML_ELEMENT
    /*!
     * \qmlproperty QQuickTextDocument SpellcheckHighlighter::document
     * This property holds the underneath document from a QML TextEdit.
     * \since 5.88
     */
    Q_PROPERTY(QQuickTextDocument *document READ quickDocument WRITE setQuickDocument NOTIFY documentChanged)

    /*!
     * \qmlproperty int SpellcheckHighlighter::cursorPosition
     *
     * This property holds the current cursor position.
     * \since 5.88
     */
    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY cursorPositionChanged)

    /*!
     * \qmlproperty int SpellcheckHighlighter::selectionStart
     *
     * This property holds the start of the selection.
     * \since 5.88
     */
    Q_PROPERTY(int selectionStart READ selectionStart WRITE setSelectionStart NOTIFY selectionStartChanged)

    /*!
     * \qmlproperty int SpellcheckHighlighter::selectionEnd
     *
     * This property holds the end of the selection.
     * \since 5.88
     */
    Q_PROPERTY(int selectionEnd READ selectionEnd WRITE setSelectionEnd NOTIFY selectionEndChanged)

    /*!
     * \qmlproperty int SpellcheckHighlighter::wordIsMisspelled
     *
     * This property holds whether the current word under the mouse is misspelled.
     * \since 5.88
     */
    Q_PROPERTY(bool wordIsMisspelled READ wordIsMisspelled NOTIFY wordIsMisspelledChanged)

    /*!
     * \qmlproperty int SpellcheckHighlighter::wordUnderMouse
     *
     * This property holds the current word under the mouse.
     * \since 5.88
     */
    Q_PROPERTY(QString wordUnderMouse READ wordUnderMouse NOTIFY wordUnderMouseChanged)

    /*!
     * \qmlproperty int SpellcheckHighlighter::misspelledColor
     *
     * This property holds the spell color. By default, it's red.
     * \since 5.88
     */
    Q_PROPERTY(QColor misspelledColor READ misspelledColor WRITE setMisspelledColor NOTIFY misspelledColorChanged)

    /*!
     * \qmlproperty int SpellcheckHighlighter::currentLanguage
     *
     * This property holds the current language used for spell checking.
     * \since 5.88
     */
    Q_PROPERTY(QString currentLanguage READ currentLanguage NOTIFY currentLanguageChanged)

    /*!
     * \qmlproperty bool SpellcheckHighlighter::spellCheckerFound
     *
     * This property holds whether a spell checking backend with support for the
     * currentLanguage was found.
     * \since 5.88
     */
    Q_PROPERTY(bool spellCheckerFound READ spellCheckerFound CONSTANT)

    /*!
     * \qmlproperty bool SpellcheckHighlighter::active
     *
     * \brief This property holds whether spell checking is enabled.
     *
     * If \a active is true then spell checking is enabled; otherwise it
     * is disabled. Note that you have to disable automatic (de)activation
     * with automatic before you change the state of spell
     * checking if you want to persistently enable/disable spell
     * checking.
     *
     * \since 5.88
     */
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)

    /*!
     * \qmlproperty bool SpellcheckHighlighter::automatic
     *
     * This property holds whether spell checking is automatically disabled
     * if there's too many errors.
     * \since 5.88
     */
    Q_PROPERTY(bool automatic READ automatic WRITE setAutomatic NOTIFY automaticChanged)

    /*!
     * \qmlproperty bool SpellcheckHighlighter::autoDetectLanguageDisabled
     *
     * This property holds whether the automatic language detection is disabled
     * overriding the Sonnet global settings.
     * \since 5.88
     */
    Q_PROPERTY(bool autoDetectLanguageDisabled READ autoDetectLanguageDisabled WRITE setAutoDetectLanguageDisabled NOTIFY autoDetectLanguageDisabledChanged)

public:
    explicit SpellcheckHighlighter(QObject *parent = nullptr);
    ~SpellcheckHighlighter() override;

    /*!
     * \qmlmethod list<string> SpellcheckHighlighter::suggestions(int position, int max = 5)
     *
     * Returns a list of suggested replacements for the given misspelled word.
     * If the word is not misspelled, the list will be empty.
     *
     * \a word the misspelled word
     *
     * \a max at most this many suggestions will be returned. If this is
     *            -1, as many suggestions as the spell backend supports will
     *            be returned.
     *
     * \return a list of suggested replacements for the word
     * \since 5.88
     */
    Q_INVOKABLE QStringList suggestions(int position, int max = 5);

    /*!
     * \qmlmethod void SpellcheckHighlighter::ignoreWord(string word)
     *
     * Ignores the given word. This word will not be marked misspelled for
     * this session. It will again be marked as misspelled when creating
     * new highlighters.
     *
     * \a word the word which will be ignored
     * \since 5.88
     */
    Q_INVOKABLE void ignoreWord(const QString &word);

    /*!
     * \qmlmethod void SpellcheckHighlighter::addWordToDictionary(string word)
     *
     * Adds the given word permanently to the dictionary. It will never
     * be marked as misspelled again, even after restarting the application.
     *
     * \a word the word which will be added to the dictionary
     * \since 5.88
     */
    Q_INVOKABLE void addWordToDictionary(const QString &word);

    /*!
     * \qmlmethod void SpellcheckHighlighter::replaceWord(string word, int at = -1)
     *
     * Replace word at the current cursor position, or \a at if
     * \a at is not -1.
     * \since 5.88
     */
    Q_INVOKABLE void replaceWord(const QString &word, int at = -1);

    /*!
     * \qmlmethod bool SpellcheckHighlighter::isWordMisspelled(string word)
     *
     * Checks if a given word is marked as misspelled by the highlighter.
     *
     * \a word the word to be checked
     * \return true if the given word is misspelled.
     * \since 5.88
     */
    Q_INVOKABLE bool isWordMisspelled(const QString &word);

    [[nodiscard]] QQuickTextDocument *quickDocument() const;
    void setQuickDocument(QQuickTextDocument *document);

    [[nodiscard]] int cursorPosition() const;
    void setCursorPosition(int position);

    [[nodiscard]] int selectionStart() const;
    void setSelectionStart(int position);

    [[nodiscard]] int selectionEnd() const;
    void setSelectionEnd(int position);

    [[nodiscard]] bool wordIsMisspelled() const;
    [[nodiscard]] QString wordUnderMouse() const;

    [[nodiscard]] bool spellCheckerFound() const;
    [[nodiscard]] QString currentLanguage() const;

    void setActive(bool active);
    [[nodiscard]] bool active() const;

    void setAutomatic(bool automatic);
    [[nodiscard]] bool automatic() const;

    void setAutoDetectLanguageDisabled(bool autoDetectDisabled);
    [[nodiscard]] bool autoDetectLanguageDisabled() const;

    void setMisspelledColor(const QColor &color);
    [[nodiscard]] QColor misspelledColor() const;

    void setQuoteColor(const QColor &color);
    [[nodiscard]] QColor quoteColor() const;

    /*
     * Set a new QTextDocument for this highlighter to operate on.
     *
     * document the new document to operate on.
     */
    void setDocument(QTextDocument *document);

Q_SIGNALS:

    void documentChanged();

    void cursorPositionChanged();

    void selectionStartChanged();

    void selectionEndChanged();

    void wordIsMisspelledChanged();

    void wordUnderMouseChanged();

    void changeCursorPosition(int start, int end);

    void activeChanged();

    void misspelledColorChanged();

    void autoDetectLanguageDisabledChanged();

    void automaticChanged();

    void currentLanguageChanged();

    /*
     * Emitted when as-you-type spell checking is enabled or disabled.
     *
     * \a description is a i18n description of the new state,
     *        with an optional reason
     * \since 5.88
     */
    void activeChanged(const QString &description);

protected:
    void highlightBlock(const QString &text) override;
    virtual void setMisspelled(int start, int count);
    virtual void setMisspelledSelected(int start, int count);
    virtual void unsetMisspelled(int start, int count);
    bool eventFilter(QObject *o, QEvent *e) override;

    bool intraWordEditing() const;
    void setIntraWordEditing(bool editing);

public Q_SLOTS:
    /*!
     * Set language to use for spell checking.
     *
     * \a language the language code for the new language to use.
     * \since 5.88
     */
    void setCurrentLanguage(const QString &language);

    /*
     * Run auto detection, disabling spell checking if too many errors are found.
     * \since 5.88
     */
    void slotAutoDetection();

    /*
     * Force a new highlighting.
     * \since 5.88
     */
    void slotRehighlight();

private:
    [[nodiscard]] QTextCursor textCursor() const;
    [[nodiscard]] QTextDocument *textDocument() const;
    void contentsChange(int pos, int add, int rem);

    void autodetectLanguage(const QString &sentence);

    HighlighterPrivate *const d;
    Q_DISABLE_COPY(SpellcheckHighlighter)
};
