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

#ifndef FINALDRAFTIMPORTER_H
#define FINALDRAFTIMPORTER_H

#include <QDomDocument>
#include "abstractimporter.h"

class FinalDraftImporter : public AbstractImporter
{
    Q_OBJECT
    Q_CLASSINFO("Format", "Final Draft")
    Q_CLASSINFO("NameFilters", "Final Draft (*.fdx)")

public:
    Q_INVOKABLE explicit FinalDraftImporter(QObject *parent = nullptr);
    ~FinalDraftImporter();

    bool canImport(const QString &fileName) const;

protected:
    bool doImport(QIODevice *device); // AbstractImporter interface
};

#endif // FINALDRAFTIMPORTER_H
