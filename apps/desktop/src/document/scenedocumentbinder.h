/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef SCENEDOCUMENTBINDER_H
#define SCENEDOCUMENTBINDER_H

#include "scene.h"
#include "screenplay.h"
#include "execlatertimer.h"
#include "qobjectproperty.h"

#include <QSyntaxHighlighter>
#include <QQuickTextDocument>
#include <QTextCharFormat>

class ScreenplayFormat;
class SpellCheckService;
class SceneDocumentBlockUserData;

class TextFormat : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    explicit TextFormat(QObject *parent = nullptr);
    ~TextFormat();

    // clang-format off
    Q_PROPERTY(bool bold
               READ isBold
               WRITE setBold
               NOTIFY boldChanged)
    // clang-format on
    void setBold(bool val);
    bool isBold() const { return m_bold; }
    Q_SIGNAL void boldChanged();

    Q_INVOKABLE void toggleBold() { this->setBold(!m_bold); }

    // clang-format off
    Q_PROPERTY(bool italics
               READ isItalics
               WRITE setItalics
               NOTIFY italicsChanged)
    // clang-format on
    void setItalics(bool val);
    bool isItalics() const { return m_italics; }
    Q_SIGNAL void italicsChanged();

    Q_INVOKABLE void toggleItalics() { this->setItalics(!m_italics); }

    // clang-format off
    Q_PROPERTY(bool underline
               READ isUnderline
               WRITE setUnderline
               NOTIFY underlineChanged)
    // clang-format on
    void setUnderline(bool val);
    bool isUnderline() const { return m_underline; }
    Q_SIGNAL void underlineChanged();

    Q_INVOKABLE void toggleUnderline() { this->setUnderline(!m_underline); }

    // clang-format off
    Q_PROPERTY(bool strikeout
               READ isStrikeout
               WRITE setStrikeout
               NOTIFY strikeoutChanged)
    // clang-format on
    void setStrikeout(bool val);
    bool isStrikeout() const { return m_strikeout; }
    Q_SIGNAL void strikeoutChanged();

    Q_INVOKABLE void toggleStrikeout() { this->setStrikeout(!m_strikeout); }

    // clang-format off
    Q_PROPERTY(QColor textColor
               READ textColor
               WRITE setTextColor
               NOTIFY textColorChanged)
    // clang-format on
    void setTextColor(const QColor &val);
    QColor textColor() const { return m_textColor; }
    Q_SIGNAL void textColorChanged();

    // clang-format off
    Q_PROPERTY(bool hasTextColor
               READ hasTextColor
               NOTIFY textColorChanged)
    // clang-format on
    bool hasTextColor() const { return m_textColor.alpha() > 0; }

    Q_INVOKABLE void resetTextColor() { this->setTextColor(Qt::transparent); }

    // clang-format off
    Q_PROPERTY(QColor backgroundColor
               READ backgroundColor
               WRITE setBackgroundColor
               NOTIFY backgroundColorChanged)
    // clang-format on
    void setBackgroundColor(const QColor &val);
    QColor backgroundColor() const { return m_backgroundColor; }
    Q_SIGNAL void backgroundColorChanged();

    // clang-format off
    Q_PROPERTY(bool hasBackgroundColor
               READ hasBackgroundColor
               NOTIFY backgroundColorChanged)
    // clang-format on
    bool hasBackgroundColor() const { return m_backgroundColor.alpha() > 0; }

    Q_INVOKABLE void resetBackgroundColor() { this->setBackgroundColor(Qt::transparent); }

    // clang-format off
    Q_PROPERTY(QString link
               READ link
               WRITE setLink
               NOTIFY linkChanged)
    // clang-format on
    void setLink(const QString &val);
    QString link() const { return m_link; }
    Q_SIGNAL void linkChanged();

    Q_INVOKABLE void reset();

    void updateFromCharFormat(const QTextCharFormat &format);
    bool isUpdatingFromCharFormat() const { return m_updatingFromFormat; }
    QTextCharFormat toCharFormat(const QList<int> &properties = allProperties()) const;

    static QList<int> allProperties();

signals:
    void formatChanged(const QList<int> &properties = allProperties());

private:
    bool m_bold = false;
    bool m_italics = false;
    bool m_strikeout = false;
    bool m_underline = false;
    bool m_updatingFromFormat = false;
    QColor m_textColor = Qt::transparent;
    QColor m_backgroundColor = Qt::transparent;
    QString m_link;
};

class SceneDocumentBinder : public QSyntaxHighlighter, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    QML_ELEMENT

public:
    explicit SceneDocumentBinder(QObject *parent = nullptr);
    ~SceneDocumentBinder();

    // clang-format off
    Q_PROPERTY(ScreenplayFormat *screenplayFormat
               READ screenplayFormat
               WRITE setScreenplayFormat
               NOTIFY screenplayFormatChanged
               RESET resetScreenplayFormat)
    // clang-format on
    void setScreenplayFormat(ScreenplayFormat *val);
    ScreenplayFormat *screenplayFormat() const { return m_screenplayFormat; }
    Q_SIGNAL void screenplayFormatChanged();

    // clang-format off
    Q_PROPERTY(Scene *scene
               READ scene
               WRITE setScene
               NOTIFY sceneChanged
               RESET resetScene)
    // clang-format on
    void setScene(Scene *val);
    Scene *scene() const { return m_scene; }
    Q_SIGNAL void sceneChanged();

    // clang-format off
    Q_PROPERTY(ScreenplayElement *screenplayElement
               READ screenplayElement
               WRITE setScreenplayElement
               NOTIFY screenplayElementChanged
               RESET resetScreenplayElement)
    // clang-format on
    void setScreenplayElement(ScreenplayElement *val);
    ScreenplayElement *screenplayElement() const { return m_screenplayElement; }
    Q_SIGNAL void screenplayElementChanged();

    // clang-format off
    Q_PROPERTY(QTextDocument *document
               READ document
               WRITE setDocument
               NOTIFY textDocumentChanged
               RESET resetTextDocument)
    Q_PROPERTY(QQuickTextDocument *textDocument
               READ textDocument
               WRITE setTextDocument
               NOTIFY textDocumentChanged
               RESET resetTextDocument)
    // clang-format on
    void setTextDocument(QQuickTextDocument *val);
    QQuickTextDocument *textDocument() const { return m_textDocument; }
    Q_SIGNAL void textDocumentChanged();

    // clang-format off
    Q_PROPERTY(bool spellCheckEnabled
               READ isSpellCheckEnabled
               WRITE setSpellCheckEnabled
               NOTIFY spellCheckEnabledChanged)
    // clang-format on
    void setSpellCheckEnabled(bool val);
    bool isSpellCheckEnabled() const { return m_spellCheckEnabled; }
    Q_SIGNAL void spellCheckEnabledChanged();

    // clang-format off
    Q_PROPERTY(bool liveSpellCheckEnabled
               READ isLiveSpellCheckEnabled
               WRITE setLiveSpellCheckEnabled
               NOTIFY liveSpellCheckEnabledChanged)
    // clang-format on
    void setLiveSpellCheckEnabled(bool val);
    bool isLiveSpellCheckEnabled() const { return m_liveSpellCheckEnabled; }
    Q_SIGNAL void liveSpellCheckEnabledChanged();

    // clang-format off
    Q_PROPERTY(bool autoCapitalizeSentences
               READ isAutoCapitalizeSentences
               WRITE setAutoCapitalizeSentences
               NOTIFY autoCapitalizeSentencesChanged)
    // clang-format on
    void setAutoCapitalizeSentences(bool val);
    bool isAutoCapitalizeSentences() const { return m_autoCapitalizeSentences; }
    Q_SIGNAL void autoCapitalizeSentencesChanged();

    // clang-format off
    Q_PROPERTY(QStringList autoCapitalizeExceptions
               READ autoCapitalizeExceptions
               WRITE setAutoCapitalizeExceptions
               NOTIFY autoCapitalizeExceptionsChanged)
    // clang-format on
    void setAutoCapitalizeExceptions(const QStringList &val);
    QStringList autoCapitalizeExceptions() const { return m_autoCapitalizeExceptions; }
    Q_SIGNAL void autoCapitalizeExceptionsChanged();

    // Adds : at end of shots & transitions, CONT'D for characters where applicable.
    // clang-format off
    Q_PROPERTY(bool autoPolishParagraphs
               READ isAutoPolishParagraphs
               WRITE setAutoPolishParagraphs
               NOTIFY autoPolishParagraphsChanged)
    // clang-format on
    void setAutoPolishParagraphs(bool val);
    bool isAutoPolishParagraphs() const { return m_autoPolishParagraphs; }
    Q_SIGNAL void autoPolishParagraphsChanged();

    // clang-format off
    Q_PROPERTY(qreal textWidth
               READ textWidth
               WRITE setTextWidth
               NOTIFY textWidthChanged)
    // clang-format on
    void setTextWidth(qreal val);
    qreal textWidth() const { return m_textWidth; }
    Q_SIGNAL void textWidthChanged();

    // clang-format off
    Q_PROPERTY(qreal bottomMargin
               READ bottomMargin
               WRITE setBottomMargin
               NOTIFY bottomMarginChanged)
    // clang-format on
    void setBottomMargin(qreal val);
    qreal bottomMargin() const { return m_bottomMargin; }
    Q_SIGNAL void bottomMarginChanged();

    // clang-format off
    Q_PROPERTY(int cursorPosition
               READ cursorPosition
               WRITE setCursorPosition
               NOTIFY cursorPositionChanged)
    // clang-format on
    void setCursorPosition(int val);
    int cursorPosition() const { return m_cursorPosition; }
    Q_SIGNAL void cursorPositionChanged();

    // clang-format off
    Q_PROPERTY(int selectionStartPosition
               READ selectionStartPosition
               WRITE setSelectionStartPosition
               NOTIFY selectionStartPositionChanged)
    // clang-format on
    void setSelectionStartPosition(int val);
    int selectionStartPosition() const { return m_selectionStartPosition; }
    Q_SIGNAL void selectionStartPositionChanged();

    // clang-format off
    Q_PROPERTY(int selectionEndPosition
               READ selectionEndPosition
               WRITE setSelectionEndPosition
               NOTIFY selectionEndPositionChanged)
    // clang-format on
    void setSelectionEndPosition(int val);
    int selectionEndPosition() const { return m_selectionEndPosition; }
    Q_SIGNAL void selectionEndPositionChanged();

    // clang-format off
    Q_PROPERTY(int selectedBlockCount
               READ selectedBlockCount
               NOTIFY selectedBlockCountChanged)
    // clang-format on
    int selectedBlockCount() const;
    Q_SIGNAL void selectedBlockCountChanged();

    enum TextCasing { LowerCase, UpperCase };
    Q_ENUM(TextCasing)

    Q_INVOKABLE bool changeTextCase(SceneDocumentBinder::TextCasing casing);

    // clang-format off
    Q_PROPERTY(bool applyTextFormat
               READ isApplyTextFormat
               WRITE setApplyTextFormat
               NOTIFY applyTextFormatChanged)
    // clang-format on
    void setApplyTextFormat(bool val);
    bool isApplyTextFormat() const { return m_applyTextFormat; }
    Q_SIGNAL void applyTextFormatChanged();

    // clang-format off
    Q_PROPERTY(TextFormat *textFormat
               READ textFormat
               CONSTANT )
    // clang-format on
    TextFormat *textFormat() const { return m_textFormat; }

    Q_SIGNAL void requestCursorPosition(int position);

    // clang-format off
    Q_PROPERTY(QStringList characterNames
               READ characterNames
               WRITE setCharacterNames
               NOTIFY characterNamesChanged)
    // clang-format on
    void setCharacterNames(const QStringList &val);
    QStringList characterNames() const { return m_characterNames; }
    Q_SIGNAL void characterNamesChanged();

    // clang-format off
    Q_PROPERTY(QStringList transitions
               READ transitions
               WRITE setTransitions
               NOTIFY transitionsChanged)
    // clang-format on
    void setTransitions(const QStringList &val);
    QStringList transitions() const { return m_transitions; }
    Q_SIGNAL void transitionsChanged();

    // clang-format off
    Q_PROPERTY(QStringList shots
               READ shots
               WRITE setShots
               NOTIFY shotsChanged)
    // clang-format on
    void setShots(const QStringList &val);
    QStringList shots() const { return m_shots; }
    Q_SIGNAL void shotsChanged();

    // clang-format off
    Q_PROPERTY(SceneElement *currentElement
               READ currentElement
               NOTIFY currentElementChanged
               RESET resetCurrentElement)
    // clang-format on
    SceneElement *currentElement() const { return m_currentElement; }
    Q_SIGNAL void currentElementChanged();

    // clang-format off
    Q_PROPERTY(int currentElementCursorPosition
               READ currentElementCursorPosition
               NOTIFY cursorPositionChanged)
    // clang-format on
    int currentElementCursorPosition() const { return m_currentElementCursorPosition; }

    // clang-format off
    Q_PROPERTY(QList<SceneElement *>
               selectedElements READ
               selectedElements NOTIFY
               selectedElementsChanged )
    // clang-format on
    QList<SceneElement *> selectedElements() const;
    Q_SIGNAL void selectedElementsChanged();

    Q_INVOKABLE SceneElement *sceneElementAt(int cursorPosition) const;
    Q_INVOKABLE QRectF sceneElementBoundingRect(SceneElement *sceneElement) const;

    // clang-format off
    Q_PROPERTY(bool forceSyncDocument
               READ isForceSyncDocument
               WRITE setForceSyncDocument
               NOTIFY forceSyncDocumentChanged)
    // clang-format on
    void setForceSyncDocument(bool val);
    bool isForceSyncDocument() const { return m_forceSyncDocument; }
    Q_SIGNAL void forceSyncDocumentChanged();

    // clang-format off
    Q_PROPERTY(bool applyLanguageFonts
               READ isApplyLanguageFonts
               WRITE setApplyLanguageFonts
               NOTIFY applyLanguageFontsChanged)
    // clang-format on
    void setApplyLanguageFonts(bool val);
    bool isApplyLanguageFonts() const { return m_applyLanguageFonts; }
    Q_SIGNAL void applyLanguageFontsChanged();

    // clang-format off
    Q_PROPERTY(QString nextTabFormatAsString
               READ nextTabFormatAsString
               NOTIFY nextTabFormatChanged)
    // clang-format on
    QString nextTabFormatAsString() const;

    // clang-format off
    Q_PROPERTY(int nextTabFormat
               READ nextTabFormat
               NOTIFY nextTabFormatChanged)
    // clang-format on
    int nextTabFormat() const;
    Q_SIGNAL void nextTabFormatChanged();

    Q_INVOKABLE void tab();
    Q_INVOKABLE void backtab();
    Q_INVOKABLE bool canGoUp();
    Q_INVOKABLE bool canGoDown();
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void reload();

    Q_INVOKABLE int lastCursorPosition() const;
    Q_INVOKABLE int cursorPositionAtBlock(int blockNumber) const;
    Q_INVOKABLE int currentBlockPosition() const;

    // clang-format off
    Q_PROPERTY(QStringList spellingSuggestions
               READ spellingSuggestions
               NOTIFY spellingSuggestionsChanged)
    // clang-format on
    QStringList spellingSuggestions() const { return m_spellingSuggestions; }
    Q_SIGNAL void spellingSuggestionsChanged();

    // clang-format off
    Q_PROPERTY(bool wordUnderCursorIsMisspelled
               READ isWordUnderCursorIsMisspelled
               NOTIFY wordUnderCursorIsMisspelledChanged)
    // clang-format on
    bool isWordUnderCursorIsMisspelled() const { return m_wordUnderCursorIsMisspelled; }
    Q_SIGNAL void wordUnderCursorIsMisspelledChanged();

    // clang-format off
    Q_PROPERTY(QString selectedText
               READ selectedText
               NOTIFY selectedTextChanged)
    // clang-format on
    QString selectedText() const;
    Q_SIGNAL void selectedTextChanged();

    // clang-format off
    Q_PROPERTY(QString wordUnderCursor
               READ wordUnderCursor
               NOTIFY cursorPositionChanged)
    // clang-format on
    QString wordUnderCursor() const;

    // clang-format off
    Q_PROPERTY(QString hyperlinkUnderCursor
               READ hyperlinkUnderCursor
               NOTIFY cursorPositionChanged)
    // clang-format on
    QString hyperlinkUnderCursor() const;

    // clang-format off
    Q_PROPERTY(int hyperlinkUnderCursorStartPosition
               READ hyperlinkUnderCursorStartPosition
               NOTIFY cursorPositionChanged)
    // clang-format on
    int hyperlinkUnderCursorStartPosition() const;
    Q_SIGNAL void hyperlinkUnderCursorStartPositionChanged();

    // clang-format off
    Q_PROPERTY(int hyperlinkUnderCursorEndPosition
               READ hyperlinkUnderCursorEndPosition
               NOTIFY cursorPositionChanged)
    // clang-format on
    int hyperlinkUnderCursorEndPosition() const;
    Q_SIGNAL void hyperlinkUnderCursorEndPositionChanged();

    Q_INVOKABLE QStringList spellingSuggestionsForWordAt(int position) const;

    Q_INVOKABLE void replaceWordAt(int position, const QString &with);
    Q_INVOKABLE void replaceWordUnderCursor(const QString &with)
    {
        this->replaceWordAt(m_cursorPosition, with);
    }

    Q_INVOKABLE void addWordAtPositionToDictionary(int position);
    Q_INVOKABLE void addWordUnderCursorToDictionary()
    {
        this->addWordAtPositionToDictionary(m_cursorPosition);
    }

    Q_INVOKABLE void addWordAtPositionToIgnoreList(int position);
    Q_INVOKABLE void addWordUnderCursorToIgnoreList()
    {
        this->addWordAtPositionToIgnoreList(m_cursorPosition);
    }

    // clang-format off
    Q_PROPERTY(QStringList autoCompleteHints
               READ autoCompleteHints
               NOTIFY autoCompleteHintsChanged)
    // clang-format on
    QStringList autoCompleteHints() const { return m_autoCompleteHints; }
    Q_SIGNAL void autoCompleteHintsChanged();

    // clang-format off
    Q_PROPERTY(QStringList priorityAutoCompleteHints
               READ priorityAutoCompleteHints
               NOTIFY autoCompleteHintsChanged)
    // clang-format on
    QStringList priorityAutoCompleteHints() const { return m_priorityAutoCompleteHints; }

    // clang-format off
    Q_PROPERTY(SceneElement::Type autoCompleteHintsFor
               READ autoCompleteHintsFor
               NOTIFY autoCompleteHintsForChanged)
    // clang-format on
    SceneElement::Type autoCompleteHintsFor() const { return m_autoCompleteHintsFor; }
    Q_SIGNAL void autoCompleteHintsForChanged();

    // clang-format off
    Q_PROPERTY(QString completionPrefix
               READ completionPrefix
               NOTIFY completionPrefixChanged)
    // clang-format on
    QString completionPrefix() const { return m_completionPrefix; }
    Q_SIGNAL void completionPrefixChanged();

    // clang-format off
    Q_PROPERTY(int completionPrefixStart
               READ completionPrefixStart
               NOTIFY completionPrefixChanged)
    // clang-format on
    int completionPrefixStart() const { return m_completionPrefixStart; }

    // clang-format off
    Q_PROPERTY(int completionPrefixEnd
               READ completionPrefixEnd
               NOTIFY completionPrefixChanged)
    // clang-format on
    int completionPrefixEnd() const { return m_completionPrefixEnd; }

    // clang-format off
    Q_PROPERTY(bool hasCompletionPrefixBoundary
               READ hasCompletionPrefixBoundary
               NOTIFY completionPrefixChanged)
    // clang-format on
    bool hasCompletionPrefixBoundary() const
    {
        return m_completionPrefixStart >= 0 && m_completionPrefixEnd >= 0;
    }

    enum CompletionMode {
        NoCompletionMode,
        CharacterNameCompletionMode,
        CharacterBracketNotationCompletionMode,
        ShotCompletionMode,
        TransitionCompletionMode
    };
    Q_ENUM(CompletionMode)
    // clang-format off
    Q_PROPERTY(CompletionMode completionMode
               READ completionMode
               NOTIFY completionModeChanged)
    // clang-format on
    CompletionMode completionMode() const { return m_completionMode; }
    Q_SIGNAL void completionModeChanged();

    // clang-format off
    Q_PROPERTY(QFont currentFont
               READ currentFont
               NOTIFY currentFontChanged)
    // clang-format on
    QFont currentFont() const;
    Q_SIGNAL void currentFontChanged();

    // clang-format off
    Q_PROPERTY(int documentLoadCount
               READ documentLoadCount
               NOTIFY documentLoadCountChanged)
    // clang-format on
    int documentLoadCount() const { return m_documentLoadCount; }
    Q_SIGNAL void documentLoadCountChanged();

    Q_SIGNAL void documentInitialized();
    Q_SIGNAL void spellingMistakesDetected();

    Q_INVOKABLE void copy(int fromPosition, int toPosition);
    Q_INVOKABLE int paste(int fromPosition = -1);

    // clang-format off
    Q_PROPERTY(bool applyFormattingEvenInTransaction
               READ isApplyFormattingEvenInTransaction
               WRITE setApplyFormattingEvenInTransaction
               NOTIFY applyFormattingEvenInTransactionChanged)
    // clang-format on
    void setApplyFormattingEvenInTransaction(bool val);
    bool isApplyFormattingEvenInTransaction() const { return m_applyFormattingEvenInTransaction; }
    Q_SIGNAL void applyFormattingEvenInTransactionChanged();

    // QQmlParserStatus interface
    void classBegin();
    void componentComplete();

protected:
    // QSyntaxHighlighter interface
    void highlightBlock(const QString &text);

    // QObject interface
    void timerEvent(QTimerEvent *te);
    bool eventFilter(QObject *watched, QEvent *event);

    // Helpers
    void mergeFormat(int start, int count, const QTextCharFormat &format);

private:
    void resetScene();
    void resetTextDocument();
    void resetScreenplayFormat();
    void resetScreenplayElement();

    void initializeDocument();
    void initializeDocumentLater();
    void setDocumentLoadCount(int val);
    void setCurrentElement(SceneElement *val);
    void resetCurrentElement();
    void activateCurrentElementDefaultLanguage();
    void onSceneElementChanged(SceneElement *element, Scene::SceneElementChangeType type);
    Q_SLOT void onSpellCheckUpdated();
    void onContentsChange(int from, int charsRemoved, int charsAdded);
    void syncSceneFromDocument(int nrBlocks = -1);
    void syncDocumentFromScene();

    void evaluateAutoCompleteHintsAndCompletionPrefix();
    void setAutoCompleteHintsFor(SceneElement::Type val);
    void setAutoCompleteHints(const QStringList &hints,
                              const QStringList &priorityHints = QStringList());
    void setCompletionPrefix(const QString &prefix, int start = -1, int end = -1);
    void setCompletionMode(CompletionMode val);
    void setSpellingSuggestions(const QStringList &val);
    void setWordUnderCursorIsMisspelled(bool val);

    void onSceneAboutToReset();
    void onSceneReset(int position);
    void onSceneRefreshed();

    void rehighlightLater();
    void rehighlightBlockLater(const QTextBlock &block);

    void applyBlockFormatLater(const QTextBlock &block);

    void onTextFormatChanged(const QList<int> &properties);

    void polishAllSceneElements();
    void polishSceneElement(SceneElement *element);

    void performAllSceneElementTasks();

private:
    friend class SpellCheckService;
    friend class ForceCursorPositionHack;
    friend class SceneDocumentBlockUserData;

    int m_completionPrefixEnd = -1;
    int m_completionPrefixStart = -1;
    int m_currentElementCursorPosition = -1;
    int m_cursorPosition = -1;
    int m_documentLoadCount = 0;
    int m_selectionEndPosition = -1;
    int m_selectionStartPosition = -1;

    bool m_acceptTextFormatChanges = true;
    bool m_applyFormattingEvenInTransaction = false;
    bool m_applyLanguageFonts = false;
    bool m_applyNextCharFormat = false;
    bool m_applyTextFormat = false;
    bool m_autoCapitalizeSentences = true;
    QStringList m_autoCapitalizeExceptions;
    bool m_autoPolishParagraphs = true;
    bool m_forceSyncDocument = false;
    bool m_initializingDocument = false;
    bool m_liveSpellCheckEnabled = true;
    bool m_pastingContent = false;
    bool m_sceneElementTaskIsRunning = false;
    bool m_sceneIsBeingRefreshed = false;
    bool m_sceneIsBeingReset = false;
    bool m_spellCheckEnabled = true;
    bool m_wordUnderCursorIsMisspelled = false;

    qreal m_textWidth = 0;
    qreal m_bottomMargin = 10;

    QString m_completionPrefix;

    QStringList m_autoCompleteHints;
    QStringList m_characterNames;
    QStringList m_priorityAutoCompleteHints;
    QStringList m_shots;
    QStringList m_spellingSuggestions;
    QStringList m_transitions;

    CompletionMode m_completionMode = NoCompletionMode;

    SceneElement::Type m_autoCompleteHintsFor = SceneElement::Action;

    ExecLaterTimer m_applyBlockFormatTimer;
    ExecLaterTimer m_initializeDocumentTimer;
    ExecLaterTimer m_rehighlightTimer;
    ExecLaterTimer m_sceneElementTaskTimer;

    QTextCharFormat m_nextCharFormat;

    QList<QTextBlock> m_applyBlockFormatQueue;
    QList<QTextBlock> m_rehighlightBlockQueue;
    QList<SceneElement::Type> m_tabHistory;

    QObjectProperty<QQuickTextDocument> m_textDocument;
    QObjectProperty<Scene> m_scene;
    QObjectProperty<SceneElement> m_currentElement;
    QObjectProperty<ScreenplayElement> m_screenplayElement;
    QObjectProperty<ScreenplayFormat> m_screenplayFormat;

    TextFormat *m_textFormat = nullptr;
};

#endif // SCENEDOCUMENTBINDER_H
