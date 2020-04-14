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

#include "undostack.h"
#include "screenplay.h"
#include "scritedocument.h"
#include "garbagecollector.h"

ScreenplayElement::ScreenplayElement(QObject *parent)
    : QObject(parent),
      m_screenplay(qobject_cast<Screenplay*>(parent))
{
    connect(this, &ScreenplayElement::sceneChanged, this, &ScreenplayElement::elementChanged);
    connect(this, &ScreenplayElement::expandedChanged, this, &ScreenplayElement::elementChanged);
}

ScreenplayElement::~ScreenplayElement()
{
    emit aboutToDelete(this);
}

void ScreenplayElement::setElementType(ScreenplayElement::ElementType val)
{
    if(m_elementType == val || m_elementTypeIsSet)
        return;

    m_elementType = val;
    emit elementTypeChanged();
}

void ScreenplayElement::setBreakType(int val)
{
    if(m_breakType == val || m_elementType != BreakElementType)
        return;

    m_breakType = val;

    if(m_sceneID.isEmpty())
    {
        QString id;
        int chapter = 1;
        for(int i=0; i<m_screenplay->elementCount(); i++)
        {
            ScreenplayElement *element = m_screenplay->elementAt(i);
            if(element->elementType() == BreakElementType)
            {
                if(element->breakType() == m_breakType)
                    ++chapter;
            }
        }

        switch(m_breakType)
        {
        case Screenplay::Act:
            id = "Act ";
            break;
        case Screenplay::Chapter:
            id = "Chapter ";
            break;
        case Screenplay::Interval:
            id = "Interval";
            break;
        default:
            id = "Break";
        }

        if(m_breakType != Screenplay::Interval)
            id += QString::number(chapter);
        this->setSceneFromID(id);
    }

    emit breakTypeChanged();
}

void ScreenplayElement::setScreenplay(Screenplay *val)
{
    if(m_screenplay != nullptr || m_screenplay == val)
        return;

    m_screenplay = val;
    emit screenplayChanged();
}

void ScreenplayElement::setSceneFromID(const QString &val)
{
    m_sceneID = val;
    if(m_elementType == BreakElementType)
        return;

    if(m_screenplay == nullptr)
        return;

    ScriteDocument *document = m_screenplay->scriteDocument();
    if(document == nullptr)
        return;

    Structure *structure = document->structure();
    if(structure == nullptr)
        return;

    StructureElement *element = structure->findElementBySceneID(val);
    if(element == nullptr)
        return;

    m_elementTypeIsSet = true;
    this->setScene(element->scene());
    if(m_scene != nullptr)
        m_sceneID.clear();
}

QString ScreenplayElement::sceneID() const
{
    if(m_elementType == BreakElementType)
    {
        if(m_sceneID.isEmpty())
        {
            switch(m_breakType)
            {
            case Screenplay::Act: return "Act";
            case Screenplay::Chapter: return "Chapter";
            case Screenplay::Interval: return "Interval";
            default: break;
            }
            return "Break";
        }

        return m_sceneID;
    }

    return m_scene ? m_scene->id() : QString();
}

void ScreenplayElement::setScene(Scene *val)
{
    if(m_scene == val || m_scene != nullptr || val == nullptr)
        return;

    m_scene = val;
    connect(m_scene, &Scene::aboutToDelete, this, &ScreenplayElement::deleteLater);
    connect(m_scene, &Scene::sceneAboutToReset, this, &ScreenplayElement::sceneAboutToReset);
    connect(m_scene, &Scene::sceneReset, this, &ScreenplayElement::sceneReset);

    emit sceneChanged();
}

void ScreenplayElement::setExpanded(bool val)
{
    if(m_expanded == val)
        return;

    m_expanded = val;
    emit expandedChanged();
}

void ScreenplayElement::setUserData(const QJsonValue &val)
{
    if(m_userData == val)
        return;

    m_userData = val;
    emit userDataChanged();
}

bool ScreenplayElement::event(QEvent *event)
{
    if(event->type() == QEvent::ParentChange)
    {
        m_screenplay = qobject_cast<Screenplay*>(this->parent());
        if(m_scene == nullptr && !m_sceneID.isEmpty())
            this->setSceneFromID(m_sceneID);
    }

    return QObject::event(event);
}

///////////////////////////////////////////////////////////////////////////////

Screenplay::Screenplay(QObject *parent)
    : QAbstractListModel(parent),
      m_scriteDocument(qobject_cast<ScriteDocument*>(parent))
{
    connect(this, &Screenplay::titleChanged, this, &Screenplay::screenplayChanged);
    connect(this, &Screenplay::authorChanged, this, &Screenplay::screenplayChanged);
    connect(this, &Screenplay::contactChanged, this, &Screenplay::screenplayChanged);
    connect(this, &Screenplay::versionChanged, this, &Screenplay::screenplayChanged);
    connect(this, &Screenplay::elementsChanged, this, &Screenplay::screenplayChanged);

    m_author = QSysInfo::machineHostName();
    m_version = "0.1";
}

Screenplay::~Screenplay()
{

}

void Screenplay::setTitle(const QString &val)
{
    if(m_title == val)
        return;

    m_title = val;
    emit titleChanged();
}

void Screenplay::setAuthor(const QString &val)
{
    if(m_author == val)
        return;

    m_author = val;
    emit authorChanged();
}

void Screenplay::setContact(const QString &val)
{
    if(m_contact == val)
        return;

    m_contact = val;
    emit contactChanged();
}

void Screenplay::setVersion(const QString &val)
{
    if(m_version == val)
        return;

    m_version = val;
    emit versionChanged();
}

QQmlListProperty<ScreenplayElement> Screenplay::elements()
{
    return QQmlListProperty<ScreenplayElement>(
                reinterpret_cast<QObject*>(this),
                static_cast<void*>(this),
                &Screenplay::staticAppendElement,
                &Screenplay::staticElementCount,
                &Screenplay::staticElementAt,
                &Screenplay::staticClearElements);
}

void Screenplay::addElement(ScreenplayElement *ptr)
{
    this->insertElementAt(ptr, -1);
}

static void screenplayAppendElement(Screenplay *screenplay, ScreenplayElement *ptr) { screenplay->addElement(ptr); }
static void screenplayRemoveElement(Screenplay *screenplay, ScreenplayElement *ptr) { screenplay->removeElement(ptr); }
static void screenplayInsertElement(Screenplay *screenplay, ScreenplayElement *ptr, int index) { screenplay->insertElementAt(ptr, index); }
static ScreenplayElement *screenplayElementAt(Screenplay *screenplay, int index) { return screenplay->elementAt(index); }
static int screenplayIndexOfElement(Screenplay *screenplay, ScreenplayElement *ptr) { return screenplay->indexOfElement(ptr); }

void Screenplay::insertElementAt(ScreenplayElement *ptr, int index)
{
    if(ptr == nullptr || m_elements.indexOf(ptr) >= 0)
        return;

    index = (index < 0 || index >= m_elements.size()) ? m_elements.size() : index;

    QScopedPointer< PushObjectListCommand<Screenplay,ScreenplayElement> > cmd;
    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "elements");
    if(!info->isLocked())
    {
        ObjectListPropertyMethods<Screenplay,ScreenplayElement> methods(&screenplayAppendElement, &screenplayRemoveElement, &screenplayInsertElement, &screenplayElementAt, screenplayIndexOfElement);
        cmd.reset( new PushObjectListCommand<Screenplay,ScreenplayElement> (ptr, this, info->property, ObjectList::InsertOperation, methods) );
    }

    this->beginInsertRows(QModelIndex(), index, index);
    if(index == m_elements.size())
        m_elements.append(ptr);
    else
        m_elements.insert(index, ptr);

    ptr->setParent(this);
    connect(ptr, &ScreenplayElement::elementChanged, this, &Screenplay::screenplayChanged);
    connect(ptr, &ScreenplayElement::aboutToDelete, this, &Screenplay::removeElement);
    connect(ptr, &ScreenplayElement::sceneReset, this, &Screenplay::onSceneReset);

    this->endInsertRows();

    emit elementCountChanged();
    emit elementsChanged();
}

void Screenplay::removeElement(ScreenplayElement *ptr)
{
    if(ptr == nullptr)
        return;

    const int row = m_elements.indexOf(ptr);
    if(row < 0)
        return;

    QScopedPointer< PushObjectListCommand<Screenplay,ScreenplayElement> > cmd;
    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "elements");
    if(!info->isLocked())
    {
        ObjectListPropertyMethods<Screenplay,ScreenplayElement> methods(&screenplayAppendElement, &screenplayRemoveElement, &screenplayInsertElement, &screenplayElementAt, screenplayIndexOfElement);
        cmd.reset( new PushObjectListCommand<Screenplay,ScreenplayElement> (ptr, this, info->property, ObjectList::RemoveOperation, methods) );
    }

    this->beginRemoveRows(QModelIndex(), row, row);
    m_elements.removeAt(row);

    disconnect(ptr, &ScreenplayElement::elementChanged, this, &Screenplay::screenplayChanged);
    disconnect(ptr, &ScreenplayElement::aboutToDelete, this, &Screenplay::removeElement);
    disconnect(ptr, &ScreenplayElement::sceneReset, this, &Screenplay::onSceneReset);

    this->endRemoveRows();

    emit elementCountChanged();
    emit elementsChanged();

    this->setCurrentElementIndex(-1);

    if(ptr->parent() == this)
        GarbageCollector::instance()->add(ptr);
}

class ScreenplayElementMoveCommand : public QUndoCommand
{
public:
    ScreenplayElementMoveCommand(Screenplay *screenplay, ScreenplayElement *element, int fromRow, int toRow);
    ~ScreenplayElementMoveCommand();

    static bool lock;

    // QUndoCommand interface
    void undo();
    void redo();

private:
    int m_toRow = -1;
    int m_fromRow = -1;
    Screenplay *m_screenplay = nullptr;
    ScreenplayElement *m_element = nullptr;
    QMetaObject::Connection m_connection1;
    QMetaObject::Connection m_connection2;
};

bool ScreenplayElementMoveCommand::lock = false;

ScreenplayElementMoveCommand::ScreenplayElementMoveCommand(Screenplay *screenplay, ScreenplayElement *element, int fromRow, int toRow)
    : QUndoCommand(),
      m_toRow(toRow),
      m_fromRow(fromRow),
      m_screenplay(screenplay),
      m_element(element)
{
    m_connection1 = QObject::connect(m_screenplay, &QObject::destroyed, [this]() {
        m_screenplay = nullptr;
        m_element = nullptr;
        this->setObsolete(true);
    });
    m_connection2 = QObject::connect(m_element, &QObject::destroyed, [this]() {
        m_screenplay = nullptr;
        m_element = nullptr;
        this->setObsolete(true);
    });
}

ScreenplayElementMoveCommand::~ScreenplayElementMoveCommand()
{
    QObject::disconnect(m_connection1);
    QObject::disconnect(m_connection2);
}

void ScreenplayElementMoveCommand::undo()
{
    if(m_screenplay == nullptr || m_element == nullptr)
        return;

    lock = true;
    m_screenplay->moveElement(m_element, m_fromRow);
    lock = false;
}

void ScreenplayElementMoveCommand::redo()
{
    if(m_screenplay == nullptr || m_element == nullptr)
        return;

    lock = true;
    m_screenplay->moveElement(m_element, m_toRow);
    lock = false;
}

void Screenplay::moveElement(ScreenplayElement *ptr, int toRow)
{
    if(ptr == nullptr || toRow >= m_elements.size())
        return;

    if(toRow < 0)
        toRow = m_elements.size()-1;

    const int fromRow = m_elements.indexOf(ptr);
    if(fromRow < 0)
        return;

    if(fromRow == toRow)
        return;

    this->beginMoveRows(QModelIndex(), fromRow, fromRow, QModelIndex(), toRow < fromRow ? toRow : toRow+1);
    m_elements.move(fromRow, toRow);
    this->endMoveRows();

    if(fromRow == m_currentElementIndex)
        this->setCurrentElementIndex(toRow);

    emit elementsChanged();

    if(UndoStack::active() != nullptr && !ScreenplayElementMoveCommand::lock)
        UndoStack::active()->push(new ScreenplayElementMoveCommand(this, ptr, fromRow, toRow));
}

ScreenplayElement *Screenplay::elementAt(int index) const
{
    return index < 0 || index >= m_elements.size() ? nullptr : m_elements.at(index);
}

int Screenplay::elementCount() const
{
    return m_elements.size();
}

class UndoClearScreenplayCommand : public QUndoCommand
{
public:
    UndoClearScreenplayCommand(Screenplay *screenplay, const QStringList &sceneIds);
    ~UndoClearScreenplayCommand();

    // QUndoCommand interface
    void undo();
    void redo();

private:
    bool m_firstRedoDone = false;
    char m_padding[7];
    QStringList m_sceneIds;
    Screenplay *m_screenplay = nullptr;
    QMetaObject::Connection m_connection;
};

UndoClearScreenplayCommand::UndoClearScreenplayCommand(Screenplay *screenplay, const QStringList &sceneIds)
    : QUndoCommand(), m_sceneIds(sceneIds), m_screenplay(screenplay)
{
    m_connection = QObject::connect(m_screenplay, &Screenplay::destroyed, [this]() {
        this->setObsolete(true);
    });
}

UndoClearScreenplayCommand::~UndoClearScreenplayCommand()
{
    QObject::disconnect(m_connection);
}

void UndoClearScreenplayCommand::undo()
{
    ObjectPropertyInfo *info = ObjectPropertyInfo::get(m_screenplay, "elements");
    if(info) info->lock();

    ScriteDocument *document = m_screenplay->scriteDocument();
    Structure *structure = document->structure();
    Q_FOREACH(QString sceneId, m_sceneIds)
    {
        StructureElement *element = structure->findElementBySceneID(sceneId);
        if(element == nullptr)
            continue;

        Scene *scene = element->scene();
        ScreenplayElement *screenplayElement = new ScreenplayElement(m_screenplay);
        screenplayElement->setScene(scene);
        m_screenplay->addElement(screenplayElement);
    }

    if(info) info->unlock();
}

void UndoClearScreenplayCommand::redo()
{
    if(!m_firstRedoDone)
    {
        m_firstRedoDone = true;
        return;
    }

    ObjectPropertyInfo *info = ObjectPropertyInfo::get(m_screenplay, "elements");
    if(info) info->lock();

    while(m_screenplay->elementCount())
        m_screenplay->removeElement( m_screenplay->elementAt(0) );

    if(info) info->unlock();
}

void Screenplay::clearElements()
{
    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "elements");
    if(info) info->lock();

    QStringList sceneIds;
    while(m_elements.size())
    {
        sceneIds << m_elements.first()->sceneID();
        this->removeElement(m_elements.first());
    }

    if(UndoStack::active())
        UndoStack::active()->push(new UndoClearScreenplayCommand(this, sceneIds));

    if(info) info->unlock();
}

int Screenplay::indexOfScene(Scene *scene) const
{
    if(scene == nullptr)
        return -1;

    for(int i=0; i<m_elements.size(); i++)
    {
        ScreenplayElement *element = m_elements.at(i);
        if(element->scene() == scene)
            return i;
    }

    return -1;
}

int Screenplay::indexOfElement(ScreenplayElement *element) const
{
    return m_elements.indexOf(element);
}

void Screenplay::addBreakElement(Screenplay::BreakType type)
{
    this->insertBreakElement(type, -1);
}

void Screenplay::insertBreakElement(Screenplay::BreakType type, int index)
{
    ScreenplayElement *element = new ScreenplayElement(this);
    element->setElementType(ScreenplayElement::BreakElementType);
    element->setBreakType(type);
    this->insertElementAt(element, index);
}

void Screenplay::setCurrentElementIndex(int val)
{
    val = qBound(-1, val, m_elements.size()-1);
    if(m_currentElementIndex == val)
        return;

    m_currentElementIndex = val;
    emit currentElementIndexChanged();

    if(m_currentElementIndex >= 0)
    {
        ScreenplayElement *element = m_elements.at(m_currentElementIndex);
        this->setActiveScene(element->scene());
    }
    else
        this->setActiveScene(nullptr);
}

void Screenplay::setActiveScene(Scene *val)
{
    if(m_activeScene == val)
        return;

    // Ensure that the scene belongs to this screenplay.
    if(m_currentElementIndex >= 0)
    {
        ScreenplayElement *element = this->elementAt(m_currentElementIndex);
        if(element && element->scene() == val)
        {
            m_activeScene = val;
            emit activeSceneChanged();
            return;
        }
    }

    const int index = this->indexOfScene(val);
    if(index < 0)
    {
        if(m_activeScene != nullptr)
        {
            m_activeScene = nullptr;
            emit activeSceneChanged();
        }
    }
    else
    {
        m_activeScene = val;
        emit activeSceneChanged();
    }

    this->setCurrentElementIndex(index);
}

QJsonArray Screenplay::search(const QString &text, int flags) const
{
    QJsonArray ret;

    const int nrScenes = m_elements.size();
    for(int i=0; i<nrScenes; i++)
    {
        Scene *scene = m_elements.at(i)->scene();

        const int nrElements = scene->elementCount();
        for(int j=0; j<nrElements; j++)
        {
            SceneElement *element = scene->elementAt(j);

            const QJsonArray results = element->find(text, flags);
            if(!results.isEmpty())
            {
                for(int r=0; r<results.size(); r++)
                {
                    const QJsonObject result = results.at(i).toObject();

                    QJsonObject item;
                    item.insert("sceneIndex", i);
                    item.insert("elementIndex", j);
                    item.insert("from", result.value("from"));
                    item.insert("to", result.value("to"));
                    ret.append(item);
                }
            }
        }
    }

    return ret;
}

int Screenplay::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_elements.size();
}

QVariant Screenplay::data(const QModelIndex &index, int role) const
{
    if(role == ScreenplayElementRole && index.isValid())
        return QVariant::fromValue<QObject*>(this->elementAt(index.row()));

    return QVariant();
}

QHash<int, QByteArray> Screenplay::roleNames() const
{
    QHash<int,QByteArray> roles;
    roles[ScreenplayElementRole] = "screenplayElement";
    return roles;
}

bool Screenplay::event(QEvent *event)
{
    if(event->type() == QEvent::ParentChange)
        m_scriteDocument = qobject_cast<ScriteDocument*>(this->parent());

    return QObject::event(event);
}

void Screenplay::onSceneReset(int elementIndex)
{
    ScreenplayElement *element = qobject_cast<ScreenplayElement*>(this->sender());
    if(element == nullptr)
        return;

    int sceneIndex = this->indexOfElement(element);
    if(sceneIndex < 0)
        return;

    emit sceneReset(sceneIndex, elementIndex);
}

void Screenplay::staticAppendElement(QQmlListProperty<ScreenplayElement> *list, ScreenplayElement *ptr)
{
    reinterpret_cast< Screenplay* >(list->data)->addElement(ptr);
}

void Screenplay::staticClearElements(QQmlListProperty<ScreenplayElement> *list)
{
    reinterpret_cast< Screenplay* >(list->data)->clearElements();
}

ScreenplayElement *Screenplay::staticElementAt(QQmlListProperty<ScreenplayElement> *list, int index)
{
    return reinterpret_cast< Screenplay* >(list->data)->elementAt(index);
}

int Screenplay::staticElementCount(QQmlListProperty<ScreenplayElement> *list)
{
    return reinterpret_cast< Screenplay* >(list->data)->elementCount();
}

