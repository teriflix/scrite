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
    width: 300
    height: 55
    property alias searchEngine: theSearchEngine
    property real borderWidth: 0

    SearchEngine {
        id: theSearchEngine
    }

    Rectangle {
        id: rect
        anchors.fill: parent
        anchors.margins: 4
        color: primaryColors.c10.background
        enabled: theSearchEngine.searchAgentCount > 0
        border.width: borderWidth
        border.color: primaryColors.borderColor

        TextArea {
            id: txtSearch
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.right: buttonsRow.left
            anchors.margins: 5
            palette: app.palette
            // renderType: Text.NativeRendering
            Material.primary: primaryColors.key
            Material.accent: accentColors.key
            background: Item {
                Rectangle {
                    width: parent.width
                    height: txtSearch.activeFocus ? 2 : 1
                    color: accentColors.c700.background
                    visible: txtSearch.enabled
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 4
                }
            }
            Keys.onReturnPressed: {
                Transliterator.transliterateLastWord()
                triggerSearch()
            }
            Keys.onEscapePressed: {
                if(searchEngine.searchResultCount > 0) {
                    clearSearch()
                    event.accepted = true
                }
            }
            placeholderText: "search"
            function triggerSearch() {
                var ss = text.trim()
                if(theSearchEngine.searchString !== ss)
                    theSearchEngine.searchString = ss
                else
                    theSearchEngine.cycleSearchResult()
            }
            function clearSearch() {
                clear()
                theSearchEngine.clearSearch()
            }
            Transliterator.textDocument: textDocument
            Transliterator.cursorPosition: cursorPosition
            Transliterator.hasActiveFocus: activeFocus
        }

        Row {
            id: buttonsRow
            spacing: 2
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 5

            ToolButton2 {
                icon.source: "../icons/action/search.png"
                anchors.verticalCenter: parent.verticalCenter
                suggestedHeight: 40
                onClicked: txtSearch.triggerSearch()
                hoverEnabled: false
            }

            Text {
                text: {
                    if(theSearchEngine.searchResultCount > 0)
                        return "  " +  (theSearchEngine.currentSearchResultIndex+1) + "/" + theSearchEngine.searchResultCount + "  "
                    return ""
                }
                anchors.verticalCenter: parent.verticalCenter
            }

            ToolButton2 {
                icon.source: "../icons/action/keyboard_arrow_up.png"
                anchors.verticalCenter: parent.verticalCenter
                enabled: theSearchEngine.searchResultCount > 0 && theSearchEngine.currentSearchResultIndex > 0
                onClicked: theSearchEngine.previousSearchResult()
                suggestedHeight: 40
                hoverEnabled: false
            }

            ToolButton2 {
                icon.source: "../icons/action/keyboard_arrow_down.png"
                anchors.verticalCenter: parent.verticalCenter
                enabled: theSearchEngine.searchResultCount > 0 && theSearchEngine.currentSearchResultIndex < theSearchEngine.searchResultCount
                onClicked: theSearchEngine.nextSearchResult()
                suggestedHeight: 40
                hoverEnabled: false
            }

            ToolButton2 {
                icon.source: "../icons/navigation/close.png"
                anchors.verticalCenter: parent.verticalCenter
                enabled: theSearchEngine.searchResultCount > 0
                onClicked: txtSearch.clearSearch()
                suggestedHeight: 40
                hoverEnabled: false
            }
        }
    }
}
