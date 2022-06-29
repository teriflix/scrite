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

#include "painterpathitem.h"
#include "application.h"

PainterPathItem::PainterPathItem(QQuickItem *parent)
    : AbstractShapeItem(parent), m_painterPath(this, "painterPath")
{
}

PainterPathItem::~PainterPathItem() { }

void PainterPathItem::setPainterPath(PainterPath *val)
{
    if (m_painterPath == val)
        return;

    if (m_painterPath) {
        disconnect(m_painterPath, SIGNAL(updated()), this, SLOT(update()));
        disconnect(this, SIGNAL(widthChanged()), m_painterPath, SIGNAL(itemRectChanged()));
        disconnect(this, SIGNAL(heightChanged()), m_painterPath, SIGNAL(itemRectChanged()));
    }

    m_painterPath = val;

    if (m_painterPath) {
        m_painterPath->setParent(this);
        connect(m_painterPath, SIGNAL(updated()), this, SLOT(update()));
        connect(this, SIGNAL(widthChanged()), m_painterPath, SIGNAL(itemRectChanged()));
        connect(this, SIGNAL(heightChanged()), m_painterPath, SIGNAL(itemRectChanged()));
    }

    emit painterPathChanged();

    this->update();
}

void PainterPathItem::setPath(QPainterPath val)
{
    if (m_path == val)
        return;

    m_path = val;
    emit pathChanged();

    this->update();
}

void PainterPathItem::setPathFromString(const QString &val)
{
    const QPainterPath path = Application::instance()->stringToPainterPath(val);
    this->setPath(path);
}

QPainterPath PainterPathItem::shape() const
{
    return m_painterPath ? m_painterPath->path() : m_path;
}

void PainterPathItem::resetPainterPath()
{
    m_painterPath = nullptr;
    emit painterPathChanged();
}

///////////////////////////////////////////////////////////////////////////////

AbstractPathElement::AbstractPathElement(QObject *parent) : QObject(parent) { }

AbstractPathElement::~AbstractPathElement() { }

void AbstractPathElement::setEnabled(bool val)
{
    if (m_enabled == val)
        return;

    m_enabled = val;
    emit enabledChanged();

    emit updated();
}

///////////////////////////////////////////////////////////////////////////////

PainterPath::PainterPath(QObject *parent) : QObject(parent) { }

PainterPath::~PainterPath() { }

QQmlListProperty<AbstractPathElement> PainterPath::elements()
{
    return QQmlListProperty<AbstractPathElement>(this, nullptr, elements_append, elements_count,
                                                 elements_at, elements_clear);
}

QJsonObject PainterPath::itemRect() const
{
    auto createJson = [](const QRectF &rect) {
        QJsonObject ret;
        ret.insert("x", rect.x());
        ret.insert("y", rect.y());
        ret.insert("width", rect.width());
        ret.insert("height", rect.height());
        ret.insert("left", rect.left());
        ret.insert("top", rect.top());
        ret.insert("right", rect.right());
        ret.insert("bottom", rect.bottom());

        QJsonObject center;
        center.insert("x", rect.center().x());
        center.insert("y", rect.center().y());
        ret.insert("center", center);
        return ret;
    };

    PainterPathItem *item = qobject_cast<PainterPathItem *>(this->parent());
    if (item)
        return createJson(item->boundingRect());

    return createJson(QRectF());
}

QPointF PainterPath::pointInLine(const QPointF &p1, const QPointF &p2, qreal t, bool absolute) const
{
    const QLineF line(p1, p2);
    if (absolute)
        t = qBound(0.0, t / line.length(), 1.0);
    return line.pointAt(t);
}

QPointF PainterPath::pointAtPercent(qreal t) const
{
    return m_path.pointAtPercent(t);
}

qreal PainterPath::length() const
{
    return m_path.length();
}

void PainterPath::reset()
{
    this->markDirty();
}

QPainterPath PainterPath::path()
{
    if (m_dirty)
        this->composePath();

    return m_path;
}

void PainterPath::markDirty()
{
    m_dirty = true;
    emit dirtyChanged();
    emit updated();
}

void PainterPath::composePath()
{
    m_path = QPainterPath();
    for (AbstractPathElement *element : qAsConst(m_pathElements)) {
        if (element->isEnabled())
            element->apply(m_path);
    }

    m_dirty = false;
    emit dirtyChanged();
}

AbstractPathElement *PainterPath::elements_at(QQmlListProperty<AbstractPathElement> *list,
                                              int index)
{
    PainterPath *path = qobject_cast<PainterPath *>(list->object);
    if (path == nullptr || index < 0 || index >= path->m_pathElements.size())
        return nullptr;

    return path->m_pathElements.at(index);
}

void PainterPath::elements_append(QQmlListProperty<AbstractPathElement> *list,
                                  AbstractPathElement *element)
{
    PainterPath *path = qobject_cast<PainterPath *>(list->object);
    if (path == nullptr || element == nullptr)
        return;

    connect(element, SIGNAL(updated()), path, SLOT(markDirty()));
    element->setParent(path);
    path->m_pathElements.append(element);
    path->markDirty();
    emit path->elementsChanged();
}

int PainterPath::elements_count(QQmlListProperty<AbstractPathElement> *list)
{
    PainterPath *path = qobject_cast<PainterPath *>(list->object);
    if (path == nullptr)
        return 0;

    return path->m_pathElements.size();
}

void PainterPath::elements_clear(QQmlListProperty<AbstractPathElement> *list)
{
    PainterPath *path = qobject_cast<PainterPath *>(list->object);
    if (path == nullptr)
        return;

    while (path->m_pathElements.size()) {
        AbstractPathElement *element = path->m_pathElements.takeFirst();
        disconnect(element, nullptr, path, nullptr);
    }

    path->markDirty();
    emit path->elementsChanged();
}

///////////////////////////////////////////////////////////////////////////////

MoveToElement::MoveToElement(QObject *parent) : AbstractPathElement(parent) { }

MoveToElement::~MoveToElement() { }

void MoveToElement::setX(qreal val)
{
    if (qFuzzyCompare(val, m_x))
        return;

    m_x = val;
    emit xChanged();
    emit updated();
}

void MoveToElement::setY(qreal val)
{
    if (qFuzzyCompare(val, m_y))
        return;

    m_y = val;
    emit yChanged();
    emit updated();
}

void MoveToElement::apply(QPainterPath &path)
{
    if (!this->isEnabled())
        return;

    path.moveTo(m_x, m_y);
}

///////////////////////////////////////////////////////////////////////////////

LineToElement::LineToElement(QObject *parent) : MoveToElement(parent) { }

LineToElement::~LineToElement() { }

void LineToElement::apply(QPainterPath &path)
{
    if (!this->isEnabled())
        return;

    path.lineTo(m_x, m_y);
}

///////////////////////////////////////////////////////////////////////////////

CloseSubpathElement::CloseSubpathElement(QObject *parent) : AbstractPathElement(parent) { }

CloseSubpathElement::~CloseSubpathElement() { }

void CloseSubpathElement::apply(QPainterPath &path)
{
    if (!this->isEnabled())
        return;

    path.closeSubpath();
}

///////////////////////////////////////////////////////////////////////////////

CubicToElement::CubicToElement(QObject *parent) : AbstractPathElement(parent) { }

CubicToElement::~CubicToElement() { }

void CubicToElement::setControlPoint1(const QPointF &val)
{
    if (m_controlPoint1 == val)
        return;

    m_controlPoint1 = val;
    emit controlPoint1Changed();
    emit updated();
}

void CubicToElement::setControlPoint2(const QPointF &val)
{
    if (m_controlPoint2 == val)
        return;

    m_controlPoint2 = val;
    emit controlPoint2Changed();
    emit updated();
}

void CubicToElement::setEndPoint(const QPointF &val)
{
    if (m_endPoint == val)
        return;

    m_endPoint = val;
    emit endPointChanged();
    emit updated();
}

void CubicToElement::apply(QPainterPath &path)
{
    path.cubicTo(m_controlPoint1, m_controlPoint2, m_endPoint);
}

///////////////////////////////////////////////////////////////////////////////

QuadToElement::QuadToElement(QObject *parent) : AbstractPathElement(parent) { }

QuadToElement::~QuadToElement() { }

void QuadToElement::setControlPoint(const QPointF &val)
{
    if (m_controlPoint == val)
        return;

    m_controlPoint = val;
    emit controlPointChanged();
    emit updated();
}

void QuadToElement::setEndPoint(const QPointF &val)
{
    if (m_endPoint == val)
        return;

    m_endPoint = val;
    emit endPointChanged();
    emit updated();
}

void QuadToElement::apply(QPainterPath &path)
{
    path.quadTo(m_controlPoint, m_endPoint);
}

///////////////////////////////////////////////////////////////////////////////

ArcToElement::ArcToElement(QObject *parent) : AbstractPathElement(parent) { }

ArcToElement::~ArcToElement() { }

void ArcToElement::setRectangle(const QRectF &val)
{
    if (m_rectangle == val)
        return;

    m_rectangle = val;
    emit rectangleChanged();
    emit updated();
}

void ArcToElement::setStartAngle(qreal val)
{
    if (qFuzzyCompare(m_startAngle, val))
        return;

    m_startAngle = val;
    emit startAngleChanged();
    emit updated();
}

void ArcToElement::setSweepLength(qreal val)
{
    if (qFuzzyCompare(m_sweepLength, val))
        return;

    m_sweepLength = val;
    emit sweepLengthChanged();
    emit updated();
}

void ArcToElement::apply(QPainterPath &path)
{
    path.arcTo(m_rectangle, m_startAngle, m_sweepLength);
}
