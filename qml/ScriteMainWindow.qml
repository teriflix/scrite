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
import QtQuick.Window 2.15
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.15
import Qt.labs.settings 1.0
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/tasks"
import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/helpers"
import "qrc:/qml/scrited"
import "qrc:/qml/controls"
import "qrc:/qml/mainwindow" as MainWindow
import "qrc:/qml/notifications"
import "qrc:/qml/screenplayeditor"
import "qrc:/qml/floatingdockpanels"

Item {
    id: scriteMainWindow

    Component.onCompleted: _private.init()

    width: 1350
    height: 700

    enabled: !Scrite.document.loading

    MainWindow.AppToolBar {
        id: _appToolBar
    }

    Loader {
        id: mainUiContentLoader

        property bool allowContent: Runtime.loadMainUiContent
        property string sessionId

        anchors.top: _appToolBar.visible ? _appToolBar.bottom : parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        active: allowContent && !Scrite.document.loading
        opacity: 0
        sourceComponent: uiLayoutComponent

        Announcement.onIncoming: (type, data) => {
                                               if(type === Runtime.announcementIds.reloadMainUiRequest) {
                                                    mainUiContentLoader.active = false

                                                    const delay = data && typeof data === "number" ? data : 100
                                                    Utils.execLater(mainUiContentLoader, delay, () => {
                                                                        mainUiContentLoader.active = true
                                                                    })
                                                }
                                           }

        // Recfactor QML: Get rid of this function, unless its called from this file itself.
        // It encourages usage of leap-of-faith IDs, which is a bad idea.
        function reset(callback) {
            active = false
            Qt.callLater( (callback) => {
                             if(callback)
                                 callback()
                             mainUiContentLoader.active = true
                         }, callback )
        }

        Connections {
            target: Runtime

            function onResetMainWindowUi(callback) {
                mainUiContentLoader.reset(callback)
            }
        }

        Component.onCompleted: Utils.execLater(mainUiContentLoader, 200, () => { mainUiContentLoader.opacity = 1 } )
    }

    Component {
        id: uiLayoutComponent

        Rectangle {
            color: mainTabBar.activeTabColor

            PainterPathItem {
                id: tabBarSeparator

                anchors.left: parent.left
                anchors.right: parent.right

                height: 1

                visible: mainTabBar.visible
                renderType: PainterPathItem.OutlineOnly
                outlineColor: Runtime.colors.primary.borderColor
                outlineWidth: height
                renderingMechanism: PainterPathItem.UseQPainter

                painterPath: PainterPath {
                    id: tabBarSeparatorPath

                    property var currentTabP1: tabBarSeparator.mapFromItem(mainTabBar, mainTabBar.currentTabP1.x, mainTabBar.currentTabP1.y)
                    property var currentTabP2: tabBarSeparator.mapFromItem(mainTabBar, mainTabBar.currentTabP2.x, mainTabBar.currentTabP2.y)
                    property point p1: Qt.point(itemRect.left, itemRect.center.y)
                    property point p2: Qt.point(currentTabP1.x, itemRect.center.y)
                    property point p3: Qt.point(currentTabP2.x, itemRect.center.y)
                    property point p4: Qt.point(itemRect.right, itemRect.center.y)

                    MoveTo { x: tabBarSeparatorPath.p1.x; y: tabBarSeparatorPath.p1.y }
                    LineTo { x: tabBarSeparatorPath.p2.x; y: tabBarSeparatorPath.p2.y }
                    MoveTo { x: tabBarSeparatorPath.p3.x; y: tabBarSeparatorPath.p3.y }
                    LineTo { x: tabBarSeparatorPath.p4.x; y: tabBarSeparatorPath.p4.y }
                }
            }

            Loader {
                id: uiLoader

                anchors.fill: parent
                anchors.topMargin: 1

                clip: true

                sourceComponent: {
                    switch(mainTabBar.currentIndex) {
                    case 1: return structureEditorComponent
                    case 2: return notebookEditorComponent
                    case 3: return scritedComponent
                    }
                    return screenplayEditorComponent
                }

                Announcement.onIncoming: (type,data) => {
                                             const stype = "" + type
                                             if(mainTabBar.currentIndex === 0 && stype === "{f4048da2-775d-11ec-90d6-0242ac120003}") {
                                                 uiLoader.active = false
                                                 Utils.execLater(uiLoader, 250, function() {
                                                    uiLoader.active = true
                                                 })
                                             }
                                         }
            }
        }
    }

    Component {
        id: screenplayEditorComponent

        ScreenplayEditor {
            id: screenplayEditor

            HelpTipNotification {
                tipName: "screenplay"
            }

            BasicAttachmentsDropArea {
                id: fileOpenDropArea

                property string droppedFilePath
                property string droppedFileName

                anchors.fill: parent

                allowedType: Attachments.NoMedia
                allowedExtensions: ["scrite", "fdx", "txt", "fountain", "html"]

                onDropped: {
                    if(Scrite.document.empty)
                        Scrite.document.openOrImport(attachment.filePath)
                    else {
                        droppedFilePath = attachment.filePath
                        droppedFileName = attachment.originalFileName
                    }

                    Announcement.shout(Runtime.announcementIds.closeDialogBoxRequest, undefined)
                }

                Loader {
                    id: fileOpenDropAreaNotification

                    Component.onDestruction: _appToolBar.enabled = true

                    anchors.fill: fileOpenDropArea

                    active: fileOpenDropArea.active || fileOpenDropArea.droppedFilePath !== ""
                    onActiveChanged: _appToolBar.enabled = !active

                    sourceComponent: Rectangle {
                        color: Scrite.app.translucent(Runtime.colors.primary.c500.background, 0.5)

                        Rectangle {
                            anchors.fill: fileOpenDropAreaNotice
                            anchors.margins: -30

                            color: Runtime.colors.primary.c700.background
                            radius: 4
                        }

                        Column {
                            id: fileOpenDropAreaNotice

                            anchors.centerIn: parent

                            width: parent.width * 0.5
                            spacing: 20

                            VclLabel {
                                width: parent.width

                                text: fileOpenDropArea.active ? fileOpenDropArea.attachment.originalFileName : fileOpenDropArea.droppedFileName
                                color: Runtime.colors.primary.c700.text
                                wrapMode: Text.WordWrap
                                horizontalAlignment: Text.AlignHCenter

                                font.bold: true
                                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                            }

                            VclLabel {
                                width: parent.width

                                text: fileOpenDropArea.active ? "Drop the file here to open/import it." : "Do you want to open, import or cancel?"
                                color: Runtime.colors.primary.c700.text
                                wrapMode: Text.WordWrap
                                horizontalAlignment: Text.AlignHCenter

                                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                            }

                            VclLabel {
                                width: parent.width

                                text: "NOTE: Any unsaved changes in the currently open document will be discarded."
                                color: Runtime.colors.primary.c700.text
                                visible: !Scrite.document.empty || Scrite.document.fileName !== ""
                                wrapMode: Text.WordWrap
                                horizontalAlignment: Text.AlignHCenter

                                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                            }

                            Row {
                                anchors.horizontalCenter: parent.horizontalCenter

                                spacing: 20
                                visible: !Scrite.document.empty

                                VclButton {
                                    text: "Open/Import"

                                    onClicked: {
                                        Scrite.document.openOrImport(fileOpenDropArea.droppedFilePath)
                                        fileOpenDropArea.droppedFileName = ""
                                        fileOpenDropArea.droppedFilePath = ""
                                    }
                                }

                                VclButton {
                                    text: "Cancel"

                                    onClicked:  {
                                        fileOpenDropArea.droppedFileName = ""
                                        fileOpenDropArea.droppedFilePath = ""
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: structureEditorComponent

        SplitView {
            id: structureEditorSplitView1

            Material.background: Qt.darker(Runtime.colors.primary.windowColor, 1.1)

            orientation: Qt.Vertical

            Rectangle {
                id: structureEditorRow1

                SplitView.fillHeight: true

                color: Runtime.colors.primary.c10.background

                SplitView {
                    id: structureEditorSplitView2

                    Material.background: Qt.darker(Runtime.colors.primary.windowColor, 1.1)

                    anchors.fill: parent

                    orientation: Qt.Horizontal

                    Rectangle {
                        SplitView.fillWidth: true
                        SplitView.minimumWidth: 80

                        color: Runtime.colors.primary.c10.background
                        border {
                            width: Runtime.showNotebookInStructure ? 0 : 1
                            color: Runtime.colors.primary.borderColor
                        }

                        Item {
                            id: structureEditorTabs

                            property int currentTabIndex: 0

                            anchors.fill: parent

                            Announcement.onIncoming: (type,data) => {
                                var sdata = "" + data
                                var stype = "" + type
                                if(Runtime.showNotebookInStructure) {
                                    if(stype === Runtime.announcementIds.tabRequest) {
                                        if(sdata === "Structure")
                                            structureEditorTabs.currentTabIndex = 0
                                        else if(sdata.startsWith("Notebook")) {
                                            structureEditorTabs.currentTabIndex = 1
                                            if(sdata !== "Notebook")
                                                Utils.execLater(notebookViewLoader, 100, function() {
                                                    notebookViewLoader.item.switchTo(sdata)
                                                })
                                        }
                                    } else if(stype === Runtime.announcementIds.characterNotesRequest) {
                                        structureEditorTabs.currentTabIndex = 1
                                        Utils.execLater(notebookViewLoader, 100, function() {
                                            notebookViewLoader.item.switchToCharacterTab(data)
                                        })
                                    }
                                    else if(stype === Runtime.announcementIds.sceneNotesRequest) {
                                        structureEditorTabs.currentTabIndex = 1
                                        Utils.execLater(notebookViewLoader, 100, function() {
                                            notebookViewLoader.item.switchToSceneTab(data)
                                        })
                                    }
                                }
                            }

                            Loader {
                                id: structureEditorTabBar

                                anchors.top: parent.top
                                anchors.left: parent.left
                                anchors.bottom: parent.bottom

                                // active: !Runtime.appFeatures.structure.enabled && Runtime.showNotebookInStructure
                                active: {
                                    if(structureEditorTabs.currentTabIndex === 0)
                                        return !Runtime.appFeatures.structure.enabled && Runtime.showNotebookInStructure
                                    else if(structureEditorTabs.currentTabIndex === 1)
                                        return !Runtime.appFeatures.notebook.enabled && Runtime.showNotebookInStructure
                                    return false
                                }
                                visible: active

                                sourceComponent: Rectangle {
                                    width: appToolBar.height+4

                                    color: Runtime.colors.primary.c100.background

                                    Column {
                                        anchors.horizontalCenter: parent.horizontalCenter

                                        FlatToolButton {
                                            ToolTip.text: "Structure\t(" + Scrite.app.polishShortcutTextForDisplay("Alt+2") + ")"

                                            down: structureEditorTabs.currentTabIndex === 0
                                            visible: Runtime.showNotebookInStructure
                                            iconSource: "qrc:/icons/navigation/structure_tab.png"

                                            onClicked: Announcement.shout(Runtime.announcementIds.tabRequest, "Structure")
                                        }

                                        FlatToolButton {
                                            ToolTip.text: "Notebook Tab (" + Scrite.app.polishShortcutTextForDisplay("Alt+3") + ")"

                                            down: structureEditorTabs.currentTabIndex === 1
                                            visible: Runtime.showNotebookInStructure
                                            iconSource: "qrc:/icons/navigation/notebook_tab.png"

                                            onClicked: Announcement.shout(Runtime.announcementIds.tabRequest, "Notebook")
                                        }
                                    }

                                    Rectangle {
                                        anchors.right: parent.right

                                        width: 1
                                        height: parent.height

                                        color: Runtime.colors.primary.borderColor
                                    }
                                }
                            }

                            Loader {
                                id: structureViewLoader

                                anchors.top: parent.top
                                anchors.left: structureEditorTabBar.active ? structureEditorTabBar.right : parent.left
                                anchors.right: parent.right
                                anchors.bottom: parent.bottom

                                active: Runtime.appFeatures.structure.enabled
                                visible: !Runtime.showNotebookInStructure || structureEditorTabs.currentTabIndex === 0
                                sourceComponent: StructureView {
                                    HelpTipNotification {
                                        tipName: "structure"
                                        enabled: structureViewLoader.visible
                                    }

                                    onEditorRequest: { } // TODO
                                    onReleaseEditorRequest: { } // TODO
                                }

                                DisabledFeatureNotice {
                                    anchors.fill: parent
                                    visible: !parent.active
                                    featureName: "Structure"
                                }
                            }

                            Loader {
                                id: notebookViewLoader
                                anchors.top: parent.top
                                anchors.left: structureEditorTabBar.active ? structureEditorTabBar.right : parent.left
                                anchors.right: parent.right
                                anchors.bottom: parent.bottom

                                active: visible && Runtime.appFeatures.notebook.enabled
                                visible: Runtime.showNotebookInStructure && structureEditorTabs.currentTabIndex === 1

                                sourceComponent: NotebookView {
                                    toolbarSize: appToolBar.height+4
                                    toolbarSpacing: appToolBar.spacing
                                    toolbarLeftMargin: appToolBar.anchors.leftMargin
                                }

                                DisabledFeatureNotice {
                                    anchors.fill: parent
                                    visible: !parent.active
                                    featureName: "Notebook"
                                }
                            }
                        }

                        /**
                          Some of our users find it difficult to know that they can pull the splitter handle
                          to reveal the timeline and/or screenplay editor. So we load an animation letting them
                          know about that and get rid of it once the animation is done.
                          */
                        Loader {
                            id: splitViewAnimationLoader

                            property string sessionId

                            anchors.fill: parent

                            active: false

                            sourceComponent: Rectangle {
                                color: Scrite.app.translucent(Runtime.colors.primary.button, 0.5)

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: splitViewAnimationLoader.active = false
                                }

                                Timer {
                                    interval: 5000
                                    repeat: false
                                    running: true
                                    onTriggered: splitViewAnimationLoader.active = false
                                }

                                Item {
                                    id: screenplayEditorHandle
                                    width: 1
                                    property real marginOnTheRight: 0
                                    anchors.top: parent.top
                                    anchors.right: parent.right
                                    anchors.bottom: parent.bottom
                                    anchors.rightMargin: marginOnTheRight
                                    visible: !screenplayEditor2.active

                                    Rectangle {
                                        height: parent.height * 0.5
                                        width: 5
                                        anchors.right: parent.right
                                        anchors.verticalCenter: parent.verticalCenter
                                        color: Runtime.colors.primary.windowColor
                                        visible: screenplayEditorHandleAnimation.running
                                    }

                                    VclLabel {
                                        color: Runtime.colors.primary.c50.background
                                        text: "Pull this handle to view the screenplay editor."
                                        font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2
                                        anchors.right: parent.left
                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.rightMargin: 20
                                    }

                                    SequentialAnimation {
                                        id: screenplayEditorHandleAnimation
                                        loops: 2
                                        running: screenplayEditorHandle.visible

                                        NumberAnimation {
                                            target: screenplayEditorHandle
                                            property: "marginOnTheRight"
                                            duration: 500
                                            from: 0; to: 50
                                        }

                                        NumberAnimation {
                                            target: screenplayEditorHandle
                                            property: "marginOnTheRight"
                                            duration: 500
                                            from: 50; to: 0
                                        }
                                    }
                                }

                                Item {
                                    id: timelineViewHandle
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.bottom: parent.bottom
                                    anchors.bottomMargin: marginOnTheBottom
                                    height: 1
                                    visible: !structureEditorRow2.active
                                    property real marginOnTheBottom: 0

                                    Rectangle {
                                        width: parent.width * 0.5
                                        height: 5
                                        anchors.bottom: parent.bottom
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        color: Runtime.colors.primary.windowColor
                                        visible: timelineViewHandleAnimation.running
                                    }

                                    VclLabel {
                                        color: Runtime.colors.primary.c50.background
                                        text: "Pull this handle to get the timeline view."
                                        font.pointSize: Runtime.idealFontMetrics.font.pointSize
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        anchors.bottom: parent.top
                                        anchors.bottomMargin: 20
                                    }

                                    SequentialAnimation {
                                        id: timelineViewHandleAnimation
                                        loops: 2
                                        running: timelineViewHandle.visible

                                        NumberAnimation {
                                            target: timelineViewHandle
                                            property: "marginOnTheBottom"
                                            duration: 500
                                            from: 0; to: 50
                                        }

                                        NumberAnimation {
                                            target: timelineViewHandle
                                            property: "marginOnTheBottom"
                                            duration: 500
                                            from: 50; to: 0
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Loader {
                        id: screenplayEditor2
                        SplitView.preferredWidth: scriteMainWindow.width * 0.5
                        SplitView.minimumWidth: 16
                        onWidthChanged: Runtime.workspaceSettings.screenplayEditorWidth = width
                        active: width >= 50
                        sourceComponent: mainTabBar.currentIndex === 1 ? screenplayEditorComponent : null

                        Rectangle {
                            visible: !parent.active
                            anchors.fill: parent
                            color: Runtime.colors.primary.c400.background
                        }
                    }
                }
            }

            Loader {
                id: structureEditorRow2
                SplitView.preferredHeight: 140 + Runtime.minimumFontMetrics.height*Runtime.screenplayTracks.trackCount
                SplitView.minimumHeight: 16
                SplitView.maximumHeight: SplitView.preferredHeight
                active: height >= 50
                sourceComponent: Rectangle {
                    color: FocusTracker.hasFocus ? Runtime.colors.accent.c100.background : Runtime.colors.accent.c50.background
                    FocusTracker.window: Scrite.window

                    Behavior on color {
                        enabled: Runtime.applicationSettings.enableAnimations
                        ColorAnimation { duration: 250 }
                    }

                    TimelineView {
                        anchors.fill: parent
                        showNotesIcon: Runtime.showNotebookInStructure
                    }

                    Rectangle {
                        anchors.fill: parent
                        color: Qt.rgba(0,0,0,0)
                        border { width: 1; color: Runtime.colors.accent.borderColor }
                    }
                }

                Rectangle {
                    visible: !parent.active
                    anchors.fill: parent
                    color: Runtime.colors.primary.c400.background
                }
            }

            Connections {
                target: Scrite.document
                function onAboutToSave() { structureEditorSplitView1.saveLayoutDetails() }
                function onJustLoaded() { structureEditorSplitView1.restoreLayoutDetails() }
            }

            Component.onCompleted: restoreLayoutDetails()
            Component.onDestruction: saveLayoutDetails()

            function saveLayoutDetails() {
                var userData = Scrite.document.userData
                userData["structureTab"] = {
                    "version": 0,
                    "screenplayEditorWidth": screenplayEditor2.width/structureEditorRow1.width,
                    "timelineViewHeight": structureEditorRow2.height
                }
                Scrite.document.userData = userData
            }

            function restoreLayoutDetails() {
                var userData = Scrite.document.userData
                if(userData.structureTab && userData.structureTab.version === 0) {
                    structureEditorRow2.SplitView.preferredHeight = userData.structureTab.timelineViewHeight
                    structureEditorRow2.height = structureEditorRow2.SplitView.preferredHeight
                    screenplayEditor2.SplitView.preferredWidth = structureEditorRow1.width*userData.structureTab.screenplayEditorWidth
                    screenplayEditor2.width = screenplayEditor2.SplitView.preferredWidth
                }

                if(Runtime.structureCanvasSettings.showPullHandleAnimation && mainUiContentLoader.sessionId !== Scrite.document.sessionId) {
                    Utils.execLater(splitViewAnimationLoader, 250, function() {
                        splitViewAnimationLoader.active = !screenplayEditor2.active || !structureEditorRow2.active
                    })
                    mainUiContentLoader.sessionId = Scrite.document.sessionId
                }
            }
        }
    }

    Component {
        id: notebookEditorComponent

        Loader {
            active: Runtime.appFeatures.notebook.enabled
            sourceComponent: NotebookView {
                Announcement.onIncoming: (type,data) => {
                    var stype = "" + Runtime.announcementIds.tabRequest
                    var sdata = "" + data
                    if(stype === Runtime.announcementIds.tabRequest)
                        switchTo(sdata)
                }
            }

            DisabledFeatureNotice {
                anchors.fill: parent
                visible: !parent.active
                featureName: "Notebook"
            }
        }
    }

    Component {
        id: scritedComponent

        Loader {
            active: Runtime.appFeatures.scrited.enabled
            sourceComponent: ScritedView {

            }

            DisabledFeatureNotice {
                anchors.fill: parent
                visible: !parent.active
                featureName: "Scrited"
            }
        }
    }

    MainWindow.AppBusyOverlay {
        id: _appBusyOverlay

        anchors.fill: parent
    }

    MainWindow.Shortcuts {
        id: _shortcuts
    }

    QtObject {
        id: _private

        Announcement.onIncoming: (type, data) => {
                                     if(type === Runtime.announcementIds.showHelpTip) {
                                         _private.showHelpTip(""+data)
                                     }
                                 }

        Component.onCompleted: {
            if(Scrite.app.isMacOSPlatform)
                Scrite.app.openFileRequest.connect(handleOpenFileRequest)
        }

        readonly property Connections runtimeConnectons: Connections {
            target: Runtime

            function onShowNotebookInStructureChanged() {
                Utils.execLater(mainTabBar, 100, function() {
                    mainTabBar.currentIndex = mainTabBar.currentIndex % (Runtime.showNotebookInStructure ? 2 : 3)
                })
            }
        }

        readonly property Connections scriteDocumentConnections: Connections {
            target: Scrite.document

            function onJustReset() {
                Runtime.screenplayEditorSettings.firstSwitchToStructureTab = true
                _appBusyOverlay.ref()
                MainWindow.ReloadPromptDialog.abortLaunchLater()
                Utils.execLater(Runtime.screenplayAdapter, 250, () => {
                                    _appBusyOverlay.deref()
                                    Runtime.screenplayAdapter.sessionId = Scrite.document.sessionId
                                })
            }

            function onJustLoaded() {
                Runtime.screenplayEditorSettings.firstSwitchToStructureTab = true
            }

            function onOpenedAnonymously(filePath) {
                MessageBox.question("Anonymous Open",
                       "The file you just opened is a backup of another file, and is being opened anonymously in <b>read-only</b> mode.<br/><br/>" +
                       "<b>NOTE:</b> In order to edit the file, you will need to first Save-As.",
                        ["Save As", "View Read Only"],
                        (answer) => {
                            if(answer === "Save As")
                                SaveFileTask.saveAs()
                        })
            }

            function onRequiresReload() {
                if(Runtime.applicationSettings.reloadPrompt)
                    MainWindow.ReloadPromptDialog.launchLater()
            }
        }

        property bool handleCloseEvent: true
        readonly property Connections scriteWindowConnections: Connections {
            target: Scrite.window

            function onClosing(close) {
                if(!Scrite.window.closeButtonVisible) {
                    close.accepted = false
                    return
                }

                if(_private.handleCloseEvent) {
                    close.accepted = false

                    Scrite.app.saveWindowGeometry(Scrite.window, "Workspace")

                    SaveFileTask.save( () => {
                                          _private.handleCloseEvent = false
                                          if( TrialNotActivatedDialog.launch() !== null)
                                            return
                                          Scrite.window.close()
                                      } )
                } else
                    close.accepted = true
            }
        }

        readonly property QtObject documentErrorHandler: QtObject {
            property bool errorReportHasError: documentErrors.hasError

            property ErrorReport documentErrors: Aggregation.findErrorReport(Scrite.document)

            onErrorReportHasErrorChanged: {
                if(errorReportHasError) {
                    var msg = documentErrors.errorMessage;

                    if(documentErrors.details && documentErrors.details.revealOnDesktopRequest)
                        msg += "<br/><br/>Click Ok to reveal <u>" + documentErrors.details.revealOnDesktopRequest + "</u> on your computer."

                    MessageBox.information("Scrite Document Error", msg, () => {
                                               if(documentErrors.details && documentErrors.details.revealOnDesktopRequest)
                                                   Scrite.app.revealFileOnDesktop(documentErrors.details.revealOnDesktopRequest)
                                               documentErrors.clear()
                                           })
                }
            }
        }

        readonly property QtObject applicationErrorHandler: QtObject {
            property bool errorReportHasError: applicationErrors.hasError

            property ErrorReport applicationErrors: Aggregation.findErrorReport(Scrite.app)

            onErrorReportHasErrorChanged: {
                if(errorReportHasError)
                    MessageBox.information("Scrite Error", applicationErrors.errorMessage, applicationErrors.clear)
            }
        }

        readonly property QtObject discordHelpTip: HelpTipNotification {
            id: _discordHelpTip

            enabled: tipName !== ""

            Component.onCompleted: {
                Qt.callLater( () => {
                                 if(Runtime.helpNotificationSettings.dayZero === "")
                                    Runtime.helpNotificationSettings.dayZero = new Date()

                                 const days = Runtime.helpNotificationSettings.daysSinceZero()
                                 if(days >= 2) {
                                     if(!Runtime.helpNotificationSettings.isTipShown("discord"))
                                         _discordHelpTip.tipName = "discord"
                                 }
                             })
            }
        }

        readonly property Component helpTipNotification : HelpTipNotification {
            id: _helpTip

            Notification.onDismissed: _helpTip.destroy()
        }

        function init() {
            if(!Scrite.app.restoreWindowGeometry(Scrite.window, "Workspace"))
                Runtime.workspaceSettings.screenplayEditorWidth = -1
            Runtime.screenplayAdapter.sessionId = Scrite.document.sessionId
            Qt.callLater( function() {
                Announcement.shout("{f4048da2-775d-11ec-90d6-0242ac120003}", "restoreWindowGeometryDone")
            })
        }

        function handleOpenFileRequest(fileName) {
            if(Scrite.app.isMacOSPlatform) {
                if(Scrite.document.empty) {
                    Announcement.shout(Runtime.announcementIds.closeHomeScreenRequest, undefined)
                    OpenFileTask.open(fileName)
                } else {
                    let fileInfo = Qt.createQmlObject("import io.scrite.components 1.0; BasicFileInfo { }", _private)
                    fileInfo.absoluteFilePath = fileName

                    const justFileName = fileInfo.baseName
                    fileInfo.destroy()

                    MessageBox.question("Open Options",
                                        "How do you want to open <b>" + justFileName + "</b>?",
                                        ["This Window", "New Window"], (answer) => {
                                            if(answer === "This Window")
                                                OpenFileTask.open(fileName)
                                            else
                                                Scrite.app.launchNewInstanceAndOpen(Scrite.window, fileName);
                                        })
                }
            }
        }

        function showHelpTip(tipName) {
            if(Runtime.helpTips[tipName] !== undefined && !Runtime.helpNotificationSettings.isTipShown(tipName)) {
                _private.helpTipNotification.createObject(Scrite.window.contentItem, {"tipName": tipName})
            }
        }
    }
}
