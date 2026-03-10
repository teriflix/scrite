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

    function openTemplateAt(libraryService, index) {
        return theTask.createObject(root, {"index": index, "libraryService": libraryService, "mode": "template"})
    }

    function openScreenplayAt(libraryService, index) {
        return theTask.createObject(root, {"index": index, "libraryService": libraryService, "mode": "screenplay"})
    }

    readonly property Component theTask: QtObject {
        id: _theTaskInstance

        property int index
        property string mode // can be one of ["template", "screenplay"]
        property LibraryService libraryService

        property Animation firstHalfTask
        property Animation secondHalfTask

        signal finished()

        Component.onCompleted: {
            firstHalfTask = theFirstHalfTask.createObject(_theTaskInstance, {"index": index, "libraryService": libraryService, "mode": mode})

            secondHalfTask = theSecondHalfTask.createObject(_theTaskInstance)
            secondHalfTask.finished.connect(finished)
            secondHalfTask.finished.connect(cleanupLater)

            libraryService.importFinished.connect(startSecondHalfTask)

            firstHalfTask.start()
        }

        function startSecondHalfTask() {
            secondHalfTask.start()
        }

        function cleanupLater() { Qt.callLater(cleanup) }

        function cleanup() {
            if(firstHalfTask.waitDialog)
                firstHalfTask.waitDialog.close()
            Qt.callLater(destroy)
        }
    }

    readonly property Component theFirstHalfTask: SequentialAnimation {
        id: _theFirstHalfTaskInstance

        property int index
        property string mode // can be one of ["template", "screenplay"]
        property LibraryService libraryService

        property VclDialog waitDialog

        running: false

        PauseAnimation {
            duration: 10
        }

        ScriptAction {
            script: {
                Runtime.activateMainWindowTab(Runtime.MainWindowTab.ScreenplayTab)
                _theFirstHalfTaskInstance.waitDialog = WaitDialog.launch()
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
                const modes = ["template", "screenplay"]
                const modeIndex = modes.indexOf(_theFirstHalfTaskInstance.mode)

                switch(modeIndex) {
                case 0:
                    _theFirstHalfTaskInstance.libraryService.openTemplateAt(_theFirstHalfTaskInstance.index)
                    break
                case 1:
                    _theFirstHalfTaskInstance.libraryService.openScreenplayAt(_theFirstHalfTaskInstance.index)
                    break
                }
            }
        }
    }

    readonly property Component theSecondHalfTask : SequentialAnimation {
        id: _theSecondHalfTaskInstance

        running: false

        ScriptAction {
            script: {
                Runtime.loadMainUiContent = true
            }
        }

        PauseAnimation {
            duration: 200
        }

        ScriptAction {
            script: {
                Scrite.document.justLoaded()
            }
        }
    }
}
