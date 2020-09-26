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

import Scrite 1.0
import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Controls.Material 2.12

Rectangle {
    property bool showFilterBox: true
    property var selectedCharacters: []
    property int sortFilterRole: 0

    color: primaryColors.c10.background
    border { width: 1; color: primaryColors.borderColor }
    implicitWidth: charactersListLayout.width

    property GenericArrayModel charactersModel: GenericArrayModel {
        array: scriteDocument.structure.characterNames
    }

    SearchBar {
        id: searchBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        searchEngine.objectName: "Characters Search Engine"
    }

    ScrollArea {
        id: charactersListView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: searchBar.bottom
        anchors.bottom: parent.bottom
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
                    checked: selectedCharacters.indexOf(text) >= 0 || (charactersModel.arrayHasObjects ? arrayItem.added : false)
                    enabled: charactersModel.arrayHasObjects ? (arrayItem.added === false) : true
                    text: charactersModel.arrayHasObjects ? arrayItem.name : arrayItem
                    onToggled: {
                        var chs = selectedCharacters
                        if(checked)
                            chs.push(text)
                        else
                            chs.splice(chs.indexOf(text), 1)
                        selectedCharacters = chs
                    }

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
                }
            }
        }
    }
}
