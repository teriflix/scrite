/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "structureexporter.h"

#include <QFileInfo>

StructureExporter::StructureExporter(QObject *parent)
                  :AbstractExporter(parent)
{

}

StructureExporter::~StructureExporter()
{

}

bool StructureExporter::doExport(QIODevice *device)
{
    device->close();
    this->document()->structure()->captureStructureAsImage(this->fileName());
    return QFile::exists(this->fileName());
}

QString StructureExporter::polishFileName(const QString &fileName) const
{
    QFileInfo fi(fileName);
    if(fi.suffix().toLower() != "png")
        return fileName + ".png";
    return fileName;
}
