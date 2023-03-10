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

#include "contextmenuevent.h"
#include "application.h"

#include <QQuickItem>

ContextMenuEvent::ContextMenuEvent(QObject *parent) : QObject(parent)
{
    m_item = qobject_cast<QQuickItem *>(parent);
    m_eventFilterTarget = m_mode == GlobalEventFilterMode ? (QObject *)qApp : (QObject *)m_item;
    this->setupEventFilter();
}

ContextMenuEvent::~ContextMenuEvent() { }

ContextMenuEvent *ContextMenuEvent::qmlAttachedProperties(QObject *object)
{
    return new ContextMenuEvent(object);
}

void ContextMenuEvent::setActive(bool val)
{
    if (m_active == val)
        return;

    m_active = val;
    this->setupEventFilter();

    emit activeChanged();
}

void ContextMenuEvent::setMode(Mode val)
{
    if (m_mode == val)
        return;

    if (m_eventFilterTarget != nullptr)
        m_eventFilterTarget->removeEventFilter(this);

    m_mode = val;
    m_eventFilterTarget = m_mode == GlobalEventFilterMode ? (QObject *)qApp : (QObject *)m_item;
    this->setupEventFilter();

    emit modeChanged();
}

bool ContextMenuEvent::eventFilter(QObject *watched, QEvent *event)
{
    if (m_item == nullptr
        || (event->type() != QEvent::ContextMenu && event->type() != QEvent::MouseButtonPress)
        || !m_item->isVisible() || !m_item->isEnabled())
        return false;

    const QPointF globalCursorPos = QCursor::pos();
    const QPointF itemCursorPos = m_item->mapFromGlobal(globalCursorPos);
    const QRectF itemRect(0, 0, m_item->width(), m_item->height());

    if (watched == m_item || itemRect.contains(itemCursorPos)) {
        if (event->type() == QEvent::ContextMenu) {
            emit popup(itemCursorPos);
            return true;
        }

        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            if (me->button() == Qt::RightButton) {
                emit popup(itemCursorPos);
                return true;
            }
        }
    }

    return false;
}

void ContextMenuEvent::setupEventFilter()
{
    if (m_item != nullptr) {
        if (m_active)
            m_eventFilterTarget->installEventFilter(this);
        else
            m_eventFilterTarget->removeEventFilter(this);
    }
}
