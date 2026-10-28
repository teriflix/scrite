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

#include "notebookproxymodels.h"

#include "notes.h"
#include "scene.h"
#include "structure.h"
#include "screenplay.h"
#include "scritedocument.h"

// How long the lenses wait after the NotebookModel changes before they look at it again. The
// NotebookModel makes its scenes and characters again a short while after the document changes, in
// pieces, so this is longer than the time it waits.
static const int refreshIntervalInMs = 120;

// How many characters of a synopsis go into the label of a scene that has no heading.
static const int synopsisLimit = 60;

// A reference to something, as it is in a key: in braces, which ids of notes already have, and the
// names of breaks do not.
static QString keyOf(const QString &kind, const QString &ref)
{
    QString inner = ref.trimmed();
    if (inner.startsWith(QLatin1Char('{')) && inner.endsWith(QLatin1Char('}')))
        inner = inner.mid(1, inner.size() - 2);
    inner.remove(QLatin1Char('{'));
    inner.remove(QLatin1Char('}'));
    return kind + QStringLiteral(":{") + inner + QStringLiteral("}");
}

// What the scene is called in the screenplay: its number, or numbers if it is in the screenplay
// more than once, as the Notebook shows them. Empty if the scene is not in the screenplay.
static QString sceneNumbers(Scene *scene)
{
    Screenplay *screenplay = ScriteDocument::instance()->screenplay();

    QStringList numbers;
    const QList<int> indexes = scene->screenplayElementIndexList();
    for (int index : indexes) {
        ScreenplayElement *element = screenplay->elementAt(index);
        numbers << (element ? element->resolvedSceneNumber() : QString::number(index + 1));
    }
    numbers.removeAll(QString());
    return numbers.join(QStringLiteral(", "));
}

// "Scene 12", or "Unused scene" for one that is not in the screenplay.
static QString sceneName(Scene *scene)
{
    const QString numbers = sceneNumbers(scene);
    return numbers.isEmpty() ? QStringLiteral("Unused scene")
                             : QStringLiteral("Scene ") + numbers;
}

static QString sceneLabel(Scene *scene)
{
    const QString name = sceneName(scene);

    StructureElement *element = scene->structureElement();
    const QString title = element ? element->title() : QString();
    if (scene->heading()->isEnabled() && !title.isEmpty())
        return name + QStringLiteral(": ") + title;

    // A scene without a heading is known by what it is about.
    QString synopsis = scene->synopsis().simplified();
    if (synopsis.size() > synopsisLimit)
        synopsis = synopsis.left(synopsisLimit).trimmed() + QStringLiteral("...");
    return name + QStringLiteral(" (no heading)")
            + (synopsis.isEmpty() ? QString() : QStringLiteral(": ") + synopsis);
}

// The title of an episode or act, with its subtitle if it has one.
static QString breakLabel(ScreenplayElement *element)
{
    const QString title = element->breakTitle().trimmed();
    const QString subtitle = element->breakSubtitle().trimmed();
    return subtitle.isEmpty() ? title : title + QStringLiteral(": ") + subtitle;
}

// The title of the episode that the row at index, in the NotebookModel, is in, or an empty string
// if it is in none, or in one that the NotebookModel made up, which has no title to go by.
static QString episodeTitleOf(const QModelIndex &index)
{
    for (QModelIndex parent = index.parent(); parent.isValid(); parent = parent.parent()) {
        if (parent.data(NotebookModel::TypeRole).toInt() != NotebookModel::EpisodeBreakType)
            continue;

        ScreenplayElement *episode = qobject_cast<ScreenplayElement *>(
                parent.data(NotebookModel::ObjectRole).value<QObject *>());
        return episode ? episode->breakTitle().trimmed() : QString();
    }

    return QString();
}

// What the item that has the notes is called, for a note of it at index in the NotebookModel, so that
// notes with the same title are told apart: the scene with its heading, the character, the act or
// episode as the tools of LLMHub name it, which is with the episode in front of an act that is in
// one. The notes of the story are the story's, which is the whole document, so they have no owner
// to tell them apart by.
static QString ownerOf(Notes *notes, const QModelIndex &index)
{
    switch (notes->ownerType()) {
    case Notes::SceneOwner:
        return sceneLabel(notes->scene());
    case Notes::CharacterOwner:
        return notes->character()->name();
    case Notes::BreakOwner: {
        ScreenplayElement *element = notes->breakElement();
        if (element == nullptr)
            return QString();

        const QString title = element->breakTitle().trimmed();
        const QString episode =
                element->breakType() == Screenplay::Act ? episodeTitleOf(index) : QString();
        return episode.isEmpty() ? title : episode + QStringLiteral(", ") + title;
    }
    default:
        break;
    }

    return QString();
}

// Which of the types of notes the note is. A text note is rich text unless it prefers a plain text
// UI, and then it is markdown text if it has markdown content.
static NotebookItemFilter::NoteTypes noteTypeOf(Note *note)
{
    switch (note->type()) {
    case Note::FormNoteType:
        return NotebookItemFilter::FormNote;
    case Note::CheckListNoteType:
        return NotebookItemFilter::ChecklistNote;
    default:
        break;
    }

    if (!note->isPreferPlainTextUi())
        return NotebookItemFilter::RichTextNote;

    return note->hasMarkdownContent() ? NotebookItemFilter::MarkdownTextNote
                                      : NotebookItemFilter::PlainTextNote;
}

static NotebookItemFilter::Kind kindOf(const NotebookItemInfo &info)
{
    if (info.kind == QStringLiteral("note"))
        return NotebookItemFilter::NoteKind;
    if (info.kind == QStringLiteral("scene"))
        return NotebookItemFilter::SceneKind;
    if (info.kind == QStringLiteral("character"))
        return NotebookItemFilter::CharacterKind;
    return NotebookItemFilter::BreakKind;
}

// Whether an item is of one of the kinds, and, if it is a note, is of one of the types of notes.
static bool isAccepted(const NotebookItemInfo &info, NotebookItemFilter::Kinds kinds,
                       NotebookItemFilter::NoteTypes noteTypes)
{
    if (!info.isValid() || !(kinds & kindOf(info)))
        return false;

    return info.kind != QStringLiteral("note") || (info.noteType & noteTypes);
}

// What the row at index, in the NotebookModel, is as an item. Invalid if it isn't one.
static NotebookItemInfo infoOf(const QModelIndex &index)
{
    NotebookItemInfo info;

    QObject *object = index.data(NotebookModel::ObjectRole).value<QObject *>();
    if (object == nullptr)
        return info;

    switch (index.data(NotebookModel::TypeRole).toInt()) {
    case NotebookModel::NoteType: {
        Note *note = qobject_cast<Note *>(object);
        if (note == nullptr)
            return info;

        info.kind = QStringLiteral("note");
        info.noteType = noteTypeOf(note);
        info.key = keyOf(info.kind, note->id());
        // Notes are called by their owner and title, as in "ACT 1 / AI Act Summary", because the
        // titles of notes are the same in many places.
        const QString title = note->title().isEmpty() ? QStringLiteral("New Note") : note->title();
        info.owner = ownerOf(note->notes(), index);
        info.label = info.owner.isEmpty() ? title : info.owner + QStringLiteral(" / ") + title;
    } break;
    case NotebookModel::NotesType: {
        Notes *notes = qobject_cast<Notes *>(object);
        if (notes == nullptr)
            return info;

        if (notes->ownerType() == Notes::SceneOwner && notes->scene() != nullptr) {
            Scene *scene = notes->scene();
            info.kind = QStringLiteral("scene");
            info.key = keyOf(info.kind, scene->property("id").toString());
            info.label = sceneLabel(scene);
        } else if (notes->ownerType() == Notes::CharacterOwner && notes->character() != nullptr) {
            Character *character = notes->character();
            info.kind = QStringLiteral("character");
            info.key = keyOf(info.kind, character->property("id").toString());
            info.label = QStringLiteral("Character ") + character->name();
        } else {
            return info;
        }
    } break;
    case NotebookModel::EpisodeBreakType:
    case NotebookModel::ActBreakType: {
        ScreenplayElement *element = qobject_cast<ScreenplayElement *>(object);
        if (element == nullptr)
            return info;

        // The tools of LLMHub name an act by its title, with that of its episode in front of it.
        const bool isAct = index.data(NotebookModel::TypeRole).toInt() == NotebookModel::ActBreakType;
        const QString episode = isAct ? episodeTitleOf(index) : QString();
        const QString title = element->breakTitle().trimmed();

        info.kind = QStringLiteral("break");
        info.key = keyOf(info.kind, episode.isEmpty() ? title : episode + QStringLiteral(", ") + title);
        info.label = breakLabel(element);
        info.owner = episode;
    } break;
    default:
        return info;
    }

    info.object = object;
    info.text = (info.kind + QLatin1Char(' ') + info.label + QLatin1Char(' ') + info.owner).toLower();
    return info;
}

static QStringList wordsOf(const QString &text)
{
    return text.simplified().toLower().split(QLatin1Char(' '), Qt::SkipEmptyParts);
}

static bool matches(const NotebookItemInfo &info, const QStringList &words)
{
    for (const QString &word : words) {
        if (!info.text.contains(word))
            return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

NotebookFlatLens::NotebookFlatLens(QObject *parent) : QAbstractListModel(parent)
{
    m_rebuildTimer.setSingleShot(true);
    m_rebuildTimer.setInterval(refreshIntervalInMs);
    connect(&m_rebuildTimer, &QTimer::timeout, this, &NotebookFlatLens::rebuild);
}

NotebookFlatLens::~NotebookFlatLens() { }

void NotebookFlatLens::setSourceModel(NotebookModel *val)
{
    if (m_sourceModel == val)
        return;

    if (m_sourceModel != nullptr)
        disconnect(m_sourceModel.data(), nullptr, &m_rebuildTimer, nullptr);

    m_sourceModel = val;

    if (m_sourceModel != nullptr) {
        auto restart = QOverload<>::of(&QTimer::start);
        connect(m_sourceModel, &QAbstractItemModel::rowsInserted, &m_rebuildTimer, restart);
        connect(m_sourceModel, &QAbstractItemModel::rowsRemoved, &m_rebuildTimer, restart);
        connect(m_sourceModel, &QAbstractItemModel::modelReset, &m_rebuildTimer, restart);
        connect(m_sourceModel, &QAbstractItemModel::layoutChanged, &m_rebuildTimer, restart);
        connect(m_sourceModel, &QAbstractItemModel::dataChanged, &m_rebuildTimer, restart);
    }

    this->rebuild();
    emit sourceModelChanged();
}

void NotebookFlatLens::setKinds(NotebookItemFilter::Kinds val)
{
    if (m_kinds == val)
        return;

    m_kinds = val;
    this->rebuild();
    emit kindsChanged();
}

void NotebookFlatLens::setNoteTypes(NotebookItemFilter::NoteTypes val)
{
    if (m_noteTypes == val)
        return;

    m_noteTypes = val;
    this->rebuild();
    emit noteTypesChanged();
}

void NotebookFlatLens::setSearchText(const QString &val)
{
    if (m_searchText == val)
        return;

    m_searchText = val;

    this->beginResetModel();
    this->refilter();
    this->endResetModel();

    emit searchTextChanged();
    emit labelsChanged();
    emit countChanged();
}

QString NotebookFlatLens::keyOf(const QString &label) const
{
    const int row = int(m_labels.indexOf(label));
    return row < 0 ? QString() : m_items.at(m_rows.at(row)).key;
}

QString NotebookFlatLens::labelOf(const QString &key) const
{
    for (const NotebookItemInfo &item : m_items) {
        if (item.key == key)
            return item.label;
    }

    return QString();
}

int NotebookFlatLens::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : int(m_rows.size());
}

QVariant NotebookFlatLens::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size())
        return QVariant();

    const NotebookItemInfo &item = m_items.at(m_rows.at(index.row()));
    switch (role) {
    case Qt::DisplayRole:
    case ItemLabelRole:
        return item.label;
    case ItemKeyRole:
        return item.key;
    case ItemKindRole:
        return item.kind;
    case ItemOwnerRole:
        return item.owner;
    case ItemObjectRole:
        return QVariant::fromValue<QObject *>(item.object.data());
    }

    return QVariant();
}

QHash<int, QByteArray> NotebookFlatLens::roleNames() const
{
    return { { ItemKeyRole, QByteArrayLiteral("itemKey") },
             { ItemKindRole, QByteArrayLiteral("itemKind") },
             { ItemLabelRole, QByteArrayLiteral("itemLabel") },
             { ItemOwnerRole, QByteArrayLiteral("itemOwner") },
             { ItemObjectRole, QByteArrayLiteral("itemObject") } };
}

void NotebookFlatLens::rebuild()
{
    m_rebuildTimer.stop();

    this->beginResetModel();
    m_items.clear();
    if (m_sourceModel != nullptr)
        this->collect(QModelIndex());
    this->refilter();
    this->endResetModel();

    emit labelsChanged();
    emit countChanged();
}

void NotebookFlatLens::refilter()
{
    const QStringList words = wordsOf(m_searchText);

    m_rows.clear();
    m_labels.clear();
    for (int i = 0; i < m_items.size(); i++) {
        if (!matches(m_items.at(i), words))
            continue;

        // Two items can have the same label, which a number tells apart.
        const QString &label = m_items.at(i).label;
        QString unique = label;
        for (int n = 2; m_labels.contains(unique); n++)
            unique = label + QStringLiteral(" (") + QString::number(n) + QStringLiteral(")");

        m_rows.append(i);
        m_labels.append(unique);
    }
}

void NotebookFlatLens::collect(const QModelIndex &parent)
{
    const int nrRows = m_sourceModel->rowCount(parent);
    for (int row = 0; row < nrRows; row++) {
        const QModelIndex index = m_sourceModel->index(row, 0, parent);

        const NotebookItemInfo info = infoOf(index);
        if (isAccepted(info, m_kinds, m_noteTypes))
            m_items.append(info);

        this->collect(index);
    }
}

///////////////////////////////////////////////////////////////////////////////

NotebookCheckedItems::NotebookCheckedItems(QObject *parent) : QObjectListModel<QObject *>(parent)
{
    m_notebook = new NotebookModel(this);
    m_notebook->setDocument(ScriteDocument::instance());

    m_refreshTimer.setSingleShot(true);
    m_refreshTimer.setInterval(refreshIntervalInMs);
    connect(&m_refreshTimer, &QTimer::timeout, this, &NotebookCheckedItems::refresh);

    auto restart = QOverload<>::of(&QTimer::start);
    connect(m_notebook, &QAbstractItemModel::rowsInserted, &m_refreshTimer, restart);
    connect(m_notebook, &QAbstractItemModel::rowsRemoved, &m_refreshTimer, restart);
    connect(m_notebook, &QAbstractItemModel::modelReset, &m_refreshTimer, restart);
    connect(m_notebook, &QAbstractItemModel::layoutChanged, &m_refreshTimer, restart);
    connect(m_notebook, &QAbstractItemModel::dataChanged, &m_refreshTimer, restart);
}

NotebookCheckedItems::~NotebookCheckedItems() { }

bool NotebookCheckedItems::addKey(const QString &key)
{
    if (this->indexOfKey(key) >= 0)
        return false;

    const NotebookItemInfo info = this->findInfo(key);
    if (!info.isValid())
        return false;

    this->add(info);
    return true;
}

int NotebookCheckedItems::indexOfKey(const QString &key) const
{
    for (int row = 0; row < m_list.size(); row++) {
        if (m_infos.value(m_list.at(row)).key == key)
            return row;
    }

    return -1;
}

// Adds the object of the item to the end of the list, if the item is not in it. If it is, then the
// item keeps its row, and only what it is called changes. If its object is a new one, as it is when
// the NotebookModel makes the object again, then it takes the row of the old one.
void NotebookCheckedItems::add(const NotebookItemInfo &info)
{
    QObject *object = info.object.data();
    if (object == nullptr)
        return;

    const int row = this->indexOfKey(info.key);
    if (row < 0) {
        m_infos.insert(object, info);
        this->append(object);
        return;
    }

    if (m_list.at(row) != object) {
        this->removeAt(row);
        m_infos.insert(object, info);
        this->insert(row, object);
        return;
    }

    const NotebookItemInfo old = m_infos.value(object);
    m_infos.insert(object, info);
    if (old.label != info.label || old.owner != info.owner) {
        const QModelIndex index = this->index(row, 0, QModelIndex());
        emit dataChanged(index, index);
    }
}

QHash<int, QByteArray> NotebookCheckedItems::roleNames() const
{
    QHash<int, QByteArray> roles = QObjectListModel<QObject *>::roleNames();
    roles.insert(ItemKeyRole, QByteArrayLiteral("itemKey"));
    roles.insert(ItemKindRole, QByteArrayLiteral("itemKind"));
    roles.insert(ItemLabelRole, QByteArrayLiteral("itemLabel"));
    roles.insert(ItemOwnerRole, QByteArrayLiteral("itemOwner"));
    return roles;
}

QVariant NotebookCheckedItems::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case ItemKeyRole:
    case ItemKindRole:
    case ItemLabelRole:
    case ItemOwnerRole: {
        QObject *object = this->at(index.row());
        if (object == nullptr)
            return QVariant();

        const NotebookItemInfo info = m_infos.value(object);
        switch (role) {
        case ItemKeyRole:
            return info.key;
        case ItemKindRole:
            return info.kind;
        case ItemLabelRole:
            return info.label;
        default:
            return info.owner;
        }
    }
    default:
        break;
    }

    return QObjectListModel<QObject *>::data(index, role);
}

// Looks at the items again: those that are not in the Notebook any more are removed, and the rest
// are told what they are called now, and what their object is, which the NotebookModel makes again.
void NotebookCheckedItems::refresh()
{
    if (m_list.isEmpty())
        return;

    QHash<QString, NotebookItemInfo> infos;
    this->collectInfos(QModelIndex(), infos);

    for (int row = m_list.size() - 1; row >= 0; row--) {
        const QString key = this->keyAt(row);
        if (infos.contains(key))
            this->add(infos.value(key));
        else
            this->removeAt(row);
    }
}

NotebookItemInfo NotebookCheckedItems::findInfo(const QString &key, const QModelIndex &parent) const
{
    const int nrRows = m_notebook->rowCount(parent);
    for (int row = 0; row < nrRows; row++) {
        const QModelIndex index = m_notebook->index(row, 0, parent);

        const NotebookItemInfo info = infoOf(index);
        if (info.isValid() && info.key == key)
            return info;

        const NotebookItemInfo found = this->findInfo(key, index);
        if (found.isValid())
            return found;
    }

    return NotebookItemInfo();
}

void NotebookCheckedItems::collectInfos(const QModelIndex &parent,
                                        QHash<QString, NotebookItemInfo> &infos) const
{
    const int nrRows = m_notebook->rowCount(parent);
    for (int row = 0; row < nrRows; row++) {
        const QModelIndex index = m_notebook->index(row, 0, parent);

        const NotebookItemInfo info = infoOf(index);
        if (info.isValid())
            infos.insert(info.key, info);

        this->collectInfos(index, infos);
    }
}

///////////////////////////////////////////////////////////////////////////////

NotebookTreeLens::NotebookTreeLens(QObject *parent) : QSortFilterProxyModel(parent)
{
    // A row that holds an item that the search finds is kept, and so are the rows above it.
    this->setRecursiveFilteringEnabled(true);

    m_itemsTimer.setSingleShot(true);
    m_itemsTimer.setInterval(refreshIntervalInMs);
    connect(&m_itemsTimer, &QTimer::timeout, this, &NotebookTreeLens::checkedItemsChanged);
}

NotebookTreeLens::~NotebookTreeLens() { }

void NotebookTreeLens::setSearchText(const QString &val)
{
    if (m_searchText == val)
        return;

    this->beginFilterChange();
    m_searchText = val;
    m_searchWords = wordsOf(val);
    this->endFilterChange();

    emit searchTextChanged();
}

void NotebookTreeLens::setKinds(NotebookItemFilter::Kinds val)
{
    if (m_kinds == val)
        return;

    m_kinds = val;
    emit kindsChanged();
}

void NotebookTreeLens::setNoteTypes(NotebookItemFilter::NoteTypes val)
{
    if (m_noteTypes == val)
        return;

    this->beginFilterChange();
    m_noteTypes = val;
    this->endFilterChange();

    emit noteTypesChanged();
}

void NotebookTreeLens::setCheckedKeys(const QStringList &val)
{
    if (m_checkedKeys == val)
        return;

    const bool wasFull = this->isFull();

    m_checkedKeys = val;
    this->announceCheckedChanged();

    emit checkedKeysChanged();
    emit checkedItemsChanged();

    if (wasFull != this->isFull())
        emit fullChanged();
}

void NotebookTreeLens::setMaxChecked(int val)
{
    val = qMax(0, val);
    if (m_maxChecked == val)
        return;

    const bool wasFull = this->isFull();

    m_maxChecked = val;
    emit maxCheckedChanged();

    if (wasFull != this->isFull())
        emit fullChanged();
}

QVariantList NotebookTreeLens::checkedItems() const
{
    QVariantList ret;
    if (this->sourceModel() != nullptr && !m_checkedKeys.isEmpty())
        this->collectChecked(QModelIndex(), ret);
    return ret;
}

void NotebookTreeLens::clearChecked()
{
    this->setCheckedKeys(QStringList());
}

void NotebookTreeLens::setItemChecked(const QModelIndex &index, bool checked)
{
    this->setData(index, checked, ItemCheckedRole);
}

QHash<int, QByteArray> NotebookTreeLens::roleNames() const
{
    QHash<int, QByteArray> roles = QSortFilterProxyModel::roleNames();
    roles.insert(ItemKeyRole, QByteArrayLiteral("itemKey"));
    roles.insert(ItemKindRole, QByteArrayLiteral("itemKind"));
    roles.insert(ItemLabelRole, QByteArrayLiteral("itemLabel"));
    roles.insert(ItemOwnerRole, QByteArrayLiteral("itemOwner"));
    roles.insert(ItemCheckableRole, QByteArrayLiteral("itemCheckable"));
    roles.insert(ItemCheckedRole, QByteArrayLiteral("itemChecked"));
    return roles;
}

QVariant NotebookTreeLens::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case ItemKeyRole:
    case ItemKindRole:
    case ItemLabelRole:
    case ItemOwnerRole:
    case ItemCheckableRole:
    case ItemCheckedRole: {
        const NotebookItemInfo info = infoOf(this->mapToSource(index));
        switch (role) {
        case ItemKeyRole:
            return info.key;
        case ItemKindRole:
            return info.kind;
        case ItemLabelRole:
            return info.label;
        case ItemOwnerRole:
            return info.owner;
        case ItemCheckableRole:
            return this->isCheckable(info);
        default:
            return info.isValid() && m_checkedKeys.contains(info.key);
        }
    }
    default:
        break;
    }

    return QSortFilterProxyModel::data(index, role);
}

bool NotebookTreeLens::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != ItemCheckedRole)
        return QSortFilterProxyModel::setData(index, value, role);

    const NotebookItemInfo info = infoOf(this->mapToSource(index));
    if (!this->isCheckable(info))
        return false;

    this->setChecked(info.key, value.toBool());
    return true;
}

// Whether an item can be ticked: it is one, and is of one of the kinds, and, if it
// is a note, of one of the types of notes, which is so for all the notes that are in the tree.
bool NotebookTreeLens::isCheckable(const NotebookItemInfo &info) const
{
    return isAccepted(info, m_kinds, m_noteTypes);
}

void NotebookTreeLens::setSourceModel(QAbstractItemModel *model)
{
    if (this->sourceModel() != nullptr)
        disconnect(this->sourceModel(), nullptr, &m_itemsTimer, nullptr);

    QSortFilterProxyModel::setSourceModel(model);

    // The label of a ticked item may change when the NotebookModel does, or it may be gone.
    if (model != nullptr) {
        auto restart = QOverload<>::of(&QTimer::start);
        connect(model, &QAbstractItemModel::rowsInserted, &m_itemsTimer, restart);
        connect(model, &QAbstractItemModel::rowsRemoved, &m_itemsTimer, restart);
        connect(model, &QAbstractItemModel::modelReset, &m_itemsTimer, restart);
        connect(model, &QAbstractItemModel::dataChanged, &m_itemsTimer, restart);
    }
}

bool NotebookTreeLens::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const QModelIndex index = this->sourceModel()->index(sourceRow, 0, sourceParent);

    // Bookmarks is a row with nothing under it.
    if (index.data(NotebookModel::TypeRole).toInt() == NotebookModel::CategoryType
        && index.data(NotebookModel::CategoryRole).toInt() == NotebookModel::BookmarksCategory)
        return false;

    // A note that is not of one of the types of notes is not in the tree at all.
    if (m_noteTypes != NotebookItemFilter::AnyNote) {
        const NotebookItemInfo note = infoOf(index);
        if (note.kind == QStringLiteral("note") && !(note.noteType & m_noteTypes))
            return false;
    }

    if (m_searchWords.isEmpty())
        return true;

    // The rows that hold items found are kept, and with them, the rows above, see the constructor.
    const NotebookItemInfo info = infoOf(index);
    return info.isValid() && matches(info, m_searchWords);
}

void NotebookTreeLens::collectChecked(const QModelIndex &parent, QVariantList &items) const
{
    const QAbstractItemModel *source = this->sourceModel();

    const int nrRows = source->rowCount(parent);
    for (int row = 0; row < nrRows; row++) {
        const QModelIndex index = source->index(row, 0, parent);

        const NotebookItemInfo info = infoOf(index);
        if (info.isValid() && m_checkedKeys.contains(info.key)) {
            items.append(QVariantMap { { QStringLiteral("key"), info.key },
                                       { QStringLiteral("kind"), info.kind },
                                       { QStringLiteral("label"), info.label },
                                       { QStringLiteral("owner"), info.owner } });
        }

        this->collectChecked(index, items);
    }
}

// Tells the views that the ticks have changed, in all the rows that they have.
void NotebookTreeLens::announceCheckedChanged(const QModelIndex &parent)
{
    const int nrRows = this->rowCount(parent);
    if (nrRows == 0)
        return;

    emit dataChanged(this->index(0, 0, parent), this->index(nrRows - 1, 0, parent),
                     { ItemCheckedRole });

    for (int row = 0; row < nrRows; row++)
        this->announceCheckedChanged(this->index(row, 0, parent));
}

void NotebookTreeLens::setChecked(const QString &key, bool checked)
{
    if (checked == m_checkedKeys.contains(key))
        return;

    // One more is not ticked when there is no room for it, but one can always be unticked.
    if (checked && this->isFull())
        return;

    QStringList keys = m_checkedKeys;
    if (checked)
        keys.append(key);
    else
        keys.removeAll(key);

    this->setCheckedKeys(keys);
}
