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

#include "textdocument.h"

#include <QQuickItem>
#include <QTextCursor>
#include <QTextDocument>
#include <QQuickTextDocument>

TextDocument::TextDocument(QObject *parent) : QObject { parent } { }

TextDocument::~TextDocument() { }

TextDocument *TextDocument::qmlAttachedProperties(QObject *object)
{
    TextDocument *ret = new TextDocument(object);

    if (object->inherits("QQuickTextEdit")) {
        ret->m_quickTextEditItem = qobject_cast<QQuickItem *>(object);

        QQuickTextDocument *qdoc = qobject_cast<QQuickTextDocument *>(
                object->property("textDocument").value<QObject *>());
        ret->setQuickTextDocument(qdoc);

        ret->updateCursorPositionFromQuickTextEditItem();
        connect(ret->m_quickTextEditItem, SIGNAL(cursorPositionChanged()), ret,
                SLOT(updateCursorPositionFromQuickTextEditItem()));
    }

    return ret;
}

void TextDocument::setCursorPosition(int val)
{
    if (m_cursorPosition == val)
        return;

    m_cursorPosition = val;
    emit cursorPositionChanged();
}

bool TextDocument::canGoUp() const
{
    if (m_document && m_cursorPosition >= 0) {
        QTextCursor cursor(m_document);
        cursor.setPosition(qMax(m_cursorPosition, 0));
        return cursor.movePosition(QTextCursor::Up);
    }

    return true;
}

bool TextDocument::canGoDown() const
{
    if (m_document && m_cursorPosition >= 0) {
        QTextCursor cursor(m_document);
        cursor.setPosition(qMax(m_cursorPosition, 0));
        return cursor.movePosition(QTextCursor::Down);
    }

    return true;
}

int TextDocument::lastCursorPosition() const
{
    if (m_document) {
        QTextCursor cursor(m_document);
        cursor.movePosition(QTextCursor::End);
        return cursor.position();
    }

    return true;
}

void TextDocument::setQuickTextDocument(QQuickTextDocument *val)
{
    if (m_quickTextDocument == val)
        return;

    m_quickTextDocument = val;
    emit quickTextDocumentChanged();

    if (m_quickTextDocument != nullptr)
        this->setDocument(m_quickTextDocument->textDocument());
}

void TextDocument::setDocument(QTextDocument *val)
{
    if (m_document == val)
        return;

    m_document = val;
    emit documentChanged();
}

void TextDocument::updateCursorPositionFromQuickTextEditItem()
{
    this->setCursorPosition(m_quickTextEditItem->property("cursorPosition").toInt());
}
