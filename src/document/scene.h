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

#ifndef SCENE_H
#define SCENE_H

#include <QMap>
#include <QList>
#include <QColor>
#include <QPointer>
#include <QQmlEngine>
#include <QJsonArray>
#include <QTextLayout>
#include <QUndoCommand>
#include <QReadWriteLock>
#include <QQmlListProperty>
#include <QQuickPaintedItem>
#include <QAbstractListModel>
#include <QQuickTextDocument>
#include <QImage>

#include "notes.h"
#include "modifiable.h"
#include "attachments.h"
#include "execlatertimer.h"
#include "qobjectproperty.h"
#include "qobjectserializer.h"
#include "spellcheckservice.h"
#include "genericarraymodel.h"
#include "qobjectlistmodel.h"

class Scene;
class SceneHeading;
class SceneElement;
class StructureElement;
class SceneDocumentBinder;
class PushSceneUndoCommand;

class SceneHeading : public QObject, public Modifiable
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    explicit SceneHeading(QObject *parent = nullptr);
    ~SceneHeading();

    Q_PROPERTY(Scene *scene READ scene CONSTANT STORED false)
    Scene *scene() const { return m_scene; }

    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    void setEnabled(bool val);
    bool isEnabled() const { return m_enabled; }
    Q_SIGNAL void enabledChanged();

    Q_PROPERTY(
            QString locationType READ locationType WRITE setLocationType NOTIFY locationTypeChanged)
    void setLocationType(const QString &val);
    QString locationType() const { return m_locationType; }
    Q_SIGNAL void locationTypeChanged();

    Q_PROPERTY(QString location READ location WRITE setLocation NOTIFY locationChanged)
    void setLocation(const QString &val);
    QString location() const { return m_location; }
    Q_SIGNAL void locationChanged();

    Q_PROPERTY(QString moment READ moment WRITE setMoment NOTIFY momentChanged)
    void setMoment(const QString &val);
    QString moment() const { return m_moment; }
    Q_SIGNAL void momentChanged();

    Q_PROPERTY(QString editText READ text NOTIFY textChanged)
    Q_PROPERTY(QString text READ text NOTIFY textChanged)
    QString text() const { return this->toString(EditMode); }
    Q_SIGNAL void textChanged();

    Q_PROPERTY(QString displayText READ displayText NOTIFY textChanged)
    QString displayText() const { return this->toString(DisplayMode); }

    static bool parse(const QString &text, QString &locationType, QString &location,
                      QString &moment, bool strict = false);

    Q_INVOKABLE void parseFrom(const QString &text);

    Q_PROPERTY(int wordCount READ wordCount NOTIFY wordCountChanged)
    int wordCount() const { return m_wordCount; }
    Q_SIGNAL void wordCountChanged();

protected:
    void timerEvent(QTimerEvent *event);

private:
    friend class Scene;
    void renameCharacter(const QString &from, const QString &to);

    enum Mode { DisplayMode, EditMode };
    QString toString(Mode mode) const;
    void setWordCount(int val);
    void evaluateWordCount();
    void evaluateWordCountLater();

private:
    bool m_enabled = true;
    char m_padding[3];
    Scene *m_scene = nullptr;
    QString m_moment = "DAY";
    QString m_location = "Somewhere";
    QString m_locationType = "EXT";
    int m_wordCount = 0;
    QBasicTimer m_wordCountTimer;
};

class SceneElement : public QObject, public Modifiable, public QObjectSerializer::Interface
{
    Q_OBJECT
    Q_INTERFACES(QObjectSerializer::Interface)
    QML_ELEMENT

public:
    Q_INVOKABLE explicit SceneElement(QObject *parent = nullptr);
    ~SceneElement();
    Q_SIGNAL void aboutToDelete(SceneElement *element);

    Q_PROPERTY(Scene *scene READ scene CONSTANT STORED false)
    Scene *scene() const { return m_scene; }

    Q_PROPERTY(SpellCheckService *spellCheck READ spellCheck CONSTANT STORED false)
    SpellCheckService *spellCheck() const;

    /*
     * The 'id' is a special property. It can be set only once. If it is not
     * set an ID is automatically generated whenever the property value is
     * queried for the first time.
     */
    Q_PROPERTY(QString id READ id WRITE setId NOTIFY idChanged)
    void setId(const QString &val);
    QString id() const;
    Q_SIGNAL void idChanged();

    enum Type {
        Action,
        Character,
        Dialogue,
        Parenthetical,
        Shot,
        Transition,
        Heading,
        Min = Action,
        Max = Heading,
        All = -1
    };
    Q_ENUM(Type)
    Q_PROPERTY(Type type READ type WRITE setType NOTIFY typeChanged)
    Q_CLASSINFO("UndoBundleFor_type", "cursorPosition")
    void setType(Type val);
    Type type() const { return m_type; }
    Q_SIGNAL void typeChanged();

    Q_PROPERTY(QString typeAsString READ typeAsString NOTIFY typeChanged)
    QString typeAsString() const;

    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    void setText(const QString &val);
    QString text() const { return m_text; }
    Q_SIGNAL void textChanged(const QString &val);

    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment NOTIFY alignmentChanged)
    void setAlignment(Qt::Alignment val);
    Qt::Alignment alignment() const;
    Q_SIGNAL void alignmentChanged();

    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY
                       cursorPositionChanged STORED false)
    void setCursorPosition(int val);
    int cursorPosition() const;
    Q_SIGNAL void cursorPositionChanged();

    QString formattedText() const;

    bool polishText(Scene *previousScene = nullptr);
    bool capitalizeSentences();
    QList<int> autoCapitalizePositions() const;
    static QList<int> autoCapitalizePositions(const QString &text);

    Q_PROPERTY(int wordCount READ wordCount NOTIFY wordCountChanged)
    int wordCount() const { return m_wordCount; }
    Q_SIGNAL void wordCountChanged();

    Q_SIGNAL void elementChanged();

    Q_INVOKABLE QJsonArray find(const QString &text, int flags) const;

    void serializeToJson(QJsonObject &) const;
    void deserializeFromJson(const QJsonObject &obj);

    // For use with SceneDocumentBinder, ScreenplayTextDocument
    void setTextFormats(const QVector<QTextLayout::FormatRange> &formats);
    QVector<QTextLayout::FormatRange> textFormats() const { return m_textFormats; }

    static QJsonArray textFormatsToJson(const QVector<QTextLayout::FormatRange> &formats);
    static QVector<QTextLayout::FormatRange> textFormatsFromJson(const QJsonArray &array);

protected:
    bool event(QEvent *event);
    void timerEvent(QTimerEvent *event);

private:
    friend class Scene;
    void renameCharacter(const QString &from, const QString &to);
    void reportSceneElementChanged(int type);
    void setWordCount(int val);
    void evaluateWordCount();
    void evaluateWordCountLater();

private:
    mutable QString m_id;
    Type m_type = Action;
    QString m_text;
    Qt::Alignment m_alignment;
    QVector<QTextLayout::FormatRange> m_textFormats;
    Scene *m_scene = nullptr;
    int m_wordCount = 0;
    mutable SpellCheckService *m_spellCheck = nullptr;
    QBasicTimer m_changeTimer;
    QBasicTimer m_wordCountTimer;
    QMap<int, int> m_changeCounters;
};

class DistinctElementValuesMap
{
public:
    DistinctElementValuesMap(SceneElement::Type type = SceneElement::Character);
    ~DistinctElementValuesMap();

    // These functions returns true if distinctValues() would return
    // a different list after this function returns
    bool include(SceneElement *element);
    bool remove(SceneElement *element);
    bool remove(const QString &name);
    bool isEmpty() const { return m_forwardMap.isEmpty() && m_reverseMap.isEmpty(); }

    QStringList distinctValues() const;
    bool containsValue(const QString &value) const;
    QList<SceneElement *> elements() const;
    QList<SceneElement *> elements(const QString &value) const;

    void include(const DistinctElementValuesMap &other);

private:
    SceneElement::Type m_type = SceneElement::Character;
    QMap<SceneElement *, QString> m_forwardMap;
    QMap<QString, QList<SceneElement *>> m_reverseMap;
};

class CharacterElementMap : public DistinctElementValuesMap
{
public:
    explicit CharacterElementMap() : DistinctElementValuesMap(SceneElement::Character) { }
    ~CharacterElementMap() { }

    QStringList characterNames() const { return this->distinctValues(); }
    bool containsCharacter(const QString &name) const { return this->containsValue(name); }
    QList<SceneElement *> characterElements() const { return this->elements(); }
    QList<SceneElement *> characterElements(const QString &name) const
    {
        return this->elements(name);
    }
};

class TransitionElementMap : public DistinctElementValuesMap
{
public:
    explicit TransitionElementMap() : DistinctElementValuesMap(SceneElement::Transition) { }
    ~TransitionElementMap() { }

    QStringList transitions() const { return this->distinctValues(); }
    bool containsTransition(const QString &name) const { return this->containsValue(name); }
    QList<SceneElement *> transitionElements() const { return this->elements(); }
    QList<SceneElement *> transitionElements(const QString &name) const
    {
        return this->elements(name);
    }
};

class ShotElementMap : public DistinctElementValuesMap
{
public:
    explicit ShotElementMap() : DistinctElementValuesMap(SceneElement::Shot) { }
    ~ShotElementMap() { }

    QStringList shots() const { return this->distinctValues(); }
    bool containsShot(const QString &name) const { return this->containsValue(name); }
    QList<SceneElement *> shotElements() const { return this->elements(); }
    QList<SceneElement *> shotElements(const QString &name) const { return this->elements(name); }
};

class Scene : public QAbstractListModel, public QObjectSerializer::Interface, public Modifiable
{
    Q_OBJECT
    Q_INTERFACES(QObjectSerializer::Interface)
    QML_ELEMENT

public:
    Q_INVOKABLE explicit Scene(QObject *parent = nullptr);
    ~Scene();
    Q_SIGNAL void aboutToDelete(Scene *scene);

    Scene *clone(QObject *parent) const;

    Q_PROPERTY(StructureElement *structureElement READ structureElement NOTIFY
                       structureElementChanged STORED false)
    StructureElement *structureElement() const { return m_structureElement; }
    Q_SIGNAL void structureElementChanged();

    Q_PROPERTY(bool empty READ isEmpty NOTIFY sceneChanged)
    bool isEmpty() const;

    Q_PROPERTY(bool hasContent READ hasContent NOTIFY sceneChanged)
    bool hasContent() const;

    /*
     * The 'id' is a special property. It can be set only once. If it is not
     * set an ID is automatically generated whenever the property value is
     * queried for the first time.
     */
    Q_PROPERTY(QString id READ id WRITE setId NOTIFY idChanged)
    void setId(const QString &val);
    QString id() const;
    Q_SIGNAL void idChanged();

    Q_PROPERTY(QString name READ name NOTIFY synopsisChanged)
    QString name() const;

    Q_PROPERTY(QString title READ synopsis WRITE setSynopsis NOTIFY synopsisChanged STORED false)
    Q_PROPERTY(QString synopsis READ synopsis WRITE setSynopsis NOTIFY synopsisChanged)
    void setSynopsis(const QString &val);
    QString synopsis() const { return m_synopsis; }
    Q_SIGNAL void synopsisChanged();

    void inferSynopsisFromContent();

    Q_PROPERTY(bool hasSynopsis READ hasSynopsis NOTIFY synopsisChanged)
    bool hasSynopsis() const { return !m_synopsis.isEmpty(); }

    Q_INVOKABLE void trimSynopsis();

    Q_PROPERTY(QStringList indexCardFieldValues READ indexCardFieldValues WRITE setIndexCardFieldValues NOTIFY indexCardFieldValuesChanged)
    void setIndexCardFieldValues(const QStringList &val);
    QStringList indexCardFieldValues() const { return m_indexCardFieldValues; }
    Q_SIGNAL void indexCardFieldValuesChanged();

    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    void setColor(const QColor &val);
    QColor color() const { return m_color; }
    Q_SIGNAL void colorChanged();

    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    void setEnabled(bool val);
    bool isEnabled() const { return m_enabled; }
    Q_SIGNAL void enabledChanged();

    // Keep this updated and synced with SceneTypeImage.qml
    Q_CLASSINFO("enum_Standard_icon", "qrc:/icons/content/blank.png")
    Q_CLASSINFO("enum_Song_icon", "qrc:/icons/content/queue_mus24px.png")
    Q_CLASSINFO("enum_Action_icon", "qrc:/icons/content/fight_scene.png")
    Q_CLASSINFO("enum_Montage_icon", "qrc:/icons/content/camera_alt.png")

    enum Type { Standard = 0, Song, Action, Montage };
    Q_ENUM(Type)
    Q_PROPERTY(Type type READ type WRITE setType NOTIFY typeChanged)
    void setType(Type val);
    Type type() const { return m_type; }
    Q_SIGNAL void typeChanged();

    Q_PROPERTY(QString comments READ comments WRITE setComments NOTIFY commentsChanged)
    void setComments(const QString &val);
    QString comments() const { return m_comments; }
    Q_SIGNAL void commentsChanged();

    Q_PROPERTY(bool isBeingReset READ isBeingReset NOTIFY resetStateChanged)
    bool isBeingReset() const { return m_isBeingReset; }
    Q_SIGNAL void resetStateChanged();

    Q_PROPERTY(bool undoRedoEnabled READ isUndoRedoEnabled WRITE setUndoRedoEnabled NOTIFY
                       undoRedoEnabledChanged STORED false)
    void setUndoRedoEnabled(bool val);
    bool isUndoRedoEnabled() const { return m_undoRedoEnabled; }
    Q_SIGNAL void undoRedoEnabledChanged();

    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY
                       cursorPositionChanged STORED false)
    void setCursorPosition(int val);
    int cursorPosition() const { return m_cursorPosition; }
    Q_SIGNAL void cursorPositionChanged();

    Q_PROPERTY(SceneHeading *heading READ heading CONSTANT)
    SceneHeading *heading() const { return m_heading; }

    Q_PROPERTY(bool hasCharacters READ hasCharacters NOTIFY characterNamesChanged)
    bool hasCharacters() const { return !m_characterElementMap.isEmpty(); }

    Q_PROPERTY(QStringList characterNames READ characterNames NOTIFY characterNamesChanged)
    QStringList characterNames() const { return m_sortedCharacterNames; }
    Q_SIGNAL void characterNamesChanged();

    Q_INVOKABLE bool hasCharacter(const QString &characterName) const;
    Q_INVOKABLE int characterPresence(const QString &characterName) const;
    Q_INVOKABLE void addMuteCharacter(const QString &characterName);
    Q_INVOKABLE void removeMuteCharacter(const QString &characterName);
    Q_INVOKABLE bool isCharacterMute(const QString &characterName) const;
    Q_INVOKABLE bool isCharacterVisible(const QString &characterName) const;
    void scanMuteCharacters(const QStringList &characterNames = QStringList());

    Q_PROPERTY(QString act READ act NOTIFY actChanged STORED false)
    void setAct(const QString &val);
    QString act() const { return m_act; }
    Q_SIGNAL void actChanged();

    Q_PROPERTY(int actIndex READ actIndex NOTIFY actIndexChanged STORED false)
    void setActIndex(const int &val);
    int actIndex() const { return m_actIndex; }
    Q_SIGNAL void actIndexChanged();

    Q_PROPERTY(int episodeIndex READ episodeIndex NOTIFY episodeIndexChanged)
    void setEpisodeIndex(const int &val);
    int episodeIndex() const { return m_episodeIndex; }
    Q_SIGNAL void episodeIndexChanged();

    Q_PROPERTY(QString episode READ episode NOTIFY episodeChanged)
    void setEpisode(const QString &val);
    QString episode() const { return m_episode; }
    Q_SIGNAL void episodeChanged();

    Q_PROPERTY(QList<int> screenplayElementIndexList READ screenplayElementIndexList NOTIFY
                       screenplayElementIndexListChanged STORED false)
    void setScreenplayElementIndexList(const QList<int> &val);
    QList<int> screenplayElementIndexList() const { return m_screenplayElementIndexList; }
    Q_SIGNAL void screenplayElementIndexListChanged();

    Q_PROPERTY(bool addedToScreenplay READ isAddedToScreenplay NOTIFY addedToScreenplayChanged)
    bool isAddedToScreenplay() const { return !m_screenplayElementIndexList.isEmpty(); }
    Q_SIGNAL void addedToScreenplayChanged();

    Q_PROPERTY(QStringList groups READ groups WRITE setGroups NOTIFY groupsChanged)
    void setGroups(const QStringList &val);
    QStringList groups() const { return m_groups; }
    Q_SIGNAL void groupsChanged();

    Q_INVOKABLE void addToGroup(const QString &group);
    Q_INVOKABLE void removeFromGroup(const QString &group);
    Q_INVOKABLE bool isInGroup(const QString &group) const;
    void verifyGroups(const QJsonArray &groupsModel);

    Q_PROPERTY(int wordCount READ wordCount NOTIFY wordCountChanged)
    int wordCount() const { return m_wordCount; }
    Q_SIGNAL void wordCountChanged();

    Q_PROPERTY(QQmlListProperty<SceneElement> elements READ elements NOTIFY elementCountChanged)
    QQmlListProperty<SceneElement> elements();
    Q_INVOKABLE SceneElement *appendElement(const QString &text, int type = SceneElement::Action);
    Q_INVOKABLE void addElement(SceneElement *ptr);
    Q_INVOKABLE void insertElementAfter(SceneElement *ptr, SceneElement *after);
    Q_INVOKABLE void insertElementBefore(SceneElement *ptr, SceneElement *before);
    Q_INVOKABLE void insertElementAt(SceneElement *ptr, int index);
    Q_INVOKABLE void removeElement(SceneElement *ptr);
    Q_INVOKABLE int indexOfElement(SceneElement *ptr) { return m_elements.indexOf(ptr); }
    Q_INVOKABLE SceneElement *elementAt(int index) const;
    void setElements(const QList<SceneElement *> &list);
    Q_PROPERTY(int elementCount READ elementCount NOTIFY elementCountChanged)
    int elementCount() const;
    Q_INVOKABLE void clearElements();
    Q_SIGNAL void elementCountChanged();

    Q_INVOKABLE void removeLastElementIfEmpty();

    enum SceneElementChangeType { ElementTypeChange, ElementTextChange };
    Q_SIGNAL void sceneElementChanged(SceneElement *element, Scene::SceneElementChangeType type);
    Q_SIGNAL void aboutToRemoveSceneElement(SceneElement *element);
    Q_SIGNAL void sceneChanged();
    Q_SIGNAL void sceneRefreshed();
    Q_SIGNAL void sceneAboutToReset();
    Q_SIGNAL void sceneReset(int elementIndex);

    Q_PROPERTY(Notes *notes READ notes CONSTANT)
    Notes *notes() const { return m_notes; }

    Q_INVOKABLE void beginUndoCapture(bool allowMerging = true);
    Q_INVOKABLE void endUndoCapture();

    Q_INVOKABLE bool polishText(Scene *previousScene = nullptr);
    Q_INVOKABLE bool capitalizeSentences();

    Q_PROPERTY(QString summary READ summary NOTIFY summaryChanged)
    QString summary() const { return m_summary; }
    Q_SIGNAL void summaryChanged();

    // Used by stats report generator code.
    QHash<QString, QList<SceneElement *>> dialogueElements() const;

    Scene *splitScene(SceneElement *element, int textPosition, QObject *parent = nullptr);
    bool mergeInto(Scene *another);

    // QAbstractItemModel interface
    enum Roles { SceneElementRole = Qt::UserRole };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

    // Serializing functions for use with Undo/Redo
    QByteArray toByteArray() const;
    bool resetFromByteArray(const QByteArray &bytes);
    static Scene *fromByteArray(const QByteArray &bytes);

    Q_PROPERTY(QJsonObject characterRelationshipGraph READ characterRelationshipGraph WRITE
                       setCharacterRelationshipGraph NOTIFY characterRelationshipGraphChanged)
    void setCharacterRelationshipGraph(const QJsonObject &val);
    QJsonObject characterRelationshipGraph() const { return m_characterRelationshipGraph; }
    Q_SIGNAL void characterRelationshipGraphChanged();

    Q_PROPERTY(Attachments *attachments READ attachments CONSTANT)
    Attachments *attachments() const { return m_attachments; }

    // QObjectSerializer::Interface interface
    void serializeToJson(QJsonObject &json) const;
    void deserializeFromJson(const QJsonObject &json);
    bool canSetPropertyFromObjectList(const QString &propName) const;
    void setPropertyFromObjectList(const QString &propName, const QList<QObject *> &objects);

    // Text Document Export Support
    struct WriteOptions
    {
        WriteOptions() { }
        int headingLevel = 3;
        bool includeFeaturedPhoto = true;
        bool includeSynopsis = true;
        bool includeIndexCardFields = true;
        bool includeComments = true;
        bool includeContent = false;
        bool includeTextNotes = true;
        bool includeFormNotes = true;
        bool includeHeading = true;
    };
    void write(QTextCursor &cursor, const WriteOptions &options = WriteOptions()) const;

protected:
    bool event(QEvent *event);
    void timerEvent(QTimerEvent *event);

private:
    void setStructureElement(StructureElement *ptr);
    QList<SceneElement *> elementsList() const { return m_elements; }
    void setElementsList(const QList<SceneElement *> &list);
    void onSceneElementChanged(SceneElement *element, SceneElementChangeType type);
    void onAboutToRemoveSceneElement(SceneElement *element);
    const CharacterElementMap &characterElementMap() const { return m_characterElementMap; }
    void renameCharacter(const QString &from, const QString &to);
    void evaluateSortedCharacterNames();
    void setWordCount(int val);
    void evaluateWordCount();
    void evaluateWordCountLater();
    void trimIndexCardFieldValues();

    void evaluateSummary();
    void setSummary(const QString &val);

private:
    friend class Structure;
    friend class StructureElement;
    friend class SceneElement;
    friend class SceneHeading;
    friend class SceneDocumentBinder;

    QString m_act;
    Type m_type = Standard;
    QColor m_color = QColor(Qt::white);
    QString m_synopsis;
    QString m_comments;
    QStringList m_groups;
    QStringList m_indexCardFieldValues;
    int m_actIndex = -1;
    int m_episodeIndex = -1;
    int m_wordCount = 0;
    QBasicTimer m_wordCountTimer;
    QString m_episode;
    StructureElement *m_structureElement = nullptr;
    QString m_summary;

    bool m_enabled = true;
    char m_padding[7];
    mutable QString m_id;
    int m_cursorPosition = -1;
    SceneHeading *m_heading = new SceneHeading(this);
    bool m_isBeingReset = false;
    bool m_undoRedoEnabled = false;
    bool m_inSetElementsList = false;
    PushSceneUndoCommand *m_pushUndoCommand = nullptr;
    QJsonObject m_characterRelationshipGraph;
    QStringList m_sortedCharacterNames;
    CharacterElementMap m_characterElementMap;
    QList<int> m_screenplayElementIndexList;

    static void staticAppendElement(QQmlListProperty<SceneElement> *list, SceneElement *ptr);
    static void staticClearElements(QQmlListProperty<SceneElement> *list);
    static SceneElement *staticElementAt(QQmlListProperty<SceneElement> *list, int index);
    static int staticElementCount(QQmlListProperty<SceneElement> *list);
    QList<SceneElement *> m_elements;

    Notes *m_notes = new Notes(this);
    Attachments *m_attachments = new Attachments(this);
};

class ScreenplayFormat;
class SceneSizeHintItem : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit SceneSizeHintItem(QQuickItem *parent = nullptr);
    ~SceneSizeHintItem();

    Q_PROPERTY(Scene *scene READ scene WRITE setScene NOTIFY sceneChanged RESET sceneReset)
    void setScene(Scene *val);
    Scene *scene() const { return m_scene; }
    Q_SIGNAL void sceneChanged();

    Q_PROPERTY(bool trackSceneChanges READ trackSceneChanges WRITE setTrackSceneChanges NOTIFY
                       trackSceneChangesChanged)
    void setTrackSceneChanges(bool val);
    bool trackSceneChanges() const { return m_trackSceneChanges; }
    Q_SIGNAL void trackSceneChangesChanged();

    Q_PROPERTY(ScreenplayFormat *format READ format WRITE setFormat NOTIFY formatChanged)
    void setFormat(ScreenplayFormat *val);
    ScreenplayFormat *format() const { return m_format; }
    Q_SIGNAL void formatChanged();

    Q_PROPERTY(bool trackFormatChanges READ trackFormatChanges WRITE setTrackFormatChanges NOTIFY
                       trackFormatChangesChanged)
    void setTrackFormatChanges(bool val);
    bool trackFormatChanges() const { return m_trackFormatChanges; }
    Q_SIGNAL void trackFormatChangesChanged();

    Q_PROPERTY(qreal contentWidth READ contentWidth NOTIFY contentWidthChanged)
    qreal contentWidth() const { return m_contentWidth; }
    Q_SIGNAL void contentWidthChanged();

    Q_PROPERTY(qreal contentHeight READ contentHeight NOTIFY contentHeightChanged)
    qreal contentHeight() const { return m_contentHeight; }
    Q_SIGNAL void contentHeightChanged();

    Q_PROPERTY(bool asynchronous READ isAsynchronous WRITE setAsynchronous NOTIFY asynchronousChanged)
    void setAsynchronous(bool val);
    bool isAsynchronous() const { return m_asynchronous; }
    Q_SIGNAL void asynchronousChanged();

    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)
    void setActive(bool val);
    bool isActive() const { return m_active; }
    Q_SIGNAL void activeChanged();

    Q_PROPERTY(bool hasPendingComputeSize READ hasPendingComputeSize NOTIFY
                       hasPendingComputeSizeChanged)
    bool hasPendingComputeSize() const { return m_hasPendingComputeSize; }
    Q_SIGNAL void hasPendingComputeSizeChanged();

    // QQmlParserStatus interface
    void classBegin();
    void componentComplete();

protected:
    void timerEvent(QTimerEvent *te);
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *);

private:
    void updateSize(const QSizeF &size);
    void updateSizeAndImageLater();
    void updateSizeAndImageNow();
    void sceneReset();
    void onSceneChanged();
    void formatReset();
    void onFormatChanged();

    void setContentWidth(qreal val);
    void setContentHeight(qreal val);
    void setHasPendingComputeSize(bool val);

private:
    bool m_active = true;
    bool m_asynchronous = true;
    qreal m_contentWidth = 0;
    qreal m_contentHeight = 0;
    QImage m_documentImage;
    bool m_componentComplete = false;
    bool m_trackSceneChanges = true;
    bool m_trackFormatChanges = true;
    ExecLaterTimer m_updateTimer;
    bool m_hasPendingComputeSize = false;
    QObjectProperty<Scene> m_scene;
    QObjectProperty<ScreenplayFormat> m_format;
};

// Used only for querying and applying tagging to a bunch of scenes
class SceneGroup : public GenericArrayModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit SceneGroup(QObject *parent = nullptr);
    ~SceneGroup();

    Q_INVOKABLE void toggle(int row);
    Q_SIGNAL void toggled(int row);

    Q_PROPERTY(QStringList groupActs READ groupActs NOTIFY groupActsChanged)
    QStringList groupActs() const { return m_groupActs; }
    Q_SIGNAL void groupActsChanged();

    Q_PROPERTY(bool hasGroupActs READ hasGroupActs NOTIFY groupActsChanged)
    bool hasGroupActs() const { return !m_groupActs.isEmpty(); }

    Q_PROPERTY(QStringList sceneStackIds READ sceneStackIds NOTIFY sceneStackIdsChanged)
    QStringList sceneStackIds() const { return m_sceneStackIds; }
    Q_SIGNAL void sceneStackIdsChanged();

    Q_PROPERTY(bool hasSceneStackIds READ hasSceneStackIds NOTIFY sceneStackIdsChanged)
    bool hasSceneStackIds() const { return !m_sceneStackIds.isEmpty(); }

    Q_PROPERTY(QStringList sceneActs READ sceneActs NOTIFY sceneActsChanged)
    QStringList sceneActs() const { return m_sceneActs; }
    Q_SIGNAL void sceneActsChanged();

    Q_PROPERTY(bool hasSceneActs READ hasSceneActs NOTIFY sceneActsChanged)
    bool hasSceneActs() const { return !m_sceneActs.isEmpty(); }

    // Custom properties
    Q_PROPERTY(Structure *structure READ structure WRITE setStructure NOTIFY structureChanged)
    void setStructure(Structure *val);
    Structure *structure() const { return m_structure; }
    Q_SIGNAL void structureChanged();

    Q_PROPERTY(QQmlListProperty<Scene> scenes READ scenes NOTIFY sceneCountChanged)
    QQmlListProperty<Scene> scenes();
    Q_INVOKABLE void addScene(Scene *ptr);
    Q_INVOKABLE void removeScene(Scene *ptr);
    Q_INVOKABLE Scene *sceneAt(int index) const;
    Q_PROPERTY(int sceneCount READ sceneCount NOTIFY sceneCountChanged)
    int sceneCount() const { return m_scenes.size(); }
    Q_INVOKABLE void clearScenes();
    Q_SIGNAL void sceneCountChanged();

protected:
    void timerEvent(QTimerEvent *te);

private:
    void setSceneActs(const QStringList &val);
    void setGroupActs(const QStringList &val);
    void setSceneStackIds(const QStringList &val);
    void reload();
    void reeval();
    void reevalLater();

private:
    static void staticAppendScene(QQmlListProperty<Scene> *list, Scene *ptr);
    static void staticClearScenes(QQmlListProperty<Scene> *list);
    static Scene *staticSceneAt(QQmlListProperty<Scene> *list, int index);
    static int staticSceneCount(QQmlListProperty<Scene> *list);
    QStringList m_sceneActs;
    QStringList m_groupActs;
    QStringList m_sceneStackIds;
    QList<Scene *> m_scenes;
    QJsonArray &m_groups;
    QObjectProperty<Structure> m_structure;
    ExecLaterTimer m_reevalTimer;
};

#endif // SCENE_H
