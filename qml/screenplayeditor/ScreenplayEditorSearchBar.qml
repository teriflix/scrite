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


import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

Rectangle {
    id: root

    required property ScreenplayAdapter screenplayAdapter

    readonly property alias searchBar: _searchBar
    readonly property alias searchAgent: _searchAgentLoader.searchAgent
    readonly property alias searchEngine: _searchBar.searchEngine
    readonly property alias searchAgentItem: _searchAgentLoader.item

    signal replaceCurrentRequest(string replacementText, SearchAgent agent)

    implicitHeight: _searchBar.height * opacity

    color: Runtime.colors.primary.c100.background
    border.width: 1
    border.color: Runtime.colors.primary.borderColor

    visible: false
    enabled: _private.screenplay !== null

    SearchBar {
        id: _searchBar

        width: root.width * 0.6

        anchors.horizontalCenter: parent.horizontalCenter

        searchEngine.objectName: "Screenplay Search Engine"

        showReplace: false
        allowReplace: !Scrite.document.readOnly

        Loader {
            id: _searchAgentLoader

            property SearchAgent searchAgent: item ? Scrite.app.findFirstChildOfType(item, "SearchAgent") : null

            active: _private.screenplay ? true : false

            sourceComponent: Item {
                property var searchResults: []

                property int previousSceneIndex: -1

                property string searchString

                signal replaceCurrentRequest(string replacementText)

                SearchAgent.engine: _searchBar.searchEngine

                SearchAgent.onReplaceAll: (replacementText) => {
                                              _private.runReplaceAllTask(searchString, replacementText)
                                          }

                SearchAgent.onReplaceCurrent: (replacementText) => {
                                                  replaceCurrentRequest(replacementText)
                                              }

                SearchAgent.onSearchRequest: (string) => {
                                                 searchString = string
                                                 searchResults = _private.screenplay.search(string, 0)
                                                 SearchAgent.searchResultCount = searchResults.length
                                             }

                SearchAgent.onCurrentSearchResultIndexChanged: () => {
                                                                   if(SearchAgent.currentSearchResultIndex >= 0) {
                                                                       let searchResult = searchResults[SearchAgent.currentSearchResultIndex]
                                                                       let sceneIndex = searchResult["sceneIndex"]
                                                                       if(sceneIndex !== previousSceneIndex) {
                                                                           clearPreviousElementUserData()
                                                                       }

                                                                       let sceneResultIndex = searchResult["sceneResultIndex"]
                                                                       let screenplayElement = _private.screenplay.elementAt(sceneIndex)
                                                                       let data = {
                                                                           "searchString": searchString,
                                                                           "sceneResultIndex": sceneResultIndex,
                                                                           "currentSearchResultIndex": SearchAgent.currentSearchResultIndex,
                                                                           "searchResultCount": SearchAgent.searchResultCount
                                                                       }

                                                                       // contentView.positionViewAtIndex(sceneIndex, ListView.Visible)
                                                                       root.screenplayAdapter.currentIndex = sceneIndex
                                                                       screenplayElement.userData = data

                                                                       previousSceneIndex = sceneIndex
                                                                   }
                                                               }

                SearchAgent.onClearSearchRequest: () => {
                                                      root.screenplayAdapter.currentIndex = previousSceneIndex
                                                      searchString = ""
                                                      searchResults = []
                                                      clearPreviousElementUserData()
                                                  }

                function clearPreviousElementUserData() {
                    if(previousSceneIndex >= 0) {
                        const screenplayElement = _private.screenplay.elementAt(previousSceneIndex)
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

    QtObject {
        id: _private

        property Screenplay screenplay: root.screenplayAdapter ? root.screenplayAdapter.screenplay : null

        property Component replaceAllTask: SequentialAnimation {
            id: _replaceAllTask

            required property string searchString
            required property string replacementText

            property VclDialog waitDialog

            property int nrReplacements: 0

            ScriptAction {
                script: {
                    Runtime.screenplayTextDocument.syncEnabled = false
                    _replaceAllTask.waitDialog = WaitDialog.launch("Please wait while replacing text ...")
                }
            }

            PauseAnimation {
                duration: Runtime.stdAnimationDuration
            }

            ScriptAction {
                script: {
                    _replaceAllTask.nrReplacements = _private.screenplay.replace(_replaceAllTask.searchString, _replaceAllTask.replacementText, 0)
                    Runtime.screenplayTextDocument.syncEnabled = true
                    _replaceAllTask.waitDialog.close()
                }
            }

            PauseAnimation {
                duration: Runtime.stdAnimationDuration
            }
        }

        function runReplaceAllTask(searchString, replacementText) {
            let task = replaceAllTask.createObject(root, {"searchString": searchString, "replacementText": replacementText})
            task.finished.connect(() => {
                                      const nrReplacements = task.nrReplacements
                                      task.destroy()
                                      MessageBox.information("Find & Replace", "Replaced " + nrReplacements + " instance(s).")
                                  })
            task.start()
        }
    }
}
