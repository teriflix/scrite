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

import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Controls.Material 2.12
import Scrite 1.0

Item {
    id: searchBar
    width: 300
    height: searchBarLayout.height
    property real borderWidth: 0
    property bool hasFocus: txtSearch.activeFocus
    property bool allowReplace: false
    property bool focusOnShortcut: false
    clip: true

    Shortcut {
        sequence: "Ctrl+F"
        enabled: searchBar.focusOnShortcut
        context: Qt.WindowShortcut
        onActivated: searchBar.forceSearchFocus()
    }

    Shortcut {
        sequence: "Ctrl+Shift+F"
        enabled: searchBar.focusOnShortcut && allowReplace
        context: Qt.WindowShortcut
        onActivated: {
            replaceUiRect.visible = true
            if(txtSearch.text === "")
                searchBar.forceSearchFocus()
            else
                searchBar.forceReplaceFocus()
        }
    }

    function forceSearchFocus() {
        txtSearch.forceActiveFocus()
    }

    function forceReplaceFocus() {
        if(txtReplace.visible)
            txtReplace.forceActiveFocus()
        else
            txtSearch.forceActiveFocus()
    }

    Behavior on height {
        enabled: screenplayEditorSettings.enableAnimations
        NumberAnimation { duration: 100 }
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
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.right: searchButtonsRow.left
                anchors.margins: 5
                placeholderText: "search"
                KeyNavigation.tab: replaceUiRect.visible ? txtReplace : null
                KeyNavigation.priority: KeyNavigation.BeforeItem
                Keys.onReturnPressed: triggerSearch()
                Keys.onEscapePressed: {
                    if(searchEngine.searchResultCount > 0) {
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
                    enabled: searchEngine.searchResultCount > 0
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
                    onToggled: replaceUiRect.visible = checked
                    suggestedHeight: 40
                    hoverEnabled: true
                    visible: allowReplace
                    ToolTip.text: checked ? "Hide replace field." : "Show replace field."
                }
            }
        }

        Rectangle {
            id: replaceUiRect
            visible: false
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
                        app.execLater(searchEngine, 250, function() { searchEngine.nextSearchResult() })
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
