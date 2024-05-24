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

#include "fountainexporter.h"
#include "fountain.h"

#include <QFileInfo>

FountainExporter::FountainExporter(QObject *parent) : AbstractExporter(parent) { }

FountainExporter::~FountainExporter() { }

bool FountainExporter::doExport(QIODevice *device)
{
    const Screenplay *screenplay = this->document()->screenplay();
    Fountain::Writer writer(screenplay);
    return writer.write(device);
}
