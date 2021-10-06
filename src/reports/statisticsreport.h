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

#ifndef STATISTICSREPORT_H
#define STATISTICSREPORT_H

#include "abstractreportgenerator.h"

class StatisticsReport : public AbstractReportGenerator
{
    Q_OBJECT
    Q_CLASSINFO("Title", "Statistics Report")
    Q_CLASSINFO("Description", "Generate a report key statistics of the screenplay.")

public:
    Q_INVOKABLE StatisticsReport(QObject *parent=nullptr);
    ~StatisticsReport();

protected:
    // AbstractReportGenerator interface
    bool doGenerate(QTextDocument *textDocument);
};

#endif // STATISTICSREPORT_H
