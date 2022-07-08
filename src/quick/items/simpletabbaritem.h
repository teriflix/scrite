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

#ifndef SIMPLETABBARITEM_H
#define SIMPLETABBARITEM_H

#include <QTimer>
#include <QPointer>
#include <QPainterPath>
#include <QQuickPaintedItem>
#include <QAbstractListModel>
#include <QJSValue>

class SimpleTabBarItem : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit SimpleTabBarItem(QQuickItem *parent = nullptr);
    ~SimpleTabBarItem();

    Q_PROPERTY(int tabCount READ tabCount WRITE setTabCount NOTIFY tabCountChanged)
    void setTabCount(int val);
    int tabCount() const { return m_tabCount; }
    Q_SIGNAL void tabCountChanged();

    Q_PROPERTY(int activeTabIndex READ activeTabIndex WRITE setActiveTabIndex NOTIFY activeTabIndexChanged)
    void setActiveTabIndex(int val);
    int activeTabIndex() const { return m_activeTabIndex; }
    Q_SIGNAL void activeTabIndexChanged();

    enum TabLabelStyle { EnglishNumbers, RomanNumerals, Alphabets };
    Q_ENUM(TabLabelStyle)
    Q_PROPERTY(TabLabelStyle tabLabelStyle READ tabLabelStyle WRITE setTabLabelStyle NOTIFY tabLabelStyleChanged)
    void setTabLabelStyle(TabLabelStyle val);
    TabLabelStyle tabLabelStyle() const { return m_tabLabelStyle; }
    Q_SIGNAL void tabLabelStyleChanged();

    Q_PROPERTY(QColor activeTabColor READ activeTabColor WRITE setActiveTabColor NOTIFY activeTabColorChanged)
    void setActiveTabColor(const QColor &val);
    QColor activeTabColor() const { return m_activeTabColor; }
    Q_SIGNAL void activeTabColorChanged();

    Q_PROPERTY(QColor inactiveTabColor READ inactiveTabColor WRITE setInactiveTabColor NOTIFY inactiveTabColorChanged)
    void setInactiveTabColor(const QColor &val);
    QColor inactiveTabColor() const { return m_inactiveTabColor; }
    Q_SIGNAL void inactiveTabColorChanged();

    Q_PROPERTY(QColor activeTabBorderColor READ activeTabBorderColor WRITE setActiveTabBorderColor NOTIFY activeTabBorderColorChanged)
    void setActiveTabBorderColor(const QColor &val);
    QColor activeTabBorderColor() const { return m_activeTabBorderColor; }
    Q_SIGNAL void activeTabBorderColorChanged();

    Q_PROPERTY(QColor inactiveTabBorderColor READ inactiveTabBorderColor WRITE setInactiveTabBorderColor NOTIFY inactiveTabBorderColorChanged)
    void setInactiveTabBorderColor(const QColor &val);
    QColor inactiveTabBorderColor() const { return m_inactiveTabBorderColor; }
    Q_SIGNAL void inactiveTabBorderColorChanged();

    Q_PROPERTY(qreal activeTabBorderWidth READ activeTabBorderWidth WRITE setActiveTabBorderWidth NOTIFY activeTabBorderWidthChanged)
    void setActiveTabBorderWidth(qreal val);
    qreal activeTabBorderWidth() const { return m_activeTabBorderWidth; }
    Q_SIGNAL void activeTabBorderWidthChanged();

    Q_PROPERTY(qreal inactiveTabBorderWidth READ inactiveTabBorderWidth WRITE setInactiveTabBorderWidth NOTIFY inactiveTabBorderWidthChanged)
    void setInactiveTabBorderWidth(qreal val);
    qreal inactiveTabBorderWidth() const { return m_inactiveTabBorderWidth; }
    Q_SIGNAL void inactiveTabBorderWidthChanged();

    Q_PROPERTY(qreal tabCurveRadius READ tabCurveRadius WRITE setTabCurveRadius NOTIFY tabCurveRadiusChanged)
    void setTabCurveRadius(qreal val);
    qreal tabCurveRadius() const { return m_tabCurveRadius; }
    Q_SIGNAL void tabCurveRadiusChanged();

    Q_PROPERTY(QColor activeTabTextColor READ activeTabTextColor WRITE setActiveTabTextColor NOTIFY activeTabTextColorChanged)
    void setActiveTabTextColor(const QColor &val);
    QColor activeTabTextColor() const { return m_activeTabTextColor; }
    Q_SIGNAL void activeTabTextColorChanged();

    Q_PROPERTY(QColor inactiveTabTextColor READ inactiveTabTextColor WRITE setInactiveTabTextColor NOTIFY inactiveTabTextColorChanged)
    void setInactiveTabTextColor(const QColor &val);
    QColor inactiveTabTextColor() const { return m_inactiveTabTextColor; }
    Q_SIGNAL void inactiveTabTextColorChanged();

    Q_PROPERTY(QFont activeTabFont READ activeTabFont WRITE setActiveTabFont NOTIFY activeTabFontChanged)
    void setActiveTabFont(const QFont &val);
    QFont activeTabFont() const { return m_activeTabFont; }
    Q_SIGNAL void activeTabFontChanged();

    Q_PROPERTY(QFont inactiveTabFont READ inactiveTabFont WRITE setInactiveTabFont NOTIFY inactiveTabFontChanged)
    void setInactiveTabFont(const QFont &val);
    QFont inactiveTabFont() const { return m_inactiveTabFont; }
    Q_SIGNAL void inactiveTabFontChanged();

    Q_PROPERTY(qreal minimumTabWidth READ minimumTabWidth WRITE setMinimumTabWidth NOTIFY minimumTabWidthChanged)
    void setMinimumTabWidth(qreal val);
    qreal minimumTabWidth() const { return m_minimumTabWidth; }
    Q_SIGNAL void minimumTabWidthChanged();

    enum TabAttribute { TabColor, TabBorderColor, TabTextColor, TabFont, TabLabel };
    Q_ENUM(TabAttribute)
    Q_SIGNAL void attributeRequest(int index, TabAttribute attr);

    Q_PROPERTY(QVariant requestedAttributeValue READ requestedAttributeValue WRITE setRequestedAttributeValue NOTIFY requestedAttributeValueChanged)
    void setRequestedAttributeValue(const QVariant &val);
    QVariant requestedAttributeValue() const { return m_requestedAttributeValue; }
    Q_SIGNAL void requestedAttributeValueChanged();

    Q_INVOKABLE QRectF tabRect(int index) const;
    Q_INVOKABLE void updateTabAttributes() { this->updateTabInfosLater(); }

    Q_SIGNAL void tabClicked(int index);
    Q_SIGNAL void tabPathsUpdated();

    // QQuickPaintedItem interface
    void paint(QPainter *painter);

protected:
    // QQuickItem interface
    void mousePressEvent(QMouseEvent *event);

private:
    void updateTabInfos();
    void updateTabInfosLater();

private:
    int m_tabCount = 0;
    int m_activeTabIndex = -1;
    qreal m_minimumTabWidth = 20;
    TabLabelStyle m_tabLabelStyle = RomanNumerals;
    QColor m_activeTabColor = Qt::white;
    QColor m_inactiveTabColor = Qt::lightGray;
    QColor m_activeTabBorderColor = Qt::black;
    QColor m_inactiveTabBorderColor = Qt::darkGray;
    QColor m_activeTabTextColor = Qt::black;
    QColor m_inactiveTabTextColor = Qt::black;
    QFont m_activeTabFont;
    QFont m_inactiveTabFont;
    qreal m_tabCurveRadius = 0.2;
    qreal m_activeTabBorderWidth = 1;
    qreal m_inactiveTabBorderWidth = 1;
    QTimer m_updateTabInfosTimer;
    QVariant m_requestedAttributeValue;

    struct TabInfo
    {
        QString label;

        QColor bgColor;
        QColor borderColor;
        QColor textColor;
        QFont font;

        QPainterPath path;
    };
    QList<TabInfo> m_tabInfos;
};

#endif // SIMPLETABBARITEM_H
