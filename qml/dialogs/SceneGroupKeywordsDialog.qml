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

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/helpers"
import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/dialogs/settingsdialog"

DialogLauncher {
    id: root

    function launch(sceneGroup) {
        if(sceneGroup === undefined || !Object.isOfType(sceneGroup, "SceneGroup") || sceneGroup.sceneCount === 0) {
            MessageBox.information("No Scene Selected",
                                   "Atleast one scene should be selected for adding / remove keywords on it.")
            return null
        }

        let clonedSceneGroup = sceneGroup.clone()
        let dlg = doLaunch({"sceneGroup": clonedSceneGroup})
        Object.reparent(clonedSceneGroup, dlg)
        return dlg
    }

    name: "SceneGroupKeywordsDialog"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: _dialog

        required property SceneGroup sceneGroup

        width: Math.min(Scrite.window.width-80, 640)
        height: Math.min(Scrite.window.height-80, 480)

        title: sceneGroup.sceneCount > 1 ? "Scene Group Keywords" : "Scene Keywords"
        titleBarCloseButtonVisible: contentInstance ? !contentInstance.acceptingNewText : true

        content: Item {
            property alias acceptingNewText: _keywordsList.acceptingNewText

            Component.onCompleted: Qt.callLater(_keywordsList.acceptNewText)

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20

                spacing: 20

                VclLabel {
                    id: _label

                    Layout.fillWidth: true

                    wrapMode: Text.WordWrap
                    text: {
                        if(_dialog.sceneGroup.sceneCount === 1)
                            return "Add / remove keywords in the selected scene."
                        return "Add / remove keywords across all the selected " + _dialog.sceneGroup.sceneCount + " scenes."
                    }
                }

                Flickable {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ScrollBar.vertical: VclScrollBar { }

                    contentWidth: width
                    contentHeight: _keywordsList.height

                    TextListInput {
                        id: _keywordsList

                        width: parent.width - 20

                        textList: _dialog.sceneGroup.openTags
                        completionStrings: Scrite.document.structure.sceneTags
                        addTextButtonTooltip: "Click here to " + _label.text.toLowerCase()
                        font: Runtime.idealFontMetrics.font
                        labelText: "Keywords"
                        labelIconSource:  "qrc:/icons/action/keyword.png"
                        labelIconVisible: true

                        readOnly: Scrite.document.readOnly

                        onEnsureVisible: (item, area) => {  }
                        onTextClicked: (text, source) => {  }
                        onTextCloseRequest: (text, source) => { _dialog.sceneGroup.removeOpenTag(text) }
                        onConfigureTextRequest: (text, tag) => { tag.closable = true }
                        onNewTextRequest: (text) => { _dialog.sceneGroup.addOpenTag(text) }
                        onNewTextCancelled: () => { }
                    }
                }
            }
        }
    }
}


