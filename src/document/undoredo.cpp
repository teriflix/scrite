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

#include "undoredo.h"
#include "application.h"

#include <QQmlListReference>

UndoStack::UndoStack(QObject *parent) : QUndoStack(parent)
{
    Application::instance()->undoGroup()->addStack(this);

    connect(Application::instance()->undoGroup(),
            &QUndoGroup::activeStackChanged,
            this, &UndoStack::activeChanged);
}

UndoStack::~UndoStack()
{

}

void UndoStack::setActive(bool val)
{
    if(val)
        Application::instance()->undoGroup()->setActiveStack(this);
    else if(this->isActive())
        Application::instance()->undoGroup()->setActiveStack(nullptr);
}

bool UndoStack::isActive() const
{
    return Application::instance()->undoGroup()->activeStack() == this;
}

void UndoStack::clearAllStacks()
{
    QList<QUndoStack*> stacks = Application::instance()->undoGroup()->stacks();
    Q_FOREACH(QUndoStack *stack, stacks)
        stack->clear();
}

bool UndoStack::ignoreUndoCommands = false;

QUndoStack *UndoStack::active()
{
    if(ignoreUndoCommands)
        return nullptr;

    QUndoStack *ret = Application::instance()->undoGroup()->activeStack();
    return ret;
}

///////////////////////////////////////////////////////////////////////////////

typedef QList<ObjectPropertyInfo*> ObjectPropertyInfoList;
Q_GLOBAL_STATIC(ObjectPropertyInfoList, GlobalObjectPropertyInfoList);

int ObjectPropertyInfo::counter = 1000;

static inline QList<QByteArray> queryPropertyBundle(QObject *object, const QByteArray &property)
{
    QList<QByteArray> propertyBundle;

    const QMetaObject *metaObject = object->metaObject();
    const QByteArray bundle = "UndoBundleFor_" + property;
    const int ciIndex = metaObject->indexOfClassInfo(bundle);
    if(ciIndex >= 0)
    {
        const QMetaClassInfo classInfo = metaObject->classInfo(ciIndex);
        const QByteArray value(classInfo.value());
        propertyBundle = value.split(',');
        for(int i=propertyBundle.size()-1; i>=0; i--) {
            QByteArray &prop = propertyBundle[i];
            prop = prop.trimmed();
        }
    }

    return propertyBundle;
}

ObjectPropertyInfo::ObjectPropertyInfo(QObject *o, const QMetaObject *mo, const QByteArray &prop)
    : id(++ObjectPropertyInfo::counter), object(o), property(prop),
      metaObject(mo), propertyBundle(queryPropertyBundle(o,prop))
{
    m_connection = QObject::connect(o, &QObject::destroyed, [this]() {
        this->deleteSelf();
    });
    ::GlobalObjectPropertyInfoList->append(this);
}

ObjectPropertyInfo::~ObjectPropertyInfo()
{
    ::GlobalObjectPropertyInfoList->removeOne(this);
    QObject::disconnect(m_connection);
}

QVariant ObjectPropertyInfo::read() const
{
    QVariantList ret;
    ret << object->property(property);
    Q_FOREACH(QByteArray prop, propertyBundle)
        ret << object->property(prop);
    return ret;
}

bool ObjectPropertyInfo::write(const QVariant &val)
{
    this->lock();
    QObject *o = const_cast<QObject*>(object);
    QVariantList list = val.toList();
    const bool ret = o->setProperty(property, list.takeFirst());
    Q_FOREACH(QByteArray prop, propertyBundle)
    {
        if(list.isEmpty())
            break;
        o->setProperty(prop, list.takeFirst());
    }
    this->unlock();
    return ret;
}

ObjectPropertyInfo *ObjectPropertyInfo::get(QObject *object, const QByteArray &property)
{
    const QMetaObject *metaObject = object->metaObject();
    const int propIndex = metaObject->indexOfProperty(property);
    if(propIndex < 0)
        return nullptr;

    const QMetaProperty prop = metaObject->property(propIndex);
    if(!prop.isReadable())
        return nullptr;

    const bool isQQmlListProperty = QByteArray(prop.typeName()).startsWith("QQmlListProperty");
    if(!prop.isWritable() && !isQQmlListProperty)
        return nullptr;

    while(metaObject != nullptr && propIndex < metaObject->propertyOffset())
        metaObject = metaObject->superClass();

    if(metaObject == nullptr)
        return nullptr; // dont know why this would happen. Just being paranoid

    ObjectPropertyInfoList &list = *::GlobalObjectPropertyInfoList();
    Q_FOREACH(ObjectPropertyInfo *info, list)
    {
        if(info->object == object && info->metaObject == metaObject && info->property == property)
            return info;
    }

    return new ObjectPropertyInfo(object, metaObject, property);
}

int ObjectPropertyInfo::querySetCounter(QObject *object, const QByteArray &property)
{
    if(object == nullptr || property.isEmpty())
        return -1;

    const int propIndex = object->metaObject()->indexOfProperty(property);
    if(propIndex < 0)
        return -1;

    const QByteArray counterProp = property + "_counter";
    const QVariant counterVal = object->property(counterProp);
    int counter = counterVal.isValid() ? counterVal.toInt() : 0;
    object->setProperty(counterProp, counter+1);
    return counter;
}

void ObjectPropertyInfo::deleteSelf()
{
    delete this;
}

ObjectPropertyUndoCommand::ObjectPropertyUndoCommand(QObject *object, const QByteArray &property)
    : QUndoCommand()
{
    if(object != nullptr)
    {
        m_propertyInfo = ObjectPropertyInfo::get(object, property);
        if(m_propertyInfo != nullptr)
        {
            if(m_propertyInfo->isLocked())
                m_propertyInfo = nullptr;
            else
            {
                m_oldValue = m_propertyInfo->read();
                this->setText( QString("%1.%2").arg(object->metaObject()->className()).arg(QString::fromLatin1(property)) );
                m_connection = QObject::connect(object, &QObject::destroyed, [this]() {
                    m_propertyInfo = nullptr;
                    this->setObsolete(true);
                });
            }
        }
    }
}

ObjectPropertyUndoCommand::~ObjectPropertyUndoCommand()
{
    QObject::disconnect(m_connection);
}

void ObjectPropertyUndoCommand::pushToActiveStack()
{
    if(m_propertyInfo != nullptr && UndoStack::active())
    {
        m_newValue = m_propertyInfo->read();
        UndoStack::active()->push(this);
    }
    else
        delete this;
}

void ObjectPropertyUndoCommand::undo()
{
    if(m_propertyInfo != nullptr)
        m_propertyInfo->write(m_oldValue);
}

void ObjectPropertyUndoCommand::redo()
{
    if(!m_firstRedoDone)
    {
        m_firstRedoDone = true;
        return;
    }

    if(m_propertyInfo != nullptr)
        m_propertyInfo->write(m_newValue);
}

bool ObjectPropertyUndoCommand::mergeWith(const QUndoCommand *other)
{
    if(other->id() != m_propertyInfo->id)
        return false;

    const ObjectPropertyUndoCommand *cmd = reinterpret_cast<const ObjectPropertyUndoCommand*>(other);
    m_newValue = cmd->m_newValue;
    return true;
}

PushObjectPropertyUndoCommand::PushObjectPropertyUndoCommand(QObject *object, const QByteArray &property, bool flag)
{
    const int counter = ObjectPropertyInfo::querySetCounter(object, property);
    if(flag && UndoStack::active() && counter > 0)
        m_command = new ObjectPropertyUndoCommand(object, property);
}

PushObjectPropertyUndoCommand::~PushObjectPropertyUndoCommand()
{
    if(m_command)
        m_command->pushToActiveStack();
}
