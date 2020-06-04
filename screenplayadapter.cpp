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

#include "screenplay.h"
#include "garbagecollector.h"
#include "screenplayadapter.h"

ScreenplayAdapter::ScreenplayAdapter(QObject *parent)
    : QIdentityProxyModel(parent)
{
    connect(this, &ScreenplayAdapter::sourceChanged, this, &ScreenplayAdapter::elementCountChanged);
    connect(this, &ScreenplayAdapter::rowsInserted, this, &ScreenplayAdapter::elementCountChanged);
    connect(this, &ScreenplayAdapter::rowsRemoved, this, &ScreenplayAdapter::elementCountChanged);
    connect(this, &ScreenplayAdapter::modelReset, this, &ScreenplayAdapter::elementCountChanged);
}

ScreenplayAdapter::~ScreenplayAdapter()
{

}

void ScreenplayAdapter::setSource(QObject *val)
{
    if(m_source == val)
        return;

    if(this->sourceModel() != nullptr)
    {
        Screenplay *screenplay = qobject_cast<Screenplay*>(this->sourceModel());
        if(screenplay->parent() == this)
            GarbageCollector::instance()->add(screenplay);
        else
        {
            disconnect(this, &ScreenplayAdapter::currentIndexChanged, screenplay, &Screenplay::setCurrentElementIndex);
            disconnect(screenplay, &Screenplay::currentElementIndexChanged, this, &ScreenplayAdapter::setCurrentIndex);
        }
    }

    this->setCurrentIndex(-1);

    m_source = val;

    if(m_source != nullptr)
    {
        Screenplay *screenplay = qobject_cast<Screenplay*>(m_source);
        if(screenplay != nullptr)
        {
            this->setSourceModel(screenplay);

            connect(this, &ScreenplayAdapter::currentIndexChanged, screenplay, &Screenplay::setCurrentElementIndex);
            connect(screenplay, &Screenplay::currentElementIndexChanged, this, &ScreenplayAdapter::setCurrentIndex);
        }
        else
        {
            Scene *scene = qobject_cast<Scene*>(m_source);
            if(scene != nullptr)
            {
                screenplay = new Screenplay(this);

                ScreenplayElement *element = new ScreenplayElement(screenplay);
                element->setScene(scene);
                screenplay->addElement(element);

                this->setSourceModel(screenplay);
            }
            else
                this->setSourceModel(nullptr);
        }
    }
    else
        this->setSourceModel(nullptr);

    emit sourceChanged();
}

Screenplay *ScreenplayAdapter::screenplay() const
{
    return qobject_cast<Screenplay*>(this->sourceModel());
}

void ScreenplayAdapter::setCurrentIndex(int val)
{
    const int nrRows = this->rowCount();
    val = nrRows > 0 ? qBound(0, val, nrRows-1) : -1;
    if(m_currentIndex == val)
        return;

    m_currentIndex = val;

    if(m_currentIndex >= 0)
    {
        const QModelIndex index = this->index(m_currentIndex, 0);
        ScreenplayElement *element = this->data(index, ScreenplayElementRole).value<ScreenplayElement*>();
        this->setCurrentElement(element);
    }
    else
        this->setCurrentElement(nullptr);

    emit currentIndexChanged(m_currentIndex);
}

Scene *ScreenplayAdapter::currentScene() const
{
    return m_currentElement == nullptr ? nullptr : m_currentElement->scene();
}

QHash<int, QByteArray> ScreenplayAdapter::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[ScreenplayElementRole] = "screenplayElement";
    roles[ScreenplayElementTypeRole] = "screenplayElementType";
    roles[BreakTypeRole] = "breakType";
    roles[SceneRole] = "scene";
    return roles;
}

QVariant ScreenplayAdapter::data(const QModelIndex &index, int role) const
{
    if(this->sourceModel() == nullptr)
        return QVariant();

    const QModelIndex srcIndex = this->mapToSource(index);
    const QVariant srcData = this->sourceModel()->data(srcIndex, Screenplay::ScreenplayElementRole);
    ScreenplayElement *element = srcData.value<ScreenplayElement*>();
    switch(role)
    {
    case IdRole:
        return element->sceneID();
    case ScreenplayElementRole:
        return srcData;
    case ScreenplayElementTypeRole:
        return element->elementType();
    case BreakTypeRole:
        return element->breakType();
    case SceneRole:
        return QVariant::fromValue<Scene*>(element->scene());
    default:
        break;
    }

    return QVariant();
}

void ScreenplayAdapter::setCurrentElement(ScreenplayElement *val)
{
    if(m_currentElement == val)
        return;

    m_currentElement = val;
    emit currentElementChanged();
}

