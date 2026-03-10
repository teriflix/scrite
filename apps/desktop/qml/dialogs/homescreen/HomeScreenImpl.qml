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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Dialogs
import QtQuick.Layouts
import QtQuick.Controls

import io.scrite.components

import "../../tasks"

import "../../globals"
import "../../helpers"
import ".."
import "../../controls"

Item {
    id: root

    property string mode

    signal closeRequest()

    Component.onCompleted: {
        _private.switchMode()
        root.modeChanged.connect(_private.switchMode)
    }

    Loader {
        anchors.fill: parent
        sourceComponent: {
            switch(_private.layoutType) {
            case 1: return _homeScreenLayout1
            case 2: return _homeScreenLayout2
            default:
            }
            return _homeScreenLayout2
        }
    }

    // Use this if the scaled banner height is less than 60% of the home screen height
    Component {
        id: _homeScreenLayout1

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
                    push(_scriptalayPage)
                }

                function onShowImportPanel() {
                    push(_importPage)
                }

                function onShowVaultPanel() {
                    push(_vaultPage)
                }

                Component.onCompleted: {
                    _private.showScriptalay.connect(onShowScriptalay)
                    _private.showVaultPanel.connect(onShowVaultPanel)
                    _private.showImportPanel.connect(onShowImportPanel)
                }
            }
        }
    }

    // Use this otherwise
    Component {
        id: _homeScreenLayout2

        StackView {
            clip: true

            initialItem: ContentPageLayout2 { }

            function onShowScriptalay() {
                push(_scriptalayPage)
            }

            function onShowImportPanel() {
                push(_importPage)
            }

            function onShowVaultPanel() {
                push(_vaultPage)
            }

            Component.onCompleted: {
                _private.showScriptalay.connect(onShowScriptalay)
                _private.showVaultPanel.connect(onShowVaultPanel)
                _private.showImportPanel.connect(onShowImportPanel)
            }
        }
    }

    component TopBanner : Image {
        id: _banner

        readonly property StackView stackView: Aggregation.firstSiblingByType("QQuickStackView")

        source: {
            if(stackView.currentItem && stackView.currentItem.bannerImage)
                return stackView.currentItem.bannerImage
            return _private.defaultBannerImage
        }
        fillMode: Image.PreserveAspectFit

        Image {
            anchors.centerIn: parent
            source: (_banner.stackView.currentItem && _banner.stackView.currentItem.bannerImage) ? "" : "qrc:/images/banner_logo_overlay.png"
            width: root.width * 0.3
            fillMode: Image.PreserveAspectFit
            smooth: true; mipmap: true
            visible: false
            Component.onCompleted: Runtime.execLater(_banner, 50, () => { visible = true } )
        }

        RowLayout {
            spacing: 20

            anchors.bottom: parent.bottom
            anchors.bottomMargin: 20
            anchors.horizontalCenter: parent.horizontalCenter
            visible: !_topBannerToolTip.visible

            LinkButton {
                Layout.fillWidth: true

                text: "Learning Guides"
                color: Qt.rgba(0,0,0,0)
                textColor: "white"
                iconSource: "qrc:/icons/action/help_inverted.png"

                onClicked: Qt.openUrlExternally(Runtime.userGuidesUrl)
            }

            LinkButton {
                Layout.fillWidth: true

                text: "Discord Community"
                color: Qt.rgba(0,0,0,0)
                textColor: "white"
                iconSource: "qrc:/icons/action/forum_inverted.png"

                onClicked: JoinDiscordCommunity.launch()
            }
        }

        VclLabel {
            id: _topBannerToolTip

            width: parent.width * 0.75

            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.leftMargin: 20
            anchors.bottomMargin: 20

            color: "white"
            elide: Text.ElideRight
            padding: 10
            visible: text !== ""
            wrapMode: Text.WordWrap
            font.pointSize: Runtime.idealFontMetrics.font.pointSize
            maximumLineCount: 4
            verticalAlignment: Text.AlignBottom
            horizontalAlignment: Text.AlignLeft

            background: Rectangle {
                color: "black"
                opacity: 0.8
                radius: 5
            }

            property Item source

            Connections {
                target: _private

                function onShowTooltipRequest(_source, _text) {
                    _topBannerToolTip.source = _source
                    _topBannerToolTip.text = _text
                }

                function onHideTooltipRequest(_source) {
                    if(_topBannerToolTip.source === _source) {
                        _topBannerToolTip.source = null
                        _topBannerToolTip.text = ""
                    }
                }
            }
        }

        Poster {
            id: _topBannerPoster

            anchors.fill: parent

            property Item sourceItem

            Connections {
                target: _private

                function onShowPosterRequest(_source, _image, _logline) {
                    _topBannerPoster.sourceItem = _source
                    _topBannerPoster.source = _image
                    _topBannerPoster.logline = _logline
                }

                function onHidePosterRequest(_source) {
                    if(_topBannerPoster.sourceItem === _source) {
                        _topBannerPoster.sourceItem = null
                        _topBannerPoster.source = undefined
                        _topBannerPoster.logline = ""
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
                readonly property StackView stackView: Aggregation.firstParentByType("QQuickStackView")

                anchors.centerIn: parent
                source: (stackView.currentItem && stackView.currentItem.bannerImage) ? "" : "qrc:/images/banner_logo_overlay.png"
                width: parent.width * 0.3
                fillMode: Image.PreserveAspectFit
                smooth: true; mipmap: true
                visible: false
                Component.onCompleted: Runtime.execLater(parent, 50, () => { visible = true } )
            }

            Poster {
                id: _sideBannerPoster

                anchors.fill: parent

                property Item sourceItem

                Connections {
                    target: _private

                    function onShowPosterRequest(_source, _image, _logline) {
                        _sideBannerPoster.sourceItem = _source
                        _sideBannerPoster.source = _image
                    }

                    function onHidePosterRequest(_source) {
                        if(_sideBannerPoster.sourceItem === _source) {
                            _sideBannerPoster.sourceItem = null
                            _sideBannerPoster.source = undefined
                        }
                    }
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: _toolTipLayout.implicitHeight > height

            ColumnLayout {
                id: _toolTipLayout

                anchors.fill: parent
                spacing: 10

                VclLabel {
                    id: _sideBannerTooltip

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
                            _sideBannerTooltip.source = _source
                            _sideBannerTooltip.text = _text
                            _sideBannerHelpButtons.visible = false
                        }

                        function onHideTooltipRequest(_source) {
                            if(_sideBannerTooltip.source === _source) {
                                _sideBannerTooltip.source = null
                                _sideBannerTooltip.text = _sideBannerTooltip.defaultText
                                _sideBannerHelpButtons.visible = true
                            }
                        }
                    }

                    readonly property string defaultText: File.read(":/misc/homescreen_info.md")
                }

                ColumnLayout {
                    id: _sideBannerHelpButtons
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    LinkButton {
                        Layout.fillWidth: true

                        text: "Learning Guides"
                        iconSource: "qrc:/icons/action/help.png"

                        onClicked: Qt.openUrlExternally(Runtime.userGuidesUrl)
                    }

                    LinkButton {
                        Layout.fillWidth: true

                        text: "Discord Community"
                        iconSource: "qrc:/icons/action/forum.png"

                        onClicked: JoinDiscordCommunity.launch()
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
                        Layout.fillHeight: true
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
        id: _iconFromSource

        Image {
            fillMode: Image.PreserveAspectFit
            smooth: true; mipmap: true
        }
    }

    Component {
        id: _iconFromImage

        QImageItem {
            fillMode: QImageItem.PreserveAspectFit
            useSoftwareRenderer: Runtime.currentUseSoftwareRenderer
        }
    }

    component LinkButton : Rectangle {
        id: _button
        property string text
        property string tooltip
        property string iconSource
        property color  textColor: Runtime.colors.primary.regular.text
        property var    iconImage: Gui.emptyQImage // has to be QImage
        property bool   singleClick: true
        property bool   showPoster: false
        property bool   containsMouse: _buttonMouseArea.containsMouse

        signal clicked()
        signal doubleClicked()

        width: 100
        height: _buttonLayout.height + 6
        color: _buttonMouseArea.containsMouse ? Runtime.colors.primary.highlight.background : Qt.rgba(0,0,0,0)

        implicitWidth: _buttonIcon.implicitWidth + _buttonLabel.implicitWidth + _buttonLayout.spacing
        implicitHeight: _buttonLayout.height + 10

        RowLayout {
            id: _buttonLayout
            width: parent.width - 10
            anchors.centerIn: parent
            spacing: 5
            opacity: enabled ? 1 : 0.6

            Loader {
                id: _buttonIcon

                property real h: _buttonLabel.contentHeight * 1.5

                Layout.preferredWidth: h
                Layout.preferredHeight: h

                sourceComponent: {
                    if(iconSource !== "")
                        return _iconFromSource
                    return _iconFromImage
                }

                onLoaded: {
                    if(iconSource !== "")
                        item.source = Qt.binding( () => { return iconSource } )
                    else
                        item.image = Qt.binding( () => { return iconImage ? iconImage : Gui.emptyQImage } )
                }
            }

            VclLabel {
                id: _buttonLabel

                Layout.fillWidth: true

                text: _button.text
                color: _button.textColor
                elide: Text.ElideRight
                padding: 3
                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                font.underline: singleClick ? _buttonMouseArea.containsMouse : false
            }
        }

        MouseArea {
            id: _buttonMouseArea
            anchors.fill: parent
            hoverEnabled: singleClick || _button.tooltip !== ""
            cursorShape: singleClick ? Qt.PointingHandCursor : Qt.ArrowCursor
            onClicked: _button.clicked()
            onDoubleClicked: _button.doubleClicked()
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
        ColumnLayout {
            anchors.fill: parent

            VclLabel {
                id: _newFileLabel
                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                text: "New File"
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                // Layout.leftMargin: 20
                Layout.rightMargin: 20
                color: Qt.rgba(0,0,0,0)
                border.width: _templatesView.interactive ? 1 : 0
                border.color: Runtime.colors.primary.borderColor

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 1
                    visible: !Runtime.appFeatures.templates.enabled

                    LinkButton {
                        Layout.fillWidth: true

                        text: "New from Clipboard"
                        enabled: Scrite.document.canImportFromClipboard
                        tooltip: "Create a new screenplay by interpreting text on the clipboard as fountain file."
                        iconSource: "qrc:/icons/filetype/document.png"

                        onClicked: {
                            if(Scrite.document.canImportFromClipboard)
                                SaveFileTask.save( () => {
                                                        Scrite.document.importFromClipboard()
                                                        root.closeRequest()
                                                    } )
                            else
                                MessageBox.information("Clipboard Empty", "No text is available in the system clipboard to import.")
                        }
                    }

                    LinkButton {
                        Layout.fillWidth: true

                        text: "Blank Document"
                        iconSource: "qrc:/icons/filetype/document.png"
                        tooltip: "A crisp and clean new document to write your next blockbuster!"

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
                    id: _templatesView
                    anchors.fill: parent
                    anchors.margins: 1
                    model: Runtime.appFeatures.templates.enabled ? Runtime.libraryService.templates : []
                    visible: Runtime.appFeatures.templates.enabled
                    currentIndex: -1
                    clip: true
                    FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor
                    ScrollBar.vertical: VclScrollBar {
                        flickable: _templatesView
                    }
                    highlightMoveDuration: 0
                    interactive: height < contentHeight
                    header: LinkButton {
                        width: _templatesView.width

                        text: "New from Clipboard"
                        enabled: Scrite.document.canImportFromClipboard
                        tooltip: "Create a new screenplay by interpreting text on the clipboard as fountain file."
                        showPoster: false
                        iconSource: "qrc:/icons/filetype/document.png"

                        onClicked: {
                            if(Scrite.document.canImportFromClipboard)
                                SaveFileTask.save( () => {
                                                        Scrite.document.importFromClipboard()
                                                        root.closeRequest()
                                                    } )
                            else
                                MessageBox.information("Clipboard Empty", "No text is available in the system clipboard to import.")
                        }
                    }

                    delegate: LinkButton {
                        required property int index
                        required property var record

                        width: _templatesView.width

                        iconSource: index === 0 ? record.poster : Runtime.libraryService.templates.baseUrl + "/" + record.poster
                        showPoster: index > 0
                        text: record.name
                        tooltip: record.description

                        onClicked: {
                            SaveFileTask.save( () => {
                                                    var task = OpenFromLibraryTask.openTemplateAt(Runtime.libraryService, index)
                                                    task.finished.connect(closeRequest)
                                                } )
                        }
                    }
                }

                BusyIndicator {
                    anchors.centerIn: parent
                    running: Runtime.libraryService.busy
                }
            }
        }
    }

    component OpenFileOptions : ColumnLayout {
        readonly property StackView stackView: Aggregation.firstParentByType("QQuickStackView")

        LinkButton {
            text: "Open ..."
            iconSource: "qrc:/icons/file/folder_open.png"
            Layout.fillWidth: true
            tooltip: "Launches a file dialog box so you can select a .scrite file to load from disk."
            onClicked: {
                SaveFileTask.save( () => {
                                        _openFileDialog.open()
                                    } )
            }
        }

        LinkButton {
            text: Runtime.recentFiles.count === 0 ? "Recent files ..." : "Scriptalay"
            iconSource: Runtime.recentFiles.count === 0 ? "qrc:/icons/filetype/document.png" : "qrc:/icons/action/library.png"
            Layout.fillWidth: true
            tooltip: Runtime.recentFiles.count === 0 ? "Reopen a recently opened file." : "Download a screenplay from our online library."
            onClicked: parent.stackView.push(_scriptalayPage)
            enabled: Runtime.recentFiles.count > 0
        }
    }

    Component {
        id: _quickFilesRecentFilesDelegate

        LinkButton {
            id: _quickRecentFile

            required property int index
            required property var fileInfo

            width: ListView.view.width

            text: {
                if(Runtime.recentFiles.preferTitleVersionText)
                    return fileInfo.title === "" ? fileInfo.baseFileName : composeTextFromTitleAndVersion(fileInfo)
                return fileInfo.baseFileName
            }
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

            ToolTipPopup {
                container: _quickRecentFile
                text: fileInfo.filePath
                visible: _quickRecentFile.containsMouse && _private.layoutType === 1
            }

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
        id: _quickFilesScriptalayDelegate

        LinkButton {
            required property int index
            required property var record

            width: ListView.view.width

            iconSource: Runtime.libraryService.screenplays.baseUrl + "/" + record.poster
            showPoster: true
            text: record.name
            tooltip: "<i>" + record.authors + "</i><br/><br/>" + record.logline

            onClicked: {
                SaveFileTask.save( () => {
                                        var task = OpenFromLibraryTask.openScreenplayAt(Runtime.libraryService, index)
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

            RowLayout {
                Layout.fillWidth: true

                VclToolButton {
                    Layout.preferredHeight: _quickFileOptionsLabel.height
                    Layout.preferredWidth: _quickFileOptionsLabel.height

                    visible: Runtime.recentFiles.count > 0
                    toolTipText: "Click to remove items from the recent files list."

                    icon.width: _quickFileOptionsLabel.height * 0.75
                    icon.height: _quickFileOptionsLabel.height * 0.75
                    icon.source: "qrc:/icons/action/edit.png"

                    onClicked: EditRecentFilesDialog.launch()
                }

                VclLabel {
                    id: _quickFileOptionsLabel

                    Layout.fillWidth: true

                    font.pointSize: Runtime.idealFontMetrics.font.pointSize
                    text: scriptalayMode ? "Scriptalay" : "Recent Files"
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                // Layout.leftMargin: 20
                // Layout.rightMargin: 20
                color: Qt.rgba(0,0,0,0)
                border.width: _quickFilesView.interactive ? 1 : 0
                border.color: Runtime.colors.primary.borderColor

                ListView {
                    id: _quickFilesView // shows either Scriptalay or Recent Files
                    anchors.fill: parent
                    anchors.margins: 1
                    model: scriptalayMode ? Runtime.libraryService.screenplays : Runtime.recentFiles
                    currentIndex: -1
                    clip: true
                    FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor
                    ScrollBar.vertical: VclScrollBar {
                        flickable: _quickFilesView
                    }
                    highlightMoveDuration: 0
                    interactive: height < contentHeight
                    delegate: scriptalayMode ? quickFilesScriptalayDelegate : _quickFilesRecentFilesDelegate
                }
            }
        }
    }

    component ImportOptions : ColumnLayout {
        readonly property StackView stackView: Aggregation.firstParentByType("QQuickStackView")

        // Show restore and import options
        LinkButton {
            Layout.fillWidth: true

            text: "Recover ..."
            tooltip: "Open cached files from your private on-disk vault."
            iconSource: "qrc:/icons/file/backup_open.png"

            onClicked: parent.stackView.push(_vaultPage)
        }

        LinkButton {
            Layout.fillWidth: true

            text: "Import ..."
            tooltip: "Import a screenplay from Final Draft, Fountain or HTML formats."
            iconSource: "qrc:/icons/file/import_export.png"

            onClicked: parent.stackView.push(_importPage)
        }
    }

    component ScriptalayPage : Item {
        // Show contents of Scriptalay
        property bool hasSelection: _screenplaysView.currentIndex >= 0

        function openSelected() {
            SaveFileTask.save( () => {
                                    if(_screenplaysView.currentIndex >= 0) {
                                         var task = OpenFromLibraryTask.openScreenplayAt(Runtime.libraryService, _screenplaysView.currentIndex)
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
                    id: _screenplaysView
                    anchors.fill: parent
                    anchors.margins: 1
                    clip: true
                    model: Runtime.libraryService.screenplays
                    currentIndex: -1
                    FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor
                    highlight: Rectangle {
                        color: Runtime.colors.primary.highlight.background
                    }
                    ScrollBar.vertical: VclScrollBar {
                        flickable: _screenplaysView
                    }
                    highlightMoveDuration: 0
                    highlightResizeDuration: 0
                    delegate: LinkButton {
                        required property int index
                        required property var record

                        width: _screenplaysView.width

                        text: record.name
                        singleClick: false
                        iconSource: Runtime.libraryService.screenplays.baseUrl + "/" + record.poster

                        onClicked: _screenplaysView.currentIndex = index
                        onDoubleClicked: {
                            _screenplaysView.currentIndex = index
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
                        readonly property StackView stackView: Aggregation.firstParentByType("QQuickStackView")

                        Layout.fillWidth: true
                        Layout.preferredHeight: (_private.bannerSize.height / _private.bannerSize.width) * width

                        visible: _private.layoutType === 2
                        source: stackView.currentItem.bannerImage
                        fillMode: Image.PreserveAspectFit

                        Poster {
                            id: _scriptalayPoster

                            anchors.fill: parent

                            property Item sourceItem

                            Connections {
                                target: _private

                                function onShowPosterRequest(_source, _image, _logline) {
                                    _scriptalayPoster.sourceItem = _source
                                    _scriptalayPoster.source = _image
                                }

                                function onHidePosterRequest(_source) {
                                    if(_scriptalayPoster.sourceItem === _source) {
                                        _scriptalayPoster.sourceItem = null
                                        _scriptalayPoster.source = undefined
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
                            id: _screenplayDetailsFlick
                            anchors.fill: parent
                            anchors.margins: 1
                            contentWidth: _screenplayDetailsText.width
                            contentHeight: _screenplayDetailsText.height
                            clip: true
                            flickableDirection: Flickable.VerticalFlick

                            ScrollBar.vertical: VclScrollBar {
                                flickable: _screenplayDetailsFlick
                            }

                            TextArea {
                                id: _screenplayDetailsText
                                width: _screenplayDetailsFlick.width-20
                                property var record: _screenplaysView.currentIndex >= 0 ? Runtime.libraryService.screenplays.recordAt(_screenplaysView.currentIndex) : undefined
                                textFormat: record ? TextArea.RichText : TextArea.MarkdownText
                                wrapMode: Text.WordWrap
                                padding: 8
                                readOnly: true
                                background: Item { }
                                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                                text: record ? composeTextFromRecord(record) : defaultText

                                onRecordChanged: {
                                    if(record) {
                                        _private.showTooltipRequest(_screenplaysView.currentItem, record.logline)
                                        _private.showPosterRequest(_screenplaysView.currentItem, _screenplaysView.currentItem.iconSource, record.logline)
                                    }
                                }
                                Component.onDestruction: {
                                    _private.hideTooltipRequest(_screenplayDetailsText)
                                    _private.hidePosterRequest(_screenplaysView.currentItem)
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

                                readonly property string defaultText: File.read(":/misc/scriptalay_info.md")
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
            if(_vaultFilesView.currentIndex < 0)
                return

            SaveFileTask.save( () => {
                                    root.enabled = false

                                    var task = OpenFileTask.openAnonymously(_vaultFilesView.currentItem.fileInfo.filePath)
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
            id: _vaultFilesView
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

                width: _vaultFilesView.width-1
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

                onClicked: _vaultFilesView.currentIndex = index
                onDoubleClicked: {
                    _vaultFilesView.currentIndex = index
                    Qt.callLater( openSelected )
                }
            }
        }

        ColumnLayout {
            anchors.centerIn: parent

            width: parent.width * 0.8

            VclLabel {
                Layout.fillWidth: true

                horizontalAlignment: Text.AlignHCenter
                text: Scrite.vault.busy ? "Looking for documents in the vault ..." : "No documents found in vault."
                visible: Scrite.vault.documentCount === 0
                wrapMode: Text.WordWrap
            }

            BusyIndicator {
                Layout.alignment: Qt.AlignHCenter

                visible: Scrite.vault.busy
                running: Scrite.vault.busy
            }
        }

    }

    component ImportPage : Item {
        property bool hasActionButton: _importPageStackLayout.currentIndex >= 1
        property string actionButtonText: _importPageStackLayout.currentIndex === 1 ? "Browse" : "Import"
        function onActionButtonClicked() {
            if(_importPageStackLayout.currentIndex === 1)
                _dropBrowseItem.doBrowse()
            else if(_importPageStackLayout.currentIndex === 2) {
                SaveFileTask.save( () => {
                                        root.enabled = false
                                        _importDroppedFileItem.doImport()
                                    } )
            }
        }

        QtObject {
            id: _fileToImport

            property bool valid: path !== ""
            property string path
            property var info: File.info(path)
            property string name: info.fileName
            property string folder: info.absolutePath
        }

        BasicAttachmentsDropArea {
            id: _importDropArea
            anchors.fill: parent
            enabled: Runtime.appFeatures.importer.enabled
            allowedType: Attachments.NoMedia
            allowedExtensions: ["scrite", "fdx", "txt", "fountain", "html"]
            onDropped: _fileToImport.path = attachment.filePath
        }

        StackLayout {
            id: _importPageStackLayout
            anchors.fill: parent
            currentIndex: Runtime.appFeatures.importer.enabled ? (_fileToImport.valid ? 2 : 1) : 0

            DisabledFeatureNotice {
                visible: !Runtime.appFeatures.importer.enabled
                color: Qt.rgba(1,1,1,0.9)
                featureName: "Import from 3rd Party Formats"
            }

            Rectangle {
                id: _dropBrowseItem
                border.width: 1
                border.color: Runtime.colors.primary.borderColor
                color: Qt.rgba(0,0,0,0)

                function doBrowse() {
                    _importFileDialog.open()
                }

                VclFileDialog {
                    id: _importFileDialog

                    title: "Import Screenplay"
                    objectName: "Import Dialog Box"
                    nameFilters: ["*.scrite *.fdx *.fountain *.txt *.html"]
                    currentFolder: Runtime.workspaceSettings.lastOpenImportFolderUrl

                    onCurrentFolderChanged: Runtime.workspaceSettings.lastOpenImportFolderUrl = folder

                    onAccepted: {
                        if(selectedFile !== "")
                            _fileToImport.path = Url.toPath(selectedFile)
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
                        text: _importDropArea.active ? _importDropArea.attachment.originalFileName : "Drop a file on to this area to import it."
                    }

                    VclLabel {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize-1
                        color: Runtime.colors.primary.c700.background
                        text: _importDropArea.active ? "Drop to import this file." : "(Allowed file types: " + _importFileDialog.nameFilters.join(", ") + ")"
                    }
                }
            }

            Rectangle {
                id: _importDroppedFileItem
                border.width: 1
                border.color: Runtime.colors.primary.borderColor
                color: Qt.rgba(0,0,0,0)

                function doImport() {
                    Runtime.workspaceSettings.lastOpenImportFolderUrl = "file://" + _fileToImport.folder

                    var task = OpenFileTask.openOrImport(_fileToImport.path)
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
                        text: _fileToImport.name
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
        id: _stackPage

        property Component content
        property QtObject contentItem: _contentLoader.item

        property Component title
        property QtObject titleItem: _titleLoader.item

        property Component buttons
        property QtObject buttonsItem: _buttonsLoader.item

        readonly property StackView stackView: Aggregation.firstParentByType("QQuickStackView")

        Item {
            anchors.fill: parent
            anchors.topMargin: 25
            anchors.leftMargin: 50
            anchors.rightMargin: 50
            anchors.bottomMargin: 25

            Loader {
                id: _contentLoader
                width: parent.width
                anchors.top: parent.top
                anchors.bottom: _buttonsRow.top
                anchors.bottomMargin: 20
                sourceComponent: _stackPage.content
            }

            RowLayout {
                id: _buttonsRow
                width: parent.width
                anchors.bottom: parent.bottom

                VclButton {
                    text: "< Back"
                    onClicked: _stackPage.stackView.pop()

                    EventFilter.target: Scrite.app
                    EventFilter.events: [EventFilter.KeyPress,EventFilter.KeyRelease,EventFilter.Shortcut]
                    EventFilter.onFilter: (watched,event,result) => {
                                              if(event.key === Qt.Key_Escape) {
                                                  result.acceptEvent = true
                                                  result.filter = true
                                                  _stackPage.stackView.pop()
                                              }
                                          }
                }

                Loader {
                    id: _titleLoader
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    sourceComponent: _stackPage.title
                }

                Loader {
                    id: _buttonsLoader
                    sourceComponent: _stackPage.buttons
                }
            }
        }
    }

    Component {
        id: _scriptalayPage

        StackPage {
            id: _scriptalayPageItem
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
                enabled: _scriptalayPageItem.contentItem.hasSelection && Runtime.libraryService.screenplays.count > 0
                onClicked: _scriptalayPageItem.contentItem.openSelected()
            }
        }
    }

    Component {
        id: _vaultPage

        StackPage {
            id: _vaultPageItem
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
                    onClicked: _vaultPageItem.contentItem.openSelected()
                    enabled: Scrite.vault.documentCount > 0
                }

                VclButton {
                    text: "Clear"
                    enabled: Scrite.vault.documentCount > 0
                    onClicked: _vaultPageItem.contentItem.clearVault()
                }
            }
        }
    }

    Component {
        id: _importPage

        StackPage {
            id: _importPageItem
            content: ImportPage { }
            title: Item { }
            buttons: RowLayout {
                VclButton {
                    visible: _importPageItem.contentItem.hasActionButton
                    text: _importPageItem.contentItem.actionButtonText
                    onClicked: _importPageItem.contentItem.onActionButtonClicked()
                }
            }
        }
    }

    VclFileDialog {
        id: _openFileDialog

        title: "Open Scrite Document"
        nameFilters: ["Scrite Documents (*.scrite)"]
        objectName: "Open File Dialog"
        currentFolder: Runtime.workspaceSettings.lastOpenFolderUrl

        onCurrentFolderChanged: Runtime.workspaceSettings.lastOpenFolderUrl = folder

        onAccepted: {
            Runtime.workspaceSettings.lastOpenFolderUrl = folder

            const path = Url.toPath(selectedFile)

            let task = OpenFileTask.open(path)
            task.finished.connect(closeRequest)
        }
    }

    VclDialog {
        id: _missingRecentFilesNotificationDialog

        property var missingFiles: Runtime.recentFiles.missingFiles

        width: root.width * 0.8
        height: Math.min(350, root.height * 0.8)
        title: "Recent Files Missing"

        content: Item {
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20

                spacing: 20

                VclLabel {
                    Layout.fillWidth: true

                    text: "The following recent file(s) were either deleted, renamed, moved, or otherwise not accessible and will no longer be shown in the home screen:"
                    wrapMode: Text.WordWrap
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    color: Runtime.colors.primary.c100.background
                    border.width: 1
                    border.color: Runtime.colors.primary.borderColor

                    ListView {
                        id: _missingFilesList

                        anchors.fill: parent
                        anchors.margins: 1

                        highlight: Rectangle {
                            color: Runtime.colors.primary.highlight.background
                        }
                        highlightMoveDuration: 0
                        highlightResizeDuration: 0

                        clip: true
                        currentIndex: -1

                        delegate: VclText {
                            required property int index
                            required property string modelData

                            width: _missingFilesList.width
                            padding: 8
                            rightPadding: 20
                            elide: Text.ElideMiddle
                            text: modelData
                            color: currentIndex === index ? Runtime.colors.primary.highlight.text : Runtime.colors.primary.c100.text

                            MouseArea {
                                id: _missingFileMouseArea

                                anchors.fill: parent
                                hoverEnabled: parent.truncated

                                onClicked: _missingFilesList.currentIndex = index
                            }

                            ToolTipPopup {
                                container: _missingFileMouseArea

                                text: modelData
                                visible: _missingFileMouseArea.containsMouse
                            }
                        }

                        Component.onCompleted: {
                            model = JSON.parse( JSON.stringify(_missingRecentFilesNotificationDialog.missingFiles) )
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true

                    VclCheckBox {
                        text: "Don't show again"
                        checked: false
                        onToggled: Runtime.applicationSettings.notifyMissingRecentFiles = !checked
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                    }

                    Link {
                        text: "More Info"
                        onClicked: Qt.openUrlExternally("https://www.scrite.io/version-1-2-released/#chapter7_recent_files_not_found_notification")
                    }
                }
            }
        }

        function reportMissingRecentFiles() {
            if(Runtime.applicationSettings.notifyMissingRecentFiles) {
                if(missingFiles.length && missingFiles.length > 0)
                    open()
            } else {
                Runtime.recentFiles.missingFiles = []
            }
        }

        onClosed: Runtime.recentFiles.missingFiles = []
        onMissingFilesChanged: Qt.callLater(reportMissingRecentFiles)

        Component.onCompleted: Qt.callLater(reportMissingRecentFiles)
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
        signal showVaultPanel()
        signal showImportPanel()

        function switchMode() {
            Runtime.execLater(root, 500, () => {
                                  if(root.mode === "Scriptalay") {
                                      _private.showScriptalay()
                                  } else if(root.mode === "Import") {
                                      _private.showImportPanel()
                                  } else if(root.mode === "Recover") {
                                      _private.showVaultPanel()
                                  }
                              })
        }
    }
}
