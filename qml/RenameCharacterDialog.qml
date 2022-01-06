/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
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
import QtQuick.Controls 2.15

import io.scrite.components 1.0

Item {
    width: 640
    height: layout.height + 80
    property Character character: modalDialog.arguments
    property bool renameWasSuccessful: false

    Component.onCompleted: {
        modalDialog.closeable = false
        modalDialog.closeOnEscape = true
        character.clearRenameError()
        Qt.callLater( function() { newNameField.forceActiveFocus() } )
    }

    Column {
        id: layout
        spacing: 40
        width: parent.width-80
        anchors.centerIn: parent

        Text {
            font.pointSize: Scrite.app.idealFontPointSize + 2
            text: "Rename Character"
            width: parent.width
            wrapMode: Text.WordWrap
            maximumLineCount: 2
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
            Component.onCompleted: {
                busyOverlay.oldName = Scrite.app.camelCased(character.name)
                text = "Rename character: <b>" + busyOverlay.oldName + "</b>"
            }
        }

        TextField2 {
            id: newNameField
            placeholderText: "New name"
            width: parent.width
            label: ""
            focus: true
            horizontalAlignment: Text.AlignHCenter
            font.pointSize: Scrite.app.idealFontPointSize + 2
            onReturnPressed: renameButton.click()
        }

        Row {
            width: parent.width
            spacing: 20

            Button2 {
                id: cancelButton
                text: "Cancel"
                anchors.top: parent.top
                onClicked: modalDialog.close()
            }

            Item {
                anchors.top: parent.top
                width: parent.width - cancelButton.width - renameButton.width - 2*parent.spacing
                height: cancelButton.height

                Text {
                    anchors.centerIn: parent
                    width: parent.width
                    text: character.renameError
                    color: "red"
                    wrapMode: Text.WordWrap
                    maximumLineCount: 2
                    elide: Text.ElideRight
                    font.pointSize: Scrite.app.idealFontPointSize
                    horizontalAlignment: Text.AlignHCenter
                }

                Column {
                    anchors.centerIn: parent
                    spacing: 5
                    width: parent.width
                    visible: character.renameError === ""

                    Text {
                        width: parent.width
                        text: "This operation cannot be undone."
                        font.pointSize: Math.max(10,Scrite.app.idealFontPointSize-4)
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    CheckBox2 {
                        id: chkNotice
                        anchors.horizontalCenter: parent.horizontalCenter
                        padding: 0
                        text: "Agree"
                    }
                }
            }

            Button2 {
                id: renameButton
                text: "Rename"
                enabled: chkNotice.checked && newNameField.length > 0 && newNameField.text.toUpperCase() !== character.name
                anchors.top: parent.top
                onClicked: click()

                function click() { renameAnimation.start() }
            }
        }
    }


    BusyOverlay {
        id: busyOverlay
        anchors.fill: parent
        property string oldName
        property string newName
        onVisibleChanged: {
            if(visible)
                newName = Scrite.app.camelCased(newNameField.text)
        }
        busyMessage: "Renaming '" + oldName + "' to '<b>" + newName + "</b>' ..."
    }

    SequentialAnimation {
        id: renameAnimation
        running: false

        ScriptAction {
            script: {
                busyOverlay.visible = true
            }
        }

        PauseAnimation {
            duration: 50
        }

        ScriptAction {
            script: {
                renameWasSuccessful = character.rename(newNameField.text)
            }
        }

        PauseAnimation {
            duration: 500
        }

        ScriptAction {
            script: {
                if(renameWasSuccessful)
                    modalDialog.close()
                else
                    busyOverlay.visible = false
            }
        }
    }
}
