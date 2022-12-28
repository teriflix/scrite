/****************************************************************************
**
** Copyright (C) VCreate Logic Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth@scrite.io)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "textlimiter.h"
#include <QTextBoundaryFinder>

AbstractTextLimiter::AbstractTextLimiter(QObject *parent) : QObject(parent)
{
    connect(this, &AbstractTextLimiter::modeChanged, this, &AbstractTextLimiter::limitText);
    connect(this, &AbstractTextLimiter::maxWordCountChanged, this, &AbstractTextLimiter::limitText);
    connect(this, &AbstractTextLimiter::maxLetterCountChanged, this,
            &AbstractTextLimiter::limitText);
    connect(this, &AbstractTextLimiter::countModeChanged, this, &AbstractTextLimiter::limitText);
}

AbstractTextLimiter::~AbstractTextLimiter() { }

void AbstractTextLimiter::setMode(Mode val)
{
    if (m_mode == val)
        return;

    m_mode = val;
    emit modeChanged();
}

void AbstractTextLimiter::setMaxWordCount(int val)
{
    if (m_maxWordCount == val)
        return;

    m_maxWordCount = qMax(0, val);
    emit maxWordCountChanged();
}

void AbstractTextLimiter::setMaxLetterCount(int val)
{
    if (m_maxLetterCount == val)
        return;

    m_maxLetterCount = qMax(0, val);
    emit maxLetterCountChanged();
}

void AbstractTextLimiter::setCountMode(CountMode val)
{
    if (m_countMode == val)
        return;

    m_countMode = val;
    emit countModeChanged();
}

void AbstractTextLimiter::setWordCount(int val)
{
    if (m_wordCount == val)
        return;

    m_wordCount = val;
    emit wordCountChanged();
}

void AbstractTextLimiter::setLetterCount(int val)
{
    if (m_letterCount == val)
        return;

    m_letterCount = val;
    emit letterCountChanged();
}

void AbstractTextLimiter::setLimitReached(bool val)
{
    if (m_limitReached == val)
        return;

    m_limitReached = val;
    emit limitReachedChanged();
}

///////////////////////////////////////////////////////////////////////////////

TextLimiter::TextLimiter(QObject *parent) : AbstractTextLimiter(parent)
{
    connect(this, &TextLimiter::textChanged, this, &TextLimiter::limitText);
}

TextLimiter::~TextLimiter() { }

void TextLimiter::setText(const QString &val)
{
    if (m_text == val)
        return;

    m_text = val;
    emit textChanged();
}

void TextLimiter::limitText()
{
    auto finishingIndexOfBoundary = [](QTextBoundaryFinder::BoundaryType type, const QString &text,
                                       int count) {
        QTextBoundaryFinder boundaryFinder(type, text);
        int counter = 0;
        int ret = -1;
        while (boundaryFinder.position() < text.length()) {
            if (boundaryFinder.boundaryReasons().testFlag(QTextBoundaryFinder::StartOfItem))
                ++counter;
            ret = boundaryFinder.toNextBoundary();
            if (ret < 0 || counter == count)
                break;
        }
        return ret < 0 ? text.length() - 1 : ret;
    };

    auto boundaryCount = [](QTextBoundaryFinder::BoundaryType type, const QString &text) {
        QTextBoundaryFinder boundaryFinder(type, text);
        int counter = 0;
        while (boundaryFinder.position() < text.length()) {
            if (boundaryFinder.boundaryReasons().testFlag(QTextBoundaryFinder::StartOfItem))
                ++counter;
            if (boundaryFinder.toNextBoundary() < 0)
                break;
        }
        return counter;
    };

    const QString simplifiedText = m_text.simplified();
    if (simplifiedText.isEmpty()) {
        this->setLimitedText(simplifiedText);
        this->setWordCount(0);
        this->setLetterCount(0);
        this->setLimitReached(false);
        return;
    }

    QString ltext = simplifiedText;
    if (this->maxWordCount() > 0
        && (this->mode() == LowerOfWordAndLetterCount || this->mode() == MatchWordCountOnly))
        ltext = ltext.left(
                finishingIndexOfBoundary(QTextBoundaryFinder::Word, ltext, this->maxWordCount()));

    if (this->maxLetterCount() > 0
        && (this->mode() == LowerOfWordAndLetterCount || this->mode() == MatchLetterCountOnly))
        ltext = ltext.left(finishingIndexOfBoundary(QTextBoundaryFinder::Grapheme, ltext,
                                                    this->maxLetterCount()));

    const int wcount =
            boundaryCount(QTextBoundaryFinder::Word,
                          this->countMode() == CountInLimitedText ? ltext : simplifiedText);
    const int lcount =
            boundaryCount(QTextBoundaryFinder::Grapheme,
                          this->countMode() == CountInLimitedText ? ltext : simplifiedText);

    this->setLimitedText(ltext);
    this->setWordCount(wcount);
    this->setLetterCount(lcount);
    this->setLimitReached(simplifiedText != ltext);
}

void TextLimiter::setLimitedText(const QString &val)
{
    if (m_limitedText == val)
        return;

    m_limitedText = val;
    emit limitedTextChanged();
}

///////////////////////////////////////////////////////////////////////////////

#include <QSyntaxHighlighter>
#include <QQuickTextDocument>
#include <QScopedValueRollback>
#include <QTimer>

class TextDocumentLimiterHighlighter : public QSyntaxHighlighter
{
public:
    TextDocumentLimiterHighlighter(TextDocumentLimiter *parent = nullptr)
        : QSyntaxHighlighter(parent), m_limiter(parent)
    {
        m_timer.setInterval(0);
        m_timer.setSingleShot(true);
        connect(&m_timer, &QTimer::timeout, this,
                &TextDocumentLimiterHighlighter::evaluateLimitPosition);
    }
    ~TextDocumentLimiterHighlighter() { }

    void highlightDocument(QTextDocument *doc);

protected:
    // QSyntaxHighlighter interface
    void highlightBlock(const QString &text);

private:
    void evaluateLimitPosition();

private:
    friend class TextDocumentLimiter;
    QTimer m_timer;
    TextDocumentLimiter *m_limiter = nullptr;
};

TextDocumentLimiter::TextDocumentLimiter(QObject *parent) : AbstractTextLimiter(parent)
{
    m_highlighter = new TextDocumentLimiterHighlighter(this);

    connect(this, &TextDocumentLimiter::highlightExtraTextChanged, m_highlighter,
            &TextDocumentLimiterHighlighter::rehighlight);
    connect(this, &TextDocumentLimiter::extraTextHighlightColorChanged, m_highlighter,
            &TextDocumentLimiterHighlighter::rehighlight);
}

TextDocumentLimiter::~TextDocumentLimiter() { }

void TextDocumentLimiter::setTextDocument(QQuickTextDocument *val)
{
    if (m_textDocument == val)
        return;

    m_textDocument = val;

    QTextDocument *document = m_textDocument == nullptr ? nullptr : m_textDocument->textDocument();
    m_highlighter->highlightDocument(document);

    emit textDocumentChanged();
}

void TextDocumentLimiter::setHighlightExtraText(bool val)
{
    if (m_highlightExtraText == val)
        return;

    m_highlightExtraText = val;
    emit highlightExtraTextChanged();
}

void TextDocumentLimiter::setExtraTextHighlightColor(const QColor &val)
{
    if (m_extraTextHighlightColor == val)
        return;

    m_extraTextHighlightColor = val;
    emit extraTextHighlightColorChanged();
}

void TextDocumentLimiter::limitText()
{
    m_highlighter->evaluateLimitPosition();
}

void TextDocumentLimiter::setLimitCursorPosition(int val)
{
    if (m_limitCursorPosition == val)
        return;

    m_limitCursorPosition = val;
    emit limitCursorPositionChanged();

    m_highlighter->rehighlight();
}

void TextDocumentLimiterHighlighter::highlightDocument(QTextDocument *doc)
{
    if (this->document() != nullptr)
        disconnect(this->document(), SIGNAL(contentsChanged()), &m_timer, SLOT(start()));

    this->QSyntaxHighlighter::setDocument(doc);
    this->evaluateLimitPosition();

    if (this->document() != nullptr)
        connect(this->document(), SIGNAL(contentsChanged()), &m_timer, SLOT(start()));
}

void TextDocumentLimiterHighlighter::highlightBlock(const QString &text)
{
    if (!m_limiter->isHighlightExtraText() || m_limiter->limitCursorPosition() < 0)
        return;

    const QTextBlock block = this->currentBlock();
    const int start = qMax(0, m_limiter->limitCursorPosition() - block.position());
    const int count = qMax(0, text.length() - start);

    if (count > 0) {
        QTextCharFormat charFormat;
        charFormat.setForeground(m_limiter->extraTextHighlightColor());
        this->setFormat(start, count, charFormat);
    }
}

void TextDocumentLimiterHighlighter::evaluateLimitPosition()
{
    QTextDocument *doc = this->document();
    if (doc == nullptr) {
        m_limiter->setWordCount(0);
        m_limiter->setLetterCount(0);
        m_limiter->setLimitReached(false);
        return;
    }

    QTextCursor cursor(doc);
    int wordLimitPosition = -1;
    int letterLimitPosition = -1;

    // count letters
    int letterCount = 0;
    while (!cursor.atEnd()) {
        if (letterLimitPosition < 0 && letterCount >= m_limiter->maxLetterCount()) {
            letterLimitPosition = cursor.position();
            if (m_limiter->countMode() == AbstractTextLimiter::CountInLimitedText)
                break;
        }

        if (cursor.movePosition(QTextCursor::NextCharacter))
            ++letterCount;
    }
    cursor.movePosition(QTextCursor::Start);

    // count words
    int wordCount = 0;
    while (!cursor.atEnd()) {
        if (wordLimitPosition < 0 && wordCount >= m_limiter->maxWordCount()) {
            wordLimitPosition = cursor.position();
            if (m_limiter->countMode() == AbstractTextLimiter::CountInLimitedText)
                break;
        }

        if (cursor.movePosition(QTextCursor::NextWord))
            ++wordCount;
    }

    m_limiter->setLetterCount(letterCount);
    m_limiter->setWordCount(wordCount);

    int limitPosition = -1;

    if (wordLimitPosition >= 0 || letterLimitPosition >= 0) {
        switch (m_limiter->mode()) {
        case AbstractTextLimiter::LowerOfWordAndLetterCount:
            limitPosition = wordLimitPosition >= 0 && letterLimitPosition >= 0
                    ? qMin(wordLimitPosition, letterLimitPosition)
                    : (wordLimitPosition >= 0 ? wordLimitPosition : letterLimitPosition);
            break;
        case AbstractTextLimiter::MatchWordCountOnly:
            limitPosition = wordLimitPosition;
            break;
        case AbstractTextLimiter::MatchLetterCountOnly:
            limitPosition = letterLimitPosition;
            break;
        }
    }

    m_limiter->setLimitReached(limitPosition >= 0);
    m_limiter->setLimitCursorPosition(limitPosition);
}
