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

#ifndef NOTEBOOKMODEL_H
#define NOTEBOOKMODEL_H

#include <QTimer>
#include <QQmlEngine>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

#include "qobjectproperty.h"
#include "qobjectlistmodel.h"

class Note;
class Notes;
class Scene;
class Character;
class ScriteDocument;
class BookmarkedNotes;
class AbstractQObjectListModel;

class NotebookModel : public QStandardItemModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit NotebookModel(QObject *parent = nullptr);
    ~NotebookModel();

    Q_PROPERTY(ScriteDocument *document READ document WRITE setDocument RESET resetDocument NOTIFY documentChanged)
    void setDocument(ScriteDocument *val);
    ScriteDocument *document() const { return m_document; }
    Q_SIGNAL void documentChanged();

    enum ItemType {
        CategoryType, // Just heading text, no "live" object
        EpisodeBreakType,
        ActBreakType,
        NotesType, // Represents a Notes instance
        NoteType // Represents a Note instance
    };
    Q_ENUM(ItemType)

    enum ItemCategory {
        BookmarksCategory,
        ScreenplayCategory,
        UnusedScenesCategory,
        CharactersCategory,
        LocationsCategory,
        PropsCategory,
        OtherCategory
    };
    Q_ENUM(ItemCategory)

    enum ItemRoles {
        TitleRole = Qt::DisplayRole,
        IdRole = Qt::UserRole,
        TypeRole,
        CategoryRole,
        ObjectRole,
        ModelDataRole
    };

    Q_INVOKABLE QVariant modelIndexData(const QModelIndex &index) const;
    Q_INVOKABLE QModelIndex findModelIndexFor(QObject *owner) const;
    Q_INVOKABLE QModelIndex findModelIndexForTopLevelItem(const QString &label) const;
    Q_INVOKABLE QModelIndex findModelIndexForCategory(NotebookModel::ItemCategory cat) const;
    Q_INVOKABLE void refresh();

    Q_PROPERTY(BookmarkedNotes *bookmarkedNotes READ bookmarkedNotes CONSTANT STORED false)
    BookmarkedNotes *bookmarkedNotes() const { return m_bookmarkedNotes; }
    Q_SIGNAL void bookmarkedNotesChanged();

    // QAbstractItemModel interface
    QHash<int, QByteArray> roleNames() const;
    QVariant data(const QModelIndex &index, int role) const;
    static QHash<int, QByteArray> staticRoleNames();

signals:
    void justRefreshed();
    void aboutToRefresh();
    void aboutToReloadScenes();
    void justReloadedScenes();
    void aboutToReloadCharacters();
    void justReloadedCharacters();

private:
    void resetDocument();
    void reload();

    void loadBookmarks();
    void loadStory();
    void loadScenes();
    void loadCharacters();
    void loadLocations();
    void loadProps();
    void loadOthers();

    void syncScenes();
    void syncCharacters();

    void onDataChanged(const QModelIndex &start, const QModelIndex &end, const QVector<int> &roles);

private:
    QTimer m_syncScenesTimer;
    QTimer m_syncCharactersTimer;
    QObjectProperty<ScriteDocument> m_document;
    BookmarkedNotes *m_bookmarkedNotes = nullptr;
};

class BookmarkedNotes : public QObjectListModel<QObject *>
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    explicit BookmarkedNotes(QObject *parent = nullptr);
    ~BookmarkedNotes();

    Q_INVOKABLE bool toggleBookmark(QObject *object);
    Q_INVOKABLE bool addToBookmark(QObject *object);
    Q_INVOKABLE bool removeFromBookmark(QObject *object);
    Q_INVOKABLE bool isBookmarked(QObject *object) const;

    enum Roles { TitleRole, SummaryRole, ObjectRole };
    QHash<int, QByteArray> roleNames() const;
    QVariant data(const QModelIndex &index, int role) const;

private:
    QVariant data(QObject *ptr, int role) const;

    void noteUpdated();
    void notesUpdated();
    void characterUpdated();

    void objectUpdated(QObject *ptr);

    void noteDestroyed(Note *ptr);
    void notesDestroyed(Notes *ptr);
    void characterDestroyed(Character *ptr);

    void sync();
    void reload();

private:
    bool m_blockReload = false;
};

#endif // NOTEBOOKMODEL_H
