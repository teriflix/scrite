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
#include <QQuickPaintedItem>

class QImageItem : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT

public:
    QImageItem(QQuickItem *parentItem = nullptr);
    ~QImageItem();

    Q_PROPERTY(bool useSoftwareRenderer READ useSoftwareRenderer WRITE setUseSoftwareRenderer NOTIFY
                       useSoftwareRendererChanged)
    void setUseSoftwareRenderer(bool val);
    bool useSoftwareRenderer() const { return m_useSoftwareRenderer; }
    Q_SIGNAL void useSoftwareRendererChanged();

    enum FillMode { Stretch, PreserveAspectFit, PreserveAspectCrop };
    Q_ENUM(FillMode)

    Q_PROPERTY(FillMode fillMode READ fillMode WRITE setFillMode NOTIFY fillModeChanged)
    void setFillMode(FillMode val);
    FillMode fillMode() const { return m_fillMode; }
    Q_SIGNAL void fillModeChanged();

    Q_INVOKABLE static QImage fromIcon(const QIcon &icon, const QSize &size);

    Q_PROPERTY(QImage image READ image WRITE setImage NOTIFY imageChanged)
    void setImage(const QImage &val);
    QImage image() const { return m_image; }
    Q_SIGNAL void imageChanged();

    Q_PROPERTY(bool imageIsEmpty READ imageIsEmpty NOTIFY imageChanged)
    bool imageIsEmpty() const { return m_image.isNull() || m_image.size().isEmpty(); }

protected:
    // QQuickPaintedItem interface
    void paint(QPainter *painter);

    // QQuickItem interface
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *);

private:
    QImage m_image;
    FillMode m_fillMode = PreserveAspectFit;
    bool m_useSoftwareRenderer = false;
    enum {
        UnknownPaintMode,
        SceneGraphPaintMode,
        PainterPaintMode
    } lastPaintMode = UnknownPaintMode;
};

#endif // QIMAGEITEM_H
