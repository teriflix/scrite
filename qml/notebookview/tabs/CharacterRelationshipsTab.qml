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
import "qrc:/qml/notebookview/helpers"

Item {
    id: root

    required property Character character

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

        Component.onCompleted: Runtime.execLater(_graphLoader, Runtime.stdAnimationDuration/2, activate)

        anchors.fill: parent

        active: Runtime.appFeatures.characterRelationshipGraph.enabled

        sourceComponent: CharacterRelationshipsGraphView {
            id: _graph

            property bool pdfExportPossible: !graphIsEmpty && visible

            Component.onCompleted: {
                character = root.character
                structure = Scrite.document.structure
                showBusyIndicator = false
            }

            scene: null
            character: null
            structure: null
            showBusyIndicator: true
            editRelationshipsEnabled: !Scrite.document.readOnly

            onCharacterDoubleClicked: (characterName, nodeItem) => {
                                          let ch = Scrite.document.structure.findCharacter(characterName)
                                          if(ch) {
                                              if(ch === root.character) {
                                                  AddRelationshipDialog.launch(root.character)
                                              } else {
                                                  root.switchRequest(ch.notes)
                                              }
                                          }
                                      }

            onAddNewRelationshipRequest: (nodeItem) => {
                                             AddRelationshipDialog.launch(root.character)
                                         }

            removeRelationshipWithRequest: (otherCharacter, nodeItem) => {
                                               let relationship = root.character.findRelationship(otherCharacter)
                                               root.character.removeRelationship(relationship)
                                           }

            ActionHandler {
                action: ActionHub.notebookOperations.find("reload")

                onTriggered: _graph.resetGraph()
            }

            ActionHandler {
                action: ActionHub.notebookOperations.find("report")
                enabled: _graph.pdfExportPossible

                onTriggered: (source) => { _graph.exportToPdf(source) }
            }
        }
    }
}
