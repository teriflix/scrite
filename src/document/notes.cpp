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
#include "scritedocument.h"

#include <QFileInfo>
#include <QMimeType>
#include <QMimeDatabase>
#include <QTextBoundaryFinder>
#include <QSet>

Attachment::Attachment(QObject *parent)
    : QObject(parent)
{
    connect(this, &Attachment::nameChanged, this, &Attachment::attachmentModified);
    connect(this, &Attachment::titleChanged, this, &Attachment::attachmentModified);
    connect(this, &Attachment::filePathChanged, this, &Attachment::attachmentModified);
    connect(this, &Attachment::mimeTypeChanged, this, &Attachment::attachmentModified);
}

Attachment::~Attachment()
{
    emit aboutToDelete(this);
}

void Attachment::setName(const QString &val)
{
    if(m_name == val)
        return;

    m_name = val;
    emit nameChanged();
}

void Attachment::setTitle(const QString &val)
{
    if(m_title == val)
        return;

    m_title = val;
    emit titleChanged();
}

void Attachment::setFilePath(const QString &val)
{
    if(m_filePath == val || !m_filePath.isEmpty())
        return;

    QFileInfo fi(val);
    if(!fi.exists() || !fi.isReadable())
        return;

    m_filePath = val;
    emit filePathChanged();
}

void Attachment::setMimeType(const QString &val)
{
    if(m_mimeType == val || !m_mimeType.isEmpty())
        return;

    m_mimeType = val;
    // m_type = ...
    // m_typeIcon = ...
    emit mimeTypeChanged();
}

void Attachment::serializeToJson(QJsonObject &json) const
{
    json.insert( QStringLiteral("#filePath"), m_filePath );
    json.insert( QStringLiteral("#mimeType"), m_mimeType );
}

void Attachment::deserializeFromJson(const QJsonObject &json)
{
    this->setFilePath( json.value( QStringLiteral("#filePath") ).toString() );
    this->setMimeType( json.value( QStringLiteral("#mimeType") ).toString() );
}

///////////////////////////////////////////////////////////////////////////////

Note::Note(QObject *parent)
    : QObject(parent)
{
    connect(this, &Note::typeChanged, this, &Note::noteModified);
    connect(this, &Note::titleChanged, this, &Note::noteModified);
    connect(this, &Note::summaryChanged, this, &Note::noteModified);
    connect(this, &Note::contentChanged, this, &Note::noteModified);
    connect(this, &Note::metaDataChanged, this, &Note::noteModified);
    connect(this, &Note::attachmentCountChanged, this, &Note::noteModified);
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

ObjectListPropertyModel<Attachment *> *Note::attachmentsModel() const
{
    return &(const_cast<Note*>(this)->m_attachments);
}

QQmlListProperty<Attachment> Note::attachments()
{
    return QQmlListProperty<Attachment>(
                reinterpret_cast<QObject*>(this),
                static_cast<void*>(this),
                &Note::staticAttachmentCount,
                &Note::staticAttachmentAt);
}

Attachment *Note::addAttachment(const QString &filePath)
{
    QFileInfo fi(filePath);
    if( !fi.exists() || !fi.isReadable() )
        return nullptr;

    static QMimeDatabase mimeDb;
    const QMimeType mimeType = mimeDb.mimeTypeForFile(fi);
    if(!mimeType.isValid())
        return nullptr;

    Notes *notes = qobject_cast<Notes*>(this->parent());
    if(notes == nullptr)
        return nullptr;

    ScriteDocument *doc = ScriteDocument::instance()->instance();
    DocumentFileSystem *dfs = doc->fileSystem();
    const QString attachedFilePath = dfs->add(filePath);

    Attachment *ptr = new Attachment(this);
    ptr->setFilePath(attachedFilePath);
    ptr->setMimeType(mimeType.name());
    ptr->setFilePath(fi.fileName());
    ptr->setTitle(fi.baseName());
    this->addAttachment(ptr);

    return ptr;
}

void Note::addAttachment(Attachment *ptr)
{
    if(ptr == nullptr || m_attachments.indexOf(ptr) >= 0)
        return;

    ptr->setParent(this);

    connect(ptr, &Attachment::aboutToDelete, this, &Note::removeAttachment);
    connect(ptr, &Attachment::attachmentModified, this, &Note::noteModified);

    m_attachments.append(ptr);
    emit attachmentCountChanged();
}

void Note::setAttachments(const QList<Attachment *> &list)
{
    if(!m_attachments.isEmpty())
        return;

    for(Attachment *ptr : list)
    {
        ptr->setParent(this);
        connect(ptr, &Attachment::aboutToDelete, this, &Note::removeAttachment);
        connect(ptr, &Attachment::attachmentModified, this, &Note::noteModified);
    }

    m_attachments.assign(list);
    emit attachmentCountChanged();
}

void Note::removeAttachment(Attachment *ptr)
{
    if(ptr == nullptr)
        return;

    const int index = m_attachments.indexOf(ptr);
    if(index < 0)
        return;

    disconnect(ptr, &Attachment::aboutToDelete, this, &Note::removeAttachment);
    disconnect(ptr, &Attachment::attachmentModified, this, &Note::noteModified);

    m_attachments.removeAt(index);
    emit attachmentCountChanged();

    ptr->deleteLater();
}

Attachment *Note::attachmentAt(int index) const
{
    return index < 0 || index >= m_attachments.size() ? nullptr : m_attachments.at(index);
}

void Note::clearAttachments()
{
    while(m_attachments.size())
        this->removeAttachment(m_attachments.first());
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

bool Note::canSetPropertyFromObjectList(const QString &propName) const
{
    if(propName == QStringLiteral("attachments"))
        return m_attachments.isEmpty();

    return false;
}

void Note::setPropertyFromObjectList(const QString &propName, const QList<QObject *> &objects)
{
    if(propName == QStringLiteral("attachments"))
        this->setAttachments( qobject_list_cast<Attachment*>(objects) );
}

void Note::staticAppendAttachment(QQmlListProperty<Attachment> *list, Attachment *ptr)
{
    reinterpret_cast< Note* >(list->data)->addAttachment(ptr);
}

void Note::staticClearAttachments(QQmlListProperty<Attachment> *list)
{
    reinterpret_cast< Note* >(list->data)->clearAttachments();
}

Attachment *Note::staticAttachmentAt(QQmlListProperty<Attachment> *list, int index)
{
    return reinterpret_cast< Note* >(list->data)->attachmentAt(index);
}

int Note::staticAttachmentCount(QQmlListProperty<Attachment> *list)
{
    return reinterpret_cast< Note* >(list->data)->attachmentCount();
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
    return index < 0 || index >= this->size() ? nullptr : this->at(index);
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

    QList<Note*> notes;
    if(jsNotes.size() == 1)
    {
        const QJsonObject &jsNote = jsNotes.first().toObject();

        Note *note = new Note(this);
        if( QObjectSerializer::fromJson(jsNote, note) )
            this->append(note);
        else
            delete note;
    }
    else
    {
        // Remove duplicate notes.
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

