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

#include "imageprinter.h"

#include <QDir>
#include <QtDebug>
#include <QDateTime>
#include <QQmlEngine>
#include <QPainterPath>
#include <QPaintEngine>

class ImagePrinterImageProvider : public QObject, public QQuickImageProvider
{
public:
    ImagePrinterImageProvider();
    ~ImagePrinterImageProvider();

    static QString urlNamespace();

    void add(ImagePrinter *printer);
    void remove(ImagePrinter *printer);
    ImagePrinter *find(const QString &name);

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);

private:
    QReadWriteLock m_printersLock;
    QSet<ImagePrinter*> m_printers;
};

class ImagePrinterEngine : public QPaintEngine
{
public:
    ImagePrinterEngine();
    ~ImagePrinterEngine();

    QImage currentPageImage() const { return m_pageImage; }
    QImage printedPageImage() const { return m_printedPageImage; }

    // Creates a brand new page image. All painting
    // will happen on this page from now on.
    // Returns the page that was just painted on.
    void newPage();

public:
    // QPaintEngine interface
    bool begin(QPaintDevice *pdev);
    bool end();

    // We redirect everything related to "actual painting" to the
    // underlying page images paint-image.
    QPaintEngine *pageImageEngine() const {
        return m_pagePainter ? m_pagePainter->paintEngine() : m_pageImage.paintEngine();
    }
    void updateState(const QPaintEngineState &pestate);
    void drawRects(const QRect *rects, int rectCount);
    void drawRects(const QRectF *rects, int rectCount);
    void drawLines(const QLine *lines, int lineCount);
    void drawLines(const QLineF *lines, int lineCount);
    void drawEllipse(const QRectF &r);
    void drawEllipse(const QRect &r);
    void drawPath(const QPainterPath &path);
    void drawPoints(const QPointF *points, int pointCount);
    void drawPoints(const QPoint *points, int pointCount);
    void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);
    void drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode);
    void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr);
    void drawTextItem(const QPointF &p, const QTextItem &textItem);
    void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s);
    void drawImage(const QRectF &r, const QImage &pm, const QRectF &sr, Qt::ImageConversionFlags flags);
    QPoint coordinateOffset() const { return this->pageImageEngine()->coordinateOffset(); }
    Type type() const { return this->pageImageEngine()->type(); }

private:
    void paintHeaderFooterWatermark();
    void savePageImage();

private:
    QSize m_pageSize = QSize(816, 1056); // US Letter Page Size @ 96dpi
    int m_pageNumber = 0;
    char m_padding1[4];
    qreal m_pageScale = 1.0;
    QImage m_pageImage = QImage(30, 30, QImage::Format_ARGB32);
    QImage m_printedPageImage;
    QColor m_pageColor = Qt::white;
    QString m_directory;
    QTransform m_pageTransform;
    QString m_baseFileName = QStringLiteral("pageImage");
    QByteArray m_fileFormat = QByteArrayLiteral("PNG");
    QImage::Format m_imageFormat = QImage::Format_ARGB32;
    char m_padding2[4];
    QPainter *m_pagePainter = nullptr;
    ImagePrinter * m_currentDevice = nullptr;

    QRectF m_headerRect;
    QRectF m_footerRect;
    QRectF m_watermarkRect;
};

///////////////////////////////////////////////////////////////////////////////

// Yes QPagedPaintDevice is depricated, but there is no way to
// call the non-depricated constructor. So, we will have to suck
// up this compiler warning for now.
ImagePrinter::ImagePrinter(QObject *parent)
    : QAbstractListModel(parent)
{    
    this->setPageSize(Letter);
    this->setPageMargins(QMarginsF(0.2,0.1,0.2,0.1), QPageLayout::Inch);

    this->setObjectName( QStringLiteral("imagePrinter") );
    connect(this, &QObject::objectNameChanged, this, &ImagePrinter::pagesChanged); // because all pageUrls will change
}

ImagePrinter::~ImagePrinter()
{
    emit aboutToDelete(this);

    m_pageImages.clear();
    emit pagesChanged();

    if(m_engine)
        delete m_engine;
    m_engine = nullptr;
}

void ImagePrinter::setDirectory(const QString &val)
{
    if(m_directory == val)
        return;

    m_directory = val;
    emit directoryChanged();
}

void ImagePrinter::setImageFormat(ImagePrinter::ImageFormat val)
{
    if(m_imageFormat == val)
        return;

    m_imageFormat = val;
    emit imageFormatChanged();
}

void ImagePrinter::setScale(qreal val)
{
    if( qFuzzyCompare(m_scale, val) )
        return;

    m_scale = val;
    emit scaleChanged();
}

QImage ImagePrinter::pageImageAt(int index)
{
    if(index < 0 || index >= m_pageImages.size())
        return QImage();

    QReadLocker lock(&m_pageImagesLock);
    return m_pageImages.at(index);
}

QString ImagePrinter::pageUrl(int index) const
{
    return "image://" + ImagePrinterImageProvider::urlNamespace() + "/" +
                        this->objectName() + "/" +
                        QString::number(this->Modifiable::modificationTime()) + "/" +
            QString::number(index);
}

void ImagePrinter::clear()
{
    if(m_printing)
        return;

    this->beginResetModel();
    m_pageImages.clear();
    this->endResetModel();
}

void ImagePrinter::setPrinting(bool val)
{
    if(m_printing == val)
        return;

    m_printing = val;
    emit printingChanged();

    if(!val)
        this->Modifiable::markAsModified();
}

int ImagePrinter::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_pageImages.size();
}

QVariant ImagePrinter::data(const QModelIndex &index, int role) const
{
    if(index.row() < 0 || index.row() >= m_pageImages.size())
        return QVariant();

    switch(role)
    {
    case PageUrlRole: return this->pageUrl(index.row());
    case PageWidthRole: return this->pageWidth();
    case PageHeightRole: return this->pageHeight();
    default: break;
    }

    return QVariant();
}

QHash<int, QByteArray> ImagePrinter::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[PageUrlRole] = "pageUrl";
    roles[PageWidthRole] = "pageWidth";
    roles[PageHeightRole] = "pageHeight";
    return roles;
}

int ImagePrinter::devType() const
{
    return QPaintDevice::devType();
}

QPaintEngine *ImagePrinter::paintEngine() const
{
    if(m_engine == nullptr)
        m_engine = new ImagePrinterEngine();

    return m_engine;
}

int ImagePrinter::metric(QPaintDevice::PaintDeviceMetric metric) const
{
    switch(metric)
    {
    case PdmWidth: return m_pageSize.width();
    case PdmHeight: return  m_pageSize.height();
    case PdmWidthMM: return qRound( qreal(m_pageSize.width()) * 1000 / qreal(m_templatePageImage.dotsPerMeterX()) );
    case PdmHeightMM: return qRound( qreal(m_pageSize.height()) * 1000 / qreal(m_templatePageImage.dotsPerMeterY()) );
    case PdmNumColors: return m_templatePageImage.colorCount();
    case PdmDepth: return m_templatePageImage.depth();
    case PdmDpiX: return qRound(m_templatePageImage.dotsPerMeterX() * 0.0254);
    case PdmDpiY: return qRound(m_templatePageImage.dotsPerMeterY() * 0.0254);
    case PdmPhysicalDpiX: return qRound(m_templatePageImage.dotsPerMeterX() * 0.0254);
    case PdmPhysicalDpiY: return qRound(m_templatePageImage.dotsPerMeterY() * 0.0254);
    case PdmDevicePixelRatio: return m_templatePageImage.QPaintDevice::devicePixelRatio();
    case PdmDevicePixelRatioScaled: return int(m_templatePageImage.QPaintDevice::devicePixelRatioF() * m_templatePageImage.QPaintDevice::devicePixelRatioFScale());
    }

    return 0;
}

void ImagePrinter::initPainter(QPainter *painter) const
{
    QPagedPaintDevice::initPainter(painter);
}

QPaintDevice *ImagePrinter::redirected(QPoint *offset) const
{
    return QPagedPaintDevice::redirected(offset);
}

QPainter *ImagePrinter::sharedPainter() const
{
    return QPagedPaintDevice::sharedPainter();
}

bool ImagePrinter::newPage()
{
    if(m_engine)
    {
        m_engine->newPage();
        this->capturePrintedPageImage();
        return true;
    }

    return false;
}

void ImagePrinter::setPageSize(QPagedPaintDevice::PageSize size)
{
    QPagedPaintDevice::setPageSize(size);
}

void ImagePrinter::setPageSizeMM(const QSizeF &size)
{
    QPagedPaintDevice::setPageSizeMM(size);
}

void ImagePrinter::setMargins(const QPagedPaintDevice::Margins &margins)
{
    QPagedPaintDevice::setMargins(margins);
}

void ImagePrinter::classBegin()
{
    // TODO:
}

void ImagePrinter::componentComplete()
{
    QJSEngine *jsEngine = qjsEngine(this);
    if(jsEngine != nullptr)
    {
        QQmlEngine *qmlEngine = qobject_cast<QQmlEngine*>(jsEngine);
        if(qmlEngine != nullptr)
        {
            QQmlImageProviderBase *imgProvider = qmlEngine->imageProvider(ImagePrinterImageProvider::urlNamespace());
            if(imgProvider == nullptr)
            {
                imgProvider = new ImagePrinterImageProvider;
                qmlEngine->addImageProvider(ImagePrinterImageProvider::urlNamespace(), imgProvider);
            }

            ImagePrinterImageProvider *imgProvider2 = static_cast<ImagePrinterImageProvider*>(imgProvider);
            imgProvider2->add(this);
        }
    }

    if(this->objectName().isEmpty())
        this->setObjectName( QStringLiteral("imagePrinter") );
}

void ImagePrinter::begin()
{
    this->setPrinting(true);
    this->beginResetModel();

    m_templatePageImage = QImage(10, 10, QImage::Format_ARGB32);
    const int resolution = qMax( this->metric(PdmDpiX), this->metric(PdmDpiY) );
    m_pageSize = this->pageLayout().pageSize().sizePixels(resolution);

    m_templatePageImage = QImage(m_pageSize.width(), m_pageSize.height(), QImage::Format_ARGB32);
    m_pageImages.clear();
}

void ImagePrinter::end()
{
    m_templatePageImage = QImage(10, 10, QImage::Format_ARGB32);

    this->capturePrintedPageImage();
    this->endResetModel();

    emit pagesChanged();

    this->setPrinting(false);
}

void ImagePrinter::capturePrintedPageImage()
{
    if(m_engine == nullptr)
        return;

    QWriteLocker lock(&m_pageImagesLock);
    m_pageImages << m_engine->printedPageImage();
}

///////////////////////////////////////////////////////////////////////////////

ImagePrinterImageProvider::ImagePrinterImageProvider()
    : QQuickImageProvider(Image, ForceAsynchronousImageLoading)
{

}

ImagePrinterImageProvider::~ImagePrinterImageProvider()
{

}

QString ImagePrinterImageProvider::urlNamespace()
{
    return QStringLiteral("imageprinter");
}

void ImagePrinterImageProvider::add(ImagePrinter *printer)
{
    if(printer == nullptr)
        return;

    QWriteLocker lock(&m_printersLock);
    m_printers += printer;

    connect(printer, &ImagePrinter::aboutToDelete, this, &ImagePrinterImageProvider::remove);
}

void ImagePrinterImageProvider::remove(ImagePrinter *printer)
{
    if(printer == nullptr)
        return;

    QWriteLocker lock(&m_printersLock);
    m_printers -= printer;
}

ImagePrinter *ImagePrinterImageProvider::find(const QString &name)
{
    QReadLocker lock(&m_printersLock);
    Q_FOREACH(ImagePrinter *printer, m_printers)
    {
        if(printer->objectName() == name)
            return printer;
    }

    return nullptr;
}

QImage ImagePrinterImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QImage ret;
    if(size)
        *size = ret.size();

    QStringList comps = id.split("/");
    if(comps.size() < 2)
        return ret;

    ImagePrinter *printer = this->find(comps.first());
    if(printer == nullptr)
        return ret;

    bool ok = false;
    const int index = comps.last().toInt(&ok);
    if(!ok)
        return ret;

    ret = printer->pageImageAt(index);

    if(!ret.isNull())
    {
        if(size)
            *size = ret.size();

        if(requestedSize.isValid())
            ret = ret.scaled(requestedSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////

ImagePrinterEngine::ImagePrinterEngine()
{
    m_padding1[0] = 0;
    m_padding2[0] = 0;
}

ImagePrinterEngine::~ImagePrinterEngine()
{

}

void ImagePrinterEngine::newPage()
{
    if(m_currentDevice)
    {
        this->paintHeaderFooterWatermark();

        m_pagePainter->end();
        delete m_pagePainter;
        m_pagePainter = nullptr;

        this->savePageImage();
    }

    const QSize pageImageSize = m_pageSize*m_pageScale;
    m_pageImage = QImage(pageImageSize, m_imageFormat);
    m_pageImage.setDevicePixelRatio(m_pageScale);
    m_pageImage.fill(m_pageColor);
    ++m_pageNumber;

    if(m_currentDevice)
    {
        m_pagePainter = new QPainter(&m_pageImage);

        m_pageTransform = QTransform();
        m_pageTransform.translate(0, -m_pageSize.height()*(m_pageNumber-1));
    }
}

bool ImagePrinterEngine::begin(QPaintDevice *pdev)
{
    ImagePrinter *printToImage = static_cast<ImagePrinter*>(pdev);
    m_currentDevice = printToImage;

    printToImage->begin();

    const QPageLayout pageLayout = m_currentDevice->pageLayout();

    m_directory = printToImage->directory();
    m_pageSize = QSize(printToImage->width(), printToImage->height());
    m_pageScale = printToImage->scale();
    m_pageColor = Qt::white;
    m_imageFormat = QImage::Format_ARGB32;
    m_pageNumber = 1;

    if(!m_directory.isEmpty())
    {
        QDir().mkpath(m_directory);

        m_baseFileName = printToImage->objectName();
        m_fileFormat = printToImage->imageFormat() == ImagePrinter::PNG ? QByteArrayLiteral("PNG") : QByteArrayLiteral("JPG");

        QDir dir(m_directory);
        QFileInfoList entryInfoList = dir.entryInfoList(QDir::Files, QDir::Name);
        if(!entryInfoList.isEmpty())
        {
            const QString subDirectory = QString::number( QDateTime::currentSecsSinceEpoch() );
            dir.mkdir(subDirectory);
            dir.cd(subDirectory);
            m_directory = dir.absolutePath();
        }
    }

    const QSize pageImageSize = m_pageSize*m_pageScale;
    m_pageImage = QImage(pageImageSize, m_imageFormat);
    m_pageImage.setDevicePixelRatio(m_pageScale);
    m_pageImage.fill(m_pageColor);

    m_pagePainter = new QPainter(&m_pageImage);
    m_pageTransform = QTransform();

    return true;
}

bool ImagePrinterEngine::end()
{
    this->paintHeaderFooterWatermark();

    m_pagePainter->end();
    delete m_pagePainter;
    m_pagePainter = nullptr;
    m_pageTransform = QTransform();

    this->savePageImage();

    m_currentDevice->end();
    m_currentDevice = nullptr;

    m_directory.clear();
    m_pageSize = QSize(10, 10); // US Letter Page Size @ 96dpi
    m_pageColor = Qt::white;
    m_imageFormat = QImage::Format_ARGB32;
    m_pageNumber = 1;
    m_baseFileName.clear();
    m_fileFormat.clear();
    m_pageImage = QImage(m_pageSize, m_imageFormat);
    m_pageImage.fill(m_pageColor);
    m_printedPageImage = QImage();

    return true;
}

void ImagePrinterEngine::updateState(const QPaintEngineState &pestate)
{
    if(m_pagePainter)
    {
        if(pestate.state() & QPaintEngine::DirtyPen)
            m_pagePainter->setPen(pestate.pen());

        if(pestate.state() & QPaintEngine::DirtyBrush)
            m_pagePainter->setBrush(pestate.brush());

        if(pestate.state() & QPaintEngine::DirtyBrushOrigin)
            m_pagePainter->setBrushOrigin(pestate.brushOrigin());

        if(pestate.state() & QPaintEngine::DirtyFont)
            m_pagePainter->setFont(pestate.font());

        if(pestate.state() & QPaintEngine::DirtyBackground)
            m_pagePainter->setBackground(pestate.backgroundBrush());

        if(pestate.state() & QPaintEngine::DirtyBackgroundMode)
            m_pagePainter->setBackgroundMode(pestate.backgroundMode());

        if(pestate.state() & QPaintEngine::DirtyTransform)
        {
            QTransform tx = pestate.transform();
            tx.translate(0, (m_pageNumber-1)*m_pageSize.height());
            m_pagePainter->setTransform(tx);
        }

        if(pestate.state() & QPaintEngine::DirtyClipPath)
        {
            QTransform tx;
            tx.translate(0, -(m_pageNumber-1)*m_pageSize.height());
            m_pagePainter->setClipPath( tx.map(pestate.clipPath()) );
        }

        if(pestate.state() & QPaintEngine::DirtyClipRegion)
        {
            QTransform tx;
            tx.translate(0, -(m_pageNumber-1)*m_pageSize.height());
            m_pagePainter->setClipRegion(tx.map(pestate.clipRegion()));
        }

        if(pestate.state() & QPaintEngine::DirtyHints)
            m_pagePainter->setRenderHints(pestate.renderHints());

        if(pestate.state() & QPaintEngine::DirtyCompositionMode)
            m_pagePainter->setCompositionMode(pestate.compositionMode());

        if(pestate.state() & QPaintEngine::DirtyClipEnabled)
            m_pagePainter->setClipping(pestate.isClipEnabled());

        if(pestate.state() & QPaintEngine::DirtyOpacity)
            m_pagePainter->setOpacity(pestate.opacity());
    }
}

void ImagePrinterEngine::drawRects(const QRect *rects, int rectCount)
{
    if(m_pageTransform.isIdentity())
    {
        m_pagePainter->drawRects(rects, rectCount);
        return;
    }

    QVector<QRect> glyphs(rectCount);
    for(int i=0; i<glyphs.size(); i++)
        glyphs[i] = m_pageTransform.mapRect(rects[i]);

    m_pagePainter->drawRects(glyphs.constData(), glyphs.size());
}

void ImagePrinterEngine::drawRects(const QRectF *rects, int rectCount)
{
    if(m_pageTransform.isIdentity())
    {
        m_pagePainter->drawRects(rects, rectCount);
        return;
    }

    QVector<QRectF> glyphs(rectCount);
    for(int i=0; i<glyphs.size(); i++)
        glyphs[i] = m_pageTransform.mapRect(rects[i]);

    m_pagePainter->drawRects(glyphs.constData(), glyphs.size());
}

void ImagePrinterEngine::drawLines(const QLine *lines, int lineCount)
{
    if(m_pageTransform.isIdentity())
    {
        m_pagePainter->drawLines(lines, lineCount);
        return;
    }

    QVector<QLine> glyphs(lineCount);
    for(int i=0; i<glyphs.size(); i++)
        glyphs[i] = m_pageTransform.map(lines[i]);

    m_pagePainter->drawLines(glyphs.constData(), glyphs.size());
}

void ImagePrinterEngine::drawLines(const QLineF *lines, int lineCount)
{
    if(m_pageTransform.isIdentity())
    {
        m_pagePainter->drawLines(lines, lineCount);
        return;
    }

    QVector<QLineF> glyphs(lineCount);
    for(int i=0; i<glyphs.size(); i++)
        glyphs[i] = m_pageTransform.map(lines[i]);

    m_pagePainter->drawLines(glyphs.constData(), glyphs.size());
}

void ImagePrinterEngine::drawEllipse(const QRectF &r)
{
    if(m_pageTransform.isIdentity())
        m_pagePainter->drawEllipse(r);
    else
    {
        const QRectF glyph = m_pageTransform.mapRect(r);
        m_pagePainter->drawEllipse(glyph);
    }
}

void ImagePrinterEngine::drawEllipse(const QRect &r)
{
    if(m_pageTransform.isIdentity())
        m_pagePainter->drawEllipse(r);
    else
    {
        const QRect glyph = m_pageTransform.mapRect(r);
        m_pagePainter->drawEllipse(glyph);
    }
}

void ImagePrinterEngine::drawPath(const QPainterPath &path)
{
    if(m_pageTransform.isIdentity())
        m_pagePainter->drawPath(path);
    else
    {
        const QPainterPath glyph = m_pageTransform.map(path);
        m_pagePainter->drawPath(glyph);
    }
}

void ImagePrinterEngine::drawPoints(const QPointF *points, int pointCount)
{
    if(m_pageTransform.isIdentity())
    {
        m_pagePainter->drawPoints(points, pointCount);
        return;
    }

    QVector<QPointF> glyphs(pointCount);
    for(int i=0; i<glyphs.size(); i++)
        glyphs[i] = m_pageTransform.map(points[i]);

    m_pagePainter->drawPoints(glyphs.constData(), glyphs.size());
}

void ImagePrinterEngine::drawPoints(const QPoint *points, int pointCount)
{
    if(m_pageTransform.isIdentity())
    {
        m_pagePainter->drawPoints(points, pointCount);
        return;
    }

    QVector<QPoint> glyphs(pointCount);
    for(int i=0; i<glyphs.size(); i++)
        glyphs[i] = m_pageTransform.map(points[i]);

    m_pagePainter->drawPoints(glyphs.constData(), glyphs.size());
}

void ImagePrinterEngine::drawPolygon(const QPointF *points, int pointCount, QPaintEngine::PolygonDrawMode mode)
{
    Qt::FillRule fillRule = Qt::OddEvenFill;
    if(mode & QPaintEngine::WindingMode)
        fillRule = Qt::WindingFill;

    if(m_pageTransform.isIdentity())
    {
        m_pagePainter->drawPolygon(points, pointCount, fillRule);
        return;
    }

    QVector<QPointF> glyphs(pointCount);
    for(int i=0; i<glyphs.size(); i++)
        glyphs[i] = m_pageTransform.map(points[i]);

    m_pagePainter->drawPolygon(glyphs.constData(), glyphs.size(), fillRule);
}

void ImagePrinterEngine::drawPolygon(const QPoint *points, int pointCount, QPaintEngine::PolygonDrawMode mode)
{
    Qt::FillRule fillRule = Qt::OddEvenFill;
    if(mode & QPaintEngine::WindingMode)
        fillRule = Qt::WindingFill;

    if(m_pageTransform.isIdentity())
    {
        m_pagePainter->drawPolygon(points, pointCount, fillRule);
        return;
    }

    QVector<QPoint> glyphs(pointCount);
    for(int i=0; i<glyphs.size(); i++)
        glyphs[i] = m_pageTransform.map(points[i]);

    m_pagePainter->drawPolygon(glyphs.constData(), glyphs.size(), fillRule);
}

void ImagePrinterEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr)
{
    if(m_pageTransform.isIdentity())
    {
        m_pagePainter->drawPixmap(r, pm, sr);
        return;
    }

    const QRectF r2 = m_pageTransform.mapRect(r);
    m_pagePainter->drawPixmap(r2, pm, sr);
}

void ImagePrinterEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
    if(m_pageTransform.isIdentity())
    {
        m_pagePainter->drawTextItem(p, textItem);
        return;
    }

    const QPointF p2 = m_pageTransform.map(p);
    m_pagePainter->drawTextItem(p2, textItem);
}

void ImagePrinterEngine::drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s)
{
    if(m_pageTransform.isIdentity())
    {
        m_pagePainter->drawTiledPixmap(r, pixmap, s);
        return;
    }

    const QRectF r2 = m_pageTransform.mapRect(r);
    m_pagePainter->drawTiledPixmap(r2, pixmap, s);
}

void ImagePrinterEngine::drawImage(const QRectF &r, const QImage &pm, const QRectF &sr, Qt::ImageConversionFlags flags)
{
    if(m_pageTransform.isIdentity())
    {
        m_pagePainter->drawImage(r, pm, sr, flags);
        return;
    }

    const QRectF r2 = m_pageTransform.mapRect(r);
    m_pagePainter->drawImage(r2, pm, sr, flags);
}

void ImagePrinterEngine::paintHeaderFooterWatermark()
{
    if(m_currentDevice != nullptr)
    {
        m_pagePainter->setTransform(QTransform());
        m_pagePainter->setClipping(false);
    }

    m_printedPageImage = m_pageImage;
}

void ImagePrinterEngine::savePageImage()
{
    if(m_directory.isEmpty())
        return;

    const QString filePath = QDir(m_directory).absoluteFilePath(m_baseFileName + QString::number(m_pageNumber) + "." + m_fileFormat);
    m_pageImage.save(filePath, m_fileFormat);
}
