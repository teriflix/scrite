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

#include "logger.h"
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
    if(m_type == Header)
    {
        m_columns[0].flags = Qt::AlignLeft|Qt::AlignVCenter|Qt::TextWordWrap;
        m_columns[1].flags = Qt::AlignHCenter|Qt::AlignVCenter|Qt::TextWordWrap;
        m_columns[2].flags = Qt::AlignRight|Qt::AlignVCenter|Qt::TextWordWrap;
    }
    else if(m_type == Footer)
    {
        m_columns[0].flags = Qt::AlignLeft|Qt::AlignVCenter|Qt::TextWordWrap;
        m_columns[1].flags = Qt::AlignHCenter|Qt::AlignVCenter|Qt::TextWordWrap;
        m_columns[2].flags = Qt::AlignRight|Qt::AlignVCenter|Qt::TextWordWrap;
    }
}

void HeaderFooter::paint(QPainter *paint, const QRectF &, int pageNr, int pageCount)
{
    auto updateContent = [pageNr,pageCount](ColumnContent &content, Field field) {
        if(field == PageNumber)
            content.content = QString::number(pageNr) + ".";
        else if(field == PageNumberOfCount)
            content.content = QString::number(pageNr) + "/" + QString::number(pageCount);
    };

    updateContent(m_columns[0], m_left);
    updateContent(m_columns[1], m_center);
    updateContent(m_columns[2], m_right);

    for(int i=0; i<3; i++)
    {
        if(m_columns.at(i).content.isEmpty())
            continue;
        paint->save();
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

QTextDocumentPagedPrinter::QTextDocumentPagedPrinter(QObject *parent)
    : QObject(parent)
{
    const QSettings *settings = Application::instance()->settings();
    auto fetchField = [settings](const QString &key, HeaderFooter::Field defaultValue) {
        const QString settingsKey = "PageSetup/" + key;
        const QVariant val = settings->value(settingsKey);
        qDebug() << settingsKey << val;
        const int min = HeaderFooter::Nothing;
        const int max = HeaderFooter::PageNumberOfCount;
        if(!val.isValid() || val.toInt() < min || val.toInt() > max)
            return defaultValue;
        return HeaderFooter::Field(val.toInt());
    };

    m_header->setLeft(fetchField("headerLeft", HeaderFooter::Title));
    m_header->setCenter(fetchField("headerCenter", HeaderFooter::Subtitle));
    m_header->setRight(fetchField("headerRight", HeaderFooter::PageNumber));

    m_footer->setLeft(fetchField("footerLeft", HeaderFooter::Author));
    m_footer->setCenter(fetchField("footerCenter", HeaderFooter::Version));
    m_footer->setRight(fetchField("footerRight", HeaderFooter::Contact));
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

    int dpiy = painter.device()->logicalDpiY();
    int margin = int((2.0/2.54)*dpiy); // 2 cm margins
    int padding = int((0.2/2.54)*dpiy); // 2 mm padding
    QTextFrameFormat fmt = doc->rootFrame()->frameFormat();
    fmt.setMargin(margin);
    doc->rootFrame()->setFrameFormat(fmt);

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

void QTextDocumentPagedPrinter::printPage(int pageNr, int pageCount, QPainter *painter, const QTextDocument *doc, const QRectF &body)
{
    if(pageNr > 1)
    {
        m_header->paint(painter, m_headerRect, pageNr-1, pageCount-1);
        m_footer->paint(painter, m_footerRect, pageNr-1, pageCount-1);

#if 0
        painter->setPen(Qt::black);
        painter->drawLine(QLineF(body.left(), m_headerRect.bottom(), body.right(), m_headerRect.bottom()));
        painter->drawLine(QLineF(body.left(), m_footerRect.top(), body.right(), m_footerRect.top()));
#endif
    }

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
