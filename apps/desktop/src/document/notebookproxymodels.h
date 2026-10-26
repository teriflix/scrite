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

#ifndef NOTEBOOKPROXYMODELS_H
#define NOTEBOOKPROXYMODELS_H

#include <QTimer>
#include <QPointer>
#include <QQmlEngine>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>

#include "notebookmodel.h"

/**
 * Two ways of looking at a NotebookModel, for whoever lets a user choose things in the document
 * to refer to or to attach: the notes, scenes, characters, episodes and acts in it. They are
 * lenses: neither changes the NotebookModel, or keeps a copy of the document.
 *
 * What can be chosen are items of four kinds: "note", "scene", "character" and "break" (an
 * episode or an act). Everything else in the NotebookModel is only there to organize them:
 * categories like Screenplay and Characters, the containers like Story Notes and Act Notes that
 * notes are in, and the episodes and acts that the NotebookModel makes up for screenplays that
 * have none. Both lenses give each item that can be chosen:
 *
 *   itemKey    : kind and reference, as in scene:{id}. It is the same whenever the item is
 *                looked at again, which row numbers and the ids of the NotebookModel are not,
 *                because it rebuilds whole branches of itself when the document changes. A
 *                break is referred to by its name, as the tools of LLMHub do: the title, with
 *                the title of the episode in front of it for an act that is in one.
 *   itemKind   : note, scene, character or break.
 *   itemLabel  : what to call the item, such as "Scene 12: INT. KITCHEN - NIGHT", "Scene 14 (no
 *                heading): Rahul stirs a pot", "Character RAHUL" or, for a note, its owner and its
 *                title, as in "ACT 1 / AI Act Summary" and "RAHUL / Styling Notes", because the
 *                titles of notes are the same in many places. Scenes that have no heading, and
 *                those that are not in the screenplay, are told apart.
 *   itemOwner  : what the item belongs to: the scene with its heading, the character, the act (with
 *                its episode in front of it, if it is in one) or the episode, for a note, which
 *                has none if it is a note of the story; the episode of an act; otherwise empty.
 *   itemObject : the Note, Scene, Character or ScreenplayElement. (NotebookFlatLens only; the
 *                NotebookModel has it as notebookItemObject.)
 *
 * Searching is by words: an item is found if each word of the search is in its kind, label or
 * owner, ignoring case. So "char ra" finds characters with "ra" in their names.
 */
struct NotebookItemInfo
{
    QString kind;
    QString key;
    QString label;
    QString owner;
    QString text; // kind, label and owner in lower case, which is what a search looks in
    QPointer<QObject> object;

    bool isValid() const { return !kind.isEmpty(); }
};

/**
 * The items of a NotebookModel as a flat list, in the order of the Notebook, with no categories
 * and no containers. It is for lists where the user picks one item, such as a popup that
 * completes what is typed after an @.
 *
 * kinds limits the list to some kinds, such as ["note"]. All of them are in it if it is empty.
 * searchText narrows the list to the items that it finds. The list is made again, a moment after
 * the NotebookModel changes, and is reset when it is, so a view of it starts at its top.
 */
class NotebookFlatLens : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit NotebookFlatLens(QObject *parent = nullptr);
    ~NotebookFlatLens() override;

    // clang-format off
    Q_PROPERTY(NotebookModel *sourceModel
               READ sourceModel
               WRITE setSourceModel
               NOTIFY sourceModelChanged)
    // clang-format on
    void setSourceModel(NotebookModel *val);
    NotebookModel *sourceModel() const { return m_sourceModel; }
    Q_SIGNAL void sourceModelChanged();

    // clang-format off
    Q_PROPERTY(QStringList kinds
               READ kinds
               WRITE setKinds
               NOTIFY kindsChanged)
    // clang-format on
    void setKinds(const QStringList &val);
    QStringList kinds() const { return m_kinds; }
    Q_SIGNAL void kindsChanged();

    // clang-format off
    Q_PROPERTY(QString searchText
               READ searchText
               WRITE setSearchText
               NOTIFY searchTextChanged)
    // clang-format on
    void setSearchText(const QString &val);
    QString searchText() const { return m_searchText; }
    Q_SIGNAL void searchTextChanged();

    // The number of items in the list.
    // clang-format off
    Q_PROPERTY(int count
               READ count
               NOTIFY countChanged)
    // clang-format on
    int count() const { return int(m_rows.size()); }
    Q_SIGNAL void countChanged();

    // The labels of the items that the search finds, in the order of the list, for code that wants
    // strings, such as a completion model. They are not the same as itemLabel where two items
    // have the same one: a number is added to the second, as in "New Note (2)", so that a label
    // says which item it is. Changes when the list does.
    // clang-format off
    Q_PROPERTY(QStringList labels
               READ labels
               NOTIFY labelsChanged)
    // clang-format on
    QStringList labels() const { return m_labels; }
    Q_SIGNAL void labelsChanged();

    // The key of the item that one of labels is the label of, or an empty string if it is none.
    Q_INVOKABLE QString keyOf(const QString &label) const;

    // The label of the item that has this key, or an empty string if there is none, which is
    // so for an item that is gone. It looks at all the items of the kinds, whatever searchText is.
    Q_INVOKABLE QString labelOf(const QString &key) const;

    // QAbstractItemModel interface
    enum Roles {
        ItemKeyRole = Qt::UserRole + 1,
        ItemKindRole,
        ItemLabelRole,
        ItemOwnerRole,
        ItemObjectRole
    };
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    void rebuild();
    void refilter();
    void collect(const QModelIndex &parent);

private:
    QTimer m_rebuildTimer;
    QString m_searchText;
    QStringList m_kinds;
    QStringList m_labels; // of the rows, see labels()
    QList<int> m_rows; // of m_items, for the items that the search finds
    QList<NotebookItemInfo> m_items;
    QPointer<NotebookModel> m_sourceModel;
};

/**
 * The tree of a NotebookModel, as the Notebook shows it, for a user who ticks the items to attach:
 * the same branches, without Bookmarks, which has nothing under it. The tree lens keeps the
 * ticks, and not the NotebookModel or the view, so that they stay as the NotebookModel rebuilds
 * its branches, and as a view is closed and opened again.
 *
 * Only items can be ticked (itemCheckable); a row that is not one is there to be expanded.
 * Ticking an item ticks nothing else, and unticking it unticks nothing else. A tick is set with
 * setData() on itemChecked, or with checkedKeys, which is also how the ticks are kept for when
 * the document is opened again. A key that no item has any more is kept in checkedKeys, but is
 * not in checkedItems.
 *
 * searchText narrows the tree to the items that it finds, and the rows that they are in.
 */
class NotebookTreeLens : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit NotebookTreeLens(QObject *parent = nullptr);
    ~NotebookTreeLens() override;

    // clang-format off
    Q_PROPERTY(QString searchText
               READ searchText
               WRITE setSearchText
               NOTIFY searchTextChanged)
    // clang-format on
    void setSearchText(const QString &val);
    QString searchText() const { return m_searchText; }
    Q_SIGNAL void searchTextChanged();

    // The keys of the items that are ticked, in the order in which they were.
    // clang-format off
    Q_PROPERTY(QStringList checkedKeys
               READ checkedKeys
               WRITE setCheckedKeys
               NOTIFY checkedKeysChanged)
    // clang-format on
    void setCheckedKeys(const QStringList &val);
    QStringList checkedKeys() const { return m_checkedKeys; }
    Q_SIGNAL void checkedKeysChanged();

    // clang-format off
    Q_PROPERTY(int checkedCount
               READ checkedCount
               NOTIFY checkedKeysChanged)
    // clang-format on
    int checkedCount() const { return int(m_checkedKeys.size()); }

    // The items that are ticked and are in the document, in the order of the Notebook, each as
    // { key, kind, label, owner }. It is for showing what is ticked. Changes when the ticks do,
    // and a moment after the NotebookModel does, because the label of an item may.
    // clang-format off
    Q_PROPERTY(QVariantList checkedItems
               READ checkedItems
               NOTIFY checkedItemsChanged)
    // clang-format on
    QVariantList checkedItems() const;
    Q_SIGNAL void checkedItemsChanged();

    Q_INVOKABLE void clearChecked();

    // Ticks, or unticks, the item in the row at index, which does nothing for a row that is not an
    // item. It is what a view calls, which can't name the role that setData() takes.
    Q_INVOKABLE void setItemChecked(const QModelIndex &index, bool checked);

    // QAbstractItemModel interface
    enum Roles {
        ItemKeyRole = Qt::UserRole + 100,
        ItemKindRole,
        ItemLabelRole,
        ItemOwnerRole,
        ItemCheckableRole,
        ItemCheckedRole
    };
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    // QAbstractProxyModel interface
    void setSourceModel(QAbstractItemModel *model) override;

protected:
    // QSortFilterProxyModel interface
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    void collectChecked(const QModelIndex &parent, QVariantList &items) const;
    void announceCheckedChanged(const QModelIndex &parent = QModelIndex());
    void setChecked(const QString &key, bool checked);

private:
    QTimer m_itemsTimer;
    QString m_searchText;
    QStringList m_searchWords;
    QStringList m_checkedKeys;
};

#endif // NOTEBOOKPROXYMODELS_H
