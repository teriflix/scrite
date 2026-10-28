/****************************************************************************
**
** Copyright (C) 2026 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "utils.h"
#include "noteskvstore.h"

#include <QJsonObject>

struct NotesKVStore::Pair
{
    QString key;
    QJsonValue value;
};

struct NotesKVStore::Category
{
    QString name;
    QList<Pair> pairs;
};

NotesKVStore::NotesKVStore(QObject *parent) : QAbstractItemModel(parent)
{
    this->insertCategory(rootCategoryName());

    connect(this, &QAbstractItemModel::dataChanged, this, &NotesKVStore::storeModified);
    connect(this, &QAbstractItemModel::rowsInserted, this, &NotesKVStore::storeModified);
    connect(this, &QAbstractItemModel::rowsRemoved, this, &NotesKVStore::storeModified);
    connect(this, &QAbstractItemModel::modelReset, this, &NotesKVStore::storeModified);
}

NotesKVStore::~NotesKVStore()
{
    qDeleteAll(m_categories);
}

bool NotesKVStore::isEmpty() const
{
    return m_categories.size() == 1 && m_categories.first()->pairs.isEmpty();
}

bool NotesKVStore::addCategory(const QString &name)
{
    return this->insertCategory(name.trimmed()) != nullptr;
}

bool NotesKVStore::removeCategory(const QString &name)
{
    Category *category = this->findCategory(name);
    const int row = m_categories.indexOf(category);
    if (row <= 0)
        return false;

    this->beginRemoveRows(QModelIndex(), row, row);
    delete m_categories.takeAt(row);
    this->endRemoveRows();
    return true;
}

bool NotesKVStore::renameCategory(const QString &name, const QString &newName)
{
    // Root, which is the first category, cannot be renamed.
    Category *category = this->findCategory(name);
    const QString trimmedName = newName.trimmed();
    const Category *other = this->findCategory(trimmedName);
    if (m_categories.indexOf(category) <= 0 || trimmedName.isEmpty()
        || category->name == trimmedName || (other != nullptr && other != category))
        return false;

    category->name = trimmedName;

    const QModelIndex index = this->categoryIndex(category);
    emit dataChanged(index, index, { Qt::DisplayRole, KeyRole });
    return true;
}

bool NotesKVStore::renameKey(const QString &category, const QString &key, const QString &newKey)
{
    Category *owner = this->findCategory(category);
    const int row = owner == nullptr ? -1 : this->findPair(owner, key);
    const QString trimmedKey = newKey.trimmed();
    const int other = row < 0 ? -1 : this->findPair(owner, trimmedKey);
    if (row < 0 || trimmedKey.isEmpty() || (other >= 0 && other != row))
        return false;

    return this->changePair(owner, row, trimmedKey, owner->pairs.at(row).value);
}

bool NotesKVStore::setValue(const QString &category, const QString &key, const QJsonValue &value,
                            bool createCategory)
{
    const QString trimmedKey = key.trimmed();
    if (trimmedKey.isEmpty())
        return false;

    Category *owner = this->findCategory(category);
    if (owner == nullptr && createCategory)
        owner = this->insertCategory(category.trimmed());
    if (owner == nullptr)
        return false;

    const int row = this->findPair(owner, trimmedKey);
    if (row < 0)
        this->insertPair(owner, trimmedKey, value);
    else
        this->changePair(owner, row, owner->pairs.at(row).key, value);

    return true;
}

bool NotesKVStore::removeValue(const QString &category, const QString &key)
{
    Category *owner = this->findCategory(category);
    const int row = owner == nullptr ? -1 : this->findPair(owner, key);
    if (row < 0)
        return false;

    this->beginRemoveRows(this->categoryIndex(owner), row, row);
    owner->pairs.removeAt(row);
    this->endRemoveRows();

    const QModelIndex index = this->categoryIndex(owner);
    emit dataChanged(index, index, { PairCountRole });
    return true;
}

bool NotesKVStore::hasKey(const QString &category, const QString &key) const
{
    const Category *owner = this->findCategory(category);
    return owner != nullptr && this->findPair(owner, key) >= 0;
}

QJsonValue NotesKVStore::value(const QString &category, const QString &key) const
{
    const Category *owner = this->findCategory(category);
    const int row = owner == nullptr ? -1 : this->findPair(owner, key);
    return row < 0 ? QJsonValue() : owner->pairs.at(row).value;
}

QStringList NotesKVStore::categoryNames() const
{
    QStringList ret;
    for (const Category *category : m_categories)
        ret << category->name;
    return ret;
}

void NotesKVStore::clear()
{
    if (this->isEmpty())
        return;

    this->beginResetModel();
    while (m_categories.size() > 1)
        delete m_categories.takeLast();
    m_categories.first()->pairs.clear();
    this->endResetModel();
}

void NotesKVStore::merge(const NotesKVStore *other)
{
    if (other == nullptr || other == this)
        return;

    for (const Category *source : other->m_categories) {
        Category *target = this->findCategory(source->name);
        if (target == nullptr)
            target = this->insertCategory(source->name);

        for (const Pair &pair : source->pairs) {
            const int row = this->findPair(target, pair.key);
            if (row < 0) {
                this->insertPair(target, pair.key, pair.value);
                continue;
            }

            const Pair existing = target->pairs.at(row);
            if (existing.value == pair.value)
                continue;

            this->changePair(target, row,
                             this->uniqueKey(target, existing.key + QStringLiteral("_original")),
                             existing.value);
            this->insertPair(target,
                             this->uniqueKey(target, pair.key + QStringLiteral("_merged")),
                             pair.value);
        }
    }
}

void NotesKVStore::renameCharacter(const QString &from, const QString &to)
{
    for (Category *category : std::as_const(m_categories)) {
        for (int row = 0; row < category->pairs.size(); row++) {
            const Pair pair = category->pairs.at(row);
            if (!pair.value.isString())
                continue;

            int nrReplacements = 0;
            const QString newValue = Utils::SMath::replaceCharacterName(
                    from, to, pair.value.toString(), &nrReplacements);
            if (nrReplacements > 0)
                this->changePair(category, row, pair.key, newValue);
        }
    }
}

QJsonArray NotesKVStore::toJson() const
{
    QJsonArray ret;
    for (const Category *category : m_categories) {
        QJsonArray pairs;
        for (const Pair &pair : category->pairs) {
            QJsonObject jsPair;
            jsPair.insert(QStringLiteral("key"), pair.key);
            jsPair.insert(QStringLiteral("value"), pair.value);
            pairs.append(jsPair);
        }

        QJsonObject jsCategory;
        jsCategory.insert(QStringLiteral("category"), category->name);
        jsCategory.insert(QStringLiteral("pairs"), pairs);
        ret.append(jsCategory);
    }
    return ret;
}

void NotesKVStore::fromJson(const QJsonArray &array)
{
    // Loaded into a store of its own, which starts with Root as its first category, whatever the
    // array has, and is then swapped in, so that views see just one reset.
    NotesKVStore loaded;

    for (const QJsonValue &jsCategoryValue : array) {
        const QJsonObject jsCategory = jsCategoryValue.toObject();
        const QString name = jsCategory.value(QStringLiteral("category")).toString().trimmed();
        if (name.isEmpty())
            continue;

        Category *category = loaded.findCategory(name);
        if (category == nullptr)
            category = loaded.insertCategory(name);

        const QJsonArray jsPairs = jsCategory.value(QStringLiteral("pairs")).toArray();
        for (const QJsonValue &jsPairValue : jsPairs) {
            const QJsonObject jsPair = jsPairValue.toObject();
            const QString key = jsPair.value(QStringLiteral("key")).toString().trimmed();
            const QJsonValue value = jsPair.value(QStringLiteral("value"));
            if (!key.isEmpty() && loaded.findPair(category, key) < 0)
                loaded.insertPair(category, key, value.isUndefined() ? QJsonValue(QString()) : value);
        }
    }

    this->beginResetModel();
    qDeleteAll(m_categories);
    m_categories = loaded.m_categories;
    loaded.m_categories.clear();
    this->endResetModel();
}

QModelIndex NotesKVStore::index(int row, int column, const QModelIndex &parent) const
{
    if (!this->hasIndex(row, column, parent))
        return QModelIndex();

    if (!parent.isValid())
        return this->createIndex(row, column, nullptr);

    return this->createIndex(row, column, m_categories.at(parent.row()));
}

QModelIndex NotesKVStore::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    Category *category = static_cast<Category *>(child.internalPointer());
    return category == nullptr ? QModelIndex() : this->categoryIndex(category);
}

int NotesKVStore::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_categories.size();

    // A DelegateModel is given a new model before its rootIndex, which is the index of another
    // store until then, so an index can be one that this store has never made.
    if (parent.model() != this || parent.internalPointer() != nullptr || parent.column() != 0
        || parent.row() >= m_categories.size())
        return 0;

    return m_categories.at(parent.row())->pairs.size();
}

int NotesKVStore::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant NotesKVStore::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const Category *owner = static_cast<const Category *>(index.internalPointer());
    const bool isCategory = owner == nullptr;

    switch (role) {
    case Qt::DisplayRole:
    case KeyRole:
        return isCategory ? m_categories.at(index.row())->name : owner->pairs.at(index.row()).key;
    case ValueRole:
        return isCategory ? QVariant() : owner->pairs.at(index.row()).value.toVariant();
    case IsCategoryRole:
        return isCategory;
    case PairCountRole:
        return isCategory ? m_categories.at(index.row())->pairs.size() : 0;
    }

    return QVariant();
}

bool NotesKVStore::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || (role != KeyRole && role != ValueRole))
        return false;

    Category *owner = static_cast<Category *>(index.internalPointer());

    // Only the name of a category can change.
    if (owner == nullptr)
        return role == KeyRole
                && this->renameCategory(m_categories.at(index.row())->name, value.toString());

    const Pair pair = owner->pairs.at(index.row());
    if (role == ValueRole)
        return this->changePair(owner, index.row(), pair.key, QJsonValue::fromVariant(value));

    return this->renameKey(owner->name, pair.key, value.toString());
}

Qt::ItemFlags NotesKVStore::flags(const QModelIndex &index) const
{
    Qt::ItemFlags ret = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    const bool isRoot = index.isValid() && index.internalPointer() == nullptr && index.row() == 0;
    return isRoot ? ret : ret | Qt::ItemIsEditable;
}

QHash<int, QByteArray> NotesKVStore::roleNames() const
{
    return { { KeyRole, QByteArrayLiteral("key") },
             { ValueRole, QByteArrayLiteral("value") },
             { IsCategoryRole, QByteArrayLiteral("isCategory") },
             { PairCountRole, QByteArrayLiteral("pairCount") } };
}

QModelIndex NotesKVStore::categoryIndex(Category *category) const
{
    return this->createIndex(m_categories.indexOf(category), 0, nullptr);
}

NotesKVStore::Category *NotesKVStore::findCategory(const QString &name) const
{
    const QString trimmedName = name.trimmed();
    for (Category *category : m_categories) {
        if (category->name.compare(trimmedName, Qt::CaseInsensitive) == 0)
            return category;
    }
    return nullptr;
}

int NotesKVStore::findPair(const Category *category, const QString &key) const
{
    const QString trimmedKey = key.trimmed();
    for (int row = 0; row < category->pairs.size(); row++) {
        if (category->pairs.at(row).key.compare(trimmedKey, Qt::CaseInsensitive) == 0)
            return row;
    }
    return -1;
}

QString NotesKVStore::uniqueKey(const Category *category, const QString &key) const
{
    QString ret = key;
    for (int n = 2; this->findPair(category, ret) >= 0; n++)
        ret = key + QStringLiteral("_") + QString::number(n);
    return ret;
}

NotesKVStore::Category *NotesKVStore::insertCategory(const QString &name)
{
    if (name.isEmpty() || this->findCategory(name) != nullptr)
        return nullptr;

    Category *category = new Category;
    category->name = name;

    this->beginInsertRows(QModelIndex(), m_categories.size(), m_categories.size());
    m_categories.append(category);
    this->endInsertRows();
    return category;
}

void NotesKVStore::insertPair(Category *category, const QString &key, const QJsonValue &value)
{
    Pair pair;
    pair.key = key;
    pair.value = value;

    const int row = category->pairs.size();
    this->beginInsertRows(this->categoryIndex(category), row, row);
    category->pairs.append(pair);
    this->endInsertRows();

    const QModelIndex index = this->categoryIndex(category);
    emit dataChanged(index, index, { PairCountRole });
}

bool NotesKVStore::changePair(Category *category, int row, const QString &key,
                              const QJsonValue &value)
{
    Pair &pair = category->pairs[row];
    if (pair.key == key && pair.value == value)
        return false;

    pair.key = key;
    pair.value = value;

    const QModelIndex index = this->index(row, 0, this->categoryIndex(category));
    emit dataChanged(index, index, { Qt::DisplayRole, KeyRole, ValueRole });
    return true;
}
