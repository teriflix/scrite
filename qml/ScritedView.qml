/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQuick 2.13
import QtMultimedia 5.13
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.13
import Qt.labs.settings 1.0
import QtQuick.Controls 2.13
import QtQuick.Controls.Material 2.12

import Scrite 1.0

Item {
    id: scritedView

    property int skipDuration: 10*1000
    property bool mediaIsLoaded: mediaPlayer.status !== MediaPlayer.NoMedia
    property bool mediaIsPlaying: mediaPlayer.playbackState === MediaPlayer.PlayingState
    property bool mediaIsPaused: mediaPlayer.playbackState === MediaPlayer.PausedState
    property alias timeOffsetVisible: screenplayOffsetsView.displayTimeOffset
    property bool nextSceneAvailable: screenplayOffsetsView.currentIndex+1 < screenplayOffsetsView.count
    property bool previousSceneAvailable: screenplayOffsetsView.currentIndex > 0
    property alias screenplaySplitsCount: screenplayOffsetsView.count
    property alias playbackScreenplaySync: mediaPlayer.keepScreenplayInSyncWithPosition
    property bool canScrollUp: textDocumentFlick.contentY > 0
    property bool canScrollDown: textDocumentFlick.contentY < textDocumentFlick.contentHeight - textDocumentFlick.height

    Component.onCompleted: {
        scritedToolbar.scritedView = scritedView
        if(!scritedViewSettings.experimentalFeatureNoticeDisplayed) {
            app.execLater(scritedView, 250, function() {
                showInformation({
                    "message": "<strong>Scrited Tab : Study screenplay and film together.</strong><br/><br/>This is an experimental feature. Help us polish it by leaving feedback on the Forum at www.scrite.io. Thank you!",
                    "callback": function() {
                        scritedViewSettings.experimentalFeatureNoticeDisplayed = true
                    }
                })
            })
        }
    }
    Component.onDestruction: scritedToolbar.scritedView = null

    function loadMedia() {
        fileDialog.open()
    }

    function togglePlayback() {
        mediaPlayer.togglePlayback()
    }

    function rewind() {
        mediaPlayer.seek( Math.max(mediaPlayer.position-skipDuration, 0 ) )
    }

    function forward() {
        mediaPlayer.seek( Math.min(mediaPlayer.position+skipDuration, mediaPlayer.duration) )
    }

    function miniRewind() {
        mediaPlayer.seek( Math.max(mediaPlayer.position-Math.max(500,mediaPlayer.notifyInterval), 0) )
    }

    function miniForward() {
        mediaPlayer.seek( Math.min(mediaPlayer.position+Math.max(500,mediaPlayer.notifyInterval), mediaPlayer.duration) )
    }

    function syncVideoTimeWithScreenplayOffsets(adjustFollowingRows) {
        screenplayOffsetsModel.setTime(screenplayOffsetsView.currentIndex, mediaPlayer.position, adjustFollowingRows === true)
    }

    function resetScreenplayOffsets() {
        screenplayOffsetsModel.resetAllTimes()
    }

    function scrollUp() {
        var newY = Math.max(textDocumentFlick.contentY - textDocumentFlick.lineHeight, 0)
        textDocumentFlick.contentY = newY
    }

    function scrollPreviousScene() {
        screenplayOffsetsView.currentIndex = Math.max(screenplayOffsetsView.currentIndex-1,0)
    }

    function scrollPreviousScreen() {
        var newY = Math.max(textDocumentFlick.contentY - textDocumentFlick.height, 0)
        textDocumentFlick.contentY = newY
    }

    function scrollPreviousPage() {
        var newY = Math.max(textDocumentFlick.contentY - textDocumentFlick.pageHeight, 0)
        textDocumentFlick.contentY = newY
    }

    function scrollDown() {
        var newY = Math.min(textDocumentFlick.contentY + textDocumentFlick.lineHeight, textDocumentFlick.contentHeight-textDocumentFlick.height)
        textDocumentFlick.contentY = newY
    }

    function scrollNextScene() {
        screenplayOffsetsView.currentIndex = Math.min(screenplayOffsetsView.currentIndex+1,screenplayOffsetsView.count-1)
    }

    function scrollNextScreen() {
        var newY = Math.min(textDocumentFlick.contentY + textDocumentFlick.height, textDocumentFlick.contentHeight-textDocumentFlick.height)
        textDocumentFlick.contentY = newY
    }

    function scrollNextPage() {
        var newY = Math.min(textDocumentFlick.contentY + textDocumentFlick.pageHeight)
        textDocumentFlick.contentY = newY
    }

    function toggleTimeOffsetDisplay() {
        screenplayOffsetsView.displayTimeOffset = !screenplayOffsetsView.displayTimeOffset
    }

    Settings {
        id: scritedViewSettings
        fileName: app.settingsFilePath
        category: "Scrited"
        property string lastOpenScritedFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.MoviesLocation)
        property bool experimentalFeatureNoticeDisplayed: false
        property bool codecsNoticeDisplayed: false
    }

    FileDialog {
        id: fileDialog
        folder: scritedViewSettings.lastOpenScritedFolderUrl
        onFolderChanged: Qt.callLater( function() { scritedViewSettings.lastOpenScritedFolderUrl = fileDialog.folder } )
        selectFolder: false
        selectMultiple: false
        selectExisting: true
        onAccepted: {
            mediaPlayer.source = fileUrl
            mediaPlayer.play()
            Qt.callLater( function() {
                mediaPlayer.pause()
                mediaPlayer.seek(0)
                screenplayOffsetsView.adjustTextDocumentAndMedia()
            })
            screenplayOffsetsModel.fileName = screenplayOffsetsModel.fileNameFrom(fileUrl)
        }
    }

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal
        Material.background: Qt.darker(primaryColors.button.background, 1.1)

        Item {
            SplitView.preferredWidth: scritedView.width * 0.50

            SplitView {
                anchors.fill: parent
                orientation: Qt.Vertical

                Rectangle {
                    SplitView.preferredHeight: width / 16 * 9
                    color: "black"

                    MediaPlayer {
                        id: mediaPlayer
                        notifyInterval: 250
                        function togglePlayback() {
                            if(status == MediaPlayer.NoMedia)
                                return

                            if(playbackState === MediaPlayer.PlayingState)
                                pause()
                            else
                                play()
                        }

                        property bool keepScreenplayInSyncWithPosition: false
                        onPositionChanged: {
                            if(keepScreenplayInSyncWithPosition && playbackState === MediaPlayer.PlayingState) {
                                var offsetInfo = screenplayOffsetsModel.offsetInfoAtTime(position, screenplayOffsetsView.currentIndex)
                                if(offsetInfo.row < 0)
                                    return

                                if(screenplayOffsetsView.currentIndex !== offsetInfo.row)
                                    screenplayOffsetsView.currentIndex = offsetInfo.row

                                var newY = screenplayOffsetsModel.evaluatePointAtTime(position, offsetInfo.row).y * textDocumentView.documentScale
                                textDocumentFlick.contentY = newY
                            }
                        }
                    }

                    VideoOutput {
                        id: videoOutput
                        source: mediaPlayer
                        anchors.fill: parent
                        fillMode: VideoOutput.PreserveAspectFit
                    }

                    Image {
                        id: logoOverlay
                        x: videoOutput.contentRect.x + 20
                        y: videoOutput.contentRect.y + 20
                        width: Math.min(videoOutput.width, videoOutput.height)*0.15
                        height: width
                        opacity: 0.25
                        visible: mediaIsLoaded
                        source: "../images/appicon.png"
                        smooth: true
                        fillMode: Image.PreserveAspectFit
                    }

                    Text {
                        text: "scrite.io"
                        font.family: "Courier Prime"
                        font.bold: true
                        font.pointSize: app.idealFontPointSize * 1.5
                        horizontalAlignment: Text.AlignRight
                        anchors.bottom: parent.bottom
                        anchors.right: videoOutput.right
                        anchors.rightMargin: 20
                        anchors.bottomMargin: mediaPlayerControls.visible ? mediaPlayerControls.height + mediaPlayerControls.anchors.bottomMargin + 20 : 20
                        opacity: logoOverlay.opacity * 1.5
                        color: "white"
                        visible: logoOverlay.visible
                    }

                    Text {
                        width: parent.width * 0.75
                        wrapMode: Text.WordWrap
                        font.pointSize: 16
                        horizontalAlignment: Text.AlignHCenter
                        anchors.centerIn: parent
                        color: "white"
                        visible: mediaPlayer.status === MediaPlayer.NoMedia
                        padding: 20
                        text: {
                            if(scriteDocument.screenplay.elementCount > 0)
                                return "Click here to load movie of \"" + scriteDocument.screenplay.title + "\"."
                            return "Load a screenplay and then click here to load its movie for syncing."
                        }

                        MouseArea {
                            anchors.fill: parent
                            enabled: scriteDocument.screenplay.elementCount > 0 && mediaPlayer.status === MediaPlayer.NoMedia
                            onClicked: fileDialog.open()
                            hoverEnabled: true
                            onEntered: parent.font.underline = true
                            onExited: parent.font.underline = false
                        }
                    }

                    Rectangle {
                        id: mediaPlayerControls
                        width: parent.width * 0.9
                        radius: 6
                        height: mediaPlayerControlsLayout.height+2*radius
                        anchors.bottomMargin: 10
                        anchors.bottom: parent.bottom
                        anchors.horizontalCenter: parent.horizontalCenter
                        color: Qt.rgba(0,0,0,0.25)
                        visible: mediaPlayerMouseArea.containsMouse

                        MouseArea {
                            anchors.fill: mediaPlayerControlsLayout
                            onClicked: {
                                var pos = Math.abs((mouse.x/width) * mediaPlayer.duration)
                                mediaPlayer.seek(pos)
                            }
                        }

                        Column {
                            id: mediaPlayerControlsLayout
                            width: parent.width-2*parent.radius
                            spacing: 5
                            anchors.centerIn: parent

                            Item {
                                width: parent.width
                                height: 20
                                enabled: mediaPlayer.status !== MediaPlayer.NoMedia

                                Rectangle {
                                    height: 2
                                    width: parent.width
                                    color: enabled ? "white" : "gray"
                                    anchors.centerIn: parent
                                }

                                Rectangle {
                                    width: 5
                                    height: parent.height
                                    color: enabled ? "white" : "gray"
                                    x: ((mediaPlayer.position / mediaPlayer.duration) * parent.width) - width/2
                                    onXChanged: {
                                        if(positionHandleMouseArea.drag.active) {
                                            var pos = Math.abs(((x + width/2)/parent.width) * mediaPlayer.duration)
                                            mediaPlayer.seek(pos)
                                        }
                                    }

                                    MouseArea {
                                        id: positionHandleMouseArea
                                        anchors.fill: parent
                                        drag.target: parent
                                        drag.axis: Drag.XAxis
                                    }
                                }
                            }

                            RowLayout {
                                width: parent.width

                                ToolButton2 {
                                    icon.source: "../icons/mediaplayer/movie_inverted.png"
                                    onClicked: fileDialog.open()
                                    suggestedHeight: 36
                                    ToolTip.text: "Load a video file for this screenplay."
                                    focusPolicy: Qt.NoFocus
                                    enabled: scriteDocument.screenplay.elementCount > 0
                                }

                                ToolButton2 {
                                    icon.source: {
                                        if(mediaPlayer.playbackState === MediaPlayer.PlayingState)
                                            return "../icons/mediaplayer/pause_inverted.png"
                                        return "../icons/mediaplayer/play_arrow_inverted.png"
                                    }
                                    onClicked: mediaPlayer.togglePlayback()
                                    enabled: mediaPlayer.status !== MediaPlayer.NoMedia
                                    suggestedHeight: 36
                                    ToolTip.text: "Play / Pause"
                                    focusPolicy: Qt.NoFocus
                                }

                                Text {
                                    Layout.fillWidth: true
                                    Layout.alignment: Qt.AlignVCenter
                                    width: parent.width
                                    opacity: mediaPlayer.status !== MediaPlayer.NoMedia ? 1 : 0
                                    enabled: mediaPlayer.status !== MediaPlayer.NoMedia
                                    horizontalAlignment: Text.AlignHCenter
                                    color: "white"
                                    font.pointSize: 16
                                    font.family: "Courier Prime"
                                    text: {
                                        var msToTime = function(ms) {
                                            var secs = Math.round(ms/1000)
                                            var second = secs%60
                                            var minute = ((secs-second)/60)%60
                                            var hour = (secs - minute*60 - second)/3600
                                            if(hour > 0)
                                                return hour + ":" + minute + ":" + second
                                            return minute + ":" + second
                                        }
                                        return msToTime(mediaPlayer.position) + " / " + msToTime(mediaPlayer.duration)
                                    }
                                }

                                ToolButton2 {
                                    icon.source: "../icons/mediaplayer/rewind_10_inverted.png"
                                    enabled: mediaPlayer.status !== MediaPlayer.NoMedia && mediaPlayer.position > 0
                                    suggestedHeight: 36
                                    onClicked: mediaPlayer.seek( Math.max(mediaPlayer.position-skipDuration, 0 ) )
                                    ToolTip.text: "Rewind by " + (skipDuration/1000) + " seconds"
                                    focusPolicy: Qt.NoFocus
                                }

                                ToolButton2 {
                                    icon.source: "../icons/mediaplayer/forward_10_inverted.png"
                                    enabled: mediaPlayer.status !== MediaPlayer.NoMedia && mediaPlayer.position < mediaPlayer.duration
                                    suggestedHeight: 36
                                    onClicked: mediaPlayer.seek( Math.min(mediaPlayer.position+skipDuration, mediaPlayer.duration) )
                                    ToolTip.text: "Forward by " + (skipDuration/1000) + " seconds"
                                    focusPolicy: Qt.NoFocus
                                }
                            }
                        }
                    }

                    EventFilter.active: scriteDocument.screenplay.elementCount > 0
                    EventFilter.acceptHoverEvents: true
                    EventFilter.events: [127,128,129] // [HoverEnter, HoverLeave, HoverMove]
                    EventFilter.onFilter: mediaPlayerControls.visible = event.type === 127 || event.type === 129
                }

                Rectangle {
                    SplitView.fillHeight: true

                    ScreenplayTextDocumentOffsets {
                        id: screenplayOffsetsModel
                        screenplay: scriteDocument.loading ? null : scriteDocument.screenplay
                        format: scriteDocument.loading ? null : scriteDocument.printFormat

                        Notification.title: "Time Offsets Error"
                        Notification.text: errorMessage
                        Notification.active: hasError
                        Notification.autoClose: false
                        Notification.onDismissed: clearErrorMessage()
                    }

                    FontMetrics {
                        id: screenplayFontMetrics
                        font: screenplayOffsetsModel.format.font
                    }

                    Flickable {
                        id: textDocumentFlick
                        anchors.fill: parent
                        contentWidth: width
                        contentHeight: textDocumentView.height + height
                        boundsBehavior: Flickable.StopAtBounds
                        clip: true

                        property real pageHeight: (screenplayOffsetsModel.format.pageLayout.contentRect.height * textDocumentView.documentScale)
                        property real lineHeight: screenplayFontMetrics.lineSpacing * textDocumentView.documentScale
                        property bool containsMouse: false
                        ScrollBar.vertical: ScrollBar {
                            policy: textDocumentFlick.contentHeight > textDocumentFlick.height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
                            minimumSize: 0.1
                            palette {
                                mid: Qt.rgba(0,0,0,0.25)
                                dark: Qt.rgba(0,0,0,0.75)
                            }
                            opacity: textDocumentFlick.containsMouse ? (active ? 1 : 0.2) : 0
                            Behavior on opacity {
                                enabled: screenplayEditorSettings.enableAnimations
                                NumberAnimation { duration: 250 }
                            }
                        }

                        TextDocumentItem {
                            id: textDocumentView
                            width: textDocumentFlick.width
                            document: screenplayOffsetsModel.document
                            documentScale: (textDocumentFlick.width*0.9) / screenplayOffsetsModel.format.pageLayout.contentWidth
                            flickable: textDocumentFlick

                            Image {
                                width: textDocumentView.width
                                height: Math.max(textDocumentFlick.contentHeight, textDocumentFlick.height)
                                fillMode: Image.Tile
                                source: "../images/notebookpage.jpg"
                                z: -1
                                opacity: 0.25
                            }
                        }

                        Behavior on contentY {
                            enabled: mediaIsLoaded && mediaIsPlaying
                            NumberAnimation {
                                id: contentYAnimation
                                duration: mediaPlayer.notifyInterval-50
                            }
                        }

                        onContentYChanged: app.execLater(textDocumentFlick, 100, updateCurrentIndexOnScreenplayOffsetsView)
                        function updateCurrentIndexOnScreenplayOffsetsView() {
                            var offsetInfo = screenplayOffsetsModel.offsetInfoAtPoint(Qt.point(10, contentY/textDocumentView.documentScale))
                            if(offsetInfo.row < 0)
                                return
                            screenplayOffsetsView.currentIndex = offsetInfo.row
                        }

                        ResetOnChange {
                            id: textDocumentFlickInteraction
                            from: true
                            to: false
                            trackChangesOn: textDocumentFlick.contentY
                            delay: mediaPlayer.notifyInterval-50
                        }

                        EventFilter.acceptHoverEvents: true
                        EventFilter.events: [127,128,129] // [HoverEnter, HoverLeave, HoverMove]
                        EventFilter.onFilter: textDocumentFlick.containsMouse = event.type === 127 || event.type === 129
                    }

                    Item {
                        x: textDocumentFlick.width * 0.025
                        anchors.verticalCenter: textDocumentFlick.verticalCenter
                        rotation: -90
                        transformOrigin: Item.Center
                        opacity: 0.1
                        visible: mediaIsLoaded

                        Text {
                            font.family: "Courier Prime"
                            text: "SCRITE"
                            font.pixelSize: textDocumentFlick.width * 0.05 * 0.65
                            font.letterSpacing: 5
                            font.bold: true
                            anchors.centerIn: parent
                        }
                    }

                    Item {
                        x: textDocumentFlick.width - textDocumentFlick.width * 0.025 - (textDocumentFlick.containsMouse ? 20 : 0)
                        anchors.verticalCenter: textDocumentFlick.verticalCenter
                        rotation: 90
                        transformOrigin: Item.Center
                        opacity: 0.1
                        visible: mediaIsLoaded

                        Text {
                            text: "scrite.io"
                            font.family: "Courier Prime"
                            font.pixelSize: textDocumentFlick.width * 0.05 * 0.65
                            font.letterSpacing: 2
                            font.bold: true
                            anchors.centerIn: parent
                        }
                    }
                }
            }
        }

        Rectangle {
            SplitView.fillWidth: true
            color: "white"

            Row {
                id: screenplayOffsesHeading
                width: screenplayOffsetsView.width-(screenplayOffsetsView.scrollBarVisible ? 20 : 1)
                visible: screenplayOffsetsView.count > 0

                Text {
                    padding: 5
                    width: parent.width * 0.1
                    text: "Scene #"
                    font.bold: true
                    font.family: "Courier Prime"
                    font.pointSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    anchors.verticalCenter: parent.verticalCenter
                    clip: true
                }

                Text {
                    padding: 5
                    width: parent.width * (screenplayOffsetsView.displayTimeOffset ? 0.6 : 0.8)
                    font.bold: true
                    text: "Scene Heading"
                    font.family: "Courier Prime"
                    font.pointSize: 16
                    anchors.verticalCenter: parent.verticalCenter
                    clip: true
                }

                Text {
                    padding: 5
                    width: parent.width * 0.1
                    font.bold: true
                    text: "Page #"
                    font.family: "Courier Prime"
                    font.pointSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    anchors.verticalCenter: parent.verticalCenter
                    clip: true
                }

                Text {
                    padding: 5
                    width: parent.width * 0.2
                    font.bold: true
                    text: "Time"
                    font.family: "Courier Prime"
                    font.pointSize: 16
                    horizontalAlignment: Text.AlignRight
                    anchors.verticalCenter: parent.verticalCenter
                    clip: true
                    visible: screenplayOffsetsView.displayTimeOffset
                }
            }

            Rectangle {
                width: parent.width
                height: 1
                anchors.bottom: screenplayOffsesHeading.bottom
                color: primaryColors.borderColor
                visible: screenplayOffsesHeading.visible
            }

            ListView {
                id: screenplayOffsetsView
                anchors.top: screenplayOffsesHeading.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                model: screenplayOffsetsModel
                clip: true
                property bool displayTimeOffset: true
                property bool scrollBarVisible: contentHeight > height

                ScrollBar.vertical: ScrollBar {
                    policy: screenplayOffsetsView.scrollBarVisible ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
                    minimumSize: 0.1
                    palette {
                        mid: Qt.rgba(0,0,0,0.25)
                        dark: Qt.rgba(0,0,0,0.75)
                    }
                    opacity: active ? 1 : 0.2
                    Behavior on opacity {
                        enabled: screenplayEditorSettings.enableAnimations
                        NumberAnimation { duration: 250 }
                    }
                }

                highlightFollowsCurrentItem: true
                highlightMoveDuration: 0
                highlightResizeDuration: 0
                highlight: Rectangle {
                    color: primaryColors.highlight.background
                }
                delegate: Item {
                    // Columns: SceneNr, Heading, PageNumber, Time
                    width: screenplayOffsetsView.width-(screenplayOffsetsView.scrollBarVisible ? 20 : 1)
                    height: 40

                    Row {
                        anchors.fill: parent

                        Text {
                            padding: 5
                            width: parent.width * 0.1
                            text: offsetInfo.sceneNumber
                            horizontalAlignment: Text.AlignHCenter
                            font.family: "Courier Prime"
                            font.pointSize: 14
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            padding: 5
                            width: parent.width * (screenplayOffsetsView.displayTimeOffset ? 0.6 : 0.8)
                            text: offsetInfo.sceneHeading
                            font.family: "Courier Prime"
                            font.pointSize: 14
                            anchors.verticalCenter: parent.verticalCenter
                            elide: Text.ElideMiddle
                        }

                        Text {
                            padding: 5
                            width: parent.width * 0.1
                            text: offsetInfo.pageNumber
                            font.family: "Courier Prime"
                            font.pointSize: 14
                            horizontalAlignment: Text.AlignHCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            padding: 5
                            width: parent.width * 0.2
                            text: offsetInfo.sceneTime.text
                            font.family: "Courier Prime"
                            font.pointSize: 14
                            horizontalAlignment: Text.AlignRight
                            anchors.verticalCenter: parent.verticalCenter
                            visible: screenplayOffsetsView.displayTimeOffset
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            screenplayOffsetsView.currentIndex = index
                            if(mediaIsLoaded && mediaIsPaused)
                                screenplayOffsetsView.adjustTextDocumentAndMedia()
                        }
                    }
                }

                onCountChanged: currentIndex = 0
                onCurrentIndexChanged: {
                    if(!mediaIsPlaying)
                        adjustTextDocumentAndMedia()
                }

                function adjustTextDocumentAndMedia() {
                    var offsetInfo = screenplayOffsetsModel.offsetInfoAt(screenplayOffsetsView.currentIndex)
                    if(!textDocumentFlickInteraction.value)
                        textDocumentFlick.contentY = offsetInfo.pixelOffset * textDocumentView.documentScale
                    mediaPlayer.seek(offsetInfo.sceneTime.timestamp)
                }
            }
        }
    }

    EventFilter.active: !modalDialog.active && !notificationsView.visible
    EventFilter.target: qmlWindow
    EventFilter.events: [6] // KeyPress
    EventFilter.onFilter: {
        var newY = 0
        switch(event.key) {
        case Qt.Key_Space:
            mediaPlayer.togglePlayback()
            break
        case Qt.Key_Up:
            if(event.controlModifier)
                scrollPreviousScene()
            else if(event.shiftModifier)
                scrollPreviousScreen()
            else if(event.altModifier)
                scrollPreviousPage()
            else
                scrollUp()
            break
        case Qt.Key_Down:
            if(event.controlModifier)
                scrollNextScene()
            else if(event.shiftModifier)
                scrollNextScreen()
            else if(event.altModifier)
                scrollNextPage()
            else
                scrollDown()
            break
        case Qt.Key_Left:
            if(event.controlModifier)
                rewind()
            else
                miniRewind()
            break
        case Qt.Key_Right:
            if(event.controlModifier)
                forward()
            else
                miniForward()
            break
        case Qt.Key_T:
            toggleTimeOffsetDisplay()
            break
        case Qt.Key_Plus:
        case Qt.Key_Equal:
            mediaPlayer.keepScreenplayInSyncWithPosition = !mediaPlayer.keepScreenplayInSyncWithPosition
            break
        case Qt.Key_Greater:
        case Qt.Key_Period:
            syncVideoTimeWithScreenplayOffsets(event.controlModifier)
            break
        }
    }

    QtObject {
        Notification.title: "Install Video Codecs"
        Notification.text: {
            if(app.isWindowsPlatform)
                return "Please install video codecs from the free and open-source LAVFilters project to load videos in this tab."
            return "Please install GStreamer codecs to load videos in this tab."
        }
        Notification.active: !scritedViewSettings.codecsNoticeDisplayed && !modalDialog.active && (app.isWindowsPlatform || app.isLinuxPlatform)
        Notification.buttons: app.isWindowsPlatform ? ["Download", "Dismiss"] : ["Learn More", "Dismiss"]
        Notification.onButtonClicked: {
            if(index === 0)
                Qt.openUrlExternally("https://www.scrite.io/index.php/video-codecs/")
            scritedViewSettings.codecsNoticeDisplayed = true
        }
    }
}
