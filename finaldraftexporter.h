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

#ifndef FINALDRAFTEXPORTER_H
#define FINALDRAFTEXPORTER_H

#include "abstractexporter.h"

class FinalDraftExporter : public AbstractExporter
{
    Q_OBJECT
    Q_CLASSINFO("Format", "Final Draft")
    Q_CLASSINFO("NameFilters", "Final Draft (*.fdx)")

public:
    Q_INVOKABLE FinalDraftExporter(QObject *parent=nullptr);
    ~FinalDraftExporter();

protected:
    bool doExport(QIODevice *device);
};

#endif // FINALDRAFTEXPORTER_H
