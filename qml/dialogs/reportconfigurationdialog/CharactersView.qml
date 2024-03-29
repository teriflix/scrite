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

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"

Rectangle {
    property bool showFilterBox: true
    property alias selectedCharacters: charactersModel.selectedCharacters
    property int sortFilterRole: 0
    property alias filterTags: charactersModel.tags

    color: Runtime.colors.primary.c10.background
    border { width: 1; color: Runtime.colors.primary.borderColor }
    implicitWidth: charactersListLayout.width

    CharacterNamesModel {
        id: charactersModel
        structure: Scrite.document.structure
    }

    Row {
        id: searchBarRow
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top

        FlatToolButton {
            id: filterButton
            iconSource: "qrc:/icons/action/filter.png"
            ToolTip.text: "Filter character names by their tags."
            enabled: charactersModel.availableTags.length > 0
            onClicked: tagsMenu.open()
            down: tagsMenu.visible

            VclText {
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                padding: 2
                font.bold: true
                font.pixelSize: parent.height * 0.2
                color: Runtime.colors.primary.highlight.text

                text: charactersModel.tags.length > 0 ? charactersModel.tags.length : ""
            }

            Item {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                height: 1

                VclMenu {
                    id: tagsMenu

                    Repeater {
                        model: charactersModel.availableTags

                        VclMenuItem {
                            text: modelData
                            checkable: true
                            checked: charactersModel.hasTag(modelData)
                            onToggled: charactersModel.toggleTag(modelData)
                        }
                    }
                }
            }
        }

        SearchBar {
            id: searchBar
            searchEngine.objectName: "Characters Search Engine"
            width: parent.width - filterButton.width - parent.spacing
        }
    }

    Flickable {
        id: charactersListView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: searchBarRow.bottom
        anchors.bottom: buttonsRow.top
        anchors.bottomMargin: 8
        clip: true
        contentWidth: charactersListLayout.width
        contentHeight: height
        interactive: false
        ScrollBar.horizontal: VclScrollBar { }

        Flow {
            id: charactersListLayout
            flow: Flow.TopToBottom
            height: charactersListView.height - 4
            property real columnWidth: 100

            Repeater {
                model: charactersModel

                VclCheckBox {
                    id: characterCheckBox
                    width: charactersListLayout.columnWidth
                    Component.onCompleted: charactersListLayout.columnWidth = Math.max(charactersListLayout.columnWidth, implicitWidth)
                    checkable: true
                    checked: charactersModel.isInSelection(modelData)
                    text: modelData
                    onToggled: charactersModel.toggleSelection(modelData)

                    property bool highlight: false
                    background: Rectangle {
                        color: characterCheckBox.Material.background
                    }
                    Material.background: highlight ? Runtime.colors.accent.c300.background : Runtime.colors.primary.c10.background
                    Material.foreground: highlight ? Runtime.colors.accent.c300.text : Runtime.colors.primary.c10.text

                    SearchAgent.engine: searchBar.searchEngine
                    SearchAgent.onSearchRequest: {
                        SearchAgent.searchResultCount = SearchAgent.indexesOf(string, characterCheckBox.text).length > 0 ? 1 : 0
                    }
                    SearchAgent.onCurrentSearchResultIndexChanged: {
                        characterCheckBox.highlight = SearchAgent.currentSearchResultIndex >= 0
                        charactersListView.ensureItemVisible(characterCheckBox,1,10)
                    }
                    SearchAgent.onClearSearchRequest: characterCheckBox.font.bold = false

                    Connections {
                        target: charactersModel
                        function onSelectedCharactersChanged() {
                            characterCheckBox.checked = charactersModel.isInSelection(characterCheckBox.text)
                        }
                    }
                }
            }
        }
    }

    Row {
        id: buttonsRow
        spacing: 20
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.leftMargin: 10
        anchors.bottomMargin: 4

        VclButton {
            text: "Select All"
            enabled: charactersModel.count > 0
            onClicked: charactersModel.selectAll()
        }

        VclButton {
            text: "Unselect All"
            enabled: charactersModel.count > 0
            onClicked: charactersModel.unselectAll()
        }
    }
}
