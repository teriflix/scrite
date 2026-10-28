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

#ifndef NOTESKVSTORE_H
#define NOTESKVSTORE_H

#include <QJsonArray>
#include <QJsonValue>
#include <QQmlEngine>
#include <QAbstractItemModel>

/**
 * Categorised key-value pairs of a Notes object. A tree with two levels: the top level rows are
 * categories, and the children of a category are its pairs. The first category is always "Root",
 * which cannot be removed or renamed, although its pairs can all be removed. Names of categories
 * and keys of pairs are unique, ignoring case, so a key is set once in a category. Keys are text.
 * A value is any JSON value: text, a number, a boolean, a list or an object.
 */
class NotesKVStore : public QAbstractItemModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use Notes.kvStore instead.")

public:
    explicit NotesKVStore(QObject *parent = nullptr);
    ~NotesKVStore() override;

    // clang-format off
    Q_PROPERTY(QString rootCategoryName
               READ rootCategoryName
               CONSTANT)
    // clang-format on
    static QString rootCategoryName() { return QStringLiteral("Root"); }

    // True if there is nothing to save: just Root, without pairs.
    // clang-format off
    Q_PROPERTY(bool empty
               READ isEmpty
               NOTIFY storeModified)
    // clang-format on
    bool isEmpty() const;

    Q_SIGNAL void storeModified();

    Q_INVOKABLE bool addCategory(const QString &name);
    Q_INVOKABLE bool removeCategory(const QString &name);
    Q_INVOKABLE bool renameCategory(const QString &name, const QString &newName);
    Q_INVOKABLE bool renameKey(const QString &category, const QString &key, const QString &newKey);
    Q_INVOKABLE bool setValue(const QString &category, const QString &key, const QJsonValue &value,
                              bool createCategory = false);
    Q_INVOKABLE bool removeValue(const QString &category, const QString &key);
    Q_INVOKABLE bool hasKey(const QString &category, const QString &key) const;
    Q_INVOKABLE QJsonValue value(const QString &category, const QString &key) const;
    Q_INVOKABLE QStringList categoryNames() const;
    Q_INVOKABLE void clear();

    // Adds the categories and pairs of other. If a key has different values in the same category
    // of both, both are kept, as key_original (this one's) and key_merged (other's).
    void merge(const NotesKVStore *other);

    // Replaces the name in the values that are text.
    void renameCharacter(const QString &from, const QString &to);

    // The categories as [ { "category": "Root", "pairs": [ { "key": "k", "value": "v" } ] } ]
    Q_INVOKABLE QJsonArray toJson() const;
    void fromJson(const QJsonArray &array);

    // PairCountRole is the number of pairs that a category has, and is 0 for a pair.
    enum Roles { KeyRole = Qt::UserRole, ValueRole, IsCategoryRole, PairCountRole };

    // QAbstractItemModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QHash<int, QByteArray> roleNames() const override;

private:
    struct Pair;
    struct Category;

    int findPair(const Category *category, const QString &key) const;
    void insertPair(Category *category, const QString &key, const QJsonValue &value);
    bool changePair(Category *category, int row, const QString &key, const QJsonValue &value);

    Category *insertCategory(const QString &name);
    Category *findCategory(const QString &name) const;
    QModelIndex categoryIndex(Category *category) const;

    QString uniqueKey(const Category *category, const QString &key) const;

private:
    QList<Category *> m_categories;
};

#endif // NOTESKVSTORE_H
