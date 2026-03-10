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

import QtQml
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import io.scrite.components

import "../globals"
import "../dialogs"
import "../controls"

Rectangle {
    id: root

    property string featureName

    signal clicked()

    color: Runtime.colors.primary.c100.background
    clip: true

    MouseArea {
        anchors.fill: parent
    }

    Rectangle {
        anchors.fill: _contentsFlick
        anchors.leftMargin: -20
        anchors.rightMargin: -20

        color: Runtime.colors.primary.c100.background
        border.width: 1
        border.color: Runtime.colors.primary.borderColor
    }

    Flickable {
        id: _contentsFlick

        ScrollBar.vertical: _vscrollBar

        anchors.centerIn: parent

        width: Math.min(parent.width * 0.8, 350)
        height: Math.min(_contents.height, parent.height)

        contentWidth: width
        contentHeight: _contents.height

        ColumnLayout {
            id: _contents

            width: _contentsFlick.width
            spacing: 10

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 10
            }

            RowLayout {
                Layout.fillWidth: true

                spacing: 10

                Image {
                    id: _icon

                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 24

                    source: "qrc:/images/feature_locked.png"
                    asynchronous: false
                    fillMode: Image.PreserveAspectFit
                }

                VclLabel {
                    Layout.fillWidth: true

                    text: featureName
                    visible: text !== ""
                    wrapMode: Text.WordWrap

                    font.bold: true
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize
                }
            }

            VclLabel {
                id: _reasonSuggestion

                Layout.fillWidth: true

                text: "This feature is not available in your current subscription plan."
                visible: text !== ""
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere

                font.pointSize: Runtime.minimumFontMetrics.font.pointSize
            }

            Link {
                Layout.alignment: Qt.AlignRight

                text: "Details »"
                enabled: Scrite.user.loggedIn && Scrite.user.info.hasActiveSubscription
                font.pointSize: Runtime.minimumFontMetrics.font.pointSize

                onClicked: UserAccountDialog.launch("Subscriptions")
            }

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 10
            }
        }
    }

    VclScrollBar {
        id: _vscrollBar

        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        flickable: _contentsFlick
        orientation: Qt.Vertical
    }
}
