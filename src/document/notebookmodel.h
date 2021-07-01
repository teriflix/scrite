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

#ifndef NOTEBOOKMODEL_H
#define NOTEBOOKMODEL_H

#include <QTimer>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

#include "qobjectproperty.h"

class Note;
class Notes;
class Scene;
class Character;
class ScriteDocument;
class ObjectListPropertyModelBase;

class NotebookModel : public QStandardItemModel
{
    Q_OBJECT

public:
    NotebookModel(QObject *parent=nullptr);
    ~NotebookModel();

    Q_PROPERTY(ScriteDocument* document READ document WRITE setDocument RESET resetDocument NOTIFY documentChanged)
    void setDocument(ScriteDocument* val);
    ScriteDocument* document() const { return m_document; }
    Q_SIGNAL void documentChanged();

    enum ItemType
    {
        CategoryType, // Just heading text, no "live" object
        EpisodeBreakType,
        ActBreakType,
        NotesType,   // Represents a Notes instance
        NoteType     // Represents a Note instance
    };
    Q_ENUM(ItemType)

    enum ItemCategory
    {
        ScreenplayCategory,
        UnusedScenesCategory,
        CharactersCategory,
        LocationsCategory,
        PropsCategory,
        OtherCategory
    };
    Q_ENUM(ItemCategory)

    enum ItemRoles
    {
        TitleRole = Qt::DisplayRole,
        IdRole = Qt::UserRole,
        TypeRole,
        CategoryRole,
        ObjectRole,
        ModelDataRole
    };

    Q_INVOKABLE QVariant modelIndexData(const QModelIndex &index) const {
        return this->data(index, ModelDataRole);
    }

    Q_INVOKABLE QModelIndex findModelIndexFor(QObject *owner) const;
    Q_INVOKABLE QModelIndex findModelIndexForTopLevelItem(const QString &label) const;

    // QAbstractItemModel interface
    QHash<int, QByteArray> roleNames() const;
    QVariant data(const QModelIndex &index, int role) const;
    static QHash<int, QByteArray> staticRoleNames();

signals:
    void aboutToReloadScenes();
    void justReloadedScenes();
    void aboutToReloadCharacters();
    void justReloadedCharacters();

private:
    void resetDocument();
    void reload();

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
    QObjectProperty<ScriteDocument> m_document;
    QTimer m_syncScenesTimer;
    QTimer m_syncCharactersTimer;
};

#endif // NOTEBOOKMODEL_H
