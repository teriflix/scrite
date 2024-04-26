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

import "qrc:/qml/tasks"
import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"

Item {
    id: root

    property string mode
    signal closeRequest()

    Component.onCompleted: {
        Utils.execLater(root, 500, () => {
                            if(mode === "Scriptalay")
                                _private.showScriptalay()
                        })
    }

    LibraryService {
        id: libraryService
    }

    Loader {
        anchors.fill: parent
        sourceComponent: {
            switch(_private.layoutType) {
            case 1: return homeScreenLayout1
            case 2: return homeScreenLayout2
            default:
            }
            return homeScreenLayout2
        }
    }

    // Use this if the scaled banner height is less than 60% of the home screen height
    Component {
        id: homeScreenLayout1

        ColumnLayout {
            spacing: 0

            TopBanner {
                Layout.fillWidth: true
                Layout.preferredHeight: (_private.bannerSize.height / _private.bannerSize.width) * width
            }

            StackView {
                Layout.fillWidth: true
                Layout.fillHeight: true

                clip: true

                initialItem: ContentPageLayout1 { }

                function onShowScriptalay() {
                    push(scriptalayPage)
                }

                Component.onCompleted: _private.showScriptalay.connect(onShowScriptalay)
            }
        }
    }

    // Use this otherwise
    Component {
        id: homeScreenLayout2

        StackView {
            clip: true

            initialItem: ContentPageLayout2 { }
        }
    }

    component TopBanner : Image {
        id: banner

        readonly property StackView stackView: Aggregation.firstSibling("QQuickStackView")

        source: {
            if(stackView.currentItem && stackView.currentItem.bannerImage)
                return stackView.currentItem.bannerImage
            return _private.defaultBannerImage
        }
        fillMode: Image.PreserveAspectFit

        Image {
            anchors.centerIn: parent
            source: (banner.stackView.currentItem && banner.stackView.currentItem.bannerImage) ? "" : "qrc:/images/banner_logo_overlay.png"
            width: root.width * 0.3
            fillMode: Image.PreserveAspectFit
            smooth: true; mipmap: true
            visible: false
            Component.onCompleted: Utils.execLater(banner, 50, () => { visible = true } )
        }

        RowLayout {
            spacing: 20

            anchors.bottom: parent.bottom
            anchors.bottomMargin: 20
            anchors.horizontalCenter: parent.horizontalCenter
            visible: !topBannerToolTip.visible

            LinkButton {
                Layout.fillWidth: true

                text: "Learning Guides"
                color: Qt.rgba(0,0,0,0)
                textColor: "white"
                iconSource: "qrc:/icons/action/help_inverted.png"

                onClicked: Qt.openUrlExternally("https://www.scrite.io/index.php/help/")
            }

            LinkButton {
                Layout.fillWidth: true

                text: "Discord Community"
                color: Qt.rgba(0,0,0,0)
                textColor: "white"
                iconSource: "qrc:/icons/action/forum_inverted.png"

                onClicked: Qt.openUrlExternally("https://www.scrite.io/index.php/forum/")
            }
        }

        VclLabel {
            id: topBannerToolTip

            width: parent.width * 0.75

            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.leftMargin: 20
            anchors.bottomMargin: 20

            color: "white"
            elide: Text.ElideRight
            padding: 8
            visible: text !== ""
            wrapMode: Text.WordWrap
            font.pointSize: Runtime.idealFontMetrics.font.pointSize
            maximumLineCount: 4
            verticalAlignment: Text.AlignBottom
            horizontalAlignment: Text.AlignLeft

            property Item source

            Connections {
                target: _private

                function onShowTooltipRequest(_source, _text) {
                    topBannerToolTip.source = _source
                    topBannerToolTip.text = _text
                }

                function onHideTooltipRequest(_source) {
                    if(topBannerToolTip.source === _source) {
                        topBannerToolTip.source = null
                        topBannerToolTip.text = ""
                    }
                }
            }
        }

        Poster {
            id: topBannerPoster

            anchors.fill: parent

            property Item sourceItem

            Connections {
                target: _private

                function onShowPosterRequest(_source, _image, _logline) {
                    topBannerPoster.sourceItem = _source
                    topBannerPoster.source = _image
                    topBannerPoster.logline = _logline
                }

                function onHidePosterRequest(_source) {
                    if(topBannerPoster.sourceItem === _source) {
                        topBannerPoster.sourceItem = null
                        topBannerPoster.source = undefined
                        topBannerPoster.logline = ""
                    }
                }
            }
        }
    }

    component SideBanner : ColumnLayout {
        spacing: 20

        Image {
            Layout.fillWidth: true
            Layout.preferredHeight: (_private.bannerSize.height / _private.bannerSize.width) * width

            source: _private.defaultBannerImage
            fillMode: Image.PreserveAspectFit

            Image {
                readonly property StackView stackView: Aggregation.firstParent("QQuickStackView")

                anchors.centerIn: parent
                source: (stackView.currentItem && stackView.currentItem.bannerImage) ? "" : "qrc:/images/banner_logo_overlay.png"
                width: parent.width * 0.3
                fillMode: Image.PreserveAspectFit
                smooth: true; mipmap: true
                visible: false
                Component.onCompleted: Utils.execLater(parent, 50, () => { visible = true } )
            }

            Poster {
                id: sideBannerPoster

                anchors.fill: parent

                property Item sourceItem

                Connections {
                    target: _private

                    function onShowPosterRequest(_source, _image, _logline) {
                        sideBannerPoster.sourceItem = _source
                        sideBannerPoster.source = _image
                    }

                    function onHidePosterRequest(_source) {
                        if(sideBannerPoster.sourceItem === _source) {
                            sideBannerPoster.sourceItem = null
                            sideBannerPoster.source = undefined
                        }
                    }
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 10

                VclLabel {
                    id: sideBannerTooltip

                    Layout.fillWidth: true
                    Layout.fillHeight: source ? true : false

                    text: defaultText
                    padding: 8

                    font.pointSize: Runtime.idealFontMetrics.font.pointSize

                    elide: Text.ElideRight
                    wrapMode: Text.WordWrap

                    property Item source

                    Connections {
                        target: _private

                        function onShowTooltipRequest(_source, _text) {
                            sideBannerTooltip.source = _source
                            sideBannerTooltip.text = _text
                            sideBannerHelpButtons.visible = false
                        }

                        function onHideTooltipRequest(_source) {
                            if(sideBannerTooltip.source === _source) {
                                sideBannerTooltip.source = null
                                sideBannerTooltip.text = sideBannerTooltip.defaultText
                                sideBannerHelpButtons.visible = true
                            }
                        }
                    }

                    readonly property string defaultText: Scrite.app.fileContents(":/misc/homescreen_info.md")
                }

                ColumnLayout {
                    id: sideBannerHelpButtons
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    LinkButton {
                        Layout.fillWidth: true

                        text: "Learning Guides"
                        iconSource: "qrc:/icons/action/help.png"

                        onClicked: Qt.openUrlExternally("https://www.scrite.io/index.php/help/")
                    }

                    LinkButton {
                        Layout.fillWidth: true

                        text: "Discord Community"
                        iconSource: "qrc:/icons/action/forum.png"

                        onClicked: Qt.openUrlExternally("https://www.scrite.io/index.php/forum/")
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                }
            }
        }
    }

    component ContentPageLayout1 : Item {
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

    component ContentPageLayout2 : Item {
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
                    spacing: 20

                    NewFileOptions {
                        Layout.fillWidth: true
                    }

                    QuickFileOpenOptions {
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
                    spacing: 20

                    SideBanner {
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
        property color  textColor: Runtime.colors.primary.regular.text
        property var    iconImage: Scrite.app.emptyQImage // has to be QImage
        property bool   singleClick: true
        property bool   showPoster: false
        property bool   containsMouse: buttonMouseArea.containsMouse

        signal clicked()
        signal doubleClicked()

        width: 100
        height: buttonLayout.height + 6
        color: buttonMouseArea.containsMouse ? Runtime.colors.primary.highlight.background : Qt.rgba(0,0,0,0)

        implicitWidth: buttonIcon.implicitWidth + buttonLabel.implicitWidth + buttonLayout.spacing
        implicitHeight: buttonLayout.height + 10

        RowLayout {
            id: buttonLayout
            width: parent.width - 10
            anchors.centerIn: parent
            spacing: 5
            opacity: enabled ? 1 : 0.6

            Loader {
                id: buttonIcon

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
                        item.image = Qt.binding( () => { return iconImage ? iconImage : Scrite.app.emptyQImage } )
                }
            }

            VclLabel {
                id: buttonLabel

                Layout.fillWidth: true

                text: button.text
                color: button.textColor
                elide: Text.ElideRight
                padding: 3
                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                font.underline: singleClick ? buttonMouseArea.containsMouse : false
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
                    if(parent.tooltip !== "")
                        _private.showTooltipRequest(parent, parent.tooltip)

                    if(parent.showPoster) {
                        if(parent.iconSource !== "")
                            _private.showPosterRequest(parent, parent.iconSource, parent.tooltip)
                        else
                            _private.showPosterRequest(parent, parent.iconImage, parent.tooltip)
                    }
                }
            }
            onExited: {
                if(hoverEnabled) {
                    _private.hideTooltipRequest(parent)
                    _private.hidePosterRequest(parent)
                }
            }
        }
    }

    component NewFileOptions : Item {
        implicitHeight: Math.max(newFileLabel.height + templatesView.contentHeight, 220)

        ColumnLayout {
            anchors.fill: parent

            VclLabel {
                id: newFileLabel
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
                            SaveFileTask.save( () => {
                                                    root.enabled = false
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
                            SaveFileTask.save( () => {
                                                    var task = OpenFromLibraryTask.openTemplateAt(libraryService, index)
                                                    task.finished.connect(closeRequest)
                                                } )
                        }
                    }
                }

                BusyIndicator {
                    anchors.centerIn: parent
                    running: libraryService.busy
                }
            }
        }
    }

    component OpenFileOptions : ColumnLayout {
        readonly property StackView stackView: Aggregation.firstParent("QQuickStackView")

        LinkButton {
            text: "Open ..."
            iconSource: "qrc:/icons/file/folder_open.png"
            Layout.fillWidth: true
            tooltip: "Launches a file dialog box so you can select a .scrite file to load from disk."
            onClicked: {
                SaveFileTask.save( () => {
                                        openFileDialog.open()
                                    } )
            }
        }

        LinkButton {
            text: Runtime.recentFiles.count === 0 ? "Recent files ..." : "Scriptalay"
            iconSource: Runtime.recentFiles.count === 0 ? "qrc:/icons/filetype/document.png" : "qrc:/icons/action/library.png"
            Layout.fillWidth: true
            tooltip: Runtime.recentFiles.count === 0 ? "Reopen a recently opened file." : "Download a screenplay from our online library."
            onClicked: parent.stackView.push(scriptalayPage)
            enabled: Runtime.recentFiles.count > 0
        }
    }

    Component {
        id: quickFilesRecentFilesDelegate

        LinkButton {
            required property int index
            required property var fileInfo

            ToolTip.text: fileInfo.filePath
            ToolTip.visible: containsMouse && _private.layoutType === 1

            width: ListView.view.width

            text: fileInfo.title === "" ? fileInfo.baseFileName : composeTextFromTitleAndVersion(fileInfo)
            tooltip: {
                let ret = fileInfo.logline
                if(_private.layoutType === 2) {
                    const fp = "<font size=\"-1\">" + fileInfo.filePath + "</font>"
                    if(ret === "")
                        ret = fp
                    else
                        ret += "<br/><br/>" + fp
                }
                return ret
            }
            iconSource: fileInfo.hasCoverPage ? "" : "qrc:/icons/filetype/document.png"
            iconImage: fileInfo.hasCoverPage ? fileInfo.coverPageImage : null
            showPoster: fileInfo.hasCoverPage
            onClicked: {
                if(fileInfo.filePath === Scrite.document.fileName)
                    closeRequest()
                else
                    SaveFileTask.save( () => {
                                            var task = OpenFileTask.open(fileInfo.filePath)
                                            task.finished.connect(closeRequest)
                                        } )
            }

            function composeTextFromTitleAndVersion(fi) {
                return fi.version === "" ? fi.title : (fi.title + " <font size=\"-1\">(" + fi.version + ")</font>")
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
                SaveFileTask.save( () => {
                                        var task = OpenFromLibraryTask.openScreenplayAt(libraryService, index)
                                        task.finished.connect(closeRequest)
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

            VclLabel {
                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                text: scriptalayMode ? "Scriptalay" : "Recent Files"
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                // Layout.leftMargin: 20
                // Layout.rightMargin: 20
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
        readonly property StackView stackView: Aggregation.firstParent("QQuickStackView")

        // Show restore and import options
        LinkButton {
            text: "Recover ..."
            tooltip: "Open cached files from your private on-disk vault."
            iconSource: "qrc:/icons/file/backup_open.png"
            Layout.fillWidth: true
            onClicked: parent.stackView.push(vaultPage)
        }

        LinkButton {
            text: "Import ..."
            tooltip: "Import a screenplay from Final Draft, Fountain or HTML formats."
            iconSource: "qrc:/icons/file/import_export.png"
            Layout.fillWidth: true
            onClicked: parent.stackView.push(importPage)
        }
    }

    component ScriptalayPage : Item {
        // Show contents of Scriptalay
        property bool hasSelection: screenplaysView.currentIndex >= 0

        function openSelected() {
            SaveFileTask.save( () => {
                                    if(screenplaysView.currentIndex >= 0) {
                                         var task = OpenFromLibraryTask.openScreenplayAt(libraryService, screenplaysView.currentIndex)
                                         task.finished.connect(closeRequest)
                                     }
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

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 20

                    Image {
                        readonly property StackView stackView: Aggregation.firstParent("QQuickStackView")

                        Layout.fillWidth: true
                        Layout.preferredHeight: (_private.bannerSize.height / _private.bannerSize.width) * width

                        visible: _private.layoutType === 2
                        source: stackView.currentItem.bannerImage
                        fillMode: Image.PreserveAspectFit

                        Poster {
                            id: scriptalayPoster

                            anchors.fill: parent

                            property Item sourceItem

                            Connections {
                                target: _private

                                function onShowPosterRequest(_source, _image, _logline) {
                                    scriptalayPoster.sourceItem = _source
                                    scriptalayPoster.source = _image
                                }

                                function onHidePosterRequest(_source) {
                                    if(scriptalayPoster.sourceItem === _source) {
                                        scriptalayPoster.sourceItem = null
                                        scriptalayPoster.source = undefined
                                    }
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
                                        _private.showTooltipRequest(screenplaysView.currentItem, record.logline)
                                        _private.showPosterRequest(screenplaysView.currentItem, screenplaysView.currentItem.iconSource, record.logline)
                                    }
                                }
                                Component.onDestruction: {
                                    _private.hideTooltipRequest(screenplayDetailsText)
                                    _private.hidePosterRequest(screenplaysView.currentItem)
                                }

                                function composeTextFromRecord(_record) {
                                    var ret =
                                            "<strong>Written By:</strong> " + _record.authors + "<br/><br/>" +
                                            "<strong>Pages:</strong> " + _record.pageCount + "<br/>" +
                                            "<strong>Revision:</strong> " + _record.revision + "<br/><br/>" +
                                            "<strong>Copyright:</strong> " + _record.copyright + "<br/><br/>" +
                                            "<strong>Source:</strong> " + _record.source
                                    if(_private.layoutType === 2)
                                        ret += "<br/><br/><strong>Logline:</strong> " + _record.logline
                                    return ret
                                }

                                readonly property string defaultText: Scrite.app.fileContents(":/misc/scriptalay_info.md")
                            }
                        }
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

            SaveFileTask.save( () => {
                                    root.enabled = false

                                    var task = OpenFileTask.openAnonymously(vaultFilesView.currentItem.fileInfo.filePath)
                                    task.finished.connect(closeRequest)
                                } )
        }

        function clearVault() {
            if(Scrite.vault.documentCount === 0)
                return

            MessageBox.question("Clear Confirmation",
                                "Are you sure you want to purge all documents in your vault?",
                                ["Yes", "No"],
                                (answer) => {
                                    if(answer === "Yes")
                                        Scrite.vault.clearAllDocuments()
                                })
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

        VclLabel {
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
                SaveFileTask.save( () => {
                                        root.enabled = false
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
                    nameFilters: ["*.scrite *.fdx *.fountain *.txt *.html"]
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

                    VclLabel {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize+2
                        text: importDropArea.active ? importDropArea.attachment.originalFileName : "Drop a file on to this area to import it."
                    }

                    VclLabel {
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
                    Runtime.workspaceSettings.lastOpenImportFolderUrl = "file://" + fileToImport.folder

                    var task = OpenFileTask.openOrImport(fileToImport.path)
                    task.finished.connect(closeRequest)
                }

                ColumnLayout {
                    width: parent.width-40
                    anchors.centerIn: parent
                    spacing: 20

                    VclLabel {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize+2
                        font.bold: true
                        elide: Text.ElideMiddle
                        text: fileToImport.name
                    }

                    VclLabel {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize
                        text: "Click on 'Import' button to import this file."
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

        readonly property StackView stackView: Aggregation.firstParent("QQuickStackView")

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
                    onClicked: stackPage.stackView.pop()

                    EventFilter.target: Scrite.app
                    EventFilter.events: [EventFilter.KeyPress,EventFilter.KeyRelease,EventFilter.Shortcut]
                    EventFilter.onFilter: (watched,event,result) => {
                                              if(event.key === Qt.Key_Escape) {
                                                  result.acceptEvent = true
                                                  result.filter = true
                                                  stackPage.stackView.pop()
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
            title: VclLabel {
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

            var task = OpenFileTask.open(path)
            task.finished.connect(closeRequest)
        }
    }

    QtObject {
        id: _private

        readonly property size bannerSize: Qt.size(1920, 1080)
        readonly property string defaultBannerImage: "qrc:/images/homescreen_banner.png"

        property int layoutType: {
            const s = root.width / bannerSize.width
            const h = bannerSize.height * s
            return (h <= root.height * 0.6) ? 1 : 2
        }

        signal showTooltipRequest(Item _source, string _text)
        signal hideTooltipRequest(Item _source)
        signal showPosterRequest(Item _source, var _image, string _logline)
        signal hidePosterRequest(Item _source)
        signal showScriptalay()
    }
}
