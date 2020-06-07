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

#include "application.h"
#include "qtextdocumentpagedprinter.h"

#include <QtDebug>
#include <QDate>
#include <QTime>
#include <QPainter>
#include <QDateTime>
#include <QSettings>
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>

HeaderFooter::HeaderFooter(Type type, QObject *parent)
    : QObject(parent), m_type(type)
{
    m_padding1[0] = 0; // just to get rid of the unused private variable warning.
    m_padding2[0] = 0; // just to get rid of the unused private variable warning.
}

HeaderFooter::~HeaderFooter()
{

}

void HeaderFooter::setLeft(HeaderFooter::Field val)
{
    if(m_left == val)
        return;

    m_left = val;
    emit leftChanged();
}

void HeaderFooter::setCenter(HeaderFooter::Field val)
{
    if(m_center == val)
        return;

    m_center = val;
    emit centerChanged();
}

void HeaderFooter::setRight(HeaderFooter::Field val)
{
    if(m_right == val)
        return;

    m_right = val;
    emit rightChanged();
}

void HeaderFooter::setFont(const QFont &val)
{
    if(m_font == val)
        return;

    m_font = val;
    emit fontChanged();
}

void HeaderFooter::setOpacity(qreal val)
{
    if( qFuzzyCompare(m_opacity, val) )
        return;

    m_opacity = val;
    emit opacityChanged();
}

void HeaderFooter::setVisibleFromPageOne(bool val)
{
    if(m_visibleFromPageOne == val)
        return;

    m_visibleFromPageOne = val;
    emit visibleFromPageOneChanged();
}

void HeaderFooter::setRect(const QRectF &val)
{
    if(m_rect == val)
        return;

    m_rect = val;
    emit rectChanged();
}

void HeaderFooter::prepare(const QMap<Field, QString> &fieldValues, const QRectF &rect)
{
    m_columns.resize(3);

    // Evalute the content we will show in each column
    m_columns[0].content = fieldValues.value(m_left);
    m_columns[1].content = fieldValues.value(m_center);
    m_columns[2].content = fieldValues.value(m_right);

    // Figure out how much size does each column actually take
    QFontMetricsF fm(m_font);
    const qreal colSizes[3] = {
        fm.horizontalAdvance(m_columns[0].content),
        fm.horizontalAdvance(m_columns[1].content),
        fm.horizontalAdvance(m_columns[2].content)
    };
    qreal totalSize = colSizes[0] + colSizes[1] + colSizes[2];
    if( qFuzzyIsNull(totalSize) )
        totalSize = rect.width();

    // Lets distribute the space available in the ratio of sizes
    for(int i=0; i<3; i++)
    {
         QRectF colRect = rect;
         colRect.setWidth( (colSizes[i]/totalSize)*rect.width() );
         if(i > 0)
             colRect.moveLeft(m_columns[i-1].columnRect.right());
         m_columns[i].columnRect = colRect;
    }

    // Set text-flags for each cell
    m_columns[0].flags = Qt::AlignLeft|Qt::AlignVCenter|Qt::TextWordWrap;
    m_columns[1].flags = Qt::AlignHCenter|Qt::AlignVCenter|Qt::TextWordWrap;
    m_columns[2].flags = Qt::AlignRight|Qt::AlignVCenter|Qt::TextWordWrap;
}

void HeaderFooter::paint(QPainter *paint, const QRectF &, int pageNr, int pageCount)
{
    if(m_left == Nothing && m_right == Nothing && m_center == Nothing)
        return;

    if(!m_visibleFromPageOne)
    {
        --pageNr;
        --pageCount;
    }

    if(pageNr == 0)
        return;

    auto updateContent = [pageNr,pageCount](ColumnContent &content, Field field) {
        if(field == PageNumber)
            content.content = QString::number(pageNr) + ".";
        else if(field == PageNumberOfCount) {
            if(pageCount >= pageNr)
                content.content = QString::number(pageNr) + "/" + QString::number(pageCount);
            else
                content.content = QString::number(pageNr) + ".";
        }
    };

    updateContent(m_columns[0], m_left);
    updateContent(m_columns[1], m_center);
    updateContent(m_columns[2], m_right);

    for(int i=0; i<3; i++)
    {
        if(m_columns.at(i).content.isEmpty())
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

Watermark::Watermark(QObject *parent)
          :QObject(parent)
{
    m_padding[0] = 0;
}

Watermark::~Watermark()
{

}

void Watermark::setEnabled(bool val)
{
    if(m_enabled == val)
        return;

    m_enabled = val;
    emit enabledChanged();
}

void Watermark::setText(const QString &val)
{
    if(m_text == val)
        return;

    m_text = val;
    emit textChanged();
}

void Watermark::setFont(const QFont &val)
{
    if(m_font == val)
        return;

    m_font = val;
    emit fontChanged();
}

void Watermark::setColor(const QColor &val)
{
    if(m_color == val)
        return;

    m_color = val;
    emit colorChanged();
}

void Watermark::setOpacity(qreal val)
{
    if( qFuzzyCompare(m_opacity, val) )
        return;

    m_opacity = val;
    emit opacityChanged();
}

void Watermark::setRotation(qreal val)
{
    if( qFuzzyCompare(m_rotation, val) )
        return;

    m_rotation = val;
    emit rotationChanged();
}

void Watermark::setAlignment(Qt::Alignment val)
{
    if(m_alignment == val)
        return;

    m_alignment = val;
    emit alignmentChanged();
}

void Watermark::setVisibleFromPageOne(bool val)
{
    if(m_visibleFromPageOne == val)
        return;

    m_visibleFromPageOne = val;
    emit visibleFromPageOneChanged();
}

void Watermark::setRect(const QRectF &val)
{
    if(m_rect == val)
        return;

    m_rect = val;
    emit rectChanged();
}

void Watermark::paint(QPainter *painter, const QRectF &pageRect, int pageNr, int pageCount)
{
    if(!m_enabled)
        return;

    Q_UNUSED(pageCount)
    if(!m_visibleFromPageOne && pageNr == 1)
        return;

    const QPointF pageCenter = pageRect.center();

    QFontMetricsF fm(m_font);
    QRectF textRect = fm.boundingRect( m_text );

    QTransform tx;
    tx.translate( textRect.center().x(), textRect.center().y() );
    tx.rotate( m_rotation );
    tx.translate( -textRect.center().x(), -textRect.center().y() );
    QRectF rotatedTextRect = tx.mapRect(textRect);

    if( m_alignment.testFlag(Qt::AlignLeft) )
        rotatedTextRect.moveLeft( pageRect.left() );
    else if( m_alignment.testFlag(Qt::AlignRight) )
        rotatedTextRect.moveRight( pageRect.right() );
    else
    {
        const QPointF textCenter = rotatedTextRect.center();
        rotatedTextRect.moveCenter( QPointF(pageCenter.x(), textCenter.y()) );
    }

    if( m_alignment.testFlag(Qt::AlignTop) )
        rotatedTextRect.moveTop( pageRect.top() );
    else if( m_alignment.testFlag(Qt::AlignBottom) )
        rotatedTextRect.moveBottom( pageRect.bottom() );
    else
    {
        const QPointF textCenter = rotatedTextRect.center();
        rotatedTextRect.moveCenter( QPointF(textCenter.x(), pageCenter.y()) );
    }

    textRect.moveCenter( rotatedTextRect.center() );

    painter->save();

    painter->setFont( m_font );
    painter->setPen( QPen(m_color) );
    painter->setOpacity( m_opacity );
    painter->translate( textRect.center() );
    painter->rotate( m_rotation );
    painter->translate( -textRect.center() );
    painter->drawText( textRect, Qt::AlignCenter|Qt::TextDontClip, m_text );

    painter->restore();
}

///////////////////////////////////////////////////////////////////////////////

QTextDocumentPagedPrinter::QTextDocumentPagedPrinter(QObject *parent)
    : QObject(parent)
{
    QTextDocumentPagedPrinter::loadSettings(m_header, m_footer, m_watermark);
}

QTextDocumentPagedPrinter::~QTextDocumentPagedPrinter()
{

}

template <class T>
struct Pointer
{
    Pointer(T **ptr) : m_ptr(ptr) { }
    ~Pointer() { *m_ptr = nullptr; }

private:
    T **m_ptr;
};

// Much of the code in this function is inspired from the implementation
// of QTextDocument::print() method implementation.
Q_DECL_IMPORT int qt_defaultDpi();

bool QTextDocumentPagedPrinter::print(QTextDocument *document, QPagedPaintDevice *device)
{
    Pointer<QTextDocument> td(&m_textDocument);
    Pointer<QPagedPaintDevice> pr(&m_printer);

    m_textDocument = document;
    m_printer = device;

    m_errorReport->clear();

    if(m_textDocument == nullptr)
    {
        m_errorReport->setErrorMessage("No document to print.");
        return false;
    }

    if(m_printer == nullptr)
    {
        m_errorReport->setErrorMessage("No printer to print.");
        return false;
    }

    // Prepare printer
    QPagedPaintDevice::Margins m = m_printer->margins();
    m.left = m.right = m.top = m.bottom = 2.; // 2 is in mm
    m_printer->setMargins(m);

    // Prepare document
    const QTextDocument *doc = m_textDocument;
    QScopedPointer<QTextDocument> clonedDoc;
    (void)doc->documentLayout(); // make sure that there is a layout

    doc = m_textDocument->clone(this);
    clonedDoc.reset(const_cast<QTextDocument *>(doc));

    for (QTextBlock srcBlock = m_textDocument->firstBlock(), dstBlock = clonedDoc->firstBlock();
         srcBlock.isValid() && dstBlock.isValid();
         srcBlock = srcBlock.next(), dstBlock = dstBlock.next())
         dstBlock.layout()->setFormats(srcBlock.layout()->formats());

    // Prepare painter
    QPainter painter(m_printer);
    QAbstractTextDocumentLayout *layout = doc->documentLayout();
    layout->setPaintDevice(painter.device());
    clonedDoc->setPageSize(QSizeF(m_printer->width(), m_printer->height()));

    const int dpiy = painter.device()->logicalDpiY();
    const int margin = int((2.0/2.54)*dpiy); // 2 cm margins
    const int padding = int((0.2/2.54)*dpiy); // 2 mm padding

    if(!doc->property("#rootFrameMarginNotRequired").toBool())
    {
        QTextFrameFormat fmt = doc->rootFrame()->frameFormat();
        fmt.setMargin(margin);
        doc->rootFrame()->setFrameFormat(fmt);
    }

    QMap<HeaderFooter::Field,QString> fieldMap;
    fieldMap[HeaderFooter::AppName] = Application::instance()->applicationName();
    fieldMap[HeaderFooter::AppVersion] = Application::instance()->applicationVersion();
    fieldMap[HeaderFooter::Title] = document->property("#title").toString();
    fieldMap[HeaderFooter::Subtitle] = document->property("#subtitle").toString();
    fieldMap[HeaderFooter::Author] = document->property("#author").toString();
    fieldMap[HeaderFooter::Contact] = document->property("#contact").toString();
    fieldMap[HeaderFooter::Version] = document->property("#version").toString();
    fieldMap[HeaderFooter::Date] = QDate::currentDate().toString(Qt::SystemLocaleShortDate);
    fieldMap[HeaderFooter::Time] = QTime::currentTime().toString(Qt::SystemLocaleShortDate);
    fieldMap[HeaderFooter::DateTime] = QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate);
    fieldMap[HeaderFooter::PageNumber] = QString::number(doc->pageCount()) + ".  ";
    fieldMap[HeaderFooter::PageNumberOfCount] = QString::number(doc->pageCount()) + "/" + QString::number(doc->pageCount()) + "  ";

    // Compute header & footer rects
    const QRectF body(0, 0, m_printer->width(), m_printer->height());
    m_headerRect = body;
    m_footerRect = body;

    m_headerRect.setLeft(body.left() + margin);
    m_headerRect.setBottom(body.top() + margin - padding);
    m_headerRect.setRight(body.right() - margin);

    m_footerRect.setTop(body.bottom() - margin + padding);
    m_footerRect.setLeft(body.left() + margin);
    m_footerRect.setRight(body.right() - margin);

    m_header->setFont(doc->defaultFont());
    m_footer->setFont(doc->defaultFont());
    m_header->prepare(fieldMap, m_headerRect);
    m_footer->prepare(fieldMap, m_footerRect);

    // Get ready to print
    const int fromPageNr = 1;
    const int toPageNr = doc->pageCount();

    m_progressReport->start();
    m_progressReport->setProgressStep(1/qreal(doc->pageCount()+1));
    int pageNr = fromPageNr;

    // Print away!
    while (pageNr <= toPageNr)
    {
        this->printPage(pageNr, doc->pageCount(), &painter, doc, body);
        m_progressReport->tick();

        if(pageNr < toPageNr)
        {
            if(!m_printer->newPage())
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

void QTextDocumentPagedPrinter::loadSettings(HeaderFooter *header, HeaderFooter *footer, Watermark *watermark)
{
    const QSettings *settings = Application::instance()->settings();
    auto fetchField = [settings](const QString &key, HeaderFooter::Field defaultValue) {
        const QString settingsKey = "PageSetup/" + key;
        const QVariant val = settings->value(settingsKey);
        const int min = HeaderFooter::Nothing;
        const int max = HeaderFooter::PageNumberOfCount;
        if(!val.isValid() || val.toInt() < min || val.toInt() > max)
            return defaultValue;
        return HeaderFooter::Field(val.toInt());
    };

    if(header != nullptr)
    {
        header->setLeft(fetchField("headerLeft", HeaderFooter::Title));
        header->setCenter(fetchField("headerCenter", HeaderFooter::Subtitle));
        header->setRight(fetchField("headerRight", HeaderFooter::PageNumber));

        const QVariant val = settings->value("PageSetup/headerOpacity");
        if(val.isValid())
            header->setOpacity( qBound(0.0,val.toDouble(),1.0) );
    }

    if(footer != nullptr)
    {
        footer->setLeft(fetchField("footerLeft", HeaderFooter::Author));
        footer->setCenter(fetchField("footerCenter", HeaderFooter::Version));
        footer->setRight(fetchField("footerRight", HeaderFooter::Contact));

        const QVariant val = settings->value("PageSetup/footerOpacity");
        if(val.isValid())
            footer->setOpacity( qBound(0.0,val.toDouble(),1.0) );
    }

    auto fetchSetting = [settings](const QString &key, const QVariant &defaultValue) {
        const QVariant val = settings->value("PageSetup/" + key);
        return val.isValid() ? val : defaultValue;
    };

    if(watermark != nullptr)
    {
        watermark->setEnabled( fetchSetting("watermarkEnabled", watermark->isEnabled()).toBool() );
        watermark->setText( fetchSetting("watermarkText", watermark->text()).toString() );

        QFont watermarkFont;
        watermarkFont.setFamily( fetchSetting("watermarkFont", "Courier Prime").toString() );
        watermarkFont.setPointSize( fetchSetting("watermarkFontSize", 120).toInt() );
        watermark->setFont(watermarkFont);

        watermark->setColor( QColor(fetchSetting("watermarkColor", "lightgray").toString()) );
        watermark->setOpacity( fetchSetting("watermarkOpacity", 0.5).toDouble() );
        watermark->setRotation( fetchSetting("watermarkRotation", 0.5).toDouble() );
        watermark->setAlignment( Qt::Alignment(fetchSetting("watermarkAlignment", Qt::AlignCenter).toInt()) );
    }
}

void QTextDocumentPagedPrinter::printPage(int pageNr, int pageCount, QPainter *painter, const QTextDocument *doc, const QRectF &body)
{
    m_header->paint(painter, m_headerRect, pageNr, pageCount);
    m_footer->paint(painter, m_footerRect, pageNr, pageCount);
    m_watermark->paint(painter, QRectF(m_headerRect.bottomLeft(), m_footerRect.topRight()), pageNr, pageCount);

#if 0
    painter->setPen(Qt::black);
    painter->drawLine(QLineF(body.left(), m_headerRect.bottom(), body.right(), m_headerRect.bottom()));
    painter->drawLine(QLineF(body.left(), m_footerRect.top(), body.right(), m_footerRect.top()));
#endif

    painter->save();
    painter->translate(body.left(), body.top() - (pageNr - 1) * body.height());
    QRectF view(0, (pageNr - 1) * body.height(), body.width(), body.height());

    QAbstractTextDocumentLayout *layout = doc->documentLayout();
    QAbstractTextDocumentLayout::PaintContext ctx;

    painter->setClipRect(view);
    ctx.clip = view;
    ctx.palette.setColor(QPalette::Text, Qt::black);

    layout->draw(painter, ctx);

    painter->restore();
}
