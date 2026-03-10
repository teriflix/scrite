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

pragma Singleton

import QtQuick
import QtQuick.Dialogs
import QtQuick.Layouts
import QtQuick.Controls

import io.scrite.components


import "../globals"
import "../dialogs"
import "../controls"

QtObject {
    id: root

    // Functions to safely open task, without compromising on UI state.
    function open(filePath) {
        return theTask.createObject(root, {"filePath": filePath, "mode": "open"})
    }

    function openAnonymously(filePath) {
        return theTask.createObject(root, {"filePath": filePath, "mode": "openAnonymously"})
    }

    function openOrImport(filePath) {
        return theTask.createObject(root, {"filePath": filePath, "mode": "openOrImport"})
    }

    readonly property Component theTask: SequentialAnimation {
        id: _theTaskInstance

        property VclDialog waitDialog
        property string filePath
        property string mode // can be one of ["open", "openAnonymously", "openOrImport"]

        running: true

        PauseAnimation {
            duration: 10
        }

        ScriptAction {
            script: {
                Runtime.activateMainWindowTab(Runtime.MainWindowTab.ScreenplayTab)
                _theTaskInstance.waitDialog = WaitDialog.launch()
            }
        }

        PauseAnimation {
            duration: 200
        }

        ScriptAction {
            script: {
                Runtime.loadMainUiContent = false
            }
        }

        PauseAnimation {
            duration: 200
        }

        ScriptAction {
            script: {
                const modes = ["open", "openAnonymously", "openOrImport"]
                const modeIndex = modes.indexOf(_theTaskInstance.mode)

                switch(modeIndex) {
                case 0:
                    Scrite.document.open(_theTaskInstance.filePath)
                    break
                case 1:
                    Scrite.document.openAnonymously(_theTaskInstance.filePath)
                    break
                case 2:
                    Scrite.document.openOrImport(_theTaskInstance.filePath)
                    break
                }
                Runtime.loadMainUiContent = true
            }
        }

        PauseAnimation {
            duration: 200
        }

        ScriptAction {
            script: {
                Scrite.document.justLoaded()
                if(_theTaskInstance.waitDialog)
                    _theTaskInstance.waitDialog.close()
                Qt.callLater(_theTaskInstance.destroy)
            }
        }
    }
}
