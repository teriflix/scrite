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

#ifndef TABSEQUENCEMANAGER_H
#define TABSEQUENCEMANAGER_H

#include <QObject>
#include <QQmlEngine>

#include "execlatertimer.h"
#include "qobjectproperty.h"
#include "qobjectlistmodel.h"

class TabSequenceItem;
class TabSequenceManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit TabSequenceManager(QObject *parent = nullptr);
    ~TabSequenceManager();

    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    void setEnabled(bool val);
    bool isEnabled() const { return m_enabled; }
    Q_SIGNAL void enabledChanged();

    Q_PROPERTY(int tabKey READ tabKey WRITE setTabKey NOTIFY tabKeyChanged)
    void setTabKey(int val);
    int tabKey() const { return m_tabKey; }
    Q_SIGNAL void tabKeyChanged();

    Q_PROPERTY(int backtabKey READ backtabKey WRITE setBacktabKey NOTIFY backtabKeyChanged)
    void setBacktabKey(int val);
    int backtabKey() const { return m_backtabKey; }
    Q_SIGNAL void backtabKeyChanged();

    Q_PROPERTY(int tabKeyModifiers READ tabKeyModifiers WRITE setTabKeyModifiers NOTIFY tabKeyModifiersChanged)
    void setTabKeyModifiers(int val);
    int tabKeyModifiers() const { return m_tabKeyModifiers; }
    Q_SIGNAL void tabKeyModifiersChanged();

    Q_PROPERTY(int backtabKeyModifiers READ backtabKeyModifiers WRITE setBacktabKeyModifiers NOTIFY backtabKeyModifiersChanged)
    void setBacktabKeyModifiers(int val);
    int backtabKeyModifiers() const { return m_backtabKeyModifiers; }
    Q_SIGNAL void backtabKeyModifiersChanged();

    Q_PROPERTY(int disabledKeyModifier READ disabledKeyModifier WRITE setDisabledKeyModifier NOTIFY disabledKeyModifierChanged)
    void setDisabledKeyModifier(int val);
    int disabledKeyModifier() const { return m_disabledKeyModifier; }
    Q_SIGNAL void disabledKeyModifierChanged();

    Q_PROPERTY(int releaseFocusKey READ releaseFocusKey WRITE setReleaseFocusKey NOTIFY releaseFocusKeyChanged)
    void setReleaseFocusKey(int val);
    int releaseFocusKey() const { return m_releaseFocusKey; }
    Q_SIGNAL void releaseFocusKeyChanged();

    Q_PROPERTY(bool releaseFocusEnabled READ isReleaseFocusEnabled WRITE setReleaseFocusEnabled NOTIFY releaseFocusEnabledChanged)
    void setReleaseFocusEnabled(bool val);
    bool isReleaseFocusEnabled() const { return m_releaseFocusEnabled; }
    Q_SIGNAL void releaseFocusEnabledChanged();

    Q_PROPERTY(bool wrapAround READ isWrapAround WRITE setWrapAround NOTIFY wrapAroundChanged)
    void setWrapAround(bool val);
    bool isWrapAround() const { return m_wrapAround; }
    Q_SIGNAL void wrapAroundChanged();

    Q_PROPERTY(QObject* currentItem READ currentItemObject NOTIFY currentItemChanged)
    TabSequenceItem *currentItem() const { return m_currentItem; }
    QObject *currentItemObject() const;
    Q_SIGNAL void currentItemChanged();

    Q_INVOKABLE void assumeFocus() { this->assumeFocusAt(0); }
    Q_INVOKABLE void assumeFocusAt(int index);
    Q_INVOKABLE void releaseFocus();

    Q_INVOKABLE bool focusPrevious() { return this->switchFocus(-1); }
    Q_INVOKABLE bool focusNext() { return this->switchFocus(1); }

    Q_SIGNAL void focusWasReleased();

private:
    bool switchFocus(int by = 1);
    bool switchFocusFrom(int fromItemIndex, int by = 1);
    int fetchItemIndex(int from, int direction, bool enabledOnly = true) const;
    void setCurrentItem(TabSequenceItem *val);

protected:
    void timerEvent(QTimerEvent *te);
    bool eventFilter(QObject *watched, QEvent *event);

private:
    int indexOf(TabSequenceItem *ptr) const { return m_tabSequenceItems.indexOf(ptr); }
    int count() const { return m_tabSequenceItems.size(); }
    void add(TabSequenceItem *ptr);
    void remove(TabSequenceItem *ptr);
    void reworkSequence();
    void reworkSequenceLater();

private:
    friend class TabSequenceItem;
    bool m_enabled = true;
    bool m_wrapAround = false;
    int m_insertCounter = 0;
    ExecLaterTimer m_timer;
    int m_tabKey = Qt::Key_Tab;
    int m_backtabKey = Qt::Key_Backtab;
    int m_tabKeyModifiers = Qt::NoModifier;
    int m_backtabKeyModifiers = Qt::NoModifier;
    int m_disabledKeyModifier = Qt::ControlModifier;
    QList<TabSequenceItem *> m_tabSequenceItems;
    int m_releaseFocusKey = Qt::Key_Escape;
    bool m_releaseFocusEnabled = false;
    TabSequenceItem *m_currentItem = nullptr;
};

class TabSequenceItem : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use as attached property.")
    QML_ATTACHED(TabSequenceItem)

public:
    ~TabSequenceItem();

    static TabSequenceItem *qmlAttachedProperties(QObject *object);

    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    void setEnabled(bool val);
    bool isEnabled() const { return m_enabled; }
    Q_SIGNAL void enabledChanged();

    Q_PROPERTY(TabSequenceManager *manager READ manager WRITE setManager NOTIFY managerChanged RESET resetManager)
    void setManager(TabSequenceManager *val);
    TabSequenceManager *manager() const { return m_manager; }
    Q_SIGNAL void managerChanged();

    Q_PROPERTY(int sequence READ sequence WRITE setSequence NOTIFY sequenceChanged)
    void setSequence(int val);
    int sequence() const { return m_sequence; }
    Q_SIGNAL void sequenceChanged();

    Q_PROPERTY(bool hasFocus READ hasFocus NOTIFY hasFocusChanged)
    bool hasFocus() const;
    Q_SIGNAL void hasFocusChanged();

    Q_INVOKABLE bool focusNext();
    Q_INVOKABLE bool focusPrevious();
    Q_INVOKABLE bool assumeFocus();

    Q_SIGNAL void aboutToReceiveFocus();

protected:
    TabSequenceItem(QObject *parent = nullptr);
    void resetManager();
    void resetKeyNavigationObject();
    void onQmlItemFocusChanged();

private:
    void setInsertIndex(int index) { m_insertIndex = index; }
    int insertIndex() const { return m_insertIndex; }

private:
    friend class TabSequenceManager;
    bool m_enabled = true;
    int m_sequence = 0;
    int m_insertIndex = -1;
    QObjectProperty<TabSequenceManager> m_manager;
};

#endif // TABSEQUENCEMANAGER_H
