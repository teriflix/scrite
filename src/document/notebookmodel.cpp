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

static int nextItemId()
{
    static int id = 1;
    return id++;
}

class StandardItemWithId : public QStandardItem
{
public:
    StandardItemWithId() : QStandardItem() {
        this->setData(::nextItemId(), NotebookModel::IdRole);
    }
    ~StandardItemWithId() { }
};

class ObjectItem : public StandardItemWithId
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

class NoteItem : public ObjectItem
{
public:
    NoteItem(Note *note);
    ~NoteItem();

private:
    void updateText();

private:
    Note *m_note = nullptr;
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
    QList<QMetaObject::Connection> m_connections;
};

struct StoryNode
{
    ~StoryNode();

    Screenplay *screenplay = nullptr;
    ScreenplayElement *episode = nullptr;
    ScreenplayElement *act = nullptr;
    ScreenplayElement *scene = nullptr;

    QString actName;
    QString episodeName;

    Structure *structure = nullptr;
    StructureElement *unusedScene = nullptr;

    QList<StoryNode*> childNodes;

    static StoryNode *create(ScriteDocument *document=nullptr);

private:
    StoryNode();
};

NotebookModel::NotebookModel(QObject *parent)
    : QStandardItemModel(parent),
      m_document(this, "document")
{
    m_syncScenesTimer.setSingleShot(true);
    m_syncCharactersTimer.setSingleShot(true);

    // The timeout interval for these has to be > 0, because we have to let
    // ExecLater timers in Structure and Screenplay to do their thing before
    // we sync.
    m_syncScenesTimer.setInterval(50);
    m_syncCharactersTimer.setInterval(50);

    connect(&m_syncScenesTimer, &QTimer::timeout, this, &NotebookModel::syncScenes);
    connect(&m_syncCharactersTimer, &QTimer::timeout, this, &NotebookModel::syncCharacters);
    connect(this, &NotebookModel::dataChanged, this, &NotebookModel::onDataChanged);
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

        ret[ QStringLiteral("modelIndex") ] = index;

        return ret;
    }

    return QStandardItemModel::data(index, role);
}

QHash<int, QByteArray> NotebookModel::staticRoleNames()
{
    static QHash<int,QByteArray> roles = {
        { IdRole, QByteArrayLiteral("notebookItemId") },
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

QStandardItem *createItemForNode(StoryNode *node)
{
    QStandardItem *nodeItem = nullptr;
    if(node->scene == nullptr && node->unusedScene == nullptr)
        nodeItem = new StandardItemWithId;
    else if(node->scene != nullptr)
        nodeItem = new NotesItem(node->scene->scene()->notes());
    else if(node->unusedScene != nullptr)
        nodeItem = new NotesItem(node->unusedScene->scene()->notes());

    Notes *nodeNotes = nullptr;

    if(node->screenplay != nullptr)
    {
        nodeItem->setText( QStringLiteral("Screenplay") );
        nodeItem->setData(NotebookModel::CategoryType, NotebookModel::TypeRole);
        nodeItem->setData(NotebookModel::ScreenplayCategory, NotebookModel::CategoryRole);
    }
    else if(node->structure != nullptr)
    {
        nodeItem->setText( QStringLiteral("Unused Scenes") );
        nodeItem->setData(NotebookModel::CategoryType, NotebookModel::TypeRole);
        nodeItem->setData(NotebookModel::UnusedScenesCategory, NotebookModel::CategoryRole);
    }
    else if(node->episode != nullptr)
    {
        if(node->episode->breakSubtitle().isEmpty())
            nodeItem->setText( node->episode->breakTitle() );
        else
            nodeItem->setText( node->episode->breakTitle() + QStringLiteral(": ") + node->episode->breakSubtitle() );

        nodeItem->setData(NotebookModel::EpisodeBreakType, NotebookModel::TypeRole);
        nodeItem->setData(QVariant::fromValue<QObject*>(node->episode), NotebookModel::ObjectRole);

        nodeNotes = node->episode->notes();
    }
    else if(node->act != nullptr)
    {
        nodeItem->setText( node->act->breakTitle() );
        nodeItem->setData(NotebookModel::ActBreakType, NotebookModel::TypeRole);
        nodeItem->setData(QVariant::fromValue<QObject*>(node->act), NotebookModel::ObjectRole);

        nodeNotes = node->act->notes();
    }
    else if(!node->episodeName.isEmpty())
    {
        nodeItem->setData(NotebookModel::EpisodeBreakType, NotebookModel::TypeRole);
        nodeItem->setText( node->episodeName );
    }
    else if(!node->actName.isEmpty())
    {
        nodeItem->setText( node->actName );
        nodeItem->setData(NotebookModel::ActBreakType, NotebookModel::TypeRole);
    }
    else if(node->scene == nullptr && node->unusedScene == nullptr)
        nodeItem->setText( QStringLiteral("Story Notes") );

    if(nodeNotes != nullptr)
    {
        NotesItem *nodeNotesItem = new NotesItem(nodeNotes);
        nodeItem->appendRow(nodeNotesItem);
    }

    for(StoryNode *childNode : qAsConst(node->childNodes))
    {
        QStandardItem *childNodeItem = createItemForNode(childNode);
        nodeItem->appendRow(childNodeItem);
    }

    return nodeItem;
}

void NotebookModel::loadScenes()
{
    Structure *structure = m_document->structure();
    Screenplay *screenplay = m_document->screenplay();

    connect(structure, &Structure::elementCountChanged, &m_syncScenesTimer, QOverload<>::of(&QTimer::start), Qt::UniqueConnection);
    connect(screenplay, &Screenplay::elementsChanged, &m_syncScenesTimer, QOverload<>::of(&QTimer::start), Qt::UniqueConnection);

    this->syncScenes();
}

void NotebookModel::loadCharacters()
{
    QStandardItem *charactersItem = new StandardItemWithId;
    charactersItem->setText( QStringLiteral("Characters") );
    charactersItem->setData(CategoryType, TypeRole);
    charactersItem->setData(CharactersCategory, CategoryRole);

    Structure *structure = m_document->structure();
    ObjectListPropertyModel<Character *> *charactersModel = structure->charactersModel();

    QList<Character*> characters = charactersModel->list();
    std::sort(characters.begin(), characters.end(), [](Character *a, Character *b) {
        return a->name() < b->name();
    });

    for(Character *character : qAsConst(characters))
    {
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
    QStandardItem *locationsGroupItem = new StandardItemWithId;
    locationsGroupItem->setText( QStringLiteral("Locations") );
    locationsGroupItem->setData(CategoryType, TypeRole);
    locationsGroupItem->setData(LocationsCategory, CategoryRole);

    this->appendRow(locationsGroupItem);
#endif
}

void NotebookModel::loadProps()
{
#if 0
    QStandardItem *propsGroupItem = new StandardItemWithId;
    propsGroupItem->setText( QStringLiteral("Props") );
    propsGroupItem->setData(CategoryType, TypeRole);
    propsGroupItem->setData(LocationsCategory, CategoryRole);

    this->appendRow(propsGroupItem);
#endif
}

void NotebookModel::loadOthers()
{
#if 0
    QStandardItem *otherCategoryGroupItem = new StandardItemWithId;
    otherCategoryGroupItem->setText( QStringLiteral("Others") );
    otherCategoryGroupItem->setData(CategoryType, TypeRole);
    otherCategoryGroupItem->setData(LocationsCategory, CategoryRole);

    this->appendRow(otherCategoryGroupItem);
#endif
}

void NotebookModel::syncScenes()
{
    QScopedPointer<StoryNode> storyNodes( StoryNode::create(m_document) );
    if(storyNodes.isNull())
        return;

    StoryNode *structureNode = nullptr, *screenplayNode = nullptr;
    for(StoryNode *storyNode : qAsConst(storyNodes->childNodes))
    {
        if(storyNode->structure != nullptr)
            structureNode = storyNode;
        else if(storyNode->screenplay != nullptr)
            screenplayNode = storyNode;
    }

    auto removeTopLevelItems = [=](QList<QStandardItem*> &items) {
        QList<int> rows;
        while(!items.isEmpty()) {
            QStandardItem *item = items.takeLast();
            rows << item->row();
        }
        std::sort(rows.begin(), rows.end());
        while(!rows.isEmpty())
            this->removeRow(rows.takeLast());
    };

    QList<QStandardItem *> unusedScenesItem = this->findItems(QStringLiteral("Unused Scenes"), Qt::MatchExactly, 0);
    QList<QStandardItem *> screenplayScenesItem = this->findItems(QStringLiteral("Screenplay"), Qt::MatchExactly, 0);
    bool hasScenes = unusedScenesItem.size() + screenplayScenesItem.size() > 0;

    if(hasScenes)
        emit aboutToReloadScenes();

    removeTopLevelItems(unusedScenesItem);
    if(structureNode != nullptr)
        this->insertRow(1, createItemForNode(structureNode));

    removeTopLevelItems(screenplayScenesItem);
    if(screenplayNode != nullptr)
        this->insertRow(1, createItemForNode(screenplayNode));

    if(hasScenes)
        emit justReloadedScenes();
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

void NotebookModel::onDataChanged(const QModelIndex &start, const QModelIndex &end, const QVector<int> &roles)
{
    /** ModelDataRole is a combination of all roles provided by the model. Anytime any role changes,
        ModelDataRole should also be announced as changed. Otherwise, views connected to this model
        which depends on ModelDataRole wont update themselves. */
    if(!roles.isEmpty() && !roles.contains(ModelDataRole))
        emit dataChanged(start, end, {ModelDataRole});
}

///////////////////////////////////////////////////////////////////////////////

ObjectItem::ObjectItem(QObject *object)
    : StandardItemWithId(), m_object(object)
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

NoteItem::NoteItem(Note *note)
         :ObjectItem(note),
           m_note(note)
{
    this->updateText();
    this->setData(NotebookModel::NoteType, NotebookModel::TypeRole);

    QObject::connect(m_note, &Note::titleChanged, m_note, [=](){
        this->updateText();
    });
}

NoteItem::~NoteItem()
{

}

void NoteItem::updateText()
{
    const QString title = m_note->title();
    this->setText( title.isEmpty() ? QStringLiteral("New Note") : title );
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
                m_connections << QObject::connect(element, &StructureElement::titleChanged, m_notes, updateTextSlot);
            else
                m_connections << QObject::connect(m_notes->scene(), &Scene::titleChanged, m_notes, updateTextSlot);
            m_connections << QObject::connect(m_notes->scene(), &Scene::screenplayElementIndexListChanged, m_notes, updateTextSlot);
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
        NoteItem *noteItem = new NoteItem(note);
        noteItems.append(noteItem);
    }

    this->appendRows(noteItems);

    // Why do we use a timer here? Why not directly call sync?
    // Because noteCountChanged() is emitted before objectDestroyed()
    m_syncTimer.setInterval(0);
    m_syncTimer.setSingleShot(true);
    m_connections << QObject::connect(&m_syncTimer, &QTimer::timeout, m_notes, [=]() { this->sync(); });
    m_connections << QObject::connect(m_notes, &Notes::noteCountChanged, &m_syncTimer, QOverload<>::of(&QTimer::start));
}

NotesItem::~NotesItem()
{
    m_syncTimer.stop();
    while(!m_connections.isEmpty())
        QObject::disconnect(m_connections.takeFirst());
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
            noteItem = new NoteItem(actualNote);
            this->insertRow(i, noteItem);
        }
    }
}

void NotesItem::updateText()
{
    switch(m_notes->ownerType())
    {
    case Notes::StructureOwner:
        this->setText(QStringLiteral("Story Notes"));
        return;
    case Notes::BreakOwner: {
        ScreenplayElement *spelement = qobject_cast<ScreenplayElement*>(m_notes->parent());
        if(spelement->breakType() < 0)
            this->setText(QStringLiteral("Break Notes"));
        if(spelement->breakType() == Screenplay::Episode)
            this->setText(QStringLiteral("Episode Notes"));
        else
            this->setText(QStringLiteral("Act Notes"));
        } return;
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

StoryNode::StoryNode()
{

}

StoryNode::~StoryNode()
{
    qDeleteAll(childNodes);
    childNodes.clear();
}

StoryNode *StoryNode::create(ScriteDocument *document)
{
    if(document == nullptr)
        document = ScriteDocument::instance();

    Structure *structure = document->structure();
    Screenplay *screenplay = document->screenplay();

    QList<StructureElement*> structureElements = structure->elementsModel()->list();

    StoryNode *rootNode = new StoryNode;

    // Dump all scenes into screenplay nodes first.
    StoryNode *screenplayNode = new StoryNode;
    screenplayNode->screenplay = screenplay;
    rootNode->childNodes.append(screenplayNode);

    StoryNode *actNode = nullptr;
    StoryNode *episodeNode = nullptr;

    const int nrElements = screenplay->elementCount();
    for(int i=0; i<nrElements; i++)
    {
        ScreenplayElement *element = screenplay->elementAt(i);

        if(element->elementType() == ScreenplayElement::BreakElementType)
        {
            if(element->breakType() == Screenplay::Episode)
            {
                if(episodeNode == nullptr && !screenplayNode->childNodes.isEmpty())
                {
                    // User has not created an Episode 1, so we are going to create Episode 1
                    // node just for the sake of the notebook model.
                    episodeNode = new StoryNode;
                    episodeNode->episodeName = QStringLiteral("EPISODE 1");
                    episodeNode->childNodes = screenplayNode->childNodes;
                    screenplayNode->childNodes.clear();
                    screenplayNode->childNodes.append(episodeNode);
                }

                episodeNode = new StoryNode;
                episodeNode->episode = element;
                screenplayNode->childNodes.append(episodeNode);

                actNode = nullptr;
            }
            else if(element->breakType() == Screenplay::Act)
            {
                StoryNode *parentNode = episodeNode ? episodeNode : screenplayNode;

                if(actNode == nullptr && !parentNode->childNodes.isEmpty())
                {
                    actNode = new StoryNode;
                    actNode->actName = QStringLiteral("ACT 1");
                    actNode->childNodes = parentNode->childNodes;
                    parentNode->childNodes.clear();
                    parentNode->childNodes.append(actNode);
                }

                actNode = new StoryNode;
                actNode->act = element;
                if(episodeNode == nullptr)
                    screenplayNode->childNodes.append(actNode);
                else
                    episodeNode->childNodes.append(actNode);
            }
        }
        else
        {
            StoryNode *sceneNode = new StoryNode;
            sceneNode->scene = element;
            if(actNode != nullptr)
                actNode->childNodes.append(sceneNode);
            else if(episodeNode != nullptr)
                episodeNode->childNodes.append(sceneNode);
            else
                screenplayNode->childNodes.append(sceneNode);

            structureElements.removeOne(element->scene()->structureElement());
        }
    }

    // Dump remaining structure scenes
    if(!structureElements.isEmpty())
    {
        StoryNode *structureNode = new StoryNode;
        structureNode->structure = structure;
        rootNode->childNodes.append(structureNode);

        for(StructureElement *element : qAsConst(structureElements))
        {
            StoryNode *unusedSceneNode = new StoryNode;
            unusedSceneNode->unusedScene = element;
            structureNode->childNodes.append(unusedSceneNode);
        }
    }

    return rootNode;
}


