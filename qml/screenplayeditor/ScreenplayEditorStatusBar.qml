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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

Rectangle {
    id: root

    required property int screenplayEditorLastItemIndex
    required property int screenplayEditorFirstItemIndex

    required property var pageMargins

    required property ListView screenplayEditorListView
    required property FontMetrics sceneHeadingFontMetrics

    readonly property alias zoomLevel: _zoomSlider.zoomLevel
    readonly property alias zoomSlider: _zoomSlider

    property int zoomLevelModifier: 0

    property ScreenplayFormat screenplayFormat: Scrite.document.displayFormat

    signal zoomLevelJustChanged()
    signal zoomLevelIsAboutToChange()

    function zoomLevelModifierToApply() {
        return _zoomSlider.zoomLevelModifierToApply()
    }

    height: Math.max(_metricsDisplay.height, _taggingOptions.height, _zoomSlider.height) + 8

    color: Runtime.colors.primary.windowColor
    border.width: 1
    border.color: Runtime.colors.primary.borderColor

    clip: true
    enabled: (width > (_metricsDisplay.width + _zoomSlider.width + 40))
    opacity: enabled ? 1 : 0

    Item {
        anchors.fill: _metricsDisplay

        ToolTipPopup {
            text: "Page count and time estimates are approximate, assuming " + Scrite.document.printFormat.secondsPerPage + "(s) per page."
            visible: _metricsDisplayOverlayMouseArea.containsMouse
        }

        MouseArea {
            id: _metricsDisplayOverlayMouseArea

            anchors.fill: parent

            hoverEnabled: true
        }
    }

    RowLayout {
        id: _metricsDisplay

        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.verticalCenter: parent.verticalCenter

        spacing: 6

        IconButton {
            readonly property Action shieldAction: ActionHub.fileOperations.find("shield")

            enabled: !Scrite.document.readOnly
            tooltipText: shieldAction.tooltip
            source: shieldAction.icon.source

            onClicked: shieldAction.trigger()
        }

        IconButton {
            source: "qrc:/icons/navigation/refresh.png"
            visible: Runtime.mainWindowTab === Runtime.MainWindowTab.ScreenplayTab
            tooltipText: "Computes page layout from scratch, thereby reevaluating page count and time."

            onClicked: Runtime.paginator.reset()
        }

        Separator {
            visible: Runtime.mainWindowTab === Runtime.MainWindowTab.ScreenplayTab
        }

        Image {
            Layout.preferredWidth: Runtime.idealFontMetrics.height
            Layout.preferredHeight: Runtime.idealFontMetrics.height

            source: "qrc:/icons/content/page_count.png"
            enabled: !Runtime.paginator.paused
            opacity: enabled ? 1 : 0.5
        }

        VclText {
            text: {
                if(Runtime.paginator.paused)
                    return ""

                if(Runtime.paginator.cursorPosition >= 0)
                    return Runtime.paginator.cursorPage + " of " + Runtime.paginator.pageCount

                return Runtime.paginator.pageCount
            }
            opacity: Runtime.paginator.paused ? 0.5 : 1
            font.pointSize: Runtime.minimumFontMetrics.font.pointSize
        }

        Separator { }


        Item {
            Layout.preferredWidth: Runtime.idealFontMetrics.height
            Layout.preferredHeight: Runtime.idealFontMetrics.height

            BusyIcon {
                anchors.fill: parent

                visible: Runtime.paginator.busy
            }

            Image {
                anchors.fill: parent

                enabled: !Runtime.paginator.paused
                opacity: enabled ? 1 : 0.5
                source: "qrc:/icons/content/time.png"
                visible: !Runtime.paginator.busy
            }
        }

        VclText {
            text: {
                if(Runtime.paginator.paused)
                    return ""

                if(Runtime.paginator.cursorPosition >= 0)
                    return TMath.timeLengthString(Runtime.paginator.cursorTime) + " of " + TMath.timeLengthString(Runtime.paginator.totalTime)

                return TMath.timeLengthString(Runtime.paginator.totalTime)
            }
            opacity: Runtime.paginator.paused ? 0.5 : 1
            font.pointSize: Runtime.minimumFontMetrics.font.pointSize
        }

        Separator {
            visible: _wordCountLabel.visible
        }

        VclText {
            id: _wordCountLabel

            visible: _taggingOptionsPosMapper.mappedPosition.x > width

            text: {
                const currentScene = Runtime.screenplayAdapter.currentScene
                const currentSceneWordCount = currentScene ? currentScene.wordCount + " / " : ""
                const totalWordCount = Runtime.screenplayAdapter.wordCount + (Runtime.screenplayAdapter.wordCount !== 1 ? " words" : " word")
                return currentSceneWordCount + totalWordCount
            }
            font.pointSize: Runtime.minimumFontMetrics.font.pointSize

            ItemPositionMapper {
                id: _taggingOptionsPosMapper

                to: _wordCountLabel
                from: _taggingOptions
                position: Qt.point(0,0)
            }

            MouseArea {
                ToolTip.text: "Displays 'current scene word count' / 'whole screenplay word count'."
                ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
                ToolTip.visible: containsMouse

                anchors.fill: parent

                hoverEnabled: true
            }
        }
    }

    Item {
        id: _headingTextAreaOnStatusBar

        anchors.left: _metricsDisplay.right
        anchors.right: _taggingOptions.left
        anchors.margins: 5

        height: parent.height

        clip: true

        ItemPositionMapper {
            id: _contentViewPositionMapper

            to: _headingTextAreaOnStatusBar
            from: screenplayEditorListView
            position: Qt.point(0,0)
        }

        Item {
            width: screenplayEditorListView.width
            height: parent.height

            x: _contentViewPositionMapper.mappedPosition.x

            visible: x > 0

            VclLabel {
                id: _currentSceneNumber

                property real recommendedMargin: sceneHeadingFontMetrics.averageCharacterWidth*5 + pageMargins.left*0.075

                anchors.left: _currentSceneHeadingText.left
                anchors.leftMargin: Math.min(-recommendedMargin, -contentWidth)
                anchors.verticalCenter: _currentSceneHeadingText.verticalCenter

                font.family: sceneHeadingFontMetrics.font.family
                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                text: _private.currentSceneHeading ? _private.currentSceneElement.resolvedSceneNumber + ". " : ''
            }

            VclText {
                id: _currentSceneHeadingText

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: pageMargins.left
                anchors.rightMargin: pageMargins.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: height*0.1

                text: _private.currentSceneHeading ? _private.currentSceneHeading.text : ''
                elide: Text.ElideRight
                font.family: sceneHeadingFontMetrics.font.family
                font.pointSize: Runtime.idealFontMetrics.font.pointSize
            }
        }
    }

    RowLayout {
        id: _taggingOptions

        anchors.right: _zoomSlider.left
        anchors.rightMargin: spacing
        anchors.verticalCenter: parent.verticalCenter

        spacing: 6

        IconButton {
            source: "qrc:/icons/action/layout_grouping.png"
            visible: Runtime.screenplayEditorSettings.allowTaggingOfScenes && Runtime.mainWindowTab === Runtime.MainWindowTab.ScreenplayTab
            tooltipText: "Grouping Options"

            onClicked: _taggingMenu.show()

            MenuLoader {
                id: _taggingMenu

                anchors.left: parent.left
                anchors.bottom: parent.top

                menu: VclMenu {
                    width: 350

                    VclMenuItem {
                        text: "None"
                        font.bold: Scrite.document.structure.preferredGroupCategory === ""
                        icon.source: font.bold ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"

                        onTriggered: Scrite.document.structure.preferredGroupCategory = ""
                    }

                    MenuSeparator { }

                    Repeater {
                        model: Scrite.document.structure.groupCategories

                        VclMenuItem {
                            text: SMath.titleCased(modelData)
                            font.bold: Scrite.document.structure.preferredGroupCategory === modelData
                            icon.source: font.bold ? "qrc:/icons/navigation/check.png" : "qrc:/icons/content/blank.png"

                            onTriggered: Scrite.document.structure.preferredGroupCategory = modelData
                        }
                    }
                }
            }
        }

        Separator { }
    }

    ZoomSlider {
        id: _zoomSlider

        property var zoomLevels: screenplayFormat.fontZoomLevels
        property int savedZoomValue: -1

        function zoomLevelModifierToApply() {
            var zls = zoomLevels
            var oneLevel = value
            for(var i=0; i<zls.length; i++) {
                if(zls[i] === 1) {
                    oneLevel = i
                    break
                }
            }
            return value - oneLevel
        }

        function reset() {
            var zls = zoomLevels
            for(var i=0; i<zls.length; i++) {
                if(zls[i] === 1) {
                    value = i
                    return
                }
            }
        }

        Component.onCompleted: {
            reset()
            value = value + zoomLevelModifier
            zoomLevel = zoomLevels[value]
            screenplayFormat.fontZoomLevelIndex = value
        }

        Announcement.onIncoming: (type, data) => {
                                     const stype = "" + type
                                     const sdata = "" + data
                                     if(stype === "DF77A452-FDB2-405C-8A0F-E48982012D36") {
                                         if(sdata === "save") {
                                             _zoomSlider.savedZoomValue = _zoomSlider.value
                                             _zoomSlider.reset()
                                         } else if(sdata === "restore") {
                                             if(_zoomSlider.savedZoomValue >= 0)
                                                _zoomSlider.value = _zoomSlider.savedZoomValue
                                             _zoomSlider.savedZoomValue = -1
                                         }
                                     }
                                 }

        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter

        to: zoomLevels.length-1
        from: 0
        stepSize: 1

        ActionHandler {
            enabled: _zoomSlider.value < _zoomSlider.to
            action: ActionHub.screenplayEditorOptions.find("zoomIn")

            onTriggered: _zoomSlider.zoomIn()
        }

        ActionHandler {
            enabled: _zoomSlider.value > _zoomSlider.from
            action: ActionHub.screenplayEditorOptions.find("zoomOut")

            onTriggered: _zoomSlider.zoomOut()
        }

        Connections {
            target: screenplayFormat

            function onFontZoomLevelIndexChanged() {
                if(!Scrite.document.empty)
                    _zoomSlider.value = screenplayFormat.fontZoomLevelIndex
            }
        }

        Connections {
            target: Runtime.language.engine

            function onScriptFontFamilyChanged() {
                const oldValue = _zoomSlider.value
                _zoomSlider.value = screenplayFormat.fontZoomLevelIndex
                Qt.callLater( (val) => { _zoomSlider.value = val }, oldValue )
            }
        }

        onValueChanged: {
            screenplayFormat.fontZoomLevelIndex = value

            root.zoomLevelIsAboutToChange()
            zoomLevel = zoomLevels[value]
            root.zoomLevelJustChanged()
        }
    }

    component Separator : Rectangle {
        Layout.fillHeight: true

        width: 1

        color: Runtime.colors.primary.borderColor
    }

    component IconButton : Image {
        property alias pressed: _iconButtonMouseArea.pressed
        property alias containsMouse: _iconButtonMouseArea.containsMouse

        property string tooltipText

        signal clicked()

        Layout.preferredWidth: Runtime.idealFontMetrics.height
        Layout.preferredHeight: Runtime.idealFontMetrics.height

        scale: _iconButtonMouseArea.containsMouse ? (_iconButtonMouseArea.pressed ? 1 : 1.5) : 1
        mipmap: true

        Behavior on scale { NumberAnimation { duration: Runtime.stdAnimationDuration } }

        MouseArea {
            id: _iconButtonMouseArea

            anchors.fill: parent

            hoverEnabled: true

            onClicked: parent.clicked()
        }

        ToolTipPopup {
            text: parent.tooltipText
            visible: _iconButtonMouseArea.containsMouse && !_iconButtonMouseArea.pressed
        }
    }

    QtObject {
        id: _private

        property ScreenplayElement currentSceneElement: Runtime.screenplayAdapter.currentElement
        property Scene currentScene: currentSceneElement ? currentSceneElement.scene : null
        property SceneHeading currentSceneHeading: currentScene && currentScene.heading.enabled ? currentScene.heading : null
    }
}
