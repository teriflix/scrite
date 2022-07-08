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

#ifdef SCRITE_ENABLE_AUTOMATION

#ifndef SCRIPTAUTOMATIONSTEP_H
#define SCRIPTAUTOMATIONSTEP_H

#include "automation.h"

class ScriptAutomationStep : public AbstractAutomationStep
{
    Q_OBJECT

public:
    explicit ScriptAutomationStep(QObject *parent = nullptr);
    ~ScriptAutomationStep();

    Q_SIGNAL void runScript();

protected:
    void run();
};

#endif // SCRIPTAUTOMATIONSTEP_H

#endif // SCRITE_ENABLE_AUTOMATION
