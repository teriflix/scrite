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

#include "notebookmodel.h"

#include "notes.h"
#include "scene.h"
#include "structure.h"
#include "application.h"
#include "timeprofiler.h"
#include "scritedocument.h"

class ObjectItem : public QStandardItem
{
public:
    ObjectItem(QObject *object);
    ~ObjectItem();

private:
    void objectDestroyed(QObject *ptr);

private:
    QObject *m_object = nullptr;
    QMetaObject::Connection m_destroyedConnection;
};

class NotesItem : public ObjectItem
{
public:
    NotesItem(Notes *notes);
    ~NotesItem();

    void sync();
    void updateText();

private:
    Notes *m_notes = nullptr;
    QTimer m_syncTimer;
};

NotebookModel::NotebookModel(QObject *parent)
    : QStandardItemModel(parent),
      m_document(this, "document")
{
    m_syncScenesTimer.setSingleShot(true);
    m_syncCharactersTimer.setSingleShot(true);

    m_syncScenesTimer.setInterval(0);
    m_syncCharactersTimer.setInterval(0);

    connect(&m_syncScenesTimer, &QTimer::timeout, this, &NotebookModel::syncScenes);
    connect(&m_syncCharactersTimer, &QTimer::timeout, this, &NotebookModel::syncCharacters);
}

NotebookModel::~NotebookModel()
{

}

void NotebookModel::setDocument(ScriteDocument *val)
{
    if(m_document == val)
        return;

    if(m_document != nullptr)
    {
        disconnect(m_document, &ScriteDocument::justReset, this, &NotebookModel::reload);
        disconnect(m_document, &ScriteDocument::justLoaded, this, &NotebookModel::reload);
    }

    m_document = val;
    emit documentChanged();

    if(m_document != nullptr)
    {
        connect(m_document, &ScriteDocument::justReset, this, &NotebookModel::reload);
        connect(m_document, &ScriteDocument::justLoaded, this, &NotebookModel::reload);
    }

    this->reload();
}

QStandardItem *recursivelyFindItemForOnwer(QStandardItem *root, QObject *owner)
{
    if(root == nullptr || owner == nullptr)
        return nullptr;

    if(root->data(NotebookModel::ObjectRole).value<QObject*>() == owner)
        return root;

    const int nrRows = root->rowCount();
    for(int i=0; i<nrRows; i++)
    {
        QStandardItem *row = root->child(i, 0);
        QStandardItem *found = recursivelyFindItemForOnwer(row, owner);
        if(found != nullptr)
            return found;
    }

    return nullptr;
}

QModelIndex NotebookModel::findModelIndexFor(QObject *owner) const
{
    QStandardItem *item = ::recursivelyFindItemForOnwer(this->invisibleRootItem(), owner);
    if(item == nullptr)
        return QModelIndex();

    return this->indexFromItem(item);
}

QHash<int, QByteArray> NotebookModel::roleNames() const
{
    return staticRoleNames();
}

QVariant NotebookModel::data(const QModelIndex &index, int role) const
{
    if(role == ModelDataRole)
    {
        QHash<int, QByteArray> roles = staticRoleNames();
        roles.remove(ModelDataRole);

        QVariantMap ret;

        auto it = roles.begin();
        auto end = roles.end();
        while(it != end)
        {
            ret[ QString::fromLatin1(it.value()) ] = QStandardItemModel::data(index, it.key());
            ++it;
        }

        return ret;
    }

    return QStandardItemModel::data(index, role);
}

QHash<int, QByteArray> NotebookModel::staticRoleNames()
{
    static QHash<int,QByteArray> roles = {
        { TitleRole, QByteArrayLiteral("notebookItemTitle") },
        { TypeRole, QByteArrayLiteral("notebookItemType") },
        { CategoryRole, QByteArrayLiteral("notebookItemCategory") },
        { ObjectRole, QByteArrayLiteral("notebookItemObject") },
        { ModelDataRole, QByteArrayLiteral("notebookItemData") }
    };

    return roles;
}

void NotebookModel::resetDocument()
{
    m_document = nullptr;
    this->reload();
}

void NotebookModel::reload()
{
    this->clear();
    m_syncScenesTimer.stop();
    m_syncCharactersTimer.stop();

    if(m_document == nullptr)
        return;

    this->loadStory();
    this->loadScenes();
    this->loadCharacters();
    this->loadLocations();
    this->loadProps();
    this->loadOthers();
}

void NotebookModel::loadStory()
{
    Structure *structure = m_document->structure();
    Notes *storyNotes = structure->notes();

    NotesItem *storyNotesItem = new NotesItem(storyNotes);
    this->appendRow(storyNotesItem);
}

void NotebookModel::loadScenes()
{
    QStandardItem *scenesItem = new QStandardItem;
    scenesItem->setText( QStringLiteral("Scenes") );
    scenesItem->setData(CategoryType, TypeRole);
    scenesItem->setData(ScenesCategory, CategoryRole);

    Structure *structure = m_document->structure();
    ObjectListPropertyModel<StructureElement *> *elements = structure->elementsModel();

    const int nrElements = elements->objectCount();
    for(int i=0; i<nrElements; i++)
    {
        StructureElement *element = elements->at(i);
        Scene *scene = element->scene();
        Notes *sceneNotes = scene->notes();

        NotesItem *sceneNotesItem = new NotesItem(sceneNotes);
        scenesItem->appendRow(sceneNotesItem);
    }

    this->appendRow(scenesItem);

    connect(structure, &Structure::elementCountChanged, &m_syncScenesTimer, QOverload<>::of(&QTimer::start), Qt::UniqueConnection);
}

void NotebookModel::loadCharacters()
{
    QStandardItem *charactersItem = new QStandardItem;
    charactersItem->setText( QStringLiteral("Characters") );
    charactersItem->setData(CategoryType, TypeRole);
    charactersItem->setData(CharactersCategory, CategoryRole);

    Structure *structure = m_document->structure();
    ObjectListPropertyModel<Character *> *characters = structure->charactersModel();

    const int nrCharacters = characters->objectCount();
    for(int i=0; i<nrCharacters; i++)
    {
        Character *character = characters->at(i);
        Notes *characterNotes = character->notes();

        NotesItem *notesItem = new NotesItem(characterNotes);
        charactersItem->appendRow(notesItem);
    }

    this->appendRow(charactersItem);

    connect(structure, &Structure::characterCountChanged, &m_syncCharactersTimer, QOverload<>::of(&QTimer::start), Qt::UniqueConnection);
}

void NotebookModel::loadLocations()
{
#if 0
    QStandardItem *locationsGroupItem = new QStandardItem;
    locationsGroupItem->setText( QStringLiteral("Locations") );
    locationsGroupItem->setData(CategoryType, TypeRole);
    locationsGroupItem->setData(LocationsCategory, CategoryRole);

    this->appendRow(locationsGroupItem);
#endif
}

void NotebookModel::loadProps()
{
#if 0
    QStandardItem *propsGroupItem = new QStandardItem;
    propsGroupItem->setText( QStringLiteral("Props") );
    propsGroupItem->setData(CategoryType, TypeRole);
    propsGroupItem->setData(LocationsCategory, CategoryRole);

    this->appendRow(propsGroupItem);
#endif
}

void NotebookModel::loadOthers()
{
#if 0
    QStandardItem *otherCategoryGroupItem = new QStandardItem;
    otherCategoryGroupItem->setText( QStringLiteral("Others") );
    otherCategoryGroupItem->setData(CategoryType, TypeRole);
    otherCategoryGroupItem->setData(LocationsCategory, CategoryRole);

    this->appendRow(otherCategoryGroupItem);
#endif
}

void NotebookModel::syncScenes()
{
    Structure *structure = m_document->structure();
    ObjectListPropertyModel<StructureElement *> *elements = structure->elementsModel();

    QList<QStandardItem *> plausibleItems = this->findItems(QStringLiteral("Scenes"), Qt::MatchExactly, 0);
    if(plausibleItems.size() != 1)
        return;

    QStandardItem *scenesItem = plausibleItems.first();
    if(scenesItem->rowCount() > elements->objectCount())
    {
        m_syncScenesTimer.start();
        return;
    }

    const int count = elements->objectCount();
    for(int i=0; i<count; i++)
    {
        StructureElement *element = elements->at(i);
        Scene *scene = element->scene();
        Notes *sceneNotes = scene->notes();

        QStandardItem *sceneNotesItem = scenesItem->child(i);
        Notes *assignedNotes = sceneNotesItem ?
                    qobject_cast<Notes*>(sceneNotesItem->data(NotebookModel::ObjectRole).value<QObject*>()) :
                    nullptr;

        if(sceneNotes != assignedNotes)
        {
            sceneNotesItem = new NotesItem(sceneNotes);
            this->insertRow(i, sceneNotesItem);
        }
    }
}

void NotebookModel::syncCharacters()
{
    Structure *structure = m_document->structure();
    ObjectListPropertyModel<Character *> *characters = structure->charactersModel();

    QList<QStandardItem *> plausibleItems = this->findItems(QStringLiteral("Characters"), Qt::MatchExactly, 0);
    if(plausibleItems.size() != 1)
        return;

    QStandardItem *charactersItem = plausibleItems.first();
    if(charactersItem->rowCount() > characters->objectCount())
    {
        m_syncCharactersTimer.start();
        return;
    }

    const int count = characters->objectCount();
    for(int i=0; i<count; i++)
    {
        Character *character = characters->at(i);
        Notes *characterNotes = character->notes();

        QStandardItem *characterNotesItem = charactersItem->child(i);
        Notes *assignedNotes = characterNotesItem ?
                    qobject_cast<Notes*>(characterNotesItem->data(NotebookModel::ObjectRole).value<QObject*>()) :
                    nullptr;

        if(characterNotes != assignedNotes)
        {
            characterNotesItem = new NotesItem(characterNotes);
            this->insertRow(i, characterNotesItem);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

ObjectItem::ObjectItem(QObject *object)
    : QStandardItem(), m_object(object)
{
    m_destroyedConnection = QObject::connect(m_object, &QObject::destroyed, m_object, [=](QObject *ptr) {
        this->objectDestroyed(ptr);
    });
    this->setData(QVariant::fromValue<QObject*>(m_object), NotebookModel::ObjectRole);
    this->setText(QStringLiteral("Note"));
}

ObjectItem::~ObjectItem()
{
    QObject::disconnect(m_destroyedConnection);
}

void ObjectItem::objectDestroyed(QObject *ptr)
{
    if(ptr == m_object)
    {
        QObject::disconnect(m_destroyedConnection);
        m_destroyedConnection = QMetaObject::Connection();

        QStandardItem *parentItem = this->parent();
        if(parentItem != nullptr)
        {
            const int row = this->row();
            parentItem->removeRow(row);
        }
        else
            delete this;
    }
}

NotesItem::NotesItem(Notes *notes)
          :ObjectItem(notes), m_notes(notes)
{
    this->updateText();
    switch(m_notes->ownerType())
    {
    case Notes::SceneOwner: {
            StructureElement *element = m_notes->scene()->structureElement();
            auto updateTextSlot = [=]() { this->updateText(); };
            if(element)
                QObject::connect(element, &StructureElement::titleChanged, m_notes, updateTextSlot);
            else
                QObject::connect(m_notes->scene(), &Scene::titleChanged, m_notes, updateTextSlot);
            QObject::connect(m_notes->scene(), &Scene::screenplayElementIndexListChanged, m_notes, updateTextSlot);
        } break;
    default:
        break;
    }

    this->setData(NotebookModel::NotesType, NotebookModel::TypeRole);

    const int nrNotes = m_notes->noteCount();

    QList<QStandardItem*> noteItems;
    noteItems.reserve(nrNotes);

    for(int i=0; i<nrNotes; i++)
    {
        Note *note = m_notes->noteAt(i);
        const QString noteTitle = note->title().isEmpty() ?
                    QStringLiteral("Note: ") + QString::number(i) :
                    note->title();

        ObjectItem *noteItem = new ObjectItem(note);
        noteItem->setText( noteTitle );
        noteItem->setData(NotebookModel::NoteType, NotebookModel::TypeRole);

        noteItems.append(noteItem);
    }

    this->appendRows(noteItems);

    // Why do we use a timer here? Why not directly call sync?
    // Because noteCountChanged() is emitted before objectDestroyed()
    m_syncTimer.setInterval(0);
    m_syncTimer.setSingleShot(true);
    QObject::connect(&m_syncTimer, &QTimer::timeout, m_notes, [=]() { this->sync(); });
    QObject::connect(m_notes, &Notes::noteCountChanged, &m_syncTimer, QOverload<>::of(&QTimer::start));

}

NotesItem::~NotesItem()
{
    m_syncTimer.stop();
}

void NotesItem::sync()
{
    if(this->rowCount() > m_notes->noteCount())
    {
        m_syncTimer.start();
        return;
    }

    const int noteCount = m_notes->noteCount();
    for(int i=0; i<noteCount; i++)
    {
        Note *actualNote = m_notes->at(i);
        QStandardItem *noteItem = this->child(i);
        Note *assignedNote = noteItem ?
                    qobject_cast<Note*>(noteItem->data(NotebookModel::ObjectRole).value<QObject*>()) :
                    nullptr;

        if(actualNote != assignedNote)
        {
            noteItem = new ObjectItem(actualNote);
            noteItem->setText( QStringLiteral("Note") );
            noteItem->setData(NotebookModel::NoteType, NotebookModel::TypeRole);
            this->insertRow(i, noteItem);
        }
    }
}

void NotesItem::updateText()
{
    switch(m_notes->ownerType())
    {
    case Notes::StructureOwner:
        this->setText(QStringLiteral("Story"));
        return;
    case Notes::CharacterOwner:
        this->setText(m_notes->character()->name());
        return;
    case Notes::RelationshipOwner:
        this->setText(m_notes->relationship()->name());
        return;
    case Notes::LocationOwner:
        this->setText(QStringLiteral("Location"));
        return;
    case Notes::PropOwner:
        this->setText(QStringLiteral("Prop"));
        return;
    case Notes::SceneOwner: {
        QList<int> indexes = m_notes->scene()->screenplayElementIndexList();
        QStringList idxStringList;
        Screenplay *screenplay = ScriteDocument::instance()->screenplay();
        for(int val : qAsConst(indexes)) {
            ScreenplayElement *element = screenplay->elementAt(val);
            if(element)
                idxStringList << element->resolvedSceneNumber();
            else
                idxStringList << QString::number(val+1);
        }
        StructureElement *element = m_notes->scene()->structureElement();
        QString title;
        if(element)
            title = element->title();
        else
            title = m_notes->scene()->title();
        if(!idxStringList.isEmpty())
            title = QStringLiteral("[") + idxStringList.join(QStringLiteral(",")) + QStringLiteral("]: ") + title;
        this->setText(title);
        } return;
    case Notes::OtherOwner:
        this->setText(QStringLiteral("Other"));
        return;
    }

    this->setText(QStringLiteral("Notes"));
}

///////////////////////////////////////////////////////////////////////////////

NotebookFilterModel::NotebookFilterModel(QObject *parent)
    :QSortFilterProxyModel(parent)
{
    this->setDynamicSortFilter(true);
    this->setSortRole(NotebookModel::ModelDataRole);
}

NotebookFilterModel::~NotebookFilterModel()
{

}

void NotebookFilterModel::setNotebookModel(NotebookModel *val)
{
    if(m_notebookModel == val)
        return;

    this->dropHookstoScreenplaySignals();

    m_notebookModel = val;
    this->setSourceModel(val);

    this->hookToScreenplaySignals();

    emit notebookModelChanged();
}

QModelIndex NotebookFilterModel::findModelIndexFor(QObject *owner) const
{
    if(m_notebookModel == nullptr)
        return QModelIndex();

    const QModelIndex source_index = m_notebookModel->findModelIndexFor(owner);
    if(source_index.isValid())
        return this->mapFromSource(source_index);

    return QModelIndex();
}

QHash<int, QByteArray> NotebookFilterModel::roleNames() const
{
    return NotebookModel::staticRoleNames();
}

bool NotebookFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    NotebookModel *nbm = qobject_cast<NotebookModel*>(this->sourceModel());
    if(nbm == nullptr || nbm != m_notebookModel)
        return true;

    QStandardItem *parentItem = nbm->itemFromIndex(source_parent);
    if(parentItem == nullptr)
        return true;

    QStandardItem *item = parentItem->child(source_row);
    if(item == nullptr)
        return true;

    if( item->data(NotebookModel::TypeRole).toInt() == NotebookModel::NotesType )
    {
        Notes *notes = qobject_cast<Notes*>(item->data(NotebookModel::ObjectRole).value<QObject*>());
        if(notes == nullptr || notes->ownerType() != Notes::SceneOwner)
            return true;

        return notes->noteCount() > 0;
    }

    return true;
}

bool NotebookFilterModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    NotebookModel *nbm = qobject_cast<NotebookModel*>(this->sourceModel());
    if(nbm == nullptr || nbm != m_notebookModel)
        return false;

    QStandardItem *leftItem = nbm->itemFromIndex(source_left);
    QStandardItem *rightItem = nbm->itemFromIndex(source_right);
    if(leftItem == nullptr || rightItem == nullptr || leftItem == rightItem ||
       leftItem->parent() != rightItem->parent() ||
       leftItem->parent() == nullptr || rightItem->parent() == nullptr)
        return false;

    Notes *leftNotes = qobject_cast<Notes*>( leftItem->data(NotebookModel::ObjectRole).value<QObject*>() );
    Notes *rightNotes = qobject_cast<Notes*>( rightItem->data(NotebookModel::ObjectRole).value<QObject*>() );
    if(leftNotes == nullptr || rightNotes == nullptr)
        return false;

    if(leftNotes->ownerType() == Notes::SceneOwner && rightNotes->ownerType() == Notes::SceneOwner)
    {
        Scene *leftScene = leftNotes->scene();
        Scene *rightScene = rightNotes->scene();
        if(leftScene == nullptr || rightScene == nullptr)
            return false;

        QList<int> leftIndexes = leftScene->screenplayElementIndexList();
        QList<int> rightIndexes = rightScene->screenplayElementIndexList();
        if(leftIndexes.isEmpty() && rightIndexes.isEmpty())
            return false;

        if(!leftIndexes.isEmpty() && rightIndexes.isEmpty())
            return true;

        if(leftIndexes.isEmpty() && !rightIndexes.isEmpty())
            return false;

        return leftIndexes.first() < rightIndexes.first();
    }

    if(leftNotes->ownerType() == Notes::CharacterOwner && rightNotes->ownerType() == Notes::CharacterOwner)
        return leftNotes->character()->name().compare( rightNotes->character()->name(), Qt::CaseInsensitive ) < 0;

    return false;
}

void NotebookFilterModel::hookToScreenplaySignals()
{
    if(m_notebookModel != nullptr)
    {
        ScriteDocument *document = m_notebookModel->document();
        if(document == nullptr)
        {
            connect(m_notebookModel, &NotebookModel::documentChanged, this, &NotebookFilterModel::hookToScreenplaySignals, Qt::UniqueConnection);
            return;
        }

        disconnect(m_notebookModel, &NotebookModel::documentChanged, this, &NotebookFilterModel::hookToScreenplaySignals);

        Screenplay *screenplay = document->screenplay();
        connect(screenplay, &Screenplay::rowsMoved, this, &NotebookFilterModel::invalidate, Qt::UniqueConnection);

        this->sort(0);
    }
}

void NotebookFilterModel::dropHookstoScreenplaySignals()
{
    if(m_notebookModel != nullptr)
    {
        disconnect(m_notebookModel, &NotebookModel::documentChanged, this, &NotebookFilterModel::hookToScreenplaySignals);
        ScriteDocument *document = m_notebookModel->document();
        if(document == nullptr)
            return;

        Screenplay *screenplay = document->screenplay();
        disconnect(screenplay, &Screenplay::rowsMoved, this, &NotebookFilterModel::invalidate);
    }
}
