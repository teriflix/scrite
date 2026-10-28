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

/**
 * What the lenses can be limited to, as flags. Kinds are the kinds of items: NoteKind, SceneKind,
 * CharacterKind and BreakKind. NoteTypes are the types of the notes: PlainTextNote,
 * MarkdownTextNote and RichTextNote are three distinct types of text notes, and TextNote is all
 * three of them.
 */
class NotebookItemFilter
{
    Q_GADGET
    QML_ELEMENT
    QML_UNCREATABLE("Only its enums are to be used.")

public:
    enum Kind {
        NoteKind = 1,
        SceneKind = 2,
        CharacterKind = 4,
        BreakKind = 8,
        AnyKind = NoteKind | SceneKind | CharacterKind | BreakKind
    };
    Q_DECLARE_FLAGS(Kinds, Kind)
    Q_FLAG(Kinds)

    enum NoteType {
        PlainTextNote = 1,
        MarkdownTextNote = 2,
        RichTextNote = 4,
        ChecklistNote = 8,
        FormNote = 16,
        TextNote = PlainTextNote | MarkdownTextNote | RichTextNote,
        AnyNote = TextNote | ChecklistNote | FormNote
    };
    Q_DECLARE_FLAGS(NoteTypes, NoteType)
    Q_FLAG(NoteTypes)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(NotebookItemFilter::Kinds)
Q_DECLARE_OPERATORS_FOR_FLAGS(NotebookItemFilter::NoteTypes)

struct NotebookItemInfo
{
    NotebookItemFilter::NoteTypes noteType; // of a note, which is the only kind that has one
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
 * kinds limits the list to some kinds, such as NoteKind | SceneKind. All of them are in it by
 * default.
 * noteTypes limits the notes in it to some types of notes, such as TextNote or ChecklistNote |
 * FormNote, and does nothing to items that are not notes. searchText narrows the list to the items that it finds. The list is made again, a moment after
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
    Q_PROPERTY(NotebookItemFilter::Kinds kinds
               READ kinds
               WRITE setKinds
               NOTIFY kindsChanged)
    // clang-format on
    void setKinds(NotebookItemFilter::Kinds val);
    NotebookItemFilter::Kinds kinds() const { return m_kinds; }
    Q_SIGNAL void kindsChanged();

    // clang-format off
    Q_PROPERTY(NotebookItemFilter::NoteTypes noteTypes
               READ noteTypes
               WRITE setNoteTypes
               NOTIFY noteTypesChanged)
    // clang-format on
    void setNoteTypes(NotebookItemFilter::NoteTypes val);
    NotebookItemFilter::NoteTypes noteTypes() const { return m_noteTypes; }
    Q_SIGNAL void noteTypesChanged();

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
    NotebookItemFilter::NoteTypes m_noteTypes = NotebookItemFilter::AnyNote;
    NotebookItemFilter::Kinds m_kinds = NotebookItemFilter::AnyKind;
    QString m_searchText;
    QStringList m_labels; // of the rows, see labels()
    QList<int> m_rows; // of m_items, for the items that the search finds
    QList<NotebookItemInfo> m_items;
    QPointer<NotebookModel> m_sourceModel;
};

/**
 * Some items of the Notebook of the document, picked one by one by the user, in a list that can be
 * reordered, for whoever uses them in the order that the user wants. It keeps a NotebookModel of
 * its own, which is also what the lens that the user picks from can look at.
 *
 * Each row is the object of an item: the Note, the Notes of a scene or a character, or the
 * ScreenplayElement of an episode or an act, and has the itemKey, itemKind, itemLabel and itemOwner
 * of the item. An item is added with its key, as in NotebookFlatLens.keyOf(). An object that is
 * deleted is removed from the list, and so is an item that is not in the Notebook any more, a
 * moment after the NotebookModel changes, which is also when the label of an item is looked at again.
 */
class NotebookCheckedItems : public QObjectListModel<QObject *>
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit NotebookCheckedItems(QObject *parent = nullptr);
    ~NotebookCheckedItems() override;

    // clang-format off
    Q_PROPERTY(NotebookModel *notebook
               READ notebook
               CONSTANT)
    // clang-format on
    NotebookModel *notebook() const { return m_notebook; }

    // Adds the object of the item that has this key to the end of the list. It returns false if
    // there is no such item, or it is already in the list.
    Q_INVOKABLE bool addKey(const QString &key);

    // Removes the object in the row, and moves the object in row from to row to, which is what
    // removeAt() and move() do, for QML.
    Q_INVOKABLE void removeObject(int row) { this->removeAt(row); }
    Q_INVOKABLE void moveObject(int from, int to) { this->move(from, to); }

    // The key of the item whose object is in the row, or an empty string if there is no such row.
    QString keyAt(int row) const { return m_infos.value(this->at(row)).key; }

    int indexOfKey(const QString &key) const;

    // QAbstractItemModel interface
    enum Roles {
        ItemKeyRole = ObjectKindRole + 1,
        ItemKindRole,
        ItemLabelRole,
        ItemOwnerRole
    };
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;

protected:
    // QObjectListModel interface
    void itemInsertEvent(QObject *ptr) override
    {
        connect(ptr, &QObject::destroyed, this, &NotebookCheckedItems::objectDestroyed);
    }
    void itemRemoveEvent(QObject *ptr) override
    {
        disconnect(ptr, &QObject::destroyed, this, &NotebookCheckedItems::objectDestroyed);
        m_infos.remove(ptr);
    }

private:
    void add(const NotebookItemInfo &info);
    void refresh();
    NotebookItemInfo findInfo(const QString &key, const QModelIndex &parent = QModelIndex()) const;
    void collectInfos(const QModelIndex &parent, QHash<QString, NotebookItemInfo> &infos) const;

private:
    QTimer m_refreshTimer;
    NotebookModel *m_notebook = nullptr;
    QHash<QObject *, NotebookItemInfo> m_infos;
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

    // Limits what can be ticked to items of some kinds, such as NoteKind | SceneKind, and all of
    // them can be by default. The rows of the other items are still in the tree, but are not
    // itemCheckable. It is to be set before a view looks at the lens.
    // clang-format off
    Q_PROPERTY(NotebookItemFilter::Kinds kinds
               READ kinds
               WRITE setKinds
               NOTIFY kindsChanged)
    // clang-format on
    void setKinds(NotebookItemFilter::Kinds val);
    NotebookItemFilter::Kinds kinds() const { return m_kinds; }
    Q_SIGNAL void kindsChanged();

    // Limits the notes in the tree to some types of notes, such as TextNote. The other notes are
    // not in the tree at all, unlike the items that kinds leaves out. It does nothing to items
    // that are not notes.
    // clang-format off
    Q_PROPERTY(NotebookItemFilter::NoteTypes noteTypes
               READ noteTypes
               WRITE setNoteTypes
               NOTIFY noteTypesChanged)
    // clang-format on
    void setNoteTypes(NotebookItemFilter::NoteTypes val);
    NotebookItemFilter::NoteTypes noteTypes() const { return m_noteTypes; }
    Q_SIGNAL void noteTypesChanged();

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

    // The most items that can be ticked, or 0 if there is no limit. An item can't be ticked when
    // as many are, which untick one first. Keys that are set with checkedKeys are all kept, even if
    // there are more of them than this allows, which only means that no more can be added.
    // clang-format off
    Q_PROPERTY(int maxChecked
               READ maxChecked
               WRITE setMaxChecked
               NOTIFY maxCheckedChanged)
    // clang-format on
    void setMaxChecked(int val);
    int maxChecked() const { return m_maxChecked; }
    Q_SIGNAL void maxCheckedChanged();

    // True if no more items can be ticked, because maxChecked of them are.
    // clang-format off
    Q_PROPERTY(bool full
               READ isFull
               NOTIFY fullChanged)
    // clang-format on
    bool isFull() const { return m_maxChecked > 0 && m_checkedKeys.size() >= m_maxChecked; }
    Q_SIGNAL void fullChanged();

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
    bool isCheckable(const NotebookItemInfo &info) const;
    void collectChecked(const QModelIndex &parent, QVariantList &items) const;
    void announceCheckedChanged(const QModelIndex &parent = QModelIndex());
    void setChecked(const QString &key, bool checked);

private:
    int m_maxChecked = 0;
    QTimer m_itemsTimer;
    NotebookItemFilter::NoteTypes m_noteTypes = NotebookItemFilter::AnyNote;
    NotebookItemFilter::Kinds m_kinds = NotebookItemFilter::AnyKind;
    QString m_searchText;
    QStringList m_searchWords;
    QStringList m_checkedKeys;
};

#endif // NOTEBOOKPROXYMODELS_H
