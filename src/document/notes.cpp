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

#include "form.h"
#include "notes.h"
#include "scene.h"
#include "undoredo.h"
#include "structure.h"
#include "screenplay.h"
#include "timeprofiler.h"
#include "scritedocument.h"

#include <QSet>

Note::Note(QObject *parent)
    : QObject(parent),
      m_form(this, "form")
{
    connect(this, &Note::typeChanged, this, &Note::noteModified);
    connect(this, &Note::titleChanged, this, &Note::noteModified);
    connect(this, &Note::summaryChanged, this, &Note::noteModified);
    connect(this, &Note::contentChanged, this, &Note::noteModified);
    connect(this, &Note::formDataChanged, this, &Note::noteModified);
    connect(m_attachments, &Attachments::attachmentsModified, this, &Note::noteModified);
}

Note::~Note()
{
    emit aboutToDelete(this);
}

Notes *Note::notes() const
{
    return qobject_cast<Notes*>(this->parent());
}

void Note::setType(Type val)
{
    if(m_type == val)
        return;

    m_type = val;
    emit typeChanged();
}

void Note::setForm(Form *val)
{
    if(m_form == val)
        return;

    if(!m_form.isNull())
        ScriteDocument::instance()->releaseForm(m_form);

    m_form = val;

    if(!m_form.isNull())
    {
        this->setTitle(m_form->title());
        this->setSummary(m_form->subtitle());

        if(m_formData.isEmpty())
        {
            m_formData = m_form->formDataTemplate();
            emit formDataChanged();
        }

        if(m_summary.isEmpty())
        {
            QStringList questions;
            const QList<FormQuestion*> formQuestions = m_form->questionsModel()->list();
            for(const FormQuestion *formQuestion : formQuestions)
                questions << formQuestion->questionText();

            m_summary = QStringLiteral("Form with ") + QString::number(questions.size()) + QStringLiteral(" question(s). ");
            m_summary += questions.join( QStringLiteral(", ") );
            emit summaryChanged();
        }
    }
    else
    {
        if(!m_formData.isEmpty())
        {
            m_formData = QJsonObject();
            emit formDataChanged();
        }
    }

    emit formChanged();
}

void Note::resetForm()
{
    m_form = nullptr;
    emit formChanged();
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

    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "color");
    QScopedPointer<PushObjectPropertyUndoCommand> cmd;
    if(!info->isLocked())
        cmd.reset(new PushObjectPropertyUndoCommand(this, info->property));

    m_color = val;
    emit colorChanged();
}

void Note::setFormId(const QString &val)
{
    QString val2 = val;
    Form *form = ScriteDocument::instance()->requestForm(val2);
    if(form == nullptr)
        val2.clear();

    if(m_formId == val2)
        return;

    m_formId = val2;
    emit formIdChanged();

    this->setForm(form);
}

void Note::setFormData(const QJsonObject &val)
{
    if(m_formData == val)
        return;

    m_formData = val;
    emit formDataChanged();
}

void Note::setFormData(const QString &key, const QJsonValue &value)
{
    m_formData.insert(key, value);
    emit formDataChanged();
}

QJsonValue Note::getFormData(const QString &key) const
{
    return m_formData.value(key);
}

void Note::prepareForSerialization()
{
    if(!m_form.isNull())
        m_form->validateFormData(m_formData);
}

void Note::serializeToJson(QJsonObject &json) const
{
    if(m_type == TextNoteType)
        json.insert( QStringLiteral("type"), QStringLiteral("Text") );
    else if(m_type == FormNoteType)
    {
        json.insert( QStringLiteral("type"), QStringLiteral("Form") );
        json.insert( QStringLiteral("formId"), m_formId );
    }
}

void Note::deserializeFromJson(const QJsonObject &json)
{
    const QString type = json.value( QStringLiteral("type") ).toString();
    if(type == QStringLiteral("Text"))
        this->setType(TextNoteType);
    else if(type == QStringLiteral("Form"))
    {
        this->setType(FormNoteType);
        this->setFormId( json.value(QStringLiteral("formId")).toString() );
    }
}

///////////////////////////////////////////////////////////////////////////////

class RemoveNoteUndoCommand : public QUndoCommand
{
public:
    RemoveNoteUndoCommand(Notes *notes, Note *note)
        : QUndoCommand(), m_note(note), m_notes(notes) {
        m_connection1 = QObject::connect(m_notes, &Notes::aboutToDelete, m_notes, [=]() {
            m_notes = nullptr;
            this->setObsolete(true);
        });
        m_connection2 = QObject::connect(m_note, &Note::aboutToDelete, m_note, [=]() {
            m_note = nullptr;
        });
    }
    ~RemoveNoteUndoCommand() {
        QObject::disconnect(m_connection1);
        QObject::disconnect(m_connection2);
    }

    static QPointer<Note> noteCurrentlyBeingRemoved;

    void redo() {
        if(m_note == nullptr || m_notes == nullptr) {
            this->setObsolete(true);
            return;
        }

        m_noteData = QObjectSerializer::toJson(m_note);
        noteCurrentlyBeingRemoved = m_note;
        QObject::disconnect(m_connection2);
        m_connection2 = QMetaObject::Connection();
        m_notes->removeNote(m_note);
        noteCurrentlyBeingRemoved = nullptr;
    }
    void undo() {
        if(m_notes == nullptr) {
            this->setObsolete(true);
            return;
        }

        m_note = new Note(m_notes);
        m_connection2 = QObject::connect(m_note, &Note::aboutToDelete, m_note, [=]() {
            m_note = nullptr;
            m_connection2 = QMetaObject::Connection();
        });
        QObjectSerializer::fromJson(m_noteData, m_note);
        m_notes->addNote(m_note);
    }

private:
    Note *m_note = nullptr;
    Notes *m_notes = nullptr;
    QJsonObject m_noteData;
    QMetaObject::Connection m_connection1;
    QMetaObject::Connection m_connection2;
};

QPointer<Note> RemoveNoteUndoCommand::noteCurrentlyBeingRemoved;

Notes::Notes(QObject *parent)
      :ObjectListPropertyModel<Note *>(parent)
{
    connect(this, &Notes::objectCountChanged, this, &Notes::noteCountChanged);
    connect(this, &Notes::noteCountChanged, this, &Notes::notesModified);

    m_compatibleFormType = Form::GeneralForm;

    if(parent != nullptr)
    {
        const QMetaObject *pmo = parent->metaObject();
        if(pmo->inherits(&Structure::staticMetaObject))
        {
            m_ownerType = StructureOwner;
            m_compatibleFormType = Form::StoryForm;
        }
        else if(pmo->inherits(&ScreenplayElement::staticMetaObject))
        {
            m_ownerType = BreakOwner;
            m_compatibleFormType = Form::SceneForm;
        }
        else if(pmo->inherits(&Scene::staticMetaObject))
        {
            m_ownerType = SceneOwner;
            m_compatibleFormType = Form::SceneForm;

            Scene *scene = (qobject_cast<Scene*>(parent));
            m_color = scene->color();
            connect(scene, &Scene::colorChanged, this, [=]() {
                this->setColor(scene->color());
            });
        }
        else if(pmo->inherits(&Character::staticMetaObject))
        {
            m_ownerType = CharacterOwner;
            m_compatibleFormType = Form::CharacterForm;

            Character *character = (qobject_cast<Character*>(parent));
            m_color = character->color();
            connect(character, &Character::colorChanged, this, [=]() {
                this->setColor(character->color());
            });
        }
        else if(pmo->inherits(&Relationship::staticMetaObject))
        {
            m_ownerType = RelationshipOwner;
            m_compatibleFormType = Form::RelationshipForm;
        }
        /*else if(pmo->inherits(&Character::staticMetaObject))
            m_ownerType = CharacterOwner;
        else if(pmo->inherits(&Prop::staticMetaObject))
            m_ownerType = PropOwner;*/
        else
        {
            m_ownerType = LocationOwner;
            m_compatibleFormType = Form::LocationForm;
        }
    }
}

Notes::~Notes()
{
    emit aboutToDelete(this);
}

Notes::OwnerType Notes::ownerType() const
{
    return m_ownerType;
}

Structure *Notes::structure() const
{
    return qobject_cast<Structure*>(this->owner());
}

ScreenplayElement *Notes::breakElement() const
{
    return qobject_cast<ScreenplayElement*>(this->owner());
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

Note *Notes::addFormNote(const QString &id)
{
    Note *ptr = new Note(this);
    ptr->setType(Note::FormNoteType);
    ptr->setFormId(id);
    if(ptr->form() == nullptr)
    {
        delete ptr;
        return nullptr;
    }

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

    if(UndoStack::active() && RemoveNoteUndoCommand::noteCurrentlyBeingRemoved.isNull())
        UndoStack::active()->push(new RemoveNoteUndoCommand(this, ptr));

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
        note->setType(Note::TextNoteType);
        note->setColor( QColor(jsNote.value(QStringLiteral("color")).toString()) );
        note->setTitle( jsNote.value(QStringLiteral("heading")).toString() );
        note->setContent( jsNote.value(QStringLiteral("content")) );
        notes.append(note);
    }

    this->setNotes(notes);
}


