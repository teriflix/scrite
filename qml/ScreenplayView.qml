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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

Item {
    id: screenplayView
    signal requestEditor()

    clip: true

    property real zoomLevel: 1
    property color dropAreaHighlightColor: "#cfd8dc"
    property string dropAreaKey: "scrite/sceneID"
    property real preferredHeight: screenplayToolsLayout.height
    property bool showNotesIcon: false
    property bool enableDragDrop: !Scrite.document.readOnly

    Connections {
        target: Scrite.document.screenplay
        function onCurrentElementIndexChanged(val) {
            if(!Scrite.document.loading) {
                Scrite.app.execLater(screenplayElementList, 150, function() {
                    if(screenplayElementList.currentIndex === 0)
                        screenplayElementList.positionViewAtBeginning()
                    else if(screenplayElementList.currentIndex === screenplayElementList.count-1)
                        screenplayElementList.positionViewAtEnd()
                    else
                        screenplayElementList.positionViewAtIndex(Scrite.document.screenplay.currentElementIndex, ListView.Contain)
                })
            }
        }

        function onRequestEditorAt(index) {
            screenplayElementList.positionViewAtIndex(index, ListView.Contain)
        }
    }

    Rectangle {
        id: screenplayTools
        z: 1
        color: accentColors.c100.background
        width: screenplayToolsLayout.width+4
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom

        Flow {
            id: screenplayToolsLayout
            spacing: 1
            height: parent.height-5
            anchors.horizontalCenter: parent.horizontalCenter
            flow: Flow.TopToBottom
            layoutDirection: Qt.RightToLeft

            ToolButton3 {
                iconSource: "../icons/content/clear_all.png"
                ToolTip.text: "Clear the screenplay, while retaining the scenes."
                enabled: !Scrite.document.readOnly
                onClicked: {
                    askQuestion({
                        "question": "Are you sure you want to clear the screenplay?",
                        "okButtonText": "Yes",
                        "cancelButtonText": "No",
                        "callback": function(val) {
                            if(val) {
                                screenplayElementList.forceActiveFocus()
                                Scrite.document.screenplay.clearElements()
                            }
                        }
                    }, this)
                }
            }

            ToolButton3 {
                iconSource: "../icons/navigation/zoom_in.png"
                ToolTip.text: "Increase size of blocks in this view."
                onClicked: {
                    zoomLevel = Math.min(zoomLevel * 1.1, 4.0)
                    screenplayElementList.updateCacheBuffer()
                }
                autoRepeat: true
            }

            ToolButton3 {
                iconSource: "../icons/navigation/zoom_out.png"
                ToolTip.text: "Decrease size of blocks in this view."
                onClicked: {
                    zoomLevel = Math.max(zoomLevel * 0.9, screenplayElementList.perElementWidth/screenplayElementList.minimumDelegateWidth)
                    screenplayElementList.updateCacheBuffer()
                }
                autoRepeat: true
            }
        }

        Rectangle {
            width: 1
            height: parent.height
            anchors.right: parent.right
            color: accentColors.borderColor
        }
    }

    DropArea {
        id: mainDropArea
        anchors.fill: parent
        keys: [dropAreaKey]
        enabled: screenplayElementList.count === 0 && enableDragDrop

        onEntered: (drag) => {
                       screenplayElementList.forceActiveFocus()
                       drag.acceptProposedAction()
                   }

        onDropped: (drop) => {
                       dropSceneAt(drop.source, Scrite.document.screenplay.elementCount)
                       drop.acceptProposedAction()
                   }
    }

    Flickable {
        id: screenplayTracksFlick
        anchors.left: screenplayElementList.left
        anchors.top: parent.top
        anchors.topMargin: screenplayTracks.trackCount > 0 ? 2 : 0
        anchors.right: screenplayElementList.right
        height: contentHeight
        contentWidth: screenplayTracksFlickContent.width
        contentHeight: screenplayTracksFlickContent.height
        interactive: false
        contentX: screenplayElementList.contentX - screenplayElementList.originX
        clip: true
        FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor

        EventFilter.events: [EventFilter.Wheel]
        EventFilter.onFilter: {
            EventFilter.forwardEventTo(screenplayElementList)
            result.filter = true
            result.accepted = true
        }

        Item {
            id: screenplayTracksFlickContent
            width: screenplayElementList.contentWidth
            height: screenplayTracks.trackCount * (minimumAppFontMetrics.height + 10)

            Repeater {
                model: screenplayTracks

                Rectangle {
                    readonly property var trackData: modelData
                    width: screenplayTracksFlickContent.width
                    height: minimumAppFontMetrics.height + 8
                    y: index * (minimumAppFontMetrics.height + 10)
                    color: Scrite.app.translucent( border.color, 0.1 )
                    border.color: accentColors.c900.background
                    border.width: 0.5

                    MouseArea {
                        id: trackMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onMouseXChanged: maybeTooltip()
                        onMouseYChanged: maybeTooltip()
                        onContainsMouseChanged: maybeTooltip()

                        function maybeTooltip() {
                            if(containsMouse) {
                                toolTipItem.x = mouseX
                                toolTipItem.y = mouseY
                                toolTipItem.ToolTip.text = "'" + trackData.category + "' Track"
                                toolTipItem.ToolTip.visible = true
                                toolTipItem.source = trackMouseArea
                            } else if(toolTipItem.source === trackMouseArea) {
                                toolTipItem.ToolTip.visible = false
                                toolTipItem.source = null
                            }
                        }
                    }

                    Repeater {
                        model: trackData.tracks

                        Rectangle {
                            readonly property var groupData: trackData.tracks[index]
                            readonly property var groupExtents: screenplayElementList.extents(groupData.startIndex, groupData.endIndex)
                            color: parent.border.color
                            border.color: Scrite.app.translucent(Scrite.app.textColorFor(color), 0.25)
                            border.width: 0.5
                            x: groupExtents.from
                            width: groupExtents.to - groupExtents.from
                            height: parent.height-4
                            anchors.verticalCenter: parent.verticalCenter

                            Text {
                                font: minimumAppFontMetrics.font
                                text: groupData.group
                                width: parent.width-10
                                horizontalAlignment: Text.AlignHCenter
                                anchors.centerIn: parent
                                elide: Text.ElideMiddle
                                color: Scrite.app.textColorFor(parent.color)
                            }

                            MouseArea {
                                id: groupMouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                onMouseXChanged: maybeTooltip()
                                onMouseYChanged: maybeTooltip()
                                onContainsMouseChanged: maybeTooltip()

                                function maybeTooltip() {
                                    if(containsMouse) {
                                        var ttText = "<b>" + trackData.category + " &gt; " + groupData.group + "</b>, "
                                        if(groupData.endIndex === groupData.startIndex)
                                            ttText += "1 Scene"
                                        else
                                            ttText += (1 + groupData.endIndex - groupData.startIndex) + " Scenes"
                                        if(!screenplayTextDocument.paused) {
                                            var from = Scrite.document.screenplay.elementWithIndex(groupData.startIndex)
                                            var to = Scrite.document.screenplay.elementWithIndex(groupData.endIndex)
                                            ttText += ", Length: " + screenplayTextDocument.lengthInTimeAsString(from, to)
                                        }

                                        toolTipItem.x = mouseX + parent.x
                                        toolTipItem.y = mouseY
                                        toolTipItem.ToolTip.text = ttText
                                        toolTipItem.ToolTip.visible = true
                                        toolTipItem.source = groupMouseArea
                                    } else if(toolTipItem.source === groupMouseArea) {
                                        toolTipItem.ToolTip.visible = false
                                        toolTipItem.source = null
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Item {
                id: toolTipItem
                property MouseArea source: null
                ToolTip.delay: 1000
            }
        }
    }

    TrackerPack {
        TrackSignal {
            target: screenplayTracks
            signal: "trackCountChanged()"
        }

        TrackProperty {
            target: Scrite.document.screenplay
            property: "currentElementIndex"
        }

        TrackSignal {
            target: Scrite.document.screenplay
            signal: "elementsChanged()"
        }

        TrackSignal {
            target: Scrite.document.screenplay
            signal: "elementInserted(ScreenplayElement*,int)"
        }

        TrackSignal {
            target: Scrite.document.screenplay
            signal: "elementRemoved(ScreenplayElement*,int)"
        }

        TrackSignal {
            target: Scrite.document.screenplay
            signal: "elementMoved(ScreenplayElement*,int,int)"
        }

        onTracked: screenplayElementList.updateCacheBuffer()
    }

    ListView {
        id: screenplayElementList
        anchors.left: screenplayTools.right
        anchors.right: parent.right
        anchors.top: screenplayTracksFlick.bottom
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0
        anchors.topMargin: screenplayTracks.trackCount > 0 ? 0 : 3
        FlickScrollSpeedControl.factor: workspaceSettings.flickScrollSpeedFactor
        clip: true
        property bool somethingIsBeingDropped: false
        // visible: count > 0 || somethingIsBeingDropped
        model: Scrite.document.loading ? null : Scrite.document.screenplay
        property real minimumDelegateWidth: {
            var treshold = Math.floor(width / 100)
            if(Scrite.document.screenplay.elementCount < treshold)
                return 100

            var pc = Scrite.document.screenplay.maximumParagraphCount
            if(pc < 4)
                return 100
            if(pc >= 5 && pc < 10)
                return 50
            return 34
        }
        Behavior on minimumDelegateWidth {
            enabled: screenplayEditorSettings.enableAnimations
            NumberAnimation { duration: 250 }
        }

        readonly property real breakDelegateWidth: 70
        readonly property real perElementWidth: 2.5
        readonly property real minimumDelegateWidthForTextVisibility: 50
        property bool moveMode: false
        orientation: Qt.Horizontal
        currentIndex: Scrite.document.screenplay.currentElementIndex
        Keys.onRightPressed: Scrite.document.screenplay.currentElementIndex = Scrite.document.screenplay.currentElementIndex+1
        Keys.onLeftPressed: Scrite.document.screenplay.currentElementIndex = Scrite.document.screenplay.currentElementIndex-1
        Keys.onPressed: {
            event.accepted = false
            if(event.key === Qt.Key_Backspace || event.key === Qt.Key_Delete) {
                if(event.isAutoRepeat)
                    return
                if(event.modifiers !== Qt.NoModifier)
                    return
                Qt.callLater(removeCurrentElement)
                event.accepted = true
            }
        }

        function removeCurrentElement() {
            if(Scrite.document.loading)
                return
            var cidx = currentIndex
            if(cidx < 0)
                return
            var celement = Scrite.document.screenplay.elementAt(cidx)
            if(celement)
                Scrite.document.screenplay.removeElement(celement)
        }

        property bool scrollBarRequired: screenplayElementList.width < screenplayElementList.contentWidth
        ScrollBar.horizontal: ScrollBar2 {
            flickable: screenplayElementList
            opacity: 1
        }

        FocusTracker.window: Scrite.window
        FocusTracker.indicator.target: mainUndoStack
        FocusTracker.indicator.property: "timelineEditorActive"

        Transition {
            id: moveAndDisplace
            NumberAnimation { properties: "x,y"; duration: 250 }
        }

        moveDisplaced: moveAndDisplace
        move: moveAndDisplace

        EventFilter.active: Scrite.app.isWindowsPlatform || Scrite.app.isLinuxPlatform
        EventFilter.events: [EventFilter.Wheel]
        EventFilter.onFilter: {
            if(event.delta < 0)
                contentX = Math.min(contentX+20, contentWidth-width)
            else
                contentX = Math.max(contentX-20, 0)
            result.acceptEvent = true
            result.filter = true
        }

        footer: Item {
            property bool highlightAsDropArea: footerDropArea.containsDrag || mainDropArea.containsDrag
            width: 100 // screenplayElementList.minimumDelegateWidth
            height: screenplayElementList.height

            Rectangle {
                width: 5
                height: parent.height
                color: parent.highlightAsDropArea ? dropAreaHighlightColor : Qt.rgba(0,0,0,0)
            }

            Rectangle {
                anchors.fill: parent
                anchors.leftMargin: 7.5
                anchors.rightMargin: 2.5
                anchors.bottomMargin: screenplayElementList.scrollBarRequired ? 20 : 0
                color: primaryColors.button.background
                border.color: primaryColors.borderColor
                border.width: 1
                opacity: parent.highlightAsDropArea ? 0.75 : 0.5
                visible: Scrite.document.structure.elementCount > 0 && enableDragDrop

                Text {
                    anchors.fill: parent
                    anchors.margins: 5
                    font.pointSize: Scrite.app.idealFontPointSize-2
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    wrapMode: Text.WordWrap
                    text: screenplayElementList.count === 0 ? "Drop the first scene here." : "Drop the last scene here."
                }
            }

            DropArea {
                id: footerDropArea
                anchors.fill: parent
                keys: [dropAreaKey]
                enabled: enableDragDrop

                onEntered: (drag) => {
                               screenplayElementList.forceActiveFocus()
                               drag.acceptProposedAction()
                           }

                onDropped: (drop) => {
                               screenplayElementList.footerItem.highlightAsDropArea = false
                               dropSceneAt(drop.source, Scrite.document.screenplay.elementCount)
                               drop.acceptProposedAction()
                           }
            }
        }

        highlight: Item {
            Item {
                anchors.leftMargin: 7.5
                anchors.rightMargin: 2.5
                anchors.bottomMargin: screenplayElementList.scrollBarRequired ? 20 : 10
                anchors.fill: parent

                BoxShadow {
                    anchors.fill: parent
                }
            }
        }
        highlightFollowsCurrentItem: true
        highlightMoveDuration: 0
        highlightResizeDuration: 0
        highlightRangeMode: FocusTracker.hasFocus ? ListView.NoHighlightRange : ListView.ApplyRange
        preferredHighlightEnd: width*0.6
        preferredHighlightBegin: width*0.2

        property bool mutiSelectionMode: false

        QtObject {
            EventFilter.target: Scrite.app
            EventFilter.active: Scrite.document.screenplay.hasSelectedElements
            EventFilter.events: [EventFilter.KeyPress]
            EventFilter.onFilter: (object,event,result) => {
                                      if(event.key === Qt.Key_Escape) {
                                          Scrite.document.screenplay.clearSelection()
                                          result.acceptEvent = true
                                          result.filter = true
                                      }
                                  }
        }

        onCountChanged: updateCacheBuffer()
        function updateCacheBuffer() {
            if(screenplayTracks.trackCount > 0)
                cacheBuffer = Math.max(extents(count-1, count-1).to + 20, contentWidth)
            else
                cacheBuffer = 0
        }

        function extents(startIndex, endIndex) {
            var x = 0;
            var ret = { "from": 0, "to": 0 }
            if(startIndex < 0 || endIndex < 0)
                return ret;

            var idx = -1
            var nrElements = Scrite.document.screenplay.elementCount
            for(var i=0; i<nrElements; i++) {
                var element = Scrite.document.screenplay.elementAt(i)
                if(element.elementType === ScreenplayElement.SceneElementType)
                    ++idx
                if(idx === startIndex)
                    ret.from = x+7.5
                if(element.elementType === ScreenplayElement.BreakElementType)
                    x += breakDelegateWidth
                else {
                    var sceneElementCount = element.scene ? element.scene.elementCount : 1
                    x += Math.max(minimumDelegateWidth, sceneElementCount*perElementWidth*zoomLevel)
                }
                if(idx === endIndex)
                    break
            }
            ret.to = x-2.5
            return ret
        }

        delegate: Item {
            id: elementItemDelegate
            property ScreenplayElement element: screenplayElement
            property bool isBreakElement: element.elementType === ScreenplayElement.BreakElementType
            property bool isEpisodeBreak: isBreakElement && element.breakType === Screenplay.Episode
            property bool active: element.scene ? Scrite.document.screenplay.activeScene === element.scene : false
            property int sceneElementCount: element.scene ? element.scene.elementCount : 1
            property string sceneTitle: {
                var ret = ""
                var escene = element.scene
                if(escene) {
                    var sheading = escene.heading
                    if(sheading.enabled)
                        ret += "[" + element.resolvedSceneNumber + "]: "

                    if(timelineViewSettings.textMode === "HeadingOrTitle") {
                        var selement = escene.structureElement
                        var ntitle = selement.nativeTitle
                        if(ntitle !== "")
                            ret += ntitle
                        else if(selement.stackId === "" || selement.stackLeader) {
                            if(sheading.enabled)
                                ret += sheading.text
                            else
                                ret += "NO SCENE HEADING"
                        }
                    } else
                        ret += escene.title
                } else if(isEpisodeBreak)
                    ret = "EP " + (element.episodeIndex+1)
                else
                    ret = element.breakTitle

                return ret;
            }
            property color sceneColor: colorPalette.background
            property var colorPalette: {
                if(element.scene) {
                    if(Scrite.app.isLightColor(element.scene.color))
                        return { "background": element.scene.color, "text": "black" }
                    return { "background": element.scene.color, "text": "white" }
                }
                if(element.breakType === Screenplay.Episode)
                    return accentColors.c700
                return accentColors.c500
            }

            width: isBreakElement ? screenplayElementList.breakDelegateWidth :
                   Math.max(screenplayElementList.minimumDelegateWidth, sceneElementCount*screenplayElementList.perElementWidth*zoomLevel)
            height: screenplayElementList.height

            Rectangle {
                visible: element.selected
                anchors.fill: elementItemBox
                anchors.margins: -5
                color: accentColors.a700.background
            }

            Loader {
                id: elementItemBox
                anchors.fill: parent
                anchors.leftMargin: 7.5
                anchors.rightMargin: 2.5
                anchors.bottomMargin: screenplayElementList.scrollBarRequired ? 17 : 3
                active: element !== null // && (isBreakElement || element.scene !== null)
                enabled: !delegateDropArea.containsDrag
                sourceComponent: Rectangle {
                    color: element.scene ? Qt.tint(sceneColor, (element.selected || elementItemDelegate.active) ? "#9CFFFFFF" : "#C0FFFFFF") : sceneColor
                    border.color: color === Qt.rgba(1,1,1,1) ? "black" : sceneColor
                    border.width: elementItemDelegate.active ? 2 : 1
                    Behavior on border.width {
                        enabled: screenplayEditorSettings.enableAnimations
                        NumberAnimation { duration: 400 }
                    }

                    Image {
                        id: breakTypeIcon
                        anchors.top: parent.top
                        anchors.margins: 10
                        anchors.horizontalCenter: parent.horizontalCenter
                        visible: isBreakElement
                        source: isEpisodeBreak ? "../icons/content/episode_inverted.png" : "../icons/content/act_inverted.png"
                        width: 24; height: 24
                    }

                    Item {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.bottom: breakTypeIcon.visible ? parent.bottom : dragTriggerButton.top
                        anchors.topMargin: breakTypeIcon.visible ? 0 : (notesIconLoader.active ? 30 : 5)
                        anchors.leftMargin: 5
                        anchors.rightMargin: 5
                        visible: isBreakElement || parent.width > screenplayElementList.minimumDelegateWidthForTextVisibility

                        Text {
                            text: sceneTitle
                            color: element.scene ? Scrite.app.textColorFor(parent.parent.color) : colorPalette.text
                            elide: Text.ElideRight
                            width: parent.width
                            height: parent.height
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            font.bold: isBreakElement || elementItemDelegate.active
                            lineHeight: 1.25
                            font.pointSize: 12
                            transformOrigin: Item.Center
                            anchors.centerIn: parent
                            maximumLineCount: isBreakElement ? 2 : 5
                            verticalAlignment: isBreakElement ? Text.AlignVCenter : Text.AlignTop
                            horizontalAlignment: isBreakElement ? Text.AlignHCenter : Text.AlignLeft
                            font.capitalization: isBreakElement ? Font.AllUppercase : Font.MixedCase
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true // !isBreakElement
                        ToolTip.visible: hoverEnabled && containsMouse
                        function evalToolTipText() {
                            var ret = ""

                            if(isBreakElement) {
                                var idxList = Scrite.document.screenplay.sceneElementsInBreak(elementItemDelegate.element)
                                if(idxList.length === 0)
                                    return "No Scenes"

                                if(idxList.length === 1)
                                    ret = "1 Scene"
                                else
                                    ret = idxList.length + " Scenes"

                                var from = Scrite.document.screenplay.elementAt(idxList[0])
                                var to = Scrite.document.screenplay.elementAt(idxList[idxList.length-1])
                                ret += ", Length: " + screenplayTextDocument.lengthInTimeAsString(from, to)

                                return ret
                            }

                            var pc = elementItemDelegate.element.scene.elementCount
                            ret += pc + " " + (pc > 1 ? "Paragraphs" : "Paragraph")

                            if(!screenplayTextDocument.paused)
                                ret += ", Length: " + screenplayTextDocument.lengthInTimeAsString(elementItemDelegate.element, null)

                            if(parent.width < screenplayElementList.minimumDelegateWidthForTextVisibility) {
                                var str = elementItemDelegate.sceneTitle
                                if(str.length > 140)
                                    str = str.substring(0, 130) + "..."
                                if(str.length > 0)
                                    ret = "(" + ret + ") " + str
                            }

                            return ret
                        }
                        onContainsMouseChanged: {
                            if(containsMouse)
                                ToolTip.text = evalToolTipText()
                        }
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        onPressed: screenplayElementList.forceActiveFocus()
                        onClicked: {
                            if(mouse.button === Qt.RightButton) {
                                if(element.elementType === ScreenplayElement.BreakElementType) {
                                    breakItemMenu.element = element
                                    breakItemMenu.popup(this)
                                } else {
                                    elementItemMenu.element = element
                                    elementItemMenu.popup(this)
                                }

                                Scrite.document.screenplay.currentElementIndex = index
                                requestEditorLater()

                                return
                            }

                            const isControlPressed = mouse.modifiers & Qt.ControlModifier
                            const isShiftPressed = mouse.modifiers & Qt.ShiftModifier
                            screenplayElementList.forceActiveFocus()
                            screenplayElementList.mutiSelectionMode = isControlPressed || isShiftPressed
                            if(isControlPressed)
                                elementItemDelegate.element.toggleSelection()
                            else if(isShiftPressed) {
                                function selectRange() {
                                    const fromIndex = Math.min(Scrite.document.screenplay.currentElementIndex,index)
                                    const toIndex = Math.max(Scrite.document.screenplay.currentElementIndex,index)
                                    if(fromIndex === toIndex)
                                        return
                                    for(var i=fromIndex; i<=toIndex; i++) {
                                        var element = Scrite.document.screenplay.elementAt(i)
                                        if(element.elementType === ScreenplayElement.SceneElementType)
                                            element.selected = true
                                    }
                                }
                                selectRange()
                            } else
                                Scrite.document.screenplay.clearSelection()

                            Scrite.document.screenplay.currentElementIndex = index
                            requestEditorLater()
                        }
                    }

                    // Drag to timeline support
                    Drag.active: dragMouseArea.drag.active
                    Drag.dragType: Drag.Automatic
                    Drag.supportedActions: Qt.MoveAction
                    Drag.hotSpot.x: width/2
                    Drag.hotSpot.y: height/2
                    Drag.source: elementItemDelegate.element
                    Drag.mimeData: {
                        "scrite/sceneID": element.sceneID
                    }
                    Drag.onActiveChanged: {
                        if(!isBreakElement)
                            Scrite.document.screenplay.currentElementIndex = index
                        screenplayElementList.moveMode = Drag.active
                    }

                    Loader {
                        id: notesIconLoader
                        active: showNotesIcon && (elementItemDelegate.element.scene && elementItemDelegate.element.scene.noteCount >= 1)
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.margins: 5
                        width: 24; height: 24
                        opacity: 0.4
                        sourceComponent: Image {
                            source: "../icons/content/bookmark_outline.png"
                        }
                    }

                    SceneTypeImage {
                        anchors.left: parent.left
                        anchors.bottom: parent.bottom
                        anchors.margins: 5
                        width: 24; height: 24
                        opacity: 0.5
                        showTooltip: false
                        visible: !isBreakElement && parent.width > screenplayElementList.minimumDelegateWidthForTextVisibility
                        sceneType: elementItemDelegate.element.scene ? elementItemDelegate.element.scene.type : Scene.Standard
                    }

                    Image {
                        id: dragTriggerButton
                        source: "../icons/action/view_array.png"
                        width: 24; height: 24
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 5
                        anchors.right: parent.right
                        anchors.rightMargin: parent.width > width+10 ? 5 : (parent.width - width)/2
                        opacity: dragMouseArea.containsMouse ? 1 : 0.1
                        scale: dragMouseArea.containsMouse ? 2 : 1
                        visible: !Scrite.document.readOnly && enableDragDrop
                        enabled: visible
                        Behavior on scale {
                            enabled: screenplayEditorSettings.enableAnimations
                            NumberAnimation { duration: 250 }
                        }

                        MouseArea {
                            id: dragMouseArea
                            hoverEnabled: true
                            anchors.fill: parent
                            drag.target: parent
                            cursorShape: Qt.SizeAllCursor
                            enabled: enableDragDrop
                            onPressed: {
                                if(!elementItemDelegate.element.selected)
                                    Scrite.document.screenplay.clearSelection()
                                elementItemDelegate.element.selected = true
                                screenplayElementList.forceActiveFocus()
                                elementItemDelegate.grabToImage(function(result) {
                                    elementItemDelegate.Drag.imageSource = result.url
                                })
                            }
                        }
                    }
                }
            }

            DropArea {
                id: delegateDropArea
                anchors.fill: parent
                keys: [dropAreaKey]
                enabled: !screenplayElement.selected

                onEntered: (drag) => {
                               screenplayElementList.forceActiveFocus()
                               drag.acceptProposedAction()
                           }

                onDropped: (drop) => {
                               dropSceneAt(drop.source, index)
                               drop.acceptProposedAction()
                           }
            }

            Rectangle {
                id: dropAreaIndicator
                width: 5
                height: parent.height
                anchors.left: parent.left

                property bool highlightAsDropArea: delegateDropArea.containsDrag
                color: highlightAsDropArea ? dropAreaHighlightColor : Qt.rgba(0,0,0,0)
            }
        }
    }

    Loader {
        anchors.fill: screenplayElementList
        active: screenplayEditorSettings.enableAnimations && !screenplayElementList.FocusTracker.hasFocus
        sourceComponent: Item {
            id: highlightedItemOverlay

            Image {
                id: highlightBackdrop
                opacity: 0.75*Math.max(scale-1.0,0)
                transformOrigin: Item.Bottom
            }

            ResetOnChange {
                trackChangesOn: screenplayElementList.currentIndex
                from: false
                to: true
                onValueChanged: {
                    if(value) {
                        var ci = screenplayElementList.currentItem
                        ci.grabToImage( function(result) {
                            highlightBackdrop.source = result.url
                            highlightAnimation.running = true
                        }, Qt.size(ci.width*2,ci.height*2))
                    } else
                        highlightAnimation.running = false
                }
            }

            SequentialAnimation {
                id: highlightAnimation
                running: false
                loops: 1

                ScriptAction {
                    script: {
                        screenplayView.clip = false

                        var ci = screenplayElementList.currentItem
                        var cipos = highlightedItemOverlay.mapFromItem(ci,0,0)
                        highlightBackdrop.scale = 1
                        highlightBackdrop.x = cipos.x
                        highlightBackdrop.y = cipos.y
                        highlightBackdrop.width = ci.width
                        highlightBackdrop.height = ci.height
                    }
                }

                NumberAnimation {
                    target: highlightBackdrop
                    property: "scale"
                    from: 1; to: 2
                    duration: 250
                    easing.type: Easing.InBack
                }

                PauseAnimation {
                    duration: 50
                }

                NumberAnimation {
                    target: highlightBackdrop
                    property: "scale"
                    from: 2; to: 1
                    duration: 250
                    easing.type: Easing.InBack
                }

                ScriptAction {
                    script: screenplayView.clip = true
                }
            }
        }
    }


    Menu2 {
        id: breakItemMenu
        property ScreenplayElement element
        onClosed: element = null

        MenuItem2 {
            text: "Remove"
            enabled: !Scrite.document.readOnly
            onClicked: {
                Scrite.document.screenplay.removeElement(breakItemMenu.element)
                breakItemMenu.close()
            }
        }
    }

    Menu2 {
        id: elementItemMenu
        property ScreenplayElement element

        SceneGroup {
            id: elementItemMenuSceneGroup
            structure: Scrite.document.structure
        }

        onAboutToShow: {
            if(element.selected) {
                Scrite.document.screenplay.gatherSelectedScenes(elementItemMenuSceneGroup)
            } else {
                Scrite.document.screenplay.clearSelection()
                element.selected = true
                elementItemMenuSceneGroup.addScene(element.scene)
            }
        }

        onClosed: {
            element = null
            elementItemMenuSceneGroup.clearScenes()
        }

        ColorMenu {
            title: "Color"
            enabled: !Scrite.document.readOnly && elementItemMenu.element
            onMenuItemClicked: {
                for(var i=0; i<elementItemMenuSceneGroup.sceneCount; i++) {
                    elementItemMenuSceneGroup.sceneAt(i).color = color
                }
                elementItemMenu.close()
            }
        }

        MarkSceneAsMenu {
            title: "Mark Scene As"
            scene: elementItemMenu.element ? elementItemMenu.element.scene : null
            enabled: !Scrite.document.readOnly
            onTriggered: {
                for(var i=0; i<elementItemMenuSceneGroup.sceneCount; i++) {
                    elementItemMenuSceneGroup.sceneAt(i).type = scene.type
                }
                elementItemMenu.close()
            }
        }

        StructureGroupsMenu {
            sceneGroup: elementItemMenuSceneGroup
            enabled: !Scrite.document.readOnly
        }

        MenuSeparator { }

        MenuItem2 {
            text: "Remove"
            enabled: !Scrite.document.readOnly
            onClicked: {
                if(elementItemMenuSceneGroup.sceneCount <= 1)
                    Scrite.document.screenplay.removeElement(elementItemMenu.element)
                else
                    Scrite.document.screenplay.removeSelectedElements();
                elementItemMenu.close()
            }
        }
    }

    SequentialAnimation {
        id: dropSceneAnimation

        property var dropSource // must be a QObject subclass
        property int dropIndex

        PauseAnimation { duration: 50 }

        ScriptAction {
            script: {
                const source = dropSceneAnimation.dropSource
                const index = dropSceneAnimation.dropIndex

                dropSceneAnimation.dropSource = null
                dropSceneAnimation.dropIndex = -2

                var sourceType = Scrite.app.typeName(source)

                if(sourceType === "ScreenplayElement") {
                    Scrite.document.screenplay.moveSelectedElements(index)
                    return
                }

                var sceneID = source.id
                if(sceneID.length === 0)
                    return

                var scene = Scrite.document.structure.findElementBySceneID(sceneID)
                if(scene === null)
                    return

                var element = screenplayElementComponent.createObject()
                element.sceneID = sceneID
                Scrite.document.screenplay.insertElementAt(element, index)
                requestEditorLater()
            }
        }
    }

    function dropSceneAt(source, index) {
        if(source === null)
            return

        dropSceneAnimation.dropSource = source
        dropSceneAnimation.dropIndex = index
        dropSceneAnimation.start()
    }

    Component {
        id: screenplayElementComponent

        ScreenplayElement {
            screenplay: Scrite.document.screenplay
        }
    }

    function requestEditorLater() {
        Scrite.app.execLater(screenplayView, 100, function() { requestEditor() })
    }
}
