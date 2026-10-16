/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

pragma ComponentBehavior: Bound

import QtQml
import QtQuick

import Scrite.App

import "../../../globals"

FocusScope {
    id: root

    // These have to be explicitly provided
    required property var pageMargins
    required property bool isCurrent
    required property real zoomLevel
    required property FontMetrics fontMetrics
    required property ScreenplayElement screenplayElement

    // These are readonly
    readonly property alias font: _private.font
    readonly property alias pageLeftMargin: _private.pageLeftMargin
    readonly property alias pageRightMargin: _private.pageRightMargin

    // These can be set
    property bool readOnly: Scrite.document.readOnly

    // Signals
    signal ensureVisible(Item item, rect area)

    QtObject {
        id: _private

        property font font: root.fontMetrics.font

        property real pageLeftMargin: root.pageMargins.left
        property real pageRightMargin: root.pageMargins.right
    }
}
