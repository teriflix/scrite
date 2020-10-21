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

#include "undoredo.h"
#include "structure.h"
#include "application.h"
#include "scritedocument.h"
#include "garbagecollector.h"

#include <QDir>
#include <QStack>
#include <QMimeData>
#include <QDateTime>
#include <QClipboard>
#include <QJsonDocument>

StructureElement::StructureElement(QObject *parent)
    : QObject(parent),
      m_structure(qobject_cast<Structure*>(parent)),
      m_follow(this, "follow")
{
    connect(this, &StructureElement::xChanged, this, &StructureElement::elementChanged);
    connect(this, &StructureElement::yChanged, this, &StructureElement::elementChanged);
    connect(this, &StructureElement::xChanged, this, &StructureElement::positionChanged);
    connect(this, &StructureElement::yChanged, this, &StructureElement::positionChanged);
    connect(this, &StructureElement::xChanged, this, &StructureElement::xfChanged);
    connect(this, &StructureElement::yChanged, this, &StructureElement::yfChanged);
    connect(this, &StructureElement::widthChanged, this, &StructureElement::elementChanged);
    connect(this, &StructureElement::heightChanged, this, &StructureElement::elementChanged);

    if(m_structure)
    {
        connect(m_structure, &Structure::canvasWidthChanged, this, &StructureElement::xfChanged);
        connect(m_structure, &Structure::canvasHeightChanged, this, &StructureElement::yfChanged);
    }
}

StructureElement::~StructureElement()
{
    emit aboutToDelete(this);
}

StructureElement *StructureElement::duplicate()
{
    if(m_structure == nullptr)
        return nullptr;

    const int newIndex = m_structure->indexOfElement(this)+1;

    StructureElement *newElement = new StructureElement(m_structure);
    newElement->setScene(m_scene->clone(newElement));
    newElement->setX(m_x);
    newElement->setY(m_y + (m_height > 0 ? m_height+100 : 200));
    m_structure->insertElement(newElement, newIndex);

    return newElement;
}

void StructureElement::setX(qreal val)
{
    if( qFuzzyCompare(m_x, val) )
        return;

    if(!m_placed)
        m_placed = !qFuzzyIsNull(m_x) && !qFuzzyIsNull(m_y);

    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "position");
    QScopedPointer<PushObjectPropertyUndoCommand> cmd;
    if(!info->isLocked())
        cmd.reset(new PushObjectPropertyUndoCommand(this, info->property, m_placed));

    m_x = val;
    emit xChanged();
}

void StructureElement::setY(qreal val)
{
    if( qFuzzyCompare(m_y, val) )
        return;

    if(!m_placed)
        m_placed = !qFuzzyIsNull(m_x) && !qFuzzyIsNull(m_y);

    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "position");
    QScopedPointer<PushObjectPropertyUndoCommand> cmd;
    if(!info->isLocked())
        cmd.reset(new PushObjectPropertyUndoCommand(this, info->property, m_placed));

    m_y = val;
    emit yChanged();
}

void StructureElement::setWidth(qreal val)
{
    if( qFuzzyCompare(m_width, val) )
        return;

    m_width = val;
    emit widthChanged();
}

void StructureElement::setHeight(qreal val)
{
    if( qFuzzyCompare(m_height, val) )
        return;

    m_height = val;
    emit heightChanged();
}

void StructureElement::setFollow(QQuickItem *val)
{
    if(m_follow == val)
        return;

    if(m_follow != nullptr)
    {
        disconnect(m_follow, &QQuickItem::xChanged, this, &StructureElement::syncWithFollowItem);
        disconnect(m_follow, &QQuickItem::yChanged, this, &StructureElement::syncWithFollowItem);
        disconnect(m_follow, &QQuickItem::widthChanged, this, &StructureElement::syncWithFollowItem);
        disconnect(m_follow, &QQuickItem::heightChanged, this, &StructureElement::syncWithFollowItem);
    }

    m_follow = val;

    if(m_follow)
    {
        this->syncWithFollowItem();
        connect(m_follow, &QQuickItem::xChanged, this, &StructureElement::syncWithFollowItem);
        connect(m_follow, &QQuickItem::yChanged, this, &StructureElement::syncWithFollowItem);
        connect(m_follow, &QQuickItem::widthChanged, this, &StructureElement::syncWithFollowItem);
        connect(m_follow, &QQuickItem::heightChanged, this, &StructureElement::syncWithFollowItem);
    }

    emit followChanged();
}

void StructureElement::resetFollow()
{
    m_follow = nullptr;
    emit followChanged();
}

void StructureElement::setXf(qreal val)
{
    if(m_structure == nullptr)
        return;

    val = qBound(0.0, val, 1.0);
    this->setX( m_structure->canvasWidth()*val );
}

qreal StructureElement::xf() const
{
    return m_structure==nullptr ? 0 : m_x / m_structure->canvasWidth();
}

void StructureElement::setYf(qreal val)
{
    if(m_structure == nullptr)
        return;

    val = qBound(0.0, val, 1.0);
    this->setY( m_structure->canvasHeight()*val );
}

qreal StructureElement::yf() const
{
    return m_structure==nullptr ? 0 : m_y / m_structure->canvasHeight();
}

void StructureElement::setPosition(const QPointF &pos)
{
    if(QPointF(m_x,m_y) == pos)
        return;

    this->setX(pos.x());
    this->setY(pos.y());
}

void StructureElement::setScene(Scene *val)
{
    if(m_scene == val || m_scene != nullptr || val == nullptr)
        return;

    m_scene = val;
    m_scene->setParent(this);
    connect(m_scene, &Scene::sceneChanged, this, &StructureElement::elementChanged);
    connect(m_scene, &Scene::aboutToDelete, this, &StructureElement::deleteLater);

    connect(m_scene->heading(), &SceneHeading::enabledChanged, this, &StructureElement::sceneHeadingChanged);
    connect(m_scene->heading(), &SceneHeading::locationTypeChanged, this, &StructureElement::sceneHeadingChanged);
    connect(m_scene->heading(), &SceneHeading::locationChanged, this, &StructureElement::sceneHeadingChanged);
    connect(m_scene->heading(), &SceneHeading::momentChanged, this, &StructureElement::sceneHeadingChanged);
    connect(m_scene->heading(), &SceneHeading::locationChanged, this, &StructureElement::sceneLocationChanged);

    emit sceneChanged();
}

void StructureElement::setSelected(bool val)
{
    if(m_selected == val)
        return;

    m_selected = val;
    emit selectedChanged();
}

bool StructureElement::event(QEvent *event)
{
    if(event->type() == QEvent::ParentChange)
    {
        if(m_structure)
        {
            disconnect(m_structure, &Structure::canvasWidthChanged, this, &StructureElement::xfChanged);
            disconnect(m_structure, &Structure::canvasHeightChanged, this, &StructureElement::yfChanged);
        }

        m_structure = qobject_cast<Structure*>(this->parent());

        if(m_structure)
        {
            connect(m_structure, &Structure::canvasWidthChanged, this, &StructureElement::xfChanged);
            connect(m_structure, &Structure::canvasHeightChanged, this, &StructureElement::yfChanged);
        }

        emit xfChanged();
        emit yfChanged();
    }

    return QObject::event(event);
}

void StructureElement::syncWithFollowItem()
{
    if(m_follow.isNull())
        return;

    this->setX(m_follow->x());
    this->setY(m_follow->y());
    this->setWidth(m_follow->width());
    this->setHeight(m_follow->height());
}

///////////////////////////////////////////////////////////////////////////////

Relationship::Relationship(QObject *parent)
    : QObject(parent),
      m_with(this, "with")
{
    m_of = qobject_cast<Character*>(parent);

    connect(this, &Relationship::ofChanged, this, &Relationship::relationshipChanged);
    connect(this, &Relationship::nameChanged, this, &Relationship::relationshipChanged);
    connect(this, &Relationship::withChanged, this, &Relationship::relationshipChanged);
    connect(this, &Relationship::directionChanged, this, &Relationship::relationshipChanged);
    connect(this, &Relationship::noteCountChanged, this, &Relationship::relationshipChanged);
}

Relationship::~Relationship()
{
    emit aboutToDelete(this);
}

void Relationship::setDirection(Relationship::Direction val)
{
    if(m_direction == val)
        return;

    m_direction = val;
    emit directionChanged();
}

QString Relationship::polishName(const QString &val)
{
    return Application::instance()->camelCased(val);
}

void Relationship::setName(const QString &val)
{
    const QString val2 = polishName(val);
    if(m_name == val2)
        return;

    m_name = val2;
    emit nameChanged();

    if(m_with != nullptr)
    {
        Relationship *rel = m_with->findRelationship(m_of);
        if(rel && rel != this)
            rel->setName(m_name);
    }
}

void Relationship::setWith(Character *val)
{
    if(m_with == val)
        return;

    m_with = val;
    emit withChanged();
}

QQmlListProperty<Note> Relationship::notes()
{
    return QQmlListProperty<Note>(
                reinterpret_cast<QObject*>(this),
                static_cast<void*>(this),
                &Relationship::staticAppendNote,
                &Relationship::staticNoteCount,
                &Relationship::staticNoteAt,
                &Relationship::staticClearNotes);
}

void Relationship::addNote(Note *ptr)
{
    if(ptr == nullptr || m_notes.indexOf(ptr) >= 0)
        return;

    ptr->setParent(this);

    connect(ptr, &Note::aboutToDelete, this, &Relationship::removeNote);
    connect(ptr, &Note::noteChanged, this, &Relationship::relationshipChanged);

    m_notes.append(ptr);
    emit noteCountChanged();
}

void Relationship::removeNote(Note *ptr)
{
    if(ptr == nullptr)
        return;

    const int index = m_notes.indexOf(ptr);
    if(index < 0)
        return;

    m_notes.removeAt(index);
    if(ptr->parent() == this)
        GarbageCollector::instance()->add(ptr);

    disconnect(ptr, &Note::aboutToDelete, this, &Relationship::removeNote);
    disconnect(ptr, &Note::noteChanged, this, &Relationship::relationshipChanged);

    emit noteCountChanged();
}

Note *Relationship::noteAt(int index) const
{
    return index < 0 || index >= m_notes.size() ? nullptr : m_notes.at(index);
}

void Relationship::clearNotes()
{
    while(m_notes.size())
        this->removeNote(m_notes.first());
}

void Relationship::serializeToJson(QJsonObject &json) const
{

    if(m_with != nullptr)
        json.insert("with", m_with->name());
}

void Relationship::deserializeFromJson(const QJsonObject &json)
{
    m_withName = json.value("with").toString();
}

void Relationship::resolveRelationship()
{
    if(!m_withName.isEmpty())
    {
        Structure *structure = nullptr;
        if(m_of != nullptr)
            structure = m_of->structure();
        else
            structure = ScriteDocument::instance()->structure();

        m_with = structure->findCharacter(m_withName);
        m_withName.clear();

        if(m_with != nullptr)
            emit withChanged();
        else
            this->deleteLater();
    }
}

void Relationship::staticAppendNote(QQmlListProperty<Note> *list, Note *ptr)
{
    reinterpret_cast< Relationship* >(list->data)->addNote(ptr);
}

void Relationship::staticClearNotes(QQmlListProperty<Note> *list)
{
    reinterpret_cast< Relationship* >(list->data)->clearNotes();
}

Note *Relationship::staticNoteAt(QQmlListProperty<Note> *list, int index)
{
    return reinterpret_cast< Relationship* >(list->data)->noteAt(index);
}

int Relationship::staticNoteCount(QQmlListProperty<Note> *list)
{
    return reinterpret_cast< Relationship* >(list->data)->noteCount();
}

bool Relationship::event(QEvent *event)
{
    if(event->type() == QEvent::ParentChange)
    {
        if(m_of == nullptr)
        {
            m_of = qobject_cast<Character*>(this->parent());
            emit ofChanged();
        }
        else if(m_of != this->parent())
            qFatal("Relationship of a character, once set, cannot be changed.");
    }

    return QObject::event(event);
}

void Relationship::setOf(Character *val)
{
    if(m_of == val)
        return;

    m_of = val;
    emit ofChanged();
}

void Relationship::resetWith()
{
    m_with = nullptr;
    emit withChanged();
}

///////////////////////////////////////////////////////////////////////////////

Character::Character(QObject *parent)
    : QObject(parent),
      m_structure(qobject_cast<Structure*>(parent))
{
    connect(this, &Character::ageChanged, this, &Character::characterChanged);
    connect(this, &Character::nameChanged, this, &Character::characterChanged);
    connect(this, &Character::typeChanged, this, &Character::characterChanged);
    connect(this, &Character::weightChanged, this, &Character::characterChanged);
    connect(this, &Character::photosChanged, this, &Character::characterChanged);
    connect(this, &Character::heightChanged, this, &Character::characterChanged);
    connect(this, &Character::genderChanged, this, &Character::characterChanged);
    connect(this, &Character::aliasesChanged, this, &Character::characterChanged);
    connect(this, &Character::bodyTypeChanged, this, &Character::characterChanged);
    connect(this, &Character::noteCountChanged, this, &Character::characterChanged);
    connect(this, &Character::designationChanged, this, &Character::characterChanged);
    connect(this, &Character::relationshipCountChanged, this, &Character::characterChanged);
    connect(this, &Character::characterRelationshipGraphChanged, this, &Character::characterChanged);
}

Character::~Character()
{

}

void Character::setName(const QString &val)
{
    if(m_name == val || val.isEmpty() || !m_name.isEmpty())
        return;

    m_name = val.toUpper().trimmed();
    emit nameChanged();
}

void Character::setVisibleOnNotebook(bool val)
{
    if(m_visibleOnNotebook == val)
        return;

    m_visibleOnNotebook = val;
    emit visibleOnNotebookChanged();
}

QQmlListProperty<Note> Character::notes()
{
    return QQmlListProperty<Note>(
                reinterpret_cast<QObject*>(this),
                static_cast<void*>(this),
                &Character::staticAppendNote,
                &Character::staticNoteCount,
                &Character::staticNoteAt,
                &Character::staticClearNotes);
}

void Character::addNote(Note *ptr)
{
    if(ptr == nullptr || m_notes.indexOf(ptr) >= 0)
        return;

    connect(ptr, &Note::aboutToDelete, this, &Character::removeNote);
    connect(ptr, &Note::noteChanged, this, &Character::characterChanged);

    ptr->setParent(this);
    m_notes.append(ptr);
    emit noteCountChanged();
}

void Character::removeNote(Note *ptr)
{
    if(ptr == nullptr)
        return;

    const int index = m_notes.indexOf(ptr);
    if(index < 0)
        return;

    m_notes.removeAt(index);
    if(ptr->parent() == this)
        GarbageCollector::instance()->add(ptr);

    disconnect(ptr, &Note::aboutToDelete, this, &Character::removeNote);
    disconnect(ptr, &Note::noteChanged, this, &Character::characterChanged);

    emit noteCountChanged();
}

Note *Character::noteAt(int index) const
{
    return index < 0 || index >= m_notes.size() ? nullptr : m_notes.at(index);
}

void Character::clearNotes()
{
    while(m_notes.size())
        this->removeNote(m_notes.first());
}

void Character::setPhotos(const QStringList &val)
{
    if(m_photos == val || !m_photos.isEmpty())
        return;

    DocumentFileSystem *dfs = m_structure->scriteDocument()->fileSystem();

    m_photos.reserve(val.size());
    Q_FOREACH(QString item, val)
    {
        if( dfs->contains(item) )
            m_photos << dfs->absolutePath(item);
    }

    emit photosChanged();
}

void Character::addPhoto(const QString &photoPath)
{
    DocumentFileSystem *dfs = m_structure->scriteDocument()->fileSystem();

    const QString dstPath = QStringLiteral("characters/") + QString::number(QDateTime::currentMSecsSinceEpoch()) + QStringLiteral(".jpg");
    const QString dfsPath = dfs->addImage(photoPath, dstPath, QSize(512,512), true);
    if(dfsPath.isEmpty())
        return;

    m_photos << dfs->absolutePath(dfsPath);
    emit photosChanged();
}

void Character::removePhoto(int index)
{
    if(index < 0 || index >= m_photos.size())
        return;

    DocumentFileSystem *dfs = m_structure->scriteDocument()->fileSystem();

    const QString dfsPath = dfs->relativePath(m_photos.at(index));
    if(dfsPath.isEmpty())
        return;

    if(dfs->remove(dfsPath))
    {
        m_photos.removeAt(index);
        emit photosChanged();
    }
}

void Character::removePhoto(const QString &photoPath)
{
    DocumentFileSystem *dfs = m_structure->scriteDocument()->fileSystem();

    const QString dfsPath = dfs->absolutePath(photoPath);
    if(dfsPath.isEmpty())
        return;

    const int index = m_photos.indexOf(photoPath);
    this->removePhoto(index);
}

void Character::setType(const QString &val)
{
    if(m_type == val)
        return;

    m_type = val;
    emit typeChanged();
}

void Character::setDesignation(const QString &val)
{
    if(m_designation == val)
        return;

    m_designation = val;
    emit designationChanged();
}

void Character::setGender(const QString &val)
{
    if(m_gender == val)
        return;

    m_gender = val;
    emit genderChanged();
}

void Character::setAge(const QString &val)
{
    if( m_age == val )
        return;

    m_age = val;
    emit ageChanged();
}

void Character::setHeight(const QString &val)
{
    if( m_height == val )
        return;

    m_height = val;
    emit heightChanged();
}

void Character::setWeight(const QString &val)
{
    if(m_weight == val)
        return;

    m_weight = val;
    emit weightChanged();
}

void Character::setBodyType(const QString &val)
{
    if(m_bodyType == val)
        return;

    m_bodyType = val;
    emit bodyTypeChanged();
}

void Character::setAliases(const QStringList &val)
{
    if(m_aliases == val)
        return;

    m_aliases.clear();
    Q_FOREACH(QString item, val)
        m_aliases << item.trimmed();

    emit aliasesChanged();
}

QQmlListProperty<Relationship> Character::relationships()
{
    return QQmlListProperty<Relationship>(
                reinterpret_cast<QObject*>(this),
                static_cast<void*>(this),
                &Character::staticAppendRelationship,
                &Character::staticRelationshipCount,
                &Character::staticRelationshipAt,
                &Character::staticClearRelationships);
}

void Character::addRelationship(Relationship *ptr)
{
    if(ptr == nullptr || m_relationships.indexOf(ptr) >= 0)
        return;

    ptr->setParent(this);

    connect(ptr, &Relationship::aboutToDelete, this, &Character::removeRelationship);
    connect(ptr, &Relationship::relationshipChanged, this, &Character::characterChanged);

    m_relationships.append(ptr);

    emit relationshipCountChanged();
}

void Character::removeRelationship(Relationship *ptr)
{
    if(ptr == nullptr)
        return;

    const int index = m_relationships.indexOf(ptr);
    if(index < 0)
        return;

    m_relationships.removeAt(index);
    if(ptr->parent() == this)
        GarbageCollector::instance()->add(ptr);

    disconnect(ptr, &Relationship::aboutToDelete, this, &Character::removeRelationship);
    disconnect(ptr, &Relationship::relationshipChanged, this, &Character::characterChanged);

    emit relationshipCountChanged();
}

Relationship *Character::relationshipAt(int index) const
{
    return index < 0 || index >= m_relationships.size() ? nullptr : m_relationships.at(index);
}

void Character::clearRelationships()
{
    while(m_relationships.size())
        this->removeRelationship(m_relationships.first());
}

Relationship *Character::addRelationship(const QString &name, Character *with)
{
    Relationship *relationship = nullptr;
    if(with == nullptr || with == this)
        return nullptr;

    // Find out if we have already established this relationsip.
    relationship = this->findRelationship(with);
    if(relationship != nullptr)
    {
        relationship->setName(name);
        return relationship;
    }

    // Create a new with-of relationship
    Relationship *withOf = new Relationship(with);
    withOf->setName(name);
    withOf->setWith(this);
    withOf->setDirection(Relationship::WithOf);
    with->addRelationship(withOf);

    // Create new of-with relationship
    Relationship *ofWith = new Relationship(this);
    ofWith->setName(name);
    ofWith->setWith(with);
    ofWith->setDirection(Relationship::OfWith);
    this->addRelationship(ofWith);

    // Ensure that if one of the relationships is destroyed, the other
    // must destroy itself.
    connect(withOf, &Relationship::aboutToDelete, ofWith, &Relationship::deleteLater);
    connect(ofWith, &Relationship::aboutToDelete, withOf, &Relationship::deleteLater);

    // Return the newly created relationship
    return ofWith;
}

Relationship *Character::findRelationshipWith(const QString &with) const
{
    const QString with2 = with.toUpper().simplified().trimmed();
    if(with2 == m_name)
        return nullptr;

    Q_FOREACH(Relationship *rel, m_relationships.list())
    {
        if(rel->with()->name() == with2)
            return rel;
    }

    return nullptr;
}

Relationship *Character::findRelationship(Character *with) const
{
    if(with == nullptr || with == this)
        return nullptr;

    Q_FOREACH(Relationship *rel, m_relationships.list())
    {
        if(rel->with() == with)
            return rel;
    }

    return nullptr;
}

bool Character::isRelatedTo(Character *with) const
{
    QStack<Character*> stack;
    return this->isRelatedToImpl(with, stack);
}

QList<Relationship *> Character::findRelationshipsWith(const QString &name) const
{
    QList<Relationship*> ret;

    const QString name2 = Relationship::polishName(name);
    Q_FOREACH(Relationship *rel, m_relationships.list())
    {
        if(rel->name() == name2)
            ret << rel;
    }

    return ret;
}

QStringList Character::unrelatedCharacterNames() const
{
    const QStringList names = this->structure()->characterNames();

    QStringList ret;
    Q_FOREACH(QString name, names)
    {
        if(this->name() == name || this->hasRelationshipWith(name))
            continue;

        ret << name;
    }

    return ret;
}

void Character::setCharacterRelationshipGraph(const QJsonObject &val)
{
    if(m_characterRelationshipGraph == val)
        return;

    m_characterRelationshipGraph = val;
    emit characterRelationshipGraphChanged();
}

void Character::serializeToJson(QJsonObject &json) const
{
    DocumentFileSystem *dfs = m_structure->scriteDocument()->fileSystem();
    QJsonArray array;
    Q_FOREACH(QString photo, m_photos)
        array.append( dfs->relativePath(photo) );
    json.insert("photos", array);
}

void Character::deserializeFromJson(const QJsonObject &json)
{
    DocumentFileSystem *dfs = m_structure->scriteDocument()->fileSystem();
    const QJsonArray array = json.value("photos").toArray();

    QStringList photoPaths;
    for(int i=0; i<array.size(); i++)
    {
        QString path = array.at(i).toString();
        if( QDir::isAbsolutePath(path) )
            continue;

        path = dfs->absolutePath(path);
        if( path.isEmpty() || !QFile::exists(path) )
            continue;

        QImage image(path);
        if(image.isNull())
            continue;

        photoPaths.append(path);
    }

    if(m_photos != photoPaths)
    {
        m_photos = photoPaths;
        emit photosChanged();
    }
}

void Character::resolveRelationships()
{
    Q_FOREACH(Relationship *rel, m_relationships.list())
        rel->resolveRelationship();
}

bool Character::event(QEvent *event)
{
    if(event->type() == QEvent::ParentChange)
        m_structure = qobject_cast<Structure*>(this->parent());

    return QObject::event(event);
}

bool Character::isRelatedToImpl(Character *with, QStack<Character *> &stack) const
{
    if(with == nullptr || with == this)
        return false;

    QList<Relationship*> rels = m_relationships.list();
    for(Relationship *rel : rels)
    {
        Character *rwith = rel->with();
        if(rwith == nullptr)
            continue;

        if(rwith == with)
            return true;

        if(stack.contains(rwith))
            continue;

        stack.push(rwith);
        const bool flag = rwith->isRelatedToImpl(with, stack);
        stack.pop();

        if(flag)
            return flag;
    }

    return false;
}

void Character::staticAppendNote(QQmlListProperty<Note> *list, Note *ptr)
{
    reinterpret_cast< Character* >(list->data)->addNote(ptr);
}

void Character::staticClearNotes(QQmlListProperty<Note> *list)
{
    reinterpret_cast< Character* >(list->data)->clearNotes();
}

Note *Character::staticNoteAt(QQmlListProperty<Note> *list, int index)
{
    return reinterpret_cast< Character* >(list->data)->noteAt(index);
}

int Character::staticNoteCount(QQmlListProperty<Note> *list)
{
    return reinterpret_cast< Character* >(list->data)->noteCount();
}

void Character::staticAppendRelationship(QQmlListProperty<Relationship> *list, Relationship *ptr)
{
    reinterpret_cast< Character* >(list->data)->addRelationship(ptr);
}

void Character::staticClearRelationships(QQmlListProperty<Relationship> *list)
{
    reinterpret_cast< Character* >(list->data)->clearRelationships();
}

Relationship *Character::staticRelationshipAt(QQmlListProperty<Relationship> *list, int index)
{
    return reinterpret_cast< Character* >(list->data)->relationshipAt(index);
}

int Character::staticRelationshipCount(QQmlListProperty<Relationship> *list)
{
    return reinterpret_cast< Character* >(list->data)->relationshipCount();
}

///////////////////////////////////////////////////////////////////////////////

Annotation::Annotation(QObject *parent)
    : QObject(parent),
      m_structure(qobject_cast<Structure*>(parent))
{
    connect(this, &Annotation::typeChanged, this, &Annotation::annotationChanged);
    connect(this, &Annotation::geometryChanged, this, &Annotation::annotationChanged);
    connect(this, &Annotation::attributesChanged, this, &Annotation::annotationChanged);
}

Annotation::~Annotation()
{
    emit aboutToDelete(this);
}

void Annotation::setType(const QString &val)
{
    // Can be set only once.
    if(m_type == val || !m_type.isEmpty())
        return;

    m_type = val;
    emit typeChanged();

    static QJsonObject metaDataDict;
    if(metaDataDict.isEmpty())
    {
        QFile file(":/misc/annotations_metadata.json");
        file.open(QFile::ReadOnly);
        metaDataDict = QJsonDocument::fromJson(file.readAll()).object();
    }

    const QJsonArray metaData = metaDataDict.value(m_type).toArray();
    this->setMetaData(metaData);
}

void Annotation::setResizable(bool val)
{
    if(m_resizable == val)
        return;

    m_resizable = val;
    emit resizableChanged();
}

void Annotation::setMovable(bool val)
{
    if(m_movable == val)
        return;

    m_movable = val;
    emit movableChanged();
}

void Annotation::setGeometry(const QRectF &val)
{
    if(m_geometry == val)
        return;

    QRectF val2;
    val2.setSize( m_resizable || m_geometry.size().isEmpty() ? val.size() : m_geometry.size() );
    val2.moveTopLeft( m_movable || m_geometry.topLeft().isNull() ? val.topLeft() : m_geometry.topLeft() );
    if(m_geometry == val2)
        return;

    m_geometry = val;
    emit geometryChanged();
}

void Annotation::setAttributes(const QJsonObject &val)
{
    if(m_attributes == val)
        return;

    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "attributes");
    QScopedPointer<PushObjectPropertyUndoCommand> cmd;
    if(!info->isLocked())
        cmd.reset(new PushObjectPropertyUndoCommand(this, info->property));

    m_attributes = val;
    this->polishAttributes();
    emit attributesChanged();
}

void Annotation::setMetaData(const QJsonArray &val)
{
    // Can be set only once.
    if(m_metaData == val || !m_metaData.isEmpty())
        return;

    m_metaData = val;
    for(int i=0; i<m_metaData.count(); i++)
    {
        QJsonObject obj = m_metaData.at(i).toObject();
        if( !obj.contains("visible") )
        {
            obj.insert("visible", true);
            m_metaData.replace(i, obj);
        }
    }

    this->polishAttributes();
    emit metaDataChanged();
}

bool Annotation::removeImage(const QString &name) const
{
    if(name.isEmpty())
        return false;

    DocumentFileSystem *dfs = ScriteDocument::instance()->fileSystem();
    if(dfs->contains(name))
    {
        dfs->remove(name);
        return true;
    }

    return false;
}

QString Annotation::addImage(const QString &path) const
{
    DocumentFileSystem *dfs = ScriteDocument::instance()->fileSystem();
    const QString addedPath = dfs->add(path, QStringLiteral("annotation"));
    return dfs->relativePath(addedPath);
}

QString Annotation::addImage(const QVariant &image) const
{
    DocumentFileSystem *dfs = ScriteDocument::instance()->fileSystem();
    const QString path = QStringLiteral("annotation/") + QString::number(QDateTime::currentSecsSinceEpoch()) + QStringLiteral(".jpg");
    const QString absPath = dfs->absolutePath(path, true);
    const QImage img = image.value<QImage>();
    img.save(absPath, "JPG");
    return dfs->relativePath(absPath);
}

QUrl Annotation::imageUrl(const QString &name) const
{
    DocumentFileSystem *dfs = ScriteDocument::instance()->fileSystem();
    return QUrl::fromLocalFile(dfs->absolutePath(name));
}

void Annotation::createCopyOfFileAttributes()
{
    for(int i=0; i<m_metaData.size(); i++)
    {
        const QJsonObject item = m_metaData.at(i).toObject();
        const QJsonValue isFileVal = item.value(QStringLiteral("isFile"));
        if( isFileVal.isBool() && isFileVal.toBool() )
        {
            const QString attrName = item.value(QStringLiteral("name")).toString();
            const QString fileName = m_attributes.value(attrName).toString();
            if(fileName.isEmpty())
                continue;

            DocumentFileSystem *dfs = ScriteDocument::instance()->fileSystem();
            m_attributes.insert(attrName, dfs->duplicate(fileName, QStringLiteral("annotation")));
            emit attributesChanged();
        }
    }
}

bool Annotation::event(QEvent *event)
{
    if(event->type() == QEvent::ParentChange)
        m_structure = qobject_cast<Structure*>(this->parent());

    return QObject::event(event);
}

void Annotation::polishAttributes()
{
    QJsonArray::const_iterator it = m_metaData.begin();
    QJsonArray::const_iterator end = m_metaData.end();
    while(it != end)
    {
        const QJsonObject meta = (*it).toObject();
        if(meta.isEmpty())
        {
            ++it;
            continue;
        }

        const QString key = meta.value( QStringLiteral("name") ).toString();
        if(key.isEmpty())
        {
            ++it;
            continue;
        }

        QJsonValue attrVal = m_attributes.value(key);
        if(attrVal.isUndefined())
            m_attributes.insert(key, meta.value(QStringLiteral("default")));
        else
        {
            const bool isNumber = meta.value(QStringLiteral("type")).toString() == QStringLiteral("number");
            if(isNumber)
            {
                const qreal min = meta.value(QStringLiteral("min")).toDouble();
                const qreal max = meta.value(QStringLiteral("max")).toDouble();
                const qreal val = qBound(min, attrVal.toDouble(), max);
                if(val != attrVal.toDouble())
                    m_attributes.insert(key, val);
            }
        }

        ++it;
    }
}

///////////////////////////////////////////////////////////////////////////////

Structure::Structure(QObject *parent)
    : QObject(parent),
      m_scriteDocument(qobject_cast<ScriteDocument*>(parent)),
      m_locationHeadingsMapTimer("Structure.m_locationHeadingsMapTimer")
{
    connect(this, &Structure::noteCountChanged, this, &Structure::structureChanged);
    connect(this, &Structure::characterCountChanged, this, &Structure::structureChanged);
    connect(this, &Structure::elementCountChanged, this, &Structure::structureChanged);
    connect(this, &Structure::annotationCountChanged, this, &Structure::structureChanged);
    connect(this, &Structure::currentElementIndexChanged, this, &Structure::structureChanged);
    connect(this, &Structure::characterRelationshipGraphChanged, this, &Structure::structureChanged);

    QClipboard *clipboard = qApp->clipboard();
    connect(clipboard, &QClipboard::dataChanged, this, &Structure::onClipboardDataChanged);
}

Structure::~Structure()
{

}

void Structure::setCanvasWidth(qreal val)
{
    if( qFuzzyCompare(m_canvasWidth, val) )
        return;

    m_canvasWidth = val;
    emit canvasWidthChanged();
}

void Structure::setCanvasHeight(qreal val)
{
    if( qFuzzyCompare(m_canvasHeight, val) )
        return;

    m_canvasHeight = val;
    emit canvasHeightChanged();
}

void Structure::setCanvasGridSize(qreal val)
{
    if( qFuzzyCompare(m_canvasGridSize, val) )
        return;

    m_canvasGridSize = val;
    emit canvasGridSizeChanged();
}

qreal Structure::snapToGrid(qreal val) const
{
    return Structure::snapToGrid(val, this);
}

qreal Structure::snapToGrid(qreal val, const Structure *structure, qreal defaultGridSize)
{
    if(val < 0)
        return 0;

    const qreal cgs = structure == nullptr ? defaultGridSize : structure->canvasGridSize();
    int nrGrids = qRound(val/cgs);
    return nrGrids * cgs;
}

void Structure::captureStructureAsImage(const QString &fileName)
{
    emit captureStructureAsImageRequest(fileName);
}

QQmlListProperty<Character> Structure::characters()
{
    return QQmlListProperty<Character>(
                reinterpret_cast<QObject*>(this),
                static_cast<void*>(this),
                &Structure::staticAppendCharacter,
                &Structure::staticCharacterCount,
                &Structure::staticCharacterAt,
                &Structure::staticClearCharacters);
}

void Structure::addCharacter(Character *ptr)
{
    if(ptr == nullptr || m_characters.indexOf(ptr) >= 0)
        return;

    Character *ch = ptr->isValid() ? this->findCharacter(ptr->name()) : nullptr;
    if(!ptr->isValid() || ch != nullptr)
    {
        if(ptr->parent() == this)
            GarbageCollector::instance()->add(ptr);
        return;
    }

    ptr->setParent(this);

    connect(ptr, &Character::aboutToDelete, this, &Structure::removeCharacter);
    connect(ptr, &Character::characterChanged, this, &Structure::structureChanged);

    m_characters.append(ptr);
    emit characterCountChanged();
}

void Structure::removeCharacter(Character *ptr)
{
    if(ptr == nullptr)
        return;

    const int index = m_characters.indexOf(ptr);
    if(index < 0)
        return ;

    m_characters.removeAt(index);

    disconnect(ptr, &Character::aboutToDelete, this, &Structure::removeCharacter);
    disconnect(ptr, &Character::characterChanged, this, &Structure::structureChanged);

    emit characterCountChanged();

    if(ptr->parent() == this)
        GarbageCollector::instance()->add(ptr);
}

Character *Structure::characterAt(int index) const
{
    return index < 0 || index >= m_characters.size() ? nullptr : m_characters.at(index);
}

void Structure::clearCharacters()
{
    while(m_characters.size())
        this->removeCharacter(m_characters.first());
}

QJsonArray Structure::detectCharacters() const
{
    QJsonArray ret;

    const QStringList names = this->allCharacterNames();

    Q_FOREACH(QString name, names)
    {
        Character *character = this->findCharacter(name);

        QJsonObject item;
        item.insert("name", name);
        item.insert("added", character != nullptr);
        ret.append(item);
    }

    return ret;
}

Character *Structure::addCharacter(const QString &name)
{
    const QString name2 = name.toUpper().simplified().trimmed();
    if(name2.isEmpty())
        return nullptr;

    Character *character = this->findCharacter(name2);
    if(character == nullptr)
    {
        character = new Character(this);
        character->setName(name2);
        this->addCharacter(character);
    }

    return character;
}

void Structure::addCharacters(const QStringList &names)
{
    Q_FOREACH(QString name, names)
        this->addCharacter(name);
}

Character *Structure::findCharacter(const QString &name) const
{
    const QString name2 = name.trimmed().toUpper();
    Q_FOREACH(Character *character, m_characters.list())
    {
        if(character->name() == name2)
            return character;
    }

    return nullptr;
}

QList<Character *> Structure::findCharacters(const QStringList &names, bool returnAssociativeList) const
{
    QList<Character*> ret;
    for(const QString &name: names)
    {
        Character *character = this->findCharacter(name);
        if(returnAssociativeList || character != nullptr)
            ret << character;
    }

    return ret;
}

QQmlListProperty<Note> Structure::notes()
{
    return QQmlListProperty<Note>(
                reinterpret_cast<QObject*>(this),
                static_cast<void*>(this),
                &Structure::staticAppendNote,
                &Structure::staticNoteCount,
                &Structure::staticNoteAt,
                &Structure::staticClearNotes);
}

void Structure::addNote(Note *ptr)
{
    if(ptr == nullptr || m_notes.indexOf(ptr) >= 0)
        return;

    ptr->setParent(this);

    connect(ptr, &Note::aboutToDelete, this, &Structure::removeNote);
    connect(ptr, &Note::noteChanged, this, &Structure::structureChanged);

    m_notes.append(ptr);
    emit noteCountChanged();
}

void Structure::removeNote(Note *ptr)
{
    if(ptr == nullptr)
        return;

    const int index = m_notes.indexOf(ptr);
    if(index < 0)
        return ;

    m_notes.removeAt(index);

    disconnect(ptr, &Note::aboutToDelete, this, &Structure::removeNote);
    disconnect(ptr, &Note::noteChanged, this, &Structure::structureChanged);

    emit noteCountChanged();

    if(ptr->parent() == this)
        GarbageCollector::instance()->add(ptr);
}

Note *Structure::noteAt(int index) const
{
    return index < 0 || index >= m_notes.size() ? nullptr : m_notes.at(index);
}

void Structure::clearNotes()
{
    while(m_notes.size())
        this->removeNote(m_notes.first());
}

QQmlListProperty<StructureElement> Structure::elements()
{
    return QQmlListProperty<StructureElement>(
                reinterpret_cast<QObject*>(this),
                static_cast<void*>(this),
                &Structure::staticAppendElement,
                &Structure::staticElementCount,
                &Structure::staticElementAt,
                &Structure::staticClearElements);
}

void Structure::addElement(StructureElement *ptr)
{
    this->insertElement(ptr, -1);
}

static void structureAppendElement(Structure *structure, StructureElement *ptr) { structure->addElement(ptr); }
static void structureRemoveElement(Structure *structure, StructureElement *ptr) { structure->removeElement(ptr); }
static void structureInsertElement(Structure *structure, StructureElement *ptr, int index) { structure->insertElement(ptr, index); }
static StructureElement *structureElementAt(Structure *structure, int index) { return structure->elementAt(index); }
static int structureIndexOfElement(Structure *structure, StructureElement *ptr) { return structure->indexOfElement(ptr); }

void Structure::removeElement(StructureElement *ptr)
{
    if(ptr == nullptr)
        return;

    const int index = m_elements.indexOf(ptr);
    if(index < 0)
        return;

    QScopedPointer< PushObjectListCommand<Structure,StructureElement> > cmd;
    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "elements");
    if(!info->isLocked() && /* DISABLES CODE */ (false))
    {
        ObjectListPropertyMethods<Structure,StructureElement> methods(&structureAppendElement, &structureRemoveElement, &structureInsertElement, &structureElementAt, structureIndexOfElement);
        cmd.reset( new PushObjectListCommand<Structure,StructureElement>(ptr, this, "elements", ObjectList::RemoveOperation, methods) );
    }

    m_elements.removeAt(index);

    disconnect(ptr, &StructureElement::elementChanged, this, &Structure::structureChanged);
    disconnect(ptr, &StructureElement::aboutToDelete, this, &Structure::removeElement);
    disconnect(ptr, &StructureElement::sceneLocationChanged, this, &Structure::updateLocationHeadingMapLater);
    this->updateLocationHeadingMapLater();

    emit elementCountChanged();
    emit elementsChanged();

    this->resetCurentElementIndex();

    if(ptr->parent() == this)
        GarbageCollector::instance()->add(ptr);
}

void Structure::insertElement(StructureElement *ptr, int index)
{
    if(ptr == nullptr || m_elements.indexOf(ptr) >= 0)
        return;

    QScopedPointer< PushObjectListCommand<Structure,StructureElement> > cmd;
    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "elements");
    if(!info->isLocked() && /* DISABLES CODE */ (false))
    {
        ObjectListPropertyMethods<Structure,StructureElement> methods(&structureAppendElement, &structureRemoveElement, &structureInsertElement, &structureElementAt, structureIndexOfElement);
        cmd.reset( new PushObjectListCommand<Structure,StructureElement>(ptr, this, "elements", ObjectList::InsertOperation, methods) );
    }

    if(index < 0 || index >= m_elements.size())
        m_elements.append(ptr);
    else
        m_elements.insert(index, ptr);

    ptr->setParent(this);

    connect(ptr, &StructureElement::elementChanged, this, &Structure::structureChanged);
    connect(ptr, &StructureElement::aboutToDelete, this, &Structure::removeElement);
    connect(ptr, &StructureElement::sceneLocationChanged, this, &Structure::updateLocationHeadingMapLater);
    this->updateLocationHeadingMapLater();

    this->onStructureElementSceneChanged(ptr);

    emit elementCountChanged();
    emit elementsChanged();

    if(this->scriteDocument() && !this->scriteDocument()->isLoading())
        this->setCurrentElementIndex(index);
}

void Structure::moveElement(StructureElement *ptr, int toRow)
{
    if(ptr == nullptr || toRow < 0 || toRow >= m_elements.size())
        return;

    const int fromRow = m_elements.indexOf(ptr);
    if(fromRow < 0)
        return;

    if(fromRow == toRow)
        return;

    m_elements.move(fromRow, toRow);
    emit elementsChanged();

    this->resetCurentElementIndex();
}

StructureElement *Structure::elementAt(int index) const
{
    return index < 0 || index >= m_elements.size() ? nullptr : m_elements.at(index);
}

void Structure::clearElements()
{
    while(m_elements.size())
        this->removeElement(m_elements.first());
}

int Structure::indexOfScene(Scene *scene) const
{
    if(scene == nullptr)
        return -1;

    for(int i=0; i<m_elements.size(); i++)
    {
        StructureElement *element = m_elements.at(i);
        if(element->scene() == scene)
            return i;
    }

    return -1;
}

int Structure::indexOfElement(StructureElement *element) const
{
    return m_elements.indexOf(element);
}

StructureElement *Structure::findElementBySceneID(const QString &id) const
{
    Q_FOREACH(StructureElement *element, m_elements.list())
    {
        if(element->scene()->id() == id)
            return element;
    }

    return nullptr;
}

QRectF Structure::layoutElements(Structure::LayoutType layoutType)
{
    QRectF newBoundingRect;

    QList<StructureElement*> elementsToLayout;
    Q_FOREACH(StructureElement *element, m_elements.list())
        if(element->isSelected())
            elementsToLayout << element;

    if(elementsToLayout.isEmpty())
        elementsToLayout = m_elements;

    if(elementsToLayout.size() < 2)
        return newBoundingRect;

    const Screenplay *screenplay = ScriteDocument::instance()->screenplay();
    if(screenplay == nullptr)
        return newBoundingRect;

    auto lessThan = [screenplay](StructureElement *e1, StructureElement *e2) -> bool {
          const int pos1 = screenplay->firstIndexOfScene(e1->scene());
          const int pos2 = screenplay->firstIndexOfScene(e2->scene());
          if(pos1 >= 0 && pos2 >= 0) return pos1 < pos2;
          if(pos2 < 0) return true;
          return false;
    };
    std::sort(elementsToLayout.begin(), elementsToLayout.end(), lessThan);

    QRectF oldBoundingRect;
    Q_FOREACH(StructureElement *element, elementsToLayout)
        oldBoundingRect |= QRectF(element->x(), element->y(), element->width(), element->height());

    static const qreal verticalLayoutSpacing = 50;
    static const qreal horizontalLayoutSpacing = 50;
    static const qreal flowVerticalLayoutSpacing = 20;
    static const qreal flowHorizontalLayoutSpacing = 20;

    int direction = 1;
    QRectF elementRect;
    for(int i=0; i<elementsToLayout.size(); i++)
    {
        StructureElement *element = elementsToLayout.at(i);
        if(i == 0)
        {
            elementRect = QRectF(element->position(), QSize(element->width(),element->height()));
            newBoundingRect = elementRect;

            if(layoutType == HorizontalLayout || layoutType == FlowHorizontalLayout)
            {
                if(elementRect.left() > oldBoundingRect.center().x())
                {
                    direction = -1;
                    elementRect.moveRight(oldBoundingRect.right());
                }
            }
            else
            {
                if(elementRect.top() > oldBoundingRect.center().y())
                {
                    direction = -1;
                    elementRect.moveBottom(oldBoundingRect.bottom());
                }
            }

            if(direction < 0)
            {
                elementRect = QRectF(element->position(), QSize(element->width(),element->height()));
                newBoundingRect = elementRect;
            }

            continue;
        }

        switch(layoutType)
        {
        case VerticalLayout:
            if(direction > 0)
                elementRect.moveTop(elementRect.bottom() + verticalLayoutSpacing);
            else
                elementRect.moveBottom(elementRect.top() - verticalLayoutSpacing);
            break;
        case HorizontalLayout:
            if(direction > 0)
                elementRect.moveLeft(elementRect.right() + horizontalLayoutSpacing);
            else
                elementRect.moveRight(elementRect.left() - horizontalLayoutSpacing);
            break;
        case FlowVerticalLayout:
            if(direction > 0)
                elementRect.moveTop(elementRect.bottom() + verticalLayoutSpacing);
            else
                elementRect.moveBottom(elementRect.top() - verticalLayoutSpacing);
            if(i%2)
                elementRect.moveLeft(elementRect.right() + flowVerticalLayoutSpacing);
            else
                elementRect.moveRight(elementRect.left() - flowVerticalLayoutSpacing);
            break;
        case FlowHorizontalLayout:
            if(direction > 0)
                elementRect.moveLeft( elementRect.center().x() + flowHorizontalLayoutSpacing );
            else
                elementRect.moveRight( elementRect.center().x() - flowHorizontalLayoutSpacing );
            if(i%2)
                elementRect.moveTop( elementRect.bottom() + verticalLayoutSpacing );
            else
                elementRect.moveBottom( elementRect.top() - verticalLayoutSpacing );
            break;
        }

        element->setPosition(elementRect.topLeft());
        newBoundingRect |= elementRect;
    }

    return newBoundingRect;
}

void Structure::scanForMuteCharacters()
{
    m_scriteDocument->setBusyMessage("Scanning for mute characters..");

    const QStringList characterNames = this->characterNames();
    Q_FOREACH(StructureElement *element, m_elements.list())
        element->scene()->scanMuteCharacters(characterNames);

    m_scriteDocument->clearBusyMessage();
}

QStringList Structure::standardLocationTypes() const
{
    static const QStringList list = QStringList() << "INT" << "EXT" << "I/E";
    return list;
}

QStringList Structure::standardMoments() const
{
    static const QStringList list = QStringList() << "DAY" << "NIGHT" << "MORNING" << "AFTERNOON"
        << "EVENING" << "LATER" << "MOMENTS LATER" << "CONTINUOUS" << "THE NEXT DAY" << "EARLIER"
        << "MOMENTS EARLIER" << "THE PREVIOUS DAY" << "DAWN" << "DUSK";
    return list;
}

void Structure::setCurrentElementIndex(int val)
{
    val = qBound(-1, val, m_elements.size()-1);
    if(m_currentElementIndex == val)
        return;

    m_currentElementIndex = val;
    emit currentElementIndexChanged();
}

void Structure::setZoomLevel(qreal val)
{
    if( qFuzzyCompare(m_zoomLevel, val) )
        return;

    m_zoomLevel = val;
    emit zoomLevelChanged();
}

QQmlListProperty<Annotation> Structure::annotations()
{
    return QQmlListProperty<Annotation>(
                reinterpret_cast<QObject*>(this),
                static_cast<void*>(this),
                &Structure::staticAppendAnnotation,
                &Structure::staticAnnotationCount,
                &Structure::staticAnnotationAt,
                &Structure::staticClearAnnotations);
}

static void structureAppendAnnotation(Structure *structure, Annotation *ptr) { structure->addAnnotation(ptr); }
static void structureRemoveAnnotation(Structure *structure, Annotation *ptr) { structure->removeAnnotation(ptr); }
static void structureInsertAnnotation(Structure *structure, Annotation *ptr, int) { structure->addAnnotation(ptr); }
static Annotation *structureAnnotationAt(Structure *structure, int index) { return structure->annotationAt(index); }
static int structureIndexOfAnnotation(Structure *, Annotation *) { return -1; }

void Structure::addAnnotation(Annotation *ptr)
{
    if(ptr == nullptr || m_annotations.indexOf(ptr) >= 0)
        return;

    QScopedPointer< PushObjectListCommand<Structure,Annotation> > cmd;
    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "annotations");
    if(!info->isLocked() && /* DISABLES CODE */ (false))
    {
        ObjectListPropertyMethods<Structure,Annotation> methods(&structureAppendAnnotation, &structureRemoveAnnotation, &structureInsertAnnotation, &structureAnnotationAt, structureIndexOfAnnotation);
        cmd.reset( new PushObjectListCommand<Structure,Annotation>(ptr, this, info->property, ObjectList::InsertOperation, methods) );
    }

    m_annotations.append(ptr);

    ptr->setParent(this);
    connect(ptr, &Annotation::aboutToDelete, this, &Structure::removeAnnotation);
    connect(ptr, &Annotation::annotationChanged, this, &Structure::structureChanged);

    emit annotationCountChanged();
}

void Structure::removeAnnotation(Annotation *ptr)
{
    if(ptr == nullptr)
        return;

    const int index = m_annotations.indexOf(ptr);
    if(index < 0)
        return;

    QScopedPointer< PushObjectListCommand<Structure,Annotation> > cmd;
    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "annotations");
    if(!info->isLocked() && /* DISABLES CODE */ (false))
    {
        ObjectListPropertyMethods<Structure,Annotation> methods(&structureAppendAnnotation, &structureRemoveAnnotation, &structureInsertAnnotation, &structureAnnotationAt, structureIndexOfAnnotation);
        cmd.reset( new PushObjectListCommand<Structure,Annotation>(ptr, this, info->property, ObjectList::RemoveOperation, methods) );
    }

    m_annotations.removeAt(index);

    disconnect(ptr, &Annotation::aboutToDelete, this, &Structure::removeAnnotation);
    disconnect(ptr, &Annotation::annotationChanged, this, &Structure::structureChanged);

    emit annotationCountChanged();

    if(ptr->parent() == this)
        GarbageCollector::instance()->add(ptr);
}

Annotation *Structure::annotationAt(int index) const
{
    return index < 0 || index >= m_annotations.size() ? nullptr : m_annotations.at(index);
}

void Structure::bringToFront(Annotation *ptr)
{
    if(ptr == nullptr || m_annotations.empty())
        return;

    if(m_annotations.last() == ptr)
        return;

    const int index = m_annotations.indexOf(ptr);
    if(index < 0)
        return;

    m_annotations.takeAt(index);
    m_annotations.append(ptr);
    emit annotationCountChanged(); // Although the count did not change, we use the same
                                   // signal to announce change in the annotations list property
}

void Structure::sendToBack(Annotation *ptr)
{
    if(ptr == nullptr || m_annotations.empty())
        return;

    if(m_annotations.first() == ptr)
        return;

    const int index = m_annotations.indexOf(ptr);
    if(index < 0)
        return;

    m_annotations.takeAt(index);
    m_annotations.prepend(ptr);
    emit annotationCountChanged(); // Although the count did not change, we use the same
                                   // signal to announce change in the annotations list property
}

void Structure::clearAnnotations()
{
    while(m_annotations.size())
        this->removeAnnotation(m_annotations.first());
}

void Structure::copy(QObject *elementOrAnnotation)
{
    if(elementOrAnnotation == nullptr)
        return;

    StructureElement *element = qobject_cast<StructureElement*>(elementOrAnnotation);
    Annotation *annotation = element ? nullptr : qobject_cast<Annotation*>(elementOrAnnotation);
    if(element != nullptr || annotation != nullptr)
    {
        QClipboard *clipboard = qApp->clipboard();

        QJsonObject objectJson = QObjectSerializer::toJson(elementOrAnnotation);
        if(element != nullptr)
        {
            QJsonObject sceneJson = objectJson.value(QStringLiteral("scene")).toObject();
            sceneJson.remove( QStringLiteral("id") );
            objectJson.insert(QStringLiteral("scene"), sceneJson);
        }

        QJsonObject clipboardJson;
        clipboardJson.insert(QStringLiteral("class"), QString::fromLatin1(elementOrAnnotation->metaObject()->className()));
        clipboardJson.insert(QStringLiteral("data"), objectJson);
        clipboardJson.insert(QStringLiteral("app"), qApp->applicationName() + QStringLiteral("-") + qApp->applicationVersion());
        clipboardJson.insert(QStringLiteral("source"), QStringLiteral("Structure"));

        const QByteArray clipboardText = QJsonDocument(clipboardJson).toJson();

        QMimeData *mimeData = new QMimeData;
        mimeData->setData(QStringLiteral("scrite/structure"), clipboardText);
#ifndef QT_NO_DEBUG
        mimeData->setData(QStringLiteral("text/plain"), clipboardText);
#endif
        clipboard->setMimeData(mimeData);
        return;
    }
}

static inline QJsonObject fetchPasteDataFromClipboard(QString *className=nullptr)
{
    QJsonObject ret;

    ScriteDocument *sdoc = ScriteDocument::instance();
    if(sdoc->isReadOnly())
        return ret;

    const QClipboard *clipboard = qApp->clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    if(mimeData == nullptr)
        return ret;

    const QByteArray clipboardText = mimeData->data(QStringLiteral("scrite/structure"));
    if(clipboardText.isEmpty())
        return ret;

    QJsonParseError parseError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(clipboardText, &parseError);
    if(parseError.error != QJsonParseError::NoError)
        return ret;

    const QString appString = qApp->applicationName() + QStringLiteral("-") + qApp->applicationVersion();

    const QJsonObject clipboardJson = jsonDoc.object();
    if( clipboardJson.value( QStringLiteral("app") ).toString() != appString )
        return ret; // We dont want to support copy/paste between different versions of Scrite.

    if( clipboardJson.value(QStringLiteral("source")).toString() != QStringLiteral("Structure") )
        return ret;

    const QJsonObject data = clipboardJson.value("data").toObject();
    if(!data.isEmpty() && className)
        *className = clipboardJson.value( QStringLiteral("class") ).toString();

    return data;
}

void Structure::paste(const QPointF &pos)
{
    QString className;
    const QJsonObject data = fetchPasteDataFromClipboard(&className);
    if(data.isEmpty())
        return;

    QObjectFactory factory;
    factory.addClass<Annotation>();
    factory.addClass<StructureElement>();

    QObject *object = factory.create(className.toLatin1(), this);
    if(object == nullptr)
        return;

    bool success = QObjectSerializer::fromJson(data, object);
    if(!success)
    {
        delete object;
        return;
    }

    Annotation *annotation = qobject_cast<Annotation*>(object);
    if(annotation != nullptr)
    {
        annotation->createCopyOfFileAttributes();
        annotation->setObjectName(QStringLiteral("ica"));

        QRectF geometry = annotation->geometry();

        if(pos.isNull())
            geometry.moveTopLeft( geometry.topLeft() + QPointF(50, 50) );
        else
            geometry.moveCenter(pos);

        annotation->setGeometry(geometry);

        this->addAnnotation(annotation);

        // We copy the newly pasted annotation once more, so that the next paste will
        // happen relative to the newly pasted annotation
        this->copy(annotation);
        return;
    }

    StructureElement *element = qobject_cast<StructureElement*>(object);
    if(element != nullptr)
    {
        if(pos.isNull())
            element->setPosition( element->position() + QPointF(50,50) );
        else
            element->setPosition(pos);

        this->addElement(element);

        // We copy the newly pasted annotation once more, so that the next paste will
        // happen relative to the newly pasted element
        this->copy(element);
        return;
    }
}

void Structure::setCharacterRelationshipGraph(const QJsonObject &val)
{
    if(m_characterRelationshipGraph == val)
        return;

    m_characterRelationshipGraph = val;
    emit characterRelationshipGraphChanged();
}

void Structure::serializeToJson(QJsonObject &) const
{
    // Do nothing
}

void Structure::deserializeFromJson(const QJsonObject &)
{
    Q_FOREACH(Character *character, m_characters.list())
        character->resolveRelationships();

    // Forward and reverse relationships must be a tuple. If one is deleted, the other must
    // get deleted right away. We cannot afford to have zombie relationships.
    Q_FOREACH(Character *character, m_characters.list())
    {
        for(int i=0; i<character->relationshipCount(); i++)
        {
            Relationship *ofWith = character->relationshipAt(i);
            if(ofWith->direction() == Relationship::OfWith)
            {
                Relationship *withOf = ofWith->with()->findRelationship(character);
                if(withOf == nullptr)
                {
                    ofWith->deleteLater();
                    continue;
                }

                if(withOf->direction() == Relationship::WithOf)
                {
                    // Ensure that if one of the relationships is destroyed, the other
                    // must destroy itself.
                    connect(withOf, &Relationship::aboutToDelete, ofWith, &Relationship::deleteLater);
                    connect(ofWith, &Relationship::aboutToDelete, withOf, &Relationship::deleteLater);
                }
                else
                {
                    ofWith->deleteLater();
                    withOf->deleteLater();
                }
            }
        }
    }
}

bool Structure::event(QEvent *event)
{
    if(event->type() == QEvent::ParentChange)
        m_scriteDocument = qobject_cast<ScriteDocument*>(this->parent());

    return QObject::event(event);
}

void Structure::timerEvent(QTimerEvent *event)
{
    if(m_locationHeadingsMapTimer.timerId() == event->timerId())
    {
        m_locationHeadingsMapTimer.stop();
        this->updateLocationHeadingMap();
        return;
    }

    QObject::timerEvent(event);
}

void Structure::resetCurentElementIndex()
{
    int val = m_currentElementIndex;
    if(m_elements.isEmpty())
        val = -1;
    else
        val = qBound(0, val, m_elements.size()-1);
    m_currentElementIndex = -2;

    this->setCurrentElementIndex(val);
}

void Structure::setCanPaste(bool val)
{
    if(m_canPaste == val)
        return;

    m_canPaste = val;
    emit canPasteChanged();
}

void Structure::onClipboardDataChanged()
{
    const QJsonObject data = fetchPasteDataFromClipboard();
    this->setCanPaste(!data.isEmpty());
}

StructureElement *Structure::splitElement(StructureElement *ptr, SceneElement *element, int textPosition)
{
    /*
     * Never call this function directly. This function __must__ be called as a part of
     * Screenplay::splitElement() call.
     */
    if(ptr == nullptr)
        return nullptr;

    const int index = this->indexOfElement(ptr);
    if(index < 0)
        return nullptr;

    Scene *newScene = ptr->scene()->splitScene(element, textPosition);
    if(newScene == nullptr)
        return nullptr;

    StructureElement *newElement = new StructureElement(this);
    newElement->setScene(newScene);
    newElement->setX( ptr->x() + 300 );
    newElement->setY( ptr->y() + 80 );
    this->insertElement(newElement, index+1);
    return newElement;
}

void Structure::staticAppendCharacter(QQmlListProperty<Character> *list, Character *ptr)
{
    reinterpret_cast< Structure* >(list->data)->addCharacter(ptr);
}

void Structure::staticClearCharacters(QQmlListProperty<Character> *list)
{
    reinterpret_cast< Structure* >(list->data)->clearCharacters();
}

Character *Structure::staticCharacterAt(QQmlListProperty<Character> *list, int index)
{
    return reinterpret_cast< Structure* >(list->data)->characterAt(index);
}

int Structure::staticCharacterCount(QQmlListProperty<Character> *list)
{
    return reinterpret_cast< Structure* >(list->data)->characterCount();
}

void Structure::staticAppendNote(QQmlListProperty<Note> *list, Note *ptr)
{
    reinterpret_cast< Structure* >(list->data)->addNote(ptr);
}

void Structure::staticClearNotes(QQmlListProperty<Note> *list)
{
    reinterpret_cast< Structure* >(list->data)->clearNotes();
}

Note *Structure::staticNoteAt(QQmlListProperty<Note> *list, int index)
{
    return reinterpret_cast< Structure* >(list->data)->noteAt(index);
}

int Structure::staticNoteCount(QQmlListProperty<Note> *list)
{
    return reinterpret_cast< Structure* >(list->data)->noteCount();
}

void Structure::staticAppendElement(QQmlListProperty<StructureElement> *list, StructureElement *ptr)
{
    reinterpret_cast< Structure* >(list->data)->addElement(ptr);
}

void Structure::staticClearElements(QQmlListProperty<StructureElement> *list)
{
    reinterpret_cast< Structure* >(list->data)->clearElements();
}

StructureElement *Structure::staticElementAt(QQmlListProperty<StructureElement> *list, int index)
{
    return reinterpret_cast< Structure* >(list->data)->elementAt(index);
}

int Structure::staticElementCount(QQmlListProperty<StructureElement> *list)
{
    return reinterpret_cast< Structure* >(list->data)->elementCount();
}

void Structure::updateLocationHeadingMap()
{
    QMap< QString, QList<SceneHeading*> > map;
    Q_FOREACH(StructureElement *element, m_elements.list())
    {
        Scene *scene = element->scene();
        if(scene == nullptr || !scene->heading()->isEnabled())
            continue;

        const QString location = scene->heading()->location();
        if(location.isEmpty())
            continue;

        map[location].append(scene->heading());
    }

    m_locationHeadingsMap = map;
}

void Structure::updateLocationHeadingMapLater()
{
    m_locationHeadingsMapTimer.start(0, this);
}

void Structure::onStructureElementSceneChanged(StructureElement *element)
{
    if(element == nullptr)
        element = qobject_cast<StructureElement*>(this->sender());

    if(element == nullptr || element->scene() == nullptr)
        return;

    connect(element->scene(), &Scene::sceneElementChanged, this, &Structure::onSceneElementChanged);
    connect(element->scene(), &Scene::aboutToRemoveSceneElement, this, &Structure::onAboutToRemoveSceneElement);
    m_characterElementMap.include(element->scene()->characterElementMap());

    this->updateLocationHeadingMapLater();
}

void Structure::onSceneElementChanged(SceneElement *element, Scene::SceneElementChangeType)
{
    if( m_characterElementMap.include(element) )
        emit characterNamesChanged();
}

void Structure::onAboutToRemoveSceneElement(SceneElement *element)
{
    if( m_characterElementMap.remove(element) )
        emit characterNamesChanged();
}

void Structure::staticAppendAnnotation(QQmlListProperty<Annotation> *list, Annotation *ptr)
{
    reinterpret_cast< Structure* >(list->data)->addAnnotation(ptr);
}

void Structure::staticClearAnnotations(QQmlListProperty<Annotation> *list)
{
    reinterpret_cast< Structure* >(list->data)->clearAnnotations();
}

Annotation *Structure::staticAnnotationAt(QQmlListProperty<Annotation> *list, int index)
{
    return reinterpret_cast< Structure* >(list->data)->annotationAt(index);
}

int Structure::staticAnnotationCount(QQmlListProperty<Annotation> *list)
{
    return reinterpret_cast< Structure* >(list->data)->annotationCount();
}

///////////////////////////////////////////////////////////////////////////////

StructureElementConnector::StructureElementConnector(QQuickItem *parent)
    :AbstractShapeItem(parent),
      m_updateTimer("StructureElementConnector.m_updateTimer"),
      m_toElement(this, "toElement"),
      m_fromElement(this, "fromElement")
{
    this->setRenderType(OutlineOnly);
    this->setOutlineColor(Qt::black);
    this->setOutlineWidth(4);

    connect(this, &AbstractShapeItem::contentRectChanged, this, &StructureElementConnector::updateArrowAndLabelPositions);
}

StructureElementConnector::~StructureElementConnector()
{

}

void StructureElementConnector::setLineType(StructureElementConnector::LineType val)
{
    if(m_lineType == val)
        return;

    m_lineType = val;
    emit lineTypeChanged();
}

void StructureElementConnector::setFromElement(StructureElement *val)
{
    if(m_fromElement == val)
        return;

    if(m_fromElement != nullptr)
    {
        disconnect(m_fromElement, &StructureElement::xChanged, this, &StructureElementConnector::requestUpdateLater);
        disconnect(m_fromElement, &StructureElement::yChanged, this, &StructureElementConnector::requestUpdateLater);
        disconnect(m_fromElement, &StructureElement::widthChanged, this, &StructureElementConnector::requestUpdateLater);
        disconnect(m_fromElement, &StructureElement::heightChanged, this, &StructureElementConnector::requestUpdateLater);

        Scene *scene = m_fromElement->scene();
        disconnect(scene, &Scene::colorChanged, this, &StructureElementConnector::pickElementColor);
    }

    m_fromElement = val;

    if(m_fromElement != nullptr)
    {
        connect(m_fromElement, &StructureElement::xChanged, this, &StructureElementConnector::requestUpdateLater);
        connect(m_fromElement, &StructureElement::yChanged, this, &StructureElementConnector::requestUpdateLater);
        connect(m_fromElement, &StructureElement::widthChanged, this, &StructureElementConnector::requestUpdateLater);
        connect(m_fromElement, &StructureElement::heightChanged, this, &StructureElementConnector::requestUpdateLater);

        Scene *scene = m_fromElement->scene();
        connect(scene, &Scene::colorChanged, this, &StructureElementConnector::pickElementColor);
    }

    emit fromElementChanged();

    this->pickElementColor();

    this->update();
}

void StructureElementConnector::setToElement(StructureElement *val)
{
    if(m_toElement == val)
        return;

    if(m_toElement != nullptr)
    {
        disconnect(m_toElement, &StructureElement::xChanged, this, &StructureElementConnector::requestUpdateLater);
        disconnect(m_toElement, &StructureElement::yChanged, this, &StructureElementConnector::requestUpdateLater);
        disconnect(m_toElement, &StructureElement::widthChanged, this, &StructureElementConnector::requestUpdateLater);
        disconnect(m_toElement, &StructureElement::heightChanged, this, &StructureElementConnector::requestUpdateLater);

        Scene *scene = m_toElement->scene();
        disconnect(scene, &Scene::colorChanged, this, &StructureElementConnector::pickElementColor);
    }

    m_toElement = val;

    if(m_toElement != nullptr)
    {
        connect(m_toElement, &StructureElement::xChanged, this, &StructureElementConnector::requestUpdateLater);
        connect(m_toElement, &StructureElement::yChanged, this, &StructureElementConnector::requestUpdateLater);
        connect(m_toElement, &StructureElement::widthChanged, this, &StructureElementConnector::requestUpdateLater);
        connect(m_toElement, &StructureElement::heightChanged, this, &StructureElementConnector::requestUpdateLater);

        Scene *scene = m_toElement->scene();
        connect(scene, &Scene::colorChanged, this, &StructureElementConnector::pickElementColor);
    }

    emit toElementChanged();

    this->pickElementColor();

    this->update();
}

void StructureElementConnector::setArrowAndLabelSpacing(qreal val)
{
    if( qFuzzyCompare(m_arrowAndLabelSpacing, val) )
        return;

    m_arrowAndLabelSpacing = val;
    emit arrowAndLabelSpacingChanged();

    this->updateArrowAndLabelPositions();
}

bool StructureElementConnector::intersects(const QRectF &rect) const
{
    const QPainterPath shape = this->currentShape();
    if(shape.isEmpty())
        return true;

    const QRectF shapeBoundingRect = this->currentShape().boundingRect();
    return rect.isValid() && !rect.isNull() ? rect.intersects( shapeBoundingRect ) : true;
}

QPainterPath StructureElementConnector::shape() const
{
    QPainterPath path;
    if(m_fromElement == nullptr || m_toElement == nullptr)
        return path;

    auto getElementRect = [](StructureElement *e) {
        QRectF r(e->x(), e->y(), e->width(), e->height());
        // r.moveCenter(QPointF(e->x(), e->y()));
        return r;
    };

    const QRectF  r1 = getElementRect(m_fromElement);
    const QRectF  r2 = getElementRect(m_toElement);
    const QLineF line(r1.center(), r2.center());
    QPointF p1, p2;
    Qt::Edge e1, e2;

    if(r2.center().x() < r1.left())
    {
        p1 = QLineF(r1.topLeft(), r1.bottomLeft()).center();
        e1 = Qt::LeftEdge;

        if(r2.top() > r1.bottom())
        {
            p2 = QLineF(r2.topLeft(), r2.topRight()).center();
            e2 = Qt::TopEdge;
        }
        else if(r1.top() > r2.bottom())
        {
            p2 = QLineF(r2.bottomLeft(), r2.bottomRight()).center();
            e2 = Qt::BottomEdge;
        }
        else
        {
            p2 = QLineF(r2.topRight(), r2.bottomRight()).center();
            e2 = Qt::RightEdge;
        }
    }
    else if(r2.center().x() > r1.right())
    {
        p1 = QLineF(r1.topRight(), r1.bottomRight()).center();
        e1 = Qt::RightEdge;

        if(r2.top() > r1.bottom())
        {
            p2 = QLineF(r2.topLeft(), r2.topRight()).center();
            e2 = Qt::TopEdge;
        }
        else if(r1.top() > r2.bottom())
        {
            p2 = QLineF(r2.bottomLeft(), r2.bottomRight()).center();
            e2 = Qt::BottomEdge;
        }
        else
        {
            p2 = QLineF(r2.topLeft(), r2.bottomLeft()).center();
            e2 = Qt::LeftEdge;
        }
    }
    else
    {
        if(r2.top() > r1.bottom())
        {
            p1 = QLineF(r1.bottomLeft(), r1.bottomRight()).center();
            e1 = Qt::BottomEdge;

            p2 = QLineF(r2.topLeft(), r2.topRight()).center();
            e2 = Qt::TopEdge;
        }
        else
        {
            p1 = QLineF(r1.topLeft(), r1.topRight()).center();
            e1 = Qt::TopEdge;

            p2 = QLineF(r2.bottomLeft(), r2.bottomRight()).center();
            e2 = Qt::BottomEdge;
        }
    }

    if(m_lineType == StraightLine)
    {
        path.moveTo(p1);
        path.lineTo(p2);
    }
    else
    {
        QPointF cp = p1;
        switch(e1)
        {
        case Qt::LeftEdge:
        case Qt::RightEdge:
            cp = (e2 == Qt::BottomEdge || e2 == Qt::TopEdge) ? QPointF(p2.x(), p1.y()) : p1;
            break;
        default:
            cp = p1;
            break;
        }

        if(cp == p1)
        {
            path.moveTo(p1);
            path.lineTo(p2);
        }
        else
        {
            const qreal length = line.length();
            const qreal dist = 20.0;
            const qreal dt = dist / length;
            const qreal maxdt = 1.0 - dt;
            const QLineF l1(p1, cp);
            const QLineF l2(cp, p2);
            qreal t = dt;

            path.moveTo(p1);
            while(t < maxdt)
            {
                const QLineF l( l1.pointAt(t), l2.pointAt(t) );
                const QPointF p = l.pointAt(t);
                path.lineTo(p);
                t += dt;
            }
            path.lineTo(p2);
        }
    }

    static const QList<QPointF> arrowPoints = QList<QPointF>()
            << QPointF(-10,-5) << QPointF(0, 0) << QPointF(-10,5);

    const qreal angle = path.angleAtPercent(0.5);
    const QPointF lineCenter = path.pointAtPercent(0.5);

    QTransform tx;
    tx.translate(lineCenter.x(), lineCenter.y());
    tx.rotate(-angle);
    path.moveTo( tx.map(arrowPoints.at(0)) );
    path.lineTo( tx.map(arrowPoints.at(1)) );
    path.lineTo( tx.map(arrowPoints.at(2)) );

    return path;
}

void StructureElementConnector::timerEvent(QTimerEvent *te)
{
    if(m_updateTimer.timerId() == te->timerId())
    {
        m_updateTimer.stop();
        this->requestUpdate();
        return;
    }

    AbstractShapeItem::timerEvent(te);
}

void StructureElementConnector::resetFromElement()
{
    m_fromElement = nullptr;
    emit fromElementChanged();
    this->pickElementColor();
    this->update();
}

void StructureElementConnector::resetToElement()
{
    m_toElement = nullptr;
    emit toElementChanged();
    this->pickElementColor();
    this->update();
}

void StructureElementConnector::requestUpdateLater()
{
    m_updateTimer.start(0, this);
}

void StructureElementConnector::pickElementColor()
{
    if(m_fromElement != nullptr && m_toElement != nullptr)
    {
        const QColor c1 = m_fromElement->scene()->color();
        const QColor c2 = m_toElement->scene()->color();
        QColor mix = QColor::fromRgbF( (c1.redF()+c2.redF())/2.0,
                                       (c1.greenF()+c2.greenF())/2.0,
                                       (c1.blueF()+c2.blueF())/2.0 );
        const qreal luma = ((0.299 * mix.redF()) + (0.587 * mix.greenF()) + (0.114 * mix.blueF()));
        if(luma > 0.5)
            mix = mix.darker();

        this->setOutlineColor(mix);
    }
}

void StructureElementConnector::updateArrowAndLabelPositions()
{
    const QPainterPath path = this->currentShape();
    if(path.isEmpty())
        return;

    const qreal pathLength = path.length();
    if(pathLength < 0 || qFuzzyCompare(pathLength,0))
        return;

    const qreal arrowT = 0.5;
    const qreal labelT = 0.45 - (m_arrowAndLabelSpacing / pathLength);

    this->setArrowPosition( this->currentShape().pointAtPercent(arrowT) );
    if(labelT < 0 || labelT > 1)
        this->setSuggestedLabelPosition(this->arrowPosition());
    else
        this->setSuggestedLabelPosition( this->currentShape().pointAtPercent(labelT) );
}

void StructureElementConnector::setArrowPosition(const QPointF &val)
{
    if(m_arrowPosition == val)
        return;

    m_arrowPosition = val;
    emit arrowPositionChanged();
}

void StructureElementConnector::setSuggestedLabelPosition(const QPointF &val)
{
    if(m_suggestedLabelPosition == val)
        return;

    m_suggestedLabelPosition = val;
    emit suggestedLabelPositionChanged();
}
