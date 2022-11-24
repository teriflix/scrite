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

#include "focustracker.h"
#include "undoredo.h"

#include <QQuickItem>

FocusTrackerIndicator::FocusTrackerIndicator(FocusTracker *parent)
    : QObject(parent), m_tracker(parent), m_target(this, "target")
{
    connect(m_tracker, &FocusTracker::hasFocusChanged, this, &FocusTrackerIndicator::apply);
}

FocusTrackerIndicator::~FocusTrackerIndicator() { }

void FocusTrackerIndicator::setTarget(QObject *val)
{
    if (m_target == val)
        return;

    m_target = val;
    emit targetChanged();

    if (qobject_cast<UndoStack *>(m_target)) {
        if (m_property.isEmpty())
            this->setProperty("active");
        if (!m_onValue.isValid())
            this->setOnValue(true);
        if (!m_offValue.isValid())
            this->setOffValue(false);
    }

    this->apply();
}

void FocusTrackerIndicator::setProperty(const QString &val)
{
    if (m_property == val)
        return;

    if (!m_property.isEmpty() && m_tracker != nullptr)
        m_target->setProperty(qPrintable(m_property), m_offValue);

    m_property = val;
    emit propertyChanged();

    this->apply();
}

void FocusTrackerIndicator::setOnValue(const QVariant &val)
{
    if (m_onValue == val)
        return;

    m_onValue = val;
    emit onValueChanged();

    this->apply();
}

void FocusTrackerIndicator::setOffValue(const QVariant &val)
{
    if (m_offValue == val)
        return;

    m_offValue = val;
    emit offValueChanged();

    this->apply();
}

void FocusTrackerIndicator::apply()
{
    if (m_target != nullptr && !m_property.isEmpty() && m_onValue.isValid() && m_offValue.isValid())
        m_target->setProperty(qPrintable(m_property),
                              m_tracker->hasFocus() ? m_onValue : m_offValue);
}

void FocusTrackerIndicator::resetTarget()
{
    this->setTarget(nullptr);
}

///////////////////////////////////////////////////////////////////////////////

typedef QList<FocusTracker *> FocusTrackerList;
Q_GLOBAL_STATIC(FocusTrackerList, GlobalFocusTrackerList);

FocusTracker::FocusTracker(QObject *parent)
    : QObject(parent), m_item(qobject_cast<QQuickItem *>(parent)), m_window(this, "window")
{
    ::GlobalFocusTrackerList->append(this);

    connect(this, &FocusTracker::windowChanged, this, &FocusTracker::evaluateHasFocus);
}

FocusTracker::~FocusTracker()
{
    if (m_window != nullptr)
        disconnect(m_window, &QQuickWindow::activeFocusItemChanged, this,
                   &FocusTracker::evaluateHasFocus);

    ::GlobalFocusTrackerList->removeOne(this);
}

FocusTracker *FocusTracker::qmlAttachedProperties(QObject *object)
{
    return new FocusTracker(object);
}

void FocusTracker::setWindow(QQuickWindow *val)
{
    if (m_window == val)
        return;

    if (m_window != nullptr)
        disconnect(m_window, &QQuickWindow::activeFocusItemChanged, this,
                   &FocusTracker::evaluateHasFocus);

    m_window = val;

    if (m_window != nullptr)
        connect(m_window, &QQuickWindow::activeFocusItemChanged, this,
                &FocusTracker::evaluateHasFocus);

    emit windowChanged();
}

void FocusTracker::resetWindow()
{
    m_window = nullptr;
    emit windowChanged();
}

void FocusTracker::setHasFocus(bool val)
{
    if (m_hasFocus == val)
        return;

    m_hasFocus = val;
    emit hasFocusChanged();
}

void FocusTracker::evaluateHasFocus()
{
    if (m_item == nullptr || (m_window != nullptr && !m_window->isActive())) {
        this->setHasFocus(false);
        return;
    }

    QList<QQuickItem *> trackedItems;
    for (FocusTracker *tracker : qAsConst(*::GlobalFocusTrackerList)) {
        if (tracker == this)
            continue;
        trackedItems << tracker->item();
    }

    bool ditchFocus = false;
    QQuickItem *item = m_window->activeFocusItem();
    while (item != nullptr) {
        if (item == m_item) {
            this->setHasFocus(true);
            return;
        }

        if (trackedItems.contains(item))
            ditchFocus = true;

        item = item->parentItem();
    }

    if (ditchFocus)
        this->setHasFocus(false);
}
