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

#include "pdfexportablegraphicsscene.h"
#include "hourglass.h"
#include "screenplay.h"
#include "application.h"
#include "scritedocument.h"

#include <QFile>
#include <QPainter>
#include <QDateTime>
#include <QPdfWriter>
#include <QPaintEngine>
#include <QGuiApplication>

PdfExportableGraphicsScene::PdfExportableGraphicsScene(QObject *parent)
    : QGraphicsScene(parent)
{
    this->setBackgroundBrush(Qt::white);
}

PdfExportableGraphicsScene::~PdfExportableGraphicsScene()
{

}

void PdfExportableGraphicsScene::setTitle(QString val)
{
    if(m_title == val)
        return;

    m_title = val;
    emit titleChanged();
}

void PdfExportableGraphicsScene::setComment(const QString &val)
{
    if(m_comment == val)
        return;

    m_comment = val;
    emit commentChanged();
}

void PdfExportableGraphicsScene::setWatermark(const QString &val)
{
    if(m_watermark == val)
        return;

    m_watermark = val;
    emit watermarkChanged();
}

bool PdfExportableGraphicsScene::exportToPdf(const QString &fileName)
{
    QFile file(fileName);
    if(!file.open(QFile::WriteOnly))
        return false;

    return this->exportToPdf(&file);
}

bool PdfExportableGraphicsScene::exportToPdf(QIODevice *device)
{
    HourGlass hourGlass;

#ifdef Q_OS_MAC
    const qreal dpi = 72.0;
#else
    const qreal dpi = 96.0;
#endif

    // How big is the scene?
    QRectF sceneRect = this->itemsBoundingRect();

    // We are going to need atleast 1" border around.
    sceneRect.adjust(-dpi, -dpi, dpi, dpi);

    // Figure out the page size in which we have to create the PDF
    QPageSize pageSize(sceneRect.size()/dpi, QPageSize::Inch, QStringLiteral("Custom"), QPageSize::FuzzyMatch );

    // Now, figure out the rect available on paper for printing
    QRectF pageRect = pageSize.rectPixels(dpi);

    // Now, calculate the target rect on paper.
    QRectF targetRect(0, 0, sceneRect.width(), sceneRect.height());
    targetRect.moveCenter(pageRect.center());

    // Now, lets create a PDF writer and draw the scene into it.
    QPdfWriter pdfWriter(device);
    pdfWriter.setPdfVersion(QPagedPaintDevice::PdfVersion_1_6);
    pdfWriter.setTitle(m_title);
    pdfWriter.setCreator(qApp->applicationName() + " " + qApp->applicationVersion());
    pdfWriter.setPageSize(pageSize);
    pdfWriter.setResolution(int(dpi));

    const qreal dpiScaleX = qreal(pdfWriter.logicalDpiX()) / dpi;
    const qreal dpiScaleY = qreal(pdfWriter.logicalDpiY()) / dpi;

    QPainter paint(&pdfWriter);
    paint.setRenderHint(QPainter::Antialiasing);
    paint.setRenderHint(QPainter::SmoothPixmapTransform);
    paint.scale(dpiScaleX, dpiScaleY);
    this->render(&paint, targetRect, sceneRect, Qt::KeepAspectRatio);
    paint.end();

    return true;
}

void PdfExportableGraphicsScene::addStandardItems(int items)
{
    const ScriteDocument *scriteDocument = ScriteDocument::instance();
    const Screenplay *screenplay = scriteDocument->screenplay();

    QRectF contentsRect = this->itemsBoundingRect();

    QGraphicsSimpleTextItem *appInfoItem = new QGraphicsSimpleTextItem;
    appInfoItem->setText( QStringLiteral("Created using Scrite (www.scrite.io)") );
    appInfoItem->setZValue(999);

    QRectF appInfoRect = appInfoItem->boundingRect();
    appInfoRect.moveCenter(contentsRect.center());
    appInfoRect.moveTop(contentsRect.bottom() + 50);
    appInfoItem->setPos(appInfoRect.topLeft());
    this->addItem(appInfoItem);

    contentsRect = this->itemsBoundingRect();

    QMap<HeaderFooter::Field,QString> fields;
    if(items & HeaderFooterOnly)
    {
        fields[HeaderFooter::AppName] = Application::instance()->applicationName();
        fields[HeaderFooter::AppVersion] = Application::instance()->applicationVersion();
        fields[HeaderFooter::Title] = screenplay->title();
        fields[HeaderFooter::Subtitle] = screenplay->subtitle();
        fields[HeaderFooter::Author] = screenplay->author();
        fields[HeaderFooter::Contact] = screenplay->contact();
        fields[HeaderFooter::Version] = screenplay->version();
        fields[HeaderFooter::Email] = screenplay->email();
        fields[HeaderFooter::Phone] = screenplay->phoneNumber();
        fields[HeaderFooter::Website] = screenplay->website();
        fields[HeaderFooter::Comment] = m_comment;
        fields[HeaderFooter::Watermark] = m_watermark;
        fields[HeaderFooter::Date] = QDate::currentDate().toString(Qt::SystemLocaleShortDate);
        fields[HeaderFooter::Time] = QTime::currentTime().toString(Qt::SystemLocaleShortDate);
        fields[HeaderFooter::DateTime] = QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate);
        fields[HeaderFooter::PageNumber] = QStringLiteral("1.");
        fields[HeaderFooter::PageNumberOfCount] = QStringLiteral("1/1");
    }

    HeaderFooter *header = (items & HeaderFooterOnly) ? new HeaderFooter(HeaderFooter::Header, this) : nullptr;
    HeaderFooter *footer = (items & HeaderFooterOnly) ? new HeaderFooter(HeaderFooter::Footer, this) : nullptr;
    Watermark *watermark = (items & WatermarkOnly) ? new Watermark(this) : nullptr;
    QTextDocumentPagedPrinter::loadSettings(header, footer, watermark);

    if(items & HeaderFooterOnly)
    {
        QRectF headerRect = contentsRect;
        headerRect.setHeight(40);
        headerRect.moveBottom(contentsRect.top());
        GraphicsHeaderFooterItem *headerItem = new GraphicsHeaderFooterItem(header, fields);
        headerItem->setRect(headerRect);
        headerItem->setZValue(999);
        this->addItem(headerItem);

        QRectF footerRect = contentsRect;
        footerRect.setHeight(40);
        footerRect.moveTop(contentsRect.bottom());
        GraphicsHeaderFooterItem *footerItem = new GraphicsHeaderFooterItem(footer, fields);
        footerItem->setRect(footerRect);
        footerItem->setZValue(999);
        this->addItem(footerItem);
    }

    if(items & WatermarkOnly)
    {
        if(!m_watermark.isEmpty())
            watermark->setText(m_watermark);

        GraphicsWatermarkItem *watermarkItem = new GraphicsWatermarkItem(watermark);
        watermarkItem->setRect(contentsRect);
        watermarkItem->setZValue(-999);
        this->addItem(watermarkItem);
    }
}

///////////////////////////////////////////////////////////////////////////////

GraphicsHeaderFooterItem::GraphicsHeaderFooterItem(HeaderFooter *headerFooter, const QMap<HeaderFooter::Field,QString> &fields)
    : m_headerFooter(headerFooter), m_fields(fields)
{
    if(m_headerFooter != nullptr)
        m_headerFooter->setVisibleFromPageOne(true);
}

GraphicsHeaderFooterItem::~GraphicsHeaderFooterItem()
{
    if(m_headerFooter != nullptr && m_headerFooter->parent() == nullptr)
        delete m_headerFooter;
}

void GraphicsHeaderFooterItem::setRect(const QRectF &rect)
{
    this->setPos(rect.topLeft());
    this->prepareGeometryChange();
    m_rect = QRectF(0, 0, rect.width(), rect.height());
    this->update();
}

void GraphicsHeaderFooterItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if(m_headerFooter == nullptr)
        return;

    Q_UNUSED(option);
    Q_UNUSED(widget);

    m_headerFooter->prepare(m_fields, m_rect, painter->paintEngine()->paintDevice());
    m_headerFooter->paint(painter, m_rect, 1, 1);
}

///////////////////////////////////////////////////////////////////////////////

GraphicsWatermarkItem::GraphicsWatermarkItem(Watermark *watermark)
    : m_watermark(watermark)
{
    if(m_watermark != nullptr)
        m_watermark->setVisibleFromPageOne(true);
}

GraphicsWatermarkItem::~GraphicsWatermarkItem()
{
    if(m_watermark != nullptr && m_watermark->parent() == nullptr)
        delete m_watermark;
}

void GraphicsWatermarkItem::setRect(const QRectF &rect)
{
    this->setPos(rect.topLeft());
    this->prepareGeometryChange();
    m_rect = QRectF(0, 0, rect.width(), rect.height());
    this->update();
}

void GraphicsWatermarkItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if(m_watermark == nullptr)
        return;

    Q_UNUSED(option);
    Q_UNUSED(widget);

    m_watermark->paint(painter, m_rect, 1, 1);
}
