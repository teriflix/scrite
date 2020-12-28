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
#include "hourglass.h"
#include "application.h"
#include "searchengine.h"
#include "timeprofiler.h"
#include "scritedocument.h"
#include "garbagecollector.h"
#include "qobjectserializer.h"

#include <QUuid>
#include <QFuture>
#include <QSGNode>
#include <QDateTime>
#include <QByteArray>
#include <QJsonArray>
#include <QJsonObject>
#include <QUndoCommand>
#include <QTextDocument>
#include <QJsonDocument>
#include <QtConcurrentRun>
#include <QScopedValueRollback>
#include <QAbstractTextDocumentLayout>
#include <QFutureWatcher>

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
    m_padding[0] = 0; // just to get rid of the unused private variable warning.
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
    m_padding[0] = 0; // just to get rid of the unused private variable warning.
    connect(this, &SceneHeading::momentChanged, this, &SceneHeading::textChanged);
    connect(this, &SceneHeading::enabledChanged, this, &SceneHeading::textChanged);
    connect(this, &SceneHeading::locationChanged, this, &SceneHeading::textChanged);
    connect(this, &SceneHeading::locationTypeChanged, this, &SceneHeading::textChanged);
    connect(this, &SceneHeading::textChanged, [=](){
        this->markAsModified();
    });
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
    if(m_enabled)
        return m_locationType + ". " + m_location + " - " + m_moment;

    return QString();
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
    connect(this, &SceneElement::elementChanged, [=](){
        this->markAsModified();
    });
}

SceneElement::~SceneElement()
{
    emit aboutToDelete(this);
}

SpellCheckService *SceneElement::spellCheck() const
{
    if(m_spellCheck == nullptr)
    {
        m_spellCheck = new SpellCheckService(const_cast<SceneElement*>(this));
        m_spellCheck->setMethod(SpellCheckService::OnDemand);
        m_spellCheck->setAsynchronous(true);
        m_spellCheck->setText(m_text);
    }

    return m_spellCheck;
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
    if(m_spellCheck != nullptr)
        m_spellCheck->setText(m_text);

    emit textChanged(val);

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
    case SceneElement::Shot:
    case SceneElement::Heading:
    case SceneElement::Character:
    case SceneElement::Transition:
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

CharacterElementMap::CharacterElementMap() { }
CharacterElementMap::~CharacterElementMap() { }

bool CharacterElementMap::include(SceneElement *element)
{
    // This function returns true if characterNames() would return
    // a different list after this function returns
    if(element == nullptr)
        return false;

    if(element->type() == SceneElement::Character)
    {
        const bool ret = this->remove(element);

        QString newName = element->formattedText();
        newName = newName.section('(', 0, 0).trimmed();
        if(newName.isEmpty())
            return ret;

        m_forwardMap[element] = newName;
        m_reverseMap[newName].append(element);
        return true;
    }

    if(m_forwardMap.contains(element))
        return this->remove(element);

    return false;
}

bool CharacterElementMap::remove(SceneElement *element)
{
    // This function returns true if characterNames() would return
    // a different list after this function returns
    const QString oldName = m_forwardMap.take(element);
    if(!oldName.isEmpty())
    {
        QList<SceneElement*> &list = m_reverseMap[oldName];
        if(list.removeOne(element))
        {
            if(list.isEmpty())
            {
                m_reverseMap.remove(oldName);
                return true;
            }

            if(list.size() == 1)
            {
                const QVariant value = list.first()->property("#mute");
                if(value.isValid() && value.toBool())
                    return true;
            }
        }
    }

    return false;
}

bool CharacterElementMap::remove(const QString &name)
{
    if(name.isEmpty())
        return false;

    const QList<SceneElement*> elements = m_reverseMap.take(name);
    if(elements.isEmpty())
        return false;

    Q_FOREACH(SceneElement *element, elements)
        m_forwardMap.take(element);

    return true;
}

QStringList CharacterElementMap::characterNames() const
{
    return m_reverseMap.keys();
}

QList<SceneElement *> CharacterElementMap::characterElements() const
{
    return m_forwardMap.keys();
}

QList<SceneElement *> CharacterElementMap::characterElements(const QString &name) const
{
    return m_reverseMap.value(name.toUpper());
}

void CharacterElementMap::include(const CharacterElementMap &other)
{
    const QList<SceneElement*> elements = other.characterElements();
    Q_FOREACH(SceneElement *element, elements)
        this->include(element);
}

///////////////////////////////////////////////////////////////////////////////

Scene::Scene(QObject *parent)
    : QAbstractListModel(parent)
{
    m_padding[0] = 0; // just to get rid of the unused private variable warning.

    connect(this, &Scene::titleChanged, this, &Scene::sceneChanged);
    connect(this, &Scene::colorChanged, this, &Scene::sceneChanged);
    connect(this, &Scene::noteCountChanged, this, &Scene::sceneChanged);
    connect(this, &Scene::elementCountChanged, this, &Scene::sceneChanged);
    connect(this, &Scene::characterRelationshipGraphChanged, this, &Scene::sceneChanged);
    connect(m_heading, &SceneHeading::textChanged, this, &Scene::sceneChanged);
    connect(this, &Scene::sceneChanged, [=](){
        this->markAsModified();
    });

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
    GarbageCollector::instance()->avoidChildrenOf(this);
    emit aboutToDelete(this);
}

Scene *Scene::clone(QObject *parent) const
{
    Scene *newScene = new Scene(parent);
    newScene->setTitle(m_title + QStringLiteral(" [Copy]"));
    newScene->setColor(m_color);
    newScene->setEnabled(m_enabled);
    newScene->heading()->setMoment(m_heading->moment());
    newScene->heading()->setLocation(m_heading->location());
    newScene->heading()->setLocationType(m_heading->locationType());

    Q_FOREACH(SceneElement *element, m_elements)
    {
        SceneElement *newElement = new SceneElement(newScene);
        newElement->setType(element->type());
        newElement->setText(element->text());
        newScene->addElement(newElement);
    }

    return newScene;
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

void Scene::setEmotionalChange(const QString &val)
{
    if(m_emotionalChange == val)
        return;

    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "emotionalChange");
    QScopedPointer<PushObjectPropertyUndoCommand> cmd;
    if(!info->isLocked())
        cmd.reset(new PushObjectPropertyUndoCommand(this, info->property));

    m_emotionalChange = val;
    emit emotionalChangeChanged();
}

void Scene::setCharactersInConflict(const QString &val)
{
    if(m_charactersInConflict == val)
        return;

    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "charactersInConflict");
    QScopedPointer<PushObjectPropertyUndoCommand> cmd;
    if(!info->isLocked())
        cmd.reset(new PushObjectPropertyUndoCommand(this, info->property));

    m_charactersInConflict = val;
    emit charactersInConflictChanged();
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

void Scene::setPageTarget(const QString &val)
{
    if(m_pageTarget == val)
        return;

    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "pageTarget");
    QScopedPointer<PushObjectPropertyUndoCommand> cmd;
    if(!info->isLocked())
        cmd.reset(new PushObjectPropertyUndoCommand(this, info->property));

    m_pageTarget = val;
    emit pageTargetChanged();
}

bool Scene::validatePageTarget(int pageNumber) const
{
    if(m_pageTarget.isEmpty())
        return true;

    if(pageNumber < 0)
        return false;

    const QStringList fields = m_pageTarget.split(QStringLiteral(","), QString::SkipEmptyParts);
    for(QString field : fields)
    {
        const QStringList nos = field.trimmed().split(QStringLiteral("-"), QString::SkipEmptyParts);
        if(nos.isEmpty())
            continue;

        const int nr1 = nos.first().trimmed().toInt();
        const int nr2 = nos.size() == 1 ? nr1 : nos.last().trimmed().toInt();
        if(pageNumber >= qMin(nr1,nr2) && pageNumber <= qMax(nr1,nr2))
            return true;
    }

    return false;
}

void Scene::setEnabled(bool val)
{
    if(m_enabled == val)
        return;

    m_enabled = val;
    emit enabledChanged();
}

void Scene::setType(Scene::Type val)
{
    if(m_type == val)
        return;

    m_type = val;
    emit typeChanged();
}

void Scene::setComments(const QString &val)
{
    if(m_comments == val)
        return;

    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "comments");
    QScopedPointer<PushObjectPropertyUndoCommand> cmd;
    if(!info->isLocked())
        cmd.reset(new PushObjectPropertyUndoCommand(this, info->property));

    m_comments = val;
    emit commentsChanged();
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

void Scene::addMuteCharacter(const QString &characterName)
{
    HourGlass hourGlass;

    const QList<SceneElement*> elements = m_characterElementMap.characterElements(characterName);
    if(!elements.isEmpty())
        return;

    SceneElement *element = new SceneElement(this);
    element->setProperty("#mute", true);
    element->setType(SceneElement::Character);
    element->setText(characterName);
    emit sceneElementChanged(element, ElementTypeChange);

    emit sceneChanged();
}

void Scene::removeMuteCharacter(const QString &characterName)
{
    const QList<SceneElement*> elements = m_characterElementMap.characterElements(characterName);
    if(elements.isEmpty() || elements.size() > 1)
        return;

    const QVariant value = elements.first()->property("#mute");
    if(value.isValid() && value.toBool())
    {
        emit aboutToRemoveSceneElement(elements.first());
        GarbageCollector::instance()->add(elements.first());
        emit sceneChanged();
    }
}

bool Scene::isCharacterMute(const QString &characterName) const
{
    const QList<SceneElement*> elements = m_characterElementMap.characterElements(characterName);
    if(elements.isEmpty() || elements.size() > 1)
        return false;

    const QVariant value = elements.first()->property("#mute");
    return (value.isValid() && value.toBool());
}

void Scene::scanMuteCharacters(const QStringList &characterNames)
{
    QStringList names = characterNames;
    if(names.isEmpty())
    {
        Structure *structure = qobject_cast<Structure*>(this->parent());
        if(structure)
            names = structure->characterNames();
    }

    const QStringList existingCharacters = this->characterNames();
    Q_FOREACH(QString existingCharacter, existingCharacters)
        names.removeAll(existingCharacter);

    const QList<SceneElement::Type> skipTypes = QList<SceneElement::Type>()
            << SceneElement::Character << SceneElement::Transition << SceneElement::Shot;

    Q_FOREACH(SceneElement *element, m_elements)
    {
        if(skipTypes.contains(element->type()))
            continue;

        const QString text = element->text();

        Q_FOREACH(QString name, names)
        {
            int pos = 0;
            while(pos < text.length())
            {
                pos = text.indexOf(name, pos, Qt::CaseInsensitive);
                if(pos < 0)
                    break;

                if(pos > 0)
                {
                    const QChar ch = text.at(pos-1);
                    if( !ch.isPunct() && !ch.isSpace() )
                    {
                        pos += name.length();
                        continue;
                    }
                }

                bool found = (pos + name.length() >= text.length());
                if(!found)
                {
                    const QChar ch = text.at(pos+name.length());
                    found = ch.isPunct() || ch.isSpace();
                }

                if(found)
                    this->addMuteCharacter(name);

                pos += name.length();
            }
        }
    }
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

SceneElement *Scene::appendElement(const QString &text, int type)
{
    SceneElement *element = new SceneElement(this);
    element->setType(SceneElement::Type(type));
    element->setText(text);
    this->addElement(element);
    return element;
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

void Scene::setElements(const QList<SceneElement *> &list)
{
    if(!m_elements.isEmpty() || list.isEmpty())
        return;

    this->beginResetModel();

    for(SceneElement *ptr : list)
    {
        ptr->setParent(this);
        connect(ptr, &SceneElement::elementChanged, this, &Scene::sceneChanged);
        connect(ptr, &SceneElement::aboutToDelete, this, &Scene::removeElement);
        connect(this, &Scene::cursorPositionChanged, ptr, &SceneElement::cursorPositionChanged);
        m_elements.append(ptr);
    }

    this->endResetModel();

    emit elementCountChanged();
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

void Scene::setNotes(const QList<Note *> &list)
{
    if(!m_notes.isEmpty() || list.isEmpty())
        return;

    for(Note *ptr : list)
    {
        ptr->setParent(this);

        connect(ptr, &Note::aboutToDelete, this, &Scene::removeNote);
        connect(ptr, &Note::noteChanged, this, &Scene::sceneChanged);
    }

    m_notes.assign(list);
    emit noteCountChanged();
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

    // Move all elements from index onwards to the new scene.
    for(int i=this->elementCount()-1; i>=index; i--)
    {
        SceneElement *oldElement = this->elementAt(i);

        SceneElement *newElement = new SceneElement(newScene);
        newElement->setType( oldElement->type() );
        newElement->setText( oldElement->text() );
        newScene->insertElementAt(newElement, 0);

        this->removeElement(oldElement);
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

void Scene::setCharacterRelationshipGraph(const QJsonObject &val)
{
    if(m_characterRelationshipGraph == val)
        return;

    m_characterRelationshipGraph = val;
    emit characterRelationshipGraphChanged();
}

void Scene::serializeToJson(QJsonObject &json) const
{
    const QStringList names = m_characterElementMap.characterNames();
    QJsonArray invisibleCharacters;

    Q_FOREACH(QString name, names)
    {
        if(this->isCharacterMute(name))
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
        this->addMuteCharacter(invisibleCharacters.at(i).toString());
}

bool Scene::canSetPropertyFromObjectList(const QString &propName) const
{
    if(propName == QStringLiteral("elements"))
        return m_elements.isEmpty();

    return false;
}

void Scene::setPropertyFromObjectList(const QString &propName, const QList<QObject *> &objects)
{
    if(propName == QStringLiteral("elements"))
    {
        this->setElements(qobject_list_cast<SceneElement*>(objects));
        return;
    }
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
    emit sceneRefreshed();
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

///////////////////////////////////////////////////////////////////////////////

SceneSizeHintItem::SceneSizeHintItem(QQuickItem *parent)
    : QQuickItem(parent),
      m_scene(this, "scene"),
      m_format(this, "format")
{
    this->setFlag(QQuickItem::ItemHasContents,false);
    this->setVisible(false);
}

SceneSizeHintItem::~SceneSizeHintItem()
{

}

void SceneSizeHintItem::setScene(Scene *val)
{
    if(m_scene == val)
        return;

    if(m_scene != nullptr)
    {
        disconnect(m_scene, &Scene::aboutToDelete, this, &SceneSizeHintItem::sceneReset);
        disconnect(m_scene, &Scene::sceneChanged, this, &SceneSizeHintItem::onSceneChanged);
    }

    m_scene = val;

    if(m_scene != nullptr)
    {
        connect(m_scene, &Scene::aboutToDelete, this, &SceneSizeHintItem::sceneReset);
        if(m_trackSceneChanges)
            connect(m_scene, &Scene::sceneChanged, this, &SceneSizeHintItem::onSceneChanged);
    }

    emit sceneChanged();

    this->evaluateSizeHintLater();
}

void SceneSizeHintItem::setTrackSceneChanges(bool val)
{
    if(m_trackSceneChanges == val)
        return;

    m_trackSceneChanges = val;
    emit trackSceneChangesChanged();

    if(val)
        connect(m_scene, &Scene::sceneChanged, this, &SceneSizeHintItem::onSceneChanged);
    else
        disconnect(m_scene, &Scene::sceneChanged, this, &SceneSizeHintItem::onSceneChanged);
}

void SceneSizeHintItem::setFormat(ScreenplayFormat *val)
{
    if(m_format == val)
        return;

    if(m_format != nullptr)
    {
        disconnect(m_format, &ScreenplayFormat::destroyed, this, &SceneSizeHintItem::formatReset);
        disconnect(m_format, &ScreenplayFormat::formatChanged, this, &SceneSizeHintItem::onFormatChanged);
    }

    m_format = val;

    if(m_format != nullptr)
    {
        connect(m_format, &ScreenplayFormat::destroyed, this, &SceneSizeHintItem::formatReset);
        if(m_trackFormatChanges)
            connect(m_format, &ScreenplayFormat::formatChanged, this, &SceneSizeHintItem::onFormatChanged);
    }

    emit formatChanged();

    this->evaluateSizeHintLater();
}

void SceneSizeHintItem::setTrackFormatChanges(bool val)
{
    if(m_trackFormatChanges == val)
        return;

    m_trackFormatChanges = val;
    emit trackFormatChangesChanged();

    if(val)
        connect(m_format, &ScreenplayFormat::formatChanged, this, &SceneSizeHintItem::onFormatChanged);
    else
        disconnect(m_format, &ScreenplayFormat::formatChanged, this, &SceneSizeHintItem::onFormatChanged);
}

void SceneSizeHintItem::setLeftMargin(qreal val)
{
    if( qFuzzyCompare(m_leftMargin, val) )
        return;

    m_leftMargin = val;
    emit leftMarginChanged();

    this->evaluateSizeHintLater();
}

void SceneSizeHintItem::setRightMargin(qreal val)
{
    if( qFuzzyCompare(m_rightMargin, val) )
        return;

    m_rightMargin = val;
    emit rightMarginChanged();

    this->evaluateSizeHintLater();
}

void SceneSizeHintItem::setTopMargin(qreal val)
{
    if( qFuzzyCompare(m_topMargin, val) )
        return;

    m_topMargin = val;
    emit topMarginChanged();

    this->evaluateSizeHintLater();
}

void SceneSizeHintItem::setBottomMargin(qreal val)
{
    if( qFuzzyCompare(m_bottomMargin, val) )
        return;

    m_bottomMargin = val;
    emit bottomMarginChanged();

    this->evaluateSizeHintLater();
}

void SceneSizeHintItem::classBegin()
{
    m_componentComplete = false;
}

void SceneSizeHintItem::componentComplete()
{
    QQuickItem::componentComplete();

    m_componentComplete = true;
    this->evaluateSizeHintLater();
}

void SceneSizeHintItem::timerEvent(QTimerEvent *te)
{
    if(te->timerId() == m_updateTimer.timerId())
    {
        m_updateTimer.stop();

        QFuture<QSizeF> future = QtConcurrent::run(this, &SceneSizeHintItem::evaluateSizeHint);

        QFutureWatcher<QSizeF> *watcher = new QFutureWatcher<QSizeF>(this);
        watcher->setFuture(future);
        connect(watcher, &QFutureWatcher<void>::finished, [=]() {
            this->updateSize( watcher->result() );
        });
        connect(watcher, &QFutureWatcher<void>::finished, watcher, &QObject::deleteLater);
    }
}

void SceneSizeHintItem::updateSize(const QSizeF &size)
{
    this->setContentWidth(size.width());
    this->setContentHeight(size.height());

    if(this->hasPendingComputeSize())
        this->setHasPendingComputeSize(false);
}

QSizeF SceneSizeHintItem::evaluateSizeHint()
{
    m_lock.lockForRead();
        const QMarginsF margins(m_leftMargin, m_topMargin, m_rightMargin, m_bottomMargin);
        const qreal pageWidth = this->width();
    m_lock.unlock();

    QTextDocument document;

    QTextFrameFormat frameFormat;
    frameFormat.setTopMargin(margins.top());
    frameFormat.setLeftMargin(margins.left());
    frameFormat.setRightMargin(margins.right());
    frameFormat.setBottomMargin(margins.bottom());

    QTextFrame *rootFrame = document.rootFrame();
    rootFrame->setFrameFormat(frameFormat);

    document.setTextWidth(pageWidth);

    if(m_scene != nullptr && m_format != nullptr)
    {
        const qreal maxParaWidth = (pageWidth - margins.left() - margins.right()) / m_format->devicePixelRatio();

        QTextCursor cursor(&document);
        for(int j=0; j<m_scene->elementCount(); j++)
        {
            const SceneElement *para = m_scene->elementAt(j);
            const SceneElementFormat *style = m_format->elementFormat(para->type());
            if(j)
                cursor.insertBlock();

            const QTextBlockFormat blockFormat = style->createBlockFormat(&maxParaWidth);
            const QTextCharFormat charFormat = style->createCharFormat(&maxParaWidth);
            cursor.setBlockFormat(blockFormat);
            cursor.setCharFormat(charFormat);
            cursor.insertText(para->text());
        }
    }

    return document.size();
}

void SceneSizeHintItem::evaluateSizeHintLater()
{
    this->setHasPendingComputeSize(true);

    m_updateTimer.start(10, this);
}

void SceneSizeHintItem::sceneReset()
{
    m_scene = nullptr;
    emit sceneChanged();

    this->evaluateSizeHintLater();
}

void SceneSizeHintItem::onSceneChanged()
{
    if(m_trackSceneChanges)
        this->evaluateSizeHintLater();
}

void SceneSizeHintItem::formatReset()
{
    m_format = nullptr;
    emit formatChanged();

    this->evaluateSizeHintLater();
}

void SceneSizeHintItem::onFormatChanged()
{
    if(m_trackFormatChanges)
        this->evaluateSizeHintLater();
}

void SceneSizeHintItem::setContentWidth(qreal val)
{
    if( qFuzzyCompare(m_contentWidth, val) )
        return;

    m_contentWidth = val;
    emit contentWidthChanged();
}

void SceneSizeHintItem::setContentHeight(qreal val)
{
    if( qFuzzyCompare(m_contentHeight, val) )
        return;

    m_contentHeight = val;
    emit contentHeightChanged();
}

void SceneSizeHintItem::setHasPendingComputeSize(bool val)
{
    if(m_hasPendingComputeSize == val)
        return;

    m_hasPendingComputeSize = val;
    emit hasPendingComputeSizeChanged();
}
