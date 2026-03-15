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

import QtQuick
import QtCore

import io.scrite.components

ScriteFileListModel {
    id: root

    required property ApplicationSettings_RT applicationSettings

    readonly property Settings recentFilesSettings : Settings {
        property var files: []

        property alias missingFiles: root.missingFiles
        property alias preferTitleVersionText: root.preferTitleVersionText

        category: "RecentFiles"
        location: Platform.settingsLocation
    }

    property var missingFiles: []
    property bool preferTitleVersionText: true

    Component.onCompleted: {
        Scrite.document.justLoaded.connect(onDocumentJustLoaded)
        Scrite.document.justSaved.connect(onDocumentJustSaved)
        filesChanged.connect(captureChangeInFiles)
    }

    function onDocumentJustSaved() {
        Qt.callLater(addDocumentFile)
    }

    function onDocumentJustLoaded() {
        Qt.callLater(addDocumentFile)
    }

    function addDocumentFile() {
        const docFilePath = Scrite.document.fileName
        if(docFilePath !== "")
            add(docFilePath)

        // Remove this file from missing files list.
        if(Array.isArray(missingFiles) || missingFiles.length) {
            let f = missingFiles
            missingFiles = f.filter(item => item === docFilePath);
        }
    }

    function captureChangeInFiles() {
        recentFilesSettings.files = files
    }

    notifyMissingFiles: applicationSettings.notifyMissingRecentFiles
    source: ScriteFileListModel.RecentFiles

    onFilesMissing: (files) => {
        let f = Array.isArray(missingFiles) || missingFiles.length ? missingFiles : []
        f.push(...files)
        missingFiles = f
    }

    onNotifyMissingFilesChanged: {
        if(!notifyMissingFiles)
            missingFiles = []
    }
}
