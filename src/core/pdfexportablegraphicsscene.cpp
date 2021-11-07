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

#include <QFile>
#include <QPainter>
#include <QPdfWriter>
#include <QGuiApplication>

PdfExportableGraphicsScene::PdfExportableGraphicsScene(QObject *parent)
    : QGraphicsScene(parent)
{

}

PdfExportableGraphicsScene::~PdfExportableGraphicsScene()
{

}

void PdfExportableGraphicsScene::setPdfTitle(QString val)
{
    if(m_pdfTitle == val)
        return;

    m_pdfTitle = val;
    emit pdfTitleChanged();
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
    pdfWriter.setTitle(m_pdfTitle);
    pdfWriter.setCreator(qApp->applicationName() + " " + qApp->applicationVersion());
    pdfWriter.setPageSize(pageSize);

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
