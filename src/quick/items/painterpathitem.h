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

#ifndef PATHITEM_H
#define PATHITEM_H

#include "abstractshapeitem.h"
#include "qobjectproperty.h"

#include <QQmlListProperty>
#include <QPainterPath>
#include <QJsonObject>

class PainterPath;

class PainterPathItem : public AbstractShapeItem
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit PainterPathItem(QQuickItem *parent = nullptr);
    ~PainterPathItem();

    Q_PROPERTY(PainterPath* painterPath READ painterPath WRITE setPainterPath NOTIFY painterPathChanged RESET resetPainterPath)
    void setPainterPath(PainterPath *val);
    PainterPath *painterPath() const { return m_painterPath; }
    Q_SIGNAL void painterPathChanged();

    Q_PROPERTY(QPainterPath path READ path WRITE setPath NOTIFY pathChanged)
    void setPath(QPainterPath val);
    QPainterPath path() const { return m_path; }
    Q_SIGNAL void pathChanged();

    Q_INVOKABLE void setPathFromString(const QString &val);

    QPainterPath shape() const;

private:
    void resetPainterPath();

protected:
    QPainterPath m_path;
    QObjectProperty<PainterPath> m_painterPath;
};

class AbstractPathElement : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use subclasses of AbstractPathElement.")

public:
    explicit AbstractPathElement(QObject *parent = nullptr);
    ~AbstractPathElement();

    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    void setEnabled(bool val);
    bool isEnabled() const { return m_enabled; }
    Q_SIGNAL void enabledChanged();

    virtual void apply(QPainterPath &path) = 0;

signals:
    void updated();

private:
    bool m_enabled = true;
};

class PainterPath : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit PainterPath(QObject *parent = nullptr);
    ~PainterPath();

    Q_CLASSINFO("DefaultProperty", "elements")
    Q_PROPERTY(QQmlListProperty<AbstractPathElement> elements READ elements NOTIFY elementsChanged STORED false)
    QQmlListProperty<AbstractPathElement> elements();
    Q_SIGNAL void elementsChanged();

    Q_PROPERTY(QJsonObject itemRect READ itemRect NOTIFY itemRectChanged)
    QJsonObject itemRect() const;
    Q_SIGNAL void itemRectChanged();

    Q_PROPERTY(bool dirty READ isDirty NOTIFY dirtyChanged)
    bool isDirty() const { return m_dirty; }
    Q_SIGNAL void dirtyChanged();

    Q_INVOKABLE QPointF pointInLine(const QPointF &p1, const QPointF &p2, qreal t,
                                    bool absolute = false) const;

    Q_INVOKABLE QPointF pointAtPercent(qreal t) const;
    Q_INVOKABLE qreal length() const;

    Q_INVOKABLE void reset();

    QPainterPath path();

signals:
    void updated();

private:
    Q_SLOT void markDirty();
    void composePath();

private:
    static AbstractPathElement *elements_at(QQmlListProperty<AbstractPathElement> *, int);
    static void elements_append(QQmlListProperty<AbstractPathElement> *, AbstractPathElement *);
    static int elements_count(QQmlListProperty<AbstractPathElement> *);
    static void elements_clear(QQmlListProperty<AbstractPathElement> *);

private:
    bool m_dirty = false;
    QPainterPath m_path;
    QList<AbstractPathElement *> m_pathElements;
};

class MoveToElement : public AbstractPathElement
{
    Q_OBJECT
    QML_ELEMENT
    QML_NAMED_ELEMENT(MoveTo)

public:
    explicit MoveToElement(QObject *parent = nullptr);
    ~MoveToElement();

    Q_PROPERTY(qreal x READ x WRITE setX NOTIFY xChanged)
    void setX(qreal val);
    qreal x() const { return m_x; }
    Q_SIGNAL void xChanged();

    Q_PROPERTY(qreal y READ y WRITE setY NOTIFY yChanged)
    void setY(qreal val);
    qreal y() const { return m_y; }
    Q_SIGNAL void yChanged();

    void apply(QPainterPath &path);

protected:
    qreal m_x = 0;
    qreal m_y = 0;
};

class LineToElement : public MoveToElement
{
    Q_OBJECT
    QML_ELEMENT
    QML_NAMED_ELEMENT(LineTo)

public:
    explicit LineToElement(QObject *parent = nullptr);
    ~LineToElement();

    void apply(QPainterPath &path);
};

class CloseSubpathElement : public AbstractPathElement
{
    Q_OBJECT
    QML_NAMED_ELEMENT(CloseSubpath)

public:
    explicit CloseSubpathElement(QObject *parent = nullptr);
    ~CloseSubpathElement();

    void apply(QPainterPath &path);
};

class CubicToElement : public AbstractPathElement
{
    Q_OBJECT
    QML_ELEMENT
    QML_NAMED_ELEMENT(CubicTo)

public:
    explicit CubicToElement(QObject *parent = nullptr);
    ~CubicToElement();

    Q_PROPERTY(QPointF controlPoint1 READ controlPoint1 WRITE setControlPoint1 NOTIFY controlPoint1Changed)
    void setControlPoint1(const QPointF &val);
    QPointF controlPoint1() const { return m_controlPoint1; }
    Q_SIGNAL void controlPoint1Changed();

    Q_PROPERTY(QPointF controlPoint2 READ controlPoint2 WRITE setControlPoint2 NOTIFY controlPoint2Changed)
    void setControlPoint2(const QPointF &val);
    QPointF controlPoint2() const { return m_controlPoint2; }
    Q_SIGNAL void controlPoint2Changed();

    Q_PROPERTY(QPointF endPoint READ endPoint WRITE setEndPoint NOTIFY endPointChanged)
    void setEndPoint(const QPointF &val);
    QPointF endPoint() const { return m_endPoint; }
    Q_SIGNAL void endPointChanged();

    void apply(QPainterPath &path);

private:
    QPointF m_endPoint;
    QPointF m_controlPoint1;
    QPointF m_controlPoint2;
};

class QuadToElement : public AbstractPathElement
{
    Q_OBJECT
    QML_ELEMENT
    QML_NAMED_ELEMENT(QuadTo)

public:
    explicit QuadToElement(QObject *parent = nullptr);
    ~QuadToElement();

    Q_PROPERTY(QPointF controlPoint READ controlPoint WRITE setControlPoint NOTIFY controlPointChanged)
    void setControlPoint(const QPointF &val);
    QPointF controlPoint() const { return m_controlPoint; }
    Q_SIGNAL void controlPointChanged();

    Q_PROPERTY(QPointF endPoint READ endPoint WRITE setEndPoint NOTIFY endPointChanged)
    void setEndPoint(const QPointF &val);
    QPointF endPoint() const { return m_endPoint; }
    Q_SIGNAL void endPointChanged();

    void apply(QPainterPath &path);

private:
    QPointF m_endPoint;
    QPointF m_controlPoint;
};

class ArcToElement : public AbstractPathElement
{
    Q_OBJECT
    QML_ELEMENT
    QML_NAMED_ELEMENT(ArcTo)

public:
    explicit ArcToElement(QObject *parent = nullptr);
    ~ArcToElement();

    Q_PROPERTY(QRectF rectangle READ rectangle WRITE setRectangle NOTIFY rectangleChanged)
    void setRectangle(const QRectF &val);
    QRectF rectangle() const { return m_rectangle; }
    Q_SIGNAL void rectangleChanged();

    Q_PROPERTY(qreal startAngle READ startAngle WRITE setStartAngle NOTIFY startAngleChanged)
    void setStartAngle(qreal val);
    qreal startAngle() const { return m_startAngle; }
    Q_SIGNAL void startAngleChanged();

    Q_PROPERTY(qreal sweepLength READ sweepLength WRITE setSweepLength NOTIFY sweepLengthChanged)
    void setSweepLength(qreal val);
    qreal sweepLength() const { return m_sweepLength; }
    Q_SIGNAL void sweepLengthChanged();

    void apply(QPainterPath &path);

private:
    QRectF m_rectangle = QRectF(0, 0, 100, 100);
    qreal m_startAngle = 0;
    qreal m_sweepLength = 360;
};

#endif // PATHITEM_H
