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

pragma Singleton

import QtQuick 2.15
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

DialogLauncher {
    id: root

    parent: Scrite.window.contentItem

    function launch(screenplayAdapter) {
        if(screenplayAdapter === null) {
            MessageBox.information("Missing Screenplay",
                                   "No Screenplay was found to determine scene numbers.")
            return
        }

        return doLaunch({"screenplayAdapter": screenplayAdapter})
    }

    name: "JumpToSceneNumberDialog"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: _dialog

        required property ScreenplayAdapter screenplayAdapter

        width: 480
        height: 320

        title: "Quick Jump"

        content: Item {
            Component.onCompleted: Qt.callLater(_sceneNumberField.forceActiveFocus)

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20

                spacing: 20

                VclLabel {
                    Layout.fillWidth: true

                    wrapMode: Text.WordWrap
                    text: "Enter a scene, act or episode number to jump:"
                }

                VclTextField {
                    id: _sceneNumberField

                    Layout.fillWidth: true

                    readonly property var availableTargets: {
                        const screenplay = _dialog.screenplayAdapter.screenplay
                        const nrElements = screenplay.elementCount
                        let ret = []
                        let lastEpisodeDesc = ""
                        for(let i=0; i<nrElements; i++) {
                            let elementDesc = ""
                            const element = screenplay.elementAt(i)
                            if(element.elementType === ScreenplayElement.SceneElementType) {
                                const sceneHeading = element.scene.heading
                                if(sceneHeading.enabled)
                                    elementDesc = element.resolvedSceneNumber + ": " + sceneHeading.displayText
                                else
                                    elementDesc = "#" + (i+1) + ": NO SCENE HEADING"
                            } else {
                                if(element.breakType !== Screenplay.Episode && lastEpisodeDesc !== "")
                                    elementDesc = lastEpisodeDesc + ", "
                                elementDesc += element.breakTitle
                                if(element.breakSubtitle !== "")
                                    elementDesc += ": " + element.breakSubtitle
                                if(element.breakType === Screenplay.Episode)
                                    lastEpisodeDesc = elementDesc
                            }
                            ret.push(elementDesc)
                        }

                        return ret
                    }

                    completionStrings: availableTargets
                    placeholderText: length > 0 ? "" : "1, 2, 2A, ACT 1"

                    onEditingComplete: {
                        const index = availableTargets.indexOf(text)
                        if(index >= 0)
                            _dialog.screenplayAdapter.currentIndex = index
                        _dialog.close()
                    }
                }

                VclButton {
                    Layout.alignment: Qt.AlignRight

                    text: "Ok"

                    onClicked: _sceneNumberField.editingComplete()
                }
            }
        }
    }
}
