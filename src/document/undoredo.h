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

#ifndef UNDOREDO_H
#define UNDOREDO_H

#include <QtDebug>
#include <QVariant>
#include <QPointer>
#include <QUndoStack>
#include <QJsonObject>
#include <QQmlProperty>
#include <QQmlEngine>

#include "qobjectfactory.h"
#include "garbagecollector.h"
#include "qobjectserializer.h"

class UndoStack : public QUndoStack
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit UndoStack(QObject *parent = nullptr);
    ~UndoStack();

    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)
    void setActive(bool val);
    bool isActive() const;
    Q_SIGNAL void activeChanged();

    static void clearAllStacks();

    static bool ignoreUndoCommands;
    static QUndoStack *active();
};

class ObjectPropertyInfoList;
struct ObjectPropertyInfo
{
    ~ObjectPropertyInfo();

    const int id = -1;
    const QObject *object = nullptr;
    const QByteArray property;
    const QMetaObject *metaObject = nullptr;
    const QList<QByteArray> propertyBundle;

    void lock() { m_locked = true; }
    void unlock() { m_locked = false; }
    bool isLocked() const { return m_locked; }

    QVariant read() const;
    bool write(const QVariant &val);

    static ObjectPropertyInfo *get(QObject *object, const QByteArray &property);
    static void lockUndoRedoFor(QObject *object);
    static void unlockUndoRedoFor(QObject *object);

    static int querySetCounter(QObject *object, const QByteArray &property);

private:
    friend class ObjectPropertyInfoList;
    ObjectPropertyInfo(QObject *o, const QMetaObject *mo, const QByteArray &prop);
    void deleteSelf();

    static int counter;
    bool m_locked = false;
    bool m_objectIsLocked = false;
    QMetaObject::Connection m_connection;
};

class PushObjectPropertyUndoCommand;
class ObjectPropertyUndoCommand : public QUndoCommand
{
public:
    ~ObjectPropertyUndoCommand();

    void pushToActiveStack();

    // QUndoCommand interface
    void undo();
    void redo();
    int id() const { return m_propertyInfo ? m_propertyInfo->id : -1; }
    bool mergeWith(const QUndoCommand *other);

private:
    friend class PushObjectPropertyUndoCommand;
    ObjectPropertyUndoCommand(QObject *object, const QByteArray &property);

private:
    QVariant m_oldValue;
    QVariant m_newValue;
    bool m_firstRedoDone = false;
    ObjectPropertyInfo *m_propertyInfo = nullptr;
    QMetaObject::Connection m_connection;
};

class PushObjectPropertyUndoCommand
{
public:
    PushObjectPropertyUndoCommand(QObject *object, const QByteArray &property, bool flag = true);
    ~PushObjectPropertyUndoCommand();

private:
    ObjectPropertyUndoCommand *m_command = nullptr;
};

template<class ParentClass, class ChildClass>
class PushObjectListCommand;

template<class ParentClass, class ChildClass>
struct ObjectListPropertyMethods
{
    ObjectListPropertyMethods()
        : appendMethod(nullptr),
          removeMethod(nullptr),
          insertMethod(nullptr),
          atMethod(nullptr),
          indexOfMethod(nullptr)
    {
    }

    ObjectListPropertyMethods(void (*append)(ParentClass *, ChildClass *),
                              void (*remove)(ParentClass *, ChildClass *),
                              void (*insert)(ParentClass *, ChildClass *, int),
                              ChildClass *(*at)(ParentClass *, int),
                              int (*indexOf)(ParentClass *, ChildClass *))
        : appendMethod(append),
          removeMethod(remove),
          insertMethod(insert),
          atMethod(at),
          indexOfMethod(indexOf)
    {
    }

    ObjectListPropertyMethods(const ObjectListPropertyMethods<ParentClass, ChildClass> &other)
    {
        appendMethod = other.appendMethod;
        removeMethod = other.removeMethod;
        insertMethod = other.insertMethod;
        atMethod = other.atMethod;
        indexOfMethod = other.indexOfMethod;
    }

    ObjectListPropertyMethods &
    operator=(const ObjectListPropertyMethods<ParentClass, ChildClass> &other)
    {
        appendMethod = other.appendMethod;
        removeMethod = other.removeMethod;
        insertMethod = other.insertMethod;
        atMethod = other.atMethod;
        indexOfMethod = other.indexOfMethod;
        return *this;
    }

    bool operator==(const ObjectListPropertyMethods<ParentClass, ChildClass> &other) const
    {
        return appendMethod == other.appendMethod && removeMethod == other.removeMethod
                && insertMethod == other.indexOfMethod && atMethod == other.atMethod
                && indexOfMethod == other.indexOfMethod;
    }

    void (*appendMethod)(ParentClass *, ChildClass *) = nullptr;
    void (*removeMethod)(ParentClass *, ChildClass *) = nullptr;
    void (*insertMethod)(ParentClass *, ChildClass *, int) = nullptr;
    ChildClass *(*atMethod)(ParentClass *, int) = nullptr;
    int (*indexOfMethod)(ParentClass *, ChildClass *) = nullptr;
};

namespace ObjectList {
enum Operation { InsertOperation, RemoveOperation };
}

template<class ParentClass, class ChildClass>
class ObjectListCommand : public QUndoCommand
{
    friend class PushObjectListCommand<ParentClass, ChildClass>;

    explicit ObjectListCommand(ChildClass *child, ParentClass *parent,
                               const QByteArray &propertyName, ObjectList::Operation operation,
                               const ObjectListPropertyMethods<ParentClass, ChildClass> &methods)
        : m_childIndex(-1),
          m_firstRedoDone(false),
          m_child(child),
          m_parent(parent),
          m_operation(operation),
          m_parentPropertyInfo(nullptr),
          m_methods(methods)
    {
        if (parent != nullptr && child != nullptr) {
            m_parentPropertyInfo = ObjectPropertyInfo::get(parent, propertyName);
            if (m_parentPropertyInfo != nullptr) {
                if (m_parentPropertyInfo->isLocked())
                    m_parentPropertyInfo = nullptr;
                else {
                    m_childMetaObject = child->metaObject();
                    const QString label = QString("%1 in %2.%3")
                                                  .arg(child->metaObject()->className())
                                                  .arg(parent->metaObject()->className())
                                                  .arg(propertyName.constData());
                    this->setText(label);
                    m_connection = QObject::connect(parent, &QObject::destroyed, [this]() {
                        m_parentPropertyInfo = nullptr;
                        this->setObsolete(true);
                    });
                    if (m_methods.indexOfMethod != nullptr)
                        m_childIndex = (*m_methods.indexOfMethod)(m_parent, m_child);
                    m_childInfo = QObjectSerializer::toJson(m_child);
                }
            }
        }
    }

public:
    ~ObjectListCommand() { QObject::disconnect(m_connection); }

    void pushToActiveStack()
    {
        if (m_parentPropertyInfo != nullptr && UndoStack::active() && !m_child.isNull())
            UndoStack::active()->push(this);
        else
            delete this;
    }

    // QUndoCommand interface
    void undo()
    {
        if (m_operation == ObjectList::InsertOperation)
            this->remove();
        else
            this->insert();
    }
    void redo()
    {
        if (!m_firstRedoDone) {
            m_firstRedoDone = true;
            return;
        }
        if (m_operation == ObjectList::InsertOperation)
            this->insert();
        else
            this->remove();
    }
    int id() const { return m_parentPropertyInfo->id; }
    bool mergeWith(const QUndoCommand *) { return false; }

private:
    void remove()
    {
        if (m_child.isNull())
            return;

        if (m_parentPropertyInfo == nullptr)
            return;

        m_parentPropertyInfo->lock();

        m_childInfo = QObjectSerializer::toJson(m_child);
        if (m_methods.removeMethod != nullptr)
            (*m_methods.removeMethod)(m_parent, m_child);
        if (!m_child.isNull()) {
            GarbageCollector::instance()->add(m_child);
            m_child.clear();
        }

        m_parentPropertyInfo->unlock();
    }

    void insert()
    {
        if (!m_child.isNull())
            return;

        if (m_parentPropertyInfo == nullptr)
            return;

        m_parentPropertyInfo->lock();

        QObjectFactory factory;
        factory.add(m_childMetaObject);
        m_child = factory.create<ChildClass>(QByteArray(m_childMetaObject->className()), m_parent);
        QObjectSerializer::fromJson(m_childInfo, m_child);

        if (m_childIndex < 0) {
            if (m_methods.appendMethod != nullptr)
                (*m_methods.appendMethod)(m_parent, m_child);
        } else {
            if (m_methods.insertMethod != nullptr)
                (*m_methods.insertMethod)(m_parent, m_child, m_childIndex);
        }

        m_parentPropertyInfo->unlock();
    }

private:
    int m_childIndex = -1;
    bool m_firstRedoDone = false;
    QJsonObject m_childInfo;
    QPointer<ChildClass> m_child;
    QPointer<ParentClass> m_parent;
    ObjectList::Operation m_operation = ObjectList::InsertOperation;
    const QMetaObject *m_childMetaObject = nullptr;
    QMetaObject::Connection m_connection;
    ObjectPropertyInfo *m_parentPropertyInfo = nullptr;
    ObjectListPropertyMethods<ParentClass, ChildClass> m_methods;
};

template<class ParentClass, class ChildClass>
class PushObjectListCommand // WE need ObjectCreationAndDeletionCommand
{
public:
    PushObjectListCommand(ChildClass *child, ParentClass *parent, const QByteArray &propertyName,
                          ObjectList::Operation operation,
                          const ObjectListPropertyMethods<ParentClass, ChildClass> &methods)
    {
        if (UndoStack::active())
            m_command = new ObjectListCommand<ParentClass, ChildClass>(child, parent, propertyName,
                                                                       operation, methods);
    }
    ~PushObjectListCommand()
    {
        if (m_command)
            m_command->pushToActiveStack();
    }

private:
    ObjectListCommand<ParentClass, ChildClass> *m_command = nullptr;
};

class UndoResult : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    explicit UndoResult(QObject *parent = nullptr);
    ~UndoResult();

    Q_PROPERTY(bool success READ isSuccess WRITE setSuccess NOTIFY successChanged)
    void setSuccess(bool val);
    bool isSuccess() const { return m_success; }
    Q_SIGNAL void successChanged();

private:
    bool m_success = true;
};

class UndoHandler : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    static QList<UndoHandler *> all();
    static bool handleUndo();
    static bool handleRedo();

    explicit UndoHandler(QObject *parent = nullptr);
    ~UndoHandler();

    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    void setEnabled(bool val);
    bool isEnabled() const { return m_enabled; }
    Q_SIGNAL void enabledChanged();

    Q_PROPERTY(bool canUndo READ canUndo WRITE setCanUndo NOTIFY canUndoChanged)
    void setCanUndo(bool val);
    bool canUndo() const { return m_canUndo; }
    Q_SIGNAL void canUndoChanged();

    Q_PROPERTY(bool canRedo READ canRedo WRITE setCanRedo NOTIFY canRedoChanged)
    void setCanRedo(bool val);
    bool canRedo() const { return m_canRedo; }
    Q_SIGNAL void canRedoChanged();

    bool undo();
    bool redo();

    Q_SIGNAL void undoRequest(UndoResult *result);
    Q_SIGNAL void redoRequest(UndoResult *result);

private:
    bool m_enabled = false;
    bool m_canUndo = false;
    bool m_canRedo = false;
};

#endif // UNDOREDO_H
