/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "application.h"
#include "tabsequencemanager.h"

#include <QQuickItem>
#include <QtQml>

TabSequenceManager::TabSequenceManager(QObject *parent) : QObject(parent)
{
    qApp->installEventFilter(this);
}

TabSequenceManager::~TabSequenceManager()
{
    qApp->removeEventFilter(this);
}

void TabSequenceManager::setWrapAround(bool val)
{
    if(m_wrapAround == val)
        return;

    m_wrapAround = val;
    emit wrapAroundChanged();

    this->reworkSequenceLater();
}

void TabSequenceManager::assumeFocusAt(int index)
{
    if(index < 0 || index >= m_tabSequenceItems.size())
        return;

    TabSequenceItem *item = m_tabSequenceItems.at(index);
    QQuickItem *qitem = qobject_cast<QQuickItem*>(item->parent());
    if(qitem)
        qitem->setFocus(true);
}

void TabSequenceManager::releaseFocus()
{
    for(TabSequenceItem *item : m_tabSequenceItems)
    {
        QQuickItem *qitem = qobject_cast<QQuickItem*>(item->parent());
        if(qitem)
            qitem->setFocus(false);
    }
}

void TabSequenceManager::timerEvent(QTimerEvent *te)
{
    if(m_timer.timerId() == te->timerId())
    {
        m_timer.stop();
        this->reworkSequence();
    }
    else
        QObject::timerEvent(te);
}

bool TabSequenceManager::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::KeyPress)
    {
        QKeyEvent *ke = static_cast<QKeyEvent*>(event);
        if(ke->key() == Qt::Key_Tab || ke->key() == Qt::Key_Backtab)
        {
            int itemIndex = -1;
            for(int i=0; i<m_tabSequenceItems.size(); i++)
            {
                TabSequenceItem *item = m_tabSequenceItems.at(i);
                if(item->parent() == watched)
                {
                    itemIndex = i;
                    break;
                }
            }

            if(itemIndex < 0)
                return false;

            int nextIndex = -1;
            if(ke->key() == Qt::Key_Tab)
            {
                nextIndex = (itemIndex+1)%m_tabSequenceItems.size();
                if(nextIndex < itemIndex && !m_wrapAround)
                    return false;
            }
            else
            {
                nextIndex = itemIndex > 0 ? itemIndex-1 : m_tabSequenceItems.size()-1;
                if(nextIndex > itemIndex && !m_wrapAround)
                    return false;
            }

            if(nextIndex < 0 || nextIndex >= m_tabSequenceItems.size())
                return false;

            TabSequenceItem *item = m_tabSequenceItems.at(nextIndex);
            if(item == nullptr)
                return false;

            QQuickItem *qmlItem = qobject_cast<QQuickItem*>(item->parent());
            if(qmlItem != nullptr)
                qmlItem->setFocus(true);
        }
    }

    return false;
}

void TabSequenceManager::add(TabSequenceItem *ptr)
{
    if(ptr == nullptr || m_tabSequenceItems.contains(ptr))
        return;

    m_tabSequenceItems.append(ptr);
    ptr->setInsertIndex(m_insertCounter++);

    this->reworkSequenceLater();
}

void TabSequenceManager::remove(TabSequenceItem *ptr)
{
    if(ptr == nullptr)
        return;

    const int index = m_tabSequenceItems.indexOf(ptr);
    if(index < 0)
        return;

    m_tabSequenceItems.removeAt(index);
    this->reworkSequenceLater();
}

void TabSequenceManager::reworkSequence()
{
    if(m_tabSequenceItems.isEmpty())
        return;

    // sort the sequence
    std::sort(m_tabSequenceItems.begin(), m_tabSequenceItems.end(), [](TabSequenceItem *a, TabSequenceItem *b) {
        if(a->sequence() == b->sequence())
            return a->insertIndex() < b->insertIndex();
        return a->sequence() < b->sequence();
    });
}

void TabSequenceManager::reworkSequenceLater()
{
    m_timer.start(100, this);
}

///////////////////////////////////////////////////////////////////////////////

TabSequenceItem::TabSequenceItem(QObject *parent)
    : QObject(parent),
      m_manager(this, "manager")
{

}

TabSequenceItem::~TabSequenceItem()
{
    if(!m_manager.isNull())
        m_manager->remove(this);
}

TabSequenceItem *TabSequenceItem::qmlAttachedProperties(QObject *object)
{
    return new TabSequenceItem(object);
}

void TabSequenceItem::setManager(TabSequenceManager *val)
{
    if(m_manager == val)
        return;

    if(!m_manager.isNull())
        m_manager->remove(this);

    m_manager = val;

    if(!m_manager.isNull())
        m_manager->add(this);

    emit managerChanged();
}

void TabSequenceItem::setSequence(int val)
{
    if(m_sequence == val)
        return;

    m_sequence = val;
    emit sequenceChanged();

    if(!m_manager.isNull())
        m_manager->reworkSequenceLater();
}

void TabSequenceItem::resetManager()
{
    m_manager = nullptr;
    emit managerChanged();
}



