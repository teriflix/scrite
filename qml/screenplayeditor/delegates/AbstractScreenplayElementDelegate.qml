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

    // These have to be components only.
    default property alias content: _contentLoader.sourceComponent

    // These are optional
    property bool canFocus: true
    property bool readOnly: Scrite.document.readOnly
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
    readonly property alias placeholderMode: _private.placeholderMode

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

    ColumnLayout {
        id: _layout

        Loader {
            Layout.fillWidth: true

            active: root.screenplayElement.pageBreakBefore
            visible: active
            sourceComponent: PageBreakItem {
                placement: Qt.TopEdge
            }
        }

        Loader {
            id: _contentLoader

            Layout.fillWidth: true

            onItemChanged: {
                if(item) {
                    root.__searchBarSaysReplaceCurrent.connect(item.__searchBarSaysReplaceCurrent)
                }
            }
        }

        Loader {
            Layout.fillWidth: true

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

        property bool hasFocus: root.FocusTracker.hasFocus
        property Scene scene: root.screenplayElement.scene

        property real pageWidth: (root.width - pageLeftMargin - pageRightMargin)
        property real pageLeftMargin: root.pageMargins.left
        property real pageRightMargin: root.pageMargins.right

        property bool placeholderMode: Runtime.screenplayEditorSettings.placeholderContentInterval > 0

        Component.onCompleted: {
            if(placeholderMode)
                Utils.execLater(root, Runtime.screenplayEditorSettings.placeholderContentInterval, () => { _private.placeholderMode = false } )
        }
    }
}

