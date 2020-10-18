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

    Q_INVOKABLE QObject *sourceAt(int row) const;
    Q_INVOKABLE QString labelAt(int row) const;
    Q_INVOKABLE QColor colorAt(int row) const;
    Q_INVOKABLE QVariantMap at(int row) const;
    Q_INVOKABLE int indexOfSource(QObject *source) const;
    Q_INVOKABLE int indexOfLabel(const QString &name) const;

    Q_SIGNAL void refreshed();

    // QAbstractItemModel interface
    enum Roles { SourceRole = Qt::UserRole+1, LabelRole, ColorRole, ModelDataRole };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

private:
    void resetStructure();
    void resetActiveScene();
    void updateModel();
    void resetModel();

private:
    QObjectProperty<Scene> m_activeScene;
    QObjectProperty<Structure> m_structure;

    struct Item
    {
        Item() { }
        Item(Structure *structure) :
            source(structure), label(QStringLiteral("Story")), color(QStringLiteral("purple")) { }
        Item(Scene *scene) :
            source(scene), label(scene->title()), color(scene->color()) { }
        Item(Character *character, const QColor &_color=Qt::blue) :
            source(character), label(character->name()), color(_color) { }

        QPointer<QObject> source;
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
