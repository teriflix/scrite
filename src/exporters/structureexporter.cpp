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

#include "structureexporter.h"
#include "structureexporter_p.h"

#include "application.h"

#include <QDir>
#include <QPainter>
#include <QFileInfo>
#include <QPdfWriter>
#include <QPainterPath>
#include <QAbstractTextDocumentLayout>

StructureExporter::StructureExporter(QObject *parent)
    :AbstractExporter(parent)
{

}

StructureExporter::~StructureExporter()
{

}

bool StructureExporter::doExport(QIODevice *device)
{
#ifdef Q_OS_MAC
    const qreal dpi = 72.0;
#else
    const qreal dpi = 96.0;
#endif

    const Screenplay *screenplay = this->document()->screenplay();
    const Structure *structure = this->document()->structure();

    if(structure->canvasUIMode() != Structure::IndexCardUI)
    {
        this->error()->setErrorMessage( QStringLiteral("Only index card based structures can be exported.") );
        return false;
    }

    // Construct the graphics scene with content of the structure
    StructureExporterScene scene(structure);

    // How big is the scene?
    QRectF sceneRect = scene.itemsBoundingRect();

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
    pdfWriter.setTitle(screenplay->title() + QStringLiteral(" - Structure"));
    pdfWriter.setCreator(qApp->applicationName() + " " + qApp->applicationVersion());
    pdfWriter.setPageSize(pageSize);

    const qreal dpiScaleX = qreal(pdfWriter.logicalDpiX()) / dpi;
    const qreal dpiScaleY = qreal(pdfWriter.logicalDpiY()) / dpi;

    QPainter paint(&pdfWriter);
    paint.setRenderHint(QPainter::Antialiasing);
    paint.setRenderHint(QPainter::SmoothPixmapTransform);
    paint.scale(dpiScaleX, dpiScaleY);
    scene.render(&paint, targetRect, sceneRect, Qt::KeepAspectRatio);
    paint.end();

    return true;
}

QString StructureExporter::polishFileName(const QString &fileName) const
{
    QFileInfo fi(fileName);

    QString baseName = fi.baseName();
    baseName.replace( QStringLiteral("Screenplay"), QStringLiteral("Structure") );

    return fi.absoluteDir().absoluteFilePath( baseName + QStringLiteral(".pdf") );
}

