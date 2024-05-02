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
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"

DialogLauncher {
    id: root

    function launch() { return doLaunch() }

    name: "CollaboratorsDialog"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: dialog

        width: 640
        height: 550
        title: "Shield"

        content: Item {

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 20

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    VclLabel {
                        Layout.alignment: Qt.AlignVCenter
                        Layout.fillWidth: true

                        font.pointSize: Runtime.idealFontMetrics.font.pointSize
                        text: {
                            if(Scrite.document.canModifyCollaborators) {
                                var ret = "Allow opening of this screenplay only on Scrite installations logged in with <strong>" + Scrite.user.info.email + "</strong>. "
                                if(Scrite.document.hasCollaborators)
                                ret += "You can optionally add emails of one or more collaborators to the list below."
                                return ret
                            }
                            return "This screenplay has been marked for collaboration by <strong>" + Scrite.document.primaryCollaborator + "</strong> with the emails listed below."
                        }
                        wrapMode: Text.WordWrap
                    }

                    Switch {
                        id: protectionSwitch
                        Layout.alignment: Qt.AlignVCenter

                        checked: Scrite.document.hasCollaborators
                        enabled: Scrite.document.canModifyCollaborators
                        onToggled: {
                            if(checked)
                            Scrite.document.enableCollaboration()
                            else
                            Scrite.document.disableCollaboration()
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    border.width: 1
                    border.color: Runtime.colors.primary.c700.background
                    color: Runtime.colors.primary.c100.background
                    enabled: Scrite.document.hasCollaborators
                    opacity: enabled ? 1.0 : 0.5

                    Connections {
                        target: Scrite.document
                        function onCollaboratorsChanged() { Qt.callLater(saveDocument) }
                        function saveDocument() {
                            if(Scrite.document.fileName !== "")
                                Scrite.document.save()
                        }
                    }

                    ListView {
                        id: collaboratorsList
                        anchors.fill: parent
                        anchors.margins: 5
                        anchors.leftMargin: 10
                        clip: true
                        property real viewportWidth: contentHeight > height ? width-20 : width-1
                        ScrollBar.vertical: VclScrollBar {
                            flickable: collaboratorsList
                        }
                        header: Scrite.document.canModifyCollaborators ? collaboratorsListHeader : null

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

                            RowLayout {
                                id: delegateLayout
                                spacing: 5
                                width: parent.width-5
                                anchors.centerIn: parent

                                VclLabel {
                                    Layout.fillWidth: true
                                    Layout.alignment: Qt.AlignVCenter

                                    wrapMode: Text.WordWrap
                                    font.pointSize: Runtime.idealFontMetrics.font.pointSize
                                    font.italic: collaboratorName === ""
                                    text: collaborator
                                }

                                FlatToolButton {
                                    id: deleteIcon
                                    Layout.alignment: Qt.AlignVCenter

                                    opacity: delegateMouseArea.containsMouse || containsMouse ? (enabled ? 1 : 0.5) : 0
                                    iconSource: "qrc:/icons/action/close.png"
                                    onClicked: Scrite.document.removeCollaborator(collaboratorEmail)
                                    enabled: Scrite.document.canModifyCollaborators
                                }
                            }
                        }
                    }

                    Component {
                        id: collaboratorsListHeader

                        RowLayout {
                            width: collaboratorsList.width
                            enabled: Scrite.document.canModifyCollaborators
                            opacity: enabled ? 1 : 0.5

                            TextField {
                                id: newCollaboratorEmail
                                Layout.fillWidth: true
                                Layout.alignment: Qt.AlignVCenter

                                placeholderText: "Enter Email ID and hit Return"
                                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                                validator: RegExpValidator {
                                    regExp: /^(([^<>()[\]\\.,;:\s@"]+(\.[^<>()[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/
                                }
                                selectByMouse: true
                                Keys.onReturnPressed: addCollaborator()

                                function addCollaborator() {
                                    if(acceptableInput) {
                                        Scrite.document.addCollaborator(text)
                                        clear()
                                        forceActiveFocus()
                                        queryCollaboratorsCall.fetchUsersInfo()
                                    }
                                }
                            }

                            FlatToolButton {
                                id: addCollaboratorButton
                                Layout.alignment: Qt.AlignVCenter

                                iconSource: "qrc:/icons/content/add_box.png"
                                onClicked: newCollaboratorEmail.addCollaborator()
                                enabled: newCollaboratorEmail.acceptableInput
                            }
                        }
                    }
                }
            }
        }
    }
}
