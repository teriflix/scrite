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

#ifndef LOCATIONREPORT_H
#define LOCATIONREPORT_H

#include "abstractreportgenerator.h"

class LocationReport : public AbstractReportGenerator
{
    Q_OBJECT
    Q_CLASSINFO("Title", "Location Report")
    Q_CLASSINFO("Description", "Generate a summary report of all locations in the screenplay.")

public:
    Q_INVOKABLE explicit LocationReport(QObject *parent = nullptr);
    ~LocationReport();

    bool requiresConfiguration() const { return true; }

protected:
    // AbstractReportGenerator interface
    bool doGenerate(QTextDocument *textDocument);
};

#endif // LOCATIONREPORT_H
