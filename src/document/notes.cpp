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

#include "user.h"
#include "form.h"
#include "notes.h"
#include "scene.h"
#include "undoredo.h"
#include "structure.h"
#include "screenplay.h"
#include "application.h"
#include "timeprofiler.h"
#include "deltadocument.h"
#include "scritedocument.h"
#include "screenplaytextdocument.h"

#include <QSet>
#include <QUuid>

typedef QHash<QString, Note *> IdNoteMapType;
Q_GLOBAL_STATIC(IdNoteMapType, GlobalIdNoteMap)

Note *Note::findById(const QString &id)
{
    return ::GlobalIdNoteMap->value(id);
}

Note::Note(QObject *parent) : QObject(parent), m_form(this, "form")
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
    ::GlobalIdNoteMap->remove(m_id);
    emit aboutToDelete(this);
}

Notes *Note::notes() const
{
    return qobject_cast<Notes *>(this->parent());
}

QString Note::id() const
{
    if (m_id.isEmpty()) {
        Note *that = const_cast<Note *>(this);
        that->m_id = QUuid::createUuid().toString();
        ::GlobalIdNoteMap->insert(m_id, that);
    }

    return m_id;
}

void Note::setId(const QString &val)
{
    if (m_id == val || !m_id.isEmpty())
        return;

    m_id = val;
    ::GlobalIdNoteMap->insert(m_id, this);
    emit idChanged();
}

void Note::setType(Type val)
{
    if (m_type == val)
        return;

    m_type = val;
    emit typeChanged();
}

void Note::setForm(Form *val)
{
    if (m_form == val)
        return;

    if (!m_form.isNull())
        ScriteDocument::instance()->releaseForm(m_form);

    m_form = val;

    if (!m_form.isNull()) {
        if (m_title.isEmpty())
            this->setTitle(m_form->title());
        if (m_summary.isEmpty())
            this->setSummary(m_form->subtitle());

        if (m_formData.isEmpty()) {
            m_formData = m_form->formDataTemplate();
            emit formDataChanged();
        }

        if (m_summary.isEmpty()) {
            QStringList questions;
            const QList<FormQuestion *> formQuestions = m_form->questionsModel()->list();
            for (const FormQuestion *formQuestion : formQuestions)
                questions << formQuestion->questionText();

            m_summary = QStringLiteral("Form with ") + QString::number(questions.size())
                    + QStringLiteral(" question(s). ");
            m_summary += questions.join(QStringLiteral(", "));
            m_autoSummaryText = true;
            emit summaryChanged();
        }

        User::instance()->logActivity2(
                QStringLiteral("notebookform"),
                QJsonObject({ { QStringLiteral("id"), m_form->id() },
                              { QStringLiteral("name"), m_form->title() } }));
    } else {
        if (!m_formData.isEmpty()) {
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
    if (m_title == val)
        return;

    m_title = val;
    emit titleChanged();
}

void Note::setSummary(const QString &val)
{
    if (m_summary == val)
        return;

    m_summary = val;
    m_autoSummaryText = false;
    emit summaryChanged();
}

void Note::setContent(const QJsonValue &val)
{
    if (m_content == val)
        return;

    m_content = val;
    emit contentChanged();
}

void Note::setColor(const QColor &val)
{
    if (m_color == val)
        return;

    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "color");
    QScopedPointer<PushObjectPropertyUndoCommand> cmd;
    if (!info->isLocked())
        cmd.reset(new PushObjectPropertyUndoCommand(this, info->property));

    m_color = val;
    emit colorChanged();
}

void Note::setFormId(const QString &val)
{
    QString val2 = val;
    Form *form = ScriteDocument::instance()->requestForm(val2);
    if (form == nullptr)
        val2.clear();

    if (m_formId == val2)
        return;

    m_formId = val2;
    emit formIdChanged();

    this->setForm(form);
}

void Note::setFormData(const QJsonObject &val)
{
    if (m_formData == val)
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
    if (!m_form.isNull())
        m_form->validateFormData(m_formData);
}

void Note::serializeToJson(QJsonObject &json) const
{
    json.insert(QStringLiteral("id"), this->id());

    if (m_type == TextNoteType)
        json.insert(QStringLiteral("type"), QStringLiteral("Text"));
    else if (m_type == FormNoteType) {
        json.insert(QStringLiteral("type"), QStringLiteral("Form"));
        json.insert(QStringLiteral("formId"), m_formId);
    }
}

void Note::deserializeFromJson(const QJsonObject &json)
{
    this->setId(json.value(QStringLiteral("id")).toString());

    const QString type = json.value(QStringLiteral("type")).toString();
    if (type == QStringLiteral("Text"))
        this->setType(TextNoteType);
    else if (type == QStringLiteral("Form")) {
        this->setType(FormNoteType);
        this->setFormId(json.value(QStringLiteral("formId")).toString());
    }
}

void Note::write(QTextCursor &cursor, const WriteOptions &options) const
{
    bool insertBlock = !cursor.block().text().isEmpty();

    if (options.includeTitle) {
        QTextBlockFormat headingBlockFormat;
        headingBlockFormat.setTopMargin(10);
        headingBlockFormat.setHeadingLevel(options.titleHeadingLevel);

        QTextCharFormat headingCharFormat;
        headingCharFormat.setFontWeight(QFont::Bold);
        headingCharFormat.setFontPointSize(
                ScreenplayTextDocument::headingFontPointSize(options.titleHeadingLevel));
        headingBlockFormat.setTopMargin(headingCharFormat.fontPointSize() / 2);

        if (insertBlock) {
            cursor.insertBlock(headingBlockFormat, headingCharFormat);
        } else {
            cursor.setBlockFormat(headingBlockFormat);
            cursor.setBlockCharFormat(headingCharFormat);
        }

        cursor.insertText(m_title);
        insertBlock = true;
    }

    if (options.includeSummary && !m_autoSummaryText) {
        QTextBlockFormat summaryBlockFormat;
        summaryBlockFormat.setIndent(1);

        QTextCharFormat summaryCharFormat;
        summaryCharFormat.setFontWeight(QFont::Normal);

        if (insertBlock) {
            cursor.insertBlock(summaryBlockFormat, summaryCharFormat);
        } else {
            cursor.setBlockFormat(summaryBlockFormat);
            cursor.setBlockCharFormat(summaryCharFormat);
        }

        cursor.insertText(m_summary);
        insertBlock = true;
    }

    if (m_type == TextNoteType) {
        QTextBlockFormat contentBlockFormat;
        contentBlockFormat.setIndent(0);

        QTextCharFormat contentCharFormat;
        contentCharFormat.setFontWeight(QFont::Normal);

        if (insertBlock) {
            cursor.insertBlock(contentBlockFormat, contentCharFormat);
        } else {
            cursor.setBlockFormat(contentBlockFormat);
            cursor.setBlockCharFormat(contentCharFormat);
        }

        if (m_content.isString())
            cursor.insertText(m_content.toString());
        else {
            const DeltaDocument::ResolveResult result =
                    DeltaDocument::blockingResolve(m_content.toObject());
            if (!result.htmlText.isEmpty())
                cursor.insertHtml(result.htmlText);
        }
    } else if (m_type == FormNoteType) {
        if (m_form != nullptr) {
            for (int i = 0; i < m_form->questionCount(); i++) {
                const FormQuestion *question = m_form->questionAt(i);
                const QString answer = m_formData.value(question->id()).toString();

                if (options.includeAnsweredFormQuestionsOnly && answer.isEmpty())
                    continue;

                QTextDocument qdoc;
                qdoc.setHtml(question->questionText());

                QTextBlockFormat questionBlockFormat;
                questionBlockFormat.setTopMargin(8);

                QTextCharFormat questionCharFormat;
                questionCharFormat.setFontWeight(QFont::Bold);
                cursor.insertBlock(questionBlockFormat, questionCharFormat);
                cursor.insertText(question->number() + QLatin1String(". ")
                                  + qdoc.toPlainText().trimmed());

                if (options.includeFormAnswerHints && !question->answerHint().isEmpty()) {
                    QTextBlockFormat answerHintBlockFormat;
                    answerHintBlockFormat.setTopMargin(5);
                    answerHintBlockFormat.setIndent(2);
                    QTextCharFormat answerHintCharFormat;
                    answerHintCharFormat.setFontWeight(QFont::Normal);
                    answerHintCharFormat.setFontItalic(true);
                    cursor.insertBlock(answerHintBlockFormat, answerHintCharFormat);
                    cursor.insertText(question->answerHint().trimmed());
                }

                QTextBlockFormat answerBlockFormat;
                answerBlockFormat.setTopMargin(5);
                answerBlockFormat.setIndent(1);
                QTextCharFormat answerCharFormat;
                answerCharFormat.setFontWeight(QFont::Normal);
                answerCharFormat.setFontItalic(false);
                cursor.insertBlock(answerBlockFormat, answerCharFormat);
                cursor.insertText(answer.trimmed());
            }
        }
    }
}

void Note::renameCharacter(const QString &from, const QString &to)
{
    {
        int nrReplacements = 0;
        const QString newTitle =
                Application::replaceCharacterName(from, to, m_title, &nrReplacements);
        if (nrReplacements > 0) {
            m_title = newTitle;
            emit titleChanged();
        }
    }

    if (m_content.isString()) {
        int nrReplacements = 0;
        const QString newContent =
                Application::replaceCharacterName(from, to, m_content.toString(), &nrReplacements);
        if (nrReplacements > 0) {
            m_content = newContent;
            emit contentChanged();
        }
    }

    {
        bool formDataModified = false;
        QJsonObject::iterator it = m_formData.begin();
        QJsonObject::iterator end = m_formData.end();
        while (it != end) {
            int nrReplacements = 0;
            const QString newValue = Application::replaceCharacterName(
                    from, to, it.value().toString(), &nrReplacements);
            if (nrReplacements > 0) {
                it.value() = newValue;
                formDataModified = true;
            }
            ++it;
        }

        if (formDataModified)
            emit formDataChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////

class RemoveNoteUndoCommand : public QUndoCommand
{
public:
    explicit RemoveNoteUndoCommand(Notes *notes, Note *note)
        : QUndoCommand(), m_note(note), m_notes(notes)
    {
        m_connection1 = QObject::connect(m_notes, &Notes::aboutToDelete, m_notes, [=]() {
            m_notes = nullptr;
            this->setObsolete(true);
        });
        m_connection2 =
                QObject::connect(m_note, &Note::aboutToDelete, m_note, [=]() { m_note = nullptr; });
    }
    ~RemoveNoteUndoCommand()
    {
        QObject::disconnect(m_connection1);
        QObject::disconnect(m_connection2);
    }

    static QPointer<Note> noteCurrentlyBeingRemoved;

    void redo()
    {
        if (m_note == nullptr || m_notes == nullptr) {
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
    void undo()
    {
        if (m_notes == nullptr) {
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

typedef QHash<QString, Notes *> IdNotesMapType;
Q_GLOBAL_STATIC(IdNotesMapType, GlobalIdNotesMap)

Notes *Notes::findById(const QString &id)
{
    return ::GlobalIdNotesMap->value(id);
}

Notes::Notes(QObject *parent) : QObjectListModel<Note *>(parent)
{
    connect(this, &Notes::objectCountChanged, this, &Notes::noteCountChanged);
    connect(this, &Notes::noteCountChanged, this, &Notes::notesModified);

    m_compatibleFormType = Form::GeneralForm;

    if (parent != nullptr) {
        const QMetaObject *pmo = parent->metaObject();
        if (pmo->inherits(&Structure::staticMetaObject)) {
            m_ownerType = StructureOwner;
            m_compatibleFormType = Form::StoryForm;
        } else if (pmo->inherits(&ScreenplayElement::staticMetaObject)) {
            m_ownerType = BreakOwner;
            m_compatibleFormType = Form::SceneForm;
        } else if (pmo->inherits(&Scene::staticMetaObject)) {
            m_ownerType = SceneOwner;
            m_compatibleFormType = Form::SceneForm;

            Scene *scene = (qobject_cast<Scene *>(parent));
            m_color = scene->color();
            connect(scene, &Scene::colorChanged, this, [=]() { this->setColor(scene->color()); });
        } else if (pmo->inherits(&Character::staticMetaObject)) {
            m_ownerType = CharacterOwner;
            m_compatibleFormType = Form::CharacterForm;

            Character *character = (qobject_cast<Character *>(parent));
            m_color = character->color();
            connect(character, &Character::colorChanged, this,
                    [=]() { this->setColor(character->color()); });
        } else if (pmo->inherits(&Relationship::staticMetaObject)) {
            m_ownerType = RelationshipOwner;
            m_compatibleFormType = Form::RelationshipForm;
        }
        /*else if(pmo->inherits(&Character::staticMetaObject))
            m_ownerType = CharacterOwner;
        else if(pmo->inherits(&Prop::staticMetaObject))
            m_ownerType = PropOwner;*/
        else {
            m_ownerType = LocationOwner;
            m_compatibleFormType = Form::LocationForm;
        }
    }
}

Notes::~Notes()
{
    ::GlobalIdNotesMap->remove(m_id);
    emit aboutToDelete(this);
}

Notes::OwnerType Notes::ownerType() const
{
    return m_ownerType;
}

Structure *Notes::structure() const
{
    return qobject_cast<Structure *>(this->owner());
}

ScreenplayElement *Notes::breakElement() const
{
    return qobject_cast<ScreenplayElement *>(this->owner());
}

Scene *Notes::scene() const
{
    return qobject_cast<Scene *>(this->owner());
}

Character *Notes::character() const
{
    return qobject_cast<Character *>(this->owner());
}

Relationship *Notes::relationship() const
{
    return qobject_cast<Relationship *>(this->owner());
}

QString Notes::id() const
{
    if (m_id.isEmpty()) {
        Notes *that = const_cast<Notes *>(this);
        that->m_id = QUuid::createUuid().toString();
        ::GlobalIdNotesMap->insert(m_id, that);
    }

    return m_id;
}

QString Notes::title() const
{
    switch (m_ownerType) {
    case SceneOwner: {
        const Scene *scene = this->scene();
        const QList<int> idxList = scene->screenplayElementIndexList();
        return (idxList.isEmpty() ? QStringLiteral("[-1]: ")
                                  : QStringLiteral("[") + QString::number(idxList.first())
                                + QStringLiteral("]: "))
                + scene->heading()->text();
    } break;
    case StructureOwner:
        return QStringLiteral("Story");
    case BreakOwner:
        return this->breakElement()->breakTitle() + QStringLiteral(": ")
                + this->breakElement()->breakSubtitle();
    case CharacterOwner:
        return this->character()->name();
    default:
        break;
    }

    return QStringLiteral("Notes");
}

QString Notes::summary() const
{
    switch (m_ownerType) {
    case SceneOwner: {
        const Scene *scene = this->scene();
        return scene->title();
    } break;
    case BreakOwner:
        return this->breakElement()->breakSummary();
    case CharacterOwner:
        return QStringList({ this->character()->designation(), this->character()->gender(),
                             this->character()->age() })
                .join(QStringLiteral(", "));
    default:
        break;
    }

    return QString();
}

void Notes::setColor(const QColor &val)
{
    if (m_color == val)
        return;

    m_color = val;
    emit colorChanged();

    switch (m_ownerType) {
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
    if (ptr->form() == nullptr) {
        delete ptr;
        return nullptr;
    }

    this->addNote(ptr);
    return ptr;
}

void Notes::setId(const QString &val)
{
    if (m_id == val || !m_id.isEmpty())
        return;

    m_id = val;
    ::GlobalIdNotesMap->insert(m_id, this);
    emit idChanged();
}

void Notes::addNote(Note *ptr)
{
    if (ptr == nullptr || this->indexOf(ptr) >= 0)
        return;

    ptr->setParent(this);

    connect(ptr, &Note::aboutToDelete, this, &Notes::removeNote);
    connect(ptr, &Note::noteModified, this, &Notes::notesModified);

    this->append(ptr);
}

void Notes::setNotes(const QList<Note *> &list)
{
    if (!this->isEmpty())
        return;

    for (Note *ptr : list) {
        ptr->setParent(this);
        connect(ptr, &Note::aboutToDelete, this, &Notes::removeNote);
        connect(ptr, &Note::noteModified, this, &Notes::notesModified);
    }

    this->assign(list);
}

void Notes::removeNote(Note *ptr)
{
    if (ptr == nullptr)
        return;

    const int index = this->indexOf(ptr);
    if (index < 0)
        return;

    disconnect(ptr, &Note::aboutToDelete, this, &Notes::removeNote);
    disconnect(ptr, &Note::noteModified, this, &Notes::notesModified);

    if (ptr->type() == Note::FormNoteType && ptr->form() != nullptr)
        ScriteDocument::instance()->releaseForm(ptr->form());

    this->removeAt(index);

    if (UndoStack::active() && RemoveNoteUndoCommand::noteCurrentlyBeingRemoved.isNull())
        UndoStack::active()->push(new RemoveNoteUndoCommand(this, ptr));

    ptr->deleteLater();
}

Note *Notes::noteAt(int index) const
{
    return this->at(index);
}

void Notes::clearNotes()
{
    while (this->size())
        this->removeNote(this->first());
}

void Notes::serializeToJson(QJsonObject &json) const
{
    json.insert(QStringLiteral("id"), this->id());

    if (this->isEmpty())
        return;

    QJsonArray jsNotes;

    const QList<Note *> &notes = this->list();
    for (Note *note : notes) {
        QJsonObject jsNote = QObjectSerializer::toJson(note);
        jsNotes.append(jsNote);
    }

    json.insert(QStringLiteral("#data"), jsNotes);
}

void Notes::deserializeFromJson(const QJsonObject &json)
{
    this->setId(json.value(QStringLiteral("id")).toString());

    const QJsonArray jsNotes = json.value(QStringLiteral("#data")).toArray();
    if (jsNotes.isEmpty())
        return;

    // Remove duplicate notes.
    const QString idAttr = QStringLiteral("id");

    QList<Note *> notes;
    QList<QJsonObject> uniqueJsNotes;
    for (const QJsonValue &jsNoteItemValue : jsNotes) {
        const QJsonObject jsNoteItem = jsNoteItemValue.toObject();
        const QString jsNoteId = jsNoteItem.value(idAttr).toString();
        bool jsNoteIsUnique = true;
        for (const QJsonObject &uniqueJsNote : qAsConst(uniqueJsNotes)) {
            if (uniqueJsNote.value(idAttr).toString() == jsNoteId) {
                jsNoteIsUnique = false;
                break;
            }
        }

        if (jsNoteIsUnique)
            uniqueJsNotes.append(jsNoteItem);
    }

    notes.reserve(jsNotes.size());
    for (const QJsonObject &jsNote : qAsConst(uniqueJsNotes)) {
        Note *note = new Note(this);
        if (QObjectSerializer::fromJson(jsNote, note))
            notes.append(note);
        else
            delete note;
    }

    this->setNotes(notes);
}

void Notes::loadOldNotes(const QJsonArray &jsNotes)
{
    if (jsNotes.isEmpty() || !this->isEmpty())
        return;

    QList<Note *> notes;
    notes.reserve(jsNotes.size());
    for (const QJsonValue &jsNotesItem : jsNotes) {
        const QJsonObject jsNote = jsNotesItem.toObject();
        Note *note = new Note(this);
        note->setType(Note::TextNoteType);
        note->setColor(QColor(jsNote.value(QStringLiteral("color")).toString()));
        note->setTitle(jsNote.value(QStringLiteral("heading")).toString());
        note->setContent(jsNote.value(QStringLiteral("content")));
        notes.append(note);
    }

    this->setNotes(notes);
}

void Notes::write(QTextCursor &cursor, const WriteOptions &options) const
{
    for (Note *note : this->constList()) {
        if ((options.includeTextNotes && note->type() == Note::TextNoteType)
            || (options.includeFormNotes && note->type() == Note::FormNoteType))
            note->write(cursor);
    }
}

void Notes::renameCharacter(const QString &from, const QString &to)
{
    for (Note *note : this->constList())
        note->renameCharacter(from, to);
}
