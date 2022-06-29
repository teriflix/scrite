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

#include "ruleritem.h"

#include <QPainter>
#include <QQuickWindow>
#include <QScreen>

RulerItem::RulerItem(QQuickItem *parent) : QQuickPaintedItem(parent) { }

RulerItem::~RulerItem() { }

void RulerItem::setDisplayUnit(RulerItem::Unit val)
{
    if (m_displayUnit == val)
        return;

    m_displayUnit = val;
    emit displayUnitChanged();

    this->update();
}

void RulerItem::setFont(const QFont &val)
{
    if (m_font == val)
        return;

    m_font = val;
    emit fontChanged();

    this->update();
}

void RulerItem::setMarginsUnit(RulerItem::Unit val)
{
    if (m_marginsUnit == val)
        return;

    m_marginsUnit = val;
    emit marginsUnitChanged();

    this->update();
}

void RulerItem::setLeftMargin(qreal val)
{
    if (qFuzzyCompare(m_leftMargin, val))
        return;

    m_leftMargin = val;
    emit leftMarginChanged();

    this->update();
}

void RulerItem::setRightMargin(qreal val)
{
    if (qFuzzyCompare(m_rightMargin, val))
        return;

    m_rightMargin = val;
    emit rightMarginChanged();

    this->update();
}

void RulerItem::setParagraphLeftMargin(qreal val)
{
    if (qFuzzyCompare(m_paragraphLeftMargin, val))
        return;

    m_paragraphLeftMargin = val;
    emit paragraphLeftMarginChanged();

    this->update();
}

void RulerItem::setParagraphRightMargin(qreal val)
{
    if (qFuzzyCompare(m_paragraphRightMargin, val))
        return;

    m_paragraphRightMargin = val;
    emit paragraphRightMarginChanged();

    this->update();
}

void RulerItem::setPageMarginColor(const QColor &val)
{
    if (m_pageMarginColor == val)
        return;

    m_pageMarginColor = val;
    emit pageMarginColorChanged();

    this->update();
}

void RulerItem::setParagraphColor(const QColor &val)
{
    if (m_paragraphColor == val)
        return;

    m_paragraphColor = val;
    emit paragraphColorChanged();

    this->update();
}

void RulerItem::setBackgroundColor(const QColor &val)
{
    if (m_backgroundColor == val)
        return;

    m_backgroundColor = val;
    emit backgroundColorChanged();

    this->update();
}

void RulerItem::setBorderColor(const QColor &val)
{
    if (m_borderColor == val)
        return;

    m_borderColor = val;
    emit borderColorChanged();

    this->update();
}

void RulerItem::setMajorTickColor(const QColor &val)
{
    if (m_majorTickColor == val)
        return;

    m_majorTickColor = val;
    emit majorTickColorChanged();

    this->update();
}

void RulerItem::setMinorTickColor(const QColor &val)
{
    if (m_minorTickColor == val)
        return;

    m_minorTickColor = val;
    emit minorTickColorChanged();

    this->update();
}

void RulerItem::setTextColor(const QColor &val)
{
    if (m_textColor == val)
        return;

    m_textColor = val;
    emit textColorChanged();

    this->update();
}

void RulerItem::setZoomLevel(qreal val)
{
    if (qFuzzyCompare(m_zoomLevel, val))
        return;

    m_zoomLevel = val;
    emit zoomLevelChanged();

    this->update();
}

void RulerItem::setResolution(qreal val)
{
    if (m_resolution == val)
        return;

    m_resolution = val;
    emit resolutionChanged();

    this->update();
}

qreal RulerItem::convert(qreal val, RulerItem::Unit from, RulerItem::Unit to) const
{
    const QScreen *screen = this->window()->screen();
    const qreal pixelsPerIn = screen->physicalDotsPerInchX();
    return RulerItem::Convert(val, from, to, pixelsPerIn);
}

qreal RulerItem::Convert(qreal val, RulerItem::Unit from, RulerItem::Unit to,
                         const qreal pixelsPerIn)
{
    if (from == to)
        return val;

    const qreal cmsPerIn = 2.54;
    const qreal pixelsPerCm = pixelsPerIn / cmsPerIn;

    auto unitToPixel = [=](qreal val, Unit unit) {
        switch (unit) {
        case Inch:
            return val * pixelsPerIn;
        case Centimeter:
            return val * pixelsPerCm;
        default:
            break;
        }
        return val;
    };

    auto pixelToUnit = [=](qreal val, Unit unit) {
        switch (unit) {
        case Inch:
            return val / pixelsPerIn;
        case Centimeter:
            return val / pixelsPerCm;
        default:
            break;
        }
        return val;
    };

    return pixelToUnit(unitToPixel(val, from), to);
}

void RulerItem::paint(QPainter *painter)
{
#ifndef QT_NO_DEBUG_OUTPUT
    qDebug("RulerItem is painting");
#endif

    const QRectF rect(0, 0, this->width(), this->height());
    if (rect.isEmpty())
        return;

    const QScreen *screen = this->window()->screen();
    const qreal pixelsPerIn = qFuzzyIsNull(m_resolution)
            ? screen->physicalDotsPerInchX()
            : (m_resolution * screen->devicePixelRatio());
    const qreal cmsPerIn = 2.54;
    const qreal pixelsPerCm = pixelsPerIn / cmsPerIn;
    const QFontMetricsF fm(m_font);

    auto unitToPixel = [=](qreal val, Unit unit) {
        switch (unit) {
        case Inch:
            return val * pixelsPerIn * m_zoomLevel;
        case Centimeter:
            return val * pixelsPerCm * m_zoomLevel;
        default:
            break;
        }
        return val * m_zoomLevel;
    };

    // This is unnecessary, since painter initializes itself with NoBrush & NoPen
    painter->setBrush(Qt::NoBrush);
    painter->setPen(Qt::NoPen);
    painter->setFont(m_font);

    // Render background
    painter->fillRect(rect, m_backgroundColor);

    // Render margins
    if (m_leftMargin > 0) {
        QRectF leftMarginRect = rect;
        leftMarginRect.setRight(unitToPixel(m_leftMargin, m_marginsUnit));
        painter->fillRect(leftMarginRect, m_pageMarginColor);
    }

    if (m_rightMargin > 0) {
        QRectF rightMarginRect = rect;
        rightMarginRect.setLeft(rect.right() - unitToPixel(m_rightMargin, m_marginsUnit));
        painter->fillRect(rightMarginRect, m_pageMarginColor);
    }

    if (m_paragraphLeftMargin > m_leftMargin && m_paragraphRightMargin > m_rightMargin) {
        QRectF paragraphMarginRect = rect;
        paragraphMarginRect.setLeft(unitToPixel(m_paragraphLeftMargin, m_marginsUnit));
        paragraphMarginRect.setRight(rect.right()
                                     - unitToPixel(m_paragraphRightMargin, m_marginsUnit));
        painter->fillRect(paragraphMarginRect, m_paragraphColor);
    }

    // Render ticks

    // For how many display units should we display a minor tick?
    const qreal minorTick = (m_displayUnit == Pixels) ? 20 : ((m_displayUnit == Inch) ? 0.1 : 0.5);
    const qreal minorTickPx = unitToPixel(minorTick, m_displayUnit);
    // For how many minor ticks should we major tick?
    const int majorTick = (m_displayUnit == Pixels) ? 5 : ((m_displayUnit == Inch) ? 5 : 2);
    // For how many minor ticks should we display label?
    const int labelTick = (m_displayUnit == Pixels) ? 10 : ((m_displayUnit == Inch) ? 10 : 4);

    const qreal verticalCenter = rect.center().y();
    const qreal majorTickHeight = fm.lineSpacing() * 0.8;
    const qreal minorTickHeight = majorTickHeight * 0.5;
    const qreal minorY1 = verticalCenter - minorTickHeight / 2;
    const qreal minorY2 = minorY1 + minorTickHeight;
    const qreal majorY1 = verticalCenter - majorTickHeight / 2;
    const qreal majorY2 = majorY1 + majorTickHeight;

    qreal x = 0;
    int nrMinorTicks = 0;
    while (x < rect.right()) {
        x += minorTickPx;
        ++nrMinorTicks;

        if (nrMinorTicks % labelTick == 0) {
            // Draw label
            const int value = int(nrMinorTicks * minorTick);
            const QString label = QString::number(value);
            QRectF labelRect = fm.boundingRect(label);
            labelRect.moveCenter(QPointF(x, verticalCenter));
            painter->setPen(m_textColor);
            painter->drawText(labelRect, Qt::AlignCenter, label);
        } else if (nrMinorTicks % majorTick == 0) {
            painter->setPen(m_majorTickColor);
            painter->drawLine(QLineF(x, majorY1, x, majorY2));
        } else {
            painter->setPen(m_minorTickColor);
            painter->drawLine(QLineF(x, minorY1, x, minorY2));
        }
    }

    // Finally render border
    painter->setBrush(Qt::NoBrush);
    painter->setPen(m_borderColor);
    painter->drawRect(rect);

    // From now on, conversion is possible.
    this->setCanConvert(true);
}

void RulerItem::setCanConvert(bool val)
{
    if (m_canConvert == val)
        return;

    m_canConvert = val;
    emit canConvertChanged();
}
