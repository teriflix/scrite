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

#ifndef NOTES_H
#define NOTES_H

#include <QJsonValue>
#include <QAbstractListModel>
#include <QQmlListProperty>

#include "qobjectserializer.h"
#include "objectlistpropertymodel.h"

class Prop;
class Notes;
class Scene;
class Location;
class Character;
class Structure;
class Relationship;

class Attachment : public QObject, public QObjectSerializer::Interface
{
    Q_OBJECT
    Q_INTERFACES(QObjectSerializer::Interface)

public:
    Attachment(QObject *parent=nullptr);
    ~Attachment();
    Q_SIGNAL void aboutToDelete(Attachment *ptr);

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    void setName(const QString &val);
    QString name() const { return m_name; }
    Q_SIGNAL void nameChanged();

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    void setTitle(const QString &val);
    QString title() const { return m_title; }
    Q_SIGNAL void titleChanged();

    Q_PROPERTY(QString filePath READ filePath NOTIFY filePathChanged)
    QString filePath() const { return m_filePath; }
    Q_SIGNAL void filePathChanged();

    Q_PROPERTY(QString type READ type NOTIFY mimeTypeChanged)
    QString type() const { return m_type; }

    Q_PROPERTY(QUrl typeIcon READ typeIcon NOTIFY mimeTypeChanged)
    QUrl typeIcon() const { return m_typeIcon; }

    Q_PROPERTY(QString mimeType READ mimeType NOTIFY mimeTypeChanged)
    QString mimeType() const { return m_mimeType; }
    Q_SIGNAL void mimeTypeChanged();

    Q_SIGNAL void attachmentModified();

    // QObjectSerializer::Interface interface
    void serializeToJson(QJsonObject &) const;
    void deserializeFromJson(const QJsonObject &);

private:
    friend class Note;
    void setFilePath(const QString &val);
    void setMimeType(const QString &val);

private:
    QString m_name;
    QString m_type;
    QString m_title;
    QUrl    m_typeIcon;
    QString m_filePath;
    QString m_mimeType;
};

class Note : public QObject, public QObjectSerializer::Interface
{
    Q_OBJECT
    Q_INTERFACES(QObjectSerializer::Interface)

public:
    Note(QObject *parent=nullptr);
    ~Note();
    Q_SIGNAL void aboutToDelete(Note *ptr);

    enum Type
    {
        TextNoteType,
        FormNoteType
    };
    Q_ENUM(Type)
    Q_PROPERTY(Type type READ type NOTIFY typeChanged)
    Type type() const { return m_type; }
    Q_SIGNAL void typeChanged();

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    void setTitle(const QString &val);
    QString title() const { return m_title; }
    Q_SIGNAL void titleChanged();

    Q_PROPERTY(QString summary READ summary WRITE setSummary NOTIFY summaryChanged STORED false)
    void setSummary(const QString &val);
    QString summary() const { return m_summary; }
    Q_SIGNAL void summaryChanged();

    Q_PROPERTY(QJsonValue content READ content WRITE setContent NOTIFY contentChanged)
    void setContent(const QJsonValue &val);
    QJsonValue content() const { return m_content; }
    Q_SIGNAL void contentChanged();

    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    void setColor(const QColor &val);
    QColor color() const { return m_color; }
    Q_SIGNAL void colorChanged();

    Q_PROPERTY(QJsonObject metaData READ metaData WRITE setMetaData NOTIFY metaDataChanged)
    void setMetaData(const QJsonObject &val);
    QJsonObject metaData() const { return m_metaData; }
    Q_SIGNAL void metaDataChanged();

    Q_PROPERTY(QAbstractItemModel* attachmentsModel READ attachmentsModel STORED false CONSTANT)
    ObjectListPropertyModel<Attachment *> *attachmentsModel() const;

    Q_PROPERTY(QQmlListProperty<Attachment> attachments READ attachments NOTIFY attachmentCountChanged)
    QQmlListProperty<Attachment> attachments();
    Q_INVOKABLE Attachment *addAttachment(const QString &filePath);
    Q_INVOKABLE void removeAttachment(Attachment *ptr);
    Q_INVOKABLE Attachment *attachmentAt(int index) const;
    Q_PROPERTY(int attachmentCount READ attachmentCount NOTIFY attachmentCountChanged)
    int attachmentCount() const { return m_attachments.size(); }
    Q_INVOKABLE void clearAttachments();
    Q_SIGNAL void attachmentCountChanged();

    Q_SIGNAL void noteModified();

    // QObjectSerializer::Interface interface
    bool canSerialize(const QMetaObject *metaObject, const QMetaProperty &metaProperty) const;
    bool canSetPropertyFromObjectList(const QString &propName) const;
    void setPropertyFromObjectList(const QString &propName, const QList<QObject*> &objects);

private:
    void setType(Type val);
    void addAttachment(Attachment *ptr);
    void setAttachments(const QList<Attachment*> &list);

    static void staticAppendAttachment(QQmlListProperty<Attachment> *list, Attachment *ptr);
    static void staticClearAttachments(QQmlListProperty<Attachment> *list);
    static Attachment* staticAttachmentAt(QQmlListProperty<Attachment> *list, int index);
    static int staticAttachmentCount(QQmlListProperty<Attachment> *list);
    ObjectListPropertyModel<Attachment *> m_attachments;

private:
    friend class Notes;
    QString m_title;
    QString m_summary;
    QJsonValue m_content;
    QJsonObject m_metaData;
    QColor m_color = Qt::white;
    Type m_type = TextNoteType;
};

class Notes : public ObjectListPropertyModel<Note *>, public QObjectSerializer::Interface
{
    Q_OBJECT
    Q_INTERFACES(QObjectSerializer::Interface)

public:
    Notes(QObject *parent = nullptr);
    ~Notes();

    enum OwnerType
    {
        StructureOwner,
        SceneOwner,
        CharacterOwner,
        RelationshipOwner,
        LocationOwner,
        PropOwner,
        OtherOwner
    };
    Q_ENUM(OwnerType)
    Q_PROPERTY(OwnerType ownerType READ ownerType CONSTANT)
    OwnerType ownerType() const;

    Q_PROPERTY(QObject* owner READ owner STORED false CONSTANT)
    QObject *owner() const { return this->QObject::parent(); }

    Q_PROPERTY(Structure* structure READ structure STORED false CONSTANT)
    Structure *structure() const;

    Q_PROPERTY(Scene* scene READ scene STORED false CONSTANT)
    Scene *scene() const;

    Q_PROPERTY(Character* character READ character STORED false CONSTANT)
    Character *character() const;

    Q_PROPERTY(Relationship* relationship READ relationship STORED false CONSTANT)
    Relationship *relationship() const;

    /*
    Q_PROPERTY(Location* location READ location STORED false CONSTANT)
    Location *location() const { return nullptr; }

    Q_PROPERTY(Prop* prop READ prop STORED false CONSTANT)
    Prop *prop() const { return nullptr; }
    */

    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    void setColor(const QColor &val);
    QColor color() const { return m_color; }
    Q_SIGNAL void colorChanged();

    Q_INVOKABLE Note *addTextNote();
    Q_INVOKABLE void removeNote(Note *ptr);
    Q_INVOKABLE Note *noteAt(int index) const;
    Q_INVOKABLE Note *firstNote() const { return this->noteAt(0); }
    Q_INVOKABLE Note *lastNote() const { return this->noteAt(this->noteCount()-1); }
    Q_INVOKABLE void clearNotes();

    Q_PROPERTY(int noteCount READ noteCount NOTIFY noteCountChanged)
    int noteCount() const { return this->objectCount(); }
    Q_SIGNAL void noteCountChanged();

    Q_SIGNAL void notesModified();

    // QObjectSerializer::Interface interface
    void serializeToJson(QJsonObject &json) const;
    void deserializeFromJson(const QJsonObject &);

    // Helper method to port notes from old notes[]
    void loadOldNotes(const QJsonArray &array);

private:
    void addNote(Note *ptr);
    void setNotes(const QList<Note*> &list);

private:
    QColor m_color = Qt::white;
    OwnerType m_ownerType = OtherOwner;
};

#endif // NOTES_H
