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

#ifndef NOTEBOOKTABMODEL_H
#define NOTEBOOKTABMODEL_H

#include <QAbstractListModel>

#include "scene.h"
#include "structure.h"
#include "qobjectproperty.h"

class NotebookTabModel : public QAbstractListModel
{
    Q_OBJECT

public:
    NotebookTabModel(QObject *parent=nullptr);
    ~NotebookTabModel();

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    int count() const { return m_items.size(); }
    Q_SIGNAL void countChanged();

    Q_PROPERTY(Structure* structure READ structure WRITE setStructure NOTIFY structureChanged RESET resetStructure)
    void setStructure(Structure* val);
    Structure* structure() const { return m_structure; }
    Q_SIGNAL void structureChanged();

    Q_PROPERTY(Scene* activeScene READ activeScene WRITE setActiveScene NOTIFY activeSceneChanged RESET resetActiveScene)
    void setActiveScene(Scene* val);
    Scene* activeScene() const { return m_activeScene; }
    Q_SIGNAL void activeSceneChanged();

    Q_INVOKABLE QObject *tabSourceAt(int row) const;
    Q_INVOKABLE QString tabLabelAt(int row) const;
    Q_INVOKABLE QColor tabColorAt(int row) const;
    Q_INVOKABLE QString tabGroupAt(int row) const;
    Q_INVOKABLE QVariantMap tabDataAt(int row) const;
    Q_INVOKABLE int indexOfTabSource(QObject *source) const;
    Q_INVOKABLE int indexOfTabLabel(const QString &name) const;

    Q_SIGNAL void refreshed();

    // QAbstractItemModel interface
    enum Roles { TabSourceRole = Qt::UserRole+1, TabLabelRole, TabColorRole, TabGroupRole, TabModelDataRole };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

private:
    void resetStructure();
    void resetActiveScene();
    void updateModel();
    void resetModel();
    void updateActiveScene();

    static QString sceneLabel(const Scene *scene);

private:
    QObjectProperty<Scene> m_activeScene;
    QObjectProperty<Structure> m_structure;

    struct Item
    {
        Item() { }
        Item(Structure *structure) : // color is hard-coded, but it should be accentColors.c300.background
            source(structure), label(QStringLiteral("Story")), color(QStringLiteral("#90A4AE")) {
            group = label;
        }
        Item(Scene *scene) :
            source(scene), group(QStringLiteral("Current Scene")), label(NotebookTabModel::sceneLabel(scene)), color(scene->color()) { }
        Item(Character *character, const QColor &_color=Qt::blue) :
            source(character), group(QStringLiteral("Character")), label(character->name()), color(_color) { }

        QPointer<QObject> source;
        QString group;
        QString label;
        QColor color;

        bool isValid() const { return !source.isNull(); }

        bool operator == (const Item &other) const {
            return this->source == other.source;
        }

        Item & operator = (const Item &other) {
            this->source = other.source;
            this->label = other.label;
            this->color = other.color;
            return *this;
        }
    };
    QList<Item> m_items;
    QList<Item> gatherItems(bool charactersOnly=false) const;
};

#endif // NOTEBOOKTABMODEL_H
