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

#include "application.h"

#include <QDate>
#include <QTime>

QString Application::buildTimestamp() const
{
    static QString ret;
    if (ret.isEmpty()) {
        const QString dateString = QString::fromUtf8(__DATE__).simplified();
        const QString timeString = QString::fromUtf8(__TIME__).simplified();
        const QDate date = QDate::fromString(dateString, "MMM d yyyy");
        const QTime time = QTime::fromString(timeString, "hh:mm:ss");
        ret = date.toString(QStringLiteral("yyMMdd")) + "-"
                + time.toString(QStringLiteral("hhmmss"));
    }

    return ret;
}
