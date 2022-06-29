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

#include <QQmlEngine>
#include <QQuickView>
#include <QQmlContext>

#include "pausestep.h"
#include "automation.h"
#include "application.h"
#include "windowcapture.h"
#include "automationrecorder.h"
#include "eventautomationstep.h"
#include "scriptautomationstep.h"

void Automation::init(QQuickView *qmlWindow)
{
    qmlRegisterType<WindowCapture>("Scrite", 1, 0, "WindowCapture");

#ifdef SCRITE_ENABLE_AUTOMATION
    qmlRegisterUncreatableType<AbstractAutomationStep>(
            "Scrite", 1, 0, "AbstractAutomationStep",
            QStringLiteral("Create concrete steps instead"));
    qmlRegisterType<PauseStep>("Scrite", 1, 0, "PauseStep");
    qmlRegisterType<Automation>("Scrite", 1, 0, "Automation");
    qmlRegisterType<EventAutomationStep>("Scrite", 1, 0, "EventStep");
    qmlRegisterType<ScriptAutomationStep>("Scrite", 1, 0, "ScriptStep");

    new AutomationRecorder(qmlWindow);

    const QString automationScript = QString::fromLatin1(qgetenv("SCRITE_AUTOMATION_SCRIPT"));
    if (QFile::exists(automationScript))
        qmlWindow->engine()->rootContext()->setContextProperty(
                "automationScript", QUrl::fromLocalFile(automationScript));
    else
#else
    Q_UNUSED(qmlWindow)
#endif
        qmlWindow->engine()->rootContext()->setContextProperty("automationScript", QString());
}
