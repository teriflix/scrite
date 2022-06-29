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

#ifndef HOURGLASS_H
#define HOURGLASS_H

#include <QCursor>
#include <QGuiApplication>

class HourGlass
{
public:
    HourGlass(const QCursor &cursor = QCursor(Qt::WaitCursor))
    {
        qApp->setOverrideCursor(cursor);
        // qApp->processEvents();
    }

    ~HourGlass()
    {
        qApp->restoreOverrideCursor();
        // qApp->processEvents();
    }
};

#endif // HOURGLASS_H
