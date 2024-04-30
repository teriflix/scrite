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

    Q_PROPERTY(Structure *structure READ structure CONSTANT STORED false)
    Structure *structure() const { return m_structure; }

    Q_PROPERTY(qreal x READ x WRITE setX NOTIFY xChanged STORED false)
    void setX(qreal val);
    qreal x() const { return m_x; }
    Q_SIGNAL void xChanged();

    Q_PROPERTY(qreal y READ y WRITE setY NOTIFY yChanged STORED false)
    void setY(qreal val);
    qreal y() const { return m_y; }
    Q_SIGNAL void yChanged();

    Q_PROPERTY(qreal width READ width WRITE setWidth NOTIFY widthChanged STORED false)
    void setWidth(qreal val);
    qreal width() const { return m_width; }
    Q_SIGNAL void widthChanged();

    Q_PROPERTY(qreal height READ height WRITE setHeight NOTIFY heightChanged STORED false)
    void setHeight(qreal val);
    qreal height() const { return m_height; }
    Q_SIGNAL void heightChanged();

    Q_PROPERTY(QRectF geometry READ geometry NOTIFY geometryChanged)
    QRectF geometry() const { return QRectF(m_x, m_y, m_width, m_height); }
    Q_SIGNAL void geometryChanged();

    Q_PROPERTY(QQuickItem *follow READ follow WRITE setFollow NOTIFY followChanged RESET resetFollow
                       STORED false)
    void setFollow(QQuickItem *val);
    QQuickItem *follow() const { return m_follow; }
    Q_SIGNAL void followChanged();

    Q_PROPERTY(bool undoRedoEnabled READ isUndoRedoEnabled WRITE setUndoRedoEnabled NOTIFY
                       undoRedoEnabledChanged)
    void setUndoRedoEnabled(bool val);
    bool isUndoRedoEnabled() const { return m_undoRedoEnabled; }
    Q_SIGNAL void undoRedoEnabledChanged();

    Q_PROPERTY(bool syncWithFollow READ isSyncWithFollow WRITE setSyncWithFollow NOTIFY
                       syncWithFollowChanged STORED false)
    void setSyncWithFollow(bool val);
    bool isSyncWithFollow() const { return m_syncWithFollow; }
    Q_SIGNAL void syncWithFollowChanged();

    Q_PROPERTY(qreal xf READ xf WRITE setXf NOTIFY xfChanged)
    void setXf(qreal val);
    qreal xf() const;
    Q_SIGNAL void xfChanged();

    Q_PROPERTY(qreal yf READ yf WRITE setYf NOTIFY yfChanged)
    void setYf(qreal val);
    qreal yf() const;
    Q_SIGNAL void yfChanged();

    Q_PROPERTY(QPointF position READ position WRITE setPosition NOTIFY positionChanged STORED false)
    void setPosition(const QPointF &pos);
    QPointF position() const { return QPointF(m_x, m_y); }
    Q_SIGNAL void positionChanged();

    Q_PROPERTY(Scene *scene READ scene WRITE setScene NOTIFY sceneChanged)
    void setScene(Scene *val);
    Scene *scene() const { return m_scene; }
    Q_SIGNAL void sceneChanged();

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    void setTitle(const QString &val);
    QString title() const;
    Q_SIGNAL void titleChanged();

    Q_PROPERTY(QString nativeTitle READ nativeTitle NOTIFY titleChanged)
    QString nativeTitle() const { return m_title; }

    Q_PROPERTY(bool hasTitle READ hasTitle NOTIFY titleChanged)
    bool hasTitle() const { return !this->title().isEmpty(); }

    Q_PROPERTY(bool hasNativeTitle READ hasNativeTitle NOTIFY titleChanged)
    bool hasNativeTitle() const { return !m_title.isEmpty(); }

    Q_PROPERTY(bool selected READ isSelected WRITE setSelected NOTIFY selectedChanged STORED false)
    void setSelected(bool val);
    bool isSelected() const { return m_selected; }
    Q_SIGNAL void selectedChanged();

    Q_PROPERTY(QString stackId READ stackId WRITE setStackId NOTIFY stackIdChanged)
    void setStackId(const QString &val);
    QString stackId() const { return m_stackId; }
    Q_SIGNAL void stackIdChanged();

    Q_PROPERTY(bool stackLeader READ isStackLeader WRITE setStackLeader NOTIFY stackLeaderChanged)
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

    Q_PROPERTY(QString stackId READ stackId NOTIFY stackIdChanged)
    QString stackId() const { return m_stackId; }
    Q_SIGNAL void stackIdChanged();

    Q_PROPERTY(int actIndex READ actIndex NOTIFY actIndexChanged)
    int actIndex() const { return m_actIndex; }
    Q_SIGNAL void actIndexChanged();

    Q_PROPERTY(StructureElement *stackLeader READ stackLeader NOTIFY stackLeaderChanged)
    StructureElement *stackLeader() const;
    Q_SIGNAL void stackLeaderChanged();

    Q_PROPERTY(int topmostElementIndex READ topmostElementIndex NOTIFY topmostElementChanged)
    int topmostElementIndex() const;
    Q_SIGNAL void topmostElementIndexChanged();

    Q_PROPERTY(StructureElement *topmostElement READ topmostElement NOTIFY topmostElementChanged)
    StructureElement *topmostElement() const;
    Q_SIGNAL void topmostElementChanged();

    Q_PROPERTY(bool hasCurrentElement READ isHasCurrentElement NOTIFY hasCurrentElementChanged)
    bool isHasCurrentElement() const { return m_hasCurrentElement; }
    Q_SIGNAL void hasCurrentElementChanged();

    Q_PROPERTY(QRectF geometry READ geometry NOTIFY geometryChanged)
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

    Q_PROPERTY(Structure *structure READ structure CONSTANT)
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
    Q_PROPERTY(Direction direction READ direction WRITE setDirection NOTIFY directionChanged)
    void setDirection(Direction val);
    Direction direction() const { return m_direction; }
    Q_SIGNAL void directionChanged();

    static QString polishName(const QString &name);

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    void setName(const QString &val);
    QString name() const { return m_name; }
    Q_SIGNAL void nameChanged();

    Q_PROPERTY(Character *withCharacter READ with WRITE setWith NOTIFY withChanged RESET resetWith
                       STORED false)
    Q_PROPERTY(
            Character *with READ with WRITE setWith NOTIFY withChanged RESET resetWith STORED false)
    void setWith(Character *val);
    Character *with() const { return m_with; }
    Q_SIGNAL void withChanged();

    Q_PROPERTY(Character *ofCharacter READ of NOTIFY ofChanged STORED false)
    Q_PROPERTY(Character *of READ of NOTIFY ofChanged STORED false)
    Character *of() const { return m_of; }
    Q_SIGNAL void ofChanged();

    Q_PROPERTY(Notes *notes READ notes CONSTANT)
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

    Q_PROPERTY(Structure *structure READ structure CONSTANT STORED false)
    Structure *structure() const { return m_structure; }

    Q_PROPERTY(bool valid READ isValid NOTIFY nameChanged)
    bool isValid() const { return !m_name.isEmpty(); }

    Q_PROPERTY(QString id READ name NOTIFY nameChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    void setName(const QString &val);
    QString name() const { return m_name; }
    Q_SIGNAL void nameChanged();

    Q_INVOKABLE bool rename(const QString &name);

    Q_INVOKABLE void clearRenameError();

    Q_PROPERTY(QString renameError READ renameError NOTIFY renameErrorChanged)
    QString renameError() const { return m_renameError; }
    Q_SIGNAL void renameErrorChanged();

    Q_PROPERTY(bool visibleOnNotebook READ isVisibleOnNotebook WRITE setVisibleOnNotebook NOTIFY
                       visibleOnNotebookChanged)
    void setVisibleOnNotebook(bool val);
    bool isVisibleOnNotebook() const { return m_visibleOnNotebook; }
    Q_SIGNAL void visibleOnNotebookChanged();

    Q_PROPERTY(Notes *notes READ notes CONSTANT)
    Notes *notes() const { return m_notes; }

    Q_PROPERTY(QStringList photos READ photos WRITE setPhotos NOTIFY photosChanged STORED false)
    void setPhotos(const QStringList &val);
    QStringList photos() const { return m_photos; }
    Q_SIGNAL void photosChanged();

    Q_INVOKABLE void addPhoto(const QString &photoPath);
    Q_INVOKABLE void removePhoto(int index);
    Q_INVOKABLE void removePhoto(const QString &photoPath);

    Q_PROPERTY(QString keyPhoto READ keyPhoto NOTIFY keyPhotoChanged)
    QString keyPhoto() const { return m_keyPhoto; }
    Q_SIGNAL void keyPhotoChanged();

    Q_PROPERTY(
            int keyPhotoIndex READ keyPhotoIndex WRITE setKeyPhotoIndex NOTIFY keyPhotoIndexChanged)
    void setKeyPhotoIndex(int val);
    int keyPhotoIndex() const { return m_keyPhotoIndex; }
    Q_SIGNAL void keyPhotoIndexChanged();

    Q_PROPERTY(bool hasKeyPhoto READ hasKeyPhoto NOTIFY keyPhotoChanged)
    bool hasKeyPhoto() const { return !m_keyPhoto.isEmpty(); }

    Q_PROPERTY(QString type READ type WRITE setType NOTIFY typeChanged)
    void setType(const QString &val);
    QString type() const { return m_type; }
    Q_SIGNAL void typeChanged();

    Q_PROPERTY(QString designation READ designation WRITE setDesignation NOTIFY designationChanged)
    void setDesignation(const QString &val);
    QString designation() const { return m_designation; }
    Q_SIGNAL void designationChanged();

    Q_PROPERTY(QString gender READ gender WRITE setGender NOTIFY genderChanged)
    void setGender(const QString &val);
    QString gender() const { return m_gender; }
    Q_SIGNAL void genderChanged();

    Q_PROPERTY(QString age READ age WRITE setAge NOTIFY ageChanged)
    void setAge(const QString &val);
    QString age() const { return m_age; }
    Q_SIGNAL void ageChanged();

    Q_PROPERTY(QString height READ height WRITE setHeight NOTIFY heightChanged)
    void setHeight(const QString &val);
    QString height() const { return m_height; }
    Q_SIGNAL void heightChanged();

    Q_PROPERTY(QString weight READ weight WRITE setWeight NOTIFY weightChanged)
    void setWeight(const QString &val);
    QString weight() const { return m_weight; }
    Q_SIGNAL void weightChanged();

    Q_PROPERTY(QString bodyType READ bodyType WRITE setBodyType NOTIFY bodyTypeChanged)
    void setBodyType(const QString &val);
    QString bodyType() const { return m_bodyType; }
    Q_SIGNAL void bodyTypeChanged();

    Q_PROPERTY(QStringList aliases READ aliases WRITE setAliases NOTIFY aliasesChanged)
    void setAliases(const QStringList &val);
    QStringList aliases() const { return m_aliases; }
    Q_SIGNAL void aliasesChanged();

    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    void setColor(const QColor &val);
    QColor color() const { return m_color; }
    Q_SIGNAL void colorChanged();

    Q_PROPERTY(QJsonValue summary READ summary WRITE setSummary NOTIFY summaryChanged)
    void setSummary(const QJsonValue &val);
    QJsonValue summary() const { return m_summary; }
    Q_SIGNAL void summaryChanged();

    Q_PROPERTY(QStringList tags READ tags WRITE setTags NOTIFY tagsChanged)
    void setTags(const QStringList &val);
    QStringList tags() const { return m_tags; }
    Q_SIGNAL void tagsChanged();

    Q_INVOKABLE bool addTag(const QString &tag);
    Q_INVOKABLE bool removeTag(const QString &tag);
    Q_INVOKABLE bool hasTag(const QString &tag) const;

    Q_PROPERTY(bool hasTags READ hasTags NOTIFY tagsChanged)
    bool hasTags() const { return !m_tags.isEmpty(); }

    Q_PROPERTY(int priority READ priority WRITE setPriority NOTIFY priorityChanged)
    void setPriority(int val);
    int priority() const { return m_priority; }
    Q_SIGNAL void priorityChanged();

    Q_PROPERTY(QAbstractListModel *relationshipsModel READ relationshipsModel CONSTANT STORED false)
    QObjectListModel<Relationship *> *relationshipsModel() const
    {
        return &((const_cast<Character *>(this))->m_relationships);
    }

    Q_PROPERTY(QQmlListProperty<Relationship> relationships READ relationships NOTIFY
                       relationshipCountChanged)
    QQmlListProperty<Relationship> relationships();
    Q_INVOKABLE void addRelationship(Relationship *ptr);
    Q_INVOKABLE void removeRelationship(Relationship *ptr);
    Q_INVOKABLE Relationship *relationshipAt(int index) const;
    void setRelationships(const QList<Relationship *> &list);
    Q_PROPERTY(int relationshipCount READ relationshipCount NOTIFY relationshipCountChanged)
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

    Q_PROPERTY(QJsonObject characterRelationshipGraph READ characterRelationshipGraph WRITE
                       setCharacterRelationshipGraph NOTIFY characterRelationshipGraphChanged)
    void setCharacterRelationshipGraph(const QJsonObject &val);
    QJsonObject characterRelationshipGraph() const { return m_characterRelationshipGraph; }
    Q_SIGNAL void characterRelationshipGraphChanged();

    Q_PROPERTY(Attachments *attachments READ attachments CONSTANT)
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
    QColor m_color = Qt::white;
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

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    int count() const;
    Q_SIGNAL void countChanged();

    Q_PROPERTY(Structure *structure READ structure WRITE setStructure NOTIFY structureChanged)
    void setStructure(Structure *val);
    Structure *structure() const { return m_structure; }
    Q_SIGNAL void structureChanged();

    Q_PROPERTY(QStringList tags READ tags WRITE setTags NOTIFY tagsChanged)
    void setTags(const QStringList &val);
    QStringList tags() const { return m_tags; }
    Q_SIGNAL void tagsChanged();

    Q_INVOKABLE void addTag(const QString &tag);
    Q_INVOKABLE void removeTag(const QString &tag);
    Q_INVOKABLE void toggleTag(const QString &tag);
    Q_INVOKABLE void clearTags();
    Q_INVOKABLE bool hasTag(const QString &tag) const;

    Q_PROPERTY(QStringList availableTags READ availableTags NOTIFY availableTagsChanged)
    QStringList availableTags() const;
    Q_SIGNAL void availableTagsChanged();

    Q_PROPERTY(QStringList allNames READ allNames NOTIFY allNamesChanged)
    QStringList allNames() const;
    Q_SIGNAL void allNamesChanged();

    Q_PROPERTY(QStringList selectedCharacters READ selectedCharacters WRITE setSelectedCharacters
                       NOTIFY selectedCharactersChanged)
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

    Q_PROPERTY(Structure *structure READ structure CONSTANT STORED false)
    Structure *structure() const { return m_structure; }

    Q_PROPERTY(QString type READ type WRITE setType NOTIFY typeChanged)
    void setType(const QString &val);
    QString type() const { return m_type; }
    Q_SIGNAL void typeChanged();

    Q_PROPERTY(
            bool resizable READ isResizable WRITE setResizable NOTIFY resizableChanged STORED false)
    void setResizable(bool val);
    bool isResizable() const { return m_resizable; }
    Q_SIGNAL void resizableChanged();

    Q_PROPERTY(bool movable READ isMovable WRITE setMovable NOTIFY movableChanged STORED false)
    void setMovable(bool val);
    bool isMovable() const { return m_movable; }
    Q_SIGNAL void movableChanged();

    Q_PROPERTY(QRectF geometry READ geometry WRITE setGeometry NOTIFY geometryChanged)
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

    Q_PROPERTY(QJsonObject attributes READ attributes WRITE setAttributes NOTIFY attributesChanged)
    void setAttributes(const QJsonObject &val);
    QJsonObject attributes() const { return m_attributes; }
    Q_SIGNAL void attributesChanged();

    Q_INVOKABLE void setAttribute(const QString &key, const QJsonValue &value);
    Q_INVOKABLE void removeAttribute(const QString &key);
    Q_INVOKABLE void saveAttributesAsDefault();

    Q_PROPERTY(
            QJsonArray metaData READ metaData WRITE setMetaData NOTIFY metaDataChanged STORED false)
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

    Q_PROPERTY(qreal canvasWidth READ canvasWidth WRITE setCanvasWidth NOTIFY canvasWidthChanged)
    void setCanvasWidth(qreal val);
    qreal canvasWidth() const { return m_canvasWidth; }
    Q_SIGNAL void canvasWidthChanged();

    Q_PROPERTY(
            qreal canvasHeight READ canvasHeight WRITE setCanvasHeight NOTIFY canvasHeightChanged)
    void setCanvasHeight(qreal val);
    qreal canvasHeight() const { return m_canvasHeight; }
    Q_SIGNAL void canvasHeightChanged();

    Q_PROPERTY(qreal canvasGridSize READ canvasGridSize WRITE setCanvasGridSize NOTIFY
                       canvasGridSizeChanged)
    void setCanvasGridSize(qreal val);
    qreal canvasGridSize() const { return m_canvasGridSize; }
    Q_SIGNAL void canvasGridSizeChanged();

    enum CanvasUIMode { SynopsisEditorUI, IndexCardUI };
    Q_ENUM(CanvasUIMode)
    Q_PROPERTY(CanvasUIMode canvasUIMode READ canvasUIMode WRITE setCanvasUIMode NOTIFY
                       canvasUIModeChanged)
    void setCanvasUIMode(CanvasUIMode val);
    CanvasUIMode canvasUIMode() const { return m_canvasUIMode; }
    Q_SIGNAL void canvasUIModeChanged();

    enum IndexCardContent { Synopsis, FeaturedPhoto };
    Q_ENUM(IndexCardContent)

    Q_PROPERTY(IndexCardContent indexCardContent READ indexCardContent WRITE setIndexCardContent
                       NOTIFY indexCardContentChanged)
    void setIndexCardContent(IndexCardContent val);
    IndexCardContent indexCardContent() const { return m_indexCardContent; }
    Q_SIGNAL void indexCardContentChanged();

    Q_INVOKABLE qreal snapToGrid(qreal val) const;
    static qreal snapToGrid(qreal val, const Structure *structure, qreal defaultGridSize = 10.0);

    void captureStructureAsImage(const QString &fileName);
    Q_SIGNAL void captureStructureAsImageRequest(const QString &fileName);

    Q_PROPERTY(ScriteDocument *scriteDocument READ scriteDocument CONSTANT STORED false)
    ScriteDocument *scriteDocument() const { return m_scriteDocument; }

    Q_PROPERTY(QAbstractListModel *charactersModel READ charactersModel CONSTANT STORED false)
    QObjectListModel<Character *> *charactersModel() const
    {
        return &((const_cast<Structure *>(this))->m_characters);
    }

    Q_PROPERTY(QQmlListProperty<Character> characters READ characters NOTIFY characterCountChanged)
    QQmlListProperty<Character> characters();
    Q_INVOKABLE void addCharacter(Character *ptr);
    Q_INVOKABLE void removeCharacter(Character *ptr);
    Q_INVOKABLE Character *characterAt(int index) const;
    Q_INVOKABLE int indexOfCharacter(Character *ptr) const;
    void setCharacters(const QList<Character *> &list);
    Q_PROPERTY(int characterCount READ characterCount NOTIFY characterCountChanged)
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

    Q_PROPERTY(Notes *notes READ notes CONSTANT)
    Notes *notes() const { return m_notes; }

    Q_PROPERTY(QAbstractListModel *elementsModel READ elementsModel CONSTANT STORED false)
    QObjectListModel<StructureElement *> *elementsModel() const
    {
        return &((const_cast<Structure *>(this))->m_elements);
    }

    Q_PROPERTY(
            QRectF elementsBoundingBox READ elementsBoundingBox NOTIFY elementsBoundingBoxChanged)
    QRectF elementsBoundingBox() const
    {
        return m_elementsBoundingBoxAggregator.aggregateValue().toRectF();
    }
    Q_SIGNAL void elementsBoundingBoxChanged();

    Q_PROPERTY(StructureElementStacks *elementStacks READ elementStacks CONSTANT STORED false)
    StructureElementStacks *elementStacks() const
    {
        return &((const_cast<Structure *>(this))->m_elementStacks);
    }

    Q_PROPERTY(QQmlListProperty<StructureElement> elements READ elements NOTIFY elementsChanged)
    QQmlListProperty<StructureElement> elements();
    Q_INVOKABLE void addElement(StructureElement *ptr);
    Q_INVOKABLE void removeElement(StructureElement *ptr);
    void removeElements(const QList<StructureElement *> &elements);
    Q_INVOKABLE void insertElement(StructureElement *ptr, int index);
    Q_INVOKABLE void moveElement(StructureElement *ptr, int toRow);
    void setElements(const QList<StructureElement *> &list);
    Q_INVOKABLE StructureElement *elementAt(int index) const;
    Q_PROPERTY(int elementCount READ elementCount NOTIFY elementCountChanged)
    int elementCount() const { return m_elements.size(); }
    Q_INVOKABLE void clearElements();
    Q_SIGNAL void elementCountChanged();
    Q_SIGNAL void elementsChanged();

    Q_INVOKABLE int indexOfScene(Scene *scene) const;
    Q_INVOKABLE int indexOfElement(StructureElement *element) const;
    Q_INVOKABLE StructureElement *findElementBySceneID(const QString &id) const;

    enum LayoutType { HorizontalLayout, VerticalLayout, FlowHorizontalLayout, FlowVerticalLayout };
    Q_ENUM(LayoutType)
    Q_INVOKABLE QRectF layoutElements(Structure::LayoutType layoutType);

    Q_PROPERTY(bool forceBeatBoardLayout READ isForceBeatBoardLayout WRITE setForceBeatBoardLayout
                       NOTIFY forceBeatBoardLayoutChanged)
    void setForceBeatBoardLayout(bool val);
    bool isForceBeatBoardLayout() const { return m_forceBeatBoardLayout; }
    Q_SIGNAL void forceBeatBoardLayoutChanged();

    Q_INVOKABLE void placeElement(StructureElement *element, Screenplay *screenplay) const;
    Q_INVOKABLE QRectF placeElementsInBeatBoardLayout(Screenplay *screenplay) const;
    Q_INVOKABLE QJsonObject evaluateEpisodeAndGroupBoxes(Screenplay *screenplay,
                                                         const QString &category) const;

    Q_INVOKABLE QJsonObject queryBreakElements(ScreenplayElement *breakElement) const;

    Q_INVOKABLE void scanForMuteCharacters();

    Q_INVOKABLE static QStringList standardLocationTypes();
    Q_INVOKABLE static QStringList standardMoments();

    Q_INVOKABLE QStringList allLocations() const { return m_locationHeadingsMap.keys(); }
    QMap<QString, QList<SceneHeading *>> locationHeadingsMap() const
    {
        return m_locationHeadingsMap;
    }

    Q_PROPERTY(int currentElementIndex READ currentElementIndex WRITE setCurrentElementIndex NOTIFY
                       currentElementIndexChanged STORED false)
    void setCurrentElementIndex(int val);
    int currentElementIndex() const { return m_currentElementIndex; }
    Q_SIGNAL void currentElementIndexChanged();

    Q_PROPERTY(qreal zoomLevel READ zoomLevel WRITE setZoomLevel NOTIFY zoomLevelChanged)
    void setZoomLevel(qreal val);
    qreal zoomLevel() const { return m_zoomLevel; }
    Q_SIGNAL void zoomLevelChanged();

    Q_PROPERTY(QStringList characterNames READ characterNames NOTIFY characterNamesChanged)
    QStringList characterNames() const { return m_characterNames; }
    Q_SIGNAL void characterNamesChanged();

    Q_PROPERTY(QStringList transitions READ transitions NOTIFY transitionsChanged)
    QStringList transitions() const { return m_transitions; }
    Q_SIGNAL void transitionsChanged();

    Q_PROPERTY(QStringList shots READ shots NOTIFY shotsChanged)
    QStringList shots() const { return m_shots; }
    Q_SIGNAL void shotsChanged();

    Q_PROPERTY(QStringList characterTags READ characterTags NOTIFY characterTagsChanged)
    QStringList characterTags() const { return m_characterTags; }
    Q_SIGNAL void characterTagsChanged();

    QStringList filteredCharacterNames(const QStringList &tags) const;

    Q_PROPERTY(QAbstractListModel *annotationsModel READ annotationsModel CONSTANT STORED false)
    QAbstractListModel *annotationsModel() const
    {
        return &((const_cast<Structure *>(this))->m_annotations);
    }

    Q_PROPERTY(QRectF annotationsBoundingBox READ annotationsBoundingBox NOTIFY
                       annotationsBoundingBoxChanged)
    QRectF annotationsBoundingBox() const
    {
        return m_annotationsBoundingBoxAggregator.aggregateValue().toRectF();
    }
    Q_SIGNAL void annotationsBoundingBoxChanged();

    Q_PROPERTY(
            QQmlListProperty<Annotation> annotations READ annotations NOTIFY annotationCountChanged)
    QQmlListProperty<Annotation> annotations();
    Q_INVOKABLE void addAnnotation(Annotation *ptr);
    Q_INVOKABLE void removeAnnotation(Annotation *ptr);
    Q_INVOKABLE Annotation *annotationAt(int index) const;
    Q_INVOKABLE bool canBringToFront(Annotation *ptr) const;
    Q_INVOKABLE bool canSendToBack(Annotation *ptr) const;
    Q_INVOKABLE void bringToFront(Annotation *ptr);
    Q_INVOKABLE void sendToBack(Annotation *ptr);
    void setAnnotations(const QList<Annotation *> &list);
    Q_PROPERTY(int annotationCount READ annotationCount NOTIFY annotationCountChanged)
    int annotationCount() const { return m_annotations.size(); }
    Q_INVOKABLE void clearAnnotations();
    Q_SIGNAL void annotationCountChanged();

    Q_PROPERTY(QString defaultGroupsDataFile READ defaultGroupsDataFile CONSTANT)
    QString defaultGroupsDataFile() const;

    Q_INVOKABLE void loadDefaultGroupsData();

    Q_PROPERTY(QString groupsData READ groupsData WRITE setGroupsData NOTIFY groupsDataChanged)
    void setGroupsData(const QString &val);
    QString groupsData() const { return m_groupsData; }
    Q_SIGNAL void groupsDataChanged();

    Q_PROPERTY(QJsonArray groupsModel READ groupsModel NOTIFY groupsModelChanged)
    QJsonArray groupsModel() const { return m_groupsModel; }
    Q_SIGNAL void groupsModelChanged();

    Q_PROPERTY(QStringList groupCategories READ groupCategories NOTIFY groupsModelChanged)
    QStringList groupCategories() const { return m_groupCategories; }

    Q_PROPERTY(QVariantMap categoryActNames READ categoryActNames NOTIFY groupsModelChanged)
    QVariantMap categoryActNames() const { return m_categoryActNames; }

    Q_PROPERTY(QString preferredGroupCategory READ preferredGroupCategory WRITE
                       setPreferredGroupCategory NOTIFY preferredGroupCategoryChanged)
    void setPreferredGroupCategory(const QString &val);
    QString preferredGroupCategory() const { return m_preferredGroupCategory; }
    Q_SIGNAL void preferredGroupCategoryChanged();

    Q_INVOKABLE QString presentableGroupNames(const QStringList &groups) const;

    // Local to the current Scrite document
    Q_PROPERTY(QJsonArray indexCardFields READ indexCardFields WRITE setIndexCardFields NOTIFY indexCardFieldsChanged)
    void setIndexCardFields(const QJsonArray &val);
    QJsonArray indexCardFields() const { return m_indexCardFields; }
    Q_SIGNAL void indexCardFieldsChanged();

    Q_PROPERTY(QJsonArray defaultIndexCardFields READ defaultIndexCardFields WRITE setDefaultIndexCardFields NOTIFY defaultIndexCardFieldsChanged STORED false)
    void setDefaultIndexCardFields(const QJsonArray &val);
    QJsonArray defaultIndexCardFields() const { return m_defaultIndexCardFields; }
    Q_SIGNAL void defaultIndexCardFieldsChanged();

    void loadDefaultIndexCardFields();

    Q_INVOKABLE Annotation *createAnnotation(const QString &type);

    Q_SIGNAL void structureChanged();

    Q_PROPERTY(bool canPaste READ canPaste NOTIFY canPasteChanged STORED false)
    bool canPaste() const { return m_canPaste; }
    Q_SIGNAL void canPasteChanged();

    Q_INVOKABLE void copy(QObject *elementOrAnnotation);
    Q_INVOKABLE void paste(const QPointF &pos = QPointF());

    Q_PROPERTY(QJsonObject characterRelationshipGraph READ characterRelationshipGraph WRITE
                       setCharacterRelationshipGraph NOTIFY characterRelationshipGraphChanged)
    void setCharacterRelationshipGraph(const QJsonObject &val);
    QJsonObject characterRelationshipGraph() const { return m_characterRelationshipGraph; }
    Q_SIGNAL void characterRelationshipGraphChanged();

    Q_PROPERTY(Attachments *attachments READ attachments CONSTANT)
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

    friend class StructureLayout;
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
    ExecLaterTimer m_updateCharacterNamesShotsTransitionsAndTagsTimer;
    CharacterElementMap m_characterElementMap;
    TransitionElementMap m_transitionElementMap;
    ShotElementMap m_shotElementMap;
    QStringList m_characterTags;
    QStringList m_characterNames;
    QStringList m_shots;
    QStringList m_transitions;

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
    Q_PROPERTY(LineType lineType READ lineType WRITE setLineType NOTIFY lineTypeChanged)
    void setLineType(LineType val);
    LineType lineType() const { return m_lineType; }
    Q_SIGNAL void lineTypeChanged();

    Q_PROPERTY(StructureElement *fromElement READ fromElement WRITE setFromElement NOTIFY
                       fromElementChanged RESET resetFromElement)
    void setFromElement(StructureElement *val);
    StructureElement *fromElement() const { return m_fromElement; }
    Q_SIGNAL void fromElementChanged();

    Q_PROPERTY(StructureElement *toElement READ toElement WRITE setToElement NOTIFY toElementChanged
                       RESET resetToElement)
    void setToElement(StructureElement *val);
    StructureElement *toElement() const { return m_toElement; }
    Q_SIGNAL void toElementChanged();

    Q_PROPERTY(qreal arrowAndLabelSpacing READ arrowAndLabelSpacing WRITE setArrowAndLabelSpacing
                       NOTIFY arrowAndLabelSpacingChanged)
    void setArrowAndLabelSpacing(qreal val);
    qreal arrowAndLabelSpacing() const { return m_arrowAndLabelSpacing; }
    Q_SIGNAL void arrowAndLabelSpacingChanged();

    Q_PROPERTY(QPointF arrowPosition READ arrowPosition NOTIFY arrowPositionChanged)
    QPointF arrowPosition() const { return m_arrowPosition; }
    Q_SIGNAL void arrowPositionChanged();

    Q_PROPERTY(QPointF suggestedLabelPosition READ suggestedLabelPosition NOTIFY
                       suggestedLabelPositionChanged)
    QPointF suggestedLabelPosition() const { return m_suggestedLabelPosition; }
    Q_SIGNAL void suggestedLabelPositionChanged();

    Q_PROPERTY(bool canBeVisible READ canBeVisible NOTIFY canBeVisibleChanged)
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

    Q_PROPERTY(Structure *structure READ structure WRITE setStructure RESET resetStructure NOTIFY
                       structureChanged)
    void setStructure(Structure *val);
    Structure *structure() const { return m_structure; }
    Q_SIGNAL void structureChanged();

    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    void setEnabled(bool val);
    bool isEnabled() const { return m_enabled; }
    Q_SIGNAL void enabledChanged();

    enum Type { AnnotationType, StructureElementType };
    Q_ENUM(Type)
    Q_PROPERTY(Type type READ type WRITE setType NOTIFY typeChanged)
    void setType(Type val);
    Type type() const { return m_type; }
    Q_SIGNAL void typeChanged();

    Q_PROPERTY(
            QRectF viewportRect READ viewportRect WRITE setViewportRect NOTIFY viewportRectChanged)
    void setViewportRect(const QRectF &val);
    QRectF viewportRect() const { return m_viewportRect; }
    Q_SIGNAL void viewportRectChanged();

    enum ComputeStrategy { PreComputeStrategy, OnDemandComputeStrategy };
    Q_ENUM(ComputeStrategy)
    Q_PROPERTY(ComputeStrategy computeStrategy READ computeStrategy WRITE setComputeStrategy NOTIFY
                       computeStrategyChanged)
    void setComputeStrategy(ComputeStrategy val);
    ComputeStrategy computeStrategy() const { return m_computeStrategy; }
    Q_SIGNAL void computeStrategyChanged();

    enum FilterStrategy { ContainsStrategy, IntersectsStrategy };
    Q_ENUM(FilterStrategy)
    Q_PROPERTY(FilterStrategy filterStrategy READ filterStrategy WRITE setFilterStrategy NOTIFY
                       filterStrategyChanged)
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
