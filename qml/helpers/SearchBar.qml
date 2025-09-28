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

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"

Item {
    id: root

    property bool hasFocus: _txtSearch.activeFocus || _txtReplace.activeFocus
    property bool showReplace: false
    property bool allowReplace: false

    property real borderWidth: 0

    property SearchEngine searchEngine: SearchEngine { }

    signal showReplaceRequest(bool flag)

    function assumeFocus() {
        _txtSearch.forceActiveFocus()
    }

    width: implicitWidth
    height: implicitHeight
    implicitWidth: 300
    implicitHeight: _layout.height

    clip: true

    Behavior on height {
        enabled: Runtime.applicationSettings.enableAnimations

        NumberAnimation { duration: Runtime.stdAnimationDuration }
    }

    Column {
        id: _layout

        width: parent.width

        spacing: 10

        Rectangle {
            width: parent.width
            height: Math.max(55, Math.max(Math.max(_txtSearch.height, _searchButtonsRow.height), (_replaceUiRect.visible ? _replaceUiRect.height : 0)))

            color: Runtime.colors.primary.c10.background
            border.width: borderWidth
            border.color: Runtime.colors.primary.borderColor

            enabled: searchEngine.searchAgentCount > 0

            TextAreaInput {
                id: _txtSearch

                property bool canClear: searchEngine.searchResultCount > 0 || text !== ""

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

                Keys.onReturnPressed: (event) => {
                                          triggerSearch()
                                      }

                Keys.onEscapePressed: (event) => {
                                          if(canClear) {
                                              clearSearch()
                                              event.accepted = true
                                          }
                                      }

                KeyNavigation.tab: _replaceUiRect.visible ? _txtReplace : null
                KeyNavigation.priority: KeyNavigation.BeforeItem

                anchors.left: parent.left
                anchors.right: _searchButtonsRow.left
                anchors.margins: 5
                anchors.verticalCenter: parent.verticalCenter

                placeholderText: "Search"

                onActiveFocusChanged: {
                    if(activeFocus)
                        selectAll()
                }
            }

            Row {
                id: _searchButtonsRow

                anchors.right: parent.right
                anchors.rightMargin: 5
                anchors.verticalCenter: parent.verticalCenter

                spacing: 2

                VclToolButton {
                    anchors.verticalCenter: parent.verticalCenter

                    suggestedHeight: 40

                    hoverEnabled: false

                    icon.source: "qrc:/icons/action/search.png"

                    VclMenu {
                        id: _optionsMenu

                        VclMenuItem {
                            text: "Case Sensitive"
                            checked: searchEngine.isSearchCaseSensitive
                            checkable: true

                            onToggled: searchEngine.isSearchCaseSensitive = checked
                        }

                        VclMenuItem {
                            text: "Whole Words"
                            checked: searchEngine.isSearchWholeWords
                            checkable: true

                            onToggled: searchEngine.isSearchWholeWords = checked
                        }
                    }

                    onClicked: _txtSearch.triggerSearch()

                    onPressAndHold: _optionsMenu.popup()
                }

                VclLabel {
                    anchors.verticalCenter: parent.verticalCenter

                    text: {
                        if(searchEngine.searchResultCount > 0)
                            return "  " +  (searchEngine.currentSearchResultIndex+1) + "/" + searchEngine.searchResultCount + "  "
                        return ""
                    }
                }

                VclToolButton {
                    anchors.verticalCenter: parent.verticalCenter

                    suggestedHeight: 40

                    enabled: searchEngine.searchResultCount > 0 && searchEngine.currentSearchResultIndex > 0
                    hoverEnabled: false

                    icon.source: "qrc:/icons/action/keyboard_arrow_up.png"

                    onClicked: searchEngine.previousSearchResult()
                }

                VclToolButton {
                    anchors.verticalCenter: parent.verticalCenter

                    suggestedHeight: 40

                    enabled: searchEngine.searchResultCount > 0 && searchEngine.currentSearchResultIndex < searchEngine.searchResultCount
                    hoverEnabled: false

                    icon.source: "qrc:/icons/action/keyboard_arrow_down.png"

                    onClicked: searchEngine.nextSearchResult()
                }

                VclToolButton {
                    anchors.verticalCenter: parent.verticalCenter

                    suggestedHeight: 40

                    enabled: _txtSearch.canClear
                    hoverEnabled: false

                    icon.source: "qrc:/icons/navigation/close.png"

                    onClicked: _txtSearch.clearSearch()
                }

                VclToolButton {
                    ToolTip.text: (checked ? "Hide replace field." : "Show replace field.") + " (" + Scrite.app.polishShortcutTextForDisplay("Ctrl+Shift+F") + ")"

                    anchors.verticalCenter: parent.verticalCenter

                    suggestedHeight: 40

                    down: checked
                    checked: _replaceUiRect.visible
                    visible: allowReplace
                    checkable: true
                    hoverEnabled: true

                    icon.source: "qrc:/icons/action/find_replace.png"

                    onToggled: showReplaceRequest(!showReplace)
                }
            }
        }

        Rectangle {
            id: _replaceUiRect

            width: parent.width
            height: Math.max(_txtReplace.height, _replaceButtonsRow.height)

            color: Runtime.colors.primary.c10.background
            visible: showReplace
            enabled: searchEngine.searchAgentCount > 0

            border.width: borderWidth
            border.color: Runtime.colors.primary.borderColor

            TextAreaInput {
                id: _txtReplace

                Keys.onReturnPressed: _cmdReplace.click()

                KeyNavigation.backtab: _txtSearch
                KeyNavigation.priority: KeyNavigation.BeforeItem

                anchors.left: parent.left
                anchors.right: _replaceButtonsRow.left
                anchors.margins: 5
                anchors.verticalCenter: parent.verticalCenter

                placeholderText: "Replace"

                onActiveFocusChanged: {
                    if(activeFocus && searchEngine.searchResultCount === 0)
                        _txtSearch.triggerSearch()
                }
            }

            Row {
                id: _replaceButtonsRow

                spacing: 10
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: 5

                VclButton {
                    id: _cmdReplace

                    text: "Replace"
                    enabled: _txtReplace.text.length > 0 && _txtSearch.text.length > 0 && searchEngine.currentSearchResultIndex >= 0 && searchEngine.searchResultCount > 0

                    onClicked: click()

                    function click() {
                        searchEngine.replace(_txtReplace.text)
                        Utils.execLater(searchEngine, 250, function() { searchEngine.nextSearchResult() })
                    }
                }

                VclButton {
                    text: "Replace All"

                    enabled: _txtReplace.text.length > 0 && _txtSearch.text.length > 0 && searchEngine.searchResultCount > 0

                    onClicked: searchEngine.replaceAll(_txtReplace.text)
                }
            }
        }
    }
}
