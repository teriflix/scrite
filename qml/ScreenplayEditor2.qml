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

import Scrite 1.0
import QtQuick 2.13
import QtQuick.Window 2.13
import Qt.labs.settings 1.0
import QtQuick.Controls 2.13

Item {
    // This editor has to specialize in rendering scenes within a ScreenplayAdapter
    // The adapter may contain a single scene or an entire screenplay, that doesnt matter.
    // This way we can avoid having a SceneEditor and ScreenplayEditor as two distinct
    // QML components.

    id: screenplayEditor
    property ScreenplayFormat screenplayFormat: scriteDocument.displayFormat
    property ScreenplayPageLayout pageLayout: screenplayFormat.pageLayout
    property alias source: screenplayAdapter.source

    property alias zoomLevel: zoomSlider.value

    ScreenplayAdapter {
        id: screenplayAdapter
        source: scriteDocument.screenplay
    }

    ScreenplayTextDocument {
        id: screenplayTextDocument
        screenplay: screenplayAdapter.screenplay
        formatting: scriteDocument.printFormat
        syncEnabled: true
    }

    Item {
        id: pageRulerArea
        width: pageLayout.paperWidth * screenplayEditor.zoomLevel * Screen.devicePixelRatio
        height: parent.height
        anchors.top: parent.top
        anchors.bottom: statusBar.top
        anchors.horizontalCenter: parent.horizontalCenter

        RulerItem {
            id: ruler
            width: parent.width
            height: 20
            font.pixelSize: 10
            leftMargin: pageLayout.leftMargin * Screen.devicePixelRatio
            rightMargin: pageLayout.rightMargin * Screen.devicePixelRatio
            zoomLevel: screenplayEditor.zoomLevel

            property real leftMarginPx: leftMargin * zoomLevel
            property real rightMarginPx: rightMargin * zoomLevel
            property real topMarginPx: pageLayout.topMargin * Screen.devicePixelRatio * zoomLevel
            property real bottomMarginPx: pageLayout.bottomMargin * Screen.devicePixelRatio * zoomLevel
        }

        Rectangle {
            id: contentArea
            anchors.top: ruler.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.topMargin: 5
            clip: true
            color: "white"

            ListView {
                id: contentView
                anchors.fill: parent
                model: screenplayAdapter
                delegate: contentComponent
                boundsBehavior: Flickable.StopAtBounds
                boundsMovement: Flickable.StopAtBounds
                cacheBuffer: Math.ceil(height / 300) * 2
                ScrollBar.vertical: verticalScrollBar
                header: Item {
                    width: contentArea.width
                    height: ruler.topMarginPx
                }
                footer: Item {
                    width: contentArea.width
                    height: ruler.bottomMarginPx
                }

                function ensureVisible(item, rect) {
                    if(item === null)
                        return

                    var pt = item.mapToItem(contentView.contentItem, rect.x, rect.y)
                    var startY = contentView.contentY
                    var endY = contentView.contentY + contentView.height - rect.height
                    if( startY < pt.y && pt.y < endY )
                        return

                    if( pt.y < startY )
                        contentView.contentY = pt.y
                    else if( pt.y > endY )
                        contentView.contentY = (pt.y + 2*rect.height) - contentView.height
                }
            }
        }
    }

    ScrollBar {
        id: verticalScrollBar
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: statusBar.top
        orientation: Qt.Vertical
        minimumSize: 0.1
        policy: screenplayAdapter.elementCount > 0 ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
    }

    Rectangle {
        id: statusBar
        height: 30
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: primaryColors.windowColor
        border.width: 1
        border.color: primaryColors.borderColor
        clip: true

        Text {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 20
            text: screenplayTextDocument.currentPage + " of " + screenplayTextDocument.pageCount
        }

        Text {
            anchors.centerIn: parent
            anchors.verticalCenterOffset: height*0.1
            font.family: headingFontMetrics.font.family
            font.pixelSize: parent.height * 0.6
            text: screenplayAdapter.currentScene && screenplayAdapter.currentScene.heading.enabled ?
                  "[" + screenplayAdapter.currentElement.sceneNumber + "] " + screenplayAdapter.currentScene.heading.text : ''
        }

        Row {
            anchors.right: parent.right
            anchors.rightMargin: 20
            anchors.verticalCenter: parent.verticalCenter
            spacing: 10

            Slider {
                id: zoomSlider
                anchors.verticalCenter: parent.verticalCenter
                from: 0.5; to: 2; value: 1
                stepSize: 0.2
                onMoved: screenplayFormat.devicePixelRatio = Screen.devicePixelRatio * value
                Component.onCompleted: screenplayFormat.devicePixelRatio = Screen.devicePixelRatio * value
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: Math.round(zoomSlider.value * 100) + "%"
            }
        }
    }

    Component {
        id: contentComponent

        Rectangle {
            id: contentItem
            width: contentArea.width
            height: contentItemLayout.height
            color: "white"

            readonly property int theIndex: index
            readonly property Scene theScene: scene
            readonly property var binder: sceneDocumentBinder
            readonly property var editor: sceneTextEditor

            SceneDocumentBinder {
                id: sceneDocumentBinder
                scene: contentItem.theScene
                textDocument: sceneTextEditor.textDocument
                cursorPosition: sceneTextEditor.cursorPosition
                characterNames: scriteDocument.structure.characterNames
                screenplayFormat: screenplayEditor.screenplayFormat
                forceSyncDocument: !sceneTextEditor.activeFocus
                onDocumentInitialized: sceneTextEditor.cursorPosition = 0
                // onRequestCursorPosition: app.execLater(sceneDocumentBinder, 100, function() { assumeFocusAt(position) })
                property var currentParagraphType: currentElement ? currentElement.type : SceneHeading.Action
                onCurrentParagraphTypeChanged: {
                    if(currentParagraphType === SceneElement.Action) {
                        ruler.paragraphLeftMargin = 0
                        ruler.paragraphRightMargin = 0
                    } else {
                        var elementFormat = screenplayEditor.screenplayFormat.elementFormat(currentParagraphType)
                        ruler.paragraphLeftMargin = ruler.leftMargin + pageLayout.contentWidth * elementFormat.leftMargin * Screen.devicePixelRatio
                        ruler.paragraphRightMargin = ruler.rightMargin + pageLayout.contentWidth * elementFormat.rightMargin * Screen.devicePixelRatio
                    }
                }
            }

            Column {
                id: contentItemLayout
                width: parent.width

                Rectangle {
                    width: parent.width
                    height: sceneHeadingLoader.height + 16
                    color: Qt.tint(contentItem.theScene.color, "#E7FFFFFF")

                    Loader {
                        id: sceneHeadingLoader
                        clip: true
                        height: item ? item.contentHeight : headingFontMetrics.lineSpacing
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.leftMargin: ruler.leftMarginPx
                        anchors.rightMargin: ruler.rightMarginPx
                        anchors.verticalCenter: parent.verticalCenter
                        property bool viewOnly: true
                        property SceneHeading sceneHeading: contentItem.theScene.heading
                        sourceComponent: {
                            if(sceneHeading.enabled)
                                return viewOnly ? sceneHeadingViewer : sceneHeadingEditor
                            return sceneHeadingDisabled
                        }

                        Connections {
                            target: sceneHeadingLoader.item
                            ignoreUnknownSignals: true
                            onEditRequest: sceneHeadingLoader.viewOnly = false
                            onEditingFinished: sceneHeadingLoader.viewOnly = true
                        }
                    }
                }

                TextArea {
                    id: sceneTextEditor
                    width: parent.width
                    height: contentHeight + topPadding + bottomPadding
                    topPadding: sceneEditorFontMetrics.lineSpacing
                    bottomPadding: sceneEditorFontMetrics.lineSpacing
                    leftPadding: ruler.leftMarginPx
                    rightPadding: ruler.rightMarginPx
                    palette: app.palette
                    selectByMouse: true
                    selectByKeyboard: true
                    background: Item { }
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    font: screenplayFormat.defaultFont
                    onActiveFocusChanged: {
                        if(activeFocus) {
                            screenplayAdapter.currentIndex = contentItem.theIndex
                            globalSceneEditorToolbar.sceneEditor = contentItem
                        } else if(globalSceneEditorToolbar.sceneEditor === contentItem)
                            globalSceneEditorToolbar.sceneEditor = null
                    }

                    FocusTracker.window: qmlWindow
                    onCursorRectangleChanged: {
                        if(activeFocus)
                            contentView.ensureVisible(sceneTextEditor, cursorRectangle)
                    }
                }
            }
        }
    }

    FontMetrics {
        id: defaultFontMetrics
        readonly property SceneElementFormat format: scriteDocument.formatting.elementFormat(SceneElement.Action)
        font: format ? format.font2 : scriteDocument.formatting.defaultFont2
    }

    FontMetrics {
        id: headingFontMetrics
        readonly property SceneElementFormat format: scriteDocument.formatting.elementFormat(SceneElement.Heading)
        font: format.font2
    }

    Component {
        id: sceneHeadingDisabled

        Item {
            property real contentHeight: headingFontMetrics.lineSpacing

            Text {
                text: "no scene heading"
                anchors.verticalCenter: parent.verticalCenter
                color: primaryColors.c10.text
                font: headingFontMetrics.font
                opacity: 0.25
            }
        }
    }

    Component {
        id: sceneHeadingEditor

        Item {
            property real contentHeight: height
            height: layout.height + 4
            Component.onCompleted: {
                locTypeEdit.forceActiveFocus()
            }

            signal editingFinished()

            FocusTracker.window: qmlWindow
            FocusTracker.onHasFocusChanged: {
                if(!FocusTracker.hasFocus)
                    editingFinished()
            }

            Row {
                id: layout
                anchors.left: parent.left
                anchors.right: parent.right

                TextField2 {
                    id: locTypeEdit
                    font: headingFontMetrics.font
                    width: Math.max(contentWidth, 80)
                    anchors.verticalCenter: parent.verticalCenter
                    text: sceneHeading.locationType
                    completionStrings: scriteDocument.structure.standardLocationTypes()
                    onEditingComplete: sceneHeading.locationType = text
                    tabItem: locEdit
                }

                Text {
                    id: sep1Text
                    font: headingFontMetrics.font
                    text: ". "
                    anchors.verticalCenter: parent.verticalCenter
                }

                TextField2 {
                    id: locEdit
                    font: headingFontMetrics.font
                    width: parent.width - locTypeEdit.width - sep1Text.width - momentEdit.width - sep2Text.width
                    anchors.verticalCenter: parent.verticalCenter
                    text: sceneHeading.location
                    enableTransliteration: true
                    completionStrings: scriteDocument.structure.allLocations()
                    onEditingComplete: sceneHeading.location = text
                    tabItem: momentEdit
                }

                Text {
                    id: sep2Text
                    font: headingFontMetrics.font
                    text: "- "
                    anchors.verticalCenter: parent.verticalCenter
                }

                TextField2 {
                    id: momentEdit
                    font: headingFontMetrics.font
                    width: Math.max(contentWidth, 150);
                    anchors.verticalCenter: parent.verticalCenter
                    text: sceneHeading.moment
                    completionStrings: scriteDocument.structure.standardMoments()
                    onEditingComplete: sceneHeading.moment = text
                    tabItem: sceneContentEditor
                }
            }
        }
    }

    Component {
        id: sceneHeadingViewer

        Item {
            property real contentHeight: sceneHeadingText.contentHeight
            signal editRequest()

            Text {
                id: sceneHeadingText
                width: parent.width
                font: headingFontMetrics.font
                text: sceneHeading.text
                anchors.verticalCenter: parent.verticalCenter
                wrapMode: Text.WordWrap
                color: headingFontMetrics.format.textColor
            }

            MouseArea {
                anchors.fill: parent
                onClicked: parent.editRequest()
            }
        }
    }
}
