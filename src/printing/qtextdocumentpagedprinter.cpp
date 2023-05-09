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

#include "user.h"
#include "scrite.h"
#include "ruleritem.h"
#include "application.h"
#include "scritedocument.h"
#include "qtextdocumentpagedprinter.h"

#include <QDate>
#include <QTime>
#include <QtDebug>
#include <QPainter>
#include <QDateTime>
#include <QSettings>
#include <QTextBlock>
#include <QPaintEngine>
#include <QAbstractTextDocumentLayout>

HeaderFooter::HeaderFooter(Type type, QObject *parent) : QObject(parent), m_type(type)
{
    m_padding1[0] = 0; // just to get rid of the unused private variable warning.
    m_padding2[0] = 0; // just to get rid of the unused private variable warning.
}

HeaderFooter::~HeaderFooter() { }

void HeaderFooter::setLeft(HeaderFooter::Field val)
{
    if (m_left == val)
        return;

    m_left = val;
    emit leftChanged();
}

void HeaderFooter::setCenter(HeaderFooter::Field val)
{
    if (m_center == val)
        return;

    m_center = val;
    emit centerChanged();
}

void HeaderFooter::setRight(HeaderFooter::Field val)
{
    if (m_right == val)
        return;

    m_right = val;
    emit rightChanged();
}

void HeaderFooter::setFont(const QFont &val)
{
    if (m_font == val)
        return;

    m_font = val;
    emit fontChanged();
}

void HeaderFooter::setOpacity(qreal val)
{
    if (qFuzzyCompare(m_opacity, val))
        return;

    m_opacity = val;
    emit opacityChanged();
}

void HeaderFooter::setVisibleFromPageOne(bool val)
{
    if (m_visibleFromPageOne == val)
        return;

    m_visibleFromPageOne = val;
    emit visibleFromPageOneChanged();
}

void HeaderFooter::setRect(const QRectF &val)
{
    if (m_rect == val)
        return;

    m_rect = val;
    emit rectChanged();
}

void HeaderFooter::prepare(const QMap<Field, QString> &fieldValues, const QRectF &rect,
                           QPaintDevice *pd)
{
    m_columns.resize(3);

    // Evalute the content we will show in each column
    m_columns[0].content = fieldValues.value(m_left);
    m_columns[1].content = fieldValues.value(m_center);
    m_columns[2].content = fieldValues.value(m_right);

    // Figure out how much size does each column actually take
    QFontMetricsF fm(m_font, pd);
    const qreal colSizes[3] = { fm.horizontalAdvance(m_columns[0].content),
                                fm.horizontalAdvance(m_columns[1].content),
                                fm.horizontalAdvance(m_columns[2].content) };

    const qreal sideColumnWidth = qMin(qMax(colSizes[0], colSizes[2]), rect.width() * 0.4);
    const qreal middleColumnWidth = rect.width() - 2 * sideColumnWidth;
    const qreal columnWidths[] = { sideColumnWidth, middleColumnWidth, sideColumnWidth };

    // Lets distribute the space available in the ratio of sizes
    for (int i = 0; i < 3; i++) {
        QRectF colRect = rect;
        colRect.setWidth(columnWidths[i]);
        if (i > 0)
            colRect.moveLeft(m_columns[i - 1].columnRect.right());
        m_columns[i].columnRect = colRect;
    }

    // Set text-flags for each cell
    m_columns[0].flags = Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap;
    m_columns[1].flags = Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextWordWrap;
    m_columns[2].flags = Qt::AlignRight | Qt::AlignVCenter | Qt::TextWordWrap;
}

void HeaderFooter::paint(QPainter *paint, const QRectF &, int pageNr, int pageCount)
{
    if (m_left == Nothing && m_right == Nothing && m_center == Nothing)
        return;

    if (!m_visibleFromPageOne) {
        --pageNr;
        --pageCount;
    }

    if (pageNr == 0)
        return;

    auto updateContent = [pageNr, pageCount](ColumnContent &content, Field field) {
        // Fixes https://github.com/teriflix/scrite/issues/207
        if ((field == PageNumber || field == PageNumberOfCount) && pageNr == 1) {
            content.content.clear();
            return;
        }

        if (field == PageNumber)
            content.content = QString::number(pageNr) + ".";
        else if (field == PageNumberOfCount) {
            if (pageCount >= pageNr)
                content.content = QString::number(pageNr) + "/" + QString::number(pageCount);
            else
                content.content = QString::number(pageNr) + ".";
        }
    };

    updateContent(m_columns[0], m_left);
    updateContent(m_columns[1], m_center);
    updateContent(m_columns[2], m_right);

    for (int i = 0; i < 3; i++) {
        if (m_columns.at(i).content.isEmpty())
            continue;

        paint->save();
        paint->setOpacity(m_opacity);
        paint->setFont(m_font);
        paint->drawText(m_columns.at(i).columnRect, m_columns.at(i).flags, m_columns.at(i).content);
        paint->restore();
    }
}

void HeaderFooter::finish()
{
    m_columns.clear();
}

///////////////////////////////////////////////////////////////////////////////

Watermark::Watermark(QObject *parent) : QObject(parent)
{
    m_padding[0] = 0;

    m_enabled = !User::instance()->isFeatureEnabled(Scrite::WatermarkFeature);
    m_visibleFromPageOne = !User::instance()->isFeatureEnabled(Scrite::WatermarkFeature);
}

Watermark::~Watermark() { }

void Watermark::setEnabled(bool val)
{
    if (m_enabled == val || !User::instance()->isFeatureEnabled(Scrite::WatermarkFeature))
        return;

    m_enabled = val;
    emit enabledChanged();
}

void Watermark::setText(const QString &val)
{
    if (m_text == val || !User::instance()->isFeatureEnabled(Scrite::WatermarkFeature))
        return;

    m_text = val;
    emit textChanged();
}

void Watermark::setFont(const QFont &val)
{
    if (m_font == val || !User::instance()->isFeatureEnabled(Scrite::WatermarkFeature))
        return;

    m_font = val;
    emit fontChanged();
}

void Watermark::setColor(const QColor &val)
{
    if (m_color == val || !User::instance()->isFeatureEnabled(Scrite::WatermarkFeature))
        return;

    m_color = val;
    emit colorChanged();
}

void Watermark::setOpacity(qreal val)
{
    if (qFuzzyCompare(m_opacity, val)
        || !User::instance()->isFeatureEnabled(Scrite::WatermarkFeature))
        return;

    m_opacity = val;
    emit opacityChanged();
}

void Watermark::setRotation(qreal val)
{
    if (qFuzzyCompare(m_rotation, val)
        || !User::instance()->isFeatureEnabled(Scrite::WatermarkFeature))
        return;

    m_rotation = val;
    emit rotationChanged();
}

void Watermark::setAlignment(Qt::Alignment val)
{
    if (m_alignment == val || !User::instance()->isFeatureEnabled(Scrite::WatermarkFeature))
        return;

    m_alignment = val;
    emit alignmentChanged();
}

void Watermark::setVisibleFromPageOne(bool val)
{
    if (m_visibleFromPageOne == val
        || !User::instance()->isFeatureEnabled(Scrite::WatermarkFeature))
        return;

    m_visibleFromPageOne = val;
    emit visibleFromPageOneChanged();
}

void Watermark::setRect(const QRectF &val)
{
    if (m_rect == val || !User::instance()->isFeatureEnabled(Scrite::WatermarkFeature))
        return;

    m_rect = val;
    emit rectChanged();
}

void Watermark::paint(QPainter *painter, const QRectF &pageRect, int pageNr, int pageCount)
{
    if (!m_enabled)
        return;

    Q_UNUSED(pageCount)
    if (!m_visibleFromPageOne && pageNr == 1)
        return;

    const QPointF pageCenter = pageRect.center();

    QFontMetricsF fm(m_font, painter->device());
    QRectF textRect = fm.boundingRect(m_text);

    QTransform tx;
    tx.translate(textRect.center().x(), textRect.center().y());
    tx.rotate(m_rotation);
    tx.translate(-textRect.center().x(), -textRect.center().y());
    QRectF rotatedTextRect = tx.mapRect(textRect);

    if (m_alignment.testFlag(Qt::AlignLeft))
        rotatedTextRect.moveLeft(pageRect.left());
    else if (m_alignment.testFlag(Qt::AlignRight))
        rotatedTextRect.moveRight(pageRect.right());
    else {
        const QPointF textCenter = rotatedTextRect.center();
        rotatedTextRect.moveCenter(QPointF(pageCenter.x(), textCenter.y()));
    }

    if (m_alignment.testFlag(Qt::AlignTop))
        rotatedTextRect.moveTop(pageRect.top());
    else if (m_alignment.testFlag(Qt::AlignBottom))
        rotatedTextRect.moveBottom(pageRect.bottom());
    else {
        const QPointF textCenter = rotatedTextRect.center();
        rotatedTextRect.moveCenter(QPointF(textCenter.x(), pageCenter.y()));
    }

    textRect.moveCenter(rotatedTextRect.center());

    qreal scale = 1.0;
    if (rotatedTextRect.width() > pageRect.width())
        scale = qMin(scale, pageRect.width() / rotatedTextRect.width());
    if (rotatedTextRect.height() > pageRect.height())
        scale = qMin(scale, pageRect.height() / rotatedTextRect.height());

    painter->save();

    painter->setFont(m_font);
    painter->setPen(QPen(m_color));
    painter->setOpacity(m_opacity);
    painter->translate(textRect.center());
    painter->rotate(m_rotation);
    painter->scale(scale, scale);
    painter->translate(-textRect.center());
    painter->drawText(textRect, Qt::AlignCenter | Qt::TextDontClip, m_text);

    painter->restore();
}

///////////////////////////////////////////////////////////////////////////////

QTextDocumentPagedPrinter::QTextDocumentPagedPrinter(QObject *parent) : QObject(parent)
{
    QTextDocumentPagedPrinter::loadSettings(m_header, m_footer, m_watermark);
}

QTextDocumentPagedPrinter::~QTextDocumentPagedPrinter() { }

// Much of the code in the print() function is inspired from the implementation
// of QTextDocument::print() method implementation. Because I tried writing
// my own print() implementation and it always sucked in stellar proportions.
Q_DECL_IMPORT int qt_defaultDpi();

template<class T>
struct Pointer
{
    Pointer(T **ptr) : m_ptr(ptr) { }
    ~Pointer() { *m_ptr = nullptr; }

private:
    T **m_ptr;
};

bool QTextDocumentPagedPrinter::print(QTextDocument *document, QPagedPaintDevice *printer)
{
    Pointer<QTextDocument> td(&m_textDocument);
    Pointer<QPagedPaintDevice> pr(&m_printer);
    m_textDocument = document;
    m_printer = printer;

    m_errorReport->clear();

    if (m_textDocument == nullptr) {
        m_errorReport->setErrorMessage("No document to print.");
        return false;
    }

    if (m_printer == nullptr) {
        m_errorReport->setErrorMessage("No printer to print.");
        return false;
    }

    if (!printer)
        return false;

    const QSizeF pageSize = document->pageSize();
    bool documentPaginated =
            pageSize.isValid() && !pageSize.isNull() && int(pageSize.height()) != INT_MAX;

    QMarginsF m = printer->pageLayout().margins(QPageLayout::Millimeter);
    if (!documentPaginated && m.left() == 0 && m.right() == 0 && m.top() == 0 && m.bottom() == 0) {
        m.setLeft(2.);
        m.setRight(2.);
        m.setTop(2.);
        m.setBottom(2.);
        printer->setPageMargins(m, QPageLayout::Millimeter);
    }

    QPainter painter(printer);
    if (!painter.isActive())
        return false;

    const QTextDocument *doc = document;
    QScopedPointer<QTextDocument> clonedDoc;
    (void)doc->documentLayout(); // make sure that there is a layout

    QRectF body = QRectF(QPointF(0, 0), pageSize);
    QPair<qreal, qreal> contentScale = qMakePair(1.0, 1.0);

    if (documentPaginated) {
        // Documents generated using ScreenplayTextDocument will come paginated.

        qreal sourceDpiX = qt_defaultDpi();
        qreal sourceDpiY = sourceDpiX;

        QPaintDevice *dev = doc->documentLayout()->paintDevice();
        if (dev) {
            sourceDpiX = dev->logicalDpiX();
            sourceDpiY = dev->logicalDpiY();
        }

        // scale to dpi
        const qreal dpiScaleX = qreal(printer->logicalDpiX()) / sourceDpiX;
        const qreal dpiScaleY = qreal(printer->logicalDpiY()) / sourceDpiY;
        QSizeF scaledPageSize = pageSize;
        scaledPageSize.rwidth() *= dpiScaleX;
        scaledPageSize.rheight() *= dpiScaleY;

        // scale to page
        const QSizeF printerPageSize(printer->width(), printer->height());
        const qreal pageScaleX = printerPageSize.width() / scaledPageSize.width();
        const qreal pageScaleY = printerPageSize.height() / scaledPageSize.height();

        contentScale.first = dpiScaleX * pageScaleX;
        contentScale.second = dpiScaleY * pageScaleY;
    } else {
        // Reports generated using AbstractReportGenerator are not paginated.

        doc = document->clone(this);
        clonedDoc.reset(const_cast<QTextDocument *>(doc));

        for (QTextBlock srcBlock = document->firstBlock(), dstBlock = clonedDoc->firstBlock();
             srcBlock.isValid() && dstBlock.isValid();
             srcBlock = srcBlock.next(), dstBlock = dstBlock.next()) {
            dstBlock.layout()->setFormats(srcBlock.layout()->formats());
        }

        QAbstractTextDocumentLayout *layout = doc->documentLayout();
        layout->setPaintDevice(painter.device());

        // We dont have to do this because we do not use any custom handlers in Scrite.
        // layout->d_func()->handlers = documentLayout()->d_func()->handlers;

        int dpiy = painter.device()->logicalDpiY();
        int margin = int(((2 / 2.54) * dpiy)); // 2 cm margins
        QTextFrameFormat fmt = doc->rootFrame()->frameFormat();
        fmt.setMargin(margin);
        doc->rootFrame()->setFrameFormat(fmt);

        body = QRectF(0, 0, printer->width(), printer->height());

        // We dont compute pageNumberPos because we have a separate mechanism for drawing
        // headers, footers and watermark
        // pageNumberPos = QPointF(body.width() - margin,
        //                         body.height() - margin
        //                         + QFontMetrics(doc->defaultFont(), p.device()).ascent()
        //                         + 5 * dpiy / 72.0);

        clonedDoc->setPageSize(body.size());
    }

    // At this point we are ready to print as far as QTextDocument is concerned.

    // Lets configure the headers and footers before we actually go ahead and print.
    QMap<HeaderFooter::Field, QString> fieldMap;
    fieldMap[HeaderFooter::AppName] = Application::instance()->applicationName();
    fieldMap[HeaderFooter::AppVersion] = Application::instance()->applicationVersion();
    fieldMap[HeaderFooter::Title] = document->property("#title").toString();
    fieldMap[HeaderFooter::Subtitle] = document->property("#subtitle").toString();
    fieldMap[HeaderFooter::Author] = document->property("#author").toString();
    fieldMap[HeaderFooter::Contact] = document->property("#contact").toString();
    fieldMap[HeaderFooter::Version] = document->property("#version").toString();
    fieldMap[HeaderFooter::Email] = document->property("#email").toString();
    fieldMap[HeaderFooter::Phone] = document->property("#phone").toString();
    fieldMap[HeaderFooter::Website] = document->property("#website").toString();
    fieldMap[HeaderFooter::Comment] = document->property("#comment").toString();
    fieldMap[HeaderFooter::Watermark] = document->property("#watermark").toString();
    fieldMap[HeaderFooter::Date] = QDate::currentDate().toString(Qt::SystemLocaleShortDate);
    fieldMap[HeaderFooter::Time] = QTime::currentTime().toString(Qt::SystemLocaleShortDate);
    fieldMap[HeaderFooter::DateTime] =
            QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate);
    fieldMap[HeaderFooter::PageNumber] = QString::number(doc->pageCount()) + ".  ";
    fieldMap[HeaderFooter::PageNumberOfCount] =
            QString::number(doc->pageCount()) + "/" + QString::number(doc->pageCount()) + "  ";

    // Here is where we got to figure out the header, footer and watermark rectangles
    const QTextFrameFormat fmt = doc->rootFrame()->frameFormat();
    const qreal topMargin = fmt.topMargin() * contentScale.second;
    const qreal leftMargin = fmt.leftMargin() * contentScale.first;
    const qreal rightMargin = fmt.rightMargin() * contentScale.first;
    const qreal bottomMargin = fmt.bottomMargin() * contentScale.second;
    const qreal padding = 0;

    m_headerRect = QRectF(0, 0, printer->width(), printer->height());
    m_footerRect = m_headerRect;

    m_headerRect.setLeft(leftMargin);
    m_headerRect.setBottom(topMargin - padding);
    m_headerRect.setRight(printer->width() - rightMargin);

    m_footerRect.setTop(printer->height() - bottomMargin + padding);
    m_footerRect.setLeft(leftMargin);
    m_footerRect.setRight(printer->width() - rightMargin);

    m_header->setFont(doc->defaultFont());
    m_footer->setFont(doc->defaultFont());
    m_header->prepare(fieldMap, m_headerRect, printer);
    m_footer->prepare(fieldMap, m_footerRect, printer);

    const QString watermarkText = fieldMap.value(HeaderFooter::Watermark);
    if (!watermarkText.isEmpty())
        m_watermark->setText(watermarkText);

    // We are now ready to print.
    const int fromPageNr = 1;
    const int toPageNr = doc->pageCount();

    m_progressReport->start();
    m_progressReport->setProgressStep(1 / qreal(doc->pageCount() + 1));
    int pageNr = fromPageNr;
    QRectF pageRect;

    const bool isPdfDevice = printer->paintEngine()->type() == QPaintEngine::Pdf;

    // Print away!
    while (pageNr <= toPageNr) {
        painter.save();
        painter.scale(contentScale.first, contentScale.second);
        this->printPageContents(pageNr, toPageNr, &painter, doc, body, pageRect);
        if (!isPdfDevice)
            this->printHeaderFooterWatermark(pageNr, toPageNr, &painter, doc, body, pageRect);
        painter.restore();

        if (isPdfDevice)
            this->printHeaderFooterWatermark(pageNr, toPageNr, &painter, doc, body, pageRect);

        m_progressReport->tick();

        if (pageNr < toPageNr) {
            if (!m_printer->newPage())
                break;
        }

        ++pageNr;
    }

    // All done!
    m_header->finish();
    m_footer->finish();
    m_progressReport->finish();

    return true;
}

void QTextDocumentPagedPrinter::loadSettings(HeaderFooter *header, HeaderFooter *footer,
                                             Watermark *watermark)
{
    const PageSetup *pageSetup = ScriteDocument::instance()->pageSetup();

    if (header != nullptr) {
        header->setLeft(HeaderFooter::Field(pageSetup->headerLeft()));
        header->setCenter(HeaderFooter::Field(pageSetup->headerCenter()));
        header->setRight(HeaderFooter::Field(pageSetup->headerRight()));
        header->setOpacity(pageSetup->headerOpacity());
    }

    if (footer != nullptr) {
        footer->setLeft(HeaderFooter::Field(pageSetup->footerLeft()));
        footer->setCenter(HeaderFooter::Field(pageSetup->footerCenter()));
        footer->setRight(HeaderFooter::Field(pageSetup->footerRight()));
        footer->setOpacity(pageSetup->footerOpacity());
    }

    if (watermark != nullptr) {
        watermark->setEnabled(pageSetup->isWatermarkEnabled());
        watermark->setText(pageSetup->watermarkText());

        QFont watermarkFont;
        watermarkFont.setFamily(pageSetup->watermarkFont());
        watermarkFont.setPointSize(pageSetup->watermarkFontSize());
        watermark->setFont(watermarkFont);

        watermark->setColor(pageSetup->watermarkColor());
        watermark->setOpacity(pageSetup->watermarkOpacity());
        watermark->setRotation(watermark->rotation());
        watermark->setAlignment(watermark->alignment());
    }
}

void QTextDocumentPagedPrinter::printPageContents(int pageNr, int pageCount, QPainter *painter,
                                                  const QTextDocument *doc, const QRectF &body,
                                                  QRectF &docPageRect)
{
    Q_UNUSED(pageCount)

    painter->save();

    painter->translate(body.left(), body.top() - (pageNr - 1) * body.height());
    const QRectF pageRect(0, (pageNr - 1) * body.height(), body.width(), body.height());
    docPageRect = pageRect;

    QAbstractTextDocumentLayout *layout = doc->documentLayout();
    QAbstractTextDocumentLayout::PaintContext ctx;

    painter->setClipRect(pageRect);
    ctx.clip = pageRect;
    ctx.palette.setColor(QPalette::Text, Qt::black);
    layout->draw(painter, ctx);

    painter->restore();
}

void QTextDocumentPagedPrinter::printHeaderFooterWatermark(int pageNr, int pageCount,
                                                           QPainter *painter,
                                                           const QTextDocument *doc,
                                                           const QRectF &body,
                                                           const QRectF &docPageRect)
{
    Q_UNUSED(doc)
    Q_UNUSED(body)

    m_header->paint(painter, m_headerRect, pageNr, pageCount);
    m_footer->paint(painter, m_footerRect, pageNr, pageCount);
    m_watermark->paint(painter, QRectF(m_headerRect.bottomLeft(), m_footerRect.topRight()), pageNr,
                       pageCount);

    if (m_sideBar) {
        const QRectF rightSideRect(m_headerRect.bottomRight(),
                                   QPointF(body.right(), m_footerRect.top()));
        m_sideBar->paint(painter, QTextDocumentPageSideBarInterface::RightSide, rightSideRect, docPageRect);

        const QRectF leftSideRect(QPointF(body.left(), m_headerRect.bottom()),
                                  m_footerRect.topLeft());
        m_sideBar->paint(painter, QTextDocumentPageSideBarInterface::LeftSide, leftSideRect, docPageRect);
    }

#if 0
    painter->setPen(Qt::black);
    painter->drawLine(
            QLineF(body.left(), m_headerRect.bottom(), body.right(), m_headerRect.bottom()));
    painter->drawLine(QLineF(body.left(), m_footerRect.top(), body.right(), m_footerRect.top()));
#endif
}
