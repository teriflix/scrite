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

#include "application.h"
#include "tabsequencemanager.h"

#include <QQuickItem>
#include <QtQml>

TabSequenceManager::TabSequenceManager(QObject *parent) : QObject(parent)
{
    qApp->installEventFilter(this);

#ifdef Q_OS_MAC
    m_disabledKeyModifier = Qt::AltModifier;
#else
    m_disabledKeyModifier = Qt::ControlModifier;
#endif
}

TabSequenceManager::~TabSequenceManager()
{
    qApp->removeEventFilter(this);
}

void TabSequenceManager::setEnabled(bool val)
{
    if (m_enabled == val)
        return;

    m_enabled = val;
    emit enabledChanged();
}

void TabSequenceManager::setTabKey(int val)
{
    if (m_tabKey == val)
        return;

    m_tabKey = val;
    emit tabKeyChanged();
}

void TabSequenceManager::setBacktabKey(int val)
{
    if (m_backtabKey == val)
        return;

    m_backtabKey = val;
    emit backtabKeyChanged();
}

void TabSequenceManager::setTabKeyModifiers(int val)
{
    if (m_tabKeyModifiers == val)
        return;

    m_tabKeyModifiers = val;
    emit tabKeyModifiersChanged();
}

void TabSequenceManager::setBacktabKeyModifiers(int val)
{
    if (m_backtabKeyModifiers == val)
        return;

    m_backtabKeyModifiers = val;
    emit backtabKeyModifiersChanged();
}

void TabSequenceManager::setDisabledKeyModifier(int val)
{
    if (m_disabledKeyModifier == val)
        return;

    m_disabledKeyModifier = val;
    emit disabledKeyModifierChanged();
}

void TabSequenceManager::setReleaseFocusKey(int val)
{
    if (m_releaseFocusKey == val)
        return;

    m_releaseFocusKey = val;
    emit releaseFocusKeyChanged();
}

void TabSequenceManager::setReleaseFocusEnabled(bool val)
{
    if (m_releaseFocusEnabled == val)
        return;

    m_releaseFocusEnabled = val;
    emit releaseFocusEnabledChanged();
}

void TabSequenceManager::setWrapAround(bool val)
{
    if (m_wrapAround == val)
        return;

    m_wrapAround = val;
    emit wrapAroundChanged();

    this->reworkSequenceLater();
}

QObject *TabSequenceManager::currentItemObject() const
{
    return m_currentItem;
}

void TabSequenceManager::assumeFocusAt(int index)
{
    if (index < 0 || index >= m_tabSequenceItems.size())
        return;

    TabSequenceItem *item = m_tabSequenceItems.at(index);
    QQuickItem *qitem = qobject_cast<QQuickItem *>(item->parent());
    if (qitem)
        qitem->setFocus(true);
}

void TabSequenceManager::releaseFocus()
{
    bool hadFocus = false;
    for (TabSequenceItem *item : qAsConst(m_tabSequenceItems)) {
        QQuickItem *qitem = qobject_cast<QQuickItem *>(item->parent());
        if (qitem) {
            hadFocus |= qitem->hasFocus();
            qitem->setFocus(false);
        }
    }

    if (hadFocus)
        emit focusWasReleased();
}

bool TabSequenceManager::switchFocus(int by)
{
    for (int i = 0; i < m_tabSequenceItems.size(); i++) {
        TabSequenceItem *item = m_tabSequenceItems.at(i);
        if (item->hasFocus())
            return this->switchFocusFrom(i, by);
    }

    return false;
}

bool TabSequenceManager::switchFocusFrom(int fromItemIndex, int by)
{
    if (fromItemIndex < 0 || fromItemIndex >= m_tabSequenceItems.size())
        return false;

    const int nextIndex = this->fetchItemIndex(fromItemIndex, by);
    if (by > 0 && nextIndex < fromItemIndex && !m_wrapAround)
        return false;

    if (by < 0 && nextIndex > fromItemIndex && !m_wrapAround)
        return false;

    if (nextIndex < 0 || nextIndex >= m_tabSequenceItems.size())
        return false;

    TabSequenceItem *item = m_tabSequenceItems.at(nextIndex);
    if (item == nullptr)
        return false;

    return item->assumeFocus();
}

int TabSequenceManager::fetchItemIndex(int from, int direction, bool enabledOnly) const
{
    int idx = from;
    while (1) {
        idx += direction;
        if (idx == from)
            break;
        if (idx >= m_tabSequenceItems.size())
            idx = 0;
        else if (idx < 0)
            idx = m_tabSequenceItems.size() - 1;
        if (!enabledOnly)
            break;
        TabSequenceItem *item = m_tabSequenceItems.at(idx);
        if (item->isEnabled())
            break;
    }
    return idx;
}

void TabSequenceManager::setCurrentItem(TabSequenceItem *val)
{
    if (m_currentItem == val)
        return;

    m_currentItem = val;
    emit currentItemChanged();
}

void TabSequenceManager::timerEvent(QTimerEvent *te)
{
    if (m_timer.timerId() == te->timerId()) {
        m_timer.stop();
        this->reworkSequence();
    } else
        QObject::timerEvent(te);
}

bool TabSequenceManager::eventFilter(QObject *watched, QEvent *event)
{
    if (m_enabled && event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if (m_releaseFocusEnabled && m_releaseFocusKey == ke->key()) {
            this->releaseFocus();
            return true;
        }

        auto isWatched = [watched](TabSequenceItem *item) {
            QObject *itemParent = item->parent();
            QObject *o = watched;
            while (o != nullptr) {
                if (o == itemParent)
                    return true;
                o = o->parent();
            }
            return false;
        };

        if (ke->key() == m_tabKey || ke->key() == m_backtabKey) {
            int itemIndex = -1;
            for (int i = 0; i < m_tabSequenceItems.size(); i++) {
                TabSequenceItem *item = m_tabSequenceItems.at(i);
                if (item->hasFocus() && isWatched(item)) {
                    itemIndex = i;
                    break;
                }
            }

            if (itemIndex < 0)
                return false;

            const Qt::KeyboardModifiers kemods = ke->modifiers();
            auto compareModifiers = [kemods](int val) {
                if (val == Qt::NoModifier)
                    return true;
                return int(kemods & Qt::KeyboardModifiers(val)) > 0;
            };

            int nextIndex = -1;
            if (ke->key() == m_tabKey && compareModifiers(m_tabKeyModifiers)) {
                nextIndex = fetchItemIndex(itemIndex, 1, !compareModifiers(m_disabledKeyModifier));
                if (nextIndex < itemIndex && !m_wrapAround)
                    return false;
            }

            if (ke->key() == m_backtabKey && compareModifiers(m_backtabKeyModifiers)) {
                nextIndex = fetchItemIndex(itemIndex, -1, !compareModifiers(m_disabledKeyModifier));
                if (nextIndex > itemIndex && !m_wrapAround)
                    return false;
            }

            if (nextIndex < 0 || nextIndex >= m_tabSequenceItems.size())
                return false;

            TabSequenceItem *item = m_tabSequenceItems.at(nextIndex);
            if (item == nullptr)
                return false;

            if (item->assumeFocus())
                return true;
        }
    }

    return false;
}

void TabSequenceManager::add(TabSequenceItem *ptr)
{
    if (ptr == nullptr || m_tabSequenceItems.contains(ptr))
        return;

    m_tabSequenceItems.append(ptr);
    ptr->setInsertIndex(m_insertCounter++);

    this->reworkSequenceLater();
}

void TabSequenceManager::remove(TabSequenceItem *ptr)
{
    if (ptr == nullptr)
        return;

    const int index = m_tabSequenceItems.indexOf(ptr);
    if (index < 0)
        return;

    m_tabSequenceItems.removeAt(index);
    this->reworkSequenceLater();
}

void TabSequenceManager::reworkSequence()
{
    if (m_tabSequenceItems.isEmpty())
        return;

    // sort the sequence
    std::sort(m_tabSequenceItems.begin(), m_tabSequenceItems.end(),
              [](TabSequenceItem *a, TabSequenceItem *b) {
                  if (a->sequence() == b->sequence())
                      return a->insertIndex() < b->insertIndex();
                  return a->sequence() < b->sequence();
              });
}

void TabSequenceManager::reworkSequenceLater()
{
    m_timer.start(0, this);
}

///////////////////////////////////////////////////////////////////////////////

TabSequenceItem::TabSequenceItem(QObject *parent) : QObject(parent), m_manager(this, "manager")
{
    QQuickItem *qmlItem = qobject_cast<QQuickItem *>(parent);
    if (qmlItem != nullptr)
        connect(qmlItem, &QQuickItem::focusChanged, this, &TabSequenceItem::onQmlItemFocusChanged);
}

TabSequenceItem::~TabSequenceItem()
{
    if (!m_manager.isNull())
        m_manager->remove(this);
}

TabSequenceItem *TabSequenceItem::qmlAttachedProperties(QObject *object)
{
    return new TabSequenceItem(object);
}

void TabSequenceItem::setEnabled(bool val)
{
    if (m_enabled == val)
        return;

    m_enabled = val;
    emit enabledChanged();
}

void TabSequenceItem::setManager(TabSequenceManager *val)
{
    if (m_manager == val)
        return;

    if (!m_manager.isNull())
        m_manager->remove(this);

    m_manager = val;

    if (!m_manager.isNull())
        m_manager->add(this);

    emit managerChanged();
}

void TabSequenceItem::setSequence(int val)
{
    if (m_sequence == val)
        return;

    m_sequence = val;
    emit sequenceChanged();

    if (!m_manager.isNull())
        m_manager->reworkSequenceLater();
}

bool TabSequenceItem::hasFocus() const
{
    QQuickItem *qmlItem = qobject_cast<QQuickItem *>(this->parent());
    if (qmlItem != nullptr) {
        QQuickWindow *qmlWindow = qmlItem->window();
        return Application::instance()->hasActiveFocus(qmlWindow, qmlItem);
    }

    return false;
}

bool TabSequenceItem::focusNext()
{
    if (m_manager != nullptr) {
        int index = m_manager->indexOf(this);
        if (index < 0)
            return false;

        return m_manager->switchFocusFrom(index, 1);
    }

    return false;
}

bool TabSequenceItem::focusPrevious()
{
    if (m_manager != nullptr) {
        int index = m_manager->indexOf(this);
        if (index < 0)
            return false;

        return m_manager->switchFocusFrom(index, -1);
    }

    return false;
}

bool TabSequenceItem::assumeFocus()
{
    QQuickItem *qmlItem = qobject_cast<QQuickItem *>(this->parent());
    if (qmlItem != nullptr) {
        emit aboutToReceiveFocus();
        qmlItem->setFocus(true);
        qmlItem->forceActiveFocus();
        return true;
    }

    return false;
}

void TabSequenceItem::resetManager()
{
    m_manager = nullptr;
    emit managerChanged();
}

void TabSequenceItem::onQmlItemFocusChanged()
{
    emit hasFocusChanged();

    if (!m_manager.isNull()) {
        if (this->hasFocus())
            m_manager->setCurrentItem(this);
        else if (m_manager->m_currentItem == this)
            m_manager->setCurrentItem(nullptr);
    }
}
