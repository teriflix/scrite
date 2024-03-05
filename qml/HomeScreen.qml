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

import "../js/utils.js" as Utils

import "./globals"

Item {
    id: homeScreen
    width: Math.min(800, scriteDocumentViewItem.height*0.9)
    height: banner.height * 2.2

    Image {
        id: banner
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        source: "../images/banner.png"
        fillMode: Image.PreserveAspectFit

        Image {
            anchors.centerIn: parent
            source: "../images/banner_logo_overlay.png"
            width: homeScreen.width * 0.2
            fillMode: Image.PreserveAspectFit
            smooth: true; mipmap: true
            visible: false
            Component.onCompleted: Utils.execLater(banner, 50, () => { visible = true } )
        }

        Text {
            id: appVersionLabel
            property real ratio: parent.height / parent.sourceSize.height
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.rightMargin: 30 * ratio
            anchors.bottomMargin: 10 * ratio
            font.pointSize: ScriteFontMetrics.ideal.font.pointSize-2
            text: Scrite.app.applicationVersion
            color: "white"
        }

        Text {
            id: commonToolTip
            width: parent.width * 0.75
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.leftMargin: 30 * appVersionLabel.ratio
            anchors.bottomMargin: 10 * appVersionLabel.ratio

            font.pointSize: ScriteFontMetrics.ideal.font.pointSize-2
            padding: 5
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

        Text {
            anchors.left: commonToolTip.left
            anchors.bottom: commonToolTip.bottom
            padding: commonToolTip.padding
            visible: !commonToolTip.visible
            text: "scrite.io"
            color: commonToolTip.color
            font.pointSize: commonToolTip.font.pointSize
        }
    }

    StackView {
        id: stackView
        anchors.top: banner.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        onDepthChanged: modalDialog.closeable = depth === 1

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
                             mainUiContentLoader.allowContent = false
                         }

        onImportFinished: (index) => {
                              mainUiContentLoader.allowContent = true
                              Utils.execLater(libraryService, 250, function() {
                                  modalDialog.close()
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
        }
    }

    component LinkButton : Rectangle {
        id: button
        property string text
        property string tooltip
        property string iconSource
        property var iconImage // has to be QImage
        property bool singleClick: true

        signal clicked()
        signal doubleClicked()

        width: 100
        height: buttonLayout.height + 6
        color: buttonMouseArea.containsMouse ? ScritePrimaryColors.highlight.background : Qt.rgba(0,0,0,0)

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

            Text {
                id: buttonLabel
                padding: 3
                font.pointSize: ScriteFontMetrics.ideal.font.pointSize
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
            onEntered: commonToolTip.show(parent, parent.tooltip)
            onExited: commonToolTip.hide(parent)
        }
    }

    component NewFileOptions : Item {
        ColumnLayout {
            anchors.fill: parent

            Text {
                font.pointSize: ScriteFontMetrics.ideal.font.pointSize
                text: "New File"
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                // Layout.leftMargin: 20
                Layout.rightMargin: 20
                color: Qt.rgba(0,0,0,0)
                border.width: templatesView.interactive ? 1 : 0
                border.color: ScritePrimaryColors.borderColor

                AppFeature {
                    id: templatesFeatureCheck
                    feature: Scrite.TemplateFeature
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 1
                    visible: !templatesFeatureCheck.enabled

                    LinkButton {
                        text: "Blank Document"
                        iconSource: "../icons/filetype/document.png"
                        Layout.fillWidth: true
                        tooltip: "Creates a new blank Scrite document."
                        onClicked: {
                            saveWorkflow.launch( () => {
                                                    homeScreen.enabled = false
                                                    Scrite.document.reset()
                                                    modalDialog.close()
                                                } )
                        }
                    }

                    DisabledFeatureNotice {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        color: Qt.rgba(1,1,1,0.9)
                        featureName: "Screenplay Templates"
                        visible: !templatesFeatureCheck.enabled
                    }
                }

                ListView {
                    id: templatesView
                    anchors.fill: parent
                    anchors.margins: 1
                    model: templatesFeatureCheck.enabled ? libraryService.templates : []
                    visible: templatesFeatureCheck.enabled
                    currentIndex: -1
                    clip: true
                    FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor
                    ScrollBar.vertical: ScrollBar2 {
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
            iconSource: "../icons/file/folder_open.png"
            Layout.fillWidth: true
            tooltip: "Launches a file dialog box so you can select a .scrite file to load from disk."
            onClicked: {
                saveWorkflow.launch( () => {
                                        openFileDialog.open()
                                    } )
            }
        }

        LinkButton {
            text: recentFilesModel.count === 0 ? "Recent files ..." : "Scriptalay"
            iconSource: recentFilesModel.count === 0 ? "../icons/filetype/document.png" : "../icons/action/library.png"
            Layout.fillWidth: true
            tooltip: recentFilesModel.count === 0 ? "Reopen a recently opened file." : "Download a screenplay from our online-library of screenplays."
            onClicked: stackView.push(scriptalayPage)
            enabled: recentFilesModel.count > 0

            Announcement.onIncoming: (type,data) => {
                                         if(type === "710A08E7-9F60-4D36-9DEA-0993EEBA7DCA") {
                                             if(data === "Scriptalay" && enabled)
                                                stackView.push(scriptalayPage)
                                         }
                                     }
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
            iconSource: fileInfo.hasCoverPage ? "" : "../icons/filetype/document.png"
            iconImage: fileInfo.hasCoverPage ? fileInfo.coverPageImage : null
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
        property bool scriptalayMode: recentFilesModel.count === 0

        ColumnLayout {
            anchors.fill: parent

            Text {
                font.pointSize: ScriteFontMetrics.ideal.font.pointSize
                text: scriptalayMode ? "Scriptalay" : "Recent Files"
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                // Layout.leftMargin: 20
                Layout.rightMargin: 20
                color: Qt.rgba(0,0,0,0)
                border.width: quickFilesView.interactive ? 1 : 0
                border.color: ScritePrimaryColors.borderColor

                ListView {
                    id: quickFilesView // shows either Scriptalay or Recent Files
                    anchors.fill: parent
                    anchors.margins: 1
                    model: scriptalayMode ? libraryService.screenplays : recentFilesModel
                    currentIndex: -1
                    clip: true
                    FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor
                    ScrollBar.vertical: ScrollBar2 {
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
            iconSource: "../icons/file/backup_open.png"
            Layout.fillWidth: true
            onClicked: stackView.push(vaultPage)
        }

        LinkButton {
            text: "Import ..."
            tooltip: "Import a screenplay from Final Draft, Fountain or HTML formats."
            iconSource: "../icons/file/import_export.png"
            Layout.fillWidth: true
            onClicked: stackView.push(importPage)
        }
    }

    component ScriptalayPage : Item {
        // Show contents of Scriptalay
        function openSelected() {
            saveWorkflow.launch( () => {
                                    if(screenplaysView.currentIndex >= 0)
                                        libraryService.openScreenplayAt(screenplaysView.currentIndex)
                                } )
        }

        RowLayout {
            anchors.fill: parent
            spacing: 10

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: Qt.rgba(0,0,0,0)
                border.width: 1
                border.color: ScritePrimaryColors.borderColor

                ListView {
                    id: screenplaysView
                    anchors.fill: parent
                    anchors.margins: 1
                    clip: true
                    model: libraryService.screenplays
                    currentIndex: count ? 0 : -1
                    FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor
                    highlight: Rectangle {
                        color: ScritePrimaryColors.highlight.background
                    }
                    ScrollBar.vertical: ScrollBar2 {
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
                border.color: ScritePrimaryColors.borderColor

                Flickable {
                    id: screenplayDetailsFlick
                    anchors.fill: parent
                    anchors.margins: 1
                    contentWidth: screenplayDetailsText.width
                    contentHeight: screenplayDetailsText.height
                    visible: screenplaysView.currentIndex >= 0
                    clip: true

                    ScrollBar.vertical: ScrollBar2 {
                        flickable: screenplayDetailsFlick
                    }

                    TextArea {
                        id: screenplayDetailsText
                        width: screenplayDetailsFlick.width-20
                        property var record: libraryService.screenplays.recordAt(screenplaysView.currentIndex)
                        textFormat: TextArea.RichText
                        wrapMode: Text.WordWrap
                        padding: 4
                        readOnly: true
                        text: "<strong>Authors:</strong> " + record.authors + "<br/><br/>" +
                              "<strong>Pages:</strong> " + record.pageCount + "<br/><br/>" +
                              "<strong>Revision:</strong> " + record.revision + "<br/><br/>" +
                              "<strong>Copyright:</strong> " + record.copyright + "<br/><br/>" +
                              "<strong>Source:</strong> " + record.source + "<br/><br/>" +
                              "<strong>Logline:</strong> " + record.logline + "<br/><br/>"
                    }
                }
            }
        }
    }

    component VaultPage : Rectangle {
        border.width: 1
        border.color: ScritePrimaryColors.borderColor
        color: Qt.rgba(0,0,0,0)

        function openSelected() {
            if(vaultFilesView.currentIndex < 0)
                return

            saveWorkflow.launch( () => {
                                    homeScreenBusyOverlay.visible = true
                                    homeScreen.enabled = false
                                    mainUiContentLoader.allowContent = false
                                    Scrite.document.openAnonymously(vaultFilesView.currentItem.fileInfo.filePath)
                                    mainUiContentLoader.allowContent = true
                                    modalDialog.close()
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
            FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor
            currentIndex: count ? 0 : -1
            visible: count > 0
            ScrollBar.vertical: ScrollBar2 { flickable: documentsView }
            highlight: Rectangle {
                color: ScritePrimaryColors.highlight.background
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
                iconSource: fileInfo.hasCoverPage ? "" : "../icons/filetype/document.png"
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

        Text {
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

        AppFeature {
            id: importFeatureCheck
            feature: Scrite.ImportFeature
        }

        AttachmentsDropArea {
            id: importDropArea
            anchors.fill: parent
            enabled: importFeatureCheck.enabled
            allowedType: Attachments.NoMedia
            allowedExtensions: ["scrite", "fdx", "txt", "fountain", "html"]
            onDropped: fileToImport.path = attachment.filePath
        }

        StackLayout {
            id: importPageStackLayout
            anchors.fill: parent
            currentIndex: importFeatureCheck.enabled ? (fileToImport.valid ? 2 : 1) : 0

            DisabledFeatureNotice {
                visible: !importFeatureCheck.enabled
                color: Qt.rgba(1,1,1,0.9)
                featureName: "Import from 3rd Party Formats"
            }

            Rectangle {
                id: dropBrowseItem
                border.width: 1
                border.color: ScritePrimaryColors.borderColor
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
                    folder: workspaceSettings.lastOpenImportFolderUrl
                    dirUpAction.shortcut: "Ctrl+Shift+U" // The default Ctrl+U interfers with underline
                    onFolderChanged: workspaceSettings.lastOpenImportFolderUrl = folder

                    onAccepted: {
                        if(fileUrl != "")
                            fileToImport.path = Scrite.app.urlToLocalFile(fileUrl)
                    }
                }

                ColumnLayout {
                    width: parent.width-40
                    anchors.centerIn: parent
                    spacing: 20

                    Text {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pointSize: ScriteFontMetrics.ideal.font.pointSize+2
                        text: importDropArea.active ? importDropArea.attachment.originalFileName : "Drop a file on to this area to import it."
                    }

                    Text {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pointSize: ScriteFontMetrics.ideal.font.pointSize-1
                        color: ScritePrimaryColors.c700.background
                        text: importDropArea.active ? "Drop to import this file." : "(Allowed file types: " + importFileDialog.nameFilters.join(", ") + ")"
                    }
                }
            }

            Rectangle {
                id: importDroppedFileItem
                border.width: 1
                border.color: ScritePrimaryColors.borderColor
                color: Qt.rgba(0,0,0,0)

                function doImport() {
                    homeScreenBusyOverlay.busyMessage = "Importing screenplay ..."
                    homeScreenBusyOverlay.visible = true

                    workspaceSettings.lastOpenImportFolderUrl = "file://" + fileToImport.folder

                    mainUiContentLoader.allowContent = false
                    Scrite.document.openOrImport(fileToImport.path)
                    mainUiContentLoader.allowContent = true

                    modalDialog.close()
                }

                ColumnLayout {
                    width: parent.width-40
                    anchors.centerIn: parent
                    spacing: 20

                    Text {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pointSize: ScriteFontMetrics.ideal.font.pointSize+2
                        font.bold: true
                        elide: Text.ElideMiddle
                        text: fileToImport.name
                    }

                    Text {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pointSize: ScriteFontMetrics.ideal.font.pointSize
                        text: "Click on 'Import' button to import this file."
                    }

                    Text {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pointSize: ScriteFontMetrics.ideal.font.pointSize-2
                        color: ScriteAccentColors.c700.background
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
        property alias contentItem: contentLoader.item

        property Component title
        property alias titleItem: titleLoader.item

        property Component buttons
        property alias buttonsItem: buttonsLoader.item

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

                Button2 {
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
                    sourceComponent: title
                }

                Loader {
                    id: buttonsLoader
                    sourceComponent: buttons
                }
            }
        }
    }

    Component {
        id: scriptalayPage

        StackPage {
            id: scriptalayPageItem
            content: ScriptalayPage { }
            title: Item {
                Image {
                    anchors.centerIn: parent
                    source: "../images/library.png"
                    height: 36
                    fillMode: Image.PreserveAspectFit
                }
            }
            buttons: Button2 {
                text: "Open"
                enabled: libraryService.screenplays.count > 0
                onClicked: scriptalayPageItem.contentItem.openSelected()
            }
        }
    }

    Component {
        id: vaultPage

        StackPage {
            id: vaultPageItem
            content: VaultPage { }
            title: Text {
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                text: Scrite.vault.documentCount > 0 ? "Select a file to restore from the vault." : ""
                font.pointSize: ScriteFontMetrics.ideal.font.pointSize
                elide: Text.ElideRight
            }
            buttons: RowLayout {
                spacing: 10

                Button2 {
                    text: "Open"
                    onClicked: vaultPageItem.contentItem.openSelected()
                    enabled: Scrite.vault.documentCount > 0
                }

                Button2 {
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
                Button2 {
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
        folder: workspaceSettings.lastOpenFolderUrl
        onFolderChanged: workspaceSettings.lastOpenFolderUrl = folder
        sidebarVisible: true
        selectExisting: true

        onAccepted: {
            workspaceSettings.lastOpenFolderUrl = folder

            const path = Scrite.app.urlToLocalFile(fileUrl)
            _private.openScriteDocument(path)
        }
    }

    QtObject {
        id: _private

        function openScriteDocument(path) {
            homeScreenBusyOverlay.busyMessage = "Opening document ..."
            homeScreenBusyOverlay.visible = true

            homeScreen.enabled = false

            mainUiContentLoader.allowContent = false
            recentFilesModel.add(path)
            Scrite.document.open(path)
            mainUiContentLoader.allowContent = true

            modalDialog.close()
        }
    }
}
