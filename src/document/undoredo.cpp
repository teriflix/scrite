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
#include "utils.h"

#include <QApplication>
#include <QQmlListReference>

void UndoHub::init(const char *uri, QQmlEngine *qmlEngine)
{
    Q_UNUSED(qmlEngine)

    static bool initedOnce = false;
    if (initedOnce)
        return;

    // @uri io.scrite.components
    // @reason Instantiation from QML not allowed.
    qmlRegisterSingletonInstance(uri, 1, 0, "UndoHub", UndoHub::instance());

    initedOnce = true;
}

UndoHub *UndoHub::instance()
{
    static QPointer<UndoHub> theInstance(new UndoHub(qApp));
    return theInstance;
}

UndoHub::UndoHub(QObject *parent) : QUndoGroup(parent)
{
    connect(this, &QUndoGroup::canUndoChanged, this, &UndoHub::_canUndoChanged);
    connect(this, &QUndoGroup::canRedoChanged, this, &UndoHub::_canRedoChanged);
    connect(this, &QUndoGroup::activeStackChanged, this, &UndoHub::_activeStackChanged);
}

UndoHub::~UndoHub() { }

void UndoHub::clearAllStacks()
{
    const QList<QUndoStack *> stacks = UndoHub::instance()->stacks();
    for (QUndoStack *stack : stacks)
        stack->clear();
}

bool UndoHub::enabled = true;

QUndoStack *UndoHub::active()
{
    return UndoHub::enabled ? UndoHub::instance()->activeStack() : nullptr;
}

///////////////////////////////////////////////////////////////////////////////

UndoStack::UndoStack(QObject *parent) : QUndoStack(parent)
{
    UndoHub::instance()->addStack(this);
}

UndoStack::~UndoStack()
{
    if (UndoHub::instance())
        UndoHub::instance()->removeStack(this);
}

void UndoStack::onActiveInGroupChanged(QUndoStack *stack)
{
    const bool a = stack == this;
    if (a != m_active) {
        m_active = a;
        emit activeChanged();
    }
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
    if (m_propertyInfo != nullptr && UndoHub::active()) {
        m_newValue = m_propertyInfo->read();
        UndoHub::active()->push(this);
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
    if (flag && UndoHub::active())
        m_command = new ObjectPropertyUndoCommand(object, property);
}

PushObjectPropertyUndoCommand::~PushObjectPropertyUndoCommand()
{
    if (m_command)
        m_command->pushToActiveStack();
}
