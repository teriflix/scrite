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

#ifndef TEXTLIMITER_H
#define TEXTLIMITER_H

#include <QColor>
#include <QObject>
#include <QQmlEngine>

class AbstractTextLimiter : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Create instance of TextLimiter or TextDocumentLimiter instead.")

public:
    ~AbstractTextLimiter();

    enum Mode { LowerOfWordAndLetterCount, MatchWordCountOnly, MatchLetterCountOnly };
    Q_ENUM(Mode)

    // clang-format off
    Q_PROPERTY(Mode mode
               READ mode
               WRITE setMode
               NOTIFY modeChanged)
    // clang-format on
    void setMode(Mode val);
    Mode mode() const { return m_mode; }
    Q_SIGNAL void modeChanged();

    // clang-format off
    Q_PROPERTY(int maxWordCount
               READ maxWordCount
               WRITE setMaxWordCount
               NOTIFY maxWordCountChanged)
    // clang-format on
    void setMaxWordCount(int val);
    int maxWordCount() const { return m_maxWordCount; }
    Q_SIGNAL void maxWordCountChanged();

    // clang-format off
    Q_PROPERTY(int maxLetterCount
               READ maxLetterCount
               WRITE setMaxLetterCount
               NOTIFY maxLetterCountChanged)
    // clang-format on
    void setMaxLetterCount(int val);
    int maxLetterCount() const { return m_maxLetterCount; }
    Q_SIGNAL void maxLetterCountChanged();

    enum CountMode { CountInText, CountInLimitedText };
    Q_ENUM(CountMode)

    // clang-format off
    Q_PROPERTY(CountMode countMode
               READ countMode
               WRITE setCountMode
               NOTIFY countModeChanged)
    // clang-format on
    void setCountMode(CountMode val);
    CountMode countMode() const { return m_countMode; }
    Q_SIGNAL void countModeChanged();

    // clang-format off
    Q_PROPERTY(int wordCount
               READ wordCount
               NOTIFY wordCountChanged)
    // clang-format on
    int wordCount() const { return m_wordCount; }
    Q_SIGNAL void wordCountChanged();

    // clang-format off
    Q_PROPERTY(int letterCount
               READ letterCount
               WRITE setLetterCount
               NOTIFY letterCountChanged)
    // clang-format on
    int letterCount() const { return m_letterCount; }
    Q_SIGNAL void letterCountChanged();

    // clang-format off
    Q_PROPERTY(bool limitReached
               READ isLimitReached
               NOTIFY limitReachedChanged)
    // clang-format on
    bool isLimitReached() const { return m_limitReached; }
    Q_SIGNAL void limitReachedChanged();

protected:
    AbstractTextLimiter(QObject *parent = nullptr);
    void setWordCount(int val);
    void setLetterCount(int val);
    void setLimitReached(bool val);

    virtual void limitText() = 0;

private:
    int m_wordCount = 0;
    int m_letterCount = 0;
    bool m_limitReached = false;
    CountMode m_countMode = CountInLimitedText;

    Mode m_mode = LowerOfWordAndLetterCount;
    int m_maxWordCount = 0;
    int m_maxLetterCount = 0;
};

class TextLimiter : public AbstractTextLimiter
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit TextLimiter(QObject *parent = nullptr);
    ~TextLimiter();

    // clang-format off
    Q_PROPERTY(QString text
               READ text
               WRITE setText
               NOTIFY textChanged)
    // clang-format on
    void setText(const QString &val);
    QString text() const { return m_text; }
    Q_SIGNAL void textChanged();

    // clang-format off
    Q_PROPERTY(QString limitedText
               READ limitedText
               NOTIFY limitedTextChanged)
    // clang-format on
    QString limitedText() const { return m_limitedText; }
    Q_SIGNAL void limitedTextChanged();

protected:
    void limitText();

private:
    void setLimitedText(const QString &val);

private:
    QString m_text;
    QString m_limitedText;
};

#endif // TEXTLIMITER_H
