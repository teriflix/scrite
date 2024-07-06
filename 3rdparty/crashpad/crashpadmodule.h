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

#ifndef CRASHPADMODULE_H
#define CRASHPADMODULE_H

#include <QString>

namespace CrashpadModule {

/** Returns true if the module is available for use with Scrite **/
bool isAvailable();

/** This function checks if there is are crash-reports already existing, if yes
    then it copies them to a folder on the Desktop making it easy for the user
    to find, and also provides option to the user if he wants to reset user
    login credentials and try again.

    If this function returns false, the application must quit right away. **/
bool prepare();

/** Initializes the module and returns true if available. The initialization
    happens only once. Subsequent calls to initialize returns whatever was the
    return value of the function when it was called the first time. **/
bool initialize();

/** Returns the complete file path of crashpad_handler executable. **/
QString handlerPath();

/** Returns the complete path of the folder where Scrite's settings.ini **/
QString dataPath();

/** Code that crashes the app for sure, provided Crashpad is available and initialized. **/
inline void crash()
{
#ifdef ENABLE_CRASHPAD_CRASH_TEST
    if (CrashpadModule::initialize())
        *(volatile int *)0 = 0;
#endif
}

}

#endif // CRASHPADMODULE_H
