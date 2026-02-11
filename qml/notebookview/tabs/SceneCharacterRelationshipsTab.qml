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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/notebookview"
import "qrc:/qml/structureview"
import "qrc:/qml/notebookview/helpers"

Item {
    id: root

    property Scene scene

    signal switchRequest(var item) // could be string, or any of the notebook objects like Notes, Character etc.

    DisabledFeatureNotice {
        anchors.fill: parent

        color: Qt.rgba(0,0,0,0)
        featureName: "Relationship Map"
        visible: !Runtime.appFeatures.characterRelationshipGraph.enabled
    }

    Loader {
        id: _graphLoader

        function reload() {
            active = false
            Qt.callLater(activate)
        }

        function activate() {
            active = Runtime.appFeatures.characterRelationshipGraph.enabled
        }

        anchors.fill: parent

        active: Runtime.appFeatures.characterRelationshipGraph.enabled

        sourceComponent: CharacterRelationshipsGraphView {
            id: _graph

            property bool pdfExportPossible: !graphIsEmpty && visible

            Component.onCompleted: {
                scene = root.scene
                showBusyIndicator = false
            }

            scene: null
            showBusyIndicator: true

            onCharacterDoubleClicked: (characterName, nodeItem) => {
                var ch = Scrite.document.structure.findCharacter(characterName)
                if(ch)
                    root.switchRequest(ch.notes)
            }

            ActionHandler {
                action: ActionHub.notebookOperations.find("reload")
                priority: 2

                onTriggered: _graph.resetGraph()
            }

            ActionHandler {
                action: ActionHub.notebookOperations.find("report")
                enabled: _graph.pdfExportPossible
                priority: 2

                onTriggered: (source) => { _graph.exportToPdf(source) }
            }
        }
    }

    onSceneChanged: _graphLoader.reload()
}
