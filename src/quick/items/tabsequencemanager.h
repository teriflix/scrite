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

#ifndef TABSEQUENCEMANAGER_H
#define TABSEQUENCEMANAGER_H

#include <QObject>
#include <QQmlEngine>

#include "execlatertimer.h"
#include "qobjectproperty.h"
#include "objectlistpropertymodel.h"

class TabSequenceItem;
class TabSequenceManager : public QObject
{
    Q_OBJECT

public:
    TabSequenceManager(QObject *parent = nullptr);
    ~TabSequenceManager();

    Q_PROPERTY(bool wrapAround READ isWrapAround WRITE setWrapAround NOTIFY wrapAroundChanged)
    void setWrapAround(bool val);
    bool isWrapAround() const { return m_wrapAround; }
    Q_SIGNAL void wrapAroundChanged();

protected:
    void timerEvent(QTimerEvent *te);
    bool eventFilter(QObject *watched, QEvent *event);

private:
    void add(TabSequenceItem *ptr);
    void remove(TabSequenceItem *ptr);
    void reworkSequence();
    void reworkSequenceLater();

private:
    friend class TabSequenceItem;
    bool m_wrapAround = false;
    int m_insertCounter = 0;
    ExecLaterTimer m_timer;
    QList<TabSequenceItem*> m_tabSequenceItems;
};

class TabSequenceItem : public QObject
{
    Q_OBJECT

public:
    ~TabSequenceItem();

    static TabSequenceItem *qmlAttachedProperties(QObject *object);

    Q_PROPERTY(TabSequenceManager* manager READ manager WRITE setManager NOTIFY managerChanged RESET resetManager)
    void setManager(TabSequenceManager* val);
    TabSequenceManager* manager() const { return m_manager; }
    Q_SIGNAL void managerChanged();

    Q_PROPERTY(int sequence READ sequence WRITE setSequence NOTIFY sequenceChanged)
    void setSequence(int val);
    int sequence() const { return m_sequence; }
    Q_SIGNAL void sequenceChanged();

protected:
    TabSequenceItem(QObject *parent=nullptr);
    void resetManager();
    void resetKeyNavigationObject();

private:
    void setInsertIndex(int index) { m_insertIndex = index; }
    int insertIndex() const { return m_insertIndex; }

private:
    friend class TabSequenceManager;
    int m_sequence = 0;
    int m_insertIndex = -1;
    QObjectProperty<TabSequenceManager> m_manager;
};
Q_DECLARE_METATYPE(TabSequenceItem*)
QML_DECLARE_TYPEINFO(TabSequenceItem, QML_HAS_ATTACHED_PROPERTIES)

#endif // TABSEQUENCEMANAGER_H
