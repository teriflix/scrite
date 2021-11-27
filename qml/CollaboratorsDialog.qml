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

import QtQml 2.13
import QtQuick 2.13
import QtQuick.Window 2.13
import QtQuick.Controls 2.13

import Scrite 1.0

Item {
    width: 640
    height: 550

    Column {
        anchors.fill: parent
        anchors.margins: 30
        spacing: 20
        enabled: User.loggedIn

        Text {
            id: titleText
            text: "Screenplay Protection"
            font.bold: true
            font.pointSize: 24
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Row {
            id: protectionSwitchRow
            width: parent.width
            spacing: 10

            Text {
                width: parent.width - protectionSwitch.width - parent.spacing
                text: {
                    if(scriteDocument.canModifyCollaborators) {
                        var ret = "Allow opening of this screenplay only on Scrite installations logged in with <strong>" + User.info.email + "</strong>. "
                        if(scriteDocument.hasCollaborators)
                            ret += "You can optionally add emails of one or more collaborators to the list below."
                        return ret
                    }
                    return "This screenplay has been marked for collaboration by <strong>" + scriteDocument.primaryCollaborator + "</strong> with the emails listed below."
                }
                font.pointSize: app.idealFontPointSize
                wrapMode: Text.WordWrap
                anchors.verticalCenter: parent.verticalCenter
            }

            Switch {
                id: protectionSwitch
                checked: scriteDocument.hasCollaborators
                enabled: scriteDocument.canModifyCollaborators
                anchors.verticalCenter: parent.verticalCenter
                onToggled: {
                    if(checked)
                        scriteDocument.enableCollaboration()
                    else
                        scriteDocument.disableCollaboration()
                }
            }
        }

        Rectangle {
            width: parent.width
            height: parent.height - titleText.height - protectionSwitchRow.height - 2*parent.spacing
            border.width: 1
            border.color: primaryColors.c700.background
            color: primaryColors.c100.background
            enabled: scriteDocument.hasCollaborators
            opacity: enabled ? 1.0 : 0.5

            ListView {
                id: collaboratorsList
                anchors.fill: parent
                anchors.margins: 5
                anchors.leftMargin: 10
                clip: true
                property real viewportWidth: contentHeight > height ? width-20 : width-1
                ScrollBar.vertical: ScrollBar2 {
                    flickable: collaboratorsList
                }
                header: scriteDocument.canModifyCollaborators ? collaboratorsListHeader : null

                model: ScriteDocumentCollaborators { }
                property var collaboratorsMetaData

                delegate: Item {
                    width: collaboratorsList.viewportWidth
                    height: delegateLayout.height + 4

                    MouseArea {
                        id: delegateMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                    }

                    Row {
                        id: delegateLayout
                        spacing: 5
                        width: parent.width-5
                        anchors.centerIn: parent

                        Text {
                            width: parent.width - (deleteIcon.opacity > 0 ? (deleteIcon.width+parent.spacing) : 0)
                            wrapMode: Text.WordWrap
                            font.pointSize: app.idealFontPointSize
                            font.italic: collaboratorName === ""
                            text: collaborator
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        ToolButton3 {
                            id: deleteIcon
                            opacity: delegateMouseArea.containsMouse || containsMouse ? (enabled ? 1 : 0.5) : 0
                            iconSource: "../icons/action/close.png"
                            anchors.verticalCenter: parent.verticalCenter
                            onClicked: scriteDocument.removeCollaborator(collaboratorEmail)
                            enabled: scriteDocument.canModifyCollaborators
                        }
                    }
                }
            }

            Component {
                id: collaboratorsListHeader

                Row {
                    width: collaboratorsList.width
                    enabled: scriteDocument.canModifyCollaborators
                    opacity: enabled ? 1 : 0.5

                    TextField {
                        id: newCollaboratorEmail
                        width: parent.width - addCollaboratorButton.width - parent.spacing
                        placeholderText: "Enter Email ID and hit Return"
                        font.pointSize: app.idealFontPointSize
                        validator: RegExpValidator {
                            regExp: /^(([^<>()[\]\\.,;:\s@"]+(\.[^<>()[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/
                        }
                        selectByMouse: true
                        Keys.onReturnPressed: addCollaborator()

                        function addCollaborator() {
                            if(acceptableInput) {
                                scriteDocument.addCollaborator(text)
                                clear()
                                forceActiveFocus()
                                queryCollaboratorsCall.fetchUsersInfo()
                            }
                        }
                    }

                    ToolButton3 {
                        id: addCollaboratorButton
                        iconSource: "../icons/content/add_box.png"
                        anchors.verticalCenter: parent.verticalCenter
                        onClicked: newCollaboratorEmail.addCollaborator()
                        enabled: newCollaboratorEmail.acceptableInput
                    }
                }
            }
        }
    }
}
