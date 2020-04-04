/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "screenplay.h"
#include "scritedocument.h"

ScreenplayElement::ScreenplayElement(QObject *parent)
    : QObject(parent),
      m_scene(nullptr),
      m_expanded(true),
      m_screenplay(qobject_cast<Screenplay*>(parent))
{
    connect(this, &ScreenplayElement::sceneChanged, this, &ScreenplayElement::elementChanged);
    connect(this, &ScreenplayElement::expandedChanged, this, &ScreenplayElement::elementChanged);
}

ScreenplayElement::~ScreenplayElement()
{
    emit aboutToDelete(this);
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

    this->setScene(element->scene());
    if(m_scene != nullptr)
        m_sceneID.clear();
}

QString ScreenplayElement::sceneID() const
{
    return m_scene ? m_scene->id() : QString();
}

void ScreenplayElement::setScene(Scene *val)
{
    if(m_scene == val || m_scene != nullptr || val == nullptr)
        return;

    m_scene = val;
    connect(m_scene, &Scene::aboutToDelete, this, &ScreenplayElement::deleteLater);
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
      m_scriteDocument(qobject_cast<ScriteDocument*>(parent)),
      m_currentElementIndex(-1),
      m_activeScene(nullptr)
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
    this->insertAt(ptr, -1);
}

void Screenplay::insertAt(ScreenplayElement *ptr, int index)
{
    if(ptr == nullptr || m_elements.indexOf(ptr) >= 0)
        return;

    index = (index < 0 || index >= m_elements.size()) ? m_elements.size() : index;

    this->beginInsertRows(QModelIndex(), index, index);
    if(index == m_elements.size())
        m_elements.append(ptr);
    else
        m_elements.insert(index, ptr);

    ptr->setParent(this);
    connect(ptr, &ScreenplayElement::elementChanged, this, &Screenplay::screenplayChanged);
    connect(ptr, &ScreenplayElement::aboutToDelete, this, &Screenplay::removeElement);

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

    this->beginRemoveRows(QModelIndex(), row, row);
    m_elements.removeAt(row);

    disconnect(ptr, &ScreenplayElement::elementChanged, this, &Screenplay::screenplayChanged);
    disconnect(ptr, &ScreenplayElement::aboutToDelete, this, &Screenplay::removeElement);

    this->endRemoveRows();

    emit elementCountChanged();
    emit elementsChanged();

    this->setCurrentElementIndex(-1);

    if(ptr->parent() == this)
        ptr->deleteLater();
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
}

ScreenplayElement *Screenplay::elementAt(int index) const
{
    return index < 0 || index >= m_elements.size() ? nullptr : m_elements.at(index);
}

int Screenplay::elementCount() const
{
    return m_elements.size();
}

void Screenplay::clearElements()
{
    while(m_elements.size())
        this->removeElement(m_elements.first());
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


