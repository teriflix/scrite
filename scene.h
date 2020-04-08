/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
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
#include <QColor>
#include <QJsonArray>
#include <QQmlListProperty>
#include <QAbstractListModel>
#include <QQuickTextDocument>

#include "note.h"

class Scene;
class SceneHeading;
class SceneElement;
class SceneDocumentBinder;

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

    enum LocationType { NoLocationType=-1, Interior, Exterior, Both };
    Q_ENUMS(LocationType)
    Q_PROPERTY(LocationType locationType READ locationType WRITE setLocationType NOTIFY locationTypeChanged)
    void setLocationType(LocationType val);
    LocationType locationType() const { return m_locationType; }
    Q_SIGNAL void locationTypeChanged();

    Q_PROPERTY(QString locationTypeAsString READ locationTypeAsString NOTIFY locationTypeChanged)
    QString locationTypeAsString() const;

    static QMap<LocationType,QString> locationTypeStringMap();

    Q_PROPERTY(QString location READ location WRITE setLocation NOTIFY locationChanged)
    void setLocation(const QString &val);
    QString location() const { return m_location; }
    Q_SIGNAL void locationChanged();

    enum Moment
    {
        NoMoment=-1,
        Day, Night,
        Morning, Afternoon, Evening,
        Later, MomentsLater,
        Continuous, TheNextDay,
        Earlier, MomentsEarlier,
        ThePreviousDay
    };
    Q_ENUM(Moment)
    Q_PROPERTY(Moment moment READ moment WRITE setMoment NOTIFY momentChanged)
    void setMoment(Moment val);
    Moment moment() const { return m_moment; }
    Q_SIGNAL void momentChanged();

    Q_PROPERTY(QString momentAsString READ momentAsString NOTIFY momentChanged)
    QString momentAsString() const;

    static QMap<Moment,QString> momentStringMap();

    QString toString() const;

private:
    Scene* m_scene;
    bool m_enabled;
    Moment m_moment;
    QString m_location;
    LocationType m_locationType;
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
    void setType(Type val);
    Type type() const { return m_type; }
    Q_SIGNAL void typeChanged();

    Q_PROPERTY(QString typeAsString READ typeAsString NOTIFY typeChanged)
    QString typeAsString() const;

    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    void setText(const QString &val);
    QString text() const;
    Q_SIGNAL void textChanged();

    Q_SIGNAL void elementChanged();

    Q_INVOKABLE QJsonArray find(const QString &text, int flags) const;

protected:
    bool event(QEvent *event);

private:
    Type m_type;
    QString m_text;
    Scene* m_scene;
};

class Scene : public QAbstractListModel
{
    Q_OBJECT

public:
    Q_INVOKABLE Scene(QObject *parent=nullptr);
    ~Scene();
    Q_SIGNAL void aboutToDelete(Scene *scene);

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

    Q_PROPERTY(SceneHeading* heading READ heading NOTIFY headingChanged)
    SceneHeading* heading() const { return m_heading; }
    Q_SIGNAL void headingChanged();

    Q_PROPERTY(QQmlListProperty<SceneElement> elements READ elements)
    QQmlListProperty<SceneElement> elements();
    Q_INVOKABLE void addElement(SceneElement *ptr);
    Q_INVOKABLE void insertAfter(SceneElement *ptr, SceneElement *after);
    Q_INVOKABLE void insertBefore(SceneElement *ptr, SceneElement *before);
    Q_INVOKABLE void insertAt(SceneElement *ptr, int index);
    Q_INVOKABLE void removeElement(SceneElement *ptr);
    Q_INVOKABLE int  indexOfElement(SceneElement *ptr) { return m_elements.indexOf(ptr); }
    Q_INVOKABLE SceneElement *elementAt(int index) const;
    Q_PROPERTY(int elementCount READ elementCount NOTIFY elementCountChanged)
    int elementCount() const;
    Q_INVOKABLE void clearElements();
    Q_SIGNAL void elementCountChanged();

    enum SceneElementChangeType { ElementTypeChange, ElementTextChange };
    Q_SIGNAL void sceneElementChanged(SceneElement *element, SceneElementChangeType type);
    Q_SIGNAL void aboutToRemoveSceneElement(SceneElement *element);
    Q_SIGNAL void sceneChanged();

    Q_PROPERTY(QQmlListProperty<Note> notes READ notes)
    QQmlListProperty<Note> notes();
    Q_INVOKABLE void addNote(Note *ptr);
    Q_INVOKABLE void removeNote(Note *ptr);
    Q_INVOKABLE Note *noteAt(int index) const;
    Q_PROPERTY(int noteCount READ noteCount NOTIFY noteCountChanged)
    int noteCount() const { return m_notes.size(); }
    Q_INVOKABLE void clearNotes();
    Q_SIGNAL void noteCountChanged();

    // QAbstractItemModel interface
    enum Roles { SceneElementRole = Qt::UserRole };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int,QByteArray> roleNames() const;

private:
    QList<SceneElement *> elementsList() const { return m_elements; }
    void setElementsList(const QList<SceneElement*> &list);

private:
    friend class SceneElement;
    friend class SceneDocumentBinder;

    mutable QString m_id;
    QString m_title;
    QColor m_color;
    bool m_enabled;
    char m_padding[7];
    SceneHeading* m_heading;
    QQuickTextDocument* m_textDocument;

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
