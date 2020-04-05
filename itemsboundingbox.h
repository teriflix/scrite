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

#ifndef ITEMSBOUNDINGBOX_H
#define ITEMSBOUNDINGBOX_H

#include <QObject>
#include <QQmlEngine>
#include <QQuickItem>
#include <QBasicTimer>

class ItemsBoundingBox;

class BoundingBoxItem : public QObject
{
    Q_OBJECT

public:
    ~BoundingBoxItem();

    static BoundingBoxItem *qmlAttachedProperties(QObject *object);

    Q_PROPERTY(QQuickItem* item READ item CONSTANT)
    QQuickItem* item() const { return m_item; }

    Q_PROPERTY(ItemsBoundingBox* belongsTo READ belongsTo WRITE setBelongsTo NOTIFY belongsToChanged)
    void setBelongsTo(ItemsBoundingBox* val);
    ItemsBoundingBox* belongsTo() const { return m_boundingBox; }
    Q_SIGNAL void belongsToChanged();

    Q_PROPERTY(QRectF itemRect READ itemRect NOTIFY itemRectChanged)
    QRectF itemRect() const;
    Q_SIGNAL void itemRectChanged();

protected:
    void onBoundingBoxDestroyed();
    BoundingBoxItem(QObject *parent=nullptr);

private:
    QQuickItem *m_item;
    ItemsBoundingBox* m_boundingBox;
};
Q_DECLARE_METATYPE(BoundingBoxItem*)
QML_DECLARE_TYPEINFO(BoundingBoxItem, QML_HAS_ATTACHED_PROPERTIES)

class ItemsBoundingBox : public QObject
{
    Q_OBJECT

public:
    ItemsBoundingBox(QObject *parent=nullptr);
    ~ItemsBoundingBox();

    Q_PROPERTY(QMarginsF margins READ margins WRITE setMargins NOTIFY marginsChanged)
    void setMargins(const QMarginsF &val);
    QMarginsF margins() const { return m_margins; }
    Q_SIGNAL void marginsChanged();

    Q_INVOKABLE QMarginsF createMargins(qreal left, qreal top, qreal right, qreal bottom) const {
        return QMarginsF(left,top,right,bottom);
    }

    Q_PROPERTY(QRectF rect READ rect NOTIFY rectChanged)
    QRectF rect() const { return m_rect; }
    Q_SIGNAL void rectChanged();

    Q_PROPERTY(qreal x READ x NOTIFY rectChanged)
    qreal x() const { return m_rect.x(); }

    Q_PROPERTY(qreal y READ y NOTIFY rectChanged)
    qreal y() const { return m_rect.y(); }

    Q_PROPERTY(qreal width READ width NOTIFY rectChanged)
    qreal width() const { return m_rect.width(); }

    Q_PROPERTY(qreal height READ height NOTIFY rectChanged)
    qreal height() const { return m_rect.height(); }

    Q_PROPERTY(qreal left READ left NOTIFY rectChanged)
    qreal left() const { return m_rect.left(); }

    Q_PROPERTY(qreal top READ top NOTIFY rectChanged)
    qreal top() const { return m_rect.top(); }

    Q_PROPERTY(qreal right READ right NOTIFY rectChanged)
    qreal right() const { return m_rect.right(); }

    Q_PROPERTY(qreal bottom READ bottom NOTIFY rectChanged)
    qreal bottom() const { return m_rect.bottom(); }

    Q_PROPERTY(QQmlListProperty<BoundingBoxItem> items READ items NOTIFY itemCountChanged)
    QQmlListProperty<BoundingBoxItem> items();
    Q_INVOKABLE void addItem(BoundingBoxItem *ptr);
    Q_INVOKABLE void removeItem(BoundingBoxItem *ptr);
    Q_INVOKABLE BoundingBoxItem *itemAt(int index) const;
    Q_PROPERTY(int itemCount READ itemCount NOTIFY itemCountChanged)
    int itemCount() const { return m_items.size(); }
    Q_INVOKABLE void clearItems();
    Q_SIGNAL void itemCountChanged();

protected:
    void timerEvent(QTimerEvent *event);

private:
    void setRect(const QRectF &val);

    void computeBoundingBox();
    void computeBoundingBoxLater();

private:
    QRectF m_rect;
    QMarginsF m_margins;
    QBasicTimer m_computeTimer;

    static BoundingBoxItem* staticItemAt(QQmlListProperty<BoundingBoxItem> *list, int index);
    static int staticItemCount(QQmlListProperty<BoundingBoxItem> *list);
    QList<BoundingBoxItem *> m_items;
};

#endif // ITEMSBOUNDINGBOX_H
