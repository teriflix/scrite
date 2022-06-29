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
import QtMultimedia 5.15
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.15
import Qt.labs.settings 1.0
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

Item {
    id: scritedView

    property int skipDuration: 10
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
        if(!scritedSettings.experimentalFeatureNoticeDisplayed) {
            Scrite.app.execLater(scritedView, 250, function() {
                showInformation({
                    "message": "<strong>Scrited Tab : Study screenplay and film together.</strong><br/><br/>This is an experimental feature. Help us polish it by leaving feedback on the Forum at www.scrite.io. Thank you!",
                })
                scritedSettings.experimentalFeatureNoticeDisplayed = true
            })
        }
        Scrite.user.logActivity1("scrited")
    }
    Component.onDestruction: scritedToolbar.scritedView = null

    function loadMedia() {
        fileDialog.open()
    }

    function togglePlayback() {
        mediaPlayer.togglePlayback()
    }

    function rewind() {
        mediaPlayer.traverse(-skipDuration)
    }

    function forward() {
        mediaPlayer.traverse(+skipDuration)
    }

    function miniRewind() {
        mediaPlayer.traverse(-1)
    }

    function miniForward() {
        mediaPlayer.traverse(1)
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
        screenplayOffsetsView.currentIndex = screenplayOffsetsModel.previousSceneHeadingIndex(screenplayOffsetsView.currentIndex)
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
        screenplayOffsetsView.currentIndex = screenplayOffsetsModel.nextSceneHeadingIndex(screenplayOffsetsView.currentIndex)
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

    property alias currentSceneTimeIsLocked: screenplayOffsetsView.currentSceneTimeIsLocked
    function toggleCurrentSceneTimeLock() {
        screenplayOffsetsModel.toggleSceneTimeLock(screenplayOffsetsView.currentIndex)
    }

    function unlockAllSceneTimes() {
        screenplayOffsetsModel.unlockAllSceneTimes()
    }

    function loadMediaUrl(fileUrl) {
        mediaPlayer.source = fileUrl
        mediaPlayer.play()
        Qt.callLater( function() {
            mediaPlayer.pause()
            mediaPlayer.seek(0)
            screenplayOffsetsView.adjustTextDocumentAndMedia()
        })
        screenplayOffsetsModel.fileName = screenplayOffsetsModel.fileNameFrom(fileUrl)
    }

    Settings {
        id: scritedSettings
        fileName: Scrite.app.settingsFilePath
        category: "Scrited"
        property string lastOpenScritedFolderUrl: "file:///" + StandardPaths.writableLocation(StandardPaths.MoviesLocation)
        property bool experimentalFeatureNoticeDisplayed: false
        property bool codecsNoticeDisplayed: false
        property real playerAreaRatio: 0.5
        property bool videoPlayerVisible: true
    }

    FileDialog {
        id: fileDialog
        folder: scritedSettings.lastOpenScritedFolderUrl
        onFolderChanged: Qt.callLater( function() { scritedSettings.lastOpenScritedFolderUrl = fileDialog.folder } )
        selectFolder: false
        selectMultiple: false
        selectExisting: true
        onAccepted: loadMediaUrl(fileUrl)
        dirUpAction.shortcut: "Ctrl+Shift+U" // The default Ctrl+U interfers with underline
    }

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal
        Material.background: Qt.darker(primaryColors.button.background, 1.1)

        Item {
            id: playerArea
            SplitView.preferredWidth: scritedView.width * scritedSettings.playerAreaRatio
            onWidthChanged: updateScritedSettings()

            function updateScritedSettings() {
                scritedSettings.playerAreaRatio = width / scritedView.width
            }

            property bool keyFrameGrabMode: false
            function grabKeyFrame() {
                keyFrameGrabMode = true
                Scrite.app.execLater(playerArea, 250, function() {
                    var dpi = Scrite.document.formatting.devicePixelRatio
                    playerArea.grabToImage( function(result) {
                        keyFrameImage.source = result.url
                        playerArea.keyFrameGrabMode = false
                    }, Qt.size(playerArea.width*dpi,playerArea.height*dpi))
                })
            }

            Column {
                anchors.fill: parent

                Rectangle {
                    id: videoArea
                    width: parent.width
                    height: width / 16 * 9
                    color: "black"
                    visible: scritedSettings.videoPlayerVisible || mediaIsLoaded

                    MediaPlayer {
                        id: mediaPlayer
                        notifyInterval: 1000

                        property int sceneStartPosition: -1
                        property int sceneEndPosition: -1
                        property bool hasScenePositions: sceneStartPosition >= 0 && sceneEndPosition > 0 && sceneEndPosition > sceneStartPosition
                        property real sceneStartOffset: sceneStartPosition > 0 ? screenplayOffsetsModel.evaluatePointAtTime(sceneStartPosition, -1).y * textDocumentView.documentScale : 0
                        property real sceneEndOffset: sceneEndPosition > 0 ? screenplayOffsetsModel.evaluatePointAtTime(sceneEndPosition, -1).y * textDocumentView.documentScale : 0

                        function togglePlayback() {
                            if(status == MediaPlayer.NoMedia)
                                return

                            if(playbackState === MediaPlayer.PlayingState)
                                pause()
                            else
                                play()
                        }

                        function traverse(secs) {
                            if(secs === 0)
                                return
                            var now = secs > 0 ? Math.ceil(position/1000) : Math.floor(position/1000)
                            var oldPos = position
                            seek( Math.min(Math.max((now+secs)*1000,0),duration) )
                        }

                        property bool keepScreenplayInSyncWithPosition: false

                        onKeepScreenplayInSyncWithPositionChanged: {
                            if(keepScreenplayInSyncWithPosition) {
                                if(hasScenePositions)
                                    startingFrameAnimation.prepare()
                            } else {
                                startingFrameOverlay.visible = false
                                closingFrameAnimation.rollback()
                                sceneStartPosition = -1
                                sceneEndPosition = -1
                            }
                        }

                        onPositionChanged: {
                            if(keepScreenplayInSyncWithPosition && playbackState === MediaPlayer.PlayingState) {
                                if(hasScenePositions && position >= sceneEndPosition)
                                    closingFrameAnimation.start()

                                var offsetInfo = screenplayOffsetsModel.offsetInfoAtTime(position, screenplayOffsetsView.currentIndex)
                                if(offsetInfo.row < 0)
                                    return

                                if(screenplayOffsetsView.currentIndex !== offsetInfo.row)
                                    screenplayOffsetsView.currentIndex = offsetInfo.row

                                var newY = screenplayOffsetsModel.evaluatePointAtTime(position, offsetInfo.row).y * textDocumentView.documentScale
                                var maxNewY = /*hasScenePositions ? (sceneEndOffset-textDocumentFlick.height*0.75) :*/ textDocumentView.height - textDocumentFlick.height
                                textDocumentFlick.contentY = Math.min(newY, maxNewY)
                            }
                        }
                    }

                    VideoOutput {
                        id: videoOutput
                        source: mediaPlayer
                        anchors.fill: parent
                        fillMode: VideoOutput.PreserveAspectCrop
                    }

                    Image {
                        id: logoOverlay
                        x: 20
                        y: 20
                        width: Math.max(Math.min(videoOutput.width, videoOutput.height)*0.10, 80)
                        height: width
                        visible: mediaIsLoaded
                        source: "../images/appicon.png"
                        smooth: true; mipmap: true
                        fillMode: Image.PreserveAspectFit
                    }

                    ToolButton3 {
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.margins: 4
                        iconSource: "../icons/navigation/arrow_up_inverted.png"
                        visible: !mediaIsLoaded
                        enabled: visible
                        opacity: hovered ? 1 : 0.5
                        onClicked: scritedSettings.videoPlayerVisible = false
                        ToolTip.text: "Closes the video player until a video file is loaded."
                    }

                    Image {
                        id: logoOverlay2
                        x: parent.width-width-20
                        y: 20
                        width: height
                        height: logoOverlay.height
                        visible: logoOverlay.visible && imagePath !== ""
                        property string imagePath: StandardPaths.locateFile(StandardPaths.DownloadLocation, "scrited_logo_overlay.png")
                        source: imagePath === "" ? "" : "file:///" + imagePath
                        smooth: true; mipmap: true
                        fillMode: Image.PreserveAspectFit
                    }

                    Rectangle {
                        anchors.fill: urlOverlay
                        anchors.margins: -urlOverlay.height*0.15
                        anchors.leftMargin: -20
                        anchors.rightMargin: -40
                        radius: height/2
                        color: "#5d3689"
                        visible: urlOverlay.visible
                    }

                    Text {
                        id: urlOverlay
                        text: "scrite.io"
                        font.family: "Courier Prime"
                        font.bold: true
                        font.pixelSize: Math.max(24, Math.min(videoOutput.width, videoOutput.height)*0.125 * 0.25)
                        horizontalAlignment: Text.AlignRight
                        anchors.bottom: parent.bottom
                        anchors.right: parent.right
                        anchors.rightMargin: 20
                        anchors.bottomMargin: mediaPlayerControls.visible ? mediaPlayerControls.height + mediaPlayerControls.anchors.bottomMargin + 20 : (20 + logoOverlay.height/2)
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
                            if(Scrite.document.screenplay.elementCount > 0)
                                return "Click here to load movie of \"" + Scrite.document.screenplay.title + "\"."
                            return "Load a screenplay and then click here to load its movie for syncing."
                        }

                        MouseArea {
                            anchors.fill: parent
                            enabled: Scrite.document.screenplay.elementCount > 0 && mediaPlayer.status === MediaPlayer.NoMedia
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
                        visible: !mediaPlayer.keepScreenplayInSyncWithPosition && !playerArea.keyFrameGrabMode

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
                                            mediaPlayer.seek( Math.round(pos/1000)*1000 )
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
                                    enabled: Scrite.document.screenplay.elementCount > 0
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
                                    onClicked: rewind()
                                    ToolTip.text: "Rewind by " + skipDuration + " seconds"
                                    focusPolicy: Qt.NoFocus
                                }

                                ToolButton2 {
                                    icon.source: "../icons/mediaplayer/forward_10_inverted.png"
                                    enabled: mediaPlayer.status !== MediaPlayer.NoMedia && mediaPlayer.position < mediaPlayer.duration
                                    suggestedHeight: 36
                                    onClicked: forward()
                                    ToolTip.text: "Forward by " + skipDuration + " seconds"
                                    focusPolicy: Qt.NoFocus
                                }

                                ToolButton2 {
                                    icon.source: "../icons/navigation/zoom_fit_inverted.png"
                                    enabled: mediaPlayer.status !== MediaPlayer.NoMedia
                                    suggestedHeight: 36
                                    onClicked: {
                                        if(videoOutput.fillMode === VideoOutput.PreserveAspectCrop)
                                            videoOutput.fillMode = VideoOutput.PreserveAspectFit
                                        else
                                            videoOutput.fillMode = VideoOutput.PreserveAspectCrop
                                    }
                                    ToolTip.text: videoOutput.fillMode === VideoOutput.PreserveAspectCrop ? "Fit video" : "Fill video"
                                    focusPolicy: Qt.NoFocus
                                }
                            }
                        }
                    }

                    Item {
                        id: titleCardOverlay
                        anchors.fill: parent
                        visible: false
                        opacity: 0.95

                        Column {
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: mediaPlayerControls.visible ? mediaPlayerControls.height + 20 : 40
                            anchors.left: parent.left
                            anchors.leftMargin: 30

                            Rectangle {
                                color: "#65318f"
                                width: titleText.width
                                height: titleText.height

                                Text {
                                    id: titleText
                                    padding: 8
                                    font.family: "Arial"
                                    font.pointSize: 28
                                    font.bold: true
                                    color: "white"
                                    text: Scrite.document.screenplay.title
                                }
                            }

                            Rectangle {
                                color: "#e665318f"
                                width: subtitleText.width
                                height: subtitleText.height
                                visible: subtitleText.text !== ""

                                Text {
                                    id: subtitleText
                                    padding: 6
                                    font.family: "Arial"
                                    font.pointSize: 20
                                    font.bold: true
                                    color: "white"
                                    text: Scrite.document.screenplay.subtitle
                                }
                            }

                            Rectangle {
                                color: "black"
                                width: authorsText.width
                                height: authorsText.height
                                visible: authorsText.text !== ""

                                Text {
                                    id: authorsText
                                    padding: 6
                                    font.family: "Arial"
                                    font.pointSize: 20
                                    font.bold: true
                                    color: "white"
                                    text: "Written by " + Scrite.document.screenplay.author
                                }
                            }
                        }
                    }
                }

                Item {
                    width: parent.width
                    height: parent.height - (videoArea.visible ? videoArea.height : 0)

                    Component.onCompleted: {
                        Scrite.app.execLater(screenplayOffsetsModel, 100, function() {
                            screenplayOffsetsModel.allowScreenplay = true
                        })
                    }

                    ScreenplayTextDocumentOffsets {
                        id: screenplayOffsetsModel
                        property bool allowScreenplay : false
                        screenplay: allowScreenplay ? (Scrite.document.loading ? null : Scrite.document.screenplay) : null
                        format: Scrite.document.loading ? null : Scrite.document.printFormat

                        Notification.title: "Time Offsets Error"
                        Notification.text: errorMessage
                        Notification.active: hasError
                        Notification.autoClose: false
                        Notification.onDismissed: clearErrorMessage()
                    }

                    FontMetrics {
                        id: screenplayFontMetrics
                        font: screenplayOffsetsModel.format.defaultFont
                    }

                    Item {
                        id: textDocumentArea
                        anchors.fill: parent
                        clip: true
                        property bool containsMouse: false

                        Item {
                            width: parent.width
                            height: Math.max(parent.height, textDocumentView.height+parent.height)
                            x: 0
                            y: -textDocumentFlick.contentY

                            Image {
                                anchors.fill: parent
                                source: "../images/white-paper-texture.jpg"
                                fillMode: Image.TileVertically
                            }
                        }

                        Text {
                            anchors.centerIn: parent
                            text: "scrite.io"
                            font.family: "Courier Prime"
                            font.bold: true
                            rotation: -Math.atan( parent.height/parent.width ) * 180 / Math.PI
                            font.pixelSize: Math.min(parent.width, parent.height) * 0.2
                            font.letterSpacing: font.pixelSize * 0.1
                            opacity: 0.025
                            visible: logoOverlay.visible
                        }

                        Item {
                            id: textDocumentFlickPadding
                            width: parent.width
                            height: videoArea.visible ? textDocumentArea.height*0.35 : textDocumentFlick.lineHeight
                            y: -1

                            Rectangle {
                                anchors.fill: parent
                                visible: videoArea.visible

                                gradient: Gradient {
                                    GradientStop {
                                        position: 0
                                        color: Scrite.app.translucent(primaryColors.c600.background, 1)
                                    }
                                    GradientStop {
                                        position: 0.175
                                        color: Scrite.app.translucent(primaryColors.c600.background, 0.5)
                                    }
                                    GradientStop {
                                        position: 0.35
                                        color: Scrite.app.translucent(primaryColors.c600.background, 0.3)
                                    }
                                    GradientStop {
                                        position: 0.56
                                        color: Scrite.app.translucent(primaryColors.c600.background, 0.1)
                                    }
                                    GradientStop {
                                        position: 0.7
                                        color: Qt.rgba(0,0,0,0)
                                    }
                                    GradientStop {
                                        position: 0.7
                                        color: Qt.rgba(0,0,0,0)
                                    }
                                }
                            }
                        }

                        Flickable {
                            id: textDocumentFlick
                            contentWidth: Math.ceil(width)
                            contentHeight: Math.ceil(textDocumentView.height + height)
                            boundsBehavior: Flickable.StopAtBounds
                            width: parent.width
                            anchors.top: textDocumentFlickPadding.bottom
                            anchors.bottom: parent.bottom
                            FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor

                            property real pageHeight: (screenplayOffsetsModel.format.pageLayout.contentRect.height * textDocumentView.documentScale)
                            property real lineHeight: screenplayFontMetrics.lineSpacing * textDocumentView.documentScale
                            ScrollBar.vertical: textDocumentScrollBar

                            // Looks like this is the only way to get the flickable
                            // to realize that it is scrollable.
                            property bool contentYAdjusted: false
                            onContentHeightChanged: {
                                if(contentYAdjusted)
                                    return

                                const cy = contentY
                                contentY = textDocumentView.height
                                Qt.callLater( (cy) => { contentY = cy }, cy )
                                contentYAdjusted = true
                            }

                            TextDocumentItem {
                                id: textDocumentView
                                width: textDocumentFlick.width
                                document: screenplayOffsetsModel.document
                                documentScale: (textDocumentFlick.width*0.90) / screenplayOffsetsModel.format.pageLayout.contentWidth
                                flickable: textDocumentFlick
                                verticalPadding: textDocumentFlickPadding.height * documentScale

                                Rectangle {
                                    visible: !mediaPlayer.keepScreenplayInSyncWithPosition && mediaIsLoaded && mediaPlayer.sceneStartPosition > 0 && !playerArea.keyFrameGrabMode
                                    width: parent.width
                                    height: 2
                                    color: "green"
                                    x: 0
                                    y: screenplayOffsetsModel.evaluatePointAtTime(mediaPlayer.sceneStartPosition).y * textDocumentView.documentScale - height
                                }

                                Rectangle {
                                    visible: !mediaPlayer.keepScreenplayInSyncWithPosition && mediaIsLoaded && mediaPlayer.sceneEndPosition > 0 && !playerArea.keyFrameGrabMode
                                    width: parent.width
                                    height: 2
                                    color: "red"
                                    x: 0
                                    y: screenplayOffsetsModel.evaluatePointAtTime(mediaPlayer.sceneEndPosition).y * textDocumentView.documentScale + height
                                }

                                Rectangle {
                                    id: textDocumentTimeCursor
                                    width: parent.width
                                    height: 2
                                    color: primaryColors.c500.background
                                    visible: !mediaPlayer.keepScreenplayInSyncWithPosition && mediaIsLoaded && !playerArea.keyFrameGrabMode
                                    x: 0
                                    Behavior on y {
                                        enabled: mediaIsLoaded && mediaIsPlaying
                                        NumberAnimation { duration: mediaPlayer.notifyInterval-50 }
                                    }

                                    TrackerPack {
                                        enabled: textDocumentTimeCursor.visible

                                        TrackProperty {
                                            target: mediaPlayer
                                            property: "position"
                                        }

                                        TrackSignal {
                                            target: screenplayOffsetsModel
                                            signal: "dataChanged(QModelIndex,QModelIndex,QVector<int>)"
                                        }

                                        onTracked: textDocumentTimeCursor.y = screenplayOffsetsModel.evaluatePointAtTime(mediaPlayer.position).y * textDocumentView.documentScale
                                    }
                                }
                            }

                            Behavior on contentY {
                                enabled: mediaIsLoaded && mediaIsPlaying
                                NumberAnimation {
                                    duration: Math.max(mediaPlayer.notifyInterval, 0)
                                }
                            }

                            onContentYChanged: Scrite.app.execLater(textDocumentFlick, 100, updateCurrentIndexOnScreenplayOffsetsView)
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
                        }

                        ScrollBar2 {
                            id: textDocumentScrollBar
                            anchors.top: parent.top
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            orientation: Qt.Vertical
                            flickable: textDocumentFlick
                            visible: !mediaPlayer.keepScreenplayInSyncWithPosition
                        }

                        EventFilter.acceptHoverEvents: true
                        EventFilter.events: [127,128,129] // [HoverEnter, HoverLeave, HoverMove]
                        EventFilter.onFilter: {
                            result.acceptEvent = false
                            result.filter = false
                            textDocumentArea.containsMouse = event.type === 127 || event.type === 129
                        }
                    }

                    ToolButton3 {
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.topMargin: 4
                        anchors.rightMargin: textDocumentScrollBar.width + 4
                        iconSource: "../icons/navigation/arrow_down.png"
                        visible: !videoArea.visible
                        enabled: visible
                        opacity: hovered ? 1 : 0.5
                        onClicked: scritedSettings.videoPlayerVisible = true
                        ToolTip.text: "Shows the video player."
                    }
                }
            }

            SequentialAnimation {
                id: startingFrameAnimation

                function prepare() {
                    startingFrameOverlayContent.opacity = 1
                    startingFrameOverlay.opacity = 1
                    startingFrameOverlay.visible = true
                    mediaPlayer.pause()
                    mediaPlayer.seek(mediaPlayer.sceneStartPosition)
                    var offsetInfo = screenplayOffsetsModel.offsetInfoAtTime(mediaPlayer.sceneStartPosition, screenplayOffsetsView.currentIndex)
                    screenplayOffsetsView.currentIndex = offsetInfo.row
                    textDocumentFlick.contentY = screenplayOffsetsModel.evaluatePointAtTime(mediaPlayer.sceneStartPosition, offsetInfo.row).y * textDocumentView.documentScale
                }

                ScriptAction {
                    script: startingFrameAnimation.prepare()
                }

                PauseAnimation {
                    duration: 250
                }

                ScriptAction {
                    script: keyFrameImage.visible = false
                }

                PauseAnimation {
                    duration: 1500
                }

                ScriptAction {
                    script: {
                        mediaPlayer.seek(mediaPlayer.sceneStartPosition-2000)
                        mediaPlayer.play()
                    }
                }

                NumberAnimation {
                    target: startingFrameOverlayContent
                    property: "opacity"
                    from: 1; to: 0
                    duration: 1500
                }

                NumberAnimation {
                    target: startingFrameOverlay
                    property: "opacity"
                    from: 1; to: 0
                    duration: 1500
                }

                ScriptAction {
                    script: {
                        startingFrameOverlay.visible = false
                        startingFrameOverlayContent.opacity = 1
                        startingFrameOverlay.opacity = 1
                        if(mediaPlayer.sceneEndPosition - mediaPlayer.sceneStartPosition < 30000)
                            startingFrameAnimation.stop()
                    }
                }

                PauseAnimation {
                    duration: 5000
                }

                ScriptAction {
                    script: {
                        titleCardOverlay.opacity = 0
                        titleCardOverlay.visible = true
                    }
                }

                NumberAnimation {
                    target: titleCardOverlay
                    property: "opacity"
                    duration: 1500
                    from: 0; to: 0.95
                }

                PauseAnimation {
                    duration: 5000
                }

                NumberAnimation {
                    target: titleCardOverlay
                    property: "opacity"
                    duration: 1500
                    from: 0.95; to: 0
                }

                ScriptAction {
                    script: {
                        finalFrameImage.prepare()
                        titleCardOverlay.visible = false
                    }
                }
            }

            Rectangle {
                id: startingFrameOverlay
                color: "black"
                anchors.fill: parent
                visible: false

                onVisibleChanged: {
                    if(!visible)
                        keyFrameImage.source = ""
                    keyFrameImage.visible = true
                }

                ColumnLayout {
                    id: startingFrameOverlayContent
                    spacing: startingFrameOverlay.height * 0.025
                    width: parent.width * 0.8
                    height: parent.height * 0.9
                    anchors.centerIn: parent

                    Text {
                        text: "Script » Screen"
                        color: "#f1be41"
                        font.pointSize: closingFrameOverlay.height * 0.025
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Image {
                        id: filmPoster
                        source: "file:///" + Scrite.document.screenplay.coverPagePhoto
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        fillMode: Image.PreserveAspectFit
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Text {
                        font.pointSize: closingFrameOverlay.height * 0.05
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                        color: "white"
                        text: Scrite.document.screenplay.title
                    }

                    Text {
                        font.pointSize: closingFrameOverlay.height * 0.0225
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                        color: "white"
                        opacity: 0.8
                        text: Scrite.document.screenplay.subtitle
                        visible: text !== ""
                    }

                    Text {
                        font.pointSize: closingFrameOverlay.height * 0.0225
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                        color: "white"
                        opacity: 0.8
                        text: {
                            var ret = "Written by " + Scrite.document.screenplay.author
                            if(filmStudioLogo.visible)
                                return ret
                            ret += "<br/><br/><font size=\"-1\">" + Scrite.document.screenplay.contact + "</font>"
                            return ret
                        }
                    }

                    Image {
                        id: filmStudioLogo
                        anchors.horizontalCenter: parent.horizontalCenter
                        Layout.preferredWidth: startingFrameOverlay.height*0.15
                        Layout.preferredHeight: startingFrameOverlay.height*0.15
                        visible: imagePath !== ""
                        property string imagePath: StandardPaths.locateFile(StandardPaths.DownloadLocation, "scrited_logo_overlay.png")
                        source: imagePath === "" ? "" : "file:///" + imagePath
                        mipmap: true
                        fillMode: Image.PreserveAspectFit
                    }
                }

                Image {
                    id: keyFrameImage
                    anchors.fill: parent
                }
            }

            SequentialAnimation {
                id: closingFrameAnimation
                loops: 1
                running: false

                function rollback() {
                    closingFrameOverlay.visible = false
                    closingMediaPlayer.stop()
                    mediaPlayer.volume = 1
                }

                ScriptAction {
                    script: {
                        closingMediaPlayer.stop()
                        closingMediaPlayer.play()
                        closingFrameOverlay.visible = true
                        closingFrameVideo.visible = true
                        closingFrameImage.visible = false
                        finalFrameImage.opacity = 0
                        finalFrameImage.visible = false
                    }
                }

                NumberAnimation {
                    target: mediaPlayer
                    property: "volume"
                    from: 1
                    to: 0.2
                    duration: 3500
                }

                PauseAnimation {
                    duration: 1500
                }

                ScriptAction {
                    script: {
                        mediaPlayer.pause()
                        closingFrameVideo.visible = false
                        closingFrameImage.opacity = 1
                        closingFrameImage.visible = true
                        finalFrameImage.opacity = 0
                        finalFrameImage.visible = true
                    }
                }

                PauseAnimation {
                    duration: 2000
                }

                ParallelAnimation {
                    NumberAnimation {
                        target: closingFrameImage
                        property: "opacity"
                        from: 1; to: 0
                        duration: 750
                    }

                    NumberAnimation {
                        target: finalFrameImage
                        property: "opacity"
                        from: 0; to: 1
                        duration: 750
                    }
                }

                PauseAnimation {
                    duration: 1000
                }
            }

            Rectangle {
                id: closingFrameOverlay
                color: "black"
                anchors.fill: parent
                visible: false

                MediaPlayer {
                    id: closingMediaPlayer
                    autoPlay: false
                    source: "qrc:/misc/scrited_closing_frame_video.mp4"
                }

                VideoOutput {
                    id: closingFrameVideo
                    width: Math.min(parent.width, parent.height)
                    height: width
                    anchors.centerIn: parent
                    source: closingMediaPlayer
                    flushMode: VideoOutput.LastFrame
                    fillMode: VideoOutput.Stretch
                }

                Image {
                    id: closingFrameImage
                    width: Math.min(parent.width, parent.height)
                    height: width
                    anchors.centerIn: parent
                    mipmap: true
                    source: "../images/scrited_closing_frame.png"
                }

                Image {
                    id: finalFrameImage
                    function prepare() {
                        startingFrameOverlay.grabToImage(function(result) {
                            source = result.url
                        }, Qt.size(startingFrameOverlay.width*2,startingFrameOverlay.height*2))
                    }
                    fillMode: Image.PreserveAspectFit
                    width: Math.min(parent.width, parent.height)
                    height: width
                    anchors.centerIn: parent
                    mipmap: true
                }
            }
        }

        Rectangle {
            SplitView.fillWidth: true
            color: primaryColors.c100.background

            Row {
                id: screenplayOffsesHeading
                width: (screenplayOffsetsView.width-x)-(screenplayOffsetsView.scrollBarVisible ? 20 : 1)
                visible: screenplayOffsetsView.count > 0
                x: 40

                Text {
                    padding: 5
                    width: parent.width * 0.1
                    text: "#"
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
                model: screenplayOffsetsModel
                anchors.top: screenplayOffsesHeading.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor
                clip: true
                property bool displayTimeOffset: true
                property bool scrollBarVisible: contentHeight > height
                property bool currentSceneTimeIsLocked: currentItem ? currentItem.locked : false

                ScrollBar.vertical: ScrollBar2 { flickable: screenplayOffsetsView }

                highlightFollowsCurrentItem: true
                highlightMoveDuration: 0
                highlightResizeDuration: 0
                preferredHighlightBegin: 150
                preferredHighlightEnd: height - 1.5*preferredHighlightBegin
                highlightRangeMode: ListView.ApplyRange
                highlight: Rectangle {
                    id: highlighter

                    SequentialAnimation {
                        loops: Animation.Infinite
                        running: true

                        ColorAnimation {
                            target: highlighter
                            property: "color"
                            from: primaryColors.c200.background
                            to: accentColors.c200.background
                            duration: 750
                        }

                        ColorAnimation {
                            target: highlighter
                            property: "color"
                            to: primaryColors.c200.background
                            from: accentColors.c200.background
                            duration: 750
                        }
                    }
                }
                delegate: Rectangle {
                    // Columns: SceneNr, Heading, PageNumber, Time
                    width: screenplayOffsetsView.width-(screenplayOffsetsView.scrollBarVisible ? 20 : 1)
                    height: isSceneItem ? 40 : 30
                    color: {
                        if(isSceneItem)
                            return screenplayOffsetsView.currentIndex === index ? Qt.rgba(0,0,0,0) : primaryColors.c300.background
                        return screenplayOffsetsView.currentIndex === index ? Qt.rgba(0,0,0,0) : primaryColors.c100.background
                    }
                    property bool isSceneItem: arrayItem.type === SceneElement.Heading
                    property bool locked: arrayItem.locked

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            screenplayOffsetsView.currentIndex = index
                            if(mediaIsLoaded && mediaIsPaused)
                                screenplayOffsetsView.adjustTextDocumentAndMedia()
                        }
                    }

                    Item {
                        id: lockIcon
                        width: 40
                        height: parent.height

                        Image {
                            source: arrayItem.locked ? "../icons/action/lock_outline.png" : "../icons/action/lock_open.png"
                            anchors.fill: parent
                            fillMode: Image.PreserveAspectFit
                            anchors.margins: 5
                            opacity: arrayItem.locked ? 1 : 0.1
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: screenplayOffsetsModel.toggleSceneTimeLock(index)
                        }
                    }

                    Row {
                        anchors.left: lockIcon.right
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom

                        Text {
                            padding: 5
                            width: parent.width * 0.1
                            text: isSceneItem ? arrayItem.number : ""
                            horizontalAlignment: Text.AlignHCenter
                            font.family: "Courier Prime"
                            font.pointSize: 14
                            font.bold: isSceneItem
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            padding: 5
                            width: parent.width * (screenplayOffsetsView.displayTimeOffset ? 0.6 : 0.8)
                            text: arrayItem.snippet
                            font.family: "Courier Prime"
                            font.pointSize: 14
                            font.bold: isSceneItem
                            anchors.verticalCenter: parent.verticalCenter
                            elide: Text.ElideMiddle
                        }

                        Text {
                            padding: 5
                            width: parent.width * 0.1
                            text: isSceneItem ? arrayItem.pageNumber : ""
                            font.family: "Courier Prime"
                            font.pointSize: 14
                            font.bold: isSceneItem
                            horizontalAlignment: Text.AlignHCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            padding: 5
                            width: parent.width * 0.2
                            text: screenplayOffsetsModel.timestampToString(arrayItem.timestamp)
                            font.family: "Courier Prime"
                            font.pointSize: 14
                            font.bold: isSceneItem
                            horizontalAlignment: Text.AlignRight
                            anchors.verticalCenter: parent.verticalCenter
                            visible: screenplayOffsetsView.displayTimeOffset
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
                    if(mediaPlayer.status !== MediaPlayer.NoMedia)
                        mediaPlayer.seek(offsetInfo.timestamp)
                }
            }
        }
    }

    EventFilter.active: !modalDialog.active && !notificationsView.visible
    EventFilter.target: Scrite.window
    EventFilter.events: [6] // KeyPress
    EventFilter.onFilter: {
        var newY = 0
        switch(event.key) {
        case Qt.Key_ParenLeft:
            videoArea.height = videoArea.height-1
            break
        case Qt.Key_ParenRight:
            videoArea.height = videoArea.height+1
            break
        case Qt.Key_Asterisk:
            videoArea.height = videoArea.width / 16 * 9
            break
        case Qt.Key_Space:
            if(mediaIsLoaded && mediaIsPaused && mediaPlayer.hasScenePositions && mediaPlayer.keepScreenplayInSyncWithPosition)
                startingFrameAnimation.start()
            else
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
        case Qt.Key_L:
            toggleCurrentSceneTimeLock()
            break
        case Qt.Key_U:
            unlockAllSceneTimes()
            break
        case Qt.Key_S: // For start
            if(mediaIsLoaded) {
                if(mediaPlayer.keepScreenplayInSyncWithPosition && mediaIsPlaying)
                    return
                mediaPlayer.sceneStartPosition = mediaPlayer.position
            }
            break
        case Qt.Key_E: // For end
            if(mediaIsLoaded) {
                if(mediaPlayer.keepScreenplayInSyncWithPosition && mediaIsPlaying)
                    return
                mediaPlayer.sceneEndPosition = mediaPlayer.position
            }
            break
        case Qt.Key_K: // Key frame
            if(mediaIsLoaded) {
                if(mediaPlayer.keepScreenplayInSyncWithPosition && mediaIsPlaying)
                    return
                playerArea.grabKeyFrame()
            }
            break
        case Qt.Key_A:
            if(mediaIsLoaded)
                screenplayOffsetsModel.adjustUnlockedTimes(mediaPlayer.duration)
            break
        }
    }

    QtObject {
        Notification.title: "Install Video Codecs"
        Notification.text: {
            if(Scrite.app.isWindowsPlatform)
                return "Please install video codecs from the free and open-source LAVFilters project to load videos in this tab."
            return "Please install GStreamer codecs to load videos in this tab."
        }
        Notification.active: !scritedSettings.codecsNoticeDisplayed && !modalDialog.active && (Scrite.app.isWindowsPlatform || Scrite.app.isLinuxPlatform)
        Notification.buttons: Scrite.app.isWindowsPlatform ? ["Download", "Dismiss"] : ["Learn More", "Dismiss"]
        Notification.onButtonClicked: {
            if(index === 0)
                Qt.openUrlExternally("https://www.scrite.io/index.php/video-codecs/")
            scritedSettings.codecsNoticeDisplayed = true
        }
    }

    BusyOverlay {
        anchors.fill: parent
        visible: screenplayOffsetsModel.busy
        busyMessage: "Computing offsets & preparing screenplay for continuous scroll ..."
    }

    AttachmentsDropArea2 {
        anchors.fill: parent
        enabled: !mediaIsLoaded && !Scrite.document.empty
        allowedType: Attachments.VideosOnly
        property string droppedFilePath
        property string droppedFileName
        onDropped: loadMediaUrl( Scrite.app.localFileToUrl(attachment.filePath) )
        attachmentNoticeSuffix: "Drop this file to load video."
    }
}
