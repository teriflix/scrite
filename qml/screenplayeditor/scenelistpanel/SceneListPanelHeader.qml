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
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

Rectangle {
    id: root

    required property real leftPadding
    required property real rightPadding

    signal clicked()

    height: _layout.height

    RowLayout {
        id: _layout

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: root.leftPadding
        anchors.rightMargin: root.rightPadding

        VclText {
            id: _headingText

            Layout.fillWidth: true

            elide: Text.ElideRight
            text: Scrite.document.screenplay.title === "" ? "[#] TITLE PAGE" : Scrite.document.screenplay.title

            font.bold: true
            font.family: Runtime.sceneEditorFontMetrics.font.family
            font.pointSize: Runtime.idealFontMetrics.font.pointSize

            MouseArea {
                ToolTip.text: _headingText.text
                ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
                ToolTip.visible: _headingText.truncated && containsMouse

                anchors.fill: parent
                hoverEnabled: _headingText.truncated

                onClicked: root.clicked()
            }
        }

        VclToolButton {
            ToolTip.text: "Scene List Options"

            down: _menu.visible
            icon.source: "qrc:/icons/content/view_options.png"

            onClicked: _menu.open()

            Item {
                anchors.bottom: parent.bottom

                width: parent.width

                SceneListPanelMenu {
                    id: _menu
                }
            }
        }
    }
}
