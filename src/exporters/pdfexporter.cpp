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

#include "pdfexporter.h"
// #include "application.h"
#include "qtextdocumentpagedprinter.h"

#include <QDir>
#include <QPrinter>
#include <QSettings>
#include <QFileInfo>
#include <QPdfWriter>
#include <QTextCursor>
#include <QFontMetrics>
#include <QTextDocument>
#include <QObjectCleanupHandler>
#include <QAbstractTextDocumentLayout>
#include <QPainter>

PdfExporter::PdfExporter(QObject *parent) : AbstractTextDocumentExporter(parent) { }

PdfExporter::~PdfExporter() { }

void PdfExporter::setGenerateTitlePage(bool val)
{
    if (m_generateTitlePage == val)
        return;

    m_generateTitlePage = val;
    emit generateTitlePageChanged();
}

void PdfExporter::setIncludeLogline(bool val)
{
    if (m_includeLogline == val)
        return;

    m_includeLogline = val;
    emit includeLoglineChanged();
}

void PdfExporter::setIncludeSceneNumbers(bool val)
{
    if (m_includeSceneNumbers == val)
        return;

    m_includeSceneNumbers = val;
    emit includeSceneNumbersChanged();
}

void PdfExporter::setIncludeSceneIcons(bool val)
{
    if (m_includeSceneIcons == val)
        return;

    m_includeSceneIcons = val;
    emit includeSceneIconsChanged();
}

void PdfExporter::setPrintEachSceneOnANewPage(bool val)
{
    if (m_printEachSceneOnANewPage == val)
        return;

    m_printEachSceneOnANewPage = val;
    emit printEachSceneOnANewPageChanged();

    if (val)
        this->setPrintEachActOnANewPage(false);
}

void PdfExporter::setPrintEachActOnANewPage(bool val)
{
    if (m_printEachActOnANewPage == val)
        return;

    m_printEachActOnANewPage = val;
    emit printEachActOnANewPageChanged();

    if (val)
        this->setPrintEachSceneOnANewPage(false);
}

void PdfExporter::setIncludeActBreaks(bool val)
{
    if (m_includeActBreaks == val)
        return;

    m_includeActBreaks = val;
    emit includeActBreaksChanged();
}

void PdfExporter::setUsePageBreaks(bool val)
{
    if (m_usePageBreaks == val)
        return;

    m_usePageBreaks = val;
    emit usePageBreaksChanged();
}

void PdfExporter::setWatermark(const QString &val)
{
    if (m_watermark == val)
        return;

    m_watermark = val;
    emit watermarkChanged();
}

void PdfExporter::setComment(const QString &val)
{
    if (m_comment == val)
        return;

    m_comment = val;
    emit commentChanged();
}

class PdfSideBar : public QTextDocumentPageSideBarInterface
{
public:
    // AbstractPageSideBar interface
    void paint(QPainter *paint, Side side, const QRectF &rect, const QRectF &docRect)
    {
#if 0
        if (side == RightSide)
            paint->fillRect(rect, Qt::yellow);
#else
        Q_UNUSED(paint)
        Q_UNUSED(side)
        Q_UNUSED(rect)
        Q_UNUSED(docRect)
#endif
    }
};

bool PdfExporter::doExport(QIODevice *device)
{
    Screenplay *screenplay = this->document()->screenplay();
    ScreenplayFormat *format = this->document()->printFormat();

    // Qt 5.15.7's PdfWriter is broken!
#if 0
    const bool usePdfWriter = Application::instance()
                                      ->settings()
                                      ->value(QStringLiteral("PdfExport/usePdfDriver"), true)
                                      .toBool();
#else
    const bool usePdfWriter = false;
#endif

    QScopedPointer<QPdfWriter> qpdfWriter;
    QScopedPointer<QPrinter> qprinter;
    QPagedPaintDevice *pdfDevice = nullptr;

    if (usePdfWriter) {
        qpdfWriter.reset(new QPdfWriter(device));
        qpdfWriter->setTitle(screenplay->title());
        qpdfWriter->setCreator(qApp->applicationName() + QStringLiteral(" ")
                               + qApp->applicationVersion() + QStringLiteral(" PdfWriter"));
        format->pageLayout()->configure(qpdfWriter.data());
        qpdfWriter->setPageMargins(QMarginsF(0.2, 0.1, 0.2, 0.1), QPageLayout::Inch);
        qpdfWriter->setPdfVersion(QPagedPaintDevice::PdfVersion_1_6);

        pdfDevice = qpdfWriter.data();
    } else {
        const QString pdfFileName = QDir::tempPath() + QStringLiteral("/scrite-pdfexporter-")
                + QString::number(QDateTime::currentSecsSinceEpoch()) + QStringLiteral(".pdf");

        qprinter.reset(new QPrinter);
        qprinter->setOutputFormat(QPrinter::PdfFormat);
        qprinter->setOutputFileName(pdfFileName);

        qprinter->setPdfVersion(QPagedPaintDevice::PdfVersion_1_6);
        qprinter->setDocName(screenplay->title());
        qprinter->setCreator(qApp->applicationName() + QStringLiteral(" ")
                             + qApp->applicationVersion() + QStringLiteral(" Printer"));
        format->pageLayout()->configure(qprinter.data());
        qprinter->setPageMargins(QMarginsF(0.2, 0.1, 0.2, 0.1), QPageLayout::Inch);

        pdfDevice = qprinter.data();
    }

    const qreal pageWidth = pdfDevice->width();
    QTextDocument textDocument;
    this->AbstractTextDocumentExporter::generate(&textDocument, pageWidth);
    textDocument.setProperty("#comment", m_comment);
    textDocument.setProperty("#watermark", m_watermark);

    PdfSideBar sideBar;

    QTextDocumentPagedPrinter printer;
    printer.setSideBar(&sideBar);
    printer.header()->setVisibleFromPageOne(!m_generateTitlePage);
    printer.footer()->setVisibleFromPageOne(!m_generateTitlePage);
    printer.watermark()->setVisibleFromPageOne(!m_generateTitlePage);
    bool success = printer.print(&textDocument, pdfDevice);
    if (!qprinter.isNull()) {
        const QString pdfFileName = qprinter->outputFileName();
        if (success) {
            QFile pdfFile(pdfFileName);
            pdfFile.open(QFile::ReadOnly);

            const int bufferSize = 65535;
            while (1) {
                const QByteArray bytes = pdfFile.read(bufferSize);
                if (bytes.isEmpty())
                    break;
                device->write(bytes);
            }
        }

        QFile::remove(pdfFileName);
    }

    return success;
}
