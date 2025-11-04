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

import io.scrite.components 1.0


import "qrc:/qml/globals"

FocusScope {
    id: root

    // These properties come from the model
    required property int index
    required property bool screenplayElementDelegateHasFocus
    required property string sceneID
    required property ScreenplayElement screenplayElement

    // These have to be explicitly provided
    required property var pageMargins
    required property real zoomLevel
    required property string partName
    required property FontMetrics fontMetrics
    required property ScreenplayAdapter screenplayAdapter

    // These are readonly
    readonly property alias font: _private.font
    readonly property alias scene: _private.scene
    readonly property alias hasFocus: _private.hasFocus
    readonly property alias pageWidth: _private.pageWidth
    readonly property alias pageLeftMargin: _private.pageLeftMargin
    readonly property alias pageRightMargin: _private.pageRightMargin

    // These can be set
    property bool readOnly: Scrite.document.readOnly

    // Signals
    signal ensureVisible(Item item, rect area)

    // Signals, but must actually be implemented as methods by implementations of this abstract item.
    // Not all implementations need to implement this, only those that implement find and replace.
    // As of now, its only SceneContentEditor
    signal __searchBarSaysReplaceCurrent(string replacementText, SearchAgent agent)

    FocusTracker.objectName: "SceneDelegate-" + root.index + ", Part-" + partName
    FocusTracker.window: Scrite.window
    FocusTracker.evaluationMethod: FocusTracker.StandardFocusEvaluation
    FocusTracker.indicator.target: _private
    FocusTracker.indicator.property: "hasFocus"

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
