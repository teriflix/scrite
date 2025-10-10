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

import QtQuick 2.15
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

Item {
    id: root

    property alias busyMessage: _overlay.busyMessage

    function ref() { _overlay.ref() }

    function deref() { _overlay.deref() }

    BusyOverlay {
        id: _overlay

        anchors.fill: parent

        visible: RefCounter.isReffed
        busyMessage: "Computing Page Layout, Evaluating Page Count & Time ..."

        function ref() { RefCounter.ref() }
        function deref() { RefCounter.deref() }
    }

    // Refactor QML TODO: Get rid of this stuff when we move to overlays and ApplicationMainWindow
    QtObject {
        property bool overlayRefCountModified: false
        property bool requiresAppBusyOverlay: Runtime.undoStack.screenplayEditorActive || Runtime.undoStack.sceneEditorActive

        function onUpdateScheduled() {
            if(requiresAppBusyOverlay && !overlayRefCountModified) {
                _overlay.ref()
                overlayRefCountModified = true
            }
        }

        function onUpdateFinished() {
            if(overlayRefCountModified)
                _overlay.deref()
            overlayRefCountModified = false
        }

        onRequiresAppBusyOverlayChanged: {
            if(!requiresAppBusyOverlay && overlayRefCountModified) {
                _overlay.deref()
                overlayRefCountModified = false
            }
        }

        Component.onCompleted: {
            // Cannot use Connections for this, because the Connections QML item
            // does not allow usage of custom properties
            Runtime.screenplayTextDocument.onUpdateScheduled.connect(onUpdateScheduled)
            Runtime.screenplayTextDocument.onUpdateFinished.connect(onUpdateFinished)
        }
    }
}
