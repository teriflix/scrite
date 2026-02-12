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

        active: allowed && Runtime.loadMainUiContent && !Scrite.document.loading
        clip: true
        opacity: _private.busyMessage.visible ? 0 : 1

        sourceComponent: {
            switch(Runtime.mainWindowTab) {
            case Runtime.MainWindowTab.ScreenplayTab: return _private.screenplayTab
            case Runtime.MainWindowTab.StructureTab: return _private.structureTab
            case Runtime.MainWindowTab.NotebookTab: return Runtime.showNotebookInStructure ? _private.structureTab : _private.notebookTab
            case Runtime.MainWindowTab.ScritedTab: return _private.scritedTab
            default: break
            }

            return _private.unknownTab
        }
    }

    QtObject {
        id: _private

        readonly property Component screenplayTab: ScreenplayTab { }
        readonly property Component structureTab: StructureTab { }
        readonly property Component notebookTab: NotebookTab { }
        readonly property Component scritedTab: ScritedTab { }
        readonly property Component unknownTab: Item {
            VclLabel {
                anchors.centerIn: parent

                text: "Unknown Tab"

                font.bold: true
                font.pointSize: Runtime.idealFontMetrics.font.pointSize + 5
            }
        }

        readonly property BusyMessage busyMessage: BusyMessage {
            message: "Loading tab ..."

            function aboutToSwitchTab(from, to) {
                visible = true
            }

            function finishedTabSwitch(to) {
                visible = false
            }
        }

        readonly property Connections runtimeConnections: Connections {
            target: Runtime

            function onAboutToSwitchTab(from, to) {
                _private.busyMessage.aboutToSwitchTab(from, to)
            }

            function onFinishedTabSwitch(to) {
                _private.busyMessage.finishedTabSwitch(to)
            }
        }
    }
}
