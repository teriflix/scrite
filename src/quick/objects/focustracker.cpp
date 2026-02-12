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

#include "focustracker.h"
#include "undoredo.h"
#include "appwindow.h"
#include "application.h"

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

    if (!m_property.isEmpty() && !m_target.isNull())
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

    this->setWindow(AppWindow::instance());

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

    if (m_window != nullptr) {
        disconnect(m_window, &QQuickWindow::activeFocusItemChanged, this,
                   &FocusTracker::evaluateHasFocus);
        disconnect(m_window, &QQuickWindow::activeChanged, this, &FocusTracker::evaluateHasFocus);
    }

    m_window = val;

    if (m_window != nullptr) {
        connect(m_window, &QQuickWindow::activeFocusItemChanged, this,
                &FocusTracker::evaluateHasFocus);
        connect(m_window, &QQuickWindow::activeChanged, this, &FocusTracker::evaluateHasFocus);
    }

    emit windowChanged();
}

void FocusTracker::setEvaluationMethod(FocusEvaluationMethod val)
{
    if (m_evaluationMethod == val)
        return;

    m_evaluationMethod = val;
    emit evaluationMethodChanged();
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
    if (m_item == nullptr || m_window.isNull() || !m_window->isActive()) {
        this->setHasFocus(false);
        return;
    }

    QQuickItem *focussedItem = m_window->activeFocusItem();
    if (focussedItem == nullptr) {
        this->setHasFocus(false);
        return;
    }

    QList<QQuickItem *> trackedItems;
    if (m_evaluationMethod == ExclusiveFocusEvaluation) {
        for (FocusTracker *tracker : qAsConst(*::GlobalFocusTrackerList)) {
            if (tracker == this)
                continue;
            trackedItems << tracker->item();
        }
    }

    while (focussedItem != nullptr) {
        if (focussedItem == m_item) {
            this->setHasFocus(true);
            return;
        }

        if (!trackedItems.isEmpty()) {
            if (trackedItems.contains(focussedItem)) {
                this->setHasFocus(false);
                return;
            }
        }

        focussedItem = focussedItem->parentItem();
    }

    this->setHasFocus(false);
}

bool FocusInspector::hasFocus(QQuickItem *item)
{
    if (item == nullptr)
        return false;

    QQuickWindow *window = item->window();
    if (window == nullptr)
        return false;

    QQuickItem *focussedItem = window->activeFocusItem();
    if (focussedItem == nullptr)
        return false;

    while (focussedItem != nullptr) {
        if (focussedItem == item)
            return true;

        focussedItem = focussedItem->parentItem();
    }

    return false;
}
