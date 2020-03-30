/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13
import Scrite 1.0

Item {
    id: sceneEditor
    property Scene scene
    property bool  readOnly: false
    property SceneElementFormat sceneHeadingFormat: scriteDocument.formatting.elementFormat(SceneElement.Heading)
    property alias binder: sceneDocumentBinder
    property Item  editor: sceneContentEditor
    property bool  editorHasActiveFocus: sceneContentEditor.activeFocus
    property real  fullHeight: (sceneHeadingLoader.active ? sceneHeadingArea.height : 0) + (sceneContentEditor ? (sceneContentEditor.contentHeight+10) : 0) + 10
    property color backgroundColor: scene ? Qt.tint(scene.color, "#E0FFFFFF") : "white"
    property bool  scrollable: true
    property bool  showOnlyEnabledSceneHeadings: false

    Rectangle {
        id: sceneHeadingArea
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        height: sceneHeadingLoader.active ? sceneHeadingLoader.height : 0
        border { width: 1; color: "lightgray" }

        Loader {
            id: sceneHeadingLoader
            width: parent.width
            height: 40
            property bool viewOnly: true
            active: scene !== null && scene.heading !== null && (showOnlyEnabledSceneHeadings ? scene.heading.enabled : true)
            sourceComponent: {
                if(scene !== null && scene.heading !== null && scene.heading.enabled)
                    return viewOnly ? sceneHeadingViewer : sceneHeadingEditor
                return sceneHeadingDisabled
            }
        }
    }

    property TextArea sceneContentEditor

    Rectangle {
        anchors.left: parent.left
        anchors.top: sceneHeadingArea.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: backgroundColor

        SceneDocumentBinder {
            id: sceneDocumentBinder
            screenplayFormat: scriteDocument.formatting
            scene: sceneEditor.scene
            textDocument: sceneContentEditor.textDocument
            cursorPosition: sceneContentEditor.cursorPosition
            characterNames: scriteDocument.structure.characterNames
            onDocumentInitialized: sceneContentEditor.cursorPosition = 0
            forceSyncDocument: !sceneContentEditor.activeFocus
        }

        Loader {
            id: contentEditorLoader
            anchors.fill: parent
            clip: true
            active: true
            sourceComponent: scrollable ? scrollableSceneContentEditorComponent : sceneContentEditorComponent
        }
    }

    onSceneChanged: {
        contentEditorLoader.active = false
        contentEditorLoader.active = true
    }

    Component {
        id: scrollableSceneContentEditorComponent

        ScrollView {
            id: scrollView

            Loader {
                width: scrollView.width
                height: Math.max(scrollView.height, item.contentHeight)
                sourceComponent: sceneContentEditorComponent
                active: true
            }

            Component.onCompleted: {
                scrollView.ScrollBar.vertical.setPosition(0)
            }
        }
    }

    Component {
        id: sceneContentEditorComponent

        TextArea {
            id: sceneTextArea
            Component.onCompleted: sceneContentEditor = sceneTextArea
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            renderType: Text.NativeRendering
            readOnly: sceneEditor.readOnly
            background: Rectangle {
                color: backgroundColor
            }
            EventFilter.events: [31] // Wheel
            EventFilter.onFilter: {
                result.acceptEvent = false
                result.filter = !scrollable
            }

            cursorDelegate: Item {
                width: sceneTextArea.cursorRectangle.width
                height: sceneTextArea.cursorRectangle.height
                visible: sceneTextArea.activeFocus

                Rectangle {
                    id: blinkingCursor
                    color: "black"
                    width: 2
                    height: parent.height

                    SequentialAnimation {
                        loops: Animation.Infinite
                        running: sceneTextArea.activeFocus

                        NumberAnimation {
                            target: blinkingCursor
                            property: "opacity"
                            duration: 400
                            easing.type: Easing.Linear
                            from: 0
                            to: 1
                        }

                        NumberAnimation {
                            target: blinkingCursor
                            property: "opacity"
                            duration: 400
                            easing.type: Easing.Linear
                            from: 1
                            to: 0
                        }
                    }
                }

                // TODO: Lets come back to this auto-complete bit later.
                // I am feeling a bit stumped with this feature at this point.
                Loader {
                    anchors.top: blinkingCursor.bottom
                    anchors.left: parent.left
                    active: false // sceneDocumentBinder.autoCompleteHints.length > 0
                    sourceComponent: Rectangle {
                        id: completionListViewPopup
                        width: completionListView.width + 10
                        height: completionListView.height + 10
                        visible: true
                        focus: false
                        color: "white"
                        border { width: 1; color: "black" }
                        radius: 2

                        property SceneElementFormat format: scriteDocument.formatting.elementFormat(sceneDocumentBinder.currentElement.type)

                        Completer {
                            id: completer
                            strings: sceneDocumentBinder.autoCompleteHints
                            completionPrefix: sceneDocumentBinder.completionPrefix
                        }

                        Column {
                            id: completionListView
                            anchors.centerIn: parent
                            spacing: 5
                            Repeater {
                                model: completer.completionModel
                                delegate: Text {
                                    text: display
                                    font: format.font
                                }
                            }
                        }
                    }
                }
            }
            onActiveFocusChanged: {
                if(activeFocus)
                    sceneHeadingLoader.viewOnly = true
            }
            Keys.onTabPressed: sceneDocumentBinder.tab()
            Keys.onBackPressed: sceneDocumentBinder.backtab()
        }
    }

    Component {
        id: sceneHeadingDisabled

        Rectangle {
            color: Qt.tint(scene.color, "#D9FFFFFF")
            property font headingFont: sceneHeadingFormat.font
            Component.onCompleted: headingFont.pointSize = headingFont.pointSize+8

            Text {
                text: "inherited from previous scene"
                anchors.centerIn: parent
                color: "gray"
                font: headingFont
            }
        }
    }

    Component {
        id: sceneHeadingEditor

        Row {
            spacing: 10
            property font headingFont: sceneHeadingFormat.font
            Component.onCompleted: headingFont.pointSize = headingFont.pointSize+8

            ComboBox {
                id: locationTypeCombo
                model: ListModel {
                    ListElement { value: SceneHeading.NoLocationType; display: "NONE" }
                    ListElement { value: SceneHeading.Interior; display: "INT" }
                    ListElement { value: SceneHeading.Exterior; display: "EXT" }
                    ListElement { value: SceneHeading.Both; display: "I/E" }
                }
                textRole: "display"
                editable: false
                currentIndex: scene.heading.locationType+1
                anchors.verticalCenter: parent.verticalCenter
                font: headingFont
                onActivated: scene.heading.locationType = model.get(index).value
            }

            TextField {
                id: locationCombo
                width: parent.width - locationTypeCombo.width - momentCombo.width - 2*parent.spacing
                text: scene.heading.location
                anchors.verticalCenter: parent.verticalCenter
                font: headingFont
                onActiveFocusChanged: {
                    if(activeFocus)
                        selectAll()
                }
                onTextEdited: scene.heading.location = text
            }

            ComboBox {
                id: momentCombo
                model: ListModel {
                    ListElement { value: SceneHeading.NoMoment; display: "NONE" }
                    ListElement { value: SceneHeading.Day; display: "DAY" }
                    ListElement { value: SceneHeading.Night; display: "NIGHT" }
                    ListElement { value: SceneHeading.Morning; display: "MORNING" }
                    ListElement { value: SceneHeading.Afternoon; display: "AFTERNOON" }
                    ListElement { value: SceneHeading.Evening; display: "EVENING" }
                    ListElement { value: SceneHeading.Later; display: "LATER" }
                    ListElement { value: SceneHeading.MomentsLater; display: "MOMENTS LATER" }
                    ListElement { value: SceneHeading.Continuous; display: "CONTINUOUS" }
                    ListElement { value: SceneHeading.TheNextDay; display: "THE NEXT DAY" }
                    ListElement { value: SceneHeading.Earlier; display: "EARLIER" }
                    ListElement { value: SceneHeading.MomentsEarlier; display: "MOMENTS EARLIER" }
                    ListElement { value: SceneHeading.ThePreviousDay; display: "THE PREVIOUS DAY" }
                }
                textRole: "display"
                editable: false
                currentIndex: scene.heading.moment+1
                anchors.verticalCenter: parent.verticalCenter
                font: headingFont
                onActivated: scene.heading.moment = model.get(index).value
                width: app.textBoundingRect("THE PREVIOUS DAY", font).width + 50
            }
        }
    }

    Component {
        id: sceneHeadingViewer

        Rectangle {
            color: Qt.tint(scene.color, "#D9FFFFFF")
            property font headingFont: sceneHeadingFormat.font
            Component.onCompleted: headingFont.pointSize = headingFont.pointSize+8

            Row {
                anchors.fill: parent
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                spacing: 30

                Text {
                    id: locationTypeText
                    text: scene.heading.locationTypeAsString
                    font: headingFont
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    text: scene.heading.location
                    font: headingFont
                    width: parent.width - locationTypeText.width - momentText.width - 2*parent.spacing
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    id: momentText
                    text: scene.heading.momentAsString
                    font: headingFont
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if(readOnly)
                        return
                    sceneHeadingLoader.viewOnly = false
                }
            }
        }
    }
}
