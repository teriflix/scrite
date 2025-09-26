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
import QtQuick.Layouts 1.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/screenplayeditor"

FocusScope {
    id: root

    // These properties come from the model
    required property int index
    required property int screenplayElementType
    required property int breakType
    required property string sceneID
    required property ScreenplayElement screenplayElement

    // These have to be explicitly provided
    required property var pageMargins // must set using Utils.margins() only
    required property real zoomLevel
    required property bool isCurrent

    // These have to be components only.
    default property alias content: _contentLoader.sourceComponent

    // These are optional
    property int placeholderInterval: Runtime.screenplayEditorSettings.placeholderInterval
    property int currentParagraphType: -1

    property bool canFocus: true
    property bool readOnly: Scrite.document.readOnly
    property bool usePlaceholder: false
    property Screenplay screenplay: Scrite.document.screenplay
    property FontMetrics fontMetrics: Runtime.sceneEditorFontMetrics
    property ScreenplayFormat format: Scrite.document.displayFormat

    // These are readonly
    readonly property alias font: _private.font
    readonly property alias scene: _private.scene
    readonly property alias hasFocus: _private.hasFocus
    readonly property alias pageWidth: _private.pageWidth
    readonly property alias pageLeftMargin: _private.pageLeftMargin
    readonly property alias pageRightMargin: _private.pageRightMargin
    readonly property alias contentItem: _contentLoader.sourceComponent

    // Public functions with fixed functionality
    function focusIn(cursorPosition) { if(canFocus) __focusIn(cursorPosition) }
    function focusOut() { if(hasFocus) __focusOut() }
    function focusInFromTop() { if(canFocus) __focusIn(0) }
    function focusInFromBottom() { if(canFocus) __focusIn(-1) }

    // Signals that must be emitted by implementations
    signal ensureVisible(Item item, rect area)

    // Signals that need to be handled in implementations
    signal __focusIn(int cursorPosition)
    signal __focusOut()
    signal __searchBarSaysReplaceCurrent(string replacementText, SearchAgent agent)

    height: _layout.height

    /**
      Initially, I did use ColumnLayout here so that we are able to consistently use QtQuick Layouts
      everywhere. But then, the issue with ColumnLayout is that we invariably use Layout attached property
      in it to specify layout hints. So, we will end up using two objects instead of one to simply
      layout. As it is the delegates used for screenplay editor list view are rather heavy. We are better
      off using lightweight Column for trival layouting.
      */
    Column {
        id: _layout

        width: parent.width

        Rectangle {
            width: parent.width
            height: Runtime.screenplayEditorSettings.spaceBetweenScenes * root.zoomLevel

            color: Runtime.colors.primary.windowColor
            visible: height > 0
        }

        Loader {
            width: parent.width

            active: root.screenplayElement.pageBreakBefore
            visible: active
            sourceComponent: PageBreakItem {
                placement: Qt.TopEdge
            }
        }

        Item {
            width: parent.width
            height: _contentLoader.height

            Rectangle {
                anchors.fill: _contentLoader
                anchors.margins: -1

                color: Qt.rgba(0,0,0,0)
                visible: Runtime.screenplayEditorSettings.spaceBetweenScenes > 0

                border.width: 1
                border.color: Runtime.colors.primary.c400.background
            }

            Loader {
                id: _contentLoader

                FocusTracker.window: Scrite.window
                FocusTracker.objectName: "SceneDelegate-" + root.index
                FocusTracker.evaluationMethod: FocusTracker.StandardFocusEvaluation
                FocusTracker.indicator.target: _private
                FocusTracker.indicator.property: "hasFocus"

                width: parent.width

                onItemChanged: {
                    if(item && item.__searchBarSaysReplaceCurrent) {
                        root.__searchBarSaysReplaceCurrent.connect(item.__searchBarSaysReplaceCurrent)
                    }
                }

                // We need a rectangle on the extreme left highlighting the delegate that is current
                Rectangle {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom

                    width: 10

                    z: 1
                    color: root.screenplayElement.scene ? root.screenplayElement.scene.color : Runtime.colors.primary.highlight.background
                    opacity: root.hasFocus ? 1 : 0.5
                    visible: root.isCurrent
                }
            }
        }


        Loader {
            width: parent.width

            active: root.screenplayElement.pageBreakAfter
            visible: active
            sourceComponent: PageBreakItem {
                placement: Qt.BottomEdge
            }
        }
    }

    QtObject {
        id: _private

        property font font: root.fontMetrics.font

        property bool hasFocus: false
        property Scene scene: root.screenplayElement.scene

        property real pageWidth: (root.width - pageLeftMargin - pageRightMargin)
        property real pageLeftMargin: root.pageMargins.left
        property real pageRightMargin: root.pageMargins.right
    }
}

