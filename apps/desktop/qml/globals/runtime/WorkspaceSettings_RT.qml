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

import QtCore

import io.scrite.components

Settings {
    location: Platform.settingsLocation
    category: "Workspace"

    property var customColors: []
    property var defaultSceneColor: SceneColors.palette[0]

    property bool animateNotebookIcon: true
    property bool animateStructureIcon: true
    property bool autoOpenLastFile: false
    property bool mouseWheelZoomsInCharacterGraph: Platform.isWindowsDesktop || Platform.isLinuxDesktop
    property bool mouseWheelZoomsInStructureCanvas: Platform.isWindowsDesktop || Platform.isLinuxDesktop
    property bool scriptalayIntroduced: false
    property bool showNotebookInStructure: true
    property bool showScritedTab: false
    property bool syncCurrentSceneOnNotebook: true

    property real flickScrollSpeedFactor: 1.0
    property real screenplayEditorWidth: -1
    property real workspaceHeight

    property string lastOpenExportFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
    property string lastOpenFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
    property string lastOpenImportFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
    property string lastOpenPhotosFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.PicturesLocation)
    property string lastOpenReportsFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
    property string lastOpenScritedFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.MoviesLocation)
}
