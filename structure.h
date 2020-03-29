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

#ifndef STRUCTURE_H
#define STRUCTURE_H

#include "scene.h"
#include "abstractshapeitem.h"

#include <QColor>
#include <QJsonArray>
#include <QJsonObject>

class Structure;
class Character;
class ScriteDocument;

class StructureElement : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE StructureElement(QObject *parent=nullptr);
    ~StructureElement();
    Q_SIGNAL void aboutToDelete(StructureElement *element);

    Q_PROPERTY(Structure* structure READ structure CONSTANT STORED false)
    Structure* structure() const { return m_structure; }

    Q_PROPERTY(Scene* scene READ scene WRITE setScene NOTIFY sceneChanged)
    void setScene(Scene* val);
    Scene* scene() const { return m_scene; }
    Q_SIGNAL void sceneChanged();

    Q_PROPERTY(qreal x READ x WRITE setX NOTIFY xChanged STORED false)
    void setX(qreal val);
    qreal x() const { return m_x; }
    Q_SIGNAL void xChanged();

    Q_PROPERTY(qreal y READ y WRITE setY NOTIFY yChanged STORED false)
    void setY(qreal val);
    qreal y() const { return m_y; }
    Q_SIGNAL void yChanged();

    Q_PROPERTY(qreal xf READ xf WRITE setXf NOTIFY xChanged)
    void setXf(qreal val);
    qreal xf() const;

    Q_PROPERTY(qreal yf READ yf WRITE setYf NOTIFY yChanged)
    void setYf(qreal val);
    qreal yf() const;

    Q_SIGNAL void elementChanged();

protected:
    bool event(QEvent *event);

private:
    qreal m_x;
    qreal m_y;
    Scene* m_scene;
    Structure *m_structure;
};

class StructureArea : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE StructureArea(QObject *parent=nullptr);
    ~StructureArea();
    Q_SIGNAL void aboutToDelete(StructureArea *area);

    Q_PROPERTY(Structure* structure READ structure CONSTANT STORED false)
    Structure* structure() const { return m_structure; }

    Q_PROPERTY(qreal x READ x WRITE setX NOTIFY xChanged)
    void setX(qreal val);
    qreal x() const { return m_x; }
    Q_SIGNAL void xChanged();

    Q_PROPERTY(qreal y READ y WRITE setY NOTIFY yChanged)
    void setY(qreal val);
    qreal y() const { return m_y; }
    Q_SIGNAL void yChanged();

    Q_PROPERTY(qreal width READ width WRITE setWidth NOTIFY widthChanged)
    void setWidth(qreal val);
    qreal width() const { return m_width; }
    Q_SIGNAL void widthChanged();

    Q_PROPERTY(qreal height READ height WRITE setHeight NOTIFY heightChanged)
    void setHeight(qreal val);
    qreal height() const { return m_height; }
    Q_SIGNAL void heightChanged();

    Q_SIGNAL void areaChanged();

protected:
    bool event(QEvent *event);

private:
    qreal m_x;
    qreal m_y;
    qreal m_width;
    qreal m_height;
    Structure *m_structure;
};

class Note : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE Note(QObject *parent=nullptr);
    ~Note();
    Q_SIGNAL void aboutToDelete(Note *ptr);

    Q_PROPERTY(Structure* structure READ structure CONSTANT STORED false)
    Structure *structure() const { return m_structure; }

    Q_PROPERTY(Character* character READ character CONSTANT STORED false)
    Character *character() const { return m_character; }

    Q_PROPERTY(QString heading READ heading WRITE setHeading NOTIFY headingChanged)
    void setHeading(const QString &val);
    QString heading() const { return m_heading; }
    Q_SIGNAL void headingChanged();

    Q_PROPERTY(QString content READ content WRITE setContent NOTIFY contentChanged)
    void setContent(const QString &val);
    QString content() const { return m_content; }
    Q_SIGNAL void contentChanged();

    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    void setColor(const QColor &val);
    QColor color() const { return m_color; }
    Q_SIGNAL void colorChanged();

    Q_SIGNAL void noteChanged();

protected:
    bool event(QEvent *event);

private:
    QColor m_color;
    QString m_heading;
    QString m_content;
    Structure *m_structure;
    Character *m_character;
};

class Character : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE Character(QObject *parent=nullptr);
    ~Character();
    Q_SIGNAL void aboutToDelete(Character *ptr);

    Q_PROPERTY(Structure* structure READ structure CONSTANT STORED false)
    Structure* structure() const { return m_structure; }

    Q_PROPERTY(bool valid READ isValid NOTIFY nameChanged)
    bool isValid() const { return !m_name.isEmpty(); }

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    void setName(const QString &val);
    QString name() const { return m_name; }
    Q_SIGNAL void nameChanged();

    Q_PROPERTY(QQmlListProperty<Note> notes READ notes)
    QQmlListProperty<Note> notes();
    Q_INVOKABLE void addNote(Note *ptr);
    Q_INVOKABLE void removeNote(Note *ptr);
    Q_INVOKABLE Note *noteAt(int index) const;
    Q_PROPERTY(int noteCount READ noteCount NOTIFY noteCountChanged)
    int noteCount() const { return m_notes.size(); }
    Q_INVOKABLE void clearNotes();
    Q_SIGNAL void noteCountChanged();

    Q_SIGNAL void characterChanged();

protected:
    bool event(QEvent *event);

private:
    static void staticAppendNote(QQmlListProperty<Note> *list, Note *ptr);
    static void staticClearNotes(QQmlListProperty<Note> *list);
    static Note* staticNoteAt(QQmlListProperty<Note> *list, int index);
    static int staticNoteCount(QQmlListProperty<Note> *list);

private:
    QString m_name;
    QList<Note *> m_notes;
    Structure* m_structure;
};

class Structure : public QObject
{
    Q_OBJECT

public:
    Structure(QObject *parent=nullptr);
    ~Structure();

    Q_PROPERTY(qreal canvasWidth READ canvasWidth WRITE setCanvasWidth NOTIFY canvasWidthChanged)
    void setCanvasWidth(qreal val);
    qreal canvasWidth() const { return m_canvasWidth; }
    Q_SIGNAL void canvasWidthChanged();

    Q_PROPERTY(qreal canvasHeight READ canvasHeight WRITE setCanvasHeight NOTIFY canvasHeightChanged)
    void setCanvasHeight(qreal val);
    qreal canvasHeight() const { return m_canvasHeight; }
    Q_SIGNAL void canvasHeightChanged();

    Q_PROPERTY(ScriteDocument* scriteDocument READ scriteDocument CONSTANT STORED false)
    ScriteDocument* scriteDocument() const { return m_scriteDocument; }

    Q_PROPERTY(QQmlListProperty<StructureArea> areas READ areas)
    QQmlListProperty<StructureArea> areas();
    Q_INVOKABLE void addArea(StructureArea *ptr);
    Q_INVOKABLE void removeArea(StructureArea *ptr);
    Q_INVOKABLE StructureArea *areaAt(int index) const;
    Q_PROPERTY(int areaCount READ areaCount NOTIFY areaCountChanged)
    int areaCount() const { return m_areas.size(); }
    Q_INVOKABLE void clearAreas();
    Q_SIGNAL void areaCountChanged();

    Q_PROPERTY(QQmlListProperty<Character> characters READ characters)
    QQmlListProperty<Character> characters();
    Q_INVOKABLE void addCharacter(Character *ptr);
    Q_INVOKABLE void removeCharacter(Character *ptr);
    Q_INVOKABLE Character *characterAt(int index) const;
    Q_PROPERTY(int characterCount READ characterCount NOTIFY characterCountChanged)
    int characterCount() const { return m_characters.size(); }
    Q_INVOKABLE void clearCharacters();
    Q_SIGNAL void characterCountChanged();

    Q_INVOKABLE QStringList allCharacterNames() const;
    Q_INVOKABLE QJsonArray detectCharacters() const;
    Q_INVOKABLE void addCharacters(const QStringList &names);

    Q_INVOKABLE Character *findCharacter(const QString &name) const;

    Q_PROPERTY(QQmlListProperty<Note> notes READ notes)
    QQmlListProperty<Note> notes();
    Q_INVOKABLE void addNote(Note *ptr);
    Q_INVOKABLE void removeNote(Note *ptr);
    Q_INVOKABLE Note *noteAt(int index) const;
    Q_PROPERTY(int noteCount READ noteCount NOTIFY noteCountChanged)
    int noteCount() const { return m_notes.size(); }
    Q_INVOKABLE void clearNotes();
    Q_SIGNAL void noteCountChanged();

    // NOTE: Elements has to be the last of QQmlListProperty in this class.
    Q_PROPERTY(QQmlListProperty<StructureElement> elements READ elements NOTIFY elementsChanged)
    QQmlListProperty<StructureElement> elements();
    Q_INVOKABLE void addElement(StructureElement *ptr);
    Q_INVOKABLE void removeElement(StructureElement *ptr);
    Q_INVOKABLE void moveElement(StructureElement *ptr, int toRow);
    Q_INVOKABLE StructureElement *elementAt(int index) const;
    Q_PROPERTY(int elementCount READ elementCount NOTIFY elementCountChanged)
    int elementCount() const { return m_elements.size(); }
    Q_INVOKABLE void clearElements();
    Q_SIGNAL void elementCountChanged();
    Q_SIGNAL void elementsChanged();

    Q_INVOKABLE int indexOfScene(Scene *scene) const;
    Q_INVOKABLE int indexOfElement(StructureElement *element) const;
    Q_INVOKABLE StructureElement *findElementBySceneID(const QString &id) const;

    Q_PROPERTY(int currentElementIndex READ currentElementIndex WRITE setCurrentElementIndex NOTIFY currentElementIndexChanged STORED false)
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

    Q_INVOKABLE QJsonObject evaluateCurve(const QPointF &p1, const QPointF &p2) const;

    Q_SIGNAL void structureChanged();

protected:
    bool event(QEvent *event);

private:
    ScriteDocument *m_scriteDocument;
    qreal m_canvasWidth;
    qreal m_canvasHeight;

    static void staticAppendArea(QQmlListProperty<StructureArea> *list, StructureArea *ptr);
    static void staticClearAreas(QQmlListProperty<StructureArea> *list);
    static StructureArea* staticAreaAt(QQmlListProperty<StructureArea> *list, int index);
    static int staticAreaCount(QQmlListProperty<StructureArea> *list);
    QList<StructureArea *> m_areas;

    static void staticAppendCharacter(QQmlListProperty<Character> *list, Character *ptr);
    static void staticClearCharacters(QQmlListProperty<Character> *list);
    static Character* staticCharacterAt(QQmlListProperty<Character> *list, int index);
    static int staticCharacterCount(QQmlListProperty<Character> *list);
    QList<Character *> m_characters;

    static void staticAppendNote(QQmlListProperty<Note> *list, Note *ptr);
    static void staticClearNotes(QQmlListProperty<Note> *list);
    static Note* staticNoteAt(QQmlListProperty<Note> *list, int index);
    static int staticNoteCount(QQmlListProperty<Note> *list);
    QList<Note *> m_notes;

    static void staticAppendElement(QQmlListProperty<StructureElement> *list, StructureElement *ptr);
    static void staticClearElements(QQmlListProperty<StructureElement> *list);
    static StructureElement* staticElementAt(QQmlListProperty<StructureElement> *list, int index);
    static int staticElementCount(QQmlListProperty<StructureElement> *list);
    QList<StructureElement *> m_elements;
    int m_currentElementIndex;
    qreal m_zoomLevel;

    void onStructureElementSceneChanged(StructureElement *element=nullptr);
    void onSceneElementChanged(SceneElement *element, Scene::SceneElementChangeType type);
    void onAboutToRemoveSceneElement(SceneElement *element);
    void evaluateCharacterNames();
    QMap<SceneElement*,QString> m_characterElementNameMap;
    QStringList m_characterNames;
};

///////////////////////////////////////////////////////////////////////////////

class StructureElementConnector : public AbstractShapeItem
{
    Q_OBJECT

public:
    StructureElementConnector(QQuickItem *parent=nullptr);
    ~StructureElementConnector();

    Q_PROPERTY(StructureElement* fromElement READ fromElement WRITE setFromElement NOTIFY fromElementChanged)
    void setFromElement(StructureElement* val);
    StructureElement* fromElement() const { return m_fromElement; }
    Q_SIGNAL void fromElementChanged();

    Q_PROPERTY(StructureElement* toElement READ toElement WRITE setToElement NOTIFY toElementChanged)
    void setToElement(StructureElement* val);
    StructureElement* toElement() const { return m_toElement; }
    Q_SIGNAL void toElementChanged();

    QPainterPath shape() const;

private:
    void requestUpdate() { this->update(); }
    void onElementDestroyed(StructureElement *element);

private:
    StructureElement* m_toElement;
    StructureElement* m_fromElement;
};

#endif // STRUCTURE_H
