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

#include "scriptautomationstep.h"

ScriptAutomationStep::ScriptAutomationStep(QObject *parent) : AbstractAutomationStep(parent) { }

ScriptAutomationStep::~ScriptAutomationStep() { }

void ScriptAutomationStep::run()
{
    emit runScript();
    this->finish();
}

#endif // SCRITE_ENABLE_AUTOMATION
