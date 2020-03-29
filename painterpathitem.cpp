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

#include "painterpathitem.h"

PainterPathItem::PainterPathItem(QQuickItem *parent)
    : AbstractShapeItem(parent),
      m_path(nullptr)
{
}

PainterPathItem::~PainterPathItem()
{

}

void PainterPathItem::setPainterPath(PainterPath *val)
{
    if(m_path == val)
        return;

    if(m_path)
        disconnect(m_path, SIGNAL(updated()), this, SLOT(update()));

    m_path = val;
    if(m_path)
        connect(m_path, SIGNAL(updated()), this, SLOT(update()));

    emit painterPathChanged();
}

QPainterPath PainterPathItem::shape() const
{
    return m_path->path();
}

///////////////////////////////////////////////////////////////////////////////

AbstractPathElement::AbstractPathElement(QObject *parent)
    : QObject(parent), m_enabled(true)
{

}

AbstractPathElement::~AbstractPathElement()
{

}

void AbstractPathElement::setEnabled(bool val)
{
    if(m_enabled == val)
        return;

    m_enabled = val;
    emit enabledChanged();

    emit updated();
}

///////////////////////////////////////////////////////////////////////////////

PainterPath::PainterPath(QObject *parent)
    : QObject(parent),
      m_dirty(false)
{

}

PainterPath::~PainterPath()
{

}

QQmlListProperty<AbstractPathElement> PainterPath::elements()
{
    return QQmlListProperty<AbstractPathElement>(this,
        nullptr, elements_append, elements_count, elements_at, elements_clear);
}

QPointF PainterPath::pointAtPercent(qreal t) const
{
    return m_path.pointAtPercent(t);
}

qreal PainterPath::length() const
{
    return m_path.length();
}

QPainterPath PainterPath::path()
{
    if(m_dirty)
        this->composePath();

    return m_path;
}

void PainterPath::markDirty()
{
    m_dirty = true;
    emit updated();
}

void PainterPath::composePath()
{
    m_path = QPainterPath();
    Q_FOREACH(AbstractPathElement *element, m_pathElements) {
        if(element->isEnabled())
            element->apply(m_path);
    }
}

AbstractPathElement *PainterPath::elements_at(QQmlListProperty<AbstractPathElement> *list, int index)
{
    PainterPath *path = qobject_cast<PainterPath*>(list->object);
    if(path == nullptr || index < 0 || index >= path->m_pathElements.size())
        return nullptr;

    return path->m_pathElements.at(index);
}

void PainterPath::elements_append(QQmlListProperty<AbstractPathElement> *list, AbstractPathElement *element)
{
    PainterPath *path = qobject_cast<PainterPath*>(list->object);
    if(path == nullptr || element == nullptr)
        return;

    connect(element, SIGNAL(updated()), path, SLOT(markDirty()));
    path->m_pathElements.append(element);
    path->markDirty();
    emit path->elementsChanged();
}

int PainterPath::elements_count(QQmlListProperty<AbstractPathElement> *list)
{
    PainterPath *path = qobject_cast<PainterPath*>(list->object);
    if(path == nullptr)
        return 0;

    return path->m_pathElements.size();
}

void PainterPath::elements_clear(QQmlListProperty<AbstractPathElement> *list)
{
    PainterPath *path = qobject_cast<PainterPath*>(list->object);
    if(path == nullptr)
        return;

    while(path->m_pathElements.size())
    {
        AbstractPathElement *element = path->m_pathElements.takeFirst();
        disconnect(element, nullptr, path, nullptr);
    }

    path->markDirty();
    emit path->elementsChanged();
}

///////////////////////////////////////////////////////////////////////////////

MoveToElement::MoveToElement(QObject *parent)
    : AbstractPathElement(parent)
{

}

MoveToElement::~MoveToElement()
{

}

void MoveToElement::setX(qreal val)
{
    if(qFuzzyCompare(val, m_x))
        return;

    m_x = val;
    emit xChanged();
    emit updated();
}

void MoveToElement::setY(qreal val)
{
    if(qFuzzyCompare(val, m_y))
        return;

    m_y = val;
    emit yChanged();
    emit updated();
}

void MoveToElement::apply(QPainterPath &path)
{
    if( !this->isEnabled() )
        return;

    path.moveTo(m_x, m_y);
}

///////////////////////////////////////////////////////////////////////////////

LineToElement::LineToElement(QObject *parent)
    : MoveToElement(parent)
{

}

LineToElement::~LineToElement()
{

}

void LineToElement::apply(QPainterPath &path)
{
    if( !this->isEnabled() )
        return;

    path.lineTo(m_x, m_y);
}

///////////////////////////////////////////////////////////////////////////////

CloseSubpathElement::CloseSubpathElement(QObject *parent)
    : AbstractPathElement(parent)
{

}

CloseSubpathElement::~CloseSubpathElement()
{

}

void CloseSubpathElement::apply(QPainterPath &path)
{
    if( !this->isEnabled() )
        return;

    path.closeSubpath();
}

///////////////////////////////////////////////////////////////////////////////

CubicToElement::CubicToElement(QObject *parent)
               :AbstractPathElement(parent)
{

}

CubicToElement::~CubicToElement()
{

}

void CubicToElement::setControlPoint1(const QPointF &val)
{
    if(m_controlPoint1 == val)
        return;

    m_controlPoint1 = val;
    emit controlPoint1Changed();
    emit updated();
}

void CubicToElement::setControlPoint2(const QPointF &val)
{
    if(m_controlPoint2 == val)
        return;

    m_controlPoint2 = val;
    emit controlPoint2Changed();
    emit updated();
}

void CubicToElement::setEndPoint(const QPointF &val)
{
    if(m_endPoint == val)
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

QuadToElement::QuadToElement(QObject *parent)
    :AbstractPathElement(parent)
{

}

QuadToElement::~QuadToElement()
{

}

void QuadToElement::setControlPoint(const QPointF &val)
{
    if(m_controlPoint == val)
        return;

    m_controlPoint = val;
    emit controlPointChanged();
    emit updated();
}

void QuadToElement::setEndPoint(const QPointF &val)
{
    if(m_endPoint == val)
        return;

    m_endPoint = val;
    emit endPointChanged();
    emit updated();
}

void QuadToElement::apply(QPainterPath &path)
{
    path.quadTo(m_controlPoint, m_endPoint);
}
