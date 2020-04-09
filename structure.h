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

#include "note.h"
#include "scene.h"
#include "abstractshapeitem.h"

#include <QColor>
#include <QPointer>
#include <QJsonArray>
#include <QJsonObject>
#include <QUndoCommand>

class Structure;
class Character;
class ScriteDocument;
class StructurePositionCommand;

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

    Q_PROPERTY(qreal width READ width WRITE setWidth NOTIFY widthChanged STORED false)
    void setWidth(qreal val);
    qreal width() const { return m_width; }
    Q_SIGNAL void widthChanged();

    Q_PROPERTY(qreal height READ height WRITE setHeight NOTIFY heightChanged STORED false)
    void setHeight(qreal val);
    qreal height() const { return m_height; }
    Q_SIGNAL void heightChanged();

    Q_PROPERTY(qreal xf READ xf WRITE setXf NOTIFY xChanged)
    void setXf(qreal val);
    qreal xf() const;

    Q_PROPERTY(qreal yf READ yf WRITE setYf NOTIFY yChanged)
    void setYf(qreal val);
    qreal yf() const;

    Q_PROPERTY(QPointF position READ position WRITE setPosition STORED false)
    void setPosition(const QPointF &pos);
    QPointF position() const { return QPointF(m_x,m_y); }

    Q_SIGNAL void elementChanged();

protected:
    bool event(QEvent *event);

private:
    friend class StructurePositionCommand;
    qreal m_x;
    qreal m_y;
    bool m_placed;
    qreal m_width;
    qreal m_height;
    Scene* m_scene;
    Structure *m_structure;
    QBasicTimer m_undoCmdTimer;
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

    Q_PROPERTY(qreal canvasGridSize READ canvasGridSize WRITE setCanvasGridSize NOTIFY canvasGridSizeChanged)
    void setCanvasGridSize(qreal val);
    qreal canvasGridSize() const { return m_canvasGridSize; }
    Q_SIGNAL void canvasGridSizeChanged();

    Q_INVOKABLE qreal snapToGrid(qreal val) const;
    static qreal snapToGrid(qreal val, const Structure *structure, qreal defaultGridSize=10.0);

    void captureStructureAsImage(const QString &fileName);
    Q_SIGNAL void captureStructureAsImageRequest(const QString &fileName);

    Q_PROPERTY(ScriteDocument* scriteDocument READ scriteDocument CONSTANT STORED false)
    ScriteDocument* scriteDocument() const { return m_scriteDocument; }

    Q_PROPERTY(QQmlListProperty<Character> characters READ characters)
    QQmlListProperty<Character> characters();
    Q_INVOKABLE void addCharacter(Character *ptr);
    Q_INVOKABLE void removeCharacter(Character *ptr);
    Q_INVOKABLE Character *characterAt(int index) const;
    Q_PROPERTY(int characterCount READ characterCount NOTIFY characterCountChanged)
    int characterCount() const { return m_characters.size(); }
    Q_INVOKABLE void clearCharacters();
    Q_SIGNAL void characterCountChanged();

    Q_INVOKABLE QStringList allCharacterNames() const { return m_characterNames; }
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
    Q_INVOKABLE void insertElement(StructureElement *ptr, int index);
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

    Q_SIGNAL void structureChanged();

protected:
    bool event(QEvent *event);

private:
    ScriteDocument *m_scriteDocument;
    qreal m_canvasWidth;
    qreal m_canvasHeight;
    qreal m_canvasGridSize;

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

    enum LineType { StraightLine, CurvedLine };
    Q_ENUM(LineType)
    Q_PROPERTY(LineType lineType READ lineType WRITE setLineType NOTIFY lineTypeChanged)
    void setLineType(LineType val);
    LineType lineType() const { return m_lineType; }
    Q_SIGNAL void lineTypeChanged();

    Q_PROPERTY(StructureElement* fromElement READ fromElement WRITE setFromElement NOTIFY fromElementChanged)
    void setFromElement(StructureElement* val);
    StructureElement* fromElement() const { return m_fromElement; }
    Q_SIGNAL void fromElementChanged();

    Q_PROPERTY(StructureElement* toElement READ toElement WRITE setToElement NOTIFY toElementChanged)
    void setToElement(StructureElement* val);
    StructureElement* toElement() const { return m_toElement; }
    Q_SIGNAL void toElementChanged();

    Q_PROPERTY(qreal arrowAndLabelSpacing READ arrowAndLabelSpacing WRITE setArrowAndLabelSpacing NOTIFY arrowAndLabelSpacingChanged)
    void setArrowAndLabelSpacing(qreal val);
    qreal arrowAndLabelSpacing() const { return m_arrowAndLabelSpacing; }
    Q_SIGNAL void arrowAndLabelSpacingChanged();

    Q_PROPERTY(QPointF arrowPosition READ arrowPosition NOTIFY arrowPositionChanged)
    QPointF arrowPosition() const { return m_arrowPosition; }
    Q_SIGNAL void arrowPositionChanged();

    Q_PROPERTY(QPointF suggestedLabelPosition READ suggestedLabelPosition NOTIFY suggestedLabelPositionChanged)
    QPointF suggestedLabelPosition() const { return m_suggestedLabelPosition; }
    Q_SIGNAL void suggestedLabelPositionChanged();

    QPainterPath shape() const;

protected:
    void timerEvent(QTimerEvent *te);

private:
    void requestUpdateLater();
    void requestUpdate() { this->update(); }
    void onElementDestroyed(StructureElement *element);
    void pickElementColor();
    void updateArrowAndLabelPositions();
    void setArrowPosition(const QPointF &val);
    void setSuggestedLabelPosition(const QPointF &val);

private:
    LineType m_lineType;
    QBasicTimer m_updateTimer;
    StructureElement* m_toElement;
    StructureElement* m_fromElement;
    qreal m_arrowAndLabelSpacing;
    QPointF m_arrowPosition;
    QPointF m_suggestedLabelPosition;
};

#endif // STRUCTURE_H
