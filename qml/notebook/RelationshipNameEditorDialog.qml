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
import "qrc:/qml/dialogs"

Item {
    id: root

    parent: Scrite.window.contentItem

    function launch(relationship) {
        var dlg = relationshipNameEditorDialogComponent.createObject(root, {"relationship": relationship})
        if(dlg) {
            dlg.closed.connect(dlg.destroy)
            dlg.open()
            return dlg
        }

        Scrite.app.log("Couldn't launch RelationshipNameEditorDialog")
        return null
    }

    Component {
        id: relationshipNameEditorDialogComponent

        VclDialog {
            id: relationshipNameEditorDialog

            property Relationship relationship
            property Character ofCharacter: relationship ? (relationship.direction === Relationship.OfWith ? relationship.ofCharacter : relationship.withCharacter) : null
            property Character withCharacter: relationship ? (relationship.direction === Relationship.OfWith ? relationship.withCharacter : relationship.ofCharacter) : null

            title: "Edit Relationship"
            width: 800
            height: 400

            content: Item {
                Component.onCompleted: {
                    Utils.execLater(dialogLayout, 100, function() {
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

                                color: relationshipNameEditorDialog.ofCharacter.photos.length === 0 ? "white" : Qt.rgba(0,0,0,0)
                                border.width: 1
                                border.color: "black"

                                Image {
                                    anchors.fill: parent
                                    source: {
                                        if(relationshipNameEditorDialog.ofCharacter.hasKeyPhoto > 0)
                                            return "file:///" + relationshipNameEditorDialog.ofCharacter.keyPhoto
                                        return "qrc:/icons/content/character_icon.png"
                                    }
                                    fillMode: Image.PreserveAspectCrop
                                    mipmap: true; smooth: true
                                }
                            }

                            VclText {
                                Layout.alignment: Qt.AlignHCenter
                                Layout.preferredWidth: 180

                                elide: Text.ElideRight
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                maximumLineCount: 2
                                horizontalAlignment: Text.AlignHCenter

                                text: Scrite.app.camelCased(relationshipNameEditorDialog.ofCharacter.name)
                            }
                        }

                        VclTextField {
                            id: txtRelationshipName

                            Layout.fillWidth: true

                            focus: true
                            text: relationshipNameEditorDialog.relationship.name
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
                                        if(relationshipNameEditorDialog.withCharacter.hasKeyPhoto > 0)
                                            return "file:///" + relationshipNameEditorDialog.withCharacter.keyPhoto
                                        return "qrc:/icons/content/character_icon.png"
                                    }
                                    fillMode: Image.PreserveAspectCrop
                                    mipmap: true; smooth: true
                                }
                            }

                            VclText {
                                Layout.alignment: Qt.AlignHCenter
                                Layout.preferredWidth: 180

                                elide: Text.ElideRight
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                maximumLineCount: 2
                                horizontalAlignment: Text.AlignHCenter

                                text: Scrite.app.camelCased(relationshipNameEditorDialog.withCharacter.name)
                            }
                        }
                    }

                    RowLayout {
                        Layout.alignment: Qt.AlignHCenter
                        spacing: 20

                        VclButton {
                            id: revertButton
                            text: "Revert"
                            enabled: txtRelationshipName.text !== relationshipNameEditorDialog.relationship.name
                            onClicked: txtRelationshipName.text = relationshipNameEditorDialog.relationship.name
                        }

                        VclButton {
                            id: doneButton
                            text: "Change"
                            enabled: txtRelationshipName.length > 0
                            onClicked: click()
                            function click() {
                                relationshipNameEditorDialog.relationship.name = txtRelationshipName.text.trim()
                                relationshipNameEditorDialog.close()
                            }
                        }
                    }
                }
            }

            onClosed: relationship = null
        }
    }
}
