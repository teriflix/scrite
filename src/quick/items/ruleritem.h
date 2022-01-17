/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef RULERITEM_H
#define RULERITEM_H

#include <QQuickPaintedItem>

class RulerItem : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT

public:
    RulerItem(QQuickItem *parent = nullptr);
    ~RulerItem();

    enum Unit { Inch, Centimeter, Pixels };
    Q_ENUM(Unit)
    Q_PROPERTY(Unit displayUnit READ displayUnit WRITE setDisplayUnit NOTIFY displayUnitChanged)
    void setDisplayUnit(Unit val);
    Unit displayUnit() const { return m_displayUnit; }
    Q_SIGNAL void displayUnitChanged();

    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged)
    void setFont(const QFont &val);
    QFont font() const { return m_font; }
    Q_SIGNAL void fontChanged();

    Q_PROPERTY(Unit marginsUnit READ marginsUnit WRITE setMarginsUnit NOTIFY marginsUnitChanged)
    void setMarginsUnit(Unit val);
    Unit marginsUnit() const { return m_marginsUnit; }
    Q_SIGNAL void marginsUnitChanged();

    Q_PROPERTY(qreal leftMargin READ leftMargin WRITE setLeftMargin NOTIFY leftMarginChanged)
    void setLeftMargin(qreal val);
    qreal leftMargin() const { return m_leftMargin; }
    Q_SIGNAL void leftMarginChanged();

    Q_PROPERTY(qreal rightMargin READ rightMargin WRITE setRightMargin NOTIFY rightMarginChanged)
    void setRightMargin(qreal val);
    qreal rightMargin() const { return m_rightMargin; }
    Q_SIGNAL void rightMarginChanged();

    Q_PROPERTY(qreal paragraphLeftMargin READ paragraphLeftMargin WRITE setParagraphLeftMargin NOTIFY paragraphLeftMarginChanged)
    void setParagraphLeftMargin(qreal val);
    qreal paragraphLeftMargin() const { return m_paragraphLeftMargin; }
    Q_SIGNAL void paragraphLeftMarginChanged();

    Q_PROPERTY(qreal paragraphRightMargin READ paragraphRightMargin WRITE setParagraphRightMargin NOTIFY paragraphRightMarginChanged)
    void setParagraphRightMargin(qreal val);
    qreal paragraphRightMargin() const { return m_paragraphRightMargin; }
    Q_SIGNAL void paragraphRightMarginChanged();

    Q_PROPERTY(QColor pageMarginColor READ pageMarginColor WRITE setPageMarginColor NOTIFY pageMarginColorChanged)
    void setPageMarginColor(const QColor &val);
    QColor pageMarginColor() const { return m_pageMarginColor; }
    Q_SIGNAL void pageMarginColorChanged();

    Q_PROPERTY(QColor paragraphColor READ paragraphColor WRITE setParagraphColor NOTIFY paragraphColorChanged)
    void setParagraphColor(const QColor &val);
    QColor paragraphColor() const { return m_paragraphColor; }
    Q_SIGNAL void paragraphColorChanged();

    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    void setBackgroundColor(const QColor &val);
    QColor backgroundColor() const { return m_backgroundColor; }
    Q_SIGNAL void backgroundColorChanged();

    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)
    void setBorderColor(const QColor &val);
    QColor borderColor() const { return m_borderColor; }
    Q_SIGNAL void borderColorChanged();

    Q_PROPERTY(QColor majorTickColor READ majorTickColor WRITE setMajorTickColor NOTIFY majorTickColorChanged)
    void setMajorTickColor(const QColor &val);
    QColor majorTickColor() const { return m_majorTickColor; }
    Q_SIGNAL void majorTickColorChanged();

    Q_PROPERTY(QColor minorTickColor READ minorTickColor WRITE setMinorTickColor NOTIFY minorTickColorChanged)
    void setMinorTickColor(const QColor &val);
    QColor minorTickColor() const { return m_minorTickColor; }
    Q_SIGNAL void minorTickColorChanged();

    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    void setTextColor(const QColor &val);
    QColor textColor() const { return m_textColor; }
    Q_SIGNAL void textColorChanged();

    Q_PROPERTY(qreal zoomLevel READ zoomLevel WRITE setZoomLevel NOTIFY zoomLevelChanged)
    void setZoomLevel(qreal val);
    qreal zoomLevel() const { return m_zoomLevel; }
    Q_SIGNAL void zoomLevelChanged();

    Q_PROPERTY(bool canConvert READ canConvert NOTIFY canConvertChanged)
    bool canConvert() const { return m_canConvert; }
    Q_SIGNAL void canConvertChanged();

    Q_PROPERTY(qreal resolution READ resolution WRITE setResolution NOTIFY resolutionChanged)
    void setResolution(qreal val);
    qreal resolution() const { return m_resolution; }
    Q_SIGNAL void resolutionChanged();

    Q_INVOKABLE qreal convert(qreal val, RulerItem::Unit from, RulerItem::Unit to) const;
    static qreal Convert(qreal val, Unit from, Unit to, const qreal pixelsPerIn);

protected:
    // QQuickPaintedItem interface
    void paint(QPainter *painter);

private:
    void setCanConvert(bool val);

private:
    QFont m_font;
    QColor m_textColor = Qt::black;
    qreal m_zoomLevel = 1.0;
    qreal m_leftMargin = 0;
    qreal m_resolution = 0;
    bool m_canConvert = false;
    qreal m_rightMargin = 0;
    QColor m_borderColor = Qt::gray;
    Unit m_displayUnit = Inch;
    Unit m_marginsUnit = Pixels;
    QColor m_majorTickColor = Qt::black;
    QColor m_minorTickColor = Qt::darkGray;
    QColor m_paragraphColor = Qt::lightGray;
    QColor m_pageMarginColor = Qt::gray;
    QColor m_backgroundColor = Qt::white;
    qreal m_paragraphRightMargin = 0;
    qreal m_paragraphLeftMargin = 0;
};

#endif // RULERITEM_H
