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

Rectangle {
    property bool showFilterBox: true
    property alias selectedCharacters: charactersModel.selectedCharacters
    property int sortFilterRole: 0
    property alias filterTags: charactersModel.tags

    color: primaryColors.c10.background
    border { width: 1; color: primaryColors.borderColor }
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

        ToolButton3 {
            id: filterButton
            iconSource: "../icons/action/filter.png"
            ToolTip.text: "Filter character names by their tags."
            enabled: charactersModel.availableTags.length > 0
            onClicked: tagsMenu.open()
            down: tagsMenu.visible

            Text {
                font.pixelSize: parent.height * 0.2
                font.bold: true
                text: charactersModel.tags.length > 0 ? charactersModel.tags.length : ""
                padding: 2
                color: primaryColors.highlight.text
                anchors.bottom: parent.bottom
                anchors.right: parent.right
            }

            Item {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1

                Menu2 {
                    id: tagsMenu

                    Repeater {
                        model: charactersModel.availableTags

                        MenuItem2 {
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

    ScrollArea {
        id: charactersListView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: searchBarRow.bottom
        anchors.bottom: buttonsRow.top
        anchors.bottomMargin: 8
        clip: true
        contentWidth: charactersListLayout.width
        contentHeight: height
        ScrollBar.vertical.policy: ScrollBar.AlwaysOff
        ScrollBar.horizontal.policy: charactersListLayout.width > width ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
        ScrollBar.horizontal.opacity: active ? 1 : 0.2

        Flow {
            id: charactersListLayout
            flow: Flow.TopToBottom
            height: charactersListView.height - 4
            property real columnWidth: 100

            Repeater {
                model: charactersModel

                CheckBox2 {
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
                    Material.background: highlight ? accentColors.c300.background : primaryColors.c10.background
                    Material.foreground: highlight ? accentColors.c300.text : primaryColors.c10.text

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

        Button2 {
            text: "Select All"
            enabled: charactersModel.count > 0
            onClicked: charactersModel.selectAll()
        }

        Button2 {
            text: "Unselect All"
            enabled: charactersModel.count > 0
            onClicked: charactersModel.unselectAll()
        }
    }
}
