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

#ifndef SCENE_H
#define SCENE_H

#include <QMap>
#include <QList>
#include <QColor>
#include <QPointer>
#include <QJsonArray>
#include <QUndoCommand>
#include <QQmlListProperty>
#include <QAbstractListModel>
#include <QQuickTextDocument>

#include "note.h"
#include "qobjectserializer.h"

class Scene;
class SceneHeading;
class SceneElement;
class SceneDocumentBinder;
class PushSceneUndoCommand;

class SceneHeading : public QObject
{
    Q_OBJECT

public:
    SceneHeading(QObject *parent=nullptr);
    ~SceneHeading();

    Q_PROPERTY(Scene* scene READ scene CONSTANT STORED false)
    Scene* scene() const { return m_scene; }

    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    void setEnabled(bool val);
    bool isEnabled() const { return m_enabled; }
    Q_SIGNAL void enabledChanged();

    Q_PROPERTY(QString locationType READ locationType WRITE setLocationType NOTIFY locationTypeChanged)
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

    Q_PROPERTY(QString text READ text NOTIFY textChanged)
    QString text() const;
    Q_SIGNAL void textChanged();

    Q_INVOKABLE void parseFrom(const QString &text);

private:
    bool m_enabled = true;
    char m_padding[7];
    Scene* m_scene = nullptr;
    QString m_moment = "DAY";
    QString m_location = "Somewhere";
    QString m_locationType = "EXT";
};

class SceneElement : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE SceneElement(QObject *parent=nullptr);
    ~SceneElement();
    Q_SIGNAL void aboutToDelete(SceneElement *element);

    Q_PROPERTY(Scene* scene READ scene CONSTANT STORED false)
    Scene* scene() const { return m_scene; }

    enum Type { Action, Character, Dialogue, Parenthetical, Shot, Transition, Heading, Min=Action, Max=Heading };
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
    Q_SIGNAL void textChanged();

    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY cursorPositionChanged STORED false)
    void setCursorPosition(int val);
    int cursorPosition() const;
    Q_SIGNAL void cursorPositionChanged();

    QString formattedText() const;

    Q_SIGNAL void elementChanged();

    Q_INVOKABLE QJsonArray find(const QString &text, int flags) const;

protected:
    bool event(QEvent *event);

private:
    Type m_type = Action;
    QString m_text;
    Scene* m_scene = nullptr;
};

class CharacterElementMap
{
public:
    CharacterElementMap();
    ~CharacterElementMap();

    // These functions returns true if characterNames() would return
    // a different list after this function returns
    bool include(SceneElement *element);
    bool remove(SceneElement *element);
    bool remove(const QString &name);

    QStringList characterNames() const;
    QList<SceneElement*> characterElements() const;
    QList<SceneElement*> characterElements(const QString &name) const;

    void include(const CharacterElementMap &other);

private:
    QMap<SceneElement*,QString> m_forwardMap;
    QMap< QString, QList<SceneElement*> > m_reverseMap;
};

class Scene : public QAbstractListModel, public QObjectSerializer::Interface
{
    Q_OBJECT
    Q_INTERFACES(QObjectSerializer::Interface)

public:
    Q_INVOKABLE Scene(QObject *parent=nullptr);
    ~Scene();
    Q_SIGNAL void aboutToDelete(Scene *scene);

    Scene *clone(QObject *parent) const;

    /*
     * The 'id' is a special property. It can be set only once. If it is not
     * set an ID is automatically generated whenever the property value is
     * queried for the first time.
    */
    Q_PROPERTY(QString id READ id WRITE setId NOTIFY idChanged)
    void setId(const QString &val);
    QString id() const;
    Q_SIGNAL void idChanged();

    Q_PROPERTY(QString name READ name NOTIFY titleChanged)
    QString name() const;

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    void setTitle(const QString &val);
    QString title() const { return m_title; }
    Q_SIGNAL void titleChanged();

    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    void setColor(const QColor &val);
    QColor color() const { return m_color; }
    Q_SIGNAL void colorChanged();

    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    void setEnabled(bool val);
    bool isEnabled() const { return m_enabled; }
    Q_SIGNAL void enabledChanged();

    Q_PROPERTY(bool isBeingReset READ isBeingReset NOTIFY resetStateChanged)
    bool isBeingReset() const { return m_isBeingReset; }
    Q_SIGNAL void resetStateChanged();

    Q_PROPERTY(bool undoRedoEnabled READ isUndoRedoEnabled WRITE setUndoRedoEnabled NOTIFY undoRedoEnabledChanged STORED false)
    void setUndoRedoEnabled(bool val);
    bool isUndoRedoEnabled() const { return m_undoRedoEnabled; }
    Q_SIGNAL void undoRedoEnabledChanged();

    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY cursorPositionChanged STORED false)
    void setCursorPosition(int val);
    int cursorPosition() const { return m_cursorPosition; }
    Q_SIGNAL void cursorPositionChanged();

    Q_PROPERTY(SceneHeading* heading READ heading NOTIFY headingChanged)
    SceneHeading* heading() const { return m_heading; }
    Q_SIGNAL void headingChanged();

    Q_PROPERTY(QStringList characterNames READ characterNames NOTIFY characterNamesChanged)
    QStringList characterNames() const { return m_characterElementMap.characterNames(); }
    Q_SIGNAL void characterNamesChanged();

    Q_INVOKABLE void addMuteCharacter(const QString &characterName);
    Q_INVOKABLE void removeMuteCharacter(const QString &characterName);
    Q_INVOKABLE bool isCharacterMute(const QString &characterName) const;
    void scanMuteCharacters(const QStringList &characterNames=QStringList());

    Q_PROPERTY(QQmlListProperty<SceneElement> elements READ elements)
    QQmlListProperty<SceneElement> elements();
    Q_INVOKABLE void addElement(SceneElement *ptr);
    Q_INVOKABLE void insertElementAfter(SceneElement *ptr, SceneElement *after);
    Q_INVOKABLE void insertElementBefore(SceneElement *ptr, SceneElement *before);
    Q_INVOKABLE void insertElementAt(SceneElement *ptr, int index);
    Q_INVOKABLE void removeElement(SceneElement *ptr);
    Q_INVOKABLE int  indexOfElement(SceneElement *ptr) { return m_elements.indexOf(ptr); }
    Q_INVOKABLE SceneElement *elementAt(int index) const;
    Q_PROPERTY(int elementCount READ elementCount NOTIFY elementCountChanged)
    int elementCount() const;
    Q_INVOKABLE void clearElements();
    Q_SIGNAL void elementCountChanged();

    Q_INVOKABLE void removeLastElementIfEmpty();

    enum SceneElementChangeType { ElementTypeChange, ElementTextChange };
    Q_SIGNAL void sceneElementChanged(SceneElement *element, SceneElementChangeType type);
    Q_SIGNAL void aboutToRemoveSceneElement(SceneElement *element);
    Q_SIGNAL void sceneChanged();
    Q_SIGNAL void sceneRefreshed();
    Q_SIGNAL void sceneAboutToReset();
    Q_SIGNAL void sceneReset(int elementIndex);

    Q_PROPERTY(QQmlListProperty<Note> notes READ notes)
    QQmlListProperty<Note> notes();
    Q_INVOKABLE void addNote(Note *ptr);
    Q_INVOKABLE void removeNote(Note *ptr);
    Q_INVOKABLE Note *noteAt(int index) const;
    Q_PROPERTY(int noteCount READ noteCount NOTIFY noteCountChanged)
    int noteCount() const { return m_notes.size(); }
    Q_INVOKABLE void clearNotes();
    Q_SIGNAL void noteCountChanged();

    Q_INVOKABLE void beginUndoCapture(bool allowMerging=true);
    Q_INVOKABLE void endUndoCapture();

    Scene *splitScene(SceneElement *element, int textPosition, QObject *parent=nullptr);

    // QAbstractItemModel interface
    enum Roles { SceneElementRole = Qt::UserRole };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int,QByteArray> roleNames() const;

    // Serializing functions for use with Undo/Redo
    QByteArray toByteArray() const;
    bool resetFromByteArray(const QByteArray &bytes);
    static Scene *fromByteArray(const QByteArray &bytes);

    // QObjectSerializer::Interface interface
    void serializeToJson(QJsonObject &json) const;
    void deserializeFromJson(const QJsonObject &json);

private:
    QList<SceneElement *> elementsList() const { return m_elements; }
    void setElementsList(const QList<SceneElement*> &list);
    void onSceneElementChanged(SceneElement *element, SceneElementChangeType type);
    void onAboutToRemoveSceneElement(SceneElement *element);
    const CharacterElementMap & characterElementMap() const { return m_characterElementMap; }

private:
    friend class Structure;
    friend class SceneElement;
    friend class SceneDocumentBinder;

    QColor m_color = QColor(Qt::white);
    QString m_title;
    bool m_enabled = true;
    char m_padding[7];
    mutable QString m_id;
    int m_cursorPosition = -1;
    SceneHeading* m_heading = new SceneHeading(this);
    bool m_isBeingReset = false;
    bool m_undoRedoEnabled = false;
    bool m_inSetElementsList = false;
    PushSceneUndoCommand *m_pushUndoCommand = nullptr;
    CharacterElementMap m_characterElementMap;

    static void staticAppendElement(QQmlListProperty<SceneElement> *list, SceneElement *ptr);
    static void staticClearElements(QQmlListProperty<SceneElement> *list);
    static SceneElement* staticElementAt(QQmlListProperty<SceneElement> *list, int index);
    static int staticElementCount(QQmlListProperty<SceneElement> *list);
    QList<SceneElement *> m_elements;

    static void staticAppendNote(QQmlListProperty<Note> *list, Note *ptr);
    static void staticClearNotes(QQmlListProperty<Note> *list);
    static Note* staticNoteAt(QQmlListProperty<Note> *list, int index);
    static int staticNoteCount(QQmlListProperty<Note> *list);
    QList<Note *> m_notes;
};

#endif // SCENE_H
