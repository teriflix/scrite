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

pragma ComponentBehavior: Bound
pragma Singleton

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import io.scrite.components

import "../../globals"
import "../../controls"
import "../../helpers"
import "../../dialogs"

DialogLauncher {
    id: root

    function launch(relationship) { return doLaunch({"relationship": relationship}) }

    name: "RelationshipNameEditorDialog"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: _dialog

        property Relationship relationship
        property Character ofCharacter: relationship ? (relationship.direction === Relationship.OfWith ? relationship.ofCharacter : relationship.withCharacter) : null
        property Character withCharacter: relationship ? (relationship.direction === Relationship.OfWith ? relationship.withCharacter : relationship.ofCharacter) : null

        title: "Edit Relationship"
        width: 800
        height: 400

        content: Item {
            Component.onCompleted: {
                Runtime.execLater(_dialogLayout, 100, function() {
                    _txtRelationshipName.forceActiveFocus()
                })
            }

            ColumnLayout {
                id: _dialogLayout
                anchors.centerIn: parent

                width: parent.width-40

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

                            color: _dialog.ofCharacter.photos.length === 0 ? "white" : Qt.rgba(0,0,0,0)
                            border.width: 1
                            border.color: "black"

                            Image {
                                anchors.fill: parent
                                source: {
                                    if(_dialog.ofCharacter.hasKeyPhoto > 0)
                                    return "file:///" + _dialog.ofCharacter.keyPhoto
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

                            text: SMath.titleCased(_dialog.ofCharacter.name)
                        }
                    }

                    VclTextField {
                        id: _txtRelationshipName

                        Layout.fillWidth: true

                        focus: true
                        text: _dialog.relationship.name
                        label: "Relationship:"
                        maximumLength: 50
                        placeholderText: "husband of, wife of, friends with, reports to ..."

                        readOnly: Scrite.document.readOnly
                        enableTransliteration: true
                        onReturnPressed: _doneButton.click()
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
                                    if(_dialog.withCharacter.hasKeyPhoto > 0)
                                    return "file:///" + _dialog.withCharacter.keyPhoto
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

                            text: SMath.titleCased(_dialog.withCharacter.name)
                        }
                    }
                }

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 20

                    VclButton {
                        id: _revertButton

                        text: "Revert"
                        enabled: _txtRelationshipName.text !== _dialog.relationship.name

                        onClicked: _txtRelationshipName.text = _dialog.relationship.name
                    }

                    VclButton {
                        id: _doneButton

                        function click() {
                            _dialog.relationship.name = _txtRelationshipName.text.trim()
                            _dialog.close()
                        }

                        text: "Change"
                        enabled: _txtRelationshipName.length > 0

                        onClicked: click()
                    }
                }
            }
        }

        onClosed: relationship = null
    }
}
