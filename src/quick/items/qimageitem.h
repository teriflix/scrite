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

#ifndef QIMAGEITEM_H
#define QIMAGEITEM_H

#include <QIcon>
#include <QImage>
#include <QQuickItem>

class QImageItem : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT

public:
    QImageItem(QQuickItem *parentItem = nullptr);
    ~QImageItem();

    Q_INVOKABLE static QImage fromIcon(const QIcon &icon, const QSize &size);

    Q_PROPERTY(QImage image READ image WRITE setImage NOTIFY imageChanged)
    void setImage(const QImage &val);
    QImage image() const { return m_image; }
    Q_SIGNAL void imageChanged();

protected:
    // QQuickItem interface
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *);

private:
    QImage m_image;
};

#endif // QIMAGEITEM_H
