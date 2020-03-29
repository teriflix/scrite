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

#ifndef FINALDRAFTIMPORTER_H
#define FINALDRAFTIMPORTER_H

#include "abstractimporter.h"

class FinalDraftImporter : public AbstractImporter
{
    Q_OBJECT
    Q_CLASSINFO("Format", "Final Draft")
    Q_CLASSINFO("NameFilters", "Final Draft (*.fdx)")

public:
    Q_INVOKABLE FinalDraftImporter(QObject *parent=nullptr);
    ~FinalDraftImporter();

protected:
    bool doImport(QIODevice *device); // AbstractImporter interface
};

#endif // FINALDRAFTIMPORTER_H
