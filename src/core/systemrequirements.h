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

#ifndef SYSTEMREQUIREMENTS_H
#define SYSTEMREQUIREMENTS_H

#include <QList>
#include <QString>

class QProgressDialog;
class SystemRequirements
{
public:
    enum Aspect {
        CpuCore,
        ScreenResolution,
        RAM,
        DiskSpace,
        OpenGLSupport,
        MinimumOSVersion,
        SupportedScriteVersion,
        AspectCount // Never check for this
    };

    // Describes the aspects passed in the list as parameter. Useful for showing
    // error on the screen if something goes wrong
    static QString describe(const QList<Aspect> aspects = QList<Aspect>());

    // Checks aspects passed in the list, and returns the checks that passed.
    // If no list is passed, then all aspects are checked
    static QList<Aspect> check(const QList<Aspect> aspects = QList<Aspect>(),
                               QProgressDialog *progressDialog = nullptr);

    // Checks and reports errors if any on a QMessageBox
    static bool checkAndReport(const QList<Aspect> aspects = QList<Aspect>());

private:
    // Individual Check Methods
    static bool hasMinimumCpuCores(int minCores = 2);
    static bool hasMinimumScreenResolution(int minWidth = 1366, int minHeight = 768);
    static bool hasMinimumRam(qint64 minRamGb = 8);
    static bool hasMinimumFreeDiskSpace(qint64 minSpaceGb = 10);
    static bool hasMinimumVram(qint64 minVramMb = 1024);
    static bool hasOpenGlSupport();
    static bool hasMinimumOSVersion();
    static bool hasSupportedScriteVersion();
};

#endif // SYSTEMREQUIREMENTS_H
