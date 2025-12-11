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

#ifndef STRUCTURE_H
#define STRUCTURE_H

#include "notes.h"
#include "scene.h"
#include "attachments.h"
#include "execlatertimer.h"
#include "modelaggregator.h"
#include "qobjectproperty.h"
#include "abstractshapeitem.h"
#include "qobjectlistmodel.h"

#include <QColor>
#include <QPointer>
#include <QJsonArray>
#include <QJsonObject>
#include <QUndoCommand>
#include <QStringListModel>
#include <QSortFilterProxyModel>

class Structure;
class Character;
class Screenplay;
class SceneHeading;
class ScriteDocument;
class StructureLayout;
class StructureExporter;
class ScreenplayElement;
class StructureElementStack;
class StructureElementStacks;
class StructurePositionCommand;

class StructureElement : public QObject, public QObjectSerializer::Interface
{
    Q_OBJECT
    Q_INTERFACES(QObjectSerializer::Interface)
    QML_ELEMENT

public:
    Q_INVOKABLE explicit StructureElement(QObject *parent = nullptr);
    ~StructureElement();
    Q_SIGNAL void aboutToDelete(StructureElement *element);

    Q_INVOKABLE StructureElement *duplicate();

    // clang-format off
    Q_PROPERTY(Structure *structure
               READ structure
               CONSTANT STORED
               false )
    // clang-format on
    Structure *structure() const { return m_structure; }

    // clang-format off
    Q_PROPERTY(qreal x
               READ x
               WRITE setX
               NOTIFY xChanged
               STORED false)
    // clang-format on
    void setX(qreal val);
    qreal x() const { return m_x; }
    Q_SIGNAL void xChanged();

    // clang-format off
    Q_PROPERTY(qreal y
               READ y
               WRITE setY
               NOTIFY yChanged
               STORED false)
    // clang-format on
    void setY(qreal val);
    qreal y() const { return m_y; }
    Q_SIGNAL void yChanged();

    // clang-format off
    Q_PROPERTY(qreal width
               READ width
               WRITE setWidth
               NOTIFY widthChanged
               STORED false)
    // clang-format on
    void setWidth(qreal val);
    qreal width() const { return m_width; }
    Q_SIGNAL void widthChanged();

    // clang-format off
    Q_PROPERTY(qreal height
               READ height
               WRITE setHeight
               NOTIFY heightChanged
               STORED false)
    // clang-format on
    void setHeight(qreal val);
    qreal height() const { return m_height; }
    Q_SIGNAL void heightChanged();

    // clang-format off
    Q_PROPERTY(QRectF geometry
               READ geometry
               NOTIFY geometryChanged)
    // clang-format on
    QRectF geometry() const { return QRectF(m_x, m_y, m_width, m_height); }
    Q_SIGNAL void geometryChanged();

    // clang-format off
    Q_PROPERTY(QQuickItem *follow
               READ follow
               WRITE setFollow
               NOTIFY followChanged
               RESET resetFollow
               STORED false)
    // clang-format on
    void setFollow(QQuickItem *val);
    QQuickItem *follow() const { return m_follow; }
    Q_SIGNAL void followChanged();

    // clang-format off
    Q_PROPERTY(bool undoRedoEnabled
               READ isUndoRedoEnabled
               WRITE setUndoRedoEnabled
               NOTIFY undoRedoEnabledChanged)
    // clang-format on
    void setUndoRedoEnabled(bool val);
    bool isUndoRedoEnabled() const { return m_undoRedoEnabled; }
    Q_SIGNAL void undoRedoEnabledChanged();

    // clang-format off
    Q_PROPERTY(bool syncWithFollow
               READ isSyncWithFollow
               WRITE setSyncWithFollow
               NOTIFY syncWithFollowChanged
               STORED false)
    // clang-format on
    void setSyncWithFollow(bool val);
    bool isSyncWithFollow() const { return m_syncWithFollow; }
    Q_SIGNAL void syncWithFollowChanged();

    // clang-format off
    Q_PROPERTY(qreal xf
               READ xf
               WRITE setXf
               NOTIFY xfChanged)
    // clang-format on
    void setXf(qreal val);
    qreal xf() const;
    Q_SIGNAL void xfChanged();

    // clang-format off
    Q_PROPERTY(qreal yf
               READ yf
               WRITE setYf
               NOTIFY yfChanged)
    // clang-format on
    void setYf(qreal val);
    qreal yf() const;
    Q_SIGNAL void yfChanged();

    // clang-format off
    Q_PROPERTY(QPointF position
               READ position
               WRITE setPosition
               NOTIFY positionChanged
               STORED false)
    // clang-format on
    void setPosition(const QPointF &pos);
    QPointF position() const { return QPointF(m_x, m_y); }
    Q_SIGNAL void positionChanged();

    // clang-format off
    Q_PROPERTY(Scene *scene
               READ scene
               WRITE setScene
               NOTIFY sceneChanged)
    // clang-format on
    void setScene(Scene *val);
    Scene *scene() const { return m_scene; }
    Q_SIGNAL void sceneChanged();

    // clang-format off
    Q_PROPERTY(QString title
               READ title
               WRITE setTitle
               NOTIFY titleChanged)
    // clang-format on
    void setTitle(const QString &val);
    QString title() const;
    Q_SIGNAL void titleChanged();

    // clang-format off
    Q_PROPERTY(QString nativeTitle
               READ nativeTitle
               NOTIFY titleChanged)
    // clang-format on
    QString nativeTitle() const { return m_title; }

    // clang-format off
    Q_PROPERTY(bool hasTitle
               READ hasTitle
               NOTIFY titleChanged)
    // clang-format on
    bool hasTitle() const { return !this->title().isEmpty(); }

    // clang-format off
    Q_PROPERTY(bool hasNativeTitle
               READ hasNativeTitle
               NOTIFY titleChanged)
    // clang-format on
    bool hasNativeTitle() const { return !m_title.isEmpty(); }

    // clang-format off
    Q_PROPERTY(bool selected
               READ isSelected
               WRITE setSelected
               NOTIFY selectedChanged
               STORED false)
    // clang-format on
    void setSelected(bool val);
    bool isSelected() const { return m_selected; }
    Q_SIGNAL void selectedChanged();

    // clang-format off
    Q_PROPERTY(QString stackId
               READ stackId
               WRITE setStackId
               NOTIFY stackIdChanged)
    // clang-format on
    void setStackId(const QString &val);
    QString stackId() const { return m_stackId; }
    Q_SIGNAL void stackIdChanged();

    // clang-format off
    Q_PROPERTY(bool stackLeader
               READ isStackLeader
               WRITE setStackLeader
               NOTIFY stackLeaderChanged)
    // clang-format on
    void setStackLeader(bool val);
    bool isStackLeader() const { return m_stackLeader; }
    Q_SIGNAL void stackLeaderChanged();

    Q_INVOKABLE void unstack();

    Q_SIGNAL void elementChanged();
    Q_SIGNAL void sceneHeadingChanged();
    Q_SIGNAL void sceneLocationChanged();

    // QObjectSerializer::Interface implementation
    void serializeToJson(QJsonObject &) const;

protected:
    bool event(QEvent *event);
    void resetFollow();
    void syncWithFollowItem();
    void groupVerificationRequired();

private:
    friend class Structure;
    void renameCharacter(const QString &from, const QString &to);

private:
    friend class StructurePositionCommand;
    qreal m_x = 0;
    qreal m_y = 0;
    bool m_placed = false;
    qreal m_width = 0;
    qreal m_height = 0;
    QString m_stackId;
    bool m_stackLeader = false;
    QString m_title;
    Scene *m_scene = nullptr;
    bool m_selected = false;
    bool m_syncWithFollow = false;
    bool m_undoRedoEnabled = false;
    Structure *m_structure = nullptr;
    QObjectProperty<QQuickItem> m_follow;
};

class StructureElementStack : public QObjectListModel<StructureElement *>
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

    StructureElementStack(QObject *parent = nullptr);

public:
    ~StructureElementStack();
    Q_SIGNAL void aboutToDelete(StructureElementStack *ptr);

    void setEnabled(bool val) { m_enabled = val; }
    bool isEnabled() const { return m_enabled; }

    // clang-format off
    Q_PROPERTY(QString stackId
               READ stackId
               NOTIFY stackIdChanged)
    // clang-format on
    QString stackId() const { return m_stackId; }
    Q_SIGNAL void stackIdChanged();

    // clang-format off
    Q_PROPERTY(int actIndex
               READ actIndex
               NOTIFY actIndexChanged)
    // clang-format on
    int actIndex() const { return m_actIndex; }
    Q_SIGNAL void actIndexChanged();

    // clang-format off
    Q_PROPERTY(StructureElement *stackLeader
               READ stackLeader
               NOTIFY stackLeaderChanged)
    // clang-format on
    StructureElement *stackLeader() const;
    Q_SIGNAL void stackLeaderChanged();

    // clang-format off
    Q_PROPERTY(int topmostElementIndex
               READ topmostElementIndex
               NOTIFY topmostElementChanged)
    // clang-format on
    int topmostElementIndex() const;
    Q_SIGNAL void topmostElementIndexChanged();

    // clang-format off
    Q_PROPERTY(StructureElement *topmostElement
               READ topmostElement
               NOTIFY topmostElementChanged)
    // clang-format on
    StructureElement *topmostElement() const;
    Q_SIGNAL void topmostElementChanged();

    // clang-format off
    Q_PROPERTY(bool hasCurrentElement
               READ isHasCurrentElement
               NOTIFY hasCurrentElementChanged)
    // clang-format on
    bool isHasCurrentElement() const { return m_hasCurrentElement; }
    Q_SIGNAL void hasCurrentElementChanged();

    // clang-format off
    Q_PROPERTY(QRectF geometry
               READ geometry
               NOTIFY geometryChanged)
    // clang-format on
    QRectF geometry() const { return m_geometry; }
    Q_SIGNAL void geometryChanged();

    Q_INVOKABLE void moveToStackId(const QString &stackID);
    Q_INVOKABLE void moveToStack(StructureElementStack *other);
    Q_INVOKABLE void bringElementToTop(int index);

    void sortByScreenplayOccurance(Screenplay *screenplay);

    static void stackEm(const QList<StructureElement *> &elements);

protected:
    void timerEvent(QTimerEvent *te);
    void itemInsertEvent(StructureElement *ptr);
    void itemRemoveEvent(StructureElement *ptr);

private:
    void setHasCurrentElement(bool val);
    void setTopmostElement(StructureElement *val);
    void setGeometry(const QRectF &val);

    void initialize();
    void onElementFollowSet();
    void onStackLeaderChanged();
    void onElementGroupChanged();
    void onElementGeometryChanged();
    void onStructureCurrentElementChanged();

private:
    friend class StructureElementStacks;
    QRectF m_geometry;
    QString m_stackId;
    int m_actIndex = -1;
    bool m_enabled = false;
    bool m_hasCurrentElement = false;
    ExecLaterTimer m_initializeTimer;
    StructureElement *m_topmostElement = nullptr;
};

class StructureElementStacks : public QObjectListModel<StructureElementStack *>
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    explicit StructureElementStacks(QObject *parent = nullptr);
    ~StructureElementStacks();

    // clang-format off
    Q_PROPERTY(Structure *structure
               READ structure
               CONSTANT )
    // clang-format on
    Structure *structure() const { return m_structure; }

    Q_INVOKABLE StructureElementStack *findStackById(const QString &stackID) const;
    Q_INVOKABLE StructureElementStack *findStackByElement(StructureElement *element) const;

protected:
    void timerEvent(QTimerEvent *te);
    void itemInsertEvent(StructureElementStack *ptr);
    void itemRemoveEvent(StructureElementStack *ptr);

private:
    void resetAllStacks();
    void evaluateStacks();
    void evaluateStacksLater();
    void evaluateStacksMuchLater(int howMuchLater);

private:
    friend class Structure;
    Structure *m_structure = nullptr;
    ExecLaterTimer m_evaluateTimer;
};

class Relationship : public QObject, public QObjectSerializer::Interface
{
    Q_OBJECT
    Q_INTERFACES(QObjectSerializer::Interface)
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    Q_INVOKABLE explicit Relationship(QObject *parent = nullptr);
    ~Relationship();
    Q_SIGNAL void aboutToDelete(Relationship *ptr);

    enum Direction { OfWith, WithOf };
    Q_ENUM(Direction)
    // clang-format off
    Q_PROPERTY(Direction direction
               READ direction
               WRITE setDirection
               NOTIFY directionChanged)
    // clang-format on
    void setDirection(Direction val);
    Direction direction() const { return m_direction; }
    Q_SIGNAL void directionChanged();

    static QString polishName(const QString &name);

    // clang-format off
    Q_PROPERTY(QString name
               READ name
               WRITE setName
               NOTIFY nameChanged)
    // clang-format on
    void setName(const QString &val);
    QString name() const { return m_name; }
    Q_SIGNAL void nameChanged();

    // clang-format off
    Q_PROPERTY(Character *withCharacter
               READ with
               WRITE setWith
               NOTIFY withChanged
               RESET resetWith
               STORED false)
    Q_PROPERTY(Character *with
               READ with
               WRITE setWith
               NOTIFY withChanged
               RESET resetWith
               STORED false)
    // clang-format on
    void setWith(Character *val);
    Character *with() const { return m_with; }
    Q_SIGNAL void withChanged();

    // clang-format off
    Q_PROPERTY(Character *ofCharacter
               READ of
               NOTIFY ofChanged
               STORED false)
    Q_PROPERTY(Character *of
               READ of
               NOTIFY ofChanged
               STORED false)
    // clang-format on
    Character *of() const { return m_of; }
    Q_SIGNAL void ofChanged();

    // clang-format off
    Q_PROPERTY(Notes *notes
               READ notes
               CONSTANT )
    // clang-format on
    Notes *notes() const { return m_notes; }

    Q_SIGNAL void relationshipChanged();

    // QObjectSerializer::Interface interface
    void serializeToJson(QJsonObject &) const;
    void deserializeFromJson(const QJsonObject &);

    void resolveRelationship();

protected:
    bool event(QEvent *event);

private:
    void setOf(Character *val);
    void resetWith();

private:
    QString m_name = QStringLiteral("Friend");
    Character *m_of;
    QString m_withName; // for delayed resolution during load
    Direction m_direction = OfWith;
    QObjectProperty<Character> m_with;
    Notes *m_notes = new Notes(this);
};

class Character : public QObject, public QObjectSerializer::Interface
{
    Q_OBJECT
    Q_INTERFACES(QObjectSerializer::Interface)
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    Q_INVOKABLE explicit Character(QObject *parent = nullptr);
    ~Character();
    Q_SIGNAL void aboutToDelete(Character *ptr);

    // clang-format off
    Q_PROPERTY(Structure *structure
               READ structure
               CONSTANT STORED
               false )
    // clang-format on
    Structure *structure() const { return m_structure; }

    // clang-format off
    Q_PROPERTY(bool valid
               READ isValid
               NOTIFY nameChanged)
    // clang-format on
    bool isValid() const { return !m_name.isEmpty(); }

    // clang-format off
    Q_PROPERTY(QString id
               READ name
               NOTIFY nameChanged)
    Q_PROPERTY(QString name
               READ name
               WRITE setName
               NOTIFY nameChanged)
    // clang-format on
    void setName(const QString &val);
    QString name() const { return m_name; }
    Q_SIGNAL void nameChanged();

    Q_INVOKABLE bool rename(const QString &name);

    Q_INVOKABLE void clearRenameError();

    // clang-format off
    Q_PROPERTY(QString renameError
               READ renameError
               NOTIFY renameErrorChanged)
    // clang-format on
    QString renameError() const { return m_renameError; }
    Q_SIGNAL void renameErrorChanged();

    // clang-format off
    Q_PROPERTY(bool visibleOnNotebook
               READ isVisibleOnNotebook
               WRITE setVisibleOnNotebook
               NOTIFY visibleOnNotebookChanged)
    // clang-format on
    void setVisibleOnNotebook(bool val);
    bool isVisibleOnNotebook() const { return m_visibleOnNotebook; }
    Q_SIGNAL void visibleOnNotebookChanged();

    // clang-format off
    Q_PROPERTY(Notes *notes
               READ notes
               CONSTANT )
    // clang-format on
    Notes *notes() const { return m_notes; }

    // clang-format off
    Q_PROPERTY(QStringList photos
               READ photos
               WRITE setPhotos
               NOTIFY photosChanged
               STORED false)
    // clang-format on
    void setPhotos(const QStringList &val);
    QStringList photos() const { return m_photos; }
    Q_SIGNAL void photosChanged();

    Q_INVOKABLE void addPhoto(const QString &photoPath);
    Q_INVOKABLE void removePhoto(int index);
    Q_INVOKABLE void removePhoto(const QString &photoPath);

    // clang-format off
    Q_PROPERTY(QString keyPhoto
               READ keyPhoto
               NOTIFY keyPhotoChanged)
    // clang-format on
    QString keyPhoto() const { return m_keyPhoto; }
    Q_SIGNAL void keyPhotoChanged();

    // clang-format off
    Q_PROPERTY(int keyPhotoIndex
               READ keyPhotoIndex
               WRITE setKeyPhotoIndex
               NOTIFY keyPhotoIndexChanged)
    // clang-format on
    void setKeyPhotoIndex(int val);
    int keyPhotoIndex() const { return m_keyPhotoIndex; }
    Q_SIGNAL void keyPhotoIndexChanged();

    // clang-format off
    Q_PROPERTY(bool hasKeyPhoto
               READ hasKeyPhoto
               NOTIFY keyPhotoChanged)
    // clang-format on
    bool hasKeyPhoto() const { return !m_keyPhoto.isEmpty(); }

    // clang-format off
    Q_PROPERTY(QString type
               READ type
               WRITE setType
               NOTIFY typeChanged)
    // clang-format on
    void setType(const QString &val);
    QString type() const { return m_type; }
    Q_SIGNAL void typeChanged();

    // clang-format off
    Q_PROPERTY(QString designation
               READ designation
               WRITE setDesignation
               NOTIFY designationChanged)
    // clang-format on
    void setDesignation(const QString &val);
    QString designation() const { return m_designation; }
    Q_SIGNAL void designationChanged();

    // clang-format off
    Q_PROPERTY(QString gender
               READ gender
               WRITE setGender
               NOTIFY genderChanged)
    // clang-format on
    void setGender(const QString &val);
    QString gender() const { return m_gender; }
    Q_SIGNAL void genderChanged();

    // clang-format off
    Q_PROPERTY(QString age
               READ age
               WRITE setAge
               NOTIFY ageChanged)
    // clang-format on
    void setAge(const QString &val);
    QString age() const { return m_age; }
    Q_SIGNAL void ageChanged();

    // clang-format off
    Q_PROPERTY(QString height
               READ height
               WRITE setHeight
               NOTIFY heightChanged)
    // clang-format on
    void setHeight(const QString &val);
    QString height() const { return m_height; }
    Q_SIGNAL void heightChanged();

    // clang-format off
    Q_PROPERTY(QString weight
               READ weight
               WRITE setWeight
               NOTIFY weightChanged)
    // clang-format on
    void setWeight(const QString &val);
    QString weight() const { return m_weight; }
    Q_SIGNAL void weightChanged();

    // clang-format off
    Q_PROPERTY(QString bodyType
               READ bodyType
               WRITE setBodyType
               NOTIFY bodyTypeChanged)
    // clang-format on
    void setBodyType(const QString &val);
    QString bodyType() const { return m_bodyType; }
    Q_SIGNAL void bodyTypeChanged();

    // clang-format off
    Q_PROPERTY(QStringList aliases
               READ aliases
               WRITE setAliases
               NOTIFY aliasesChanged)
    // clang-format on
    void setAliases(const QStringList &val);
    QStringList aliases() const { return m_aliases; }
    Q_SIGNAL void aliasesChanged();

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
    Q_PROPERTY(QJsonValue summary
               READ summary
               WRITE setSummary
               NOTIFY summaryChanged)
    // clang-format on
    void setSummary(const QJsonValue &val);
    QJsonValue summary() const { return m_summary; }
    Q_SIGNAL void summaryChanged();

    // clang-format off
    Q_PROPERTY(QStringList tags
               READ tags
               WRITE setTags
               NOTIFY tagsChanged)
    // clang-format on
    void setTags(const QStringList &val);
    QStringList tags() const { return m_tags; }
    Q_SIGNAL void tagsChanged();

    Q_INVOKABLE bool addTag(const QString &tag);
    Q_INVOKABLE bool removeTag(const QString &tag);
    Q_INVOKABLE bool hasTag(const QString &tag) const;

    // clang-format off
    Q_PROPERTY(bool hasTags
               READ hasTags
               NOTIFY tagsChanged)
    // clang-format on
    bool hasTags() const { return !m_tags.isEmpty(); }

    // clang-format off
    Q_PROPERTY(int priority
               READ priority
               WRITE setPriority
               NOTIFY priorityChanged)
    // clang-format on
    void setPriority(int val);
    int priority() const { return m_priority; }
    Q_SIGNAL void priorityChanged();

    // clang-format off
    Q_PROPERTY(QAbstractListModel *relationshipsModel
               READ relationshipsModel
               CONSTANT STORED
               false )
    // clang-format on
    QObjectListModel<Relationship *> *relationshipsModel() const
    {
        return &((const_cast<Character *>(this))->m_relationships);
    }

    // clang-format off
    Q_PROPERTY(QQmlListProperty<Relationship> relationships
               READ relationships
               NOTIFY relationshipCountChanged)
    // clang-format on
    QQmlListProperty<Relationship> relationships();
    Q_INVOKABLE void addRelationship(Relationship *ptr);
    Q_INVOKABLE void removeRelationship(Relationship *ptr);
    Q_INVOKABLE Relationship *relationshipAt(int index) const;
    void setRelationships(const QList<Relationship *> &list);
    // clang-format off
    Q_PROPERTY(int relationshipCount
               READ relationshipCount
               NOTIFY relationshipCountChanged)
    // clang-format on
    int relationshipCount() const { return m_relationships.size(); }
    Q_INVOKABLE void clearRelationships();
    Q_SIGNAL void relationshipCountChanged();

    Q_INVOKABLE Relationship *addRelationship(const QString &name, Character *with);

    Q_INVOKABLE Relationship *findRelationshipWith(const QString &with) const;
    Q_INVOKABLE Relationship *findRelationship(Character *with) const;
    Q_INVOKABLE bool hasRelationshipWith(const QString &with) const
    {
        return this->findRelationshipWith(with) != nullptr;
    }
    Q_INVOKABLE bool isDirectlyRelatedTo(Character *with) const
    {
        return this->findRelationship(with) != nullptr;
    }
    Q_INVOKABLE bool isRelatedTo(Character *with) const;
    Q_INVOKABLE QList<Relationship *> findRelationshipsWith(const QString &name = QString()) const;
    Q_INVOKABLE QStringList unrelatedCharacterNames() const;

    // clang-format off
    Q_PROPERTY(QJsonObject characterRelationshipGraph
               READ characterRelationshipGraph
               WRITE setCharacterRelationshipGraph
               NOTIFY characterRelationshipGraphChanged)
    // clang-format on
    void setCharacterRelationshipGraph(const QJsonObject &val);
    QJsonObject characterRelationshipGraph() const { return m_characterRelationshipGraph; }
    Q_SIGNAL void characterRelationshipGraphChanged();

    // clang-format off
    Q_PROPERTY(Attachments *attachments
               READ attachments
               CONSTANT )
    // clang-format on
    Attachments *attachments() const { return m_attachments; }

    Q_SIGNAL void characterChanged();

    // QObjectSerializer::Interface interface
    void serializeToJson(QJsonObject &) const;
    void deserializeFromJson(const QJsonObject &);
    bool canSetPropertyFromObjectList(const QString &propName) const;
    void setPropertyFromObjectList(const QString &propName, const QList<QObject *> &objects);

    void resolveRelationships();

    // Text Document Export Support
    struct WriteOptions
    {
        WriteOptions() { }
        int headingLevel = 2;
        bool includeSummary = true;
        bool includeTextNotes = true;
        bool includeFormNotes = true;
    };
    void write(QTextCursor &cursor, const WriteOptions &options = WriteOptions()) const;

    static bool LessThan(Character *a, Character *b);

protected:
    bool event(QEvent *event);

private:
    bool isRelatedToImpl(Character *with, QStack<Character *> &stack) const;
    void onDfsAuction(const QString &filePath, int *claims);
    void setKeyPhoto(const QString &val);

    static void staticAppendRelationship(QQmlListProperty<Relationship> *list, Relationship *ptr);
    static void staticClearRelationships(QQmlListProperty<Relationship> *list);
    static Relationship *staticRelationshipAt(QQmlListProperty<Relationship> *list, int index);
    static int staticRelationshipCount(QQmlListProperty<Relationship> *list);

private:
    QString m_age;
    QString m_name;
    QColor m_color = Qt::transparent;
    QString m_type = QStringLiteral("Human");
    QString m_weight;
    QString m_height;
    QString m_gender;
    QString m_keyPhoto;
    int m_keyPhotoIndex = -1;
    QJsonValue m_summary;
    QString m_bodyType;
    int m_priority = 0;
    QStringList m_tags;
    QStringList m_photos;
    QString m_designation;
    QStringList m_aliases;
    QString m_renameError;
    bool m_visibleOnNotebook = true;
    Structure *m_structure = nullptr;
    Notes *m_notes = new Notes(this);
    QJsonObject m_characterRelationshipGraph;
    Attachments *m_attachments = new Attachments(this);
    QObjectListModel<Relationship *> m_relationships;
};

class CharacterNamesModel : public QStringListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit CharacterNamesModel(QObject *parent = nullptr);
    ~CharacterNamesModel();

    Q_INVOKABLE Character *findCharacter(const QString &name) const;

    // clang-format off
    Q_PROPERTY(int count
               READ count
               NOTIFY countChanged)
    // clang-format on
    int count() const;
    Q_SIGNAL void countChanged();

    // clang-format off
    Q_PROPERTY(Structure *structure
               READ structure
               WRITE setStructure
               NOTIFY structureChanged)
    // clang-format on
    void setStructure(Structure *val);
    Structure *structure() const { return m_structure; }
    Q_SIGNAL void structureChanged();

    // clang-format off
    Q_PROPERTY(QStringList tags
               READ tags
               WRITE setTags
               NOTIFY tagsChanged)
    // clang-format on
    void setTags(const QStringList &val);
    QStringList tags() const { return m_tags; }
    Q_SIGNAL void tagsChanged();

    Q_INVOKABLE void addTag(const QString &tag);
    Q_INVOKABLE void removeTag(const QString &tag);
    Q_INVOKABLE void toggleTag(const QString &tag);
    Q_INVOKABLE void clearTags();
    Q_INVOKABLE bool hasTag(const QString &tag) const;

    // clang-format off
    Q_PROPERTY(QStringList availableTags
               READ availableTags
               NOTIFY availableTagsChanged)
    // clang-format on
    QStringList availableTags() const;
    Q_SIGNAL void availableTagsChanged();

    // clang-format off
    Q_PROPERTY(QStringList allNames
               READ allNames
               NOTIFY allNamesChanged)
    // clang-format on
    QStringList allNames() const;
    Q_SIGNAL void allNamesChanged();

    // clang-format off
    Q_PROPERTY(QStringList selectedCharacters
               READ selectedCharacters
               WRITE setSelectedCharacters
               NOTIFY selectedCharactersChanged)
    // clang-format on
    void setSelectedCharacters(const QStringList &val);
    QStringList selectedCharacters() const { return m_selectedCharacters; }
    Q_SIGNAL void selectedCharactersChanged();

    Q_INVOKABLE void addToSelection(const QString &name);
    Q_INVOKABLE void removeFromSelection(const QString &name);
    Q_INVOKABLE bool isInSelection(const QString &name) const;
    Q_INVOKABLE void clearSelection();
    Q_INVOKABLE void toggleSelection(const QString &name);
    Q_INVOKABLE void selectAll();
    Q_INVOKABLE void unselectAll();

    // QAbstractItemModel interface
    QHash<int, QByteArray> roleNames() const;

private:
    void reload();

private:
    QStringList m_tags;
    Structure *m_structure = nullptr;
    QStringList m_selectedCharacters;
};

class Annotation : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    Q_INVOKABLE explicit Annotation(QObject *parent = nullptr);
    ~Annotation();
    Q_SIGNAL void aboutToDelete(Annotation *ptr);

    // clang-format off
    Q_PROPERTY(Structure *structure
               READ structure
               CONSTANT STORED
               false )
    // clang-format on
    Structure *structure() const { return m_structure; }

    // clang-format off
    Q_PROPERTY(QString type
               READ type
               WRITE setType
               NOTIFY typeChanged)
    // clang-format on
    void setType(const QString &val);
    QString type() const { return m_type; }
    Q_SIGNAL void typeChanged();

    // clang-format off
    Q_PROPERTY(bool resizable
               READ isResizable
               WRITE setResizable
               NOTIFY resizableChanged
               STORED false)
    // clang-format on
    void setResizable(bool val);
    bool isResizable() const { return m_resizable; }
    Q_SIGNAL void resizableChanged();

    // clang-format off
    Q_PROPERTY(bool movable
               READ isMovable
               WRITE setMovable
               NOTIFY movableChanged
               STORED false)
    // clang-format on
    void setMovable(bool val);
    bool isMovable() const { return m_movable; }
    Q_SIGNAL void movableChanged();

    // clang-format off
    Q_PROPERTY(QRectF geometry
               READ geometry
               WRITE setGeometry
               NOTIFY geometryChanged)
    // clang-format on
    void setGeometry(const QRectF &val);
    QRectF geometry() const { return m_geometry; }
    Q_SIGNAL void geometryChanged();

    Q_INVOKABLE void move(qreal x, qreal y)
    {
        this->setGeometry(QRectF(x, y, m_geometry.width(), m_geometry.height()));
    }
    Q_INVOKABLE void resize(qreal w, qreal h)
    {
        this->setGeometry(QRectF(m_geometry.x(), m_geometry.y(), w, h));
    }
    Q_INVOKABLE void place(qreal x, qreal y, qreal w, qreal h)
    {
        this->setGeometry(QRectF(x, y, w, h));
    }

    // clang-format off
    Q_PROPERTY(QJsonObject attributes
               READ attributes
               WRITE setAttributes
               NOTIFY attributesChanged)
    // clang-format on
    void setAttributes(const QJsonObject &val);
    QJsonObject attributes() const { return m_attributes; }
    Q_SIGNAL void attributesChanged();

    Q_INVOKABLE void setAttribute(const QString &key, const QJsonValue &value);
    Q_INVOKABLE void removeAttribute(const QString &key);
    Q_INVOKABLE void saveAttributesAsDefault();

    // clang-format off
    Q_PROPERTY(QJsonArray metaData
               READ metaData
               WRITE setMetaData
               NOTIFY metaDataChanged
               STORED false)
    // clang-format on
    void setMetaData(const QJsonArray &val);
    QJsonArray metaData() const { return m_metaData; }
    Q_SIGNAL void metaDataChanged();

    Q_INVOKABLE bool removeImage(const QString &name) const;
    Q_INVOKABLE QString addImage(const QString &path) const;
    Q_INVOKABLE QString addImage(const QVariant &image) const;
    Q_INVOKABLE QUrl imageUrl(const QString &name) const;

    void createCopyOfFileAttributes();

    Q_SIGNAL void annotationChanged();

protected:
    bool event(QEvent *event);
    void polishAttributes();
    void onDfsAuction(const QString &filePath, int *claims);

private:
    QRectF m_geometry;
    QString m_type;
    bool m_movable = true;
    bool m_resizable = true;
    Structure *m_structure = nullptr;
    QJsonArray m_metaData;
    QStringList m_fileAttributes;
    QJsonObject m_attributes;
};

class Structure : public QObject, public QObjectSerializer::Interface
{
    Q_OBJECT
    Q_INTERFACES(QObjectSerializer::Interface)
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    explicit Structure(QObject *parent = nullptr);
    ~Structure();
    Q_SIGNAL void aboutToDelete(Structure *ptr);

    // clang-format off
    Q_PROPERTY(qreal canvasWidth
               READ canvasWidth
               WRITE setCanvasWidth
               NOTIFY canvasWidthChanged)
    // clang-format on
    void setCanvasWidth(qreal val);
    qreal canvasWidth() const { return m_canvasWidth; }
    Q_SIGNAL void canvasWidthChanged();

    // clang-format off
    Q_PROPERTY(qreal canvasHeight
               READ canvasHeight
               WRITE setCanvasHeight
               NOTIFY canvasHeightChanged)
    // clang-format on
    void setCanvasHeight(qreal val);
    qreal canvasHeight() const { return m_canvasHeight; }
    Q_SIGNAL void canvasHeightChanged();

    // clang-format off
    Q_PROPERTY(qreal canvasGridSize
               READ canvasGridSize
               WRITE setCanvasGridSize
               NOTIFY canvasGridSizeChanged)
    // clang-format on
    void setCanvasGridSize(qreal val);
    qreal canvasGridSize() const { return m_canvasGridSize; }
    Q_SIGNAL void canvasGridSizeChanged();

    enum CanvasUIMode { SynopsisEditorUI, IndexCardUI };
    Q_ENUM(CanvasUIMode)
    // clang-format off
    Q_PROPERTY(CanvasUIMode canvasUIMode
               READ canvasUIMode
               WRITE setCanvasUIMode
               NOTIFY canvasUIModeChanged)
    // clang-format on
    void setCanvasUIMode(CanvasUIMode val);
    CanvasUIMode canvasUIMode() const { return m_canvasUIMode; }
    Q_SIGNAL void canvasUIModeChanged();

    enum IndexCardContent { Synopsis, FeaturedPhoto };
    Q_ENUM(IndexCardContent)

    // clang-format off
    Q_PROPERTY(IndexCardContent indexCardContent
               READ indexCardContent
               WRITE setIndexCardContent
               NOTIFY indexCardContentChanged)
    // clang-format on
    void setIndexCardContent(IndexCardContent val);
    IndexCardContent indexCardContent() const { return m_indexCardContent; }
    Q_SIGNAL void indexCardContentChanged();

    Q_INVOKABLE qreal snapToGrid(qreal val) const;
    static qreal snapToGrid(qreal val, const Structure *structure, qreal defaultGridSize = 10.0);

    void captureStructureAsImage(const QString &fileName);
    Q_SIGNAL void captureStructureAsImageRequest(const QString &fileName);

    // clang-format off
    Q_PROPERTY(ScriteDocument *scriteDocument
               READ scriteDocument
               CONSTANT STORED
               false )
    // clang-format on
    ScriteDocument *scriteDocument() const { return m_scriteDocument; }

    // clang-format off
    Q_PROPERTY(QAbstractListModel *charactersModel
               READ charactersModel
               CONSTANT STORED
               false )
    // clang-format on
    QObjectListModel<Character *> *charactersModel() const
    {
        return &((const_cast<Structure *>(this))->m_characters);
    }

    // clang-format off
    Q_PROPERTY(QQmlListProperty<Character> characters
               READ characters
               NOTIFY characterCountChanged)
    // clang-format on
    QQmlListProperty<Character> characters();
    Q_INVOKABLE void addCharacter(Character *ptr);
    Q_INVOKABLE void removeCharacter(Character *ptr);
    Q_INVOKABLE Character *characterAt(int index) const;
    Q_INVOKABLE int indexOfCharacter(Character *ptr) const;
    void setCharacters(const QList<Character *> &list);
    // clang-format off
    Q_PROPERTY(int characterCount
               READ characterCount
               NOTIFY characterCountChanged)
    // clang-format on
    int characterCount() const { return m_characters.size(); }
    Q_INVOKABLE void clearCharacters();
    Q_SIGNAL void characterCountChanged();

    Q_INVOKABLE QStringList allCharacterNames() const { return m_characterNames; }
    Q_INVOKABLE QJsonArray detectCharacters() const;
    Q_INVOKABLE Character *addCharacter(const QString &name);
    Q_INVOKABLE void addCharacters(const QStringList &names);

    Q_INVOKABLE Character *findCharacter(const QString &name) const;
    QList<Character *> findCharacters(const QStringList &names,
                                      bool returnAssociativeList = false) const;

    // clang-format off
    Q_PROPERTY(Notes *notes
               READ notes
               CONSTANT )
    // clang-format on
    Notes *notes() const { return m_notes; }

    // clang-format off
    Q_PROPERTY(QAbstractListModel *elementsModel
               READ elementsModel
               CONSTANT STORED
               false )
    // clang-format on
    QObjectListModel<StructureElement *> *elementsModel() const
    {
        return &((const_cast<Structure *>(this))->m_elements);
    }

    // clang-format off
    Q_PROPERTY(QRectF elementsBoundingBox
               READ elementsBoundingBox
               NOTIFY elementsBoundingBoxChanged)
    // clang-format on
    QRectF elementsBoundingBox() const
    {
        return m_elementsBoundingBoxAggregator.aggregateValue().toRectF();
    }
    Q_SIGNAL void elementsBoundingBoxChanged();

    // clang-format off
    Q_PROPERTY(StructureElementStacks *elementStacks
               READ elementStacks
               CONSTANT STORED
               false )
    // clang-format on
    StructureElementStacks *elementStacks() const
    {
        return &((const_cast<Structure *>(this))->m_elementStacks);
    }

    // clang-format off
    Q_PROPERTY(QQmlListProperty<StructureElement> elements
               READ elements
               NOTIFY elementsChanged)
    // clang-format on
    QQmlListProperty<StructureElement> elements();
    Q_INVOKABLE void addElement(StructureElement *ptr);
    Q_INVOKABLE void removeElement(StructureElement *ptr);
    void removeElements(const QList<StructureElement *> &elements);
    Q_INVOKABLE void insertElement(StructureElement *ptr, int index);
    Q_INVOKABLE void moveElement(StructureElement *ptr, int toRow);
    void setElements(const QList<StructureElement *> &list);
    Q_INVOKABLE StructureElement *elementAt(int index) const;
    // clang-format off
    Q_PROPERTY(int elementCount
               READ elementCount
               NOTIFY elementCountChanged)
    // clang-format on
    int elementCount() const { return m_elements.size(); }
    Q_INVOKABLE void clearElements();
    Q_SIGNAL void elementCountChanged();
    Q_SIGNAL void elementsChanged();
    Q_SIGNAL void elementStackingChanged();

    Q_INVOKABLE int indexOfScene(Scene *scene) const;
    Q_INVOKABLE int indexOfElement(StructureElement *element) const;
    Q_INVOKABLE StructureElement *findElementBySceneID(const QString &id) const;

    enum LayoutType { HorizontalLayout, VerticalLayout, FlowHorizontalLayout, FlowVerticalLayout };
    Q_ENUM(LayoutType)
    Q_INVOKABLE QRectF layoutElements(Structure::LayoutType layoutType);

    // clang-format off
    Q_PROPERTY(bool forceBeatBoardLayout
               READ isForceBeatBoardLayout
               WRITE setForceBeatBoardLayout
               NOTIFY forceBeatBoardLayoutChanged)
    // clang-format on
    void setForceBeatBoardLayout(bool val);
    bool isForceBeatBoardLayout() const { return m_forceBeatBoardLayout; }
    Q_SIGNAL void forceBeatBoardLayoutChanged();

    Q_INVOKABLE void placeElement(StructureElement *element, Screenplay *screenplay) const;
    Q_INVOKABLE QRectF placeElementsInBeatBoardLayout(Screenplay *screenplay) const;
    Q_INVOKABLE QJsonObject evaluateEpisodeAndGroupBoxes(Screenplay *screenplay,
                                                         const QString &category) const;

    Q_INVOKABLE QJsonObject queryBreakElements(ScreenplayElement *breakElement) const;

    Q_INVOKABLE void scanForMuteCharacters();
    Q_INVOKABLE QStringList lookupMuteCharacters();
    Q_INVOKABLE void pruneCharacters(const QStringList &characters);

    Q_INVOKABLE static QStringList standardLocationTypes();
    Q_INVOKABLE static QStringList standardMoments();

    Q_INVOKABLE QStringList allLocations() const { return m_locationHeadingsMap.keys(); }
    QMap<QString, QList<SceneHeading *>> locationHeadingsMap() const
    {
        return m_locationHeadingsMap;
    }

    // clang-format off
    Q_PROPERTY(int currentElementIndex
               READ currentElementIndex
               WRITE setCurrentElementIndex
               NOTIFY currentElementIndexChanged
               STORED false)
    // clang-format on
    void setCurrentElementIndex(int val);
    int currentElementIndex() const { return m_currentElementIndex; }
    Q_SIGNAL void currentElementIndexChanged();

    // clang-format off
    Q_PROPERTY(qreal zoomLevel
               READ zoomLevel
               WRITE setZoomLevel
               NOTIFY zoomLevelChanged)
    // clang-format on
    void setZoomLevel(qreal val);
    qreal zoomLevel() const { return m_zoomLevel; }
    Q_SIGNAL void zoomLevelChanged();

    // clang-format off
    Q_PROPERTY(QStringList characterNames
               READ characterNames
               NOTIFY characterNamesChanged)
    // clang-format on
    QStringList characterNames() const { return m_characterNames; }
    Q_SIGNAL void characterNamesChanged();

    // clang-format off
    Q_PROPERTY(QStringList transitions
               READ transitions
               NOTIFY transitionsChanged)
    // clang-format on
    QStringList transitions() const { return m_transitions; }
    Q_SIGNAL void transitionsChanged();

    // clang-format off
    Q_PROPERTY(QStringList shots
               READ shots
               NOTIFY shotsChanged)
    // clang-format on
    QStringList shots() const { return m_shots; }
    Q_SIGNAL void shotsChanged();

    // clang-format off
    Q_PROPERTY(QStringList characterTags
               READ characterTags
               NOTIFY characterTagsChanged)
    // clang-format on
    QStringList characterTags() const { return m_characterTags; }
    Q_SIGNAL void characterTagsChanged();

    // clang-format off
    Q_PROPERTY(QStringList sceneTags
               READ sceneTags
               NOTIFY sceneTagsChanged)
    // clang-format on
    QStringList sceneTags() const { return m_sceneTags; }
    Q_SIGNAL void sceneTagsChanged();

    QStringList filteredCharacterNames(const QStringList &tags) const;

    // clang-format off
    Q_PROPERTY(QAbstractListModel *annotationsModel
               READ annotationsModel
               CONSTANT STORED
               false )
    // clang-format on
    QAbstractListModel *annotationsModel() const
    {
        return &((const_cast<Structure *>(this))->m_annotations);
    }

    // clang-format off
    Q_PROPERTY(QRectF annotationsBoundingBox
               READ annotationsBoundingBox
               NOTIFY annotationsBoundingBoxChanged)
    // clang-format on
    QRectF annotationsBoundingBox() const
    {
        return m_annotationsBoundingBoxAggregator.aggregateValue().toRectF();
    }
    Q_SIGNAL void annotationsBoundingBoxChanged();

    // clang-format off
    Q_PROPERTY(QQmlListProperty<Annotation> annotations
               READ annotations
               NOTIFY annotationCountChanged)
    // clang-format on
    QQmlListProperty<Annotation> annotations();
    Q_INVOKABLE void addAnnotation(Annotation *ptr);
    Q_INVOKABLE void removeAnnotation(Annotation *ptr);
    Q_INVOKABLE Annotation *annotationAt(int index) const;
    Q_INVOKABLE bool canBringToFront(Annotation *ptr) const;
    Q_INVOKABLE bool canSendToBack(Annotation *ptr) const;
    Q_INVOKABLE void bringToFront(Annotation *ptr);
    Q_INVOKABLE void sendToBack(Annotation *ptr);
    void setAnnotations(const QList<Annotation *> &list);
    // clang-format off
    Q_PROPERTY(int annotationCount
               READ annotationCount
               NOTIFY annotationCountChanged)
    // clang-format on
    int annotationCount() const { return m_annotations.size(); }
    Q_INVOKABLE void clearAnnotations();
    Q_SIGNAL void annotationCountChanged();

    // clang-format off
    Q_PROPERTY(QString defaultGroupsDataFile
               READ defaultGroupsDataFile
               CONSTANT )
    // clang-format on
    QString defaultGroupsDataFile() const;

    Q_INVOKABLE void loadDefaultGroupsData();

    // clang-format off
    Q_PROPERTY(QString groupsData
               READ groupsData
               WRITE setGroupsData
               NOTIFY groupsDataChanged)
    // clang-format on
    void setGroupsData(const QString &val);
    QString groupsData() const { return m_groupsData; }
    Q_SIGNAL void groupsDataChanged();

    // clang-format off
    Q_PROPERTY(QJsonArray groupsModel
               READ groupsModel
               NOTIFY groupsModelChanged)
    // clang-format on
    QJsonArray groupsModel() const { return m_groupsModel; }
    Q_SIGNAL void groupsModelChanged();

    // clang-format off
    Q_PROPERTY(QStringList groupCategories
               READ groupCategories
               NOTIFY groupsModelChanged)
    // clang-format on
    QStringList groupCategories() const { return m_groupCategories; }

    // clang-format off
    Q_PROPERTY(QVariantMap categoryActNames
               READ categoryActNames
               NOTIFY groupsModelChanged)
    // clang-format on
    QVariantMap categoryActNames() const { return m_categoryActNames; }

    // clang-format off
    Q_PROPERTY(QString preferredGroupCategory
               READ preferredGroupCategory
               WRITE setPreferredGroupCategory
               NOTIFY preferredGroupCategoryChanged)
    // clang-format on
    void setPreferredGroupCategory(const QString &val);
    QString preferredGroupCategory() const { return m_preferredGroupCategory; }
    Q_SIGNAL void preferredGroupCategoryChanged();

    Q_INVOKABLE QString presentableGroupNames(const QStringList &groups) const;

    // Local to the current Scrite document
    // clang-format off
    Q_PROPERTY(QJsonArray indexCardFields
               READ indexCardFields
               WRITE setIndexCardFields
               NOTIFY indexCardFieldsChanged)
    // clang-format on
    void setIndexCardFields(const QJsonArray &val);
    QJsonArray indexCardFields() const { return m_indexCardFields; }
    Q_SIGNAL void indexCardFieldsChanged();

    // clang-format off
    Q_PROPERTY(QJsonArray defaultIndexCardFields
               READ defaultIndexCardFields
               WRITE setDefaultIndexCardFields
               NOTIFY defaultIndexCardFieldsChanged
               STORED false)
    // clang-format on
    void setDefaultIndexCardFields(const QJsonArray &val);
    QJsonArray defaultIndexCardFields() const { return m_defaultIndexCardFields; }
    Q_SIGNAL void defaultIndexCardFieldsChanged();

    void loadDefaultIndexCardFields();

    Q_INVOKABLE Annotation *createAnnotation(const QString &type);

    Q_SIGNAL void structureChanged();

    // clang-format off
    Q_PROPERTY(bool canPaste
               READ canPaste
               NOTIFY canPasteChanged
               STORED false)
    // clang-format on
    bool canPaste() const { return m_canPaste; }
    Q_SIGNAL void canPasteChanged();

    Q_INVOKABLE void copy(QObject *elementOrAnnotation);
    Q_INVOKABLE void paste(const QPointF &pos = QPointF());

    // clang-format off
    Q_PROPERTY(QJsonObject characterRelationshipGraph
               READ characterRelationshipGraph
               WRITE setCharacterRelationshipGraph
               NOTIFY characterRelationshipGraphChanged)
    // clang-format on
    void setCharacterRelationshipGraph(const QJsonObject &val);
    QJsonObject characterRelationshipGraph() const { return m_characterRelationshipGraph; }
    Q_SIGNAL void characterRelationshipGraphChanged();

    // clang-format off
    Q_PROPERTY(Attachments *attachments
               READ attachments
               CONSTANT )
    // clang-format on
    Attachments *attachments() const { return m_attachments; }

    StructureExporter *createExporter();
    Q_INVOKABLE QObject *createExporterObject();

    // QObjectSerializer::Interface interface
    void prepareForDeserialization();
    void serializeToJson(QJsonObject &) const;
    void deserializeFromJson(const QJsonObject &);
    bool canSetPropertyFromObjectList(const QString &propName) const;
    void setPropertyFromObjectList(const QString &propName, const QList<QObject *> &objects);

    Q_INVOKABLE QStringList sortCharacterNames(const QStringList &names) const;

    // Text Document Export Support
    struct WriteOptions
    {
        WriteOptions() { }
        bool includeTextNotes = true;
        bool includeFormNotes = true;
        bool charactersOnly = false;
    };
    void write(QTextCursor &cursor, const WriteOptions &options = WriteOptions()) const;

protected:
    bool event(QEvent *event);
    void timerEvent(QTimerEvent *event);
    void resetCurentElementIndex();
    void setCanPaste(bool val);
    void onClipboardDataChanged();

private:
    friend class Character;
    friend class Screenplay;
    friend class ScriteDocument;
    StructureElement *splitElement(StructureElement *ptr, SceneElement *element, int textPosition);
    QList<QPair<QString, QList<StructureElement *>>>
    evaluateGroupsImpl(Screenplay *screenplay, const QString &category = QString()) const;

    bool renameCharacter(const QString &from, const QString &to, QString *errMsg);

private:
    friend class StructureLayout;
    friend class StructureElement;

    qreal m_canvasWidth = 120000;
    qreal m_canvasHeight = 120000;
    qreal m_canvasGridSize = 10;
    CanvasUIMode m_canvasUIMode = IndexCardUI;
    IndexCardContent m_indexCardContent = Synopsis;
    ScriteDocument *m_scriteDocument = nullptr;

    static void staticAppendCharacter(QQmlListProperty<Character> *list, Character *ptr);
    static void staticClearCharacters(QQmlListProperty<Character> *list);
    static Character *staticCharacterAt(QQmlListProperty<Character> *list, int index);
    static int staticCharacterCount(QQmlListProperty<Character> *list);
    QObjectListModel<Character *> m_characters;

    Notes *m_notes = new Notes(this);

    static void staticAppendElement(QQmlListProperty<StructureElement> *list,
                                    StructureElement *ptr);
    static void staticClearElements(QQmlListProperty<StructureElement> *list);
    static StructureElement *staticElementAt(QQmlListProperty<StructureElement> *list, int index);
    static int staticElementCount(QQmlListProperty<StructureElement> *list);
    QObjectListModel<StructureElement *> m_elements;
    ModelAggregator m_elementsBoundingBoxAggregator;
    StructureElementStacks m_elementStacks;
    int m_currentElementIndex = -1;
    qreal m_zoomLevel = 1.0;

    void updateLocationHeadingMap();
    void updateLocationHeadingMapLater();
    ExecLaterTimer m_locationHeadingsMapTimer;
    QMap<QString, QList<SceneHeading *>> m_locationHeadingsMap;

    void onStructureElementSceneChanged(StructureElement *element = nullptr);
    void onSceneElementChanged(SceneElement *element, Scene::SceneElementChangeType type);
    void onAboutToRemoveSceneElement(SceneElement *element);
    void updateCharacterNamesShotsTransitionsAndTags();
    void updateCharacterNamesShotsTransitionsAndTagsLater();
    void updateSceneTags();
    void updateSceneTagsLater();

    ExecLaterTimer m_updateCharacterNamesShotsTransitionsAndTagsTimer;
    ExecLaterTimer m_updateSceneTagsTimer;
    CharacterElementMap m_characterElementMap;
    TransitionElementMap m_transitionElementMap;
    ShotElementMap m_shotElementMap;
    QStringList m_shots;
    QStringList m_sceneTags;
    QStringList m_transitions;
    QStringList m_characterTags;
    QStringList m_characterNames;

    static void staticAppendAnnotation(QQmlListProperty<Annotation> *list, Annotation *ptr);
    static void staticClearAnnotations(QQmlListProperty<Annotation> *list);
    static Annotation *staticAnnotationAt(QQmlListProperty<Annotation> *list, int index);
    static int staticAnnotationCount(QQmlListProperty<Annotation> *list);
    QObjectListModel<Annotation *> m_annotations;
    ModelAggregator m_annotationsBoundingBoxAggregator;
    bool m_canPaste = false;

    bool m_forceBeatBoardLayout = false;
    QJsonObject m_characterRelationshipGraph;
    Attachments *m_attachments = new Attachments(this);

    QString m_groupsData;
    QJsonArray m_groupsModel;
    QVariantMap m_categoryActNames;
    QStringList m_groupCategories;
    QString m_preferredGroupCategory;

    QJsonArray m_indexCardFields;
    QJsonArray m_defaultIndexCardFields;

    enum DeserializationStage { FullyDeserialized = -1, JustDeserialized, BeingDeserialized };
    DeserializationStage m_deserializationStage = FullyDeserialized;
};

///////////////////////////////////////////////////////////////////////////////

class StructureElementConnector : public AbstractShapeItem
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit StructureElementConnector(QQuickItem *parent = nullptr);
    ~StructureElementConnector();

    enum LineType { StraightLine, CurvedLine };
    Q_ENUM(LineType)
    // clang-format off
    Q_PROPERTY(LineType lineType
               READ lineType
               WRITE setLineType
               NOTIFY lineTypeChanged)
    // clang-format on
    void setLineType(LineType val);
    LineType lineType() const { return m_lineType; }
    Q_SIGNAL void lineTypeChanged();

    // clang-format off
    Q_PROPERTY(StructureElement *fromElement
               READ fromElement
               WRITE setFromElement
               NOTIFY fromElementChanged
               RESET resetFromElement)
    // clang-format on
    void setFromElement(StructureElement *val);
    StructureElement *fromElement() const { return m_fromElement; }
    Q_SIGNAL void fromElementChanged();

    // clang-format off
    Q_PROPERTY(StructureElement *toElement
               READ toElement
               WRITE setToElement
               NOTIFY toElementChanged
               RESET resetToElement)
    // clang-format on
    void setToElement(StructureElement *val);
    StructureElement *toElement() const { return m_toElement; }
    Q_SIGNAL void toElementChanged();

    // clang-format off
    Q_PROPERTY(qreal arrowAndLabelSpacing
               READ arrowAndLabelSpacing
               WRITE setArrowAndLabelSpacing
               NOTIFY arrowAndLabelSpacingChanged)
    // clang-format on
    void setArrowAndLabelSpacing(qreal val);
    qreal arrowAndLabelSpacing() const { return m_arrowAndLabelSpacing; }
    Q_SIGNAL void arrowAndLabelSpacingChanged();

    // clang-format off
    Q_PROPERTY(QPointF arrowPosition
               READ arrowPosition
               NOTIFY arrowPositionChanged)
    // clang-format on
    QPointF arrowPosition() const { return m_arrowPosition; }
    Q_SIGNAL void arrowPositionChanged();

    // clang-format off
    Q_PROPERTY(QPointF suggestedLabelPosition
               READ suggestedLabelPosition
               NOTIFY suggestedLabelPositionChanged)
    // clang-format on
    QPointF suggestedLabelPosition() const { return m_suggestedLabelPosition; }
    Q_SIGNAL void suggestedLabelPositionChanged();

    // clang-format off
    Q_PROPERTY(bool canBeVisible
               READ canBeVisible
               NOTIFY canBeVisibleChanged)
    // clang-format on
    bool canBeVisible() const;
    Q_SIGNAL void canBeVisibleChanged();

    Q_INVOKABLE bool intersects(const QRectF &rect) const;

    QPainterPath shape() const;

    static QPainterPath curvedArrowPath(const QRectF &rect1, const QRectF &rect2,
                                        const qreal arrowSize = 6, bool fillArrow = false);

protected:
    // QObject interface
    void timerEvent(QTimerEvent *te);

protected:
    // QQuickItem interface
    void itemChange(ItemChange, const ItemChangeData &);

private:
    void resetFromElement();
    void resetToElement();
    void pickElementColor();
    void updateArrowAndLabelPositions();
    void setArrowPosition(const QPointF &val);
    void setSuggestedLabelPosition(const QPointF &val);
    void computeConnectorShape();
    void computeConnectorShapeLater();

private:
    QPainterPath m_connectorShape;
    LineType m_lineType = StraightLine;
    QPointF m_arrowPosition;
    qreal m_arrowAndLabelSpacing = 30;
    ExecLaterTimer m_computeConnectorShapeTimer;
    QObjectProperty<StructureElement> m_toElement;
    QObjectProperty<StructureElement> m_fromElement;
    QPointF m_suggestedLabelPosition;
};

class StructureCanvasViewportFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit StructureCanvasViewportFilterModel(QObject *parent = nullptr);
    ~StructureCanvasViewportFilterModel();

    // clang-format off
    Q_PROPERTY(Structure *structure
               READ structure
               WRITE setStructure
               RESET resetStructure
               NOTIFY structureChanged)
    // clang-format on
    void setStructure(Structure *val);
    Structure *structure() const { return m_structure; }
    Q_SIGNAL void structureChanged();

    // clang-format off
    Q_PROPERTY(bool enabled
               READ isEnabled
               WRITE setEnabled
               NOTIFY enabledChanged)
    // clang-format on
    void setEnabled(bool val);
    bool isEnabled() const { return m_enabled; }
    Q_SIGNAL void enabledChanged();

    enum Type { AnnotationType, StructureElementType };
    Q_ENUM(Type)
    // clang-format off
    Q_PROPERTY(Type type
               READ type
               WRITE setType
               NOTIFY typeChanged)
    // clang-format on
    void setType(Type val);
    Type type() const { return m_type; }
    Q_SIGNAL void typeChanged();

    // clang-format off
    Q_PROPERTY(QRectF viewportRect
               READ viewportRect
               WRITE setViewportRect
               NOTIFY viewportRectChanged)
    // clang-format on
    void setViewportRect(const QRectF &val);
    QRectF viewportRect() const { return m_viewportRect; }
    Q_SIGNAL void viewportRectChanged();

    enum ComputeStrategy { PreComputeStrategy, OnDemandComputeStrategy };
    Q_ENUM(ComputeStrategy)
    // clang-format off
    Q_PROPERTY(ComputeStrategy computeStrategy
               READ computeStrategy
               WRITE setComputeStrategy
               NOTIFY computeStrategyChanged)
    // clang-format on
    void setComputeStrategy(ComputeStrategy val);
    ComputeStrategy computeStrategy() const { return m_computeStrategy; }
    Q_SIGNAL void computeStrategyChanged();

    enum FilterStrategy { ContainsStrategy, IntersectsStrategy };
    Q_ENUM(FilterStrategy)
    // clang-format off
    Q_PROPERTY(FilterStrategy filterStrategy
               READ filterStrategy
               WRITE setFilterStrategy
               NOTIFY filterStrategyChanged)
    // clang-format on
    void setFilterStrategy(FilterStrategy val);
    FilterStrategy filterStrategy() const { return m_filterStrategy; }
    Q_SIGNAL void filterStrategyChanged();

    Q_INVOKABLE int mapFromSourceRow(int source_row) const;
    Q_INVOKABLE int mapToSourceRow(int filter_row) const;

    // QAbstractProxyModel interface
    void setSourceModel(QAbstractItemModel *model);

protected:
    // QSortFilterProxyModel interface
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

    // QObject interface
    void timerEvent(QTimerEvent *te);

private:
    void resetStructure();
    void updateSourceModel();
    void invalidateSelf();
    void invalidateSelfLater();

private:
    bool m_enabled = true;
    QRectF m_viewportRect;
    ExecLaterTimer m_invalidateTimer;
    Type m_type = StructureElementType;
    QObjectProperty<Structure> m_structure;
    FilterStrategy m_filterStrategy = IntersectsStrategy;
    ComputeStrategy m_computeStrategy = OnDemandComputeStrategy;
    QList<QPair<const QObject *, bool>> m_visibleSourceRows;
};

#endif // STRUCTURE_H
