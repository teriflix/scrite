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
import "../js/utils.js" as Utils

Item {
    id: searchBar
    width: 300
    height: searchBarLayout.height
    property real borderWidth: 0
    property bool hasFocus: txtSearch.activeFocus || txtReplace.activeFocus
    property bool allowReplace: false
    property bool showReplace: false
    clip: true

    Behavior on height {
        enabled: applicationSettings.enableAnimations
        NumberAnimation { duration: 100 }
    }

    signal showReplaceRequest(bool flag)

    function assumeFocus() {
        txtSearch.forceActiveFocus()
    }

    property SearchEngine searchEngine: SearchEngine { }

    Column {
        id: searchBarLayout
        width: parent.width
        spacing: 10

        Rectangle {
            id: findUiRect
            width: parent.width
            height: Math.max(55, Math.max(Math.max(txtSearch.height, searchButtonsRow.height), (replaceUiRect.visible ? replaceUiRect.height : 0)))
            color: primaryColors.c10.background
            enabled: searchEngine.searchAgentCount > 0
            border.width: borderWidth
            border.color: primaryColors.borderColor

            TextAreaInput {
                id: txtSearch
                property bool canClear: searchEngine.searchResultCount > 0 || text !== ""
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.right: searchButtonsRow.left
                anchors.margins: 5
                placeholderText: "search"
                KeyNavigation.tab: replaceUiRect.visible ? txtReplace : null
                KeyNavigation.priority: KeyNavigation.BeforeItem
                Keys.onReturnPressed: triggerSearch()
                Keys.onEscapePressed: {
                    if(canClear) {
                        clearSearch()
                        event.accepted = true
                    }
                }
                function triggerSearch() {
                    var ss = text.trim()
                    if(searchEngine.searchString !== ss)
                        searchEngine.searchString = ss
                    else
                        searchEngine.cycleSearchResult()
                }
                function clearSearch() {
                    clear()
                    searchEngine.clearSearch()
                }
                onActiveFocusChanged: {
                    if(activeFocus)
                        selectAll()
                }
            }

            Row {
                id: searchButtonsRow
                spacing: 2
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: 5

                ToolButton2 {
                    icon.source: "../icons/action/search.png"
                    anchors.verticalCenter: parent.verticalCenter
                    suggestedHeight: 40
                    onClicked: txtSearch.triggerSearch()
                    onPressAndHold: searchOptionsMenu.popup()
                    hoverEnabled: false

                    Menu2 {
                        id: searchOptionsMenu

                        MenuItem2 {
                            text: "Case Sensitive"
                            checkable: true
                            checked: searchEngine.isSearchCaseSensitive
                            onToggled: searchEngine.isSearchCaseSensitive = checked
                        }

                        MenuItem2 {
                            text: "Whole Words"
                            checkable: true
                            checked: searchEngine.isSearchWholeWords
                            onToggled: searchEngine.isSearchWholeWords = checked
                        }
                    }
                }

                Text {
                    text: {
                        if(searchEngine.searchResultCount > 0)
                            return "  " +  (searchEngine.currentSearchResultIndex+1) + "/" + searchEngine.searchResultCount + "  "
                        return ""
                    }
                    anchors.verticalCenter: parent.verticalCenter
                }

                ToolButton2 {
                    icon.source: "../icons/action/keyboard_arrow_up.png"
                    anchors.verticalCenter: parent.verticalCenter
                    enabled: searchEngine.searchResultCount > 0 && searchEngine.currentSearchResultIndex > 0
                    onClicked: searchEngine.previousSearchResult()
                    suggestedHeight: 40
                    hoverEnabled: false
                }

                ToolButton2 {
                    icon.source: "../icons/action/keyboard_arrow_down.png"
                    anchors.verticalCenter: parent.verticalCenter
                    enabled: searchEngine.searchResultCount > 0 && searchEngine.currentSearchResultIndex < searchEngine.searchResultCount
                    onClicked: searchEngine.nextSearchResult()
                    suggestedHeight: 40
                    hoverEnabled: false
                }

                ToolButton2 {
                    icon.source: "../icons/navigation/close.png"
                    anchors.verticalCenter: parent.verticalCenter
                    enabled: txtSearch.canClear
                    onClicked: txtSearch.clearSearch()
                    suggestedHeight: 40
                    hoverEnabled: false
                }

                ToolButton2 {
                    icon.source: "../icons/action/find_replace.png"
                    anchors.verticalCenter: parent.verticalCenter
                    down: checked
                    checked: replaceUiRect.visible
                    checkable: true
                    onToggled: showReplaceRequest(!showReplace)
                    suggestedHeight: 40
                    hoverEnabled: true
                    visible: allowReplace
                    ToolTip.text: (checked ? "Hide replace field." : "Show replace field.") + " (" + Scrite.app.polishShortcutTextForDisplay("Ctrl+Shift+F") + ")"
                }
            }
        }

        Rectangle {
            id: replaceUiRect
            visible: showReplace
            width: parent.width
            height: Math.max(txtReplace.height, replaceButtonsRow.height)
            color: primaryColors.c10.background
            enabled: searchEngine.searchAgentCount > 0
            border.width: borderWidth
            border.color: primaryColors.borderColor

            TextAreaInput {
                id: txtReplace
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.right: replaceButtonsRow.left
                anchors.margins: 5
                placeholderText: "replace"
                KeyNavigation.backtab: txtSearch
                KeyNavigation.priority: KeyNavigation.BeforeItem
                Keys.onReturnPressed: cmdReplace.click()
                onActiveFocusChanged: {
                    if(activeFocus && searchEngine.searchResultCount === 0)
                        txtSearch.triggerSearch()
                }
            }

            Row {
                id: replaceButtonsRow
                spacing: 10
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: 5

                Button2 {
                    id: cmdReplace
                    text: "Replace"
                    enabled: txtReplace.text.length > 0 && txtSearch.text.length > 0 && searchEngine.currentSearchResultIndex >= 0 && searchEngine.searchResultCount > 0
                    onClicked: click()
                    function click() {
                        searchEngine.replace(txtReplace.text)
                        Utils.execLater(searchEngine, 250, function() { searchEngine.nextSearchResult() })
                    }
                }

                Button2 {
                    text: "Replace All"
                    enabled: txtReplace.text.length > 0 && txtSearch.text.length > 0 && searchEngine.searchResultCount > 0
                    onClicked: searchEngine.replaceAll(txtReplace.text)
                }
            }
        }
    }
}
