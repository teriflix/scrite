/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
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
#include "screenplayformat.h"
#include "screenplay.h"
#include "scritedocument.h"

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

void FountainExporter::setIncludeTitlePage(bool val)
{
    if (m_includeTitlePage == val)
        return;

    m_includeTitlePage = val;
    emit includeTitlePageChanged();
}

void FountainExporter::setIncludeActBreaks(bool val)
{
    if (m_includeActBreaks == val)
        return;

    m_includeActBreaks = val;
    emit includeActBreaksChanged();
}

void FountainExporter::setIncludeEpisodeBreaks(bool val)
{
    if (m_includeEpisodeBreaks == val)
        return;

    m_includeEpisodeBreaks = val;
    emit includeEpisodeBreaksChanged();
}

bool FountainExporter::doExport(QIODevice *device)
{
    const Screenplay *screenplay = this->document()->screenplay();

    int options = 0;
    if (m_useEmphasis)
        options += Fountain::Writer::EmphasisOption;
    if (m_followStrictSyntax)
        options += Fountain::Writer::StrictSyntaxOption;

    Fountain::TitlePage titlePage;
    Fountain::Body body;

    if (m_includeTitlePage)
        Fountain::populateTitlePage(screenplay, titlePage);

    Fountain::populateBody(screenplay, body, [=](const ScreenplayElement *element) -> bool {
        if (element->elementType() == ScreenplayElement::BreakElementType) {
            switch (element->breakType()) {
            case Screenplay::Act:
                return m_includeActBreaks;
            case Screenplay::Episode:
                return m_includeEpisodeBreaks;
            default:
                break;
            }
            return false;
        }
        return true;
    });

    Fountain::Writer writer(titlePage, body, options);
    return writer.write(device);
}
