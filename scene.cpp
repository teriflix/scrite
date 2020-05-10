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
#include "application.h"
#include "searchengine.h"
#include "timeprofiler.h"
#include "scritedocument.h"
#include "garbagecollector.h"
#include "qobjectserializer.h"

#include <QUuid>
#include <QDateTime>
#include <QByteArray>
#include <QJsonArray>
#include <QJsonObject>
#include <QUndoCommand>
#include <QTextDocument>
#include <QJsonDocument>
#include <QScopedValueRollback>

class PushSceneUndoCommand;
class SceneUndoCommand : public QUndoCommand
{
public:
    static SceneUndoCommand *current;

    SceneUndoCommand(Scene *scene, bool allowMerging=true);
    ~SceneUndoCommand();

    // QUndoCommand interface
    enum { ID = 100 };
    void undo();
    void redo();
    int id() const { return ID; }
    bool mergeWith(const QUndoCommand *other);

private:
    QByteArray toByteArray(Scene *scene) const;
    Scene *fromByteArray(const QByteArray &bytes) const;

private:
    friend class PushSceneUndoCommand;
    Scene *m_scene = nullptr;
    QString m_sceneId;
    QByteArray m_after;
    QByteArray m_before;
    bool m_allowMerging = true;
    char m_padding[7];
    QDateTime m_timestamp;
};

SceneUndoCommand *SceneUndoCommand::current = nullptr;

SceneUndoCommand::SceneUndoCommand(Scene *scene, bool allowMerging)
    : m_scene(scene), m_allowMerging(allowMerging),
      m_timestamp(QDateTime::currentDateTime())
{
    m_sceneId = m_scene->id();
    m_before = this->toByteArray(scene);
}

SceneUndoCommand::~SceneUndoCommand()
{

}

void SceneUndoCommand::undo()
{
    SceneUndoCommand::current = this;
    Scene *scene = this->fromByteArray(m_before);
    SceneUndoCommand::current = nullptr;

    if(scene == nullptr)
        this->setObsolete(true);
}

void SceneUndoCommand::redo()
{
    if(m_scene != nullptr)
    {
        m_after = this->toByteArray(m_scene);
        m_scene = nullptr;
        return;
    }

    SceneUndoCommand::current = this;
    Scene *scene = this->fromByteArray(m_after);
    SceneUndoCommand::current = nullptr;

    if(scene == nullptr)
        this->setObsolete(true);
}

bool SceneUndoCommand::mergeWith(const QUndoCommand *other)
{
    if(m_allowMerging && this->id() == other->id())
    {
        const SceneUndoCommand *cmd = reinterpret_cast<const SceneUndoCommand*>(other);
        if(cmd->m_allowMerging == false)
            return false;

        if(cmd->m_sceneId != m_sceneId)
            return false;

        const qint64 timegap = qAbs(m_timestamp.msecsTo(cmd->m_timestamp));
        static qint64 minTimegap = 1000;
        if(timegap < minTimegap)
        {
            m_after = cmd->m_after;
            m_timestamp = cmd->m_timestamp;
            return true;
        }
    }

    return false;
}

QByteArray SceneUndoCommand::toByteArray(Scene *scene) const
{
    return scene->toByteArray();
}

Scene *SceneUndoCommand::fromByteArray(const QByteArray &bytes) const
{
    return Scene::fromByteArray(bytes);
}

class PushSceneUndoCommand
{
    static UndoStack *allowedStack;

public:
    PushSceneUndoCommand(Scene *scene, bool allowMerging=true);
    ~PushSceneUndoCommand();

private:
    SceneUndoCommand *m_command = nullptr;
};

UndoStack *PushSceneUndoCommand::allowedStack = nullptr;

PushSceneUndoCommand::PushSceneUndoCommand(Scene *scene, bool allowMerging)
{
    if(allowedStack == nullptr)
        allowedStack = Application::instance()->findUndoStack("MainUndoStack");

    if(SceneUndoCommand::current == nullptr &&
       allowedStack != nullptr &&
       UndoStack::active() != nullptr &&
       UndoStack::active() == allowedStack &&
       scene != nullptr && scene->isUndoRedoEnabled())
    {
        if(scene != nullptr)
            m_command = new SceneUndoCommand(scene, allowMerging);
    }
}

PushSceneUndoCommand::~PushSceneUndoCommand()
{
    if(m_command != nullptr &&
        SceneUndoCommand::current == nullptr &&
        allowedStack != nullptr &&
        UndoStack::active() != nullptr &&
        UndoStack::active() == allowedStack)
        UndoStack::active()->push(m_command);
    else
        delete m_command;
}

///////////////////////////////////////////////////////////////////////////////

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

    PushSceneUndoCommand cmd(m_scene);

    m_enabled = val;
    emit enabledChanged();
}

void SceneHeading::setLocationType(const QString &val2)
{
    const QString val = val2.toUpper().trimmed();
    if(m_locationType == val)
        return;

    PushSceneUndoCommand cmd(m_scene);

    m_locationType = val;
    emit locationTypeChanged();
}

void SceneHeading::setLocation(const QString &val2)
{
    const QString val = val2.toUpper().trimmed();
    if(m_location == val)
        return;

    PushSceneUndoCommand cmd(m_scene);

    m_location = val;
    emit locationChanged();
}

void SceneHeading::setMoment(const QString &val2)
{
    const QString val = val2.toUpper().trimmed();
    if(m_moment == val)
        return;

    PushSceneUndoCommand cmd(m_scene);

    m_moment = val;
    emit momentChanged();
}

QString SceneHeading::text() const
{
    return m_locationType + ". " + m_location + " - " + m_moment;
}

void SceneHeading::parseFrom(const QString &text)
{
    if(!m_enabled || this->text() == text)
        return;

    const Structure *structure = ScriteDocument::instance()->structure();
    const QString heading = text.toUpper().trimmed();
    const int field1SepLoc = heading.indexOf('.');
    const int field2SepLoc = heading.lastIndexOf('-');

    if(field1SepLoc < 0 && field2SepLoc < 0)
    {
        if( structure->standardLocationTypes().contains(heading) )
            this->setLocationType(heading);
        else if( structure->standardMoments().contains(heading) )
            this->setMoment(heading);
        else
            this->setLocation(heading);
        return;
    }

    if(field1SepLoc < 0)
    {
        const QString moment = heading.mid(field2SepLoc+1).trimmed();
        const QString location = heading.mid(field1SepLoc+1,(field2SepLoc-field1SepLoc-1)).trimmed();

        this->setMoment(moment);
        this->setLocation(location);
        return;
    }

    if(field2SepLoc < 0)
    {
        const QString locationType = heading.left(field1SepLoc).trimmed();
        const QString location = heading.mid(field1SepLoc+1,(field2SepLoc-field1SepLoc-1)).trimmed();

        this->setLocationType(locationType);
        this->setLocation(location);
        return;
    }

    const QString locationType = heading.left(field1SepLoc).trimmed();
    const QString moment = heading.mid(field2SepLoc+1).trimmed();
    const QString location = heading.mid(field1SepLoc+1,(field2SepLoc-field1SepLoc-1)).trimmed();

    this->setMoment(moment);
    this->setLocation(location);
    this->setLocationType(locationType);
}

///////////////////////////////////////////////////////////////////////////////

SceneElement::SceneElement(QObject *parent)
    : QObject(parent),
      m_scene(qobject_cast<Scene*>(parent))
{
    connect(this, &SceneElement::typeChanged, this, &SceneElement::elementChanged);
    connect(this, &SceneElement::textChanged, this, &SceneElement::elementChanged);
}

SceneElement::~SceneElement()
{
    emit aboutToDelete(this);
}

void SceneElement::setType(SceneElement::Type val)
{
    if(m_type == val)
        return;

    PushSceneUndoCommand cmd(m_scene);

    m_type = val;
    emit typeChanged();

    if(m_scene != nullptr)
        emit m_scene->sceneElementChanged(this, Scene::ElementTypeChange);
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

    PushSceneUndoCommand cmd(m_scene);

    m_text = val.trimmed();
    emit textChanged();

    if(m_scene != nullptr)
        emit m_scene->sceneElementChanged(this, Scene::ElementTextChange);
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

    connect(this, &Scene::sceneElementChanged, this, &Scene::onSceneElementChanged);
    connect(this, &Scene::aboutToRemoveSceneElement, this, &Scene::onAboutToRemoveSceneElement);

    connect(this, &Scene::sceneAboutToReset, [this]() {
        m_isBeingReset = true;
        emit resetStateChanged();
    });
    connect(this, &Scene::sceneReset, [this]() {
        m_isBeingReset = false;
        emit resetStateChanged();
    });
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

    PushSceneUndoCommand cmd(this);

    m_title = val;
    emit titleChanged();
}

void Scene::setColor(const QColor &val)
{
    if(m_color == val)
        return;

    PushSceneUndoCommand cmd(this);

    m_color = val;
    emit colorChanged();
}

void Scene::setEnabled(bool val)
{
    if(m_enabled == val)
        return;

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

void Scene::addInvisibleCharacter(const QString &characterName)
{
    const QList<SceneElement*> elements = m_characterElementMap.characterElements(characterName);
    if(!elements.isEmpty())
        return;

    SceneElement *element = new SceneElement(this);
    element->setProperty("#invisible", true);
    element->setType(SceneElement::Character);
    element->setText(characterName);

    emit sceneChanged();
}

void Scene::removeInvisibleCharacter(const QString &characterName)
{
    const QList<SceneElement*> elements = m_characterElementMap.characterElements(characterName);
    if(elements.isEmpty() || elements.size() > 1)
        return;

    const QVariant value = elements.first()->property("#invisible");
    if(value.isValid() && value.toBool())
    {
        emit aboutToRemoveSceneElement(elements.first());
        GarbageCollector::instance()->add(elements.first());
        emit sceneChanged();
    }
}

bool Scene::isCharacterInvisible(const QString &characterName) const
{
    const QList<SceneElement*> elements = m_characterElementMap.characterElements(characterName);
    if(elements.isEmpty() || elements.size() > 1)
        return false;

    const QVariant value = elements.first()->property("#invisible");
    return (value.isValid() && value.toBool());
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

void Scene::insertElementAt(SceneElement *ptr, int index)
{
    if(ptr == nullptr || m_elements.indexOf(ptr) >= 0)
        return;

    if(index < 0 || index > m_elements.size())
        return;

    PushSceneUndoCommand cmd(this);

    if(!m_inSetElementsList)
        this->beginInsertRows(QModelIndex(), index, index);

    ptr->setParent(this);

    m_elements.insert(index, ptr);
    connect(ptr, &SceneElement::elementChanged, this, &Scene::sceneChanged);
    connect(ptr, &SceneElement::aboutToDelete, this, &Scene::removeElement);
    connect(this, &Scene::cursorPositionChanged, ptr, &SceneElement::cursorPositionChanged);

    if(!m_inSetElementsList)
        this->endInsertRows();

    emit elementCountChanged();

    // To ensure that character names are collected under all-character names
    // while an import is being done.
    if(ptr->type() == SceneElement::Character)
        emit sceneElementChanged(ptr, ElementTypeChange);
}

void Scene::removeElement(SceneElement *ptr)
{
    if(ptr == nullptr)
        return;

    const int row = m_elements.indexOf(ptr);
    if(row < 0)
        return;

    PushSceneUndoCommand cmd(this);

    if(!m_inSetElementsList)
        this->beginRemoveRows(QModelIndex(), row, row);

    emit aboutToRemoveSceneElement(ptr);
    m_elements.removeAt(row);

    disconnect(ptr, &SceneElement::elementChanged, this, &Scene::sceneChanged);
    disconnect(ptr, &SceneElement::aboutToDelete, this, &Scene::removeElement);
    disconnect(this, &Scene::cursorPositionChanged, ptr, &SceneElement::cursorPositionChanged);

    if(!m_inSetElementsList)
        this->endRemoveRows();

    emit elementCountChanged();

    if(ptr->parent() == this)
        GarbageCollector::instance()->add(ptr);
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

void Scene::removeLastElementIfEmpty()
{
    if(m_elements.isEmpty())
        return;

    SceneElement *element = m_elements.last();
    if(element->text().isEmpty())
    {
        emit sceneAboutToReset();
        this->removeElement(element);
        emit sceneReset(-1);
    }
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

void Scene::beginUndoCapture(bool allowMerging)
{
    if(m_pushUndoCommand != nullptr)
        return;

    m_pushUndoCommand = new PushSceneUndoCommand(this, allowMerging);
}

void Scene::endUndoCapture()
{
    if(m_pushUndoCommand == nullptr)
        return;

    delete m_pushUndoCommand;
    m_pushUndoCommand = nullptr;
}

Scene *Scene::splitScene(SceneElement *element, int textPosition, QObject *parent)
{
    if(element == nullptr)
        return nullptr;

    const int index = this->indexOfElement(element);
    if(index < 0)
        return nullptr;

    // We cannot split the scene across these types.
    static const QList<SceneElement::Type> unsplittableTypes = QList<SceneElement::Type>()
            << SceneElement::Heading << SceneElement::Parenthetical;
    if(element->type() == SceneElement::Heading || element->type() == SceneElement::Parenthetical)
        return nullptr;

    PushSceneUndoCommand cmd(this);

    emit sceneAboutToReset();

    Scene *newScene = new Scene(parent);
    newScene->setTitle("2nd Part Of " + this->title());
    newScene->setColor(this->color());
    newScene->heading()->setEnabled( this->heading()->isEnabled() );
    newScene->heading()->setLocationType( this->heading()->locationType() );
    newScene->heading()->setLocation( this->heading()->location() );
    newScene->heading()->setMoment("LATER");
    newScene->id(); // trigger creation of new Scene ID

    this->setTitle("1st Part Of " + this->title());

    // Move all elements from index+1 onwards to the new scene.
    for(int i=this->elementCount()-1; i>=index+1; i--)
    {
        SceneElement *oldElement = this->elementAt(i);

        SceneElement *newElement = new SceneElement(newScene);
        newElement->setType( oldElement->type() );
        newElement->setText( oldElement->text() );
        newScene->insertElementAt(newElement, 0);

        this->removeElement(oldElement);
    }

    auto splitText = [](const QString &text, int position) {
        if(position <= 0)
            return qMakePair<QString,QString>(QString(), text);

        if(position >= text.length())
            return qMakePair<QString,QString>(text, QString());

        // Go to the previous word.
        while(position >= 1) {
            if(text.at(position-1).isSpace())
                break;
            --position;
        }

        if(position <= 0)
            return qMakePair<QString,QString>(QString(), text);

        QString first = text.left(position).trimmed();
        QString second = text.mid(position);
        if(!first.endsWith("."))
            first += ".";

        return qMakePair<QString,QString>(first, second);
    };

    switch(element->type())
    {
    case SceneElement::Action:
        {
            const QPair<QString,QString> texts = splitText(element->text(), textPosition);
            if(texts.first.isEmpty())
                this->removeElement(element);
            else
                element->setText(texts.first);

            if(!texts.second.isEmpty())
            {
                SceneElement *newElement = new SceneElement(newScene);
                newElement->setType(SceneElement::Action);
                newElement->setText(texts.second);
                newScene->insertElementAt(newElement, 0);
            }
        } break;
    case SceneElement::Character:
        {
            // We cannot split the character element into two, we have to move
            // everything from the character element and down to the new scene
            newScene->insertElementAt(element, 0);
        } break;
    case SceneElement::Dialogue:
        {
            const QPair<QString,QString> texts = splitText(element->text(), textPosition);
            if(!texts.second.isEmpty())
            {
                // Dialogue cannot be split independently. We have to split carry
                // over the character element for which it was created to the
                // next scene.
                SceneElement *characterElement = nullptr;
                SceneElement *newCharacterElement = nullptr;
                int characterElementIndex = index-1;
                while(characterElementIndex >= 0)
                {
                    characterElement = this->elementAt(characterElementIndex);
                    if(characterElement->type() == SceneElement::Character)
                    {
                        newCharacterElement = new SceneElement(newScene);
                        newCharacterElement->setType(SceneElement::Character);
                        newCharacterElement->setText(characterElement->text());
                        break;
                    }

                    --characterElementIndex;
                    characterElement = nullptr;
                }

                if(texts.first.isEmpty())
                    this->removeElement(element);
                else
                    element->setText(texts.first);

                SceneElement *newElement = new SceneElement(newScene);
                newElement->setType(newCharacterElement ? SceneElement::Dialogue : SceneElement::Action);
                newElement->setText(texts.second);
                newScene->insertElementAt(newElement, 0);
                if(newCharacterElement)
                    newScene->insertElementAt(newCharacterElement, 0);
            }
        } break;
    default:
        {
            if(textPosition <= 0)
                newScene->insertElementAt(element, 0);
        } break;
    }

    while(1)
    {
        SceneElement *newSceneElement = newScene->elementAt(0);
        if(newSceneElement == nullptr)
            break;

        if(newSceneElement->text().isEmpty())
        {
            newScene->removeElement(newSceneElement);
            break;
        }

        break;
    }

    emit sceneReset(textPosition);

    return newScene;
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

QByteArray Scene::toByteArray() const
{
    QByteArray bytes;
    QDataStream ds(&bytes, QIODevice::WriteOnly);
    ds << m_id;
    ds << m_title;
    ds << m_color;
    ds << m_cursorPosition;
    ds << m_heading->locationType();
    ds << m_heading->location();
    ds << m_heading->moment();
    ds << m_elements.size();
    Q_FOREACH(SceneElement *element, m_elements)
    {
        ds << int(element->type());
        ds << element->text();
    }

    return bytes;
}

bool Scene::resetFromByteArray(const QByteArray &bytes)
{
    QDataStream ds(bytes);

    QString sceneID;
    ds >> sceneID;
    if(m_id.isEmpty())
        this->setId(sceneID);
    else if(sceneID != m_id)
        return false;

    this->sceneAboutToReset();
    this->clearElements();

    QString title;
    ds >> title;
    this->setTitle(title);

    QColor color;
    ds >> color;
    this->setColor(color);

    int curPosition = -1;
    ds >> curPosition;
    this->setCursorPosition(curPosition);

    QString locType;
    ds >> locType;
    this->heading()->setLocationType(locType);

    QString loc;
    ds >> loc;
    this->heading()->setLocation(loc);

    QString moment;
    ds >> moment;
    this->heading()->setMoment(moment);

    int nrElements = 0;
    ds >> nrElements;

    for(int i=0; i<nrElements; i++)
    {
        SceneElement *element = new SceneElement(this);

        int type = SceneElement::Action;
        ds >> type;
        element->setType( SceneElement::Type(type) );

        QString text;
        ds >> text;
        element->setText(text);

        this->addElement(element);
    }

    this->sceneReset(curPosition);

    return true;
}

Scene *Scene::fromByteArray(const QByteArray &bytes)
{
    QDataStream ds(bytes);

    QString sceneId;
    ds >> sceneId;

    const Structure *structure = ScriteDocument::instance()->structure();
    const StructureElement *element = structure->findElementBySceneID(sceneId);
    if(element == nullptr || element->scene() == nullptr)
        return nullptr;

    Scene *scene = element->scene();
    if(scene->resetFromByteArray(bytes))
        return scene;

    return nullptr;
}

void Scene::serializeToJson(QJsonObject &json) const
{
    const QStringList names = m_characterElementMap.characterNames();
    QJsonArray invisibleCharacters;

    Q_FOREACH(QString name, names)
    {
        if(this->isCharacterInvisible(name))
            invisibleCharacters.append(name);
    }

    if(!invisibleCharacters.isEmpty())
        json.insert("#invisibleCharacters", invisibleCharacters);
}

void Scene::deserializeFromJson(const QJsonObject &json)
{
    const QJsonArray invisibleCharacters = json.value("#invisibleCharacters").toArray();
    if(invisibleCharacters.isEmpty())
        return;

    for(int i=0; i<invisibleCharacters.size(); i++)
        this->addInvisibleCharacter(invisibleCharacters.at(i).toString());
}

void Scene::setElementsList(const QList<SceneElement *> &list)
{
    QScopedValueRollback<bool> isel(m_inSetElementsList, true);

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
        if( !oldElements.removeOne(item) )
            this->addElement(item);
        else
            m_elements.append(item);
    }

    while(!oldElements.isEmpty())
    {
        SceneElement *ptr = oldElements.takeFirst();
        emit aboutToRemoveSceneElement(ptr);
        GarbageCollector::instance()->add(ptr);
    }

    this->endResetModel();

    if(sizeChanged)
        emit elementCountChanged();

    emit sceneChanged();
}

void Scene::onSceneElementChanged(SceneElement *element, Scene::SceneElementChangeType)
{
    if( m_characterElementMap.include(element) )
        emit characterNamesChanged();
}

void Scene::onAboutToRemoveSceneElement(SceneElement *element)
{
    if( m_characterElementMap.remove(element) )
        emit characterNamesChanged();
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

