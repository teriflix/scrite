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

#ifndef TEXTSHAPEITEM_H
#define TEXTSHAPEITEM_H

#include "abstractshapeitem.h"

class TextShapeItem : public AbstractShapeItem
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit TextShapeItem(QQuickItem *parent = nullptr);
    ~TextShapeItem();

    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    void setText(const QString &val);
    QString text() const { return m_text; }
    Q_SIGNAL void textChanged();

    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged)
    void setFont(const QFont &val);
    QFont font() const { return m_font; }
    Q_SIGNAL void fontChanged();

protected:
    QPainterPath shape() const;

private:
    QFont m_font;
    QString m_text;
};

#endif // TEXTSHAPEITEM_H
