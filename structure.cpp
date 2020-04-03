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

#include "structure.h"
#include "scritedocument.h"

StructureElement::StructureElement(QObject *parent)
    : QObject(parent),
      m_x(0),
      m_y(0),
      m_width(0),
      m_height(0),
      m_scene(nullptr),
      m_structure(qobject_cast<Structure*>(parent))
{
    connect(this, &StructureElement::xChanged, this, &StructureElement::elementChanged);
    connect(this, &StructureElement::yChanged, this, &StructureElement::elementChanged);
    connect(this, &StructureElement::widthChanged, this, &StructureElement::elementChanged);
    connect(this, &StructureElement::heightChanged, this, &StructureElement::elementChanged);
}

StructureElement::~StructureElement()
{
    emit aboutToDelete(this);
}

void StructureElement::setScene(Scene *val)
{
    if(m_scene == val || m_scene != nullptr || val == nullptr)
        return;

    m_scene = val;
    m_scene->setParent(this);
    connect(m_scene, &Scene::sceneChanged, this, &StructureElement::elementChanged);
    connect(m_scene, &Scene::aboutToDelete, this, &StructureElement::deleteLater);
    connect(m_scene, &Scene::aboutToDelete, this, &StructureElement::deleteLater);

    emit sceneChanged();
}

void StructureElement::setX(qreal val)
{
    if( qFuzzyCompare(m_x, val) )
        return;

    m_x = val;
    emit xChanged();
}

void StructureElement::setY(qreal val)
{
    if( qFuzzyCompare(m_y, val) )
        return;

    m_y = val;
    emit yChanged();
}

void StructureElement::setWidth(qreal val)
{
    if( qFuzzyCompare(m_width, val) )
        return;

    m_width = val;
    emit widthChanged();
}

void StructureElement::setHeight(qreal val)
{
    if( qFuzzyCompare(m_height, val) )
        return;

    m_height = val;
    emit heightChanged();
}

void StructureElement::setXf(qreal val)
{
    if(m_structure == nullptr)
        return;

    val = qBound(0.0, val, 1.0);
    this->setX( m_structure->canvasWidth()*val );
}

qreal StructureElement::xf() const
{
    return m_structure==nullptr ? 0 : m_x / m_structure->canvasWidth();
}

void StructureElement::setYf(qreal val)
{
    if(m_structure == nullptr)
        return;

    val = qBound(0.0, val, 1.0);
    this->setY( m_structure->canvasHeight()*val );
}

qreal StructureElement::yf() const
{
    return m_structure==nullptr ? 0 : m_y / m_structure->canvasHeight();
}

bool StructureElement::event(QEvent *event)
{
    if(event->type() == QEvent::ParentChange)
    {
        m_structure = qobject_cast<Structure*>(this->parent());
        emit xChanged();
        emit yChanged();
    }

    return QObject::event(event);
}

///////////////////////////////////////////////////////////////////////////////

StructureArea::StructureArea(QObject *parent)
    : QObject(parent),
      m_x(0),
      m_y(0),
      m_width(0),
      m_height(0),
      m_structure(qobject_cast<Structure*>(parent))
{
    connect(this, &StructureArea::xChanged, &StructureArea::areaChanged);
    connect(this, &StructureArea::yChanged, &StructureArea::areaChanged);
    connect(this, &StructureArea::widthChanged, &StructureArea::areaChanged);
    connect(this, &StructureArea::heightChanged, &StructureArea::areaChanged);
}

StructureArea::~StructureArea()
{
    emit aboutToDelete(this);
}

void StructureArea::setX(qreal val)
{
    if( qFuzzyCompare(m_x, val) )
        return;
    m_x = val;
    emit xChanged();
}

void StructureArea::setY(qreal val)
{
    if( qFuzzyCompare(m_y, val) )
        return;

    m_y = val;
    emit yChanged();
}

void StructureArea::setWidth(qreal val)
{
    if( qFuzzyCompare(m_width, val) )
        return;

    m_width = val;
    emit widthChanged();
}

void StructureArea::setHeight(qreal val)
{
    if( qFuzzyCompare(m_height, val) )
        return;

    m_height = val;
    emit heightChanged();
}

bool StructureArea::event(QEvent *event)
{
    if(event->type() == QEvent::ParentChange)
        m_structure = qobject_cast<Structure*>(this->parent());

    return QObject::event(event);
}

///////////////////////////////////////////////////////////////////////////////

Character::Character(QObject *parent)
    : QObject(parent),
      m_structure(qobject_cast<Structure*>(parent))
{
    connect(this, &Character::nameChanged, this, &Character::characterChanged);
    connect(this, &Character::noteCountChanged, this, &Character::characterChanged);
}

Character::~Character()
{

}

void Character::setName(const QString &val)
{
    if(m_name == val || val.isEmpty() || !m_name.isEmpty())
        return;

    m_name = val.toUpper().trimmed();
    emit nameChanged();
}

QQmlListProperty<Note> Character::notes()
{
    return QQmlListProperty<Note>(
                reinterpret_cast<QObject*>(this),
                static_cast<void*>(this),
                &Character::staticAppendNote,
                &Character::staticNoteCount,
                &Character::staticNoteAt,
                &Character::staticClearNotes);
}

void Character::addNote(Note *ptr)
{
    if(ptr == nullptr || m_notes.indexOf(ptr) >= 0)
        return;

    connect(ptr, &Note::aboutToDelete, this, &Character::removeNote);
    connect(ptr, &Note::noteChanged, this, &Character::characterChanged);

    ptr->setParent(this);
    m_notes.append(ptr);
    emit noteCountChanged();
}

void Character::removeNote(Note *ptr)
{
    if(ptr == nullptr)
        return;

    const int index = m_notes.indexOf(ptr);
    if(index < 0)
        return ;

    m_notes.removeAt(index);

    disconnect(ptr, &Note::aboutToDelete, this, &Character::removeNote);
    disconnect(ptr, &Note::noteChanged, this, &Character::characterChanged);

    emit noteCountChanged();
}

Note *Character::noteAt(int index) const
{
    return index < 0 || index >= m_notes.size() ? nullptr : m_notes.at(index);
}

void Character::clearNotes()
{
    while(m_notes.size())
        this->removeNote(m_notes.first());
}

bool Character::event(QEvent *event)
{
    if(event->type() == QEvent::ParentChange)
        m_structure = qobject_cast<Structure*>(this->parent());

    return QObject::event(event);
}

void Character::staticAppendNote(QQmlListProperty<Note> *list, Note *ptr)
{
    reinterpret_cast< Character* >(list->data)->addNote(ptr);
}

void Character::staticClearNotes(QQmlListProperty<Note> *list)
{
    reinterpret_cast< Character* >(list->data)->clearNotes();
}

Note *Character::staticNoteAt(QQmlListProperty<Note> *list, int index)
{
    return reinterpret_cast< Character* >(list->data)->noteAt(index);
}

int Character::staticNoteCount(QQmlListProperty<Note> *list)
{
    return reinterpret_cast< Character* >(list->data)->noteCount();
}

///////////////////////////////////////////////////////////////////////////////

Structure::Structure(QObject *parent)
    : QObject(parent),
      m_scriteDocument(qobject_cast<ScriteDocument*>(parent)),
      m_canvasWidth(10000),
      m_canvasHeight(10000),
      m_currentElementIndex(-1),
      m_zoomLevel(1.0)
{
    connect(this, &Structure::noteCountChanged, this, &Structure::structureChanged);
}

Structure::~Structure()
{

}

void Structure::setCanvasWidth(qreal val)
{
    if( qFuzzyCompare(m_canvasWidth, val) )
        return;

    m_canvasWidth = val;
    emit canvasWidthChanged();
}

void Structure::setCanvasHeight(qreal val)
{
    if( qFuzzyCompare(m_canvasHeight, val) )
        return;

    m_canvasHeight = val;
    emit canvasHeightChanged();
}

QQmlListProperty<StructureArea> Structure::areas()
{
    return QQmlListProperty<StructureArea>(
                reinterpret_cast<QObject*>(this),
                static_cast<void*>(this),
                &Structure::staticAppendArea,
                &Structure::staticAreaCount,
                &Structure::staticAreaAt,
                &Structure::staticClearAreas);
}

void Structure::addArea(StructureArea *ptr)
{
    if(ptr == nullptr || m_areas.indexOf(ptr) >= 0)
        return;

    m_areas.append(ptr);
    ptr->setParent(this);
    connect(ptr, &StructureArea::areaChanged, this, &Structure::structureChanged);
    emit areaCountChanged();
}

void Structure::removeArea(StructureArea *ptr)
{
    if(ptr == nullptr)
        return;

    const int index = m_areas.indexOf(ptr);
    if(index < 0)
        return ;

    m_areas.removeAt(index);
    disconnect(ptr, &StructureArea::areaChanged, this, &Structure::structureChanged);
    emit areaCountChanged();

    if(ptr->parent() == this)
        ptr->deleteLater();
}

StructureArea *Structure::areaAt(int index) const
{
    return index < 0 || index >= m_areas.size() ? nullptr : m_areas.at(index);
}

void Structure::clearAreas()
{
    while(m_areas.size())
        this->removeArea(m_areas.first());
}

QQmlListProperty<Character> Structure::characters()
{
    return QQmlListProperty<Character>(
                reinterpret_cast<QObject*>(this),
                static_cast<void*>(this),
                &Structure::staticAppendCharacter,
                &Structure::staticCharacterCount,
                &Structure::staticCharacterAt,
                &Structure::staticClearCharacters);
}

void Structure::addCharacter(Character *ptr)
{
    if(ptr == nullptr || m_characters.indexOf(ptr) >= 0)
        return;

    Character *ch = ptr->isValid() ? this->findCharacter(ptr->name()) : nullptr;
    if(!ptr->isValid() || ch != nullptr)
    {
        if(ptr->parent() == this)
            ptr->deleteLater();
        return;
    }

    ptr->setParent(this);

    connect(ptr, &Character::aboutToDelete, this, &Structure::removeCharacter);
    connect(ptr, &Character::characterChanged, this, &Structure::structureChanged);

    m_characters.append(ptr);
    emit characterCountChanged();
}

void Structure::removeCharacter(Character *ptr)
{
    if(ptr == nullptr)
        return;

    const int index = m_characters.indexOf(ptr);
    if(index < 0)
        return ;

    m_characters.removeAt(index);

    disconnect(ptr, &Character::aboutToDelete, this, &Structure::removeCharacter);
    disconnect(ptr, &Character::characterChanged, this, &Structure::structureChanged);

    emit characterCountChanged();

    if(ptr->parent() == this)
        ptr->deleteLater();
}

Character *Structure::characterAt(int index) const
{
    return index < 0 || index >= m_characters.size() ? nullptr : m_characters.at(index);
}

void Structure::clearCharacters()
{
    while(m_characters.size())
        this->removeCharacter(m_characters.first());
}

QJsonArray Structure::detectCharacters() const
{
    QJsonArray ret;

    const QStringList names = this->allCharacterNames();

    Q_FOREACH(QString name, names)
    {
        Character *character = this->findCharacter(name);

        QJsonObject item;
        item.insert("name", name);
        item.insert("added", character != nullptr);
        ret.append(item);
    }

    return ret;
}

void Structure::addCharacters(const QStringList &names)
{
    Q_FOREACH(QString name, names)
    {
        Character *character = this->findCharacter(name);
        if(character == nullptr)
        {
            character = new Character(this);
            character->setName(name);
            this->addCharacter(character);
        }
    }
}

Character *Structure::findCharacter(const QString &name) const
{
    const QString name2 = name.toUpper();
    Q_FOREACH(Character *character, m_characters)
    {
        if(character->name() == name2)
            return character;
    }

    return nullptr;
}

QQmlListProperty<Note> Structure::notes()
{
    return QQmlListProperty<Note>(
                reinterpret_cast<QObject*>(this),
                static_cast<void*>(this),
                &Structure::staticAppendNote,
                &Structure::staticNoteCount,
                &Structure::staticNoteAt,
                &Structure::staticClearNotes);
}

void Structure::addNote(Note *ptr)
{
    if(ptr == nullptr || m_notes.indexOf(ptr) >= 0)
        return;

    ptr->setParent(this);

    connect(ptr, &Note::aboutToDelete, this, &Structure::removeNote);
    connect(ptr, &Note::noteChanged, this, &Structure::structureChanged);

    m_notes.append(ptr);
    emit noteCountChanged();
}

void Structure::removeNote(Note *ptr)
{
    if(ptr == nullptr)
        return;

    const int index = m_notes.indexOf(ptr);
    if(index < 0)
        return ;

    m_notes.removeAt(index);

    disconnect(ptr, &Note::aboutToDelete, this, &Structure::removeNote);
    disconnect(ptr, &Note::noteChanged, this, &Structure::structureChanged);

    emit noteCountChanged();

    if(ptr->parent() == this)
        ptr->deleteLater();
}

Note *Structure::noteAt(int index) const
{
    return index < 0 || index >= m_notes.size() ? nullptr : m_notes.at(index);
}

void Structure::clearNotes()
{
    while(m_notes.size())
        this->removeNote(m_notes.first());
}

QQmlListProperty<StructureElement> Structure::elements()
{
    return QQmlListProperty<StructureElement>(
                reinterpret_cast<QObject*>(this),
                static_cast<void*>(this),
                &Structure::staticAppendElement,
                &Structure::staticElementCount,
                &Structure::staticElementAt,
                &Structure::staticClearElements);
}

void Structure::addElement(StructureElement *ptr)
{
    if(ptr == nullptr || m_elements.indexOf(ptr) >= 0)
        return;

    m_elements.append(ptr);

    ptr->setParent(this);

    connect(ptr, &StructureElement::elementChanged, this, &Structure::structureChanged);
    connect(ptr, &StructureElement::aboutToDelete, this, &Structure::removeElement);

    this->onStructureElementSceneChanged(ptr);

    emit elementCountChanged();
    emit elementsChanged();
}

void Structure::removeElement(StructureElement *ptr)
{
    if(ptr == nullptr)
        return;

    const int index = m_elements.indexOf(ptr);
    if(index < 0)
        return ;

    m_elements.removeAt(index);

    disconnect(ptr, &StructureElement::elementChanged, this, &Structure::structureChanged);
    disconnect(ptr, &StructureElement::aboutToDelete, this, &Structure::removeElement);

    emit elementCountChanged();
    emit elementsChanged();

    if(ptr->parent() == this)
        ptr->deleteLater();
}

void Structure::moveElement(StructureElement *ptr, int toRow)
{
    if(ptr == nullptr || toRow < 0 || toRow >= m_elements.size())
        return;

    const int fromRow = m_elements.indexOf(ptr);
    if(fromRow < 0)
        return;

    if(fromRow == toRow)
        return;

    m_elements.move(fromRow, toRow);
    emit elementsChanged();
}

StructureElement *Structure::elementAt(int index) const
{
    return index < 0 || index >= m_elements.size() ? nullptr : m_elements.at(index);
}

void Structure::clearElements()
{
    while(m_elements.size())
        this->removeElement(m_elements.first());
}

int Structure::indexOfScene(Scene *scene) const
{
    if(scene == nullptr)
        return -1;

    for(int i=0; i<m_elements.size(); i++)
    {
        StructureElement *element = m_elements.at(i);
        if(element->scene() == scene)
            return i;
    }

    return -1;
}

int Structure::indexOfElement(StructureElement *element) const
{
    return m_elements.indexOf(element);
}

StructureElement *Structure::findElementBySceneID(const QString &id) const
{
    Q_FOREACH(StructureElement *element, m_elements)
    {
        if(element->scene()->id() == id)
            return element;
    }

    return nullptr;
}

void Structure::setCurrentElementIndex(int val)
{
    val = qBound(-1, val, m_elements.size()-1);
    if(m_currentElementIndex == val)
        return;

    m_currentElementIndex = val;
    emit currentElementIndexChanged();
}

void Structure::setZoomLevel(qreal val)
{
    if( qFuzzyCompare(m_zoomLevel, val) )
        return;

    m_zoomLevel = val;
    emit zoomLevelChanged();
}

bool Structure::event(QEvent *event)
{
    if(event->type() == QEvent::ParentChange)
        m_scriteDocument = qobject_cast<ScriteDocument*>(this->parent());

    return QObject::event(event);
}

void Structure::staticAppendArea(QQmlListProperty<StructureArea> *list, StructureArea *ptr)
{
    reinterpret_cast< Structure* >(list->data)->addArea(ptr);
}

void Structure::staticClearAreas(QQmlListProperty<StructureArea> *list)
{
    reinterpret_cast< Structure* >(list->data)->clearAreas();
}

StructureArea *Structure::staticAreaAt(QQmlListProperty<StructureArea> *list, int index)
{
    return reinterpret_cast< Structure* >(list->data)->areaAt(index);
}

int Structure::staticAreaCount(QQmlListProperty<StructureArea> *list)
{
    return reinterpret_cast< Structure* >(list->data)->areaCount();
}

void Structure::staticAppendCharacter(QQmlListProperty<Character> *list, Character *ptr)
{
    reinterpret_cast< Structure* >(list->data)->addCharacter(ptr);
}

void Structure::staticClearCharacters(QQmlListProperty<Character> *list)
{
    reinterpret_cast< Structure* >(list->data)->clearCharacters();
}

Character *Structure::staticCharacterAt(QQmlListProperty<Character> *list, int index)
{
    return reinterpret_cast< Structure* >(list->data)->characterAt(index);
}

int Structure::staticCharacterCount(QQmlListProperty<Character> *list)
{
    return reinterpret_cast< Structure* >(list->data)->characterCount();
}

void Structure::staticAppendNote(QQmlListProperty<Note> *list, Note *ptr)
{
    reinterpret_cast< Structure* >(list->data)->addNote(ptr);
}

void Structure::staticClearNotes(QQmlListProperty<Note> *list)
{
    reinterpret_cast< Structure* >(list->data)->clearNotes();
}

Note *Structure::staticNoteAt(QQmlListProperty<Note> *list, int index)
{
    return reinterpret_cast< Structure* >(list->data)->noteAt(index);
}

int Structure::staticNoteCount(QQmlListProperty<Note> *list)
{
    return reinterpret_cast< Structure* >(list->data)->noteCount();
}

void Structure::staticAppendElement(QQmlListProperty<StructureElement> *list, StructureElement *ptr)
{
    reinterpret_cast< Structure* >(list->data)->addElement(ptr);
}

void Structure::staticClearElements(QQmlListProperty<StructureElement> *list)
{
    reinterpret_cast< Structure* >(list->data)->clearElements();
}

StructureElement *Structure::staticElementAt(QQmlListProperty<StructureElement> *list, int index)
{
    return reinterpret_cast< Structure* >(list->data)->elementAt(index);
}

int Structure::staticElementCount(QQmlListProperty<StructureElement> *list)
{
    return reinterpret_cast< Structure* >(list->data)->elementCount();
}

void Structure::onStructureElementSceneChanged(StructureElement *element)
{
    if(element == nullptr)
        element = qobject_cast<StructureElement*>(this->sender());

    if(element == nullptr || element->scene() == nullptr)
        return;

    connect(element->scene(), &Scene::sceneElementChanged, this, &Structure::onSceneElementChanged);
    connect(element->scene(), &Scene::aboutToRemoveSceneElement, this, &Structure::onAboutToRemoveSceneElement);

    const int nrSceneElements = element->scene()->elementCount();
    for(int i=0; i<nrSceneElements; i++)
        this->onSceneElementChanged(element->scene()->elementAt(i), Scene::ElementTextChange);
}

void Structure::onSceneElementChanged(SceneElement *element, Scene::SceneElementChangeType)
{
    if(element == nullptr)
        return;

    if(element->type() == SceneElement::Character)
    {
        QString characterName = element->text();
        characterName = characterName.section('(', 0, 0).trimmed();
        m_characterElementNameMap[element] = characterName;
        this->evaluateCharacterNames();
    }
    else if(m_characterElementNameMap.contains(element))
    {
        m_characterElementNameMap.remove(element);
        this->evaluateCharacterNames();
    }
}

void Structure::onAboutToRemoveSceneElement(SceneElement *element)
{
    if(m_characterElementNameMap.contains(element))
    {
        m_characterElementNameMap.remove(element);
        this->evaluateCharacterNames();
    }
}

void Structure::evaluateCharacterNames()
{
    m_characterNames = m_characterElementNameMap.values();
    std::sort(m_characterNames.begin(), m_characterNames.end());
    m_characterNames.removeDuplicates();
    m_characterNames.removeAll(QString());
    emit characterNamesChanged();
}

///////////////////////////////////////////////////////////////////////////////

StructureElementConnector::StructureElementConnector(QQuickItem *parent)
                          :AbstractShapeItem(parent),
                            m_lineType(StraightLine),
                            m_toElement(nullptr),
                            m_fromElement(nullptr),
                            m_arrowAndLabelSpacing(30)
{
    this->setRenderType(OutlineOnly);
    this->setOutlineColor(Qt::black);
    this->setOutlineWidth(4);

    connect(this, &AbstractShapeItem::contentRectChanged, this, &StructureElementConnector::updateArrowAndLabelPositions);
}

StructureElementConnector::~StructureElementConnector()
{

}

void StructureElementConnector::setLineType(StructureElementConnector::LineType val)
{
    if(m_lineType == val)
        return;

    m_lineType = val;
    emit lineTypeChanged();
}

void StructureElementConnector::setFromElement(StructureElement *val)
{
    if(m_fromElement == val)
        return;

    if(m_fromElement != nullptr)
    {
        disconnect(m_fromElement, &StructureElement::xChanged, this, &StructureElementConnector::requestUpdate);
        disconnect(m_fromElement, &StructureElement::yChanged, this, &StructureElementConnector::requestUpdate);
        disconnect(m_fromElement, &StructureElement::widthChanged, this, &StructureElementConnector::widthChanged);
        disconnect(m_fromElement, &StructureElement::heightChanged, this, &StructureElementConnector::heightChanged);
        disconnect(m_fromElement, &StructureElement::aboutToDelete, this, &StructureElementConnector::onElementDestroyed);

        Scene *scene = m_fromElement->scene();
        disconnect(scene, &Scene::colorChanged, this, &StructureElementConnector::pickElementColor);
    }

    m_fromElement = val;

    if(m_fromElement != nullptr)
    {
        connect(m_fromElement, &StructureElement::xChanged, this, &StructureElementConnector::requestUpdate);
        connect(m_fromElement, &StructureElement::yChanged, this, &StructureElementConnector::requestUpdate);
        connect(m_fromElement, &StructureElement::widthChanged, this, &StructureElementConnector::widthChanged);
        connect(m_fromElement, &StructureElement::heightChanged, this, &StructureElementConnector::heightChanged);
        connect(m_fromElement, &StructureElement::aboutToDelete, this, &StructureElementConnector::onElementDestroyed);

        Scene *scene = m_fromElement->scene();
        connect(scene, &Scene::colorChanged, this, &StructureElementConnector::pickElementColor);
    }

    emit fromElementChanged();

    this->pickElementColor();

    this->update();
}

void StructureElementConnector::setToElement(StructureElement *val)
{
    if(m_toElement == val)
        return;

    if(m_toElement != nullptr)
    {
        disconnect(m_toElement, &StructureElement::xChanged, this, &StructureElementConnector::requestUpdate);
        disconnect(m_toElement, &StructureElement::yChanged, this, &StructureElementConnector::requestUpdate);
        disconnect(m_toElement, &StructureElement::widthChanged, this, &StructureElementConnector::widthChanged);
        disconnect(m_toElement, &StructureElement::heightChanged, this, &StructureElementConnector::heightChanged);
        disconnect(m_toElement, &StructureElement::aboutToDelete, this, &StructureElementConnector::onElementDestroyed);

        Scene *scene = m_toElement->scene();
        disconnect(scene, &Scene::colorChanged, this, &StructureElementConnector::pickElementColor);
    }

    m_toElement = val;

    if(m_toElement != nullptr)
    {
        connect(m_toElement, &StructureElement::xChanged, this, &StructureElementConnector::requestUpdate);
        connect(m_toElement, &StructureElement::yChanged, this, &StructureElementConnector::requestUpdate);
        connect(m_toElement, &StructureElement::widthChanged, this, &StructureElementConnector::widthChanged);
        connect(m_toElement, &StructureElement::heightChanged, this, &StructureElementConnector::heightChanged);
        connect(m_toElement, &StructureElement::aboutToDelete, this, &StructureElementConnector::onElementDestroyed);

        Scene *scene = m_toElement->scene();
        connect(scene, &Scene::colorChanged, this, &StructureElementConnector::pickElementColor);
    }

    emit toElementChanged();

    this->pickElementColor();

    this->update();
}

void StructureElementConnector::setArrowAndLabelSpacing(qreal val)
{
    if( qFuzzyCompare(m_arrowAndLabelSpacing, val) )
        return;

    m_arrowAndLabelSpacing = val;
    emit arrowAndLabelSpacingChanged();

    this->updateArrowAndLabelPositions();
}

QPainterPath StructureElementConnector::shape() const
{
    QPainterPath path;
    if(m_fromElement == nullptr || m_toElement == nullptr)
        return path;

    auto getElementRect = [](StructureElement *e) {
        QRectF r(e->x(), e->y(), e->width(), e->height());
        r.moveCenter(QPointF(e->x(), e->y()));
        return r;
    };

    const QRectF  r1 = getElementRect(m_fromElement);
    const QRectF  r2 = getElementRect(m_toElement);
    const QLineF line(r1.center(), r2.center());
    QPointF p1, p2;
    Qt::Edge e1, e2;

    if(r2.center().x() < r1.left())
    {
        p1 = QLineF(r1.topLeft(), r1.bottomLeft()).center();
        e1 = Qt::LeftEdge;

        if(r2.top() > r1.bottom())
        {
            p2 = QLineF(r2.topLeft(), r2.topRight()).center();
            e2 = Qt::TopEdge;
        }
        else if(r1.top() > r2.bottom())
        {
            p2 = QLineF(r2.bottomLeft(), r2.bottomRight()).center();
            e2 = Qt::BottomEdge;
        }
        else
        {
            p2 = QLineF(r2.topRight(), r2.bottomRight()).center();
            e2 = Qt::RightEdge;
        }
    }
    else if(r2.center().x() > r1.right())
    {
        p1 = QLineF(r1.topRight(), r1.bottomRight()).center();
        e1 = Qt::RightEdge;

        if(r2.top() > r1.bottom())
        {
            p2 = QLineF(r2.topLeft(), r2.topRight()).center();
            e2 = Qt::TopEdge;
        }
        else if(r1.top() > r2.bottom())
        {
            p2 = QLineF(r2.bottomLeft(), r2.bottomRight()).center();
            e2 = Qt::BottomEdge;
        }
        else
        {
            p2 = QLineF(r2.topLeft(), r2.bottomLeft()).center();
            e2 = Qt::LeftEdge;
        }
    }
    else
    {
        if(r2.top() > r1.bottom())
        {
            p1 = QLineF(r1.bottomLeft(), r1.bottomRight()).center();
            e1 = Qt::BottomEdge;

            p2 = QLineF(r2.topLeft(), r2.topRight()).center();
            e2 = Qt::TopEdge;
        }
        else
        {
            p1 = QLineF(r1.topLeft(), r1.topRight()).center();
            e1 = Qt::TopEdge;

            p2 = QLineF(r2.bottomLeft(), r2.bottomRight()).center();
            e2 = Qt::BottomEdge;
        }
    }

    if(m_lineType == StraightLine)
    {
        path.moveTo(p1);
        path.lineTo(p2);
    }
    else
    {
        QPointF cp = p1;
        switch(e1)
        {
        case Qt::LeftEdge:
        case Qt::RightEdge:
            cp = (e2 == Qt::BottomEdge || e2 == Qt::TopEdge) ? QPointF(p2.x(), p1.y()) : p1;
            break;
        default:
            cp = p1;
            break;
        }

        if(cp == p1)
        {
            path.moveTo(p1);
            path.lineTo(p2);
        }
        else
        {
            const qreal length = line.length();
            const qreal dist = 20.0;
            const qreal dt = dist / length;
            const qreal maxdt = 1.0 - dt;
            const QLineF l1(p1, cp);
            const QLineF l2(cp, p2);
            qreal t = dt;

            path.moveTo(p1);
            while(t < maxdt)
            {
                const QLineF l( l1.pointAt(t), l2.pointAt(t) );
                const QPointF p = l.pointAt(t);
                path.lineTo(p);
                t += dt;
            }
            path.lineTo(p2);
        }
    }

    static const QList<QPointF> arrowPoints = QList<QPointF>()
            << QPointF(-10,-5) << QPointF(0, 0) << QPointF(-10,5);

    const qreal angle = path.angleAtPercent(0.5);
    const QPointF lineCenter = path.pointAtPercent(0.5);

    QTransform tx;
    tx.translate(lineCenter.x(), lineCenter.y());
    tx.rotate(-angle);
    path.moveTo( tx.map(arrowPoints.at(0)) );
    path.lineTo( tx.map(arrowPoints.at(1)) );
    path.lineTo( tx.map(arrowPoints.at(2)) );

    return path;
}

void StructureElementConnector::onElementDestroyed(StructureElement *element)
{
    if(element == m_fromElement)
        this->setFromElement(nullptr);
    else if(element == m_toElement)
        this->setToElement(nullptr);
}

void StructureElementConnector::pickElementColor()
{
    if(m_fromElement != nullptr && m_toElement != nullptr)
    {
        const QColor c1 = m_fromElement->scene()->color();
        const QColor c2 = m_toElement->scene()->color();
        QColor mix = QColor::fromRgbF( (c1.redF()+c2.redF())/2.0,
                                       (c1.greenF()+c2.greenF())/2.0,
                                       (c1.blueF()+c2.blueF())/2.0 );
        this->setOutlineColor(mix);
    }
}

void StructureElementConnector::updateArrowAndLabelPositions()
{
    const QPainterPath path = this->currentShape();
    if(path.isEmpty())
        return;

    const qreal pathLength = path.length();
    if(pathLength < 0 || qFuzzyCompare(pathLength,0))
        return;

    const qreal arrowT = 0.5;
    const qreal labelT = 0.45 - (m_arrowAndLabelSpacing / pathLength);

    this->setArrowPosition( this->currentShape().pointAtPercent(arrowT) );
    if(labelT < 0 || labelT > 1)
        this->setSuggestedLabelPosition(this->arrowPosition());
    else
        this->setSuggestedLabelPosition( this->currentShape().pointAtPercent(labelT) );
}

void StructureElementConnector::setArrowPosition(const QPointF &val)
{
    if(m_arrowPosition == val)
        return;

    m_arrowPosition = val;
    emit arrowPositionChanged();
}

void StructureElementConnector::setSuggestedLabelPosition(const QPointF &val)
{
    if(m_suggestedLabelPosition == val)
        return;

    m_suggestedLabelPosition = val;
    emit suggestedLabelPositionChanged();
}
