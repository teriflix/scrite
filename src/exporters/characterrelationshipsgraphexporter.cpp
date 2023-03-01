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

#include "characterrelationshipsgraphexporter.h"
#include "characterrelationshipsgraphexporter_p.h"

#include "characterrelationshipgraph.h"

#include <QStandardPaths>

CharacterRelationshipsGraphExporter::CharacterRelationshipsGraphExporter(QObject *parent)
    : AbstractExporter(parent)
{
    this->setGraph(qobject_cast<CharacterRelationshipGraph *>(this));
}

CharacterRelationshipsGraphExporter::~CharacterRelationshipsGraphExporter() { }

void CharacterRelationshipsGraphExporter::setGraph(CharacterRelationshipGraph *val)
{
    if (m_graph == val)
        return;

    m_graph = val;

    const QFileInfo fi(this->fileName());
    const QDir dir = fi.isFile()
            ? fi.absoluteDir()
            : QDir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
    const QString defaultFileName = QStringLiteral("Character Relationship Graph");

    if (m_graph == nullptr)
        this->setFileName(dir.absoluteFilePath(defaultFileName));
    else {
        QString title = m_graph->title();
        title = title.remove(QStringLiteral("\""));
        this->setFileName(dir.absoluteFilePath(title));
    }
}

void CharacterRelationshipsGraphExporter::setEnableHeaderFooter(bool val)
{
    if (m_enableHeaderFooter == val)
        return;

    m_enableHeaderFooter = val;
    emit enableHeaderFooterChanged();
}

void CharacterRelationshipsGraphExporter::setWatermark(const QString &val)
{
    if (m_watermark == val)
        return;

    m_watermark = val;
    emit watermarkChanged();
}

void CharacterRelationshipsGraphExporter::setComment(const QString &val)
{
    if (m_comment == val)
        return;

    m_comment = val;
    emit commentChanged();
}

bool CharacterRelationshipsGraphExporter::doExport(QIODevice *device)
{
    this->error()->clear();

    if (m_graph == nullptr || m_graph->isEmpty()) {
        this->error()->setErrorMessage(QStringLiteral("No graph to export."));
        return false;
    }

    CharacterRelationshipsGraphScene scene(m_graph, this);
    scene.setTitle(m_graph->title());
    scene.setComment(m_comment);
    scene.setWatermark(m_watermark);
    if (m_enableHeaderFooter)
        scene.addStandardItems(PdfExportableGraphicsScene::WatermarkOverlayLayer
                               | PdfExportableGraphicsScene::FooterLayer
                               | PdfExportableGraphicsScene::DontIncludeScriteLink);
    else
        scene.addStandardItems(PdfExportableGraphicsScene::WatermarkOverlayLayer);
    return scene.exportToPdf(device);
}
