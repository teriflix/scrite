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

#ifndef QOBJECTFACTORY_H
#define QOBJECTFACTORY_H

#include <QSet>
#include <QMap>
#ifdef QT_WIDGETS_LIB
#include <QWidget>
#endif
#include <QObject>
#include <QMetaObject>
#include <QMetaClassInfo>

template<class T>
class QtFactory
{
public:
    QtFactory(const QByteArray &key = QByteArray()) : m_classInfoKey(key) { }
    ~QtFactory() { }

    QByteArray classInfoKey() const { return m_classInfoKey; }

    bool isEmpty() const { return m_metaObjects.isEmpty(); }

    void add(const QMetaObject *mo)
    {
#ifndef QT_NO_DEBUG_OUTPUT
        if (mo->constructorCount() == 0) {
            qDebug("QtFactory: Trying to register class '%s' with no invokable constructor.",
                   mo->className());
            return;
        }
#endif
        QList<QByteArray> keys;
        const int ciIndex = mo->indexOfClassInfo(m_classInfoKey.constData());
        if (ciIndex >= 0) {
            const QMetaClassInfo ci = mo->classInfo(ciIndex);
            keys = QByteArray(ci.value()).split(';');
        } else
            keys.append(QByteArray(mo->className()));
        m_metaObjects.insert(mo, keys);
        for (const QByteArray &key : qAsConst(keys))
            m_keyMap[key].append(mo);
    }
    void remove(const QMetaObject *mo)
    {
        const QList<QByteArray> keys = m_metaObjects.take(mo);
        for (const QByteArray &key : keys) {
            QList<const QMetaObject *> mos = m_keyMap.take(key);
            mos.removeOne(mo);
            if (mos.isEmpty())
                continue;
            m_keyMap[key] = mos;
        }
    }

    template<class Class>
    void addClass()
    {
        this->add(&Class::staticMetaObject);
    }

    template<class Class>
    void removeClass()
    {
        this->remove(&Class::staticMetaObject);
    }

    QList<QByteArray> keys() const { return m_keyMap.keys(); }

    const QMetaObject *find(const QByteArray &val) const
    {
        const QList<const QMetaObject *> mos = m_keyMap.value(val);
        return mos.isEmpty() ? nullptr : mos.last();
    }

    T *create(const QByteArray &key, T *parent = nullptr) const
    {
        const QMetaObject *mo = this->find(key);
        if (mo == nullptr)
            return nullptr;
        const char *t = this->type();
        QObject *obj = qobject_cast<T *>(mo->newInstance(QArgument<T *>(t, parent)));
        return qobject_cast<T *>(obj);
    }

    template<class Class>
    Class *create(const QByteArray &className, T *parent = nullptr) const
    {
        T *obj = this->create(className, parent);
        return qobject_cast<Class *>(obj);
    }

private:
    const char *type() const
    {
        static const char *qobjectstar = "QObject*";
#ifdef QT_WIDGETS_LIB
        static const char *qwidgetstar = "QWidget*";
        if (typeid(T) == typeid(QWidget))
            return qwidgetstar;
#endif
        return qobjectstar;
    }

private:
    QMap<const QMetaObject *, QList<QByteArray>> m_metaObjects;
    QMap<QByteArray, QList<const QMetaObject *>> m_keyMap;
    QByteArray m_classInfoKey;
};

typedef QtFactory<QObject> QObjectFactory;

#ifdef QT_WIDGETS_LIB
typedef QtFactory<QWidget> QWidgetFactory;
#endif

#endif // QOBJECTFACTORY_H
