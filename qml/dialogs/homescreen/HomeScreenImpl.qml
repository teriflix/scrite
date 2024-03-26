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

/**
  This is a replacement for a bunch of UI elements that were prevalent in
  Scrite until 0.9.4f, viz:
  - New From Template (dialog)
  - Scriptalay (dialog)
  - Recent Files (menu)
  - Vault (dialog)
  - Import (menu)
  - Open (menu item)
  In the future, we may add support for open file from Google Drive in this same
  dialog box.

  By introducing this dialog box, we will deprecate (as in delete) all the QML
  files associated with the dialog boxes and menus listed above. This will make
  things streamlined and provide more open space in the main-window.
*/

import QtQuick 2.15
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"

Item {
    id: homeScreen

    property string mode
    signal closeRequest()

    Component.onCompleted: {
        Utils.execLater(homeScreen, 500, () => {
                            if(mode === "Scriptalay")
                                stackView.push(scriptalayPage)
                        })
    }

    Image {
        id: banner
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        source: {
            if(stackView.currentItem && stackView.currentItem.bannerImage)
                return stackView.currentItem.bannerImage
            return _private.defaultBannerImage
        }
        fillMode: Image.PreserveAspectFit
        visible: banner.height <= homeScreen.height * 0.6

        Image {
            anchors.centerIn: parent
            source: (stackView.currentItem && stackView.currentItem.bannerImage) ? "" : "qrc:/images/banner_logo_overlay.png"
            width: homeScreen.width * 0.2
            fillMode: Image.PreserveAspectFit
            smooth: true; mipmap: true
            visible: false
            Component.onCompleted: Utils.execLater(banner, 50, () => { visible = true } )
        }

        VclText {
            id: commonToolTip
            width: parent.width * 0.75
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.leftMargin: 20
            anchors.bottomMargin: 20

            font.pointSize: Runtime.idealFontMetrics.font.pointSize
            padding: 8
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignBottom
            wrapMode: Text.WordWrap
            color: "white"
            visible: text !== ""
            maximumLineCount: 4
            elide: Text.ElideRight

            property Item source

            function show(_source, _text) {
                source = _source
                text = _text
            }

            function hide(_source) {
                if(source === _source)
                    text = ""
            }
        }

        Poster {
            id: poster
            anchors.fill: parent

            property Item sourceItem

            function show(_source, _image, _logline) {
                sourceItem = _source
                source = _image
                logline = _logline
            }

            function hide(_source) {
                if(sourceItem === _source) {
                    source = undefined
                    logline = ""
                }
            }
        }
    }

    StackView {
        id: stackView
        anchors.top: banner.visible ? banner.bottom : parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        clip: true

        initialItem: ContentPage1 { }
    }

    BusyOverlay {
        id: homeScreenBusyOverlay
        anchors.fill: parent
        busyMessage: "Fetching content ..."
        visible: libraryService.busy
    }

    Loader {
        id: saveWorkflow
        anchors.fill: parent
        active: false
        property var operation: null

        function launch(op) {
            operation = op
            active = true
        }

        sourceComponent: SaveWorkflow {
            onDone: {
                saveWorkflow.operation = null
                saveWorkflow.active = false
            }
            operation: saveWorkflow.operation
        }
    }

    LibraryService {
        id: libraryService

        onImportStarted: (index) => {
                             homeScreen.enabled = false
                             Runtime.loadMainUiContent = false
                         }

        onImportFinished: (index) => {
                              Runtime.loadMainUiContent = true
                              Utils.execLater(libraryService, 250, function() {
                                  closeRequest()
                              })
                          }
    }

    component ContentPage1 : Item {
        RowLayout {
            anchors.fill: parent
            anchors.topMargin: 25
            anchors.leftMargin: 50
            anchors.rightMargin: 50
            anchors.bottomMargin: 25
            spacing: 20

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10

                    NewFileOptions {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }

                    OpenFileOptions {
                        Layout.fillWidth: true
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10

                    QuickFileOpenOptions {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }

                    ImportOptions {
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }

    Component {
        id: iconFromSource

        Image {
            fillMode: Image.PreserveAspectFit
            smooth: true; mipmap: true
        }
    }

    Component {
        id: iconFromImage

        QImageItem {
            fillMode: QImageItem.PreserveAspectFit
            useSoftwareRenderer: Runtime.currentUseSoftwareRenderer
        }
    }

    component LinkButton : Rectangle {
        id: button
        property string text
        property string tooltip
        property string iconSource
        property var iconImage // has to be QImage
        property bool singleClick: true
        property bool showPoster: false

        signal clicked()
        signal doubleClicked()

        width: 100
        height: buttonLayout.height + 6
        color: buttonMouseArea.containsMouse ? Runtime.colors.primary.highlight.background : Qt.rgba(0,0,0,0)

        RowLayout {
            id: buttonLayout
            width: parent.width - 10
            anchors.centerIn: parent
            spacing: 5
            opacity: enabled ? 1 : 0.6

            Loader {
                property real h: buttonLabel.contentHeight * 1.5
                Layout.preferredWidth: h
                Layout.preferredHeight: h
                sourceComponent: {
                    if(iconSource !== "")
                        return iconFromSource
                    return iconFromImage
                }
                onLoaded: {
                    if(iconSource !== "")
                        item.source = Qt.binding( () => { return iconSource } )
                    else
                        item.image = Qt.binding( () => { return iconImage } )
                }
            }

            VclText {
                id: buttonLabel
                padding: 3
                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                font.underline: singleClick ? buttonMouseArea.containsMouse : false
                text: button.text
                Layout.fillWidth: true
                elide: Text.ElideRight
            }
        }

        MouseArea {
            id: buttonMouseArea
            anchors.fill: parent
            hoverEnabled: singleClick || button.tooltip !== ""
            cursorShape: singleClick ? Qt.PointingHandCursor : Qt.ArrowCursor
            onClicked: button.clicked()
            onDoubleClicked: button.doubleClicked()
            onEntered: {
                if(hoverEnabled) {
                    commonToolTip.show(parent, parent.tooltip)
                    if(parent.showPoster) {
                        if(parent.iconSource !== "")
                            poster.show(parent, parent.iconSource, parent.tooltip)
                        else
                            poster.show(parent, parent.iconImage, parent.tooltip)
                    }
                }
            }
            onExited: {
                if(hoverEnabled) {
                    commonToolTip.hide(parent)
                    poster.hide(parent)
                }
            }
        }
    }

    component NewFileOptions : Item {
        ColumnLayout {
            anchors.fill: parent

            VclText {
                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                text: "New File"
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                // Layout.leftMargin: 20
                Layout.rightMargin: 20
                color: Qt.rgba(0,0,0,0)
                border.width: templatesView.interactive ? 1 : 0
                border.color: Runtime.colors.primary.borderColor

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 1
                    visible: !Runtime.appFeatures.templates.enabled

                    LinkButton {
                        text: "Blank Document"
                        iconSource: "qrc:/icons/filetype/document.png"
                        Layout.fillWidth: true
                        tooltip: "Creates a new blank Scrite document."
                        onClicked: {
                            saveWorkflow.launch( () => {
                                                    homeScreen.enabled = false
                                                    Scrite.document.reset()
                                                    closeRequest()
                                                } )
                        }
                    }

                    DisabledFeatureNotice {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        color: Qt.rgba(1,1,1,0.9)
                        featureName: "Screenplay Templates"
                        visible: !Runtime.appFeatures.templates.enabled
                    }
                }

                ListView {
                    id: templatesView
                    anchors.fill: parent
                    anchors.margins: 1
                    model: Runtime.appFeatures.templates.enabled ? libraryService.templates : []
                    visible: Runtime.appFeatures.templates.enabled
                    currentIndex: -1
                    clip: true
                    FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor
                    ScrollBar.vertical: VclScrollBar {
                        flickable: templatesView
                    }
                    highlightMoveDuration: 0
                    interactive: height < contentHeight
                    delegate: LinkButton {
                        required property int index
                        required property var record
                        width: templatesView.width
                        text: record.name
                        tooltip: record.description
                        iconSource: index === 0 ? record.poster : libraryService.templates.baseUrl + "/" + record.poster
                        showPoster: index > 0
                        onClicked: {
                            saveWorkflow.launch( () => {
                                                    libraryService.openTemplateAt(index)
                                                } )
                        }
                    }
                }
            }
        }
    }

    component OpenFileOptions : ColumnLayout {
        LinkButton {
            text: "Open ..."
            iconSource: "qrc:/icons/file/folder_open.png"
            Layout.fillWidth: true
            tooltip: "Launches a file dialog box so you can select a .scrite file to load from disk."
            onClicked: {
                saveWorkflow.launch( () => {
                                        openFileDialog.open()
                                    } )
            }
        }

        LinkButton {
            text: Runtime.recentFiles.count === 0 ? "Recent files ..." : "Scriptalay"
            iconSource: Runtime.recentFiles.count === 0 ? "qrc:/icons/filetype/document.png" : "qrc:/icons/action/library.png"
            Layout.fillWidth: true
            tooltip: Runtime.recentFiles.count === 0 ? "Reopen a recently opened file." : "Download a screenplay from our online library."
            onClicked: stackView.push(scriptalayPage)
            enabled: Runtime.recentFiles.count > 0
        }
    }

    Component {
        id: quickFilesRecentFilesDelegate

        LinkButton {
            required property int index
            required property var fileInfo

            width: ListView.view.width
            text: fileInfo.title === "" ? fileInfo.baseFileName : fileInfo.title
            tooltip: fileInfo.logline
            iconSource: fileInfo.hasCoverPage ? "" : "qrc:/icons/filetype/document.png"
            iconImage: fileInfo.hasCoverPage ? fileInfo.coverPageImage : null
            showPoster: fileInfo.hasCoverPage
            onClicked: {
                saveWorkflow.launch( () => {
                                      _private.openScriteDocument(fileInfo.filePath)
                                    } )
            }
        }
    }

    Component {
        id: quickFilesScriptalayDelegate

        LinkButton {
            required property int index
            required property var record

            width: ListView.view.width
            text: record.name
            tooltip: "<i>" + record.authors + "</i><br/><br/>" + record.logline
            iconSource: libraryService.screenplays.baseUrl + "/" + record.poster
            showPoster: true
            onClicked: {
                saveWorkflow.launch( () => {
                                        libraryService.openScreenplayAt(index)
                                    } )
            }
        }
    }

    // This component should show "Recent Files", if recent files exist
    // It should show Scriptalay Scripts, if no recent files exist.
    component QuickFileOpenOptions : Item {
        property bool scriptalayMode: Runtime.recentFiles.count === 0

        ColumnLayout {
            anchors.fill: parent

            VclText {
                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                text: scriptalayMode ? "Scriptalay" : "Recent Files"
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                // Layout.leftMargin: 20
                Layout.rightMargin: 20
                color: Qt.rgba(0,0,0,0)
                border.width: quickFilesView.interactive ? 1 : 0
                border.color: Runtime.colors.primary.borderColor

                ListView {
                    id: quickFilesView // shows either Scriptalay or Recent Files
                    anchors.fill: parent
                    anchors.margins: 1
                    model: scriptalayMode ? libraryService.screenplays : Runtime.recentFiles
                    currentIndex: -1
                    clip: true
                    FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor
                    ScrollBar.vertical: VclScrollBar {
                        flickable: quickFilesView
                    }
                    highlightMoveDuration: 0
                    interactive: height < contentHeight
                    delegate: scriptalayMode ? quickFilesScriptalayDelegate : quickFilesRecentFilesDelegate
                }
            }
        }
    }

    component ImportOptions : ColumnLayout {
        // Show restore and import options
        LinkButton {
            text: "Recover ..."
            tooltip: "Open cached files from your private on-disk vault."
            iconSource: "qrc:/icons/file/backup_open.png"
            Layout.fillWidth: true
            onClicked: stackView.push(vaultPage)
        }

        LinkButton {
            text: "Import ..."
            tooltip: "Import a screenplay from Final Draft, Fountain or HTML formats."
            iconSource: "qrc:/icons/file/import_export.png"
            Layout.fillWidth: true
            onClicked: stackView.push(importPage)
        }
    }

    component ScriptalayPage : Item {
        // Show contents of Scriptalay
        property bool hasSelection: screenplaysView.currentIndex >= 0

        function openSelected() {
            saveWorkflow.launch( () => {
                                    if(screenplaysView.currentIndex >= 0)
                                        libraryService.openScreenplayAt(screenplaysView.currentIndex)
                                } )
        }

        RowLayout {
            anchors.fill: parent
            spacing: 30

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: Qt.rgba(0,0,0,0)
                border.width: 1
                border.color: Runtime.colors.primary.borderColor

                ListView {
                    id: screenplaysView
                    anchors.fill: parent
                    anchors.margins: 1
                    clip: true
                    model: libraryService.screenplays
                    currentIndex: -1
                    FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor
                    highlight: Rectangle {
                        color: Runtime.colors.primary.highlight.background
                    }
                    ScrollBar.vertical: VclScrollBar {
                        flickable: screenplaysView
                    }
                    highlightMoveDuration: 0
                    highlightResizeDuration: 0
                    delegate: LinkButton {
                        required property int index
                        required property var record
                        width: screenplaysView.width
                        text: record.name
                        singleClick: false
                        iconSource: libraryService.screenplays.baseUrl + "/" + record.poster
                        onClicked: screenplaysView.currentIndex = index
                        onDoubleClicked: {
                            screenplaysView.currentIndex = index
                            Qt.callLater(openSelected)
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: Qt.rgba(0,0,0,0)
                border.width: 1
                border.color: Runtime.colors.primary.borderColor

                Flickable {
                    id: screenplayDetailsFlick
                    anchors.fill: parent
                    anchors.margins: 1
                    contentWidth: screenplayDetailsText.width
                    contentHeight: screenplayDetailsText.height
                    clip: true
                    flickableDirection: Flickable.VerticalFlick

                    ScrollBar.vertical: VclScrollBar {
                        flickable: screenplayDetailsFlick
                    }

                    TextArea {
                        id: screenplayDetailsText
                        width: screenplayDetailsFlick.width-20
                        property var record: screenplaysView.currentIndex >= 0 ? libraryService.screenplays.recordAt(screenplaysView.currentIndex) : undefined
                        textFormat: record ? TextArea.RichText : TextArea.MarkdownText
                        wrapMode: Text.WordWrap
                        padding: 8
                        readOnly: true
                        background: Item { }
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize
                        text: record ? composeTextFromRecord(record) : defaultText

                        onRecordChanged: {
                            if(record) {
                                commonToolTip.show(screenplaysView.currentItem, record.logline)
                                poster.show(screenplaysView.currentItem, screenplaysView.currentItem.iconSource, record.logline)
                            }
                        }
                        Component.onDestruction: {
                            commonToolTip.hide(screenplayDetailsText)
                            poster.hide(screenplaysView.currentItem)
                        }

                        function composeTextFromRecord(_record) {
                            var ret =
                              "<strong>Written By:</strong> " + _record.authors + "<br/><br/>" +
                              "<strong>Pages:</strong> " + _record.pageCount + "<br/>" +
                              "<strong>Revision:</strong> " + _record.revision + "<br/><br/>" +
                              "<strong>Copyright:</strong> " + _record.copyright + "<br/><br/>" +
                              "<strong>Source:</strong> " + _record.source
                            if(!banner.visible)
                                ret += "<br/><br/><strong>Logline:</strong> " + record._record
                            return ret
                        }

                        readonly property string defaultText: Scrite.app.fileContents(":/misc/scriptalay_info.md")
                    }
                }
            }
        }
    }

    component VaultPage : Rectangle {
        border.width: 1
        border.color: Runtime.colors.primary.borderColor
        color: Qt.rgba(0,0,0,0)

        function openSelected() {
            if(vaultFilesView.currentIndex < 0)
                return

            saveWorkflow.launch( () => {
                                    homeScreenBusyOverlay.visible = true
                                    homeScreen.enabled = false
                                    Runtime.loadMainUiContent = false
                                    Scrite.document.openAnonymously(vaultFilesView.currentItem.fileInfo.filePath)
                                    Runtime.loadMainUiContent = true
                                    closeRequest()
                                } )
        }

        function clearVault() {
            Scrite.vault.clearAllDocuments()
        }

        ListView {
            id: vaultFilesView
            anchors.fill: parent
            anchors.margins: 1
            clip: true
            model: Scrite.vault
            FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor
            currentIndex: count ? 0 : -1
            visible: count > 0
            ScrollBar.vertical: VclScrollBar { flickable: documentsView }
            highlight: Rectangle {
                color: Runtime.colors.primary.highlight.background
            }
            highlightMoveDuration: 0
            highlightResizeDuration: 0
            delegate: LinkButton {
                required property int index
                required property var fileInfo
                required property string timestampAsString
                required property string relativeTime
                width: vaultFilesView.width-1
                height: 60

                singleClick: false
                iconImage: fileInfo.hasCoverPage ? fileInfo.coverPageImage : null
                iconSource: fileInfo.hasCoverPage ? "" : "qrc:/icons/filetype/document.png"
                text: "<b>" + fileInfo.title + "</b> (" + fileInfo.sceneCount + (fileInfo.sceneCount === 1 ? " Scene" : " Scenes") + ")<br/>" +
                      "<font size=\"-1\">" + fileSizeInfo + ", " + relativeTime + " on " + timestampAsString + "</font>"
                property string fileSizeInfo: {
                    const fileSize = fileInfo.fileSize
                    if(fileSize < 1024)
                        return fileSize + " B"
                    if(fileSize < 1024*1024)
                        return Math.round(fileSize / 1024, 2) + " KB"
                    return Math.round(fileSize / (1024*1024), 2) + " MB"
                }

                onClicked: vaultFilesView.currentIndex = index
                onDoubleClicked: {
                    vaultFilesView.currentIndex = index
                    Qt.callLater( openSelected )
                }
            }
        }

        VclText {
            anchors.centerIn: parent
            width: parent.width * 0.8
            visible: Scrite.vault.documentCount === 0
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            text: "No documents found in vault."
        }
    }

    component ImportPage : Item {
        property bool hasActionButton: importPageStackLayout.currentIndex >= 1
        property string actionButtonText: importPageStackLayout.currentIndex === 1 ? "Browse" : "Import"
        function onActionButtonClicked() {
            if(importPageStackLayout.currentIndex === 1)
                dropBrowseItem.doBrowse()
            else if(importPageStackLayout.currentIndex === 2) {
                saveWorkflow.launch( () => {
                                        homeScreen.enabled = false
                                        importDroppedFileItem.doImport()
                                    } )
            }
        }

        QtObject {
            id: fileToImport

            property bool valid: path !== ""
            property string path
            property var info: Scrite.app.fileInfo(path)
            property string name: info.fileName
            property string folder: info.absolutePath
        }

        BasicAttachmentsDropArea {
            id: importDropArea
            anchors.fill: parent
            enabled: Runtime.appFeatures.importer.enabled
            allowedType: Attachments.NoMedia
            allowedExtensions: ["scrite", "fdx", "txt", "fountain", "html"]
            onDropped: fileToImport.path = attachment.filePath
        }

        StackLayout {
            id: importPageStackLayout
            anchors.fill: parent
            currentIndex: Runtime.appFeatures.importer.enabled ? (fileToImport.valid ? 2 : 1) : 0

            DisabledFeatureNotice {
                visible: !Runtime.appFeatures.importer.enabled
                color: Qt.rgba(1,1,1,0.9)
                featureName: "Import from 3rd Party Formats"
            }

            Rectangle {
                id: dropBrowseItem
                border.width: 1
                border.color: Runtime.colors.primary.borderColor
                color: Qt.rgba(0,0,0,0)

                function doBrowse() {
                    importFileDialog.open()
                }

                FileDialog {
                    id: importFileDialog
                    title: "Import Screenplay"
                    objectName: "Import Dialog Box"
                    nameFilters: ["*.scrite *.fdx *.fountain *.txt *.fountain *.html"]
                    selectFolder: false
                    selectMultiple: false
                    sidebarVisible: true
                    selectExisting: true
                    folder: Runtime.workspaceSettings.lastOpenImportFolderUrl
                    dirUpAction.shortcut: "Ctrl+Shift+U" // The default Ctrl+U interfers with underline
                    onFolderChanged: Runtime.workspaceSettings.lastOpenImportFolderUrl = folder

                    onAccepted: {
                        if(fileUrl != "")
                            fileToImport.path = Scrite.app.urlToLocalFile(fileUrl)
                    }
                }

                ColumnLayout {
                    width: parent.width-40
                    anchors.centerIn: parent
                    spacing: 20

                    VclText {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize+2
                        text: importDropArea.active ? importDropArea.attachment.originalFileName : "Drop a file on to this area to import it."
                    }

                    VclText {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize-1
                        color: Runtime.colors.primary.c700.background
                        text: importDropArea.active ? "Drop to import this file." : "(Allowed file types: " + importFileDialog.nameFilters.join(", ") + ")"
                    }
                }
            }

            Rectangle {
                id: importDroppedFileItem
                border.width: 1
                border.color: Runtime.colors.primary.borderColor
                color: Qt.rgba(0,0,0,0)

                function doImport() {
                    homeScreenBusyOverlay.busyMessage = "Importing screenplay ..."
                    homeScreenBusyOverlay.visible = true

                    Runtime.workspaceSettings.lastOpenImportFolderUrl = "file://" + fileToImport.folder

                    Runtime.loadMainUiContent = false
                    Scrite.document.openOrImport(fileToImport.path)
                    Runtime.loadMainUiContent = true

                    closeRequest()
                }

                ColumnLayout {
                    width: parent.width-40
                    anchors.centerIn: parent
                    spacing: 20

                    VclText {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize+2
                        font.bold: true
                        elide: Text.ElideMiddle
                        text: fileToImport.name
                    }

                    VclText {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize
                        text: "Click on 'Import' button to import this file."
                    }

                    VclText {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize-2
                        color: Runtime.colors.accent.c700.background
                        text: "<b>NOTE</b>: Unsaved changes in the current document will be discarded."
                        visible: !Scrite.document.empty
                    }
                }
            }
        }
    }

    component StackPage : Item {
        id: stackPage

        property Component content
        property QtObject contentItem: contentLoader.item

        property Component title
        property QtObject titleItem: titleLoader.item

        property Component buttons
        property QtObject buttonsItem: buttonsLoader.item

        Item {
            anchors.fill: parent
            anchors.topMargin: 25
            anchors.leftMargin: 50
            anchors.rightMargin: 50
            anchors.bottomMargin: 25

            Loader {
                id: contentLoader
                width: parent.width
                anchors.top: parent.top
                anchors.bottom: buttonsRow.top
                anchors.bottomMargin: 20
                sourceComponent: stackPage.content
            }

            RowLayout {
                id: buttonsRow
                width: parent.width
                anchors.bottom: parent.bottom

                VclButton {
                    text: "< Back"
                    onClicked: stackView.pop()

                    EventFilter.target: Scrite.app
                    EventFilter.events: [EventFilter.KeyPress,EventFilter.KeyRelease,EventFilter.Shortcut]
                    EventFilter.onFilter: (watched,event,result) => {
                                              if(event.key === Qt.Key_Escape) {
                                                  result.acceptEvent = true
                                                  result.filter = true
                                                  stackView.pop()
                                              }
                                          }
                }

                Loader {
                    id: titleLoader
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    sourceComponent: stackPage.title
                }

                Loader {
                    id: buttonsLoader
                    sourceComponent: stackPage.buttons
                }
            }
        }
    }

    Component {
        id: scriptalayPage

        StackPage {
            id: scriptalayPageItem
            readonly property string bannerImage: "qrc:/images/homescreen_scriptalay_banner.png"
            content: ScriptalayPage { }
            title: Item {
                Image {
                    anchors.centerIn: parent
                    source: "qrc:/images/library.png"
                    height: 36
                    fillMode: Image.PreserveAspectFit
                }
            }
            buttons: VclButton {
                text: "Open"
                enabled: scriptalayPageItem.contentItem.hasSelection && libraryService.screenplays.count > 0
                onClicked: scriptalayPageItem.contentItem.openSelected()
            }
        }
    }

    Component {
        id: vaultPage

        StackPage {
            id: vaultPageItem
            content: VaultPage { }
            title: VclText {
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                text: Scrite.vault.documentCount > 0 ? "Select a file to restore from the vault." : ""
                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                elide: Text.ElideRight
            }
            buttons: RowLayout {
                spacing: 10

                VclButton {
                    text: "Open"
                    onClicked: vaultPageItem.contentItem.openSelected()
                    enabled: Scrite.vault.documentCount > 0
                }

                VclButton {
                    text: "Clear"
                    enabled: Scrite.vault.documentCount > 0
                    onClicked: vaultPageItem.contentItem.clearVault()
                }
            }
        }
    }

    Component {
        id: importPage

        StackPage {
            id: importPageItem
            content: ImportPage { }
            title: Item { }
            buttons: RowLayout {
                VclButton {
                    visible: importPageItem.contentItem.hasActionButton
                    text: importPageItem.contentItem.actionButtonText
                    onClicked: importPageItem.contentItem.onActionButtonClicked()
                }
            }
        }
    }

    FileDialog {
        id: openFileDialog

        title: "Open Scrite Document"
        nameFilters: ["Scrite Documents (*.scrite)"]
        selectFolder: false
        selectMultiple: false
        objectName: "Open File Dialog"
        dirUpAction.shortcut: "Ctrl+Shift+U"
        folder: Runtime.workspaceSettings.lastOpenFolderUrl
        onFolderChanged: Runtime.workspaceSettings.lastOpenFolderUrl = folder
        sidebarVisible: true
        selectExisting: true

        onAccepted: {
            Runtime.workspaceSettings.lastOpenFolderUrl = folder

            const path = Scrite.app.urlToLocalFile(fileUrl)
            _private.openScriteDocument(path)
        }
    }

    QtObject {
        id: _private

        readonly property string defaultBannerImage: "qrc:/images/homescreen_banner.png"

        function openScriteDocument(path) {
            homeScreenBusyOverlay.busyMessage = "Opening document ..."
            homeScreenBusyOverlay.visible = true

            homeScreen.enabled = false

            Runtime.loadMainUiContent = false

            Utils.execLater(_private, 100, () => {
                                Runtime.recentFiles.add(path)
                                Scrite.document.open(path)
                                Runtime.loadMainUiContent = true

                                closeRequest()
                            })
        }
    }
}
