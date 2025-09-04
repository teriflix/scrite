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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

Rectangle {
    id: root

    readonly property alias searchBar: _searchBar
    readonly property alias searchAgent: _searchAgentLoader.searchAgent
    readonly property alias searchAgentItem: _searchAgentLoader.item

    signal replaceCurrentRequest(string replacementText, SearchAgent agent)

    width: ruler.width
    height: _searchBar.height * opacity

    color: Runtime.colors.primary.c100.background
    border.width: 1
    border.color: Runtime.colors.primary.borderColor

    visible: false
    enabled: Runtime.screenplayAdapter.screenplay

    SearchBar {
        id: _searchBar

        width: root.width * 0.6

        anchors.horizontalCenter: parent.horizontalCenter

        searchEngine.objectName: "Screenplay Search Engine"

        showReplace: false
        allowReplace: !Scrite.document.readOnly

        Loader {
            id: _searchAgentLoader

            property SearchAgent searchAgent: item ? item.SearchAgent : null

            active: Runtime.screenplayAdapter.screenplay ? true : false

            sourceComponent: Item {
                property var searchResults: []

                property int previousSceneIndex: -1

                property string searchString

                signal replaceCurrentRequest(string replacementText)

                SearchAgent.engine: _searchBar.searchEngine

                SearchAgent.onReplaceAll: (replacementText) => {
                                              Runtime.screenplayTextDocument.syncEnabled = false
                                              Runtime.screenplayAdapter.screenplay.replace(searchString, replacementText, 0)
                                              Runtime.screenplayTextDocument.syncEnabled = true
                                          }

                SearchAgent.onReplaceCurrent: (replacementText) => {
                                                  replaceCurrentRequest(replacementText)
                                              }

                SearchAgent.onSearchRequest: (string) => {
                                                 searchString = string
                                                 searchResults = Runtime.screenplayAdapter.screenplay.search(string, 0)
                                                 SearchAgent.searchResultCount = searchResults.length
                                             }

                SearchAgent.onCurrentSearchResultIndexChanged: () => {
                                                                   if(SearchAgent.currentSearchResultIndex >= 0) {
                                                                       var searchResult = searchResults[SearchAgent.currentSearchResultIndex]
                                                                       var sceneIndex = searchResult["sceneIndex"]
                                                                       if(sceneIndex !== previousSceneIndex)
                                                                       clearPreviousElementUserData()
                                                                       var sceneResultIndex = searchResult["sceneResultIndex"]
                                                                       var screenplayElement = Runtime.screenplayAdapter.screenplay.elementAt(sceneIndex)
                                                                       var data = {
                                                                           "searchString": searchString,
                                                                           "sceneResultIndex": sceneResultIndex,
                                                                           "currentSearchResultIndex": SearchAgent.currentSearchResultIndex,
                                                                           "searchResultCount": SearchAgent.searchResultCount
                                                                       }
                                                                       contentView.positionViewAtIndex(sceneIndex, ListView.Visible)
                                                                       screenplayElement.userData = data
                                                                       previousSceneIndex = sceneIndex
                                                                   }
                                                               }

                SearchAgent.onClearSearchRequest: () => {
                                                      Runtime.screenplayAdapter.screenplay.currentElementIndex = previousSceneIndex
                                                      searchString = ""
                                                      searchResults = []
                                                      clearPreviousElementUserData()
                                                  }

                function clearPreviousElementUserData() {
                    if(previousSceneIndex >= 0) {
                        const screenplayElement = Runtime.screenplayAdapter.screenplay.elementAt(previousSceneIndex)
                        if(screenplayElement)
                            screenplayElement.userData = undefined
                    }
                    previousSceneIndex = -1
                }
            }

            onItemChanged: {
                if(item) {
                    item.replaceCurrentRequest.connect( (replacementText) => {
                                                            root.replaceCurrentRequest(replacementText, searchAgent)
                                                       } )
                }
            }
        }

        onShowReplaceRequest: showReplace = flag
    }
}
