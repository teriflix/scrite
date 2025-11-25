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
import "qrc:/qml/notebookview/menus"
import "qrc:/qml/notebookview/dialogs"

Item {
    id: root

    signal switchRequest(var item) // could be string, or any of the notebook objects like Notes, Character etc.
    signal deleteCharacterRequest(Character character)

    GridView {
        id: _charactersView

        Component.onCompleted: {
            __evaluateCounts()
            headerItem.assumeFocus()
        }

        ScrollBar.vertical: _vscrollBar

        anchors.fill: parent
        anchors.rightMargin: contentHeight > height ? 20 : 0

        cellHeight: 120
        cellWidth: width/__columnCount
        clip: true
        highlightMoveDuration: 0
        model: _charactersModel

        highlight: Item {
            BoxShadow {
                anchors.fill: _highlightedItem
                opacity: 0.5
            }

            Item {
                id: _highlightedItem
                anchors.fill: parent
                anchors.margins: 5
            }
        }

        delegate: Item {
            id: _delegate

            required property int index
            required property var objectItem

            property Character character: objectItem

            width: _charactersView.cellWidth
            height: _charactersView.cellHeight

            Rectangle {
                anchors.fill: parent
                anchors.margins: 5

                color: Runtime.colors.tint(_delegate.character.color, _charactersView.currentIndex === index ? Runtime.colors.currentNoteTint : Runtime.colors.sceneHeadingTint)
                border.width: 1
                border.color: Color.isLight(_delegate.character.color) ? (_charactersView.currentIndex === index ? "darkgray" : Runtime.colors.primary.borderColor) : _delegate.character.color

                Row {
                    anchors.fill: parent
                    anchors.margins: 10

                    spacing: 10

                    Image {
                        width: parent.height
                        height: parent.height

                        fillMode: Image.PreserveAspectCrop
                        mipmap: true
                        smooth: true

                        source: {
                            if(_delegate.character.hasKeyPhoto > 0)
                                return "file:///" + _delegate.character.keyPhoto
                            return "qrc:/icons/content/character_icon.png"
                        }
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter

                        width: parent.width - parent.height - parent.spacing

                        spacing: parent.spacing/2

                        VclLabel {
                            width: parent.width

                            elide: Text.ElideRight
                            text: _delegate.character.name

                            font.bold: true
                            font.pointSize: Runtime.idealFontMetrics.font.pointSize
                        }

                        VclLabel {
                            width: parent.width

                            elide: Text.ElideRight
                            opacity: 0.75
                            text: "Role: " + _private.polishedStr(_delegate.character.designation, "-")

                            font.pointSize: Runtime.idealFontMetrics.font.pointSize - 2
                        }

                        VclLabel {
                            width: parent.width

                            elide: Text.ElideRight
                            opacity: 0.75
                            text: ["Age: " + _private.polishedStr(_delegate.character.age, "-"), "Gender: " + _private.polishedStr(_delegate.character.gender, "-")].join(", ")

                            font.pointSize: Runtime.idealFontMetrics.font.pointSize - 2
                        }
                    }
                }
            }

            MouseArea {
                anchors.fill: parent

                acceptedButtons: Qt.LeftButton | Qt.RightButton

                onClicked: (mouse) => {
                               _charactersView.currentIndex = index
                               if(mouse.button === Qt.RightButton) {
                                   _private.popupCharacterMenu(_delegate.character, _delegate)
                               }
                           }

                onDoubleClicked: (mouse) => {
                                     root.switchRequest(_delegate.character.notes)
                                 }
            }
        }

        header: _headerFooter
        footer: __rowCount > __visibleRowCount ? _headerFooter : null

        TrackerPack {
            TrackProperty { target: _charactersView; property: "width" }
            TrackProperty { target: _charactersView; property: "height" }
            onTracked:  _charactersView.__evaluateCounts()
        }

        function __evaluateCounts() {
            __idealCellWidth = Math.min(250,width)
            __columnCount = Math.floor(width/__idealCellWidth)
            __rowCount = Math.ceil(model.objectCount/__columnCount)
            __visibleRowCount = Math.ceil((height-60)/cellHeight)
        }

        property int __rowCount: 0
        property int __columnCount: 0
        property int __visibleRowCount: 0
        property real __idealCellWidth: 0
    }

    Component {
        id: _headerFooter

        Item {
            function assumeFocus() {
                _nameField.forceActiveFocus()
            }

            width: _charactersView.width
            height: 60

            Rectangle {
                anchors.fill: parent
                anchors.margins: 5

                color: Color.translucent(Runtime.colors.primary.windowColor, 0.5)
                border { width: 1; color: Runtime.colors.primary.borderColor }

                RowLayout {
                    anchors.centerIn: parent

                    width: parent.width-20

                    spacing: 10

                    VclTextField {
                        id: _nameField

                        Layout.fillWidth: true

                        label: ""
                        placeholderText: Scrite.document.readOnly ? "Enter character name to search." : "Enter character name to search/add."
                        completionStrings: Scrite.document.structure.characterNames

                        onReturnPressed: _addButton.click()
                    }

                    FlatToolButton {
                        id: _addButton

                        iconSource: "qrc:/icons/content/person_add.png"
                        toolTipText: "Add Character"

                        onClicked: {
                            let chName = _nameField.text
                            let ch = Scrite.document.structure.findCharacter(chName)
                            if(ch)
                                root.switchRequest(ch.notes)
                            else if(!Scrite.document.readOnly) {
                                ch = Scrite.document.structure.addCharacter(chName)

                                let notebookModel = ObjectRegistry.find("notebookModel")
                                if(notebookModel)
                                    notebookModel.preferredItem = ch.notes
                                else
                                    root.switchRequest(ch.notes)
                            }
                        }
                    }

                    FlatToolButton {
                        id: _addAllButton

                        iconSource: "qrc:/icons/content/persons_add.png"
                        toolTipText: "Add Existing Characters"

                        onClicked: AddCharactersDialog.launch()
                    }
                }
            }
        }
    }

    VclScrollBar {
        id: _vscrollBar

        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        flickable: _charactersView
        orientation: Qt.Vertical
    }

    SortFilterObjectListModel {
        id: _charactersModel

        sortByProperty: "name"
        sourceModel: Scrite.document.structure.charactersModel
    }

    QtObject {
        id: _private

        readonly property Component characterMenu: CharacterMenu {
            onDeleteCharacterRequest: () => { root.deleteCharacterRequest(character) }
        }

        function popupCharacterMenu(character, source) {
            let menu = characterMenu.createObject(source, {"character": character})
            menu.aboutToHide.connect(menu.destroy)
            menu.popup()
        }

        function polishedStr(val,defval) {
            return val === "" ? defval : val
        }
    }
}
