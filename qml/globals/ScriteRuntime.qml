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

pragma Singleton

import QtQuick 2.15
import Qt.labs.settings 1.0

import io.scrite.components 1.0

import "../../js/utils.js" as Utils


Item {
    id: scriteRuntime

    // These properties are only accessible at runtime, which means they are not
    // stored in persistent settings file.
    readonly property int e_ScreenplayTab: 0
    readonly property int e_StructureTab: 1
    readonly property int e_NotebookTab: 2
    readonly property int e_ScritedTab: 3
    property int mainWindowTab: e_ScreenplayTab
    signal activateMainWindowTab(int tabType)

    // This model provides access to recently accessed files. It is updated from
    // different parts of the UI where opening / saving of files is triggered.
    // Contents of this model is listed in the HomeScreen.
    readonly property ScriteFileListModel recentFiles: ScriteFileListModel {
        id: _recentFiles

        function addLater(filePath) {
            Utils.execLater(_recentFiles, 50, () => { _recentFiles.add(filePath) } )
        }

        onFilesChanged: _recentFilesSettings.files = files
        Component.onCompleted: files = _recentFilesSettings.files
    }


    // This model is how the screenplay of the current ScriteDocument is accessed.
    readonly property ScreenplayAdapter screenplayAdapter: ScreenplayAdapter {
        property string sessionId
        source: {
            if(Scrite.document.sessionId !== sessionId)
                return null

            if(mainWindowTab === e_ScreenplayTab)
                return Scrite.document.screenplay

            if(Scrite.document.screenplay.currentElementIndex < 0) {
                var index = Scrite.document.structure.currentElementIndex
                var element = Scrite.document.structure.elementAt(index)
                if(element) {
                    if(element.scene.addedToScreenplay) {
                        Scrite.document.screenplay.currentElementIndex = element.scene.screenplayElementIndexList[0]
                        return Scrite.document.screenplay
                    }
                    return element.scene
                }
            }

            return Scrite.document.screenplay
        }
    }

    // This property holds reference to an instance of ScreenplayEditor
    property Item screenplayEditor

    // This model provides access to the paginated-text-document constructed from the screenplay
    // of the current Scrite file.
    readonly property ScreenplayTextDocument screenplayTextDocument: ScreenplayTextDocument {
        // Setting this is as good as setting the other.
        // when paused = true, page and time computation is halted.
        property bool paused: ScriteSettings.screenplayEditor.pausePageAndTimeComputation
        onPausedChanged: Qt.callLater( function() {
            ScriteSettings.screenplayEditor.pausePageAndTimeComputation = screenplayTextDocument.paused
        })

        screenplay: Scrite.document.loading || paused ? null : ScriteRuntime.screenplayAdapter.screenplay
        formatting: Scrite.document.loading || paused ? null : Scrite.document.printFormat
        syncEnabled: true
        sceneNumbers: false
        titlePage: false
        sceneIcons: false
        listSceneCharacters: false
        includeSceneSynopsis: false
        printEachSceneOnANewPage: false
        secondsPerPage: Scrite.document.printFormat.secondsPerPage

        // FIXME: Do we really need this?
        Component.onCompleted: Scrite.app.registerObject(screenplayTextDocument, "screenplayTextDocument")
    }

    readonly property ScreenplayTracks screenplayTracks : ScreenplayTracks {
        screenplay: Scrite.document.screenplay
        Component.onCompleted: Scrite.app.registerObject(screenplayTracks, "screenplayTracks")
    }

    // Private objects
    Settings {
        id: _recentFilesSettings
        fileName: Scrite.app.settingsFilePath
        category: "RecentFiles"

        property var files
    }
}
