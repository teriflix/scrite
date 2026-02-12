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

    color: Runtime.colors.accent.c100.background

    border.color: Runtime.colors.accent.borderColor
    border.width: 0.5

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
            color: Color.textColorFor(root.color)
            text: Scrite.document.screenplay.title === "" ? "[#] TITLE PAGE" : Scrite.document.screenplay.title
            padding: 10

            font.bold: true
            font.family: Runtime.sceneEditorFontMetrics.font.family
            font.pointSize: Runtime.idealFontMetrics.font.pointSize

            MouseArea {
                anchors.fill: parent
                hoverEnabled: _headingText.truncated

                ToolTipPopup {
                    text: _headingText.text
                    visible: _headingText.truncated && container.containsMouse
                }

                onClicked: root.clicked()
            }
        }

        Image {
            id: _menuButton

            Layout.preferredWidth: _headingText.height * 0.6
            Layout.preferredHeight: _headingText.height * 0.6

            source: Color.isLight(root.color) ? "qrc:/icons/content/view_options.png" : "qrc:/icons/content/view_options_inverted.png"
            fillMode: Image.PreserveAspectFit

            MouseArea {
                anchors.fill: parent

                hoverEnabled: true

                ToolTipPopup {
                    text: "Scene List Options"
                    visible: parent.containsMouse
                }

                onClicked: _menu.open()
            }

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
