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

#include "undoredo.h"
#include "application.h"

#include <QQmlListReference>

UndoStack::UndoStack(QObject *parent) : QUndoStack(parent)
{
    Application::instance()->undoGroup()->addStack(this);

    connect(Application::instance()->undoGroup(), &QUndoGroup::activeStackChanged, this,
            &UndoStack::activeChanged);
}

UndoStack::~UndoStack() { }

void UndoStack::setActive(bool val)
{
    if (val)
        Application::instance()->undoGroup()->setActiveStack(this);
    else if (this->isActive())
        Application::instance()->undoGroup()->setActiveStack(nullptr);
}

bool UndoStack::isActive() const
{
    return Application::instance()->undoGroup()->activeStack() == this;
}

void UndoStack::clearAllStacks()
{
    const QList<QUndoStack *> stacks = Application::instance()->undoGroup()->stacks();
    for (QUndoStack *stack : stacks)
        stack->clear();
}

bool UndoStack::ignoreUndoCommands = false;

QUndoStack *UndoStack::active()
{
    if (ignoreUndoCommands)
        return nullptr;

    QUndoStack *ret = Application::instance()->undoGroup()->activeStack();
    return ret;
}

///////////////////////////////////////////////////////////////////////////////

int ObjectPropertyInfo::counter = 1000;
static QByteArray objectUndoRedoLockProperty()
{
    static QByteArray ret = QByteArrayLiteral("#lockUndoRedo");
    return ret;
}

class ObjectPropertyInfoList : public QObject, public QList<ObjectPropertyInfo *>
{
public:
    explicit ObjectPropertyInfoList() : QObject() { qApp->installEventFilter(this); }
    ~ObjectPropertyInfoList()
    {
        qDeleteAll(*this);
        this->clear();
    }

    bool eventFilter(QObject *object, QEvent *event)
    {
        if (event->type() != QEvent::DynamicPropertyChange)
            return false;

        QDynamicPropertyChangeEvent *dpe = static_cast<QDynamicPropertyChangeEvent *>(event);
        if (dpe->propertyName() != objectUndoRedoLockProperty())
            return false;

        const bool locked = object->property(objectUndoRedoLockProperty()).toBool();

        for (int i = 0; i < this->size(); i++) {
            ObjectPropertyInfo *pinfo = this->at(i);
            if (pinfo->object == object)
                pinfo->m_objectIsLocked = locked;
        }

        return false;
    }
};
Q_GLOBAL_STATIC(ObjectPropertyInfoList, GlobalObjectPropertyInfoList);

static inline QList<QByteArray> queryPropertyBundle(QObject *object, const QByteArray &property)
{
    QList<QByteArray> propertyBundle;

    const QMetaObject *metaObject = object->metaObject();
    const QByteArray bundle = "UndoBundleFor_" + property;
    const int ciIndex = metaObject->indexOfClassInfo(bundle);
    if (ciIndex >= 0) {
        const QMetaClassInfo classInfo = metaObject->classInfo(ciIndex);
        const QByteArray value(classInfo.value());
        propertyBundle = value.split(',');
        for (int i = propertyBundle.size() - 1; i >= 0; i--) {
            QByteArray &prop = propertyBundle[i];
            prop = prop.trimmed();
        }
    }

    return propertyBundle;
}

ObjectPropertyInfo::ObjectPropertyInfo(QObject *o, const QMetaObject *mo, const QByteArray &prop)
    : id(++ObjectPropertyInfo::counter),
      object(o),
      property(prop),
      metaObject(mo),
      propertyBundle(queryPropertyBundle(o, prop))
{
    m_objectIsLocked = o->property(objectUndoRedoLockProperty()).toBool();
    m_connection = QObject::connect(o, &QObject::destroyed, o, [this]() { this->deleteSelf(); });

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
    for (const QByteArray &prop : propertyBundle)
        ret << object->property(prop);
    return ret;
}

bool ObjectPropertyInfo::write(const QVariant &val)
{
    this->lock();
    QObject *o = const_cast<QObject *>(object);
    QVariantList list = val.toList();
    const bool ret = o->setProperty(property, list.takeFirst());
    for (const QByteArray &prop : propertyBundle) {
        if (list.isEmpty())
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
    if (propIndex < 0)
        return nullptr;

    const QMetaProperty prop = metaObject->property(propIndex);
    if (!prop.isReadable())
        return nullptr;

    const bool isQQmlListProperty = QByteArray(prop.typeName()).startsWith("QQmlListProperty");
    if (!prop.isWritable() && !isQQmlListProperty)
        return nullptr;

    while (metaObject != nullptr && propIndex < metaObject->propertyOffset())
        metaObject = metaObject->superClass();

    if (metaObject == nullptr)
        return nullptr; // dont know why this would happen. Just being paranoid

    ObjectPropertyInfoList &list = *::GlobalObjectPropertyInfoList();
    for (int i = 0; i < list.size(); i++) {
        ObjectPropertyInfo *info = list.at(i);
        if (info->object == object && info->metaObject == metaObject && info->property == property)
            return info;
    }

    return new ObjectPropertyInfo(object, metaObject, property);
}

void ObjectPropertyInfo::lockUndoRedoFor(QObject *object)
{
    if (object)
        object->setProperty(objectUndoRedoLockProperty(), true);
}

void ObjectPropertyInfo::unlockUndoRedoFor(QObject *object)
{
    if (object)
        object->setProperty(objectUndoRedoLockProperty(), false);
}

int ObjectPropertyInfo::querySetCounter(QObject *object, const QByteArray &property)
{
    if (object == nullptr || property.isEmpty())
        return -1;

    const int propIndex = object->metaObject()->indexOfProperty(property);
    if (propIndex < 0)
        return -1;

    const QByteArray counterProp = property + "_counter";
    const QVariant counterVal = object->property(counterProp);
    int counter = counterVal.isValid() ? counterVal.toInt() : 0;
    object->setProperty(counterProp, counter + 1);
    return counter;
}

void ObjectPropertyInfo::deleteSelf()
{
    delete this;
}

ObjectPropertyUndoCommand::ObjectPropertyUndoCommand(QObject *object, const QByteArray &property)
    : QUndoCommand()
{
    if (object != nullptr) {
        m_propertyInfo = ObjectPropertyInfo::get(object, property);
        if (m_propertyInfo != nullptr) {
            if (m_propertyInfo->isLocked())
                m_propertyInfo = nullptr;
            else {
                m_oldValue = m_propertyInfo->read();
                this->setText(QString("%1.%2").arg(object->metaObject()->className(),
                                                   QString::fromLatin1(property)));
                m_connection = QObject::connect(object, &QObject::destroyed, object, [this]() {
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
    if (m_propertyInfo != nullptr && UndoStack::active()) {
        m_newValue = m_propertyInfo->read();
        UndoStack::active()->push(this);
    } else
        delete this;
}

void ObjectPropertyUndoCommand::undo()
{
    if (m_propertyInfo != nullptr)
        m_propertyInfo->write(m_oldValue);
}

void ObjectPropertyUndoCommand::redo()
{
    if (!m_firstRedoDone) {
        m_firstRedoDone = true;
        return;
    }

    if (m_propertyInfo != nullptr)
        m_propertyInfo->write(m_newValue);
}

bool ObjectPropertyUndoCommand::mergeWith(const QUndoCommand *other)
{
    if (other->id() != m_propertyInfo->id)
        return false;

    const ObjectPropertyUndoCommand *cmd =
            reinterpret_cast<const ObjectPropertyUndoCommand *>(other);
    m_newValue = cmd->m_newValue;
    return true;
}

PushObjectPropertyUndoCommand::PushObjectPropertyUndoCommand(QObject *object,
                                                             const QByteArray &property, bool flag)
{
    const int counter = ObjectPropertyInfo::querySetCounter(object, property);
    if (flag && UndoStack::active() && counter > 0)
        m_command = new ObjectPropertyUndoCommand(object, property);
}

PushObjectPropertyUndoCommand::~PushObjectPropertyUndoCommand()
{
    if (m_command)
        m_command->pushToActiveStack();
}

///////////////////////////////////////////////////////////////////////////////

UndoResult::UndoResult(QObject *parent) : QObject(parent) { }

UndoResult::~UndoResult() { }

void UndoResult::setSuccess(bool val)
{
    if (m_success == val)
        return;

    m_success = val;
    emit successChanged();
}

Q_GLOBAL_STATIC(QList<UndoHandler *>, AllUndoHandlers);

QList<UndoHandler *> UndoHandler::all()
{
    return *AllUndoHandlers;
}

bool UndoHandler::handleUndo()
{
    const QList<UndoHandler *> handlers = UndoHandler::all();
    for (UndoHandler *handler : qAsConst(handlers)) {
        if (handler->isEnabled() && handler->canUndo()) {
            if (handler->undo())
                return true;
        }
    }

    return false;
}

bool UndoHandler::handleRedo()
{
    const QList<UndoHandler *> handlers = UndoHandler::all();
    for (UndoHandler *handler : qAsConst(handlers)) {
        if (handler->isEnabled() && handler->canRedo()) {
            if (handler->redo())
                return true;
        }
    }

    return false;
}

UndoHandler::UndoHandler(QObject *parent) : QObject(parent)
{
    ::AllUndoHandlers->append(this);
}

UndoHandler::~UndoHandler()
{
    ::AllUndoHandlers->removeOne(this);
}

void UndoHandler::setEnabled(bool val)
{
    if (m_enabled == val)
        return;

    m_enabled = val;
    emit enabledChanged();
}

void UndoHandler::setCanUndo(bool val)
{
    if (m_canUndo == val)
        return;

    m_canUndo = val;
    emit canUndoChanged();
}

void UndoHandler::setCanRedo(bool val)
{
    if (m_canRedo == val)
        return;

    m_canRedo = val;
    emit canRedoChanged();
}

bool UndoHandler::undo()
{
    UndoResult result;
    emit undoRequest(&result);
    return result.isSuccess();
}

bool UndoHandler::redo()
{
    UndoResult result;
    emit redoRequest(&result);
    return result.isSuccess();
}
