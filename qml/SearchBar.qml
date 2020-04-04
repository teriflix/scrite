/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
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
import Scrite 1.0

Item {
    width: 300
    height: 55
    property alias searchEngine: theSearchEngine

    SearchEngine {
        id: theSearchEngine
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: 4
        color: "lightgray"
        border.width: 1
        border.color: "gray"
        radius: 5
        enabled: theSearchEngine.searchAgentCount > 0

        TextField {
            id: txtSearch
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.right: buttonsRow.left
            anchors.margins: 5
            Keys.onReturnPressed:triggerSearch()
            Keys.onEscapePressed: {
                clear()
                theSearchEngine.clearSearch()
            }
            placeholderText: "search within " + theSearchEngine.searchAgentCount + " scenes."
            function triggerSearch() {
                if(theSearchEngine.searchString !== text)
                    theSearchEngine.searchString = text
                else
                    theSearchEngine.cycleSearchResult()
            }
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
                onClicked: theSearchEngine.clearSearch()
                suggestedHeight: 40
                hoverEnabled: false
            }
        }
    }
}
