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

#ifndef NOTES_H
#define NOTES_H

#include <QJsonValue>
#include <QQmlEngine>
#include <QQmlListProperty>
#include <QAbstractListModel>

#include "attachments.h"
#include "qobjectproperty.h"

class Form;
class Prop;
class Notes;
class Scene;
class Location;
class Character;
class Structure;
class Attachments;
class Relationship;
class ScreenplayElement;

class Note : public QObject, public QObjectSerializer::Interface
{
    Q_OBJECT
    Q_INTERFACES(QObjectSerializer::Interface)
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    static Note *findById(const QString &id);

    explicit Note(QObject *parent = nullptr);
    ~Note();
    Q_SIGNAL void aboutToDelete(Note *ptr);

    // clang-format off
    Q_PROPERTY(Notes *notes
               READ notes
               CONSTANT STORED
               false )
    // clang-format on
    Notes *notes() const;

    // clang-format off
    Q_PROPERTY(QString id
               READ id
               NOTIFY idChanged)
    // clang-format on
    QString id() const;
    Q_SIGNAL void idChanged();

    enum Type { TextNoteType, FormNoteType, CheckListNoteType };
    Q_ENUM(Type)
    // clang-format off
    Q_PROPERTY(Type type
               READ type
               NOTIFY typeChanged)
    // clang-format on
    Type type() const { return m_type; }
    Q_SIGNAL void typeChanged();

    // clang-format off
    Q_PROPERTY(QString title
               READ title
               WRITE setTitle
               NOTIFY titleChanged)
    // clang-format on
    void setTitle(const QString &val);
    QString title() const { return m_title; }
    Q_SIGNAL void titleChanged();

    // clang-format off
    Q_PROPERTY(QString summary
               READ summary
               WRITE setSummary
               NOTIFY summaryChanged
               STORED false)
    // clang-format on
    void setSummary(const QString &val);
    QString summary() const { return m_summary; }
    Q_SIGNAL void summaryChanged();

    // clang-format off
    Q_PROPERTY(QJsonValue content
               READ content
               WRITE setContent
               NOTIFY contentChanged)
    // clang-format on
    void setContent(const QJsonValue &val);
    QJsonValue content() const { return m_content; }
    Q_SIGNAL void contentChanged();

    // clang-format off
    Q_PROPERTY(QColor color
               READ color
               WRITE setColor
               NOTIFY colorChanged)
    // clang-format on
    void setColor(const QColor &val);
    QColor color() const { return m_color; }
    Q_SIGNAL void colorChanged();

    // clang-format off
    Q_PROPERTY(QString formId
               READ formId
               NOTIFY formIdChanged)
    // clang-format on
    QString formId() const { return m_formId; }
    Q_SIGNAL void formIdChanged();

    // clang-format off
    Q_PROPERTY(Form *form
               READ form
               RESET resetForm
               NOTIFY formChanged
               STORED false)
    // clang-format on
    Form *form() const { return m_form; }
    Q_SIGNAL void formChanged();

    // clang-format off
    Q_PROPERTY(QJsonObject formData
               READ formData
               WRITE setFormData
               NOTIFY formDataChanged)
    // clang-format on
    void setFormData(const QJsonObject &val);
    QJsonObject formData() const { return m_formData; }
    Q_SIGNAL void formDataChanged();

    Q_INVOKABLE void setFormData(const QString &key, const QJsonValue &value);
    Q_INVOKABLE QJsonValue getFormData(const QString &key) const;

    // clang-format off
    Q_PROPERTY(Attachments *attachments
               READ attachments
               CONSTANT )
    // clang-format on
    Attachments *attachments() const { return m_attachments; }

    Q_SIGNAL void noteModified();

    // QObjectSerializer::Interface interface
    void prepareForSerialization();
    void serializeToJson(QJsonObject &json) const;
    void deserializeFromJson(const QJsonObject &);

    // Text Document Export Support
    struct WriteOptions
    {
        WriteOptions() { }
        int titleHeadingLevel = 2;
        bool includeTitle = true;
        bool includeSummary = true;
        bool includeFormAnswerHints = true;
        bool includeAnsweredFormQuestionsOnly = true;
    };
    void write(QTextCursor &cursor, const WriteOptions &options = WriteOptions()) const;

private:
    void setId(const QString &val);
    void setType(Type val);
    void setFormId(const QString &val);
    void setForm(Form *val);
    void resetForm();
    void addAttachment(Attachment *ptr);
    void renameCharacter(const QString &from, const QString &to);

private:
    friend class Notes;
    QString m_id;
    QString m_title;
    QString m_formId;
    QString m_summary;
    bool m_autoSummaryText = false;
    QJsonValue m_content;
    QJsonObject m_formData;
    QColor m_color = Qt::white;
    Type m_type = TextNoteType;
    QObjectProperty<Form> m_form;
    Attachments *m_attachments = new Attachments(this);
};

class RemoveNoteUndoCommand;
class Notes : public QObjectListModel<Note *>, public QObjectSerializer::Interface
{
    Q_OBJECT
    Q_INTERFACES(QObjectSerializer::Interface)
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    static Notes *findById(const QString &id);

    explicit Notes(QObject *parent = nullptr);
    ~Notes();
    Q_SIGNAL void aboutToDelete(Notes *ptr);

    enum OwnerType {
        StructureOwner,
        SceneOwner,
        BreakOwner, // Act, Episode etc..
        CharacterOwner,
        RelationshipOwner,
        LocationOwner,
        PropOwner,
        OtherOwner
    };
    Q_ENUM(OwnerType)
    // clang-format off
    Q_PROPERTY(OwnerType ownerType
               READ ownerType
               CONSTANT )
    // clang-format on
    OwnerType ownerType() const;

    // clang-format off
    Q_PROPERTY(QObject *owner
               READ owner
               STORED false
               CONSTANT )
    // clang-format on
    QObject *owner() const { return this->QObject::parent(); }

    // clang-format off
    Q_PROPERTY(Structure *structure
               READ structure
               STORED false
               CONSTANT )
    // clang-format on
    Structure *structure() const;

    // clang-format off
    Q_PROPERTY(ScreenplayElement *breakElement
               READ breakElement
               STORED false
               CONSTANT )
    // clang-format on
    ScreenplayElement *breakElement() const;

    // clang-format off
    Q_PROPERTY(Scene *scene
               READ scene
               STORED false
               CONSTANT )
    // clang-format on
    Scene *scene() const;

    // clang-format off
    Q_PROPERTY(Character *character
               READ character
               STORED false
               CONSTANT )
    // clang-format on
    Character *character() const;

    // clang-format off
    Q_PROPERTY(Relationship *relationship
               READ relationship
               STORED false
               CONSTANT )
    // clang-format on
    Relationship *relationship() const;

    /*
    // clang-format off
    Q_PROPERTY(Location* location
               READ location
               STORED false
               CONSTANT )
    // clang-format on
    Location *location() const { return nullptr; }

    // clang-format off
    Q_PROPERTY(Prop* prop
               READ prop
               STORED false
               CONSTANT )
    // clang-format on
    Prop *prop() const { return nullptr; }
    */

    // clang-format off
    Q_PROPERTY(QString id
               READ id
               NOTIFY idChanged)
    // clang-format on
    QString id() const;
    Q_SIGNAL void idChanged();

    // Called from BookmarkedNotes
    QString title() const;
    QString summary() const;

    // clang-format off
    Q_PROPERTY(QColor color
               READ color
               WRITE setColor
               NOTIFY colorChanged)
    // clang-format on
    void setColor(const QColor &val);
    QColor color() const { return m_color; }
    Q_SIGNAL void colorChanged();

    Q_INVOKABLE Note *addTextNote();
    Q_INVOKABLE Note *addFormNote(const QString &id);
    Q_INVOKABLE Note *addCheckListNote();
    Q_INVOKABLE void removeNote(Note *ptr);
    Q_INVOKABLE Note *noteAt(int index) const;
    Q_INVOKABLE Note *firstNote() const { return this->noteAt(0); }
    Q_INVOKABLE Note *lastNote() const { return this->noteAt(this->noteCount() - 1); }
    Q_INVOKABLE void clearNotes();

    // clang-format off
    Q_PROPERTY(int noteCount
               READ noteCount
               NOTIFY noteCountChanged)
    // clang-format on
    int noteCount() const { return this->objectCount(); }
    Q_SIGNAL void noteCountChanged();

    // clang-format off
    Q_PROPERTY(int compatibleFormType
               READ compatibleFormType
               CONSTANT )
    // clang-format on
    int compatibleFormType() const { return m_compatibleFormType; }

    Q_SIGNAL void notesModified();

    // QObjectSerializer::Interface interface
    void serializeToJson(QJsonObject &json) const;
    void deserializeFromJson(const QJsonObject &);

    // Helper method to port notes from old notes[]
    void loadOldNotes(const QJsonArray &array);

    // Text Document Export Support
    struct WriteOptions
    {
        WriteOptions() { }
        bool includeTextNotes = true;
        bool includeFormNotes = true;
        bool includeCheckListNotes = true;
    };
    void write(QTextCursor &cursor, const WriteOptions &options = WriteOptions()) const;

private:
    friend class Scene;
    friend class Character;
    friend class Structure;
    void setId(const QString &val);
    void addNote(Note *ptr);
    void setNotes(const QList<Note *> &list);
    void moveNotes(Notes *target);
    void renameCharacter(const QString &from, const QString &to);

private:
    friend class RemoveNoteUndoCommand;
    int m_compatibleFormType = -1;
    QString m_id;
    QColor m_color = Qt::white;
    OwnerType m_ownerType = OtherOwner;
};

#endif // NOTES_H
