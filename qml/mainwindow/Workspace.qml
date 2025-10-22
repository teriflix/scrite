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


import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/overlays"

Rectangle {
    id: _workspace

    function reset() { _workspaceLoader.reset() }

    z: 0
    color: Runtime.colors.primary.windowColor

    Loader {
        id: _workspaceLoader

        property int currentTab: Runtime.MainWindowTab.ScreenplayTab
        property bool allowed: false

        function reset(callback, delay) {
            allowed = false

            Runtime.execLater(_workspaceLoader, delay ? delay : 250, (callback) => {
                                  if(callback)
                                      callback()
                                  _workspaceLoader.allowed = true
                              }, callback)
        }

        Component.onCompleted: Runtime.resetMainWindowUi.connect(reset)

        Behavior on opacity {
            enabled: Runtime.applicationSettings.enableAnimations
            NumberAnimation { duration: Runtime.stdAnimationDuration }
        }

        anchors.fill: parent

        clip: true
        opacity: _private.busyMessage.visible ? 0 : 1
        active: allowed && Runtime.loadMainUiContent && !Scrite.document.loading

        sourceComponent: {
            switch(currentTab) {
            case Runtime.MainWindowTab.ScreenplayTab: return _private.screenplayTab
            case Runtime.MainWindowTab.StructureTab: return _private.structureTab
            case Runtime.MainWindowTab.NotebookTab: return _private.notebookTab
            case Runtime.MainWindowTab.ScritedTab: return _private.scritedTab
            default: break
            }

            return _private.screenplayTab
        }
    }

    QtObject {
        id: _private

        readonly property Component screenplayTab: ScreenplayTab { }
        readonly property Component structureTab: StructureTab { }
        readonly property Component notebookTab: NotebookTab { }
        readonly property Component scritedTab: ScritedTab { }
        readonly property BusyMessage busyMessage: BusyMessage {
            message: "Loading tab ..."
        }
        readonly property SequentialAnimation tabSwitchProcess: SequentialAnimation {
            loops: 1

            ScriptAction {
                script: {
                    _private.busyMessage.visible = true
                }
            }

            PauseAnimation {
                duration: Runtime.applicationSettings.enableAnimations ? (Runtime.stdAnimationDuration+50) : Runtime.stdAnimationDuration/2
            }

            ScriptAction {
                script: {
                    _workspaceLoader.currentTab = _private.currentTab
                }
            }

            PauseAnimation {
                duration: Runtime.applicationSettings.enableAnimations ? (Runtime.stdAnimationDuration+50) : 0
            }

            ScriptAction {
                script: {
                    _private.busyMessage.visible = false
                }
            }
        }

        property int currentTab: Runtime.mainWindowTab
        onCurrentTabChanged: tabSwitchProcess.start()

    }
}
