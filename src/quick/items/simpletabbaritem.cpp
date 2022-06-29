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

#include "simpletabbaritem.h"
#include "application.h"

#include <QPainter>
#include <QFontMetrics>
#include <QPainterPath>

SimpleTabBarItem::SimpleTabBarItem(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    this->setAcceptedMouseButtons(Qt::LeftButton);

    m_updateTabInfosTimer.setSingleShot(true);
    m_updateTabInfosTimer.setInterval(0);
    connect(&m_updateTabInfosTimer, &QTimer::timeout, this, &SimpleTabBarItem::updateTabInfos);
}

SimpleTabBarItem::~SimpleTabBarItem() { }

void SimpleTabBarItem::setTabCount(int val)
{
    if (m_tabCount == val)
        return;

    m_tabCount = val;
    emit tabCountChanged();

    this->setActiveTabIndex(m_activeTabIndex);

    this->updateTabInfosLater();
}

void SimpleTabBarItem::setActiveTabIndex(int val)
{
    int val2 = m_tabCount == 0 ? -1 : qBound(0, val, m_tabCount - 1);
    if (m_activeTabIndex == val2)
        return;

    m_activeTabIndex = val2;
    emit activeTabIndexChanged();

    this->updateTabInfosLater();
}

void SimpleTabBarItem::setTabLabelStyle(SimpleTabBarItem::TabLabelStyle val)
{
    if (m_tabLabelStyle == val)
        return;

    m_tabLabelStyle = val;
    emit tabLabelStyleChanged();

    this->updateTabInfosLater();
}

void SimpleTabBarItem::setActiveTabColor(const QColor &val)
{
    if (m_activeTabColor == val)
        return;

    m_activeTabColor = val;
    emit activeTabColorChanged();

    this->updateTabInfosLater();
}

void SimpleTabBarItem::setInactiveTabColor(const QColor &val)
{
    if (m_inactiveTabColor == val)
        return;

    m_inactiveTabColor = val;
    emit inactiveTabColorChanged();

    this->updateTabInfosLater();
}

void SimpleTabBarItem::setActiveTabBorderColor(const QColor &val)
{
    if (m_activeTabBorderColor == val)
        return;

    m_activeTabBorderColor = val;
    emit activeTabBorderColorChanged();

    this->updateTabInfosLater();
}

void SimpleTabBarItem::setInactiveTabBorderColor(const QColor &val)
{
    if (m_inactiveTabBorderColor == val)
        return;

    m_inactiveTabBorderColor = val;
    emit inactiveTabBorderColorChanged();

    this->updateTabInfosLater();
}

void SimpleTabBarItem::setActiveTabBorderWidth(qreal val)
{
    qreal val2 = qBound(1.0, val, 5.0);
    if (qFuzzyCompare(m_activeTabBorderWidth, val2))
        return;

    m_activeTabBorderWidth = val2;
    emit activeTabBorderWidthChanged();

    this->updateTabInfosLater();
}

void SimpleTabBarItem::setInactiveTabBorderWidth(qreal val)
{
    qreal val2 = qBound(1.0, val, 5.0);
    if (qFuzzyCompare(m_inactiveTabBorderWidth, val2))
        return;

    m_inactiveTabBorderWidth = val2;
    emit inactiveTabBorderWidthChanged();

    this->updateTabInfosLater();
}

void SimpleTabBarItem::setTabCurveRadius(qreal val)
{
    qreal val2 = qBound(0.0, val, 0.5);
    if (qFuzzyCompare(m_tabCurveRadius, val2))
        return;

    m_tabCurveRadius = val2;
    emit tabCurveRadiusChanged();

    this->updateTabInfosLater();
}

void SimpleTabBarItem::setActiveTabTextColor(const QColor &val)
{
    if (m_activeTabTextColor == val)
        return;

    m_activeTabTextColor = val;
    emit activeTabTextColorChanged();

    this->updateTabInfosLater();
}

void SimpleTabBarItem::setInactiveTabTextColor(const QColor &val)
{
    if (m_inactiveTabTextColor == val)
        return;

    m_inactiveTabTextColor = val;
    emit inactiveTabTextColorChanged();

    this->updateTabInfosLater();
}

void SimpleTabBarItem::setActiveTabFont(const QFont &val)
{
    if (m_activeTabFont == val)
        return;

    m_activeTabFont = val;
    emit activeTabFontChanged();

    this->updateTabInfosLater();
}

void SimpleTabBarItem::setInactiveTabFont(const QFont &val)
{
    if (m_inactiveTabFont == val)
        return;

    m_inactiveTabFont = val;
    emit inactiveTabFontChanged();

    this->updateTabInfosLater();
}

void SimpleTabBarItem::setMinimumTabWidth(qreal val)
{
    if (qFuzzyCompare(m_minimumTabWidth, val))
        return;

    m_minimumTabWidth = val;
    emit minimumTabWidthChanged();

    this->updateTabInfosLater();
}

void SimpleTabBarItem::setRequestedAttributeValue(const QVariant &val)
{
    if (m_requestedAttributeValue == val)
        return;

    m_requestedAttributeValue = val;
    emit requestedAttributeValueChanged();
}

QRectF SimpleTabBarItem::tabRect(int index) const
{
    if (index < 0 || index >= m_tabInfos.size())
        return QRectF();

    return m_tabInfos.at(index).path.boundingRect();
}

void SimpleTabBarItem::paint(QPainter *painter)
{
    painter->setRenderHint(QPainter::Antialiasing);

    auto drawTab = [=](const TabInfo &item, bool active) {
        painter->fillPath(item.path, item.bgColor);

        painter->setPen(
                QPen(item.borderColor, active ? m_activeTabBorderWidth : m_inactiveTabBorderWidth));
        painter->drawPath(item.path);

        painter->setPen(QPen(item.textColor));
        painter->setFont(item.font);

        const QRectF tabRect = item.path.boundingRect();
        painter->drawText(tabRect, Qt::AlignCenter, item.label);
    };

    const int ati = qBound(-1, m_activeTabIndex, m_tabInfos.size() - 1);
    if (ati >= 0) {
        for (int i = m_tabInfos.size() - 1; i > ati; i--) {
            const TabInfo &item = m_tabInfos.at(i);
            drawTab(item, false);
        }

        for (int i = 0; i < ati; i++) {
            const TabInfo &item = m_tabInfos.at(i);
            drawTab(item, false);
        }

        const TabInfo &item = m_tabInfos.at(ati);
        drawTab(item, true);
    } else {
        for (int i = m_tabInfos.size() - 1; i >= 0; i--) {
            const TabInfo &item = m_tabInfos.at(i);
            drawTab(item, false);
        }
    }
}

void SimpleTabBarItem::mousePressEvent(QMouseEvent *event)
{
    event->ignore();

    const QPointF pt = event->localPos();
    for (int i = 0; i < m_tabInfos.size(); i++) {
        const TabInfo &item = m_tabInfos.at(i);
        if (item.path.contains(pt)) {
            emit tabClicked(i);
            return;
        }
    }
}

QString to_roman(int value)
{
    struct romandata_t
    {
        int value;
        QString numeral;
    };
    static const struct romandata_t romandata[] = {
        { 1000, QStringLiteral("M") }, { 900, QStringLiteral("CM") },
        { 500, QStringLiteral("D") },  { 400, QStringLiteral("CD") },
        { 100, QStringLiteral("C") },  { 90, QStringLiteral("XC") },
        { 50, QStringLiteral("L") },   { 40, QStringLiteral("XL") },
        { 10, QStringLiteral("X") },   { 9, QStringLiteral("IX") },
        { 5, QStringLiteral("V") },    { 4, QStringLiteral("IV") },
        { 1, QStringLiteral("I") },    { 0, NULL } // end marker
    };

    QString result;
    for (const romandata_t *current = romandata; current->value > 0; ++current) {
        while (value >= current->value) {
            result += current->numeral;
            value -= current->value;
        }
    }

    return result;
}

QString to_alphabetical(int value)
{
    QString result;
    while (value > 0) {
        result = QChar((char)(65 + (value - 1) % 26)) + result;
        value = (value - 1) / 26;
    }

    return result;
}

QString to_english(int value)
{
    return QString::number(value);
}

void SimpleTabBarItem::updateTabInfos()
{
    QList<TabInfo> tabInfos;

    const qreal padding = 0.75 * m_tabCurveRadius;
    const qreal overlap = padding;

    auto evaluateTabLabel = [](int nr, int type) {
        switch (type) {
        case SimpleTabBarItem::Alphabets:
            return to_alphabetical(nr);
        case SimpleTabBarItem::RomanNumerals:
            return to_roman(nr);
        case SimpleTabBarItem::EnglishNumbers:
            return to_english(nr);
        default:
            break;
        }
        return to_english(nr);
    };

    QRectF boundingRect;

    auto requestAttribute = [=](int index, TabAttribute attr) {
        this->setRequestedAttributeValue(QVariant());
        emit attributeRequest(index, attr);
        return m_requestedAttributeValue;
    };

    for (int i = 0; i < m_tabCount; i++) {
        TabInfo tabInfo;
        tabInfo.label = evaluateTabLabel(i + 1, m_tabLabelStyle);
        tabInfo.bgColor = m_activeTabIndex == i ? m_activeTabColor : m_inactiveTabColor;
        tabInfo.borderColor =
                m_activeTabIndex == i ? m_activeTabBorderColor : m_inactiveTabBorderColor;
        tabInfo.textColor = m_activeTabIndex == i ? m_activeTabTextColor : m_inactiveTabTextColor;
        tabInfo.font = m_activeTabIndex == i ? m_activeTabFont : m_inactiveTabFont;

        if (requestAttribute(i, TabColor).isValid())
            tabInfo.bgColor = m_requestedAttributeValue.value<QColor>();
        if (requestAttribute(i, TabBorderColor).isValid())
            tabInfo.borderColor = m_requestedAttributeValue.value<QColor>();
        if (requestAttribute(i, TabTextColor).isValid())
            tabInfo.textColor = m_requestedAttributeValue.value<QColor>();
        if (requestAttribute(i, TabFont).isValid())
            tabInfo.font = m_requestedAttributeValue.value<QFont>();
        if (requestAttribute(i, TabLabel).isValid())
            tabInfo.label = m_requestedAttributeValue.value<QString>();
        this->setRequestedAttributeValue(QVariant());

        QFontMetricsF fontMetrics(tabInfo.font);

        QRectF tabRect = fontMetrics.boundingRect(tabInfo.label);

        const qreal hpadding = tabRect.width() * padding;
        const qreal vpadding = tabRect.height() * padding;
        tabRect.adjust(-hpadding, -vpadding, hpadding, vpadding);
        tabRect.moveBottom(m_activeTabIndex == i ? 0 : -m_activeTabBorderWidth - 0.5);

        tabRect.setWidth(qMax(tabRect.width(), m_minimumTabWidth));
        const qreal hoverlap = tabRect.width() * overlap;

        if (boundingRect.isValid())
            tabRect.moveLeft(boundingRect.right() - hoverlap);
        else
            tabRect.moveLeft(0);

        const qreal m =
                (m_activeTabIndex == i ? m_activeTabBorderWidth : m_inactiveTabBorderWidth) / 2.0;
        const QRectF pathRect = tabRect.adjusted(m, m, -m, 0);

        const QPointF p1 = pathRect.bottomLeft();
        const QPointF c2 = pathRect.topLeft() + QPointF(hoverlap, 0);
        const QPointF c3 = pathRect.topRight() - QPointF(hoverlap, 0);
        const QPointF p4 = pathRect.bottomRight();

        const QPointF p2a = QLineF(c2, p1).pointAt(m_tabCurveRadius);
        const QPointF p2b = QLineF(c2, c3).pointAt(m_tabCurveRadius);
        const QPointF p3a = QLineF(c3, c2).pointAt(m_tabCurveRadius);
        const QPointF p3b = QLineF(c3, p4).pointAt(m_tabCurveRadius);

        QPainterPath path;
        path.moveTo(p1);
        path.lineTo(p2a);
        path.quadTo(c2, p2b);
        path.lineTo(p3a);
        path.quadTo(c3, p3b);
        path.lineTo(p4);

        tabInfo.path = path;

        boundingRect |= tabRect;

        tabInfos.append(tabInfo);
    }

    const QPointF dp = -boundingRect.topLeft();
    QTransform tx;
    tx.translate(dp.x(), dp.y());

    for (TabInfo &item : tabInfos)
        item.path = tx.map(item.path);

    this->setWidth(boundingRect.width());
    this->setHeight(boundingRect.height());

    m_tabInfos = tabInfos;
    emit tabPathsUpdated();

    this->update();
}

void SimpleTabBarItem::updateTabInfosLater()
{
    m_updateTabInfosTimer.start();
}
