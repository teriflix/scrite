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

#include "notebooktabmodel.h"
#include "application.h"

NotebookTabModel::NotebookTabModel(QObject *parent)
    : QAbstractListModel(parent),
      m_activeScene(this, "activeScene"),
      m_structure(this, "structure")
{
    connect(this, &NotebookTabModel::rowsInserted, this, &NotebookTabModel::countChanged);
    connect(this, &NotebookTabModel::rowsRemoved, this, &NotebookTabModel::countChanged);
    connect(this, &NotebookTabModel::modelReset, this, &NotebookTabModel::countChanged);

    connect(this, &NotebookTabModel::countChanged, this, &NotebookTabModel::refreshed);
    connect(this, &NotebookTabModel::dataChanged, this, &NotebookTabModel::refreshed);
}

NotebookTabModel::~NotebookTabModel()
{

}

void NotebookTabModel::setStructure(Structure *val)
{
    if(m_structure == val)
        return;

    if(!m_structure.isNull())
        disconnect(m_structure, &Structure::characterCountChanged, this, &NotebookTabModel::updateModel);

    m_structure = val;

    if(!m_structure.isNull())
        connect(m_structure, &Structure::characterCountChanged, this, &NotebookTabModel::updateModel);

    emit structureChanged();

    this->resetModel();
}

void NotebookTabModel::setActiveScene(Scene *val)
{
    if(m_activeScene == val)
        return;

    m_activeScene = val;
    emit activeSceneChanged();

    this->updateModel();
}

QObject *NotebookTabModel::sourceAt(int row) const
{
    if(row < 0 || row >= m_items.size())
        return nullptr;

    return m_items.at(row).source;
}

QString NotebookTabModel::labelAt(int row) const
{
    if(row < 0 || row >= m_items.size())
        return QString();

    return m_items.at(row).label;
}

QColor NotebookTabModel::colorAt(int row) const
{
    if(row < 0 || row >= m_items.size())
        return QColor();

    return m_items.at(row).color;
}

QVariantMap NotebookTabModel::at(int row) const
{
    const Item item = (row < 0 || row >= m_items.size()) ? Item() : m_items.at(row);

    QVariantMap ret;
    ret["source"] = QVariant::fromValue<QObject*>(item.source);
    ret["label"] = item.label;
    ret["color"] = item.color;

    return ret;
}

int NotebookTabModel::indexOfSource(QObject *source) const
{
    Item item;
    item.source = source;
    return m_items.indexOf( item );
}

int NotebookTabModel::indexOfLabel(const QString &name) const
{
    for(int i=m_items.size()-1; i>=0; i--)
    {
        if(m_items.at(i).label == name)
            return i;
    }

    return -1;
}

int NotebookTabModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_items.size();
}

QVariant NotebookTabModel::data(const QModelIndex &index, int role) const
{
    if(index.row() < 0 || index.row() >= m_items.size())
        return QVariant();

    const Item item = m_items.at(index.row());

    switch(role)
    {
    case SourceRole:
        return QVariant::fromValue<QObject*>(item.source);
    case LabelRole:
        return item.label;
    case ColorRole:
        return item.color;
    case ModelDataRole:
        return this->at(index.row());
    default:
        break;
    }

    return QVariant();
}

QHash<int, QByteArray> NotebookTabModel::roleNames() const
{
    QHash<int,QByteArray> roles;
    roles[SourceRole] = "source";
    roles[LabelRole] = "label";
    roles[ColorRole] = "color";
    roles[ModelDataRole] = "modelData";
    return roles;
}

void NotebookTabModel::resetStructure()
{
    m_structure = nullptr;
    emit structureChanged();

    this->resetModel();
}

void NotebookTabModel::resetActiveScene()
{
    m_activeScene = nullptr;
    emit activeSceneChanged();

    this->updateModel();
}

void NotebookTabModel::updateModel()
{
    if(m_items.isEmpty())
    {
        this->resetModel();
        return;
    }

    if(m_items.first().source != m_structure)
    {
        this->resetModel();
        return;
    }

    if(!m_activeScene.isNull())
    {
        const Item sceneItem(m_activeScene);
        if(m_items.size() >= 2 && m_items.at(1).source->inherits("Scene"))
        {
            const QModelIndex idx = this->index(1, 0, QModelIndex());
            m_items.replace(1, sceneItem);
            emit dataChanged(idx, idx);
        }
        else
        {
            this->beginInsertRows(QModelIndex(), 1, 1);
            m_items.insert(1, sceneItem);
            this->endInsertRows();
        }
    }
    else
    {
        if(m_items.size() >= 2 && m_items.at(1).source->inherits("Scene"))
        {
            this->beginRemoveRows(QModelIndex(), 1, 1);
            m_items.removeAt(1);
            this->endRemoveRows();
        }
    }

    QList<NotebookTabModel::Item> newItems = this->gatherItems(true);
    if(newItems.isEmpty())
    {
        const int expectedSize = 1 + (m_activeScene.isNull() ? 0 : 1);
        if(m_items.size() > expectedSize)
        {
            this->beginRemoveRows(QModelIndex(), expectedSize, m_items.size()-1);
            while(m_items.size() > expectedSize)
                m_items.takeLast();
            this->endRemoveRows();
        }

        return;
    }

    // First remove items from existing list that are not present in the new list.
    for(int i=m_items.size()-1; i>=0; i--)
    {
        const Item item = m_items.at(i);
        if(item.source == m_structure || (item.source == m_activeScene && !m_activeScene.isNull()))
            continue;

        const int ni = newItems.indexOf(item);
        if(ni >= 0)
        {
            newItems.removeAt(ni);
            continue;
        }

        this->beginRemoveRows(QModelIndex(), i, i);
        m_items.removeAt(i);
        this->endRemoveRows();
    }

    // Now newItems consists of only those items that have to be included in the scene
    if(!newItems.isEmpty())
    {
        this->beginInsertRows(QModelIndex(), m_items.size(), m_items.size()+newItems.size()-1);
        m_items += newItems;
        this->endInsertRows();
    }
}

void NotebookTabModel::resetModel()
{
    this->beginResetModel();
    m_items = this->gatherItems();
    this->endResetModel();
}

QList<NotebookTabModel::Item> NotebookTabModel::gatherItems(bool charactersOnly) const
{
    QList<NotebookTabModel::Item> ret;

    if(!m_structure.isNull())
    {
        if(!charactersOnly)
        {
            ret << Item(m_structure);
            if(!m_activeScene.isNull())
                ret << Item(m_activeScene);
        }

        for(int i=0; i<m_structure->characterCount(); i++)
            ret << Item(m_structure->characterAt(i), Application::instance()->pickStandardColor(i));
    }

    return ret;
}
