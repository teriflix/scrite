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

#ifndef QOBJECTPROPERTY_H
#define QOBJECTPROPERTY_H

#include <QObject>
#include <QPointer>
#include <QMetaProperty>

class QObjectPropertyBase : public QObject
{
    Q_OBJECT

protected:
    explicit QObjectPropertyBase(QObject *notify, const char *resettablePropertyName);
    ~QObjectPropertyBase();

    void setPointer(QObject *pointer);
    inline QObject *pointer() const { return m_pointer; }

    void objectDestroyed(QObject *ptr);
    virtual void resetPointer() { }

private:
    QPointer<QObject> m_notify;
    QObject *m_pointer = nullptr;
    QMetaProperty m_resettableProperty;
};

template<class T>
class QObjectProperty : public QObjectPropertyBase
{
public:
    explicit QObjectProperty(QObject *notify, const char *resettablePropertyName)
        : QObjectPropertyBase(notify, resettablePropertyName)
    {
    }
    ~QObjectProperty() { this->setPointer(nullptr); }

    inline QObjectProperty &operator=(T *pointer)
    {
        this->QObjectPropertyBase::setPointer(pointer);
        m_tpointer = pointer;
        return *this;
    }

    inline operator bool() const { return m_tpointer != nullptr; }
    inline bool operator==(T *pointer) const { return m_tpointer == pointer; }
    inline T *data() const { return m_tpointer; }
    inline T *operator->() const { return m_tpointer; }
    inline T &operator*() const { return *m_tpointer; }
    inline operator T *() const { return m_tpointer; }
    inline bool isNull() const { return m_tpointer == nullptr; }

protected:
    void resetPointer() { m_tpointer = nullptr; }

private:
    T *m_tpointer = nullptr;
};

#endif // QOBJECTPROPERTY_H
