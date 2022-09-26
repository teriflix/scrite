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

#ifndef TEXTDOCUMENT_H
#define TEXTDOCUMENT_H

#include <QObject>
#include <QQmlEngine>

class QQuickItem;
class QTextDocument;
class QQuickTextDocument;

class TextDocument : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use as attached property.")
    QML_ATTACHED(TextDocument)

public:
    ~TextDocument();

    static TextDocument *qmlAttachedProperties(QObject *object);

    Q_PROPERTY(QQuickTextDocument* quickTextDocument READ quickTextDocument WRITE setQuickTextDocument NOTIFY quickTextDocumentChanged)
    QQuickTextDocument *quickTextDocument() const { return m_quickTextDocument; }
    Q_SIGNAL void quickTextDocumentChanged();

    Q_PROPERTY(QTextDocument* document READ document NOTIFY documentChanged)
    QTextDocument *document() const { return m_document; }
    Q_SIGNAL void documentChanged();

    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY cursorPositionChanged)
    void setCursorPosition(int val);
    int cursorPosition() const { return m_cursorPosition; }
    Q_SIGNAL void cursorPositionChanged();

    Q_INVOKABLE bool canGoUp() const;
    Q_INVOKABLE bool canGoDown() const;
    Q_INVOKABLE int lastCursorPosition() const;

private:
    explicit TextDocument(QObject *parent = nullptr);
    void setQuickTextDocument(QQuickTextDocument *val);
    void setDocument(QTextDocument *val);

    Q_SLOT void updateCursorPositionFromQuickTextEditItem();

private:
    int m_cursorPosition = -1;
    QTextDocument *m_document = nullptr;
    QQuickTextDocument *m_quickTextDocument = nullptr;

    QQuickItem *m_quickTextEditItem = nullptr;
};

#endif // TEXTDOCUMENT_H
