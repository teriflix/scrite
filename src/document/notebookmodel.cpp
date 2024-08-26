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

#include "notebookmodel.h"

#include "form.h"
#include "notes.h"
#include "scene.h"
#include "structure.h"
#include "application.h"
#include "timeprofiler.h"
#include "scritedocument.h"

#include <QScopedValueRollback>

static int nextItemId()
{
    static int id = 1000;
    return id++;
}

class StandardItemWithId : public QStandardItem
{
public:
    explicit StandardItemWithId(int id = -1) : QStandardItem()
    {
        this->setData(id >= 0 ? id : ::nextItemId(), NotebookModel::IdRole);
    }
    ~StandardItemWithId() { }
};

class BookmarksItem : public StandardItemWithId
{
public:
    explicit BookmarksItem();
    ~BookmarksItem();

private:
    void updateText();

private:
    QMetaObject::Connection m_connection;
};

class ObjectItem : public StandardItemWithId
{
public:
    explicit ObjectItem(QObject *object);
    ~ObjectItem();

private:
    void objectDestroyed(QObject *ptr);

private:
    QObject *m_object = nullptr;
    QMetaObject::Connection m_destroyedConnection;

protected:
    QList<QMetaObject::Connection> m_connections;
};

class NoteItem : public ObjectItem
{
public:
    explicit NoteItem(Note *note);
    ~NoteItem();

private:
    void updateText();

private:
    Note *m_note = nullptr;
};

class NotesItem : public ObjectItem
{
public:
    explicit NotesItem(Notes *notes);
    ~NotesItem();

    void sync();
    void updateText();

private:
    Notes *m_notes = nullptr;
    QTimer m_syncTimer;
};

class ActItem : public ObjectItem
{
public:
    explicit ActItem(ScreenplayElement *element);
    ~ActItem();

    void updateText();

private:
    ScreenplayElement *m_element;
};

class EpisodeItem : public ObjectItem
{
public:
    explicit EpisodeItem(ScreenplayElement *element);
    ~EpisodeItem();

    void updateText();

private:
    ScreenplayElement *m_element;
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

    QList<StoryNode *> childNodes;

    static StoryNode *create(ScriteDocument *document = nullptr);

private:
    StoryNode();
};

NotebookModel::NotebookModel(QObject *parent)
    : QStandardItemModel(parent),
      m_document(this, "document"),
      m_bookmarkedNotes(new BookmarkedNotes(this))
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

    // Ensure that we query the global forms object right away, so that the NotebookView.qml
    // is capable of showing forms against characters.
    Forms::global();
}

NotebookModel::~NotebookModel() { }

void NotebookModel::setDocument(ScriteDocument *val)
{
    if (m_document == val)
        return;

    if (m_document != nullptr) {
        disconnect(m_document, &ScriteDocument::justReset, this, &NotebookModel::reload);
        disconnect(m_document, &ScriteDocument::justLoaded, this, &NotebookModel::reload);
    }

    m_document = val;
    emit documentChanged();

    if (m_document != nullptr) {
        connect(m_document, &ScriteDocument::justReset, this, &NotebookModel::reload);
        connect(m_document, &ScriteDocument::justLoaded, this, &NotebookModel::reload);
    }

    this->reload();
}

QVariant NotebookModel::modelIndexData(const QModelIndex &index) const
{
    return this->data(index, ModelDataRole);
}

QStandardItem *recursivelyFindItemForOnwer(QStandardItem *root, QObject *owner)
{
    if (root == nullptr || owner == nullptr)
        return nullptr;

    if (root->data(NotebookModel::ObjectRole).value<QObject *>() == owner)
        return root;

    const int nrRows = root->rowCount();
    for (int i = 0; i < nrRows; i++) {
        QStandardItem *row = root->child(i, 0);
        QStandardItem *found = recursivelyFindItemForOnwer(row, owner);
        if (found != nullptr)
            return found;
    }

    return nullptr;
}

QModelIndex NotebookModel::findModelIndexFor(QObject *owner) const
{
    QStandardItem *item = ::recursivelyFindItemForOnwer(this->invisibleRootItem(), owner);
    if (item == nullptr)
        return QModelIndex();

    return this->indexFromItem(item);
}

QModelIndex NotebookModel::findModelIndexForTopLevelItem(const QString &label) const
{
    QList<QStandardItem *> items = this->findItems(label, Qt::MatchExactly);
    if (items.isEmpty())
        return QModelIndex();

    return this->indexFromItem(items.first());
}

QModelIndex NotebookModel::findModelIndexForCategory(ItemCategory cat) const
{
    const int nrItems = this->rowCount();
    for (int i = 0; i < nrItems; i++) {
        QStandardItem *row = this->item(i, 0);
        if (row->data(CategoryRole).toInt() == cat)
            return this->indexFromItem(row);
    }

    return QModelIndex();
}

void NotebookModel::refresh()
{
    emit aboutToRefresh();
    this->reload();
    emit justRefreshed();
}

QHash<int, QByteArray> NotebookModel::roleNames() const
{
    return staticRoleNames();
}

QVariant NotebookModel::data(const QModelIndex &index, int role) const
{
    if (role == ModelDataRole) {
        QHash<int, QByteArray> roles = staticRoleNames();
        roles.remove(ModelDataRole);

        QVariantMap ret;

        auto it = roles.begin();
        auto end = roles.end();
        while (it != end) {
            ret[QString::fromLatin1(it.value())] = QStandardItemModel::data(index, it.key());
            ++it;
        }

        ret[QStringLiteral("modelIndex")] = index;

        return ret;
    }

    return QStandardItemModel::data(index, role);
}

QHash<int, QByteArray> NotebookModel::staticRoleNames()
{
    static QHash<int, QByteArray> roles = {
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

    if (m_document == nullptr)
        return;

    this->loadBookmarks();
    this->loadStory();
    this->loadScenes();
    this->loadCharacters();
    this->loadLocations();
    this->loadProps();
    this->loadOthers();
}

void NotebookModel::loadBookmarks()
{
    this->appendRow(new BookmarksItem);
}

void NotebookModel::loadStory()
{
    Structure *structure = m_document->structure();
    Notes *storyNotes = structure->notes();

    NotesItem *storyNotesItem = new NotesItem(storyNotes);
    storyNotesItem->setData(1, NotebookModel::IdRole);
    this->appendRow(storyNotesItem);
}

QStandardItem *createItemForNode(StoryNode *node)
{
    QStandardItem *nodeItem = nullptr;
    if (node->scene == nullptr && node->unusedScene == nullptr) {
        if (node->screenplay != nullptr)
            nodeItem = new StandardItemWithId(3);
        else if (node->structure != nullptr)
            nodeItem = new StandardItemWithId(2);
        else if (node->episode != nullptr)
            nodeItem = new EpisodeItem(node->episode);
        else if (node->act != nullptr)
            nodeItem = new ActItem(node->act);
        else
            nodeItem = new StandardItemWithId;
    } else if (node->scene != nullptr)
        nodeItem = new NotesItem(node->scene->scene()->notes());
    else if (node->unusedScene != nullptr)
        nodeItem = new NotesItem(node->unusedScene->scene()->notes());

    Notes *nodeNotes = nullptr;

    if (node->screenplay != nullptr) {
        nodeItem->setText(QStringLiteral("Screenplay"));
        nodeItem->setData(NotebookModel::CategoryType, NotebookModel::TypeRole);
        nodeItem->setData(NotebookModel::ScreenplayCategory, NotebookModel::CategoryRole);
    } else if (node->structure != nullptr) {
        nodeItem->setText(QStringLiteral("Unused Scenes"));
        nodeItem->setData(NotebookModel::CategoryType, NotebookModel::TypeRole);
        nodeItem->setData(NotebookModel::UnusedScenesCategory, NotebookModel::CategoryRole);
    } else if (node->episode != nullptr)
        nodeNotes = node->episode->notes();
    else if (node->act != nullptr)
        nodeNotes = node->act->notes();
    else if (!node->episodeName.isEmpty()) {
        nodeItem->setData(NotebookModel::EpisodeBreakType, NotebookModel::TypeRole);
        nodeItem->setText(node->episodeName);
    } else if (!node->actName.isEmpty()) {
        nodeItem->setText(node->actName);
        nodeItem->setData(NotebookModel::ActBreakType, NotebookModel::TypeRole);
    } else if (node->scene == nullptr && node->unusedScene == nullptr)
        nodeItem->setText(QStringLiteral("Story Notes"));

    if (nodeNotes != nullptr) {
        NotesItem *nodeNotesItem = new NotesItem(nodeNotes);
        nodeItem->appendRow(nodeNotesItem);
    }

    for (StoryNode *childNode : qAsConst(node->childNodes)) {
        QStandardItem *childNodeItem = createItemForNode(childNode);
        nodeItem->appendRow(childNodeItem);
    }

    return nodeItem;
}

void NotebookModel::loadScenes()
{
    Structure *structure = m_document->structure();
    Screenplay *screenplay = m_document->screenplay();

    this->syncScenes();

    connect(structure, &Structure::elementCountChanged, &m_syncScenesTimer,
            QOverload<>::of(&QTimer::start), Qt::UniqueConnection);
    connect(screenplay, &Screenplay::elementsChanged, &m_syncScenesTimer,
            QOverload<>::of(&QTimer::start), Qt::UniqueConnection);
}

void NotebookModel::loadCharacters()
{
    Structure *structure = m_document->structure();

    this->syncCharacters();

    connect(structure, &Structure::characterCountChanged, &m_syncCharactersTimer,
            QOverload<>::of(&QTimer::start), Qt::UniqueConnection);
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

static inline void removeTopLevelItems(QStandardItemModel *model, QList<QStandardItem *> &items)
{
    QList<int> rows;
    while (!items.isEmpty()) {
        QStandardItem *item = items.takeLast();
        rows << item->row();
    }
    std::sort(rows.begin(), rows.end());
    while (!rows.isEmpty())
        model->removeRow(rows.takeLast());
};

void NotebookModel::syncScenes()
{
    QScopedPointer<StoryNode> storyNodes(StoryNode::create(m_document));
    if (storyNodes.isNull())
        return;

    StoryNode *structureNode = nullptr, *screenplayNode = nullptr;
    for (StoryNode *storyNode : qAsConst(storyNodes->childNodes)) {
        if (storyNode->structure != nullptr)
            structureNode = storyNode;
        else if (storyNode->screenplay != nullptr)
            screenplayNode = storyNode;
    }

    QList<QStandardItem *> unusedScenesItem =
            this->findItems(QStringLiteral("Unused Scenes"), Qt::MatchExactly, 0);
    QList<QStandardItem *> screenplayScenesItem =
            this->findItems(QStringLiteral("Screenplay"), Qt::MatchExactly, 0);
    bool hasScenes = unusedScenesItem.size() + screenplayScenesItem.size() > 0;

    if (hasScenes)
        emit aboutToReloadScenes();

    removeTopLevelItems(this, unusedScenesItem);
    if (structureNode != nullptr)
        this->insertRow(2, createItemForNode(structureNode));

    removeTopLevelItems(this, screenplayScenesItem);
    if (screenplayNode != nullptr)
        this->insertRow(2, createItemForNode(screenplayNode));

    if (hasScenes)
        emit justReloadedScenes();
}

void NotebookModel::syncCharacters()
{
    Structure *structure = m_document->structure();
    QObjectListModel<Character *> *charactersModel = structure->charactersModel();

    QList<QStandardItem *> characterItems =
            this->findItems(QStringLiteral("Characters"), Qt::MatchExactly, 0);
    const bool hasCharacterItems = !characterItems.isEmpty();

    if (hasCharacterItems)
        emit aboutToReloadCharacters();

    removeTopLevelItems(this, characterItems);

    QStandardItem *charactersItem = new StandardItemWithId(4);
    charactersItem->setText(QStringLiteral("Characters"));
    charactersItem->setData(CategoryType, TypeRole);
    charactersItem->setData(CharactersCategory, CategoryRole);

    QList<Character *> characters = charactersModel->list();
    std::sort(characters.begin(), characters.end(), [](Character *a, Character *b) {
        if (a->priority() == b->priority())
            return a->name() < b->name();
        return a->priority() > b->priority();
    });

    for (Character *character : qAsConst(characters)) {
        Notes *characterNotes = character->notes();

        NotesItem *notesItem = new NotesItem(characterNotes);
        charactersItem->appendRow(notesItem);
    }

    this->appendRow(charactersItem);

    if (hasCharacterItems)
        emit justReloadedCharacters();
}

void NotebookModel::onDataChanged(const QModelIndex &start, const QModelIndex &end,
                                  const QVector<int> &roles)
{
    /** ModelDataRole is a combination of all roles provided by the model. Anytime any role changes,
        ModelDataRole should also be announced as changed. Otherwise, views connected to this model
        which depends on ModelDataRole wont update themselves. */
    if (!roles.isEmpty() && !roles.contains(ModelDataRole))
        emit dataChanged(start, end, { ModelDataRole });
}

///////////////////////////////////////////////////////////////////////////////

BookmarksItem::BookmarksItem() : StandardItemWithId(0)
{
    ScriteDocument *doc = ScriteDocument::instance();
    m_connection = QObject::connect(doc, &ScriteDocument::bookmarkedNotesChanged, doc,
                                    [=]() { this->updateText(); });
    this->updateText();

    this->setData(NotebookModel::CategoryType, NotebookModel::TypeRole);
    this->setData(NotebookModel::BookmarksCategory, NotebookModel::CategoryRole);
}

BookmarksItem::~BookmarksItem()
{
    QObject::disconnect(m_connection);
}

void BookmarksItem::updateText()
{
    const ScriteDocument *doc = ScriteDocument::instance();
    const int nr = doc->bookmarkedNotes().size();
    this->setText(QStringLiteral("Bookmarks (") + QString::number(nr) + QStringLiteral(")"));
}

ObjectItem::ObjectItem(QObject *object) : StandardItemWithId(), m_object(object)
{
    m_connections << QObject::connect(m_object, &QObject::destroyed, m_object,
                                      [=](QObject *ptr) { this->objectDestroyed(ptr); });
    this->setData(QVariant::fromValue<QObject *>(m_object), NotebookModel::ObjectRole);
    this->setText(QStringLiteral("Note"));
}

ObjectItem::~ObjectItem()
{
    while (!m_connections.isEmpty())
        QObject::disconnect(m_connections.takeFirst());
}

void ObjectItem::objectDestroyed(QObject *ptr)
{
    if (ptr == m_object) {
        QObject::disconnect(m_destroyedConnection);
        m_destroyedConnection = QMetaObject::Connection();

        QStandardItem *parentItem = this->parent();
        if (parentItem != nullptr) {
            const int row = this->row();
            parentItem->removeRow(row);
        } else
            delete this;
    }
}

NoteItem::NoteItem(Note *note) : ObjectItem(note), m_note(note)
{
    this->updateText();
    this->setData(NotebookModel::NoteType, NotebookModel::TypeRole);

    m_connections << QObject::connect(m_note, &Note::titleChanged, m_note,
                                      [=]() { this->updateText(); });
}

NoteItem::~NoteItem() { }

void NoteItem::updateText()
{
    const QString title = m_note->title();
    this->setText(title.isEmpty() ? QStringLiteral("New Note") : title);
}

NotesItem::NotesItem(Notes *notes) : ObjectItem(notes), m_notes(notes)
{
    this->updateText();
    switch (m_notes->ownerType()) {
    case Notes::SceneOwner: {
        StructureElement *element = m_notes->scene()->structureElement();
        auto updateTextSlot = [=]() { this->updateText(); };
        if (element)
            m_connections << QObject::connect(element, &StructureElement::titleChanged, m_notes,
                                              updateTextSlot);
        else
            m_connections << QObject::connect(m_notes->scene(), &Scene::synopsisChanged, m_notes,
                                              updateTextSlot);
        m_connections << QObject::connect(m_notes->scene(),
                                          &Scene::screenplayElementIndexListChanged, m_notes,
                                          updateTextSlot);
    } break;
    case Notes::CharacterOwner: {
        Character *character = m_notes->character();
        if (character) {
            auto updateTextSlot = [=]() { this->updateText(); };
            m_connections << QObject::connect(character, &Character::nameChanged, m_notes,
                                              updateTextSlot);
        }
    } break;
    default:
        break;
    }

    this->setData(NotebookModel::NotesType, NotebookModel::TypeRole);

    const int nrNotes = m_notes->noteCount();

    QList<QStandardItem *> noteItems;
    noteItems.reserve(nrNotes);

    for (int i = 0; i < nrNotes; i++) {
        Note *note = m_notes->noteAt(i);
        NoteItem *noteItem = new NoteItem(note);
        noteItems.append(noteItem);
    }

    this->appendRows(noteItems);

    // Why do we use a timer here? Why not directly call sync?
    // Because noteCountChanged() is emitted before objectDestroyed()
    m_syncTimer.setInterval(0);
    m_syncTimer.setSingleShot(true);
    m_connections << QObject::connect(&m_syncTimer, &QTimer::timeout, m_notes,
                                      [=]() { this->sync(); });
    m_connections << QObject::connect(m_notes, &Notes::noteCountChanged, &m_syncTimer,
                                      QOverload<>::of(&QTimer::start));
}

NotesItem::~NotesItem()
{
    m_syncTimer.stop();
}

void NotesItem::sync()
{
    if (this->rowCount() > m_notes->noteCount()) {
        m_syncTimer.start();
        return;
    }

    const int noteCount = m_notes->noteCount();
    for (int i = 0; i < noteCount; i++) {
        Note *actualNote = m_notes->at(i);
        QStandardItem *noteItem = this->child(i);
        Note *assignedNote = noteItem
                ? qobject_cast<Note *>(noteItem->data(NotebookModel::ObjectRole).value<QObject *>())
                : nullptr;

        if (actualNote != assignedNote) {
            noteItem = new NoteItem(actualNote);
            this->insertRow(i, noteItem);
        }
    }
}

void NotesItem::updateText()
{
    switch (m_notes->ownerType()) {
    case Notes::StructureOwner:
        this->setText(QStringLiteral("Story Notes"));
        return;
    case Notes::BreakOwner: {
        ScreenplayElement *spelement = qobject_cast<ScreenplayElement *>(m_notes->parent());
        if (spelement->breakType() < 0)
            this->setText(QStringLiteral("Break Notes"));
        if (spelement->breakType() == Screenplay::Episode)
            this->setText(QStringLiteral("Episode Notes"));
        else
            this->setText(QStringLiteral("Act Notes"));
    }
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
        for (int val : qAsConst(indexes)) {
            ScreenplayElement *element = screenplay->elementAt(val);
            if (element)
                idxStringList << element->resolvedSceneNumber();
            else
                idxStringList << QString::number(val + 1);
        }
        idxStringList.removeAll(QString());

        StructureElement *element = m_notes->scene()->structureElement();
        QString title;
        if (element)
            title = element->title();
        else
            title = m_notes->scene()->synopsis();
        if (!idxStringList.isEmpty())
            title = QStringLiteral("[") + idxStringList.join(QStringLiteral(","))
                    + QStringLiteral("]: ") + title;
        this->setText(title);
    }
        return;
    case Notes::OtherOwner:
        this->setText(QStringLiteral("Other"));
        return;
    }

    this->setText(QStringLiteral("Notes"));
}

ActItem::ActItem(ScreenplayElement *element) : ObjectItem(element), m_element(element)
{
    this->updateText();
    auto callUpdateText = [=]() { this->updateText(); };
    m_connections << QObject::connect(element, &ScreenplayElement::breakTitleChanged,
                                      callUpdateText);
    m_connections << QObject::connect(element, &ScreenplayElement::breakSubtitleChanged,
                                      callUpdateText);
    this->setData(NotebookModel::ActBreakType, NotebookModel::TypeRole);
    this->setData(QVariant::fromValue<QObject *>(element), NotebookModel::ObjectRole);
}

ActItem::~ActItem() { }

void ActItem::updateText()
{
    if (m_element->breakSubtitle().isEmpty())
        this->setText(m_element->breakTitle());
    else
        this->setText(m_element->breakTitle() + QStringLiteral(": ") + m_element->breakSubtitle());
}

EpisodeItem::EpisodeItem(ScreenplayElement *element) : ObjectItem(element), m_element(element)
{
    this->updateText();
    auto callUpdateText = [=]() { this->updateText(); };
    m_connections << QObject::connect(element, &ScreenplayElement::breakTitleChanged,
                                      callUpdateText);
    m_connections << QObject::connect(element, &ScreenplayElement::breakSubtitleChanged,
                                      callUpdateText);

    this->setData(NotebookModel::EpisodeBreakType, NotebookModel::TypeRole);
    this->setData(QVariant::fromValue<QObject *>(element), NotebookModel::ObjectRole);
}

EpisodeItem::~EpisodeItem() { }

void EpisodeItem::updateText()
{
    if (m_element->breakSubtitle().isEmpty())
        this->setText(m_element->breakTitle());
    else
        this->setText(m_element->breakTitle() + QStringLiteral(": ") + m_element->breakSubtitle());
}

StoryNode::StoryNode() { }

StoryNode::~StoryNode()
{
    qDeleteAll(childNodes);
    childNodes.clear();
}

StoryNode *StoryNode::create(ScriteDocument *document)
{
    if (document == nullptr)
        document = ScriteDocument::instance();

    Structure *structure = document->structure();
    Screenplay *screenplay = document->screenplay();

    QList<StructureElement *> structureElements = structure->elementsModel()->list();

    StoryNode *rootNode = new StoryNode;

    // Dump all scenes into screenplay nodes first.
    StoryNode *screenplayNode = new StoryNode;
    screenplayNode->screenplay = screenplay;
    rootNode->childNodes.append(screenplayNode);

    StoryNode *actNode = nullptr;
    StoryNode *episodeNode = nullptr;

    const int nrElements = screenplay->elementCount();
    for (int i = 0; i < nrElements; i++) {
        ScreenplayElement *element = screenplay->elementAt(i);

        if (element->elementType() == ScreenplayElement::BreakElementType) {
            if (element->breakType() == Screenplay::Episode) {
                if (episodeNode == nullptr && !screenplayNode->childNodes.isEmpty()) {
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
            } else if (element->breakType() == Screenplay::Act) {
                StoryNode *parentNode = episodeNode ? episodeNode : screenplayNode;

                if (actNode == nullptr && !parentNode->childNodes.isEmpty()) {
                    // User has not created an Act 1, so we are going to create Act 1
                    // node just for the sake of the notebook model.
                    actNode = new StoryNode;
                    actNode->actName = QStringLiteral("ACT 1");
                    actNode->childNodes = parentNode->childNodes;
                    parentNode->childNodes.clear();
                    parentNode->childNodes.append(actNode);
                }

                actNode = new StoryNode;
                actNode->act = element;
                if (episodeNode == nullptr)
                    screenplayNode->childNodes.append(actNode);
                else
                    episodeNode->childNodes.append(actNode);
            }
        } else {
            StoryNode *sceneNode = new StoryNode;
            sceneNode->scene = element;
            if (actNode != nullptr)
                actNode->childNodes.append(sceneNode);
            else if (episodeNode != nullptr)
                episodeNode->childNodes.append(sceneNode);
            else
                screenplayNode->childNodes.append(sceneNode);

            structureElements.removeOne(element->scene()->structureElement());
        }
    }

    // Dump remaining structure scenes
    if (!structureElements.isEmpty()) {
        StoryNode *structureNode = new StoryNode;
        structureNode->structure = structure;
        rootNode->childNodes.append(structureNode);

        for (StructureElement *element : qAsConst(structureElements)) {
            StoryNode *unusedSceneNode = new StoryNode;
            unusedSceneNode->unusedScene = element;
            structureNode->childNodes.append(unusedSceneNode);
        }
    }

    return rootNode;
}

///////////////////////////////////////////////////////////////////////////////

BookmarkedNotes::BookmarkedNotes(QObject *parent) : QObjectListModel<QObject *>(parent)
{
    this->reload();
}

BookmarkedNotes::~BookmarkedNotes() { }

bool BookmarkedNotes::toggleBookmark(QObject *object)
{
    if (this->indexOf(object) >= 0)
        return this->removeFromBookmark(object);

    return this->addToBookmark(object);
}

bool BookmarkedNotes::addToBookmark(QObject *object)
{
    if (object == nullptr)
        return false;

    const QMetaObject *mo = object->metaObject();
    if (mo->inherits(&Notes::staticMetaObject)) {
        Notes *notes = qobject_cast<Notes *>(object);
        connect(notes, &Notes::aboutToDelete, this, &BookmarkedNotes::notesDestroyed);
        connect(notes, &Notes::notesModified, this, &BookmarkedNotes::notesUpdated);
        this->append(notes);
        this->sync();
        return true;
    }

    if (mo->inherits(&Note::staticMetaObject)) {
        Note *note = qobject_cast<Note *>(object);
        connect(note, &Note::aboutToDelete, this, &BookmarkedNotes::noteDestroyed);
        connect(note, &Note::noteModified, this, &BookmarkedNotes::noteUpdated);
        this->append(note);
        this->sync();
        return true;
    }

    if (mo->inherits(&Character::staticMetaObject)) {
        Character *character = qobject_cast<Character *>(object);
        connect(character, &Character::aboutToDelete, this, &BookmarkedNotes::characterDestroyed);
        connect(character, &Character::characterChanged, this, &BookmarkedNotes::characterUpdated);
        this->append(character);
        this->sync();
        return true;
    }

    return false;
}

bool BookmarkedNotes::removeFromBookmark(QObject *object)
{
    if (object == nullptr)
        return false;

    const int row = this->indexOf(object);
    if (row < 0)
        return false;

    const QMetaObject *mo = object->metaObject();
    if (mo->inherits(&Notes::staticMetaObject)) {
        Notes *notes = qobject_cast<Notes *>(object);
        disconnect(notes, &Notes::aboutToDelete, this, &BookmarkedNotes::notesDestroyed);
        disconnect(notes, &Notes::notesModified, this, &BookmarkedNotes::notesUpdated);
        this->removeAt(row);
        this->sync();
        return true;
    }

    if (mo->inherits(&Note::staticMetaObject)) {
        Note *note = qobject_cast<Note *>(object);
        disconnect(note, &Note::aboutToDelete, this, &BookmarkedNotes::noteDestroyed);
        disconnect(note, &Note::noteModified, this, &BookmarkedNotes::noteUpdated);
        this->removeAt(row);
        this->sync();
        return true;
    }

    if (mo->inherits(&Character::staticMetaObject)) {
        Character *character = qobject_cast<Character *>(object);
        disconnect(character, &Character::aboutToDelete, this,
                   &BookmarkedNotes::characterDestroyed);
        disconnect(character, &Character::characterChanged, this,
                   &BookmarkedNotes::characterUpdated);
        this->removeAt(row);
        this->sync();
        return true;
    }

    return false;
}

bool BookmarkedNotes::isBookmarked(QObject *object) const
{
    return this->indexOf(object) >= 0;
}

QHash<int, QByteArray> BookmarkedNotes::roleNames() const
{
    return { { TitleRole, QByteArrayLiteral("noteTitle") },
             { SummaryRole, QByteArrayLiteral("noteSummary") },
             { ObjectRole, QByteArrayLiteral("noteObject") } };
}

QVariant BookmarkedNotes::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        QObject *object = this->objectAt(index.row());
        return this->data(object, role);
    }

    return QVariant();
}

QVariant BookmarkedNotes::data(QObject *ptr, int role) const
{
    if (ptr == nullptr)
        return QVariant();

    switch (role) {
    case TitleRole: {
        if (ptr->metaObject()->inherits(&Notes::staticMetaObject)) {
            Notes *notes = qobject_cast<Notes *>(ptr);
            return notes->title();
        } else if (ptr->metaObject()->inherits(&Note::staticMetaObject)) {
            Note *note = qobject_cast<Note *>(ptr);
            return note->title();
        } else if (ptr->metaObject()->inherits(&Character::staticMetaObject)) {
            Character *character = qobject_cast<Character *>(ptr);
            return character->name();
        }
    } break;
    case SummaryRole: {
        if (ptr->metaObject()->inherits(&Notes::staticMetaObject)) {
            Notes *notes = qobject_cast<Notes *>(ptr);
            return notes->summary();
        } else if (ptr->metaObject()->inherits(&Note::staticMetaObject)) {
            Note *note = qobject_cast<Note *>(ptr);
            return note->summary();
        } else if (ptr->metaObject()->inherits(&Character::staticMetaObject)) {
            Character *character = qobject_cast<Character *>(ptr);
            return QStringList({ character->designation(), character->gender(), character->age() })
                    .join(QStringLiteral(", "));
        }
    } break;
    case ObjectRole:
        return QVariant::fromValue<QObject *>(ptr);
    }

    return QVariant();
}

void BookmarkedNotes::noteUpdated()
{
    QObject *ptr = this->sender();
    this->objectUpdated(ptr);
}

void BookmarkedNotes::notesUpdated()
{
    QObject *ptr = this->sender();
    this->objectUpdated(ptr);
}

void BookmarkedNotes::characterUpdated()
{
    QObject *ptr = this->sender();
    this->objectUpdated(ptr);
}

void BookmarkedNotes::objectUpdated(QObject *ptr)
{
    const int row = this->indexOf(ptr);
    const QModelIndex index = this->index(row, 0, QModelIndex());
    emit dataChanged(index, index);
}

void BookmarkedNotes::noteDestroyed(Note *ptr)
{
    this->objectDestroyed(ptr);
    this->sync();
}

void BookmarkedNotes::notesDestroyed(Notes *ptr)
{
    this->objectDestroyed(ptr);
    this->sync();
}

void BookmarkedNotes::characterDestroyed(Character *ptr)
{
    this->objectDestroyed(ptr);
    this->sync();
}

void BookmarkedNotes::sync()
{
    QScopedValueRollback<bool> block(m_blockReload, true);

    QJsonArray array;

    const QList<QObject *> objects = this->list();
    for (QObject *ptr : objects) {
        QJsonObject item;
        item.insert(QStringLiteral("type"), QString::fromLatin1(ptr->metaObject()->className()));
        item.insert(QStringLiteral("id"), ptr->property("id").toString());
        array.append(item);
    }

    ScriteDocument *doc = ScriteDocument::instance();
    doc->setBookmarkedNotes(array);
}

void BookmarkedNotes::reload()
{
    if (m_blockReload)
        return;

    ScriteDocument *doc = ScriteDocument::instance();
    disconnect(doc, &ScriteDocument::bookmarkedNotesChanged, this, &BookmarkedNotes::reload);

    const QJsonArray array = doc->bookmarkedNotes();
    for (const QJsonValue &value : array) {
        const QJsonObject item = value.toObject();
        const QString type = item.value(QStringLiteral("type")).toString();
        const QString id = item.value(QStringLiteral("id")).toString();

        if (type == QStringLiteral("Notes")) {
            Notes *notes = Notes::findById(id);
            this->append(notes);
        } else if (type == QStringLiteral("Note")) {
            Note *note = Note::findById(id);
            this->append(note);
        } else if (type == QStringLiteral("Character")) {
            Character *character = doc->structure()->findCharacter(id);
            this->append(character);
        }
    }

    connect(doc, &ScriteDocument::bookmarkedNotesChanged, this, &BookmarkedNotes::reload);
}
