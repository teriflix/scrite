/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "screenplaytreeadapter.h"

#include <QStack>

ScreenplayTreeAdapter::ScreenplayTreeAdapter(QObject *parent) : QAbstractItemModel(parent) { }

ScreenplayTreeAdapter::~ScreenplayTreeAdapter()
{
    this->clear();
}

void ScreenplayTreeAdapter::setIncludeFormalTags(bool val)
{
    if (m_includeFormalTags == val)
        return;

    m_includeFormalTags = val;
    emit includeFormalTagsChanged();

    this->reload();
}

void ScreenplayTreeAdapter::setIncludeOpenTags(bool val)
{
    if (m_includeOpenTags == val)
        return;

    m_includeOpenTags = val;
    emit includeOpenTagsChanged();

    this->reload();
}

void ScreenplayTreeAdapter::setAllowedOpenTags(const QStringList &val)
{
    if (m_allowedOpenTags == val)
        return;

    m_allowedOpenTags = val;
    emit allowedOpenTagsChanged();
}

void ScreenplayTreeAdapter::setScreenplay(Screenplay *val)
{
    if (m_screenplay == val)
        return;

    if (m_screenplay != nullptr)
        disconnect(m_screenplay, nullptr, this, nullptr);

    m_screenplay = val;

    if (m_screenplay != nullptr) {
        connect(m_screenplay, &QAbstractListModel::modelReset, this,
                &ScreenplayTreeAdapter::reload);

        /** These have to be handled better, we cannot be reloading all the time */
        // clang-format off
        connect(m_screenplay, &QAbstractListModel::rowsInserted, this,
                &ScreenplayTreeAdapter::reload);
        connect(m_screenplay, &QAbstractListModel::rowsRemoved, this,
                &ScreenplayTreeAdapter::reload);
        connect(m_screenplay, &QAbstractListModel::rowsMoved, this,
                &ScreenplayTreeAdapter::reload);
        connect(m_screenplay, &QAbstractListModel::dataChanged, this,
                &ScreenplayTreeAdapter::reload);
        // clang-format on
    }

    emit screenplayChanged();

    this->reload();
}

void ScreenplayTreeAdapter::classBegin()
{
    m_componentComplete = false;
}

void ScreenplayTreeAdapter::componentComplete()
{
    m_componentComplete = true;

    this->reload();
}

QModelIndex ScreenplayTreeAdapter::index(int row, int column, const QModelIndex &parent) const
{
    if (column != 0)
        return QModelIndex();

    Item *ret = nullptr;
    if (parent.isValid()) {
        Item *parentItem = Item::get(parent);
        if (parentItem == nullptr || row < 0 || row >= parentItem->children.size())
            return QModelIndex();

        ret = parentItem->children.at(row);
    } else {
        if (row < 0 || row >= m_items.size())
            return QModelIndex();

        ret = m_items[row];
    }

    return this->createIndex(row, column, ret);
}

QModelIndex ScreenplayTreeAdapter::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    Item *childItem = Item::get(child);
    if (childItem == nullptr)
        return QModelIndex();

    Item *parentItem = childItem->parent;
    int parentRow = parentItem->parent ? parentItem->parent->children.indexOf(parentItem)
                                       : m_items.indexOf(parentItem);
    return this->createIndex(parentRow, 0, parentItem);
}

int ScreenplayTreeAdapter::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        Item *parentItem = Item::get(parent);
        return parentItem ? parentItem->children.size() : 0;
    }

    return m_items.size();
}

int ScreenplayTreeAdapter::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant ScreenplayTreeAdapter::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const Item *item = Item::get(index);
    if (item == nullptr)
        return QVariant();

    switch (role) {
    case TextRole:
        return item->text;
    case ScreenplayElementRole:
        return QVariant::fromValue<QObject *>(item->element);
    default:
        break;
    }

    return QVariant();
}

QHash<int, QByteArray> ScreenplayTreeAdapter::roleNames() const
{
    return { { TextRole, QByteArrayLiteral("displayText") },
             { ScreenplayElementRole, QByteArrayLiteral("screenplayElement") } };
}

void ScreenplayTreeAdapter::clear()
{
    this->beginResetModel();
    qDeleteAll(m_items);
    m_items.clear();
    this->endResetModel();
}

void ScreenplayTreeAdapter::reload()
{
    if (!m_componentComplete || m_screenplay == nullptr) {
        if (!m_items.isEmpty())
            this->clear();
        return;
    }

    this->beginResetModel();

    if (!m_items.isEmpty()) {
        qDeleteAll(m_items);
        m_items.clear();
    }

    /**
     * The hierarchy is
     * Episode > Act > Tag > Tag > Element
     * Here tag can be a formal tag, or open-tag.
     */
    QStack<Item *> parentStack;

    for (int i = 0; i < m_screenplay->elementCount(); i++) {
        ScreenplayElement *element = m_screenplay->elementAt(i);

        if (element->elementType() == ScreenplayElement::SceneElementType) {
            Item *parent = parentStack.isEmpty() ? nullptr : parentStack.top();
            Item *item = new Item(parent);
            if (parent == nullptr)
                m_items.append(item);

            item->type = SceneType;
            item->element = element;

            const Scene *scene = element->scene();
            if (scene->heading()->isEnabled())
                item->text = QString("%1. %2").arg(element->resolvedSceneNumber(),
                                                   scene->heading()->displayText());
            else
                item->text = QStringLiteral("NO SCENE HEADING");
        } else {
            if (element->breakType() == Screenplay::Interval)
                continue; // we ignore interval break, since it makes no sense here.

            ItemType itemType = element->breakType() == Screenplay::Episode ? EpisodeType : ActType;
            while (!parentStack.isEmpty()) {
                Item *top = parentStack.pop();
                if (top->type == itemType)
                    break;
            }

            Item *parent = parentStack.isEmpty() ? nullptr : parentStack.top();
            Item *item = new Item(parent);
            if (parent == nullptr)
                m_items.append(item);

            item->type = itemType;
            item->element = element;
            item->text = element->breakTitle();
            if (!element->breakSubtitle().isEmpty())
                item->text += QStringLiteral(": ") + element->breakSubtitle();
        }
    }

    this->endResetModel();
}

ScreenplayTreeAdapter::Item *ScreenplayTreeAdapter::Item::get(const QModelIndex &index)
{
    if (index.isValid())
        return static_cast<Item *>(index.internalPointer());

    return nullptr;
}

ScreenplayTreeAdapter::Item::Item(Item *parent)
{
    if (parent)
        parent->children.append(this);
}

ScreenplayTreeAdapter::Item::~Item()
{
    qDeleteAll(children);
    if (parent)
        parent->children.removeOne(this);
}
