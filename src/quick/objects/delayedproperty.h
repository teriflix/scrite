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

#ifndef DELAYEDPROPERTYBINDER_H
#define DELAYEDPROPERTYBINDER_H

#include <QQuickItem>

#include "execlatertimer.h"

class DelayedPropertyBinder : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit DelayedPropertyBinder(QQuickItem *parent = nullptr);
    ~DelayedPropertyBinder();

    // clang-format off
    Q_PROPERTY(QString name
               READ name
               WRITE setName
               NOTIFY nameChanged)
    // clang-format on
    void setName(const QString &val);
    QString name() const { return m_name; }
    Q_SIGNAL void nameChanged();

    // clang-format off
    Q_PROPERTY(QVariant set
               READ set
               WRITE setSet
               NOTIFY setChanged)
    // clang-format on
    void setSet(const QVariant &val);
    QVariant set() const { return m_set; }
    Q_SIGNAL void setChanged();

    // clang-format off
    Q_PROPERTY(QVariant get
               READ get
               NOTIFY getChanged)
    // clang-format on
    QVariant get() const { return m_get; }
    Q_SIGNAL void getChanged();

    // clang-format off
    Q_PROPERTY(QVariant initial
               READ initial
               WRITE setInitial
               NOTIFY initialChanged)
    // clang-format on
    void setInitial(const QVariant &val);
    QVariant initial() const { return m_initial; }
    Q_SIGNAL void initialChanged();

    // clang-format off
    Q_PROPERTY(int delay
               READ delay
               WRITE setDelay
               NOTIFY delayChanged)
    // clang-format on
    void setDelay(int val);
    int delay() const { return m_delay; }
    Q_SIGNAL void delayChanged();

private:
    void setGet(const QVariant &val);
    void schedule();
    void timerEvent(QTimerEvent *te);
    void parentHasChanged();

private:
    int m_delay = 0;
    QString m_name;
    QVariant m_set;
    QVariant m_get;
    QVariant m_initial;
    ExecLaterTimer m_timer;
};

class DelayedProperty : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_ATTACHED(DelayedProperty)

public:
    virtual ~DelayedProperty();

    static DelayedProperty *qmlAttachedProperties(QObject *parent);

    // clang-format off
    Q_PROPERTY(QString name
               READ name
               WRITE setName
               NOTIFY nameChanged)
    // clang-format on
    void setName(const QString &val);
    QString name() const { return m_name; }
    Q_SIGNAL void nameChanged();

    // clang-format off
    Q_PROPERTY(QVariant watch
               READ watch
               WRITE setWatch
               NOTIFY watchChanged)
    // clang-format on
    void setWatch(const QVariant &val);
    QVariant watch() const { return m_watch; }
    Q_SIGNAL void watchChanged();

    // clang-format off
    Q_PROPERTY(int delay
               READ delay
               WRITE setDelay
               NOTIFY delayChanged)
    // clang-format on
    void setDelay(int val);
    int delay() const { return m_delay; }
    Q_SIGNAL void delayChanged();

    // clang-format off
    Q_PROPERTY(QVariant initial
               READ initial
               WRITE setInitial
               NOTIFY initialChanged)
    // clang-format on
    void setInitial(const QVariant &val);
    QVariant initial() const { return m_initial; }
    Q_SIGNAL void initialChanged();

    // clang-format off
    Q_PROPERTY(QVariant value
               READ value
               NOTIFY valueChanged)
    // clang-format on
    QVariant value() const { return m_value; }
    Q_SIGNAL void valueChanged();

protected:
    explicit DelayedProperty(QObject *parent = nullptr);
    void timerEvent(QTimerEvent *te);
    void setValue(const QVariant &val);

private:
    QString m_name;
    int m_delay = 100;
    QVariant m_watch;
    QVariant m_value;
    QVariant m_initial;
    QBasicTimer m_timer;
};

#endif // DELAYEDPROPERTYBINDER_H
