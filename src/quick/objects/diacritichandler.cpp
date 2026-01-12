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

#include "diacritichandler.h"

#include <QKeyEvent>
#include <QQuickItem>
#include <QGuiApplication>

DiacriticHandler::DiacriticHandler(QObject *parent) : QObject { parent }
{
    m_editorItem = qobject_cast<QQuickItem *>(parent);

#ifdef Q_OS_MACOS
    if (m_editorItem)
        m_editorItem->installEventFilter(this);
#endif
}

DiacriticHandler::~DiacriticHandler()
{
#ifdef Q_OS_MACOS
    if (m_editorItem)
        m_editorItem->removeEventFilter(this);
#endif
}

DiacriticHandler *DiacriticHandler::qmlAttachedProperties(QObject *parent)
{
    return new DiacriticHandler(parent);
}

void DiacriticHandler::setEnabled(bool val)
{
    if (m_enabled == val)
        return;

    m_enabled = val;
    emit enabledChanged();
}

bool DiacriticHandler::eventFilter(QObject *object, QEvent *event)
{
#ifdef Q_OS_MACOS
    if (m_enabled && object == m_editorItem) {
        const QEvent::Type eventType = event->type();

        if (eventType == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->isAutoRepeat()) {
                if (keyEvent->text().isEmpty())
                    return false;

                keyEvent->accept();
                return true;
            }
            m_lastKeyText = keyEvent->text();
        } else if (eventType == QEvent::InputMethod) {
            QInputMethodEvent *imEvent = static_cast<QInputMethodEvent *>(event);

            if (!imEvent->commitString().isEmpty() && !m_lastKeyText.isEmpty()) {
                QInputMethodQueryEvent query(Qt::ImCursorPosition);
                qApp->sendEvent(m_editorItem, &query);

                const int cp = query.value(Qt::ImCursorPosition).toInt();
                const int removeLength =
                        m_lastKeyText.length() /*+ imEvent->commitString().length()*/;

                QInputMethodEvent *replaceEvent = new QInputMethodEvent;

                if (cp >= removeLength) {
                    const int offset = -removeLength;
                    replaceEvent->setCommitString(imEvent->commitString(), offset, removeLength);
                } else {
                    replaceEvent->setCommitString(imEvent->commitString(), -cp, cp);
                }

                qApp->postEvent(m_editorItem, replaceEvent);
                m_lastKeyText.clear();
                imEvent->accept();
                return true;
            }
        }
    }
#endif

    return QObject::eventFilter(object, event);
}
