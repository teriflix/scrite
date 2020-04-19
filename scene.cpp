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

#include "scene.h"
#include "undoredo.h"
#include "searchengine.h"
#include "garbagecollector.h"

#include <QUuid>
#include <QJsonArray>
#include <QJsonObject>
#include <QTextDocument>

SceneHeading::SceneHeading(QObject *parent)
    : QObject(parent),
      m_scene(qobject_cast<Scene*>(parent))
{
    connect(this, &SceneHeading::momentChanged, this, &SceneHeading::textChanged);
    connect(this, &SceneHeading::enabledChanged, this, &SceneHeading::textChanged);
    connect(this, &SceneHeading::locationChanged, this, &SceneHeading::textChanged);
    connect(this, &SceneHeading::locationTypeChanged, this, &SceneHeading::textChanged);
}

SceneHeading::~SceneHeading()
{

}

void SceneHeading::setEnabled(bool val)
{
    if(m_enabled == val)
        return;

    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "enabled");
    QScopedPointer<PushObjectPropertyUndoCommand> cmd;
    if(info->isLocked())
    {
        // This happens when a scene element text is reset because of a
        // undo or redo command. It is best if we allow SceneDocumentBinder
        // an opportunity to know about this in advance.
        emit m_scene->sceneAboutToReset();
    }
    else
        cmd.reset(new PushObjectPropertyUndoCommand(this, info->property));

    m_enabled = val;
    emit enabledChanged();

    if(info->isLocked())
    {
        // This happens when a scene element text is reset because of a
        // undo or redo command. It is best if we allow SceneDocumentBinder
        // an opportunity to know about this in advance.
        emit m_scene->sceneReset(-1);
    }
}

void SceneHeading::setLocationType(const QString &val2)
{
    const QString val = val2.toUpper().trimmed();
    if(m_locationType == val)
        return;

    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "locationType");
    QScopedPointer<PushObjectPropertyUndoCommand> cmd;
    if(info->isLocked())
    {
        // This happens when a scene element text is reset because of a
        // undo or redo command. It is best if we allow SceneDocumentBinder
        // an opportunity to know about this in advance.
        emit m_scene->sceneAboutToReset();
    }
    else
        cmd.reset(new PushObjectPropertyUndoCommand(this, info->property));

    m_locationType = val;
    emit locationTypeChanged();

    if(info->isLocked())
    {
        // This happens when a scene element text is reset because of a
        // undo or redo command. It is best if we allow SceneDocumentBinder
        // an opportunity to know about this in advance.
        emit m_scene->sceneReset(-1);
    }
}

void SceneHeading::setLocation(const QString &val2)
{
    const QString val = val2.toUpper().trimmed();
    if(m_location == val)
        return;

    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "location");
    QScopedPointer<PushObjectPropertyUndoCommand> cmd;
    if(info->isLocked())
    {
        // This happens when a scene element text is reset because of a
        // undo or redo command. It is best if we allow SceneDocumentBinder
        // an opportunity to know about this in advance.
        emit m_scene->sceneAboutToReset();
    }
    else
        cmd.reset(new PushObjectPropertyUndoCommand(this, info->property));

    m_location = val;
    emit locationChanged();

    if(info->isLocked())
    {
        // This happens when a scene element text is reset because of a
        // undo or redo command. It is best if we allow SceneDocumentBinder
        // an opportunity to know about this in advance.
        emit m_scene->sceneReset(-1);
    }
}

void SceneHeading::setMoment(const QString &val2)
{
    const QString val = val2.toUpper().trimmed();
    if(m_moment == val)
        return;

    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "moment");
    QScopedPointer<PushObjectPropertyUndoCommand> cmd;
    if(info->isLocked())
    {
        // This happens when a scene element text is reset because of a
        // undo or redo command. It is best if we allow SceneDocumentBinder
        // an opportunity to know about this in advance.
        emit m_scene->sceneAboutToReset();
    }
    else
        cmd.reset(new PushObjectPropertyUndoCommand(this, info->property));

    m_moment = val;
    emit momentChanged();

    if(info->isLocked())
    {
        // This happens when a scene element text is reset because of a
        // undo or redo command. It is best if we allow SceneDocumentBinder
        // an opportunity to know about this in advance.
        emit m_scene->sceneReset(-1);
    }
}

QString SceneHeading::text() const
{
    return m_locationType + ". " + m_location + " - " + m_moment;
}

///////////////////////////////////////////////////////////////////////////////

SceneElement::SceneElement(QObject *parent)
    : QObject(parent),
      m_scene(qobject_cast<Scene*>(parent))
{
    connect(this, &SceneElement::typeChanged, this, &SceneElement::elementChanged);
    connect(this, &SceneElement::textChanged, this, &SceneElement::elementChanged);

    ObjectPropertyInfo::querySetCounter(this, "type");
    ObjectPropertyInfo::querySetCounter(this, "text");
}

SceneElement::~SceneElement()
{
    emit aboutToDelete(this);
}

void SceneElement::setType(SceneElement::Type val)
{
    if(m_type == val)
        return;

    ObjectPropertyInfo *info = m_scene->isUndoRedoEnabled() ? ObjectPropertyInfo::get(this, "type") : nullptr;
    QScopedPointer<PushObjectPropertyUndoCommand> cmd;
    if(info == nullptr || info->isLocked())
    {
        // This happens when a scene element text is reset because of a
        // undo or redo command. It is best if we allow SceneDocumentBinder
        // an opportunity to know about this in advance.
        emit m_scene->sceneAboutToReset();
    }
    else
        cmd.reset(new PushObjectPropertyUndoCommand(this, info->property));

    m_type = val;
    emit typeChanged();

    if(m_scene != nullptr)
        emit m_scene->sceneElementChanged(this, Scene::ElementTypeChange);

    if(info == nullptr || info->isLocked())
    {
        // This happens when a scene element text is reset because of a
        // undo or redo command. It is best if we allow SceneDocumentBinder
        // an opportunity to know about this in advance.
        emit m_scene->sceneReset(m_scene->indexOfElement(this));
    }
}

QString SceneElement::typeAsString() const
{
    switch(m_type)
    {
    case Action: return "Action";
    case Character: return "Character";
    case Dialogue: return "Dialogue";
    case Parenthetical: return "Parenthetical";
    case Shot: return "Shot";
    case Transition: return "Transition";
    case Heading: return "Scene Heading";
    }

    return "Unknown";
}

void SceneElement::setText(const QString &val)
{
    if(m_text == val)
        return;

    ObjectPropertyInfo *info = m_scene->isUndoRedoEnabled() ? ObjectPropertyInfo::get(this, "text") : nullptr;
    QScopedPointer<PushObjectPropertyUndoCommand> cmd;
    if(info == nullptr || info->isLocked())
    {
        // This happens when a scene element text is reset because of a
        // undo or redo command. It is best if we allow SceneDocumentBinder
        // an opportunity to know about this in advance.
        emit m_scene->sceneAboutToReset();
    }
    else
        cmd.reset(new PushObjectPropertyUndoCommand(this, info->property));        

    m_text = val.trimmed();
    emit textChanged();

    if(m_scene != nullptr)
        emit m_scene->sceneElementChanged(this, Scene::ElementTextChange);

    if(info == nullptr || info->isLocked())
    {
        // This happens when a scene element text is reset because of a
        // undo or redo command. It is best if we allow SceneDocumentBinder
        // an opportunity to know about this in advance.
        emit m_scene->sceneReset(m_scene->indexOfElement(this));
    }
}

void SceneElement::setCursorPosition(int val)
{
    m_scene->setCursorPosition(val);
}

int SceneElement::cursorPosition() const
{
    return m_scene->cursorPosition();
}

QString SceneElement::formattedText() const
{
    if(m_type == SceneElement::Parenthetical)
    {
        QString text = m_text;
        if(!text.startsWith("("))
            text.prepend("(");
        if(!text.endsWith(")"))
            text.append(")");
        return text;
    }

    switch(m_type)
    {
    case SceneElement::Character:
    case SceneElement::Shot:
    case SceneElement::Transition:
    case SceneElement::Heading:
        return m_text.toUpper();
    default:
        break;
    }

    return m_text;
}

QJsonArray SceneElement::find(const QString &text, int flags) const
{
    return SearchEngine::indexesOf(text, m_text, flags);
}

bool SceneElement::event(QEvent *event)
{
    if(event->type() == QEvent::ParentChange)
        m_scene = qobject_cast<Scene*>(this->parent());

    return QObject::event(event);
}

///////////////////////////////////////////////////////////////////////////////

Scene::Scene(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &Scene::titleChanged, this, &Scene::sceneChanged);
    connect(this, &Scene::colorChanged, this, &Scene::sceneChanged);
    connect(this, &Scene::noteCountChanged, this, &Scene::sceneChanged);
    connect(this, &Scene::headingChanged, this, &Scene::sceneChanged);
    connect(this, &Scene::elementCountChanged, this, &Scene::sceneChanged);
    connect(m_heading, &SceneHeading::textChanged, this, &Scene::headingChanged);
}

Scene::~Scene()
{
    emit aboutToDelete(this);
}

void Scene::setId(const QString &val)
{
    if(m_id == val || !m_id.isEmpty())
        return;

    m_id = val;
    emit idChanged();
}

QString Scene::id() const
{
    if(m_id.isEmpty())
        m_id = QUuid::createUuid().toString();

    return m_id;
}

QString Scene::name() const
{
    if(m_title.length() > 15)
        return QString("Scene: %1...").arg(m_title.left(13));

    return QString("Scene: %1").arg(m_title);
}

void Scene::setTitle(const QString &val)
{
    if(m_title == val)
        return;

    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "title");
    QScopedPointer<PushObjectPropertyUndoCommand> cmd;
    if(!info->isLocked())
        cmd.reset(new PushObjectPropertyUndoCommand(this, info->property));

    m_title = val;
    emit titleChanged();
}

void Scene::setColor(const QColor &val)
{
    if(m_color == val)
        return;

    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "color");
    QScopedPointer<PushObjectPropertyUndoCommand> cmd;
    if(!info->isLocked())
        cmd.reset(new PushObjectPropertyUndoCommand(this, info->property));

    m_color = val;
    emit colorChanged();
}

void Scene::setEnabled(bool val)
{
    if(m_enabled == val)
        return;

    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "enabled");
    QScopedPointer<PushObjectPropertyUndoCommand> cmd;
    if(!info->isLocked())
        cmd.reset(new PushObjectPropertyUndoCommand(this, info->property));

    m_enabled = val;
    emit enabledChanged();
}

void Scene::setUndoRedoEnabled(bool val)
{
    if(m_undoRedoEnabled == val)
        return;

    m_undoRedoEnabled = val;
    emit undoRedoEnabledChanged();
}

void Scene::setCursorPosition(int val)
{
    if(m_cursorPosition == val)
        return;

    m_cursorPosition = val;
    emit cursorPositionChanged();
}

QQmlListProperty<SceneElement> Scene::elements()
{
    return QQmlListProperty<SceneElement>(
                reinterpret_cast<QObject*>(this),
                static_cast<void*>(this),
                &Scene::staticAppendElement,
                &Scene::staticElementCount,
                &Scene::staticElementAt,
                &Scene::staticClearElements);
}

void Scene::addElement(SceneElement *ptr)
{
    this->insertElementAt(ptr, m_elements.size());
}

void Scene::insertElementAfter(SceneElement *ptr, SceneElement *after)
{
    int index = m_elements.indexOf(after);
    if(index < 0)
        return;

    this->insertElementAt(ptr, index+1);
}

void Scene::insertElementBefore(SceneElement *ptr, SceneElement *before)
{
    int index = m_elements.indexOf(before);
    if(index < 0)
        return;

    this->insertElementAt(ptr, index);
}

static void sceneAppendElement(Scene *scene, SceneElement *ptr) { scene->addElement(ptr); }
static void sceneRemoveElement(Scene *scene, SceneElement *ptr) { scene->removeElement(ptr); }
static void sceneInsertElement(Scene *scene, SceneElement *ptr, int index) { scene->insertElementAt(ptr, index); }
static SceneElement *sceneElementAt(Scene *scene, int index) { return scene->elementAt(index); }
static int sceneIndexOfElement(Scene *scene, SceneElement *ptr) { return scene->indexOfElement(ptr); }

void Scene::insertElementAt(SceneElement *ptr, int index)
{
    if(ptr == nullptr || m_elements.indexOf(ptr) >= 0)
        return;

    if(index < 0 || index > m_elements.size())
        return;

    this->beginInsertRows(QModelIndex(), index, index);

    m_elements.insert(index, ptr);
    connect(ptr, &SceneElement::elementChanged, this, &Scene::sceneChanged);
    connect(ptr, &SceneElement::aboutToDelete, this, &Scene::removeElement);
    connect(this, &Scene::cursorPositionChanged, ptr, &SceneElement::cursorPositionChanged);

    // START HERE
    // 3. sceneAboutToReset() and sceneReset() to be used to reload documents from scene in binder.

    QScopedPointer< PushObjectListCommand<Scene,SceneElement> > cmd;
    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "elements");
    if(info->isLocked())
    {
        // This happens when a scene element text is reset because of a
        // undo or redo command. It is best if we allow SceneDocumentBinder
        // an opportunity to know about this in advance.
        this->sceneAboutToReset();
    }
    else
    {
        ObjectListPropertyMethods<Scene,SceneElement> methods(&sceneAppendElement, &sceneRemoveElement, &sceneInsertElement, &sceneElementAt, sceneIndexOfElement);
        cmd.reset( new PushObjectListCommand<Scene,SceneElement> (ptr, this, info->property, ObjectList::InsertOperation, methods) );
    }

    this->endInsertRows();

    emit elementCountChanged();

    if(info->isLocked())
    {
        // This happens when a scene element text is reset because of a
        // undo or redo command. It is best if we allow SceneDocumentBinder
        // an opportunity to know about this in advance.
        this->sceneReset(index);
    }
}

void Scene::removeElement(SceneElement *ptr)
{
    if(ptr == nullptr)
        return;

    const int row = m_elements.indexOf(ptr);
    if(row < 0)
        return;

    QScopedPointer< PushObjectListCommand<Scene,SceneElement> > cmd;
    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "elements");
    if(info->isLocked())
    {
        // This happens when a scene element text is reset because of a
        // undo or redo command. It is best if we allow SceneDocumentBinder
        // an opportunity to know about this in advance.
        this->sceneAboutToReset();
    }
    else
    {
        ObjectListPropertyMethods<Scene,SceneElement> methods(&sceneAppendElement, &sceneRemoveElement, &sceneInsertElement, &sceneElementAt, sceneIndexOfElement);
        cmd.reset( new PushObjectListCommand<Scene,SceneElement> (ptr, this, info->property, ObjectList::RemoveOperation, methods) );
    }

    this->beginRemoveRows(QModelIndex(), row, row);

    emit aboutToRemoveSceneElement(ptr);
    m_elements.removeAt(row);

    disconnect(ptr, &SceneElement::elementChanged, this, &Scene::sceneChanged);
    disconnect(ptr, &SceneElement::aboutToDelete, this, &Scene::removeElement);
    disconnect(this, &Scene::cursorPositionChanged, ptr, &SceneElement::cursorPositionChanged);

    this->endRemoveRows();

    emit elementCountChanged();

    if(ptr->parent() == this)
        GarbageCollector::instance()->add(ptr);

    if(info->isLocked())
    {
        // This happens when a scene element text is reset because of a
        // undo or redo command. It is best if we allow SceneDocumentBinder
        // an opportunity to know about this in advance.
        this->sceneReset(qBound(0, row, m_elements.size()-1));
    }
}

SceneElement *Scene::elementAt(int index) const
{
    return index < 0 || index >= m_elements.size() ? nullptr : m_elements.at(index);
}

int Scene::elementCount() const
{
    return m_elements.size();
}

void Scene::clearElements()
{
    while(m_elements.size())
        this->removeElement(m_elements.first());
}

QQmlListProperty<Note> Scene::notes()
{
    return QQmlListProperty<Note>(
                reinterpret_cast<QObject*>(this),
                static_cast<void*>(this),
                &Scene::staticAppendNote,
                &Scene::staticNoteCount,
                &Scene::staticNoteAt,
                &Scene::staticClearNotes);
}

void Scene::addNote(Note *ptr)
{
    if(ptr == nullptr || m_notes.indexOf(ptr) >= 0)
        return;

    ptr->setParent(this);

    connect(ptr, &Note::aboutToDelete, this, &Scene::removeNote);
    connect(ptr, &Note::noteChanged, this, &Scene::sceneChanged);

    m_notes.append(ptr);
    emit noteCountChanged();
}

void Scene::removeNote(Note *ptr)
{
    if(ptr == nullptr)
        return;

    const int index = m_notes.indexOf(ptr);
    if(index < 0)
        return ;

    m_notes.removeAt(index);

    disconnect(ptr, &Note::aboutToDelete, this, &Scene::removeNote);
    disconnect(ptr, &Note::noteChanged, this, &Scene::sceneChanged);

    emit noteCountChanged();

    if(ptr->parent() == this)
        GarbageCollector::instance()->add(ptr);
}

Note *Scene::noteAt(int index) const
{
    return index < 0 || index >= m_notes.size() ? nullptr : m_notes.at(index);
}

void Scene::clearNotes()
{
    while(m_notes.size())
        this->removeNote(m_notes.first());
}

int Scene::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_elements.size();
}

QVariant Scene::data(const QModelIndex &index, int role) const
{
    if(role == SceneElementRole && index.isValid())
        return QVariant::fromValue<QObject*>(this->elementAt(index.row()));

    return QVariant();
}

QHash<int, QByteArray> Scene::roleNames() const
{
    QHash<int,QByteArray> roles;
    roles[SceneElementRole] = "sceneElement";
    return roles;
}

void Scene::setElementsList(const QList<SceneElement *> &list)
{
    Q_FOREACH(SceneElement *item, list)
    {
        if(item->scene() != this)
            return;
    }

    const bool sizeChanged = m_elements.size() != list.size();
    QList<SceneElement*> oldElements = m_elements;

    this->beginResetModel();
    m_elements.clear();
    m_elements.reserve(list.size());
    Q_FOREACH(SceneElement *item, list)
    {
        m_elements.append(item);
        oldElements.removeOne(item);
    }
    this->endResetModel();

    if(sizeChanged)
        emit elementCountChanged();

    emit sceneChanged();

    qDeleteAll(oldElements);
}

void Scene::staticAppendElement(QQmlListProperty<SceneElement> *list, SceneElement *ptr)
{
    reinterpret_cast< Scene* >(list->data)->addElement(ptr);
}

void Scene::staticClearElements(QQmlListProperty<SceneElement> *list)
{
    reinterpret_cast< Scene* >(list->data)->clearElements();
}

SceneElement *Scene::staticElementAt(QQmlListProperty<SceneElement> *list, int index)
{
    return reinterpret_cast< Scene* >(list->data)->elementAt(index);
}

int Scene::staticElementCount(QQmlListProperty<SceneElement> *list)
{
    return reinterpret_cast< Scene* >(list->data)->elementCount();
}

void Scene::staticAppendNote(QQmlListProperty<Note> *list, Note *ptr)
{
    reinterpret_cast< Scene* >(list->data)->addNote(ptr);
}

void Scene::staticClearNotes(QQmlListProperty<Note> *list)
{
    reinterpret_cast< Scene* >(list->data)->clearNotes();
}

Note *Scene::staticNoteAt(QQmlListProperty<Note> *list, int index)
{
    return reinterpret_cast< Scene* >(list->data)->noteAt(index);
}

int Scene::staticNoteCount(QQmlListProperty<Note> *list)
{
    return reinterpret_cast< Scene* >(list->data)->noteCount();
}

