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

#include "notes.h"
#include "scene.h"
#include "structure.h"
#include "timeprofiler.h"

#include <QSet>
#include <QTextBoundaryFinder>

Note::Note(QObject *parent)
    : QObject(parent)
{
    connect(this, &Note::typeChanged, this, &Note::noteModified);
    connect(this, &Note::titleChanged, this, &Note::noteModified);
    connect(this, &Note::summaryChanged, this, &Note::noteModified);
    connect(this, &Note::contentChanged, this, &Note::noteModified);
    connect(this, &Note::metaDataChanged, this, &Note::noteModified);
    connect(m_attachments, &Attachments::attachmentsModified, this, &Note::noteModified);
}

Note::~Note()
{
    emit aboutToDelete(this);
}

void Note::setType(Type val)
{
    if(m_type == val)
        return;

    m_type = val;
    emit typeChanged();
}

void Note::setTitle(const QString &val)
{
    if(m_title == val)
        return;

    m_title = val;
    emit titleChanged();
}

void Note::setSummary(const QString &val)
{
    if(m_summary == val)
        return;

    m_summary = val;
    emit summaryChanged();
}

void Note::setContent(const QJsonValue &val)
{
    if(m_content == val)
        return;

    m_content = val;
    emit contentChanged();
}

void Note::setColor(const QColor &val)
{
    if(m_color == val)
        return;

    m_color = val;
    emit colorChanged();
}

void Note::setMetaData(const QJsonObject &val)
{
    if(m_metaData == val)
        return;

    m_metaData = val;
    emit metaDataChanged();
}

bool Note::canSerialize(const QMetaObject *metaObject, const QMetaProperty &metaProperty) const
{
    if(!this->metaObject()->inherits(metaObject))
        return false;

    static const int propIndex = this->metaObject()->indexOfProperty("metaData");
    if(propIndex >= 0 && metaProperty.propertyIndex() == propIndex)
        return m_type == FormNoteType;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

Notes::Notes(QObject *parent)
      :ObjectListPropertyModel<Note *>(parent)
{
    connect(this, &Notes::objectCountChanged, this, &Notes::noteCountChanged);
    connect(this, &Notes::noteCountChanged, this, &Notes::notesModified);

    if(parent != nullptr)
    {
        const QMetaObject *pmo = parent->metaObject();
        if(pmo->inherits(&Structure::staticMetaObject))
            m_ownerType = StructureOwner;
        else if(pmo->inherits(&Scene::staticMetaObject))
        {
            m_ownerType = SceneOwner;

            Scene *scene = (qobject_cast<Scene*>(parent));
            m_color = scene->color();
            connect(scene, &Scene::colorChanged, this, [=]() {
                this->setColor(scene->color());
            });
        }
        else if(pmo->inherits(&Character::staticMetaObject))
        {
            m_ownerType = CharacterOwner;

            Character *character = (qobject_cast<Character*>(parent));
            m_color = character->color();
            connect(character, &Character::colorChanged, this, [=]() {
                this->setColor(character->color());
            });
        }
        else if(pmo->inherits(&Relationship::staticMetaObject))
            m_ownerType = RelationshipOwner;
        /*else if(pmo->inherits(&Character::staticMetaObject))
            m_ownerType = CharacterOwner;
        else if(pmo->inherits(&Prop::staticMetaObject))
            m_ownerType = PropOwner;*/
        else
            m_ownerType = LocationOwner;
    }
}

Notes::~Notes()
{

}

Notes::OwnerType Notes::ownerType() const
{
    return m_ownerType;
}

Structure *Notes::structure() const
{
    return qobject_cast<Structure*>(this->owner());
}

Scene *Notes::scene() const
{
    return qobject_cast<Scene*>(this->owner());
}

Character *Notes::character() const
{
    return qobject_cast<Character*>(this->owner());
}

Relationship *Notes::relationship() const
{
    return qobject_cast<Relationship*>(this->owner());
}

void Notes::setColor(const QColor &val)
{
    if(m_color == val)
        return;

    m_color = val;
    emit colorChanged();

    switch(m_ownerType)
    {
    case SceneOwner:
        this->scene()->setColor(val);
        break;
    case CharacterOwner:
        this->character()->setColor(val);
        break;
    default:
        break;
    }
}

Note *Notes::addTextNote()
{
    Note *ptr = new Note(this);
    ptr->setType(Note::TextNoteType);
    this->addNote(ptr);
    return ptr;
}

void Notes::addNote(Note *ptr)
{
    if(ptr == nullptr || this->indexOf(ptr) >= 0)
        return;

    ptr->setParent(this);

    connect(ptr, &Note::aboutToDelete, this, &Notes::removeNote);
    connect(ptr, &Note::noteModified, this, &Notes::notesModified);

    this->append(ptr);
}

void Notes::setNotes(const QList<Note *> &list)
{
    if(!this->isEmpty())
        return;

    for(Note *ptr : list)
    {
        ptr->setParent(this);
        connect(ptr, &Note::aboutToDelete, this, &Notes::removeNote);
        connect(ptr, &Note::noteModified, this, &Notes::notesModified);

        if(ptr->type() == Note::TextNoteType && ptr->summary().isEmpty())
            ptr->setSummary( ptr->content().toString() );
    }

    this->assign(list);
}

void Notes::removeNote(Note *ptr)
{
    if(ptr == nullptr)
        return;

    const int index = this->indexOf(ptr);
    if(index < 0)
        return;

    disconnect(ptr, &Note::aboutToDelete, this, &Notes::removeNote);
    disconnect(ptr, &Note::noteModified, this, &Notes::notesModified);

    this->removeAt(index);

    ptr->deleteLater();
}

Note *Notes::noteAt(int index) const
{
    return this->at(index);
}

void Notes::clearNotes()
{
    while(this->size())
        this->removeNote(this->first());
}

void Notes::serializeToJson(QJsonObject &json) const
{
    if(this->isEmpty())
        return;

    QJsonArray jsNotes;

    const QList<Note*> &notes = this->list();
    for(Note *note : notes)
    {
        QJsonObject jsNote = QObjectSerializer::toJson(note);
        jsNotes.append(jsNote);
    }

    json.insert( QStringLiteral("#data"), jsNotes );
}

void Notes::deserializeFromJson(const QJsonObject &json)
{
    const QJsonArray jsNotes = json.value( QStringLiteral("#data") ).toArray();
    if(jsNotes.isEmpty())
        return;

    // Remove duplicate notes.
    QList<Note*> notes;
    QSet<QJsonObject> uniqueJsNotes;
    for(const QJsonValue &jsNotesItem : jsNotes)
        uniqueJsNotes |= jsNotesItem.toObject();

    notes.reserve(jsNotes.size());
    for(const QJsonObject &jsNote : qAsConst(uniqueJsNotes))
    {
        Note *note = new Note(this);
        if( QObjectSerializer::fromJson(jsNote, note) )
            notes.prepend(note);
        else
            delete note;
    }

    this->setNotes(notes);
}

void Notes::loadOldNotes(const QJsonArray &jsNotes)
{
    if(jsNotes.isEmpty() || !this->isEmpty())
        return;

    QList<Note*> notes;
    notes.reserve(jsNotes.size());
    for(const QJsonValue &jsNotesItem : jsNotes)
    {
        const QJsonObject jsNote = jsNotesItem.toObject();
        Note *note = new Note(this);
        note->setColor( QColor(jsNote.value(QStringLiteral("color")).toString()) );
        note->setTitle( jsNote.value(QStringLiteral("heading")).toString() );
        note->setContent( jsNote.value(QStringLiteral("content")) );

        const QString noteContent = note->content().toString();
        QTextBoundaryFinder sentenceFinder(QTextBoundaryFinder::Sentence, noteContent);
        const int from = sentenceFinder.position();
        const int to = sentenceFinder.toNextBoundary();
        if(from < 0 || to < 0)
            note->setSummary(noteContent);
        else
            note->setSummary(noteContent.mid(from, (to-from)).trimmed());

        notes.append(note);
    }

    this->setNotes(notes);
}


