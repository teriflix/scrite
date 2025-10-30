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


import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"

DialogLauncher {
    id: root

    function launch(relationship) { return doLaunch({"relationship": relationship}) }

    name: "RelationshipNameEditorDialog"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: dialog

        property Relationship relationship
        property Character ofCharacter: relationship ? (relationship.direction === Relationship.OfWith ? relationship.ofCharacter : relationship.withCharacter) : null
        property Character withCharacter: relationship ? (relationship.direction === Relationship.OfWith ? relationship.withCharacter : relationship.ofCharacter) : null

        title: "Edit Relationship"
        width: 800
        height: 400

        content: Item {
            Component.onCompleted: {
                Runtime.execLater(dialogLayout, 100, function() {
                    txtRelationshipName.forceActiveFocus()
                })
            }

            ColumnLayout {
                id: dialogLayout
                width: parent.width-40
                anchors.centerIn: parent
                spacing: 20

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter

                    spacing: 10

                    ColumnLayout {
                        spacing: 10

                        Rectangle {
                            Layout.preferredWidth: 150
                            Layout.preferredHeight: 150
                            Layout.alignment: Qt.AlignHCenter

                            color: dialog.ofCharacter.photos.length === 0 ? "white" : Qt.rgba(0,0,0,0)
                            border.width: 1
                            border.color: "black"

                            Image {
                                anchors.fill: parent
                                source: {
                                    if(dialog.ofCharacter.hasKeyPhoto > 0)
                                    return "file:///" + dialog.ofCharacter.keyPhoto
                                    return "qrc:/icons/content/character_icon.png"
                                }
                                fillMode: Image.PreserveAspectCrop
                                mipmap: true; smooth: true
                            }
                        }

                        VclLabel {
                            Layout.alignment: Qt.AlignHCenter
                            Layout.preferredWidth: 180

                            elide: Text.ElideRight
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            maximumLineCount: 2
                            horizontalAlignment: Text.AlignHCenter

                            text: SMath.titleCased(dialog.ofCharacter.name)
                        }
                    }

                    VclTextField {
                        id: txtRelationshipName

                        Layout.fillWidth: true

                        focus: true
                        text: dialog.relationship.name
                        label: "Relationship:"
                        maximumLength: 50
                        placeholderText: "husband of, wife of, friends with, reports to ..."

                        readOnly: Scrite.document.readOnly
                        enableTransliteration: true
                        onReturnPressed: doneButton.click()
                    }

                    ColumnLayout {
                        spacing: 10

                        Rectangle {
                            Layout.preferredWidth: 150
                            Layout.preferredHeight: 150
                            Layout.alignment: Qt.AlignHCenter

                            color: withCharacter.photos.length === 0 ? "white" : Qt.rgba(0,0,0,0)
                            border.width: 1
                            border.color: "black"

                            Image {
                                anchors.fill: parent
                                source: {
                                    if(dialog.withCharacter.hasKeyPhoto > 0)
                                    return "file:///" + dialog.withCharacter.keyPhoto
                                    return "qrc:/icons/content/character_icon.png"
                                }
                                fillMode: Image.PreserveAspectCrop
                                mipmap: true; smooth: true
                            }
                        }

                        VclLabel {
                            Layout.alignment: Qt.AlignHCenter
                            Layout.preferredWidth: 180

                            elide: Text.ElideRight
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            maximumLineCount: 2
                            horizontalAlignment: Text.AlignHCenter

                            text: SMath.titleCased(dialog.withCharacter.name)
                        }
                    }
                }

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 20

                    VclButton {
                        id: revertButton
                        text: "Revert"
                        enabled: txtRelationshipName.text !== dialog.relationship.name
                        onClicked: txtRelationshipName.text = dialog.relationship.name
                    }

                    VclButton {
                        id: doneButton
                        text: "Change"
                        enabled: txtRelationshipName.length > 0
                        onClicked: click()
                        function click() {
                            dialog.relationship.name = txtRelationshipName.text.trim()
                            dialog.close()
                        }
                    }
                }
            }
        }

        onClosed: relationship = null
    }
}
