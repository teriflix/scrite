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

import QtCore
import QtQuick

import io.scrite.components

import "../"

Settings {
    property bool displayAnnotationProperties: true
    property bool showGrid: true
    property bool showPreview: true
    property bool showPullHandleAnimation: true

    property real connectorLineWidth: 2
    property real overflowFactor: 0.05
    property real previewSize: 300
    property real annotationDockX: 80
    property real annotationDockY: 100

    property color gridColor: Runtime.colors.accent.c400.background
    property color canvasColor: Runtime.colors.accent.c50.background

    function restoreDefaultGridColor() {
        gridColor = Runtime.colors.accent.c400.background
    }

    function restoreDefaultCanvasColor() {
        canvasColor = Runtime.colors.accent.c50.background
    }

    category: "Structure Tab"
    location: Platform.settingsLocation

}
