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

#ifndef PDFEXPORTABLEGRAPHICSSCENE_H
#define PDFEXPORTABLEGRAPHICSSCENE_H

#include <QGraphicsItem>
#include <QGraphicsScene>

#include "qtextdocumentpagedprinter.h"

class QPdfWriter;

class PdfExportableGraphicsScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit PdfExportableGraphicsScene(QObject *parent = nullptr);
    ~PdfExportableGraphicsScene();

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    void setTitle(QString val);
    QString title() const { return m_title; }
    Q_SIGNAL void titleChanged();

    Q_PROPERTY(QString comment READ comment WRITE setComment NOTIFY commentChanged)
    void setComment(const QString &val);
    QString comment() const { return m_comment; }
    Q_SIGNAL void commentChanged();

    Q_PROPERTY(QString watermark READ watermark WRITE setWatermark NOTIFY watermarkChanged)
    void setWatermark(const QString &val);
    QString watermark() const { return m_watermark; }
    Q_SIGNAL void watermarkChanged();

    enum StandardItems {
        HeaderLayer = 1,
        FooterLayer = 2,
        HeaderFooterLayer = 3,
        WatermarkUnderlayLayer = 4,
        WatermarkOverlayLayer = 8,
        HeaderFooterAndWatermarkUnderlay = 7,
        DontIncludeScriteLink = 16
    };
    void addStandardItems(int items = HeaderFooterAndWatermarkUnderlay);

    bool exportToPdf(const QString &fileName);
    bool exportToPdf(QIODevice *device);
    bool exportToPdf(QPdfWriter *pdfWriter);

protected:
private:
    QString m_title;
    QString m_comment;
    QString m_watermark;
};

class GraphicsHeaderFooterItem : public QGraphicsItem
{
public:
    explicit GraphicsHeaderFooterItem(HeaderFooter *headerFooter,
                                      const QMap<HeaderFooter::Field, QString> &fields);
    ~GraphicsHeaderFooterItem();

    void setRect(const QRectF &rect);
    QRectF rect() const { return m_rect; }

    // QGraphicsItem interface
    QRectF boundingRect() const { return m_rect; }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    QRectF m_rect;
    HeaderFooter *m_headerFooter = nullptr;
    QMap<HeaderFooter::Field, QString> m_fields;
};

class GraphicsWatermarkItem : public QGraphicsItem
{
public:
    explicit GraphicsWatermarkItem(Watermark *watermark);
    ~GraphicsWatermarkItem();

    void setRect(const QRectF &rect);
    QRectF rect() const { return m_rect; }

    // QGraphicsItem interface
    QRectF boundingRect() const { return m_rect; }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    QRectF m_rect;
    Watermark *m_watermark = nullptr;
};

class GraphicsHeaderItem : public QGraphicsRectItem
{
public:
    explicit GraphicsHeaderItem(const QString &title, const QString &subtitle,
                                qreal containerWidth);
    ~GraphicsHeaderItem();

    static qreal idealContainerWidth(const QString &title);
};

class GraphicsImageRectItem : public QGraphicsRectItem
{
public:
    explicit GraphicsImageRectItem(QGraphicsItem *parent = nullptr);
    ~GraphicsImageRectItem();

    enum FillMode { Stretch, PreserveAspectFit, PreserveAspectCrop };

    void setFillMode(FillMode val);
    FillMode fillMode() const { return m_fillMode; }

    void setImage(const QImage &image) { m_image = image; }
    QImage image() const { return m_image; }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    QImage m_image;
    FillMode m_fillMode = PreserveAspectCrop;
};

#endif // PDFEXPORTABLEGRAPHICSSCENE_H
