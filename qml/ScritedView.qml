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

import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/notifications"

Item {
    id: root

    SplitView {
        Material.background: Qt.darker(Runtime.colors.primary.button.background, 1.1)

        anchors.fill: parent
        orientation: Qt.Horizontal

        Item {
            id: _playerArea

            property bool keyFrameGrabMode: false

            SplitView.preferredWidth: root.width * Runtime.scritedSettings.playerAreaRatio

            function updateScritedSettings() {
                Runtime.scritedSettings.playerAreaRatio = width / root.width
            }

            function grabKeyFrame() {
                keyFrameGrabMode = true
                Runtime.execLater(_playerArea, 250, function() {
                    const dpi = Scrite.document.formatting.devicePixelRatio
                    _playerArea.grabToImage( function(result) {
                        _keyFrameImage.source = result.url
                        _playerArea.keyFrameGrabMode = false
                    }, Qt.size(_playerArea.width*dpi,_playerArea.height*dpi))
                })
            }

            onWidthChanged: updateScritedSettings()

            Column {
                anchors.fill: parent

                Rectangle {
                    id: _videoArea

                    property real minHeight: 10
                    property real maxHeight: Math.max(root.height * 0.75, idealHeight)
                    property real idealHeight: width / 16 * 9

                    width: parent.width
                    height: width / 16 * 9

                    color: "black"
                    visible: Runtime.scritedSettings.videoPlayerVisible || _private.mediaIsLoaded

                    MediaPlayer {
                        id: _mediaPlayer

                        property int sceneEndPosition: -1
                        property int sceneStartPosition: -1
                        property bool hasScenePositions: sceneStartPosition >= 0 && sceneEndPosition > 0 && sceneEndPosition > sceneStartPosition
                        property bool keepScreenplayInSyncWithPosition: false
                        property real sceneEndOffset: sceneEndPosition > 0 ? _screenplayOffsetsModel.evaluatePointAtTime(sceneEndPosition, -1).y * _textDocumentView.documentScale : 0
                        property real sceneStartOffset: sceneStartPosition > 0 ? _screenplayOffsetsModel.evaluatePointAtTime(sceneStartPosition, -1).y * _textDocumentView.documentScale : 0

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
                            const now = secs > 0 ? Math.ceil(position/1000) : Math.floor(position/1000)
                            // const oldPos = position
                            seek( Math.min(Math.max((now+secs)*1000,0),duration) )
                        }


                        notifyInterval: 1000

                        onKeepScreenplayInSyncWithPositionChanged: {
                            if(keepScreenplayInSyncWithPosition) {
                                if(hasScenePositions)
                                    _startingFrameAnimation.prepare()
                            } else {
                                _startingFrameOverlay.visible = false
                                _closingFrameAnimation.rollback()
                                sceneStartPosition = -1
                                sceneEndPosition = -1
                            }
                        }

                        onPositionChanged: {
                            if(keepScreenplayInSyncWithPosition && playbackState === MediaPlayer.PlayingState) {
                                if(hasScenePositions && position >= sceneEndPosition)
                                    _closingFrameAnimation.start()

                                const offsetInfo = _screenplayOffsetsModel.offsetInfoAtTime(position, _screenplayOffsetsView.currentIndex)
                                if(offsetInfo.row < 0)
                                    return

                                if(_screenplayOffsetsView.currentIndex !== offsetInfo.row)
                                    _screenplayOffsetsView.currentIndex = offsetInfo.row

                                const newY = _screenplayOffsetsModel.evaluatePointAtTime(position, offsetInfo.row).y * _textDocumentView.documentScale
                                const maxNewY = /*hasScenePositions ? (sceneEndOffset-textDocumentFlick.height*0.75) :*/ _textDocumentView.height - _textDocumentFlick.height
                                _textDocumentFlick.contentY = Math.min(newY, maxNewY)
                            }
                        }
                    }

                    VideoOutput {
                        id: _videoOutput

                        anchors.fill: parent

                        source: _mediaPlayer
                        fillMode: VideoOutput.PreserveAspectCrop
                    }

                    Image {
                        id: _logoOverlay

                        x: 20
                        y: 20
                        width: Math.max(Math.min(_videoOutput.width, _videoOutput.height)*0.10, 80)
                        height: width

                        fillMode: Image.PreserveAspectFit
                        smooth: true; mipmap: true
                        source: "qrc:/images/appicon.png"
                        visible: _private.mediaIsLoaded
                    }

                    FlatToolButton {
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.margins: 4

                        enabled: visible
                        iconSource: "qrc:/icons/navigation/arrow_up_inverted.png"
                        onClicked: Runtime.scritedSettings.videoPlayerVisible = false
                        opacity: hovered ? 1 : 0.5
                        toolTipText: "Closes the video player until a video file is loaded."
                        visible: !_private.mediaIsLoaded
                    }

                    Image {
                        id: _logoOverlay2

                        property string imagePath: StandardPaths.locateFile(StandardPaths.DownloadLocation, "scrited_logo_overlay.png")

                        x: parent.width-width-20
                        y: 20
                        width: height
                        height: _logoOverlay.height

                        fillMode: Image.PreserveAspectFit
                        smooth: true; mipmap: true
                        source: imagePath === "" ? "" : "file:///" + imagePath
                        visible: _logoOverlay.visible && imagePath !== ""
                    }

                    Rectangle {
                        anchors.fill: _urlOverlay
                        anchors.margins: -_urlOverlay.height*0.15
                        anchors.leftMargin: -20
                        anchors.rightMargin: -40

                        color: "#5d3689"
                        radius: height/2
                        visible: _urlOverlay.visible
                    }

                    VclText {
                        id: _urlOverlay

                        anchors.bottom: parent.bottom
                        anchors.right: parent.right
                        anchors.rightMargin: 20
                        anchors.bottomMargin: _mediaPlayerControls.visible ? _mediaPlayerControls.height + _mediaPlayerControls.anchors.bottomMargin + 20 : (20 + _logoOverlay.height/2)

                        color: "white"
                        horizontalAlignment: Text.AlignRight
                        text: "scrite.io"
                        visible: _logoOverlay.visible

                        font.bold: true
                        font.family: "Courier Prime"
                        font.pixelSize: Math.max(24, Math.min(_videoOutput.width, _videoOutput.height)*0.125 * 0.25)
                    }

                    VclLabel {
                        anchors.centerIn: parent

                        width: parent.width * 0.75

                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        padding: 20
                        visible: _mediaPlayer.status === MediaPlayer.NoMedia
                        wrapMode: Text.WordWrap

                        text: {
                            if(Scrite.document.screenplay.elementCount > 0)
                                return "Click here to load movie of \"" + Scrite.document.screenplay.title + "\"."
                            return "Load a screenplay and then click here to load its movie for syncing."
                        }

                        font.pointSize: 16

                        MouseArea {
                            anchors.fill: parent

                            enabled: Scrite.document.screenplay.elementCount > 0 && _mediaPlayer.status === MediaPlayer.NoMedia
                            hoverEnabled: true

                            onClicked: _fileDialog.open()
                            onEntered: parent.font.underline = true
                            onExited: parent.font.underline = false
                        }
                    }

                    Rectangle {
                        id: _mediaPlayerControls

                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 10
                        anchors.horizontalCenter: parent.horizontalCenter

                        width: parent.width * 0.9
                        height: _mediaPlayerControlsLayout.height+2*radius

                        color: Qt.rgba(0,0,0,0.25)
                        radius: 6
                        visible: !_mediaPlayer.keepScreenplayInSyncWithPosition && !_playerArea.keyFrameGrabMode

                        MouseArea {
                            anchors.fill: _mediaPlayerControlsLayout

                            onClicked: {
                                var pos = Math.abs((mouse.x/width) * _mediaPlayer.duration)
                                _mediaPlayer.seek(pos)
                            }
                        }

                        Column {
                            id: _mediaPlayerControlsLayout

                            anchors.centerIn: parent

                            width: parent.width-2*parent.radius
                            spacing: 5

                            Item {
                                width: parent.width
                                height: 20

                                enabled: _mediaPlayer.status !== MediaPlayer.NoMedia

                                Rectangle {
                                    anchors.centerIn: parent

                                    height: 2
                                    width: parent.width

                                    color: enabled ? "white" : "gray"
                                }

                                Rectangle {
                                    x: ((_mediaPlayer.position / _mediaPlayer.duration) * parent.width) - width/2
                                    width: 5
                                    height: parent.height

                                    color: enabled ? "white" : "gray"

                                    onXChanged: {
                                        if(_positionHandleMouseArea.drag.active) {
                                            var pos = Math.abs(((x + width/2)/parent.width) * _mediaPlayer.duration)
                                            _mediaPlayer.seek( Math.round(pos/1000)*1000 )
                                        }
                                    }

                                    MouseArea {
                                        id: _positionHandleMouseArea

                                        anchors.fill: parent

                                        drag.target: parent
                                        drag.axis: Drag.XAxis
                                    }
                                }
                            }

                            RowLayout {
                                width: parent.width

                                VclToolButton {
                                    enabled: Scrite.document.screenplay.elementCount > 0
                                    focusPolicy: Qt.NoFocus
                                    suggestedHeight: 36
                                    toolTipText: "Load a video file for this screenplay."

                                    icon.source: "qrc:/icons/mediaplayer/movie_inverted.png"

                                    onClicked: _fileDialog.open()
                                }

                                VclToolButton {
                                    enabled: _mediaPlayer.status !== MediaPlayer.NoMedia
                                    focusPolicy: Qt.NoFocus
                                    suggestedHeight: 36
                                    toolTipText: "Play / Pause"

                                    icon.source: {
                                        if(_mediaPlayer.playbackState === MediaPlayer.PlayingState)
                                            return "qrc:/icons/mediaplayer/pause_inverted.png"
                                        return "qrc:/icons/mediaplayer/play_arrow_inverted.png"
                                    }

                                    onClicked: _mediaPlayer.togglePlayback()
                                }

                                VclLabel {
                                    Layout.fillWidth: true
                                    Layout.alignment: Qt.AlignVCenter

                                    width: parent.width

                                    color: "white"
                                    enabled: _mediaPlayer.status !== MediaPlayer.NoMedia
                                    horizontalAlignment: Text.AlignHCenter
                                    opacity: _mediaPlayer.status !== MediaPlayer.NoMedia ? 1 : 0

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
                                        return msToTime(_mediaPlayer.position) + " / " + msToTime(_mediaPlayer.duration)
                                    }

                                    font.family: "Courier Prime"
                                    font.pointSize: 16
                                }

                                VclToolButton {
                                    enabled: _mediaPlayer.status !== MediaPlayer.NoMedia && _mediaPlayer.position > 0
                                    focusPolicy: Qt.NoFocus
                                    suggestedHeight: 36
                                    toolTipText: "Rewind by " + _private.skipDuration + " seconds"

                                    icon.source: "qrc:/icons/mediaplayer/rewind_10_inverted.png"

                                    onClicked: _private.rewind()
                                }

                                VclToolButton {
                                    enabled: _mediaPlayer.status !== MediaPlayer.NoMedia && _mediaPlayer.position < _mediaPlayer.duration
                                    focusPolicy: Qt.NoFocus
                                    toolTipText: "Forward by " + _private.skipDuration + " seconds"
                                    suggestedHeight: 36

                                    icon.source: "qrc:/icons/mediaplayer/forward_10_inverted.png"
                                    onClicked: _private.forward()
                                }

                                VclToolButton {
                                    enabled: _mediaPlayer.status !== MediaPlayer.NoMedia
                                    focusPolicy: Qt.NoFocus
                                    suggestedHeight: 36
                                    toolTipText: _videoOutput.fillMode === VideoOutput.PreserveAspectCrop ? "Fit video" : "Fill video"

                                    icon.source: "qrc:/icons/navigation/zoom_fit_inverted.png"

                                    onClicked: {
                                        if(_videoOutput.fillMode === VideoOutput.PreserveAspectCrop)
                                            _videoOutput.fillMode = VideoOutput.PreserveAspectFit
                                        else
                                            _videoOutput.fillMode = VideoOutput.PreserveAspectCrop
                                    }
                                }
                            }
                        }
                    }

                    Item {
                        id: _titleCardOverlay

                        anchors.fill: parent

                        visible: false
                        opacity: 0.95

                        Column {
                            anchors.left: parent.left
                            anchors.bottom: parent.bottom
                            anchors.leftMargin: 30
                            anchors.bottomMargin: _mediaPlayerControls.visible ? _mediaPlayerControls.height + 20 : 40

                            Rectangle {
                                width: _titleText.width
                                height: _titleText.height

                                color: "#65318f"

                                VclLabel {
                                    id: _titleText

                                    color: "white"
                                    padding: 8
                                    text: Scrite.document.screenplay.title

                                    font.bold: true
                                    font.family: "Arial"
                                    font.pointSize: 28
                                }
                            }

                            Rectangle {
                                width: _subtitleText.width
                                height: _subtitleText.height

                                color: "#e665318f"
                                visible: _subtitleText.text !== ""

                                VclLabel {
                                    id: _subtitleText

                                    color: "white"
                                    padding: 6
                                    text: Scrite.document.screenplay.subtitle

                                    font.bold: true
                                    font.family: "Arial"
                                    font.pointSize: 20
                                }
                            }

                            Rectangle {
                                width: _authorsText.width
                                height: _authorsText.height

                                color: "black"
                                visible: _authorsText.text !== ""

                                VclLabel {
                                    id: _authorsText

                                    color: "white"
                                    padding: 6
                                    text: "Written by " + Scrite.document.screenplay.author

                                    font.bold: true
                                    font.family: "Arial"
                                    font.pointSize: 20
                                }
                            }
                        }
                    }
                }

                Item {
                    width: parent.width
                    height: parent.height - (_videoArea.visible ? _videoArea.height : 0)

                    Component.onCompleted: {
                        Runtime.execLater(_screenplayOffsetsModel, 100, function() {
                            _screenplayOffsetsModel.allowScreenplay = true
                        })
                    }

                    ScreenplayTextDocumentOffsets {
                        id: _screenplayOffsetsModel

                        property bool allowScreenplay : false

                        Notification.title: "Time Offsets Error"
                        Notification.text: errorMessage
                        Notification.active: hasError
                        Notification.autoClose: false
                        Notification.onDismissed: clearErrorMessage()

                        format: Scrite.document.loading ? null : Scrite.document.printFormat
                        screenplay: allowScreenplay ? (Scrite.document.loading ? null : Scrite.document.screenplay) : null
                    }

                    FontMetrics {
                        id: _screenplayFontMetrics

                        font: _screenplayOffsetsModel.format.defaultFont
                    }

                    Item {
                        id: _textDocumentArea

                        property bool containsMouse: false

                        EventFilter.acceptHoverEvents: true
                        EventFilter.events: [127,128,129] // [HoverEnter, HoverLeave, HoverMove]
                        EventFilter.onFilter: {
                            result.acceptEvent = false
                            result.filter = false
                            _textDocumentArea.containsMouse = event.type === 127 || event.type === 129
                        }

                        anchors.fill: parent

                        clip: true

                        Item {
                            x: 0
                            y: -_textDocumentFlick.contentY
                            width: parent.width
                            height: Math.max(parent.height, _textDocumentView.height+parent.height)

                            Image {
                                anchors.fill: parent

                                fillMode: Image.TileVertically
                                source: "qrc:/images/white-paper-texture.jpg"
                            }
                        }

                        VclText {
                            anchors.centerIn: parent

                            opacity: 0.025
                            rotation: -Math.atan( parent.height/parent.width ) * 180 / Math.PI
                            text: "scrite.io"
                            visible: _logoOverlay.visible

                            font.bold: true
                            font.family: "Courier Prime"
                            font.pixelSize: Math.min(parent.width, parent.height) * 0.2
                            font.letterSpacing: Math.min(parent.width, parent.height) * 0.2 * 0.1
                        }

                        Item {
                            id: _textDocumentFlickPadding

                            y: -1
                            width: parent.width
                            height: _videoArea.visible ? _textDocumentArea.height*0.35 : _textDocumentFlick.lineHeight

                            Rectangle {
                                anchors.fill: parent
                                visible: _videoArea.visible

                                gradient: Gradient {
                                    GradientStop {
                                        position: 0
                                        color: Color.translucent(Runtime.colors.primary.c600.background, 1)
                                    }
                                    GradientStop {
                                        position: 0.175
                                        color: Color.translucent(Runtime.colors.primary.c600.background, 0.5)
                                    }
                                    GradientStop {
                                        position: 0.35
                                        color: Color.translucent(Runtime.colors.primary.c600.background, 0.3)
                                    }
                                    GradientStop {
                                        position: 0.56
                                        color: Color.translucent(Runtime.colors.primary.c600.background, 0.1)
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
                            id: _textDocumentFlick

                            property real pageHeight: (_screenplayOffsetsModel.format.pageLayout.contentRect.height * _textDocumentView.documentScale)
                            property real lineHeight: _screenplayFontMetrics.lineSpacing * _textDocumentView.documentScale

                            // Looks like this is the only way to get the flickable
                            // to realize that it is scrollable.
                            property bool contentYAdjusted: false

                            function updateCurrentIndexOnScreenplayOffsetsView() {
                                var offsetInfo = _screenplayOffsetsModel.offsetInfoAtPoint(Qt.point(10, contentY/_textDocumentView.documentScale))
                                if(offsetInfo.row < 0)
                                    return
                                _screenplayOffsetsView.currentIndex = offsetInfo.row
                            }

                            FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

                            ScrollBar.vertical: _textDocumentScrollBar

                            anchors.top: _textDocumentFlickPadding.bottom
                            anchors.bottom: parent.bottom

                            contentWidth: Math.ceil(width)
                            contentHeight: Math.ceil(_textDocumentView.height + height)
                            boundsBehavior: Flickable.StopAtBounds
                            width: parent.width

                            onContentHeightChanged: {
                                if(contentYAdjusted)
                                    return

                                const cy = contentY
                                contentY = _textDocumentView.height
                                Qt.callLater( (cy) => { contentY = cy }, cy )
                                contentYAdjusted = true
                            }

                            onContentYChanged: Runtime.execLater(_textDocumentFlick, 100, updateCurrentIndexOnScreenplayOffsetsView)

                            Behavior on contentY {
                                enabled: _private.mediaIsLoaded && _private.mediaIsPlaying

                                NumberAnimation {
                                    duration: Math.max(_mediaPlayer.notifyInterval, 0)
                                }
                            }

                            ResetOnChange {
                                id: _textDocumentFlickInteraction

                                delay: _mediaPlayer.notifyInterval-50
                                from: true
                                to: false
                                trackChangesOn: _textDocumentFlick.contentY
                            }

                            TextDocumentItem {
                                id: _textDocumentView

                                width: _textDocumentFlick.width

                                document: _screenplayOffsetsModel.document
                                documentScale: (_textDocumentFlick.width*0.90) / _screenplayOffsetsModel.format.pageLayout.contentWidth
                                flickable: _textDocumentFlick
                                verticalPadding: _textDocumentFlickPadding.height * documentScale

                                Rectangle {
                                    x: 0
                                    y: _screenplayOffsetsModel.evaluatePointAtTime(_mediaPlayer.sceneStartPosition).y * _textDocumentView.documentScale - height

                                    width: parent.width
                                    height: 2

                                    color: "green"
                                    visible: !_mediaPlayer.keepScreenplayInSyncWithPosition && _private.mediaIsLoaded && _mediaPlayer.sceneStartPosition > 0 && !_playerArea.keyFrameGrabMode
                                }

                                Rectangle {
                                    x: 0
                                    y: _screenplayOffsetsModel.evaluatePointAtTime(_mediaPlayer.sceneEndPosition).y * _textDocumentView.documentScale + height

                                    width: parent.width
                                    height: 2

                                    color: "red"
                                    visible: !_mediaPlayer.keepScreenplayInSyncWithPosition && _private.mediaIsLoaded && _mediaPlayer.sceneEndPosition > 0 && !_playerArea.keyFrameGrabMode
                                }

                                Rectangle {
                                    id: _textDocumentTimeCursor

                                    x: 0

                                    width: parent.width
                                    height: 2

                                    color: Runtime.colors.primary.c500.background
                                    visible: !_mediaPlayer.keepScreenplayInSyncWithPosition && _private.mediaIsLoaded && !_playerArea.keyFrameGrabMode

                                    Behavior on y {
                                        enabled: _private.mediaIsLoaded && _private.mediaIsPlaying
                                        NumberAnimation { duration: _mediaPlayer.notifyInterval-50 }
                                    }

                                    TrackerPack {
                                        enabled: _textDocumentTimeCursor.visible

                                        TrackProperty {
                                            target: _mediaPlayer
                                            property: "position"
                                        }

                                        TrackSignal {
                                            target: _screenplayOffsetsModel
                                            signal: "dataChanged(QModelIndex,QModelIndex,QVector<int>)"
                                        }

                                        onTracked: _textDocumentTimeCursor.y = _screenplayOffsetsModel.evaluatePointAtTime(_mediaPlayer.position).y * _textDocumentView.documentScale
                                    }
                                }
                            }
                        }

                        VclScrollBar {
                            id: _textDocumentScrollBar

                            anchors.top: parent.top
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom

                            flickable: _textDocumentFlick
                            orientation: Qt.Vertical
                            visible: !_mediaPlayer.keepScreenplayInSyncWithPosition
                        }
                    }

                    FlatToolButton {
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.topMargin: 4
                        anchors.rightMargin: _textDocumentScrollBar.width + 4

                        enabled: visible
                        iconSource: "qrc:/icons/navigation/arrow_down.png"
                        opacity: hovered ? 1 : 0.5
                        toolTipText: "Shows the video player."
                        visible: !_videoArea.visible

                        onClicked: Runtime.scritedSettings.videoPlayerVisible = true
                    }
                }
            }

            SequentialAnimation {
                id: _startingFrameAnimation

                function prepare() {
                    _startingFrameOverlayContent.opacity = 1
                    _startingFrameOverlay.opacity = 1
                    _startingFrameOverlay.visible = true
                    _mediaPlayer.pause()
                    _mediaPlayer.seek(_mediaPlayer.sceneStartPosition)
                    var offsetInfo = _screenplayOffsetsModel.offsetInfoAtTime(_mediaPlayer.sceneStartPosition, _screenplayOffsetsView.currentIndex)
                    _screenplayOffsetsView.currentIndex = offsetInfo.row
                    _textDocumentFlick.contentY = _screenplayOffsetsModel.evaluatePointAtTime(_mediaPlayer.sceneStartPosition, offsetInfo.row).y * _textDocumentView.documentScale
                }

                ScriptAction {
                    script: _startingFrameAnimation.prepare()
                }

                PauseAnimation {
                    duration: 250
                }

                ScriptAction {
                    script: _keyFrameImage.visible = false
                }

                PauseAnimation {
                    duration: 1500
                }

                ScriptAction {
                    script: {
                        _mediaPlayer.seek(_mediaPlayer.sceneStartPosition-2000)
                        _mediaPlayer.play()
                    }
                }

                NumberAnimation {
                    target: _startingFrameOverlayContent
                    property: "opacity"
                    from: 1; to: 0
                    duration: 1500
                }

                NumberAnimation {
                    target: _startingFrameOverlay
                    property: "opacity"
                    from: 1; to: 0
                    duration: 1500
                }

                ScriptAction {
                    script: {
                        _startingFrameOverlay.visible = false
                        _startingFrameOverlayContent.opacity = 1
                        _startingFrameOverlay.opacity = 1
                        if(_mediaPlayer.sceneEndPosition - _mediaPlayer.sceneStartPosition < 30000)
                            _startingFrameAnimation.stop()
                    }
                }

                PauseAnimation {
                    duration: 5000
                }

                ScriptAction {
                    script: {
                        _titleCardOverlay.opacity = 0
                        _titleCardOverlay.visible = true
                    }
                }

                NumberAnimation {
                    target: _titleCardOverlay
                    property: "opacity"
                    duration: 1500
                    from: 0; to: 0.95
                }

                PauseAnimation {
                    duration: 5000
                }

                NumberAnimation {
                    target: _titleCardOverlay
                    property: "opacity"
                    duration: 1500
                    from: 0.95; to: 0
                }

                ScriptAction {
                    script: {
                        _finalFrameImage.prepare()
                        _titleCardOverlay.visible = false
                    }
                }
            }

            Rectangle {
                id: _startingFrameOverlay

                anchors.fill: parent

                color: "black"
                visible: false

                onVisibleChanged: {
                    if(!visible)
                        _keyFrameImage.source = ""
                    _keyFrameImage.visible = true
                }

                ColumnLayout {
                    id: _startingFrameOverlayContent

                    anchors.centerIn: parent

                    width: parent.width * 0.8
                    height: parent.height * 0.9

                    spacing: _startingFrameOverlay.height * 0.025

                    VclLabel {
                        Layout.alignment: Qt.AlignHCenter

                        color: "#f1be41"
                        text: "Script » Screen"

                        font.pointSize: _closingFrameOverlay.height * 0.025
                    }

                    Image {
                        id: _filmPoster

                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignHCenter

                        fillMode: Image.PreserveAspectFit
                        source: Scrite.document.screenplay.coverPagePhoto !== "" ? "file:///" + Scrite.document.screenplay.coverPagePhoto : ""
                    }

                    VclLabel {
                        Layout.fillWidth: true

                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        text: Scrite.document.screenplay.title
                        wrapMode: Text.WordWrap

                        font.bold: true
                        font.pointSize: _closingFrameOverlay.height * 0.05
                    }

                    VclLabel {
                        Layout.fillWidth: true

                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        opacity: 0.8
                        text: Scrite.document.screenplay.subtitle
                        visible: text !== ""
                        wrapMode: Text.WordWrap

                        font.pointSize: _closingFrameOverlay.height * 0.0225
                    }

                    VclLabel {
                        Layout.fillWidth: true

                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        opacity: 0.8
                        wrapMode: Text.WordWrap

                        text: {
                            let ret = "Written by " + Scrite.document.screenplay.author
                            if(_filmStudioLogo.visible)
                                return ret
                            ret += "<br/><br/><font size=\"-1\">" + Scrite.document.screenplay.contact + "</font>"
                            return ret
                        }

                        font.pointSize: _closingFrameOverlay.height * 0.0225
                    }

                    Image {
                        id: _filmStudioLogo

                        property string imagePath: StandardPaths.locateFile(StandardPaths.DownloadLocation, "scrited_logo_overlay.png")

                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: _startingFrameOverlay.height*0.15
                        Layout.preferredHeight: _startingFrameOverlay.height*0.15

                        fillMode: Image.PreserveAspectFit
                        mipmap: true
                        source: imagePath === "" ? "" : "file:///" + imagePath
                        visible: imagePath !== ""
                    }
                }

                Image {
                    id: _keyFrameImage

                    anchors.fill: parent
                }
            }

            SequentialAnimation {
                id: _closingFrameAnimation

                loops: 1
                running: false

                function rollback() {
                    _closingFrameOverlay.visible = false
                    _closingMediaPlayer.stop()
                    _mediaPlayer.volume = 1
                }

                ScriptAction {
                    script: {
                        _closingMediaPlayer.stop()
                        _closingMediaPlayer.play()
                        _closingFrameOverlay.visible = true
                        _closingFrameVideo.visible = true
                        _closingFrameImage.visible = false
                        _finalFrameImage.opacity = 0
                        _finalFrameImage.visible = false
                    }
                }

                NumberAnimation {
                    target: _mediaPlayer
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
                        _mediaPlayer.pause()
                        _closingFrameVideo.visible = false
                        _closingFrameImage.opacity = 1
                        _closingFrameImage.visible = true
                        _finalFrameImage.opacity = 0
                        _finalFrameImage.visible = true
                    }
                }

                PauseAnimation {
                    duration: 2000
                }

                ParallelAnimation {
                    NumberAnimation {
                        target: _closingFrameImage
                        property: "opacity"
                        from: 1; to: 0
                        duration: 750
                    }

                    NumberAnimation {
                        target: _finalFrameImage
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
                id: _closingFrameOverlay

                anchors.fill: parent

                color: "black"
                visible: false

                MediaPlayer {
                    id: _closingMediaPlayer

                    autoPlay: false
                    source: "qrc:/misc/scrited_closing_frame_video.mp4"
                }

                VideoOutput {
                    id: _closingFrameVideo

                    anchors.centerIn: parent

                    width: Math.min(parent.width, parent.height)
                    height: width

                    fillMode: VideoOutput.Stretch
                    flushMode: VideoOutput.LastFrame
                    source: _closingMediaPlayer
                }

                Image {
                    id: _closingFrameImage

                    anchors.centerIn: parent

                    width: Math.min(parent.width, parent.height)
                    height: width

                    mipmap: true
                    source: "qrc:/images/scrited_closing_frame.png"
                }

                Image {
                    id: _finalFrameImage

                    function prepare() {
                        _startingFrameOverlay.grabToImage(function(result) {
                            source = result.url
                        }, Qt.size(_startingFrameOverlay.width*2,_startingFrameOverlay.height*2))
                    }

                    anchors.centerIn: parent

                    width: Math.min(parent.width, parent.height)
                    height: width

                    fillMode: Image.PreserveAspectFit
                    mipmap: true
                }
            }
        }

        Rectangle {
            SplitView.fillWidth: true

            color: Runtime.colors.primary.c100.background

            Row {
                id: _screenplayOffsesHeading

                x: 40
                width: (_screenplayOffsetsView.width-x)-(_screenplayOffsetsView.scrollBarVisible ? 20 : 1)

                visible: _screenplayOffsetsView.count > 0

                VclLabel {
                    anchors.verticalCenter: parent.verticalCenter

                    width: parent.width * 0.1

                    clip: true
                    horizontalAlignment: Text.AlignHCenter
                    padding: 5
                    text: "#"

                    font.bold: true
                    font.family: "Courier Prime"
                    font.pointSize: 16
                }

                VclLabel {
                    anchors.verticalCenter: parent.verticalCenter

                    width: parent.width * (_screenplayOffsetsView.displayTimeOffset ? 0.6 : 0.8)

                    clip: true
                    padding: 5
                    text: "Scene Heading"

                    font.bold: true
                    font.family: "Courier Prime"
                    font.pointSize: 16
                }

                VclLabel {
                    anchors.verticalCenter: parent.verticalCenter

                    clip: true
                    horizontalAlignment: Text.AlignHCenter
                    padding: 5
                    text: "Page #"
                    width: parent.width * 0.1

                    font.bold: true
                    font.family: "Courier Prime"
                    font.pointSize: 16
                }

                VclLabel {
                    anchors.verticalCenter: parent.verticalCenter

                    width: parent.width * 0.2

                    clip: true
                    horizontalAlignment: Text.AlignRight
                    padding: 5
                    text: "Time"
                    visible: _screenplayOffsetsView.displayTimeOffset

                    font.bold: true
                    font.family: "Courier Prime"
                    font.pointSize: 16
                }
            }

            Rectangle {
                anchors.bottom: _screenplayOffsesHeading.bottom

                width: parent.width
                height: 1

                color: Runtime.colors.primary.borderColor
                visible: _screenplayOffsesHeading.visible
            }

            ListView {
                id: _screenplayOffsetsView

                property bool displayTimeOffset: true
                property bool scrollBarVisible: contentHeight > height
                property bool currentSceneTimeIsLocked: currentItem ? currentItem.locked : false

                function adjustTextDocumentAndMedia() {
                    var offsetInfo = _screenplayOffsetsModel.offsetInfoAt(_screenplayOffsetsView.currentIndex)
                    if(!_textDocumentFlickInteraction.value)
                        _textDocumentFlick.contentY = offsetInfo.pixelOffset * _textDocumentView.documentScale
                    if(_mediaPlayer.status !== MediaPlayer.NoMedia)
                        _mediaPlayer.seek(offsetInfo.timestamp)
                }

                FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

                ScrollBar.vertical: VclScrollBar { flickable: _screenplayOffsetsView }

                anchors.top: _screenplayOffsesHeading.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                model: _screenplayOffsetsModel
                clip: true

                highlightFollowsCurrentItem: true
                highlightMoveDuration: 0
                highlightResizeDuration: 0
                preferredHighlightBegin: 150
                preferredHighlightEnd: height - 1.5*preferredHighlightBegin
                highlightRangeMode: ListView.ApplyRange
                highlight: Rectangle {
                    id: _highlighter

                    SequentialAnimation {
                        loops: Animation.Infinite
                        running: true

                        ColorAnimation {
                            target: _highlighter
                            property: "color"
                            from: Runtime.colors.primary.c200.background
                            to: Runtime.colors.accent.c200.background
                            duration: 750
                        }

                        ColorAnimation {
                            target: _highlighter
                            property: "color"
                            to: Runtime.colors.primary.c200.background
                            from: Runtime.colors.accent.c200.background
                            duration: 750
                        }
                    }
                }

                delegate: Rectangle {
                    property bool locked: arrayItem.locked
                    property bool isSceneItem: arrayItem.type === SceneElement.Heading

                    // Columns: SceneNr, Heading, PageNumber, Time
                    width: _screenplayOffsetsView.width-(_screenplayOffsetsView.scrollBarVisible ? 20 : 1)
                    height: isSceneItem ? 40 : 30

                    color: {
                        if(isSceneItem)
                            return _screenplayOffsetsView.currentIndex === index ? Qt.rgba(0,0,0,0) : Runtime.colors.primary.c300.background
                        return _screenplayOffsetsView.currentIndex === index ? Qt.rgba(0,0,0,0) : Runtime.colors.primary.c100.background
                    }

                    MouseArea {
                        anchors.fill: parent

                        onClicked: {
                            _screenplayOffsetsView.currentIndex = index
                            if(_private.mediaIsLoaded && _private.mediaIsPaused)
                                _screenplayOffsetsView.adjustTextDocumentAndMedia()
                        }
                    }

                    Item {
                        id: _lockIcon

                        width: 40
                        height: parent.height

                        Image {
                            anchors.fill: parent
                            anchors.margins: 5

                            fillMode: Image.PreserveAspectFit
                            opacity: arrayItem.locked ? 1 : 0.1
                            source: arrayItem.locked ? "qrc:/icons/action/lock_outline.png" : "qrc:/icons/action/lock_open.png"
                        }

                        MouseArea {
                            anchors.fill: parent

                            onClicked: _screenplayOffsetsModel.toggleSceneTimeLock(index)
                        }
                    }

                    Row {
                        anchors.top: parent.top
                        anchors.left: _lockIcon.right
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom

                        VclLabel {
                            anchors.verticalCenter: parent.verticalCenter

                            width: parent.width * 0.1

                            horizontalAlignment: Text.AlignHCenter
                            padding: 5
                            text: isSceneItem ? arrayItem.number : ""

                            font.bold: isSceneItem
                            font.family: "Courier Prime"
                            font.pointSize: 14
                        }

                        VclLabel {
                            anchors.verticalCenter: parent.verticalCenter

                            width: parent.width * (_screenplayOffsetsView.displayTimeOffset ? 0.6 : 0.8)

                            elide: Text.ElideMiddle
                            padding: 5
                            text: arrayItem.snippet

                            font.bold: isSceneItem
                            font.family: "Courier Prime"
                            font.pointSize: 14
                        }

                        VclLabel {
                            anchors.verticalCenter: parent.verticalCenter

                            width: parent.width * 0.1

                            horizontalAlignment: Text.AlignHCenter
                            padding: 5
                            text: isSceneItem ? arrayItem.pageNumber : ""

                            font.bold: isSceneItem
                            font.family: "Courier Prime"
                            font.pointSize: 14
                        }

                        VclLabel {
                            anchors.verticalCenter: parent.verticalCenter

                            width: parent.width * 0.2

                            horizontalAlignment: Text.AlignRight
                            padding: 5
                            text: _screenplayOffsetsModel.timestampToString(arrayItem.timestamp)
                            visible: _screenplayOffsetsView.displayTimeOffset

                            font.bold: isSceneItem
                            font.family: "Courier Prime"
                            font.pointSize: 14
                        }
                    }
                }

                onCountChanged: currentIndex = 0
                onCurrentIndexChanged: {
                    if(!_private.mediaIsPlaying)
                        adjustTextDocumentAndMedia()
                }
            }
        }
    }

    VclFileDialog {
        id: _fileDialog

        folder: Runtime.scritedSettings.lastOpenScritedFolderUrl
        onFolderChanged: Qt.callLater( function() { Runtime.scritedSettings.lastOpenScritedFolderUrl = _fileDialog.folder } )
        selectFolder: false
        selectMultiple: false
        selectExisting: true
        onAccepted: _private.loadMediaUrl(fileUrl)
         // The default Ctrl+U interfers with underline
    }

    BusyMessage {
        visible: _screenplayOffsetsModel.busy

        message: "Computing offsets & preparing screenplay for continuous scroll ..."
    }

    AttachmentsDropArea {
        property string droppedFilePath
        property string droppedFileName

        anchors.fill: parent

        allowedType: Attachments.VideosOnly
        attachmentNoticeSuffix: "Drop this file to load video."
        enabled: !_private.mediaIsLoaded && !Scrite.document.empty

        onDropped: _private.loadMediaUrl( Url.fromPath(attachment.filePath) )
    }

    ActionHandler {
        action: ActionHub.scritedOptions.find("loadMovie")
        enabled: _private.screenplaySplitsCount > 0

        onTriggered: _private.loadMedia()
    }

    ActionHandler {
        action: ActionHub.scritedOptions.find("togglePlayback")
        enabled: _private.mediaIsLoaded

        onTriggered: {
            if(_private.mediaIsLoaded && _private.mediaIsPaused && _mediaPlayer.hasScenePositions && _mediaPlayer.keepScreenplayInSyncWithPosition)
                _startingFrameAnimation.start()
            else
                _mediaPlayer.togglePlayback()
        }
    }

    ActionHandler {
        action: ActionHub.scritedOptions.find("rewind10")
        enabled: _private.mediaIsLoaded

        onTriggered: _private.rewind()
    }

    ActionHandler {
        action: ActionHub.scritedOptions.find("rewind1")
        enabled: _private.mediaIsLoaded

        onTriggered: _private.miniRewind()
    }

    ActionHandler {
        action: ActionHub.scritedOptions.find("forward1")
        enabled: _private.mediaIsLoaded

        onTriggered: _private.miniForward()
    }

    ActionHandler {
        action: ActionHub.scritedOptions.find("forward10")
        enabled: _private.mediaIsLoaded

        onTriggered: _private.forward()
    }

    ActionHandler {
        action: ActionHub.scritedOptions.find("previousScene")
        enabled: _private.previousSceneAvailable

        onTriggered: _private.scrollPreviousScene()
    }

    ActionHandler {
        action: ActionHub.scritedOptions.find("nextScene")
        enabled: _private.nextSceneAvailable

        onTriggered: _private.scrollNextScene()
    }

    ActionHandler {
        action: ActionHub.scritedOptions.find("scrollUp")
        enabled: _private.canScrollUp

        onTriggered: _private.scrollUp()
    }

    ActionHandler {
        action: ActionHub.scritedOptions.find("scrollDown")
        enabled: _private.canScrollDown

        onTriggered: _private.scrollDown()
    }

    ActionHandler {
        action: ActionHub.scritedOptions.find("previousPage")
        enabled: _private.canScrollUp

        onTriggered: _private.scrollPreviousPage()
    }

    ActionHandler {
        action: ActionHub.scritedOptions.find("nextPage")
        enabled: _private.canScrollDown

        onTriggered: _private.scrollNextPage()
    }

    ActionHandler {
        action: ActionHub.scritedOptions.find("previousScreen")
        enabled: _private.canScrollUp

        onTriggered: _private.scrollPreviousScreen()
    }

    ActionHandler {
        action: ActionHub.scritedOptions.find("nextScreen")
        enabled: _private.canScrollDown

        onTriggered: _private.scrollNextScreen()
    }

    ActionHandler {
        action: ActionHub.scritedOptions.find("syncTime")
        enabled: _private.screenplaySplitsCount > 0 && _private.mediaIsLoaded

        onTriggered: _private.syncVideoTimeWithScreenplayOffsets(true)
    }

    ActionHandler {
        action: ActionHub.scritedOptions.find("noteOffset")
        enabled: _private.screenplaySplitsCount > 0 && _private.mediaIsLoaded

        onTriggered: _private.syncVideoTimeWithScreenplayOffsets(false)
    }

    ActionHandler {
        action: ActionHub.scritedOptions.find("adjustOffsets")
        enabled: _private.screenplaySplitsCount > 0 && _private.mediaIsLoaded

        onTriggered: _private.syncVideoTimeWithScreenplayOffsets(true)
    }

    ActionHandler {
        action: ActionHub.scritedOptions.find("resetOffsets")
        enabled: _private.screenplaySplitsCount > 0 && _private.mediaIsLoaded

        onTriggered: _private.resetScreenplayOffsets()
    }

    ActionHandler {
        action: ActionHub.scritedOptions.find("toggleTimeColumn")
        checked: _screenplayOffsetsView.displayTimeOffset
        enabled: _private.screenplaySplitsCount > 0

        onToggled: _private.toggleTimeOffsetDisplay()
    }

    ActionHandler {
        action: ActionHub.scritedOptions.find("autoScroll")
        checked: _private.playbackScreenplaySync
        enabled: _private.screenplaySplitsCount > 0

        onToggled: _private.playbackScreenplaySync = !_private.playbackScreenplaySync
    }

    ActionHandler {
        action: ActionHub.scritedOptions.find("toggleSceneTimeLock")
        enabled: _private.mediaIsLoaded

        onTriggered: _private.toggleCurrentSceneTimeLock()
    }

    ActionHandler {
        action: ActionHub.scritedOptions.find("unlockAllSceneTimes")
        enabled: _private.mediaIsLoaded

        onTriggered: _private.unlockAllSceneTimes()
    }

    ActionHandler {
        action: ActionHub.scritedOptions.find("markStart")
        enabled: _private.mediaIsLoaded && !(_mediaPlayer.keepScreenplayInSyncWithPosition && _private.mediaIsPlaying)

        onTriggered: _mediaPlayer.sceneStartPosition = _mediaPlayer.position
    }

    ActionHandler {
        action: ActionHub.scritedOptions.find("markEnd")
        enabled: _private.mediaIsLoaded && !(_mediaPlayer.keepScreenplayInSyncWithPosition && _private.mediaIsPlaying)

        onTriggered: _mediaPlayer.sceneEndPosition = _mediaPlayer.position
    }

    ActionHandler {
        action: ActionHub.scritedOptions.find("markKeyFrame")
        enabled: _private.mediaIsLoaded && !(_mediaPlayer.keepScreenplayInSyncWithPosition && _private.mediaIsPlaying)

        onTriggered: _playerArea.grabKeyFrame()
    }

    ActionHandler {
        action: ActionHub.scritedOptions.find("adjustUnlockedTimes")
        enabled: _private.mediaIsLoaded

        onTriggered: _screenplayOffsetsModel.adjustUnlockedTimes(_mediaPlayer.duration)
    }

    ActionHandler {
        action: ActionHub.scritedOptions.find("decreaseVideoHeight")
        enabled: _videoArea.height > _videoArea.minHeight

        onTriggered: _videoArea.height = _videoArea.height-1
    }

    ActionHandler {
        action: ActionHub.scritedOptions.find("increaseVideoHeight")
        enabled: _videoArea.height < _videoArea.maxHeight

        onTriggered: _videoArea.height = _videoArea.height+1
    }

    ActionHandler {
        action: ActionHub.scritedOptions.find("resetVideoHeight")
        enabled: _videoArea.height != _videoArea.idealHeight

        onTriggered: _videoArea.height = _videoArea.idealHeight
    }

    QtObject {
        id: _private

        readonly property int skipDuration: 10

        property bool canScrollDown: _textDocumentFlick.contentY < _textDocumentFlick.contentHeight - _textDocumentFlick.height
        property bool canScrollUp: _textDocumentFlick.contentY > 0
        property bool mediaIsLoaded: _mediaPlayer.status !== MediaPlayer.NoMedia
        property bool mediaIsPaused: _mediaPlayer.playbackState === MediaPlayer.PausedState
        property bool mediaIsPlaying: _mediaPlayer.playbackState === MediaPlayer.PlayingState
        property bool nextSceneAvailable: _screenplayOffsetsView.currentIndex+1 < _screenplayOffsetsView.count
        property bool previousSceneAvailable: _screenplayOffsetsView.currentIndex > 0

        property alias currentSceneTimeIsLocked: _screenplayOffsetsView.currentSceneTimeIsLocked
        property alias playbackScreenplaySync: _mediaPlayer.keepScreenplayInSyncWithPosition
        property alias screenplaySplitsCount: _screenplayOffsetsView.count
        property alias timeOffsetVisible: _screenplayOffsetsView.displayTimeOffset

        Component.onCompleted: {
            if(!Runtime.scritedSettings.experimentalFeatureNoticeDisplayed) {
                Runtime.execLater(root, 250, function() {
                    MessageBox.information("Experimental Feature",
                        "<strong>Scrited Tab : Study screenplay and film together.</strong><br/><br/>This is an experimental feature. Help us polish it by leaving feedback on the Forum at www.scrite.io. Thank you!"
                    )
                    Runtime.scritedSettings.experimentalFeatureNoticeDisplayed = true
                })
            }
            Scrite.user.logActivity1("scrited")
        }

        Notification.title: "Install Video Codecs"
        Notification.text: {
            if(Platform.isWindowsDesktop)
                return "Please install video codecs from the free and open-source LAVFilters project to load videos in this tab."
            return "Please install GStreamer codecs to load videos in this tab."
        }
        Notification.active: !Runtime.scritedSettings.codecsNoticeDisplayed && (Platform.isWindowsDesktop || Platform.isLinuxDesktop)
        Notification.buttons: Platform.isWindowsDesktop ? ["Download", "Dismiss"] : ["Learn More", "Dismiss"]
        Notification.onButtonClicked: {
            if(index === 0)
                Qt.openUrlExternally("https://www.scrite.io/video-codecs/")
            Runtime.scritedSettings.codecsNoticeDisplayed = true
        }

        function loadMedia() {
            _fileDialog.open()
        }

        function togglePlayback() {
            _mediaPlayer.togglePlayback()
        }

        function rewind() {
            _mediaPlayer.traverse(-_private.skipDuration)
        }

        function forward() {
            _mediaPlayer.traverse(+_private.skipDuration)
        }

        function miniRewind() {
            _mediaPlayer.traverse(-1)
        }

        function miniForward() {
            _mediaPlayer.traverse(1)
        }

        function syncVideoTimeWithScreenplayOffsets(adjustFollowingRows) {
            _screenplayOffsetsModel.setTime(_screenplayOffsetsView.currentIndex, _mediaPlayer.position, adjustFollowingRows === true)
        }

        function resetScreenplayOffsets() {
            _screenplayOffsetsModel.resetAllTimes()
        }

        function scrollUp() {
            var newY = Math.max(_textDocumentFlick.contentY - _textDocumentFlick.lineHeight, 0)
            _textDocumentFlick.contentY = newY
        }

        function scrollPreviousScene() {
            _screenplayOffsetsView.currentIndex = _screenplayOffsetsModel.previousSceneHeadingIndex(_screenplayOffsetsView.currentIndex)
        }

        function scrollPreviousScreen() {
            var newY = Math.max(_textDocumentFlick.contentY - _textDocumentFlick.height, 0)
            _textDocumentFlick.contentY = newY
        }

        function scrollPreviousPage() {
            var newY = Math.max(_textDocumentFlick.contentY - _textDocumentFlick.pageHeight, 0)
            _textDocumentFlick.contentY = newY
        }

        function scrollDown() {
            var newY = Math.min(_textDocumentFlick.contentY + _textDocumentFlick.lineHeight, _textDocumentFlick.contentHeight-_textDocumentFlick.height)
            _textDocumentFlick.contentY = newY
        }

        function scrollNextScene() {
            _screenplayOffsetsView.currentIndex = _screenplayOffsetsModel.nextSceneHeadingIndex(_screenplayOffsetsView.currentIndex)
        }

        function scrollNextScreen() {
            var newY = Math.min(_textDocumentFlick.contentY + _textDocumentFlick.height, _textDocumentFlick.contentHeight-_textDocumentFlick.height)
            _textDocumentFlick.contentY = newY
        }

        function scrollNextPage() {
            var newY = Math.min(_textDocumentFlick.contentY + _textDocumentFlick.pageHeight)
            _textDocumentFlick.contentY = newY
        }

        function toggleTimeOffsetDisplay() {
            _screenplayOffsetsView.displayTimeOffset = !_screenplayOffsetsView.displayTimeOffset
        }

        function toggleCurrentSceneTimeLock() {
            _screenplayOffsetsModel.toggleSceneTimeLock(_screenplayOffsetsView.currentIndex)
        }

        function unlockAllSceneTimes() {
            _screenplayOffsetsModel.unlockAllSceneTimes()
        }

        function loadMediaUrl(fileUrl) {
            _mediaPlayer.source = fileUrl
            _mediaPlayer.play()
            Qt.callLater( function() {
                _mediaPlayer.pause()
                _mediaPlayer.seek(0)
                _screenplayOffsetsView.adjustTextDocumentAndMedia()
            })
            _screenplayOffsetsModel.fileName = _screenplayOffsetsModel.fileNameFrom(fileUrl)
        }
    }
}
