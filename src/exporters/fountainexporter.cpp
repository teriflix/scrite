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

void FountainExporter::setFollowStrictSyntax(bool val)
{
    if (m_followStrictSyntax == val)
        return;

    m_followStrictSyntax = val;
    emit followStrictSyntaxChanged();
}

void FountainExporter::setUseEmphasis(bool val)
{
    if (m_useEmphasis == val)
        return;

    m_useEmphasis = val;
    emit useEmphasisChanged();
}

bool FountainExporter::doExport(QIODevice *device)
{
    const Screenplay *screenplay = this->document()->screenplay();

    int options = 0;
    if (m_useEmphasis)
        options += Fountain::Writer::EmphasisOption;
    if (m_followStrictSyntax)
        options += Fountain::Writer::StrictSyntaxOption;

    Fountain::Writer writer(screenplay, options);
    return writer.write(device);
}
