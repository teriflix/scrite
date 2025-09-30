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
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/helpers"
import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/structureview"
import "qrc:/qml/screenplayeditor"

Item {
    id: root

    required property real zoomLevel

    required property TextEdit sceneTextEditor
    required property SceneDocumentBinder sceneDocumentBinder

    // Current line highlight
    Rectangle {
        x: 0
        y: sceneTextEditor.cursorRectangle.y-2*zoomLevel
        width: parent.width
        height: sceneTextEditor.cursorRectangle.height+4*zoomLevel

        color: Runtime.colors.primary.c100.background
        visible: sceneTextEditor.cursorVisible && sceneTextEditor.activeFocus && Runtime.screenplayEditorSettings.highlightCurrentLine && Scrite.app.usingMaterialTheme

        Rectangle {
            width: 20 * root.zoomLevel
            height: parent.height

            color: root.sceneDocumentBinder.scene.color
        }
    }
}
