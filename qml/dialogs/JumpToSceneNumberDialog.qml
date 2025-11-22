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

        width: Math.max(640, Scrite.window.width*0.5)
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
                        const suffixMarker = completionIgnoreSuffixAfter
                        let ret = []
                        let lastEpisodeDesc = ""
                        let lastBreakDesc = ""
                        for(let i=0; i<nrElements; i++) {
                            const element = screenplay.elementAt(i)
                            let elementDesc = ""
                            let suffix = suffixMarker + element.serialNumber

                            if(element.elementType === ScreenplayElement.SceneElementType) {
                                const sceneHeading = element.scene.heading
                                const summary = element.scene.structureElement.hasNativeTitle ? element.scene.structureElement.nativeTitle : element.scene.summary
                                if(Runtime.sceneListPanelSettings.sceneTextMode === "HEADING") {
                                    if(sceneHeading.enabled) {
                                        elementDesc = element.resolvedSceneNumber + ": " + sceneHeading.displayText
                                    } else {
                                        if(summary !== "") {
                                            elementDesc = summary.length > 55 ? summary.substring(0, 50) + "..." : summary
                                        } else {
                                            elementDesc = "NO SCENE HEADING"
                                        }
                                    }
                                } else {
                                    if(summary !== "") {
                                        elementDesc = element.resolvedSceneNumber + ": " + summary
                                    } else {
                                        if(sceneHeading.enabled) {
                                            elementDesc = element.resolvedSceneNumber + ": " + sceneHeading.displayText
                                        } else {
                                            elementDesc = "NO SCENE HEADING"
                                        }
                                    }
                                }

                                if(lastBreakDesc !== "")
                                    ret.push(lastBreakDesc + " - " + elementDesc + suffix)

                                const groups = element.scene.groups
                                if(groups.length > 0) {
                                    for(let g=0; g<groups.length; g++) {
                                        ret.push( SMath.titleCased(groups[g]) + " - " + elementDesc + suffix)
                                    }
                                }

                                const keywords = element.scene.tags
                                if(keywords.length > 0) {
                                    for(let k=0; k<keywords.length; k++) {
                                        ret.push(keywords[k] + " - " + elementDesc + suffix)
                                    }
                                }
                            } else {
                                if(element.breakType !== Screenplay.Episode && lastEpisodeDesc !== "")
                                    elementDesc = lastEpisodeDesc + ", "
                                elementDesc += element.breakTitle
                                if(element.breakSubtitle !== "")
                                    elementDesc += ": " + element.breakSubtitle
                                if(element.breakType === Screenplay.Episode)
                                    lastEpisodeDesc = elementDesc
                                lastBreakDesc = elementDesc
                            }
                            ret.push(elementDesc + suffix)
                        }

                        return ret
                    }

                    completionStrings: availableTargets
                    completionFilterMode: Runtime.screenplayEditorSettings.jumpToSceneFilterMode
                    maxCompletionItems: -1
                    completionIgnoreSuffixAfter: " %"
                    completionAcceptsEnglishStringsOnly: false

                    placeholderText: length > 0 ? "" : "1, 2, 2A, ACT 1"

                    includeSuggestion: (suggestion) => {
                        Qt.callLater(jumpToScene)
                        return suggestion
                    }

                    polishCompletionText: (text) => {
                        let str = text
                        const suffixIndex = str.lastIndexOf(completionIgnoreSuffixAfter);
                        return text.substring(0, suffixIndex)
                    }

                    onReturnPressed: Qt.callLater(jumpToScene)

                    function getSerialNumberFromText() {
                        const str = text
                        const suffixMarker = completionIgnoreSuffixAfter
                        const suffixIndex = str.lastIndexOf(suffixMarker);
                        if (suffixIndex === -1) {
                            return -1; // No '#' found
                        }
                        const substring = str.substring(suffixIndex + suffixMarker.length);
                        const match = substring.match(/^\d+/);
                        return match ? parseInt(match[0], 10) : -1;
                    }

                    function jumpToScene() {
                        const serialNumber = getSerialNumberFromText()
                        const elementIndex = _dialog.screenplayAdapter.screenplay.indexOfSerialNumber(serialNumber)
                        if(elementIndex >= 0) {
                            _dialog.screenplayAdapter.currentIndex = elementIndex
                            ActionHub.triggerLater(ActionHub.editOptions.find("editSceneContent"))
                        }
                        _dialog.close()
                    }
                }

                RowLayout {
                    Layout.fillWidth: true

                    VclLabel {
                        text: "Filter Mode: "
                    }

                    VclRadioButton {
                        text: "Starts With"

                        checked: _sceneNumberField.completionFilterMode == CompletionModel.StartsWithPrefix
                        onClicked: Runtime.screenplayEditorSettings.jumpToSceneFilterMode = CompletionModel.StartsWithPrefix
                    }

                    VclRadioButton {
                        text: "Contains"

                        checked: _sceneNumberField.completionFilterMode == CompletionModel.ContainsPrefix
                        onClicked: Runtime.screenplayEditorSettings.jumpToSceneFilterMode = CompletionModel.ContainsPrefix
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    VclButton {
                        text: "Ok"

                        onClicked: _sceneNumberField.editingComplete()
                    }
                }
            }
        }
    }
}
