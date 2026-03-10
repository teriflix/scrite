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

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

import io.scrite.components

import "../../globals"
import "../../controls"
import "../../helpers"

Rectangle {
    id: root
    property bool showFilterBox: true
    property alias selectedCharacters: _root_2.selectedCharacters
    property int sortFilterRole: 0
    property alias filterTags: _root_2.tags

    color: Runtime.colors.primary.c10.background
    border { width: 1; color: Runtime.colors.primary.borderColor }
    implicitWidth: _charactersListLayout.width

    CharacterNamesModel {
        id: _root_2
        structure: Scrite.document.structure
    }

    Row {
        id: _searchBarRow
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top

        FlatToolButton {
            id: _filterButton

            down: _tagsMenu.visible
            enabled: _root_2.availableTags.length > 0
            iconSource: "qrc:/icons/action/filter.png"
            toolTipText: "Filter character names by their tags."

            onClicked: _tagsMenu.open()

            VclText {
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                padding: 2
                font.bold: true
                font.pixelSize: parent.height * 0.2
                color: Runtime.colors.primary.highlight.text

                text: _root_2.tags.length > 0 ? _root_2.tags.length : ""
            }

            Item {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                height: 1

                VclMenu {
                    id: _tagsMenu

                    Repeater {
                        model: _root_2.availableTags

                        delegate: VclMenuItem {
                            required property int index
                            required property string modelData

                            text: modelData
                            checkable: true
                            checked: _root_2.hasTag(modelData)
                            onToggled: _root_2.toggleTag(modelData)
                        }
                    }
                }
            }
        }

        SearchBar {
            id: _searchBar
            searchEngine.objectName: "Characters Search Engine"
            width: parent.width - _filterButton.width - parent.spacing
        }
    }

    Flickable {
        id: _charactersListView

        function ensureItemVisible(item) {
            const viewportRect = Qt.rect(contentX, contentY, width, height)
            const itemRect = Qt.rect(item.x, item.y, item.width, item.height)
            const dp = GMath.translationRequiredToBringRectangleInRectangle(viewportRect, itemRect)
            contentX -= dp.x
        }

        ScrollBar.horizontal: VclScrollBar { }

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: _searchBarRow.bottom
        anchors.bottom: _buttonsRow.top
        anchors.bottomMargin: 8

        clip: true
        contentWidth: _charactersListLayout.width
        contentHeight: height
        interactive: false

        Flow {
            id: _charactersListLayout

            property real columnWidth: 100

            height: _charactersListView.height - 4

            flow: Flow.TopToBottom

            Repeater {
                model: _root_2

                delegate: VclCheckBox {
                    required property int index
                    required property string modelData

                    id: _characterCheckBox

                    property bool highlight: false

                    Component.onCompleted: _charactersListLayout.columnWidth = Math.max(_charactersListLayout.columnWidth, implicitWidth)

                    Material.background: highlight ? Runtime.colors.accent.c300.background : Runtime.colors.primary.c10.background
                    Material.foreground: highlight ? Runtime.colors.accent.c300.text : Runtime.colors.primary.c10.text

                    SearchAgent.engine: _searchBar.searchEngine
                    SearchAgent.onSearchRequest: {
                        SearchAgent.searchResultCount = SearchAgent.indexesOf(string, _characterCheckBox.text).length > 0 ? 1 : 0
                    }
                    SearchAgent.onCurrentSearchResultIndexChanged: {
                        _characterCheckBox.highlight = SearchAgent.currentSearchResultIndex >= 0
                        _charactersListView.ensureItemVisible(_characterCheckBox,1,10)
                    }
                    SearchAgent.onClearSearchRequest: _characterCheckBox.font.bold = false

                    width: _charactersListLayout.columnWidth
                    checkable: true
                    checked: _root_2.isInSelection(modelData)
                    text: modelData

                    onToggled: _root_2.toggleSelection(modelData)

                    background: Rectangle {
                        color: _characterCheckBox.Material.background
                    }

                    Connections {
                        target: _root_2

                        function onSelectedCharactersChanged() {
                            _characterCheckBox.checked = _root_2.isInSelection(_characterCheckBox.text)
                        }
                    }
                }
            }
        }
    }

    Row {
        id: _buttonsRow
        spacing: 20
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.leftMargin: 10
        anchors.bottomMargin: 4

        VclButton {
            text: "Select All"
            enabled: _root_2.count > 0
            onClicked: _root_2.selectAll()
        }

        VclButton {
            text: "Unselect All"
            enabled: _root_2.count > 0
            onClicked: _root_2.unselectAll()
        }
    }
}
