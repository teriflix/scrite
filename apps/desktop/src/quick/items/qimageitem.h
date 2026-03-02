/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
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
#include <QQuickImageProvider>
#include <QQuickPaintedItem>

class QImageItem : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT

public:
    QImageItem(QQuickItem *parentItem = nullptr);
    ~QImageItem();

    // clang-format off
    Q_PROPERTY(bool useSoftwareRenderer
               READ useSoftwareRenderer
               WRITE setUseSoftwareRenderer
               NOTIFY useSoftwareRendererChanged)
    // clang-format on
    void setUseSoftwareRenderer(bool val);
    bool useSoftwareRenderer() const { return m_useSoftwareRenderer; }
    Q_SIGNAL void useSoftwareRendererChanged();

    enum FillMode { Stretch, PreserveAspectFit, PreserveAspectCrop };
    Q_ENUM(FillMode)

    // clang-format off
    Q_PROPERTY(FillMode fillMode
               READ fillMode
               WRITE setFillMode
               NOTIFY fillModeChanged)
    // clang-format on
    void setFillMode(FillMode val);
    FillMode fillMode() const { return m_fillMode; }
    Q_SIGNAL void fillModeChanged();

    Q_INVOKABLE static QImage fromIcon(const QIcon &icon, const QSize &size);

    // clang-format off
    Q_PROPERTY(QImage image
               READ image
               WRITE setImage
               NOTIFY imageChanged)
    // clang-format on
    void setImage(const QImage &val);
    QImage image() const { return m_image; }
    Q_SIGNAL void imageChanged();

    // clang-format off
    Q_PROPERTY(bool imageIsEmpty
               READ imageIsEmpty
               NOTIFY imageChanged)
    // clang-format on
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

class ImageIcon : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_ATTACHED(ImageIcon)

public:
    virtual ~ImageIcon();

    static ImageIcon *qmlAttachedProperties(QObject *parent);

    // clang-format off
    Q_PROPERTY(QUrl url
               READ url
               NOTIFY imageChanged)
    // clang-format on
    QUrl url() const;

    // clang-format off
    Q_PROPERTY(QImage image
               READ image
               WRITE setImage
               NOTIFY imageChanged)
    // clang-format on
    void setImage(const QImage &val);
    QImage image() const { return m_image; }
    Q_SIGNAL void imageChanged();

    QString imageId() const { return m_imageId; }

protected:
    explicit ImageIcon(QObject *parent = nullptr);

private:
    QImage m_image;
    QString m_imageId;
};

class ImageIconProvider : public QQuickImageProvider
{
public:
    explicit ImageIconProvider();
    ~ImageIconProvider();

    static QString name();

    // QQuickImageProvider interface
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);
};

#endif // QIMAGEITEM_H
