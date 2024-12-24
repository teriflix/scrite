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
import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"

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
        anchors.fill: contentsFlick
        anchors.leftMargin: -20
        anchors.rightMargin: -20

        color: Runtime.colors.primary.c100.background
        border.width: 1
        border.color: Runtime.colors.primary.borderColor
    }

    Flickable {
        id: contentsFlick
        anchors.centerIn: parent
        width: Math.min(parent.width * 0.8, 350)
        height: Math.min(contents.height, parent.height)
        contentWidth: width
        contentHeight: contents.height

        ScrollBar.vertical: vscrollBar

        ColumnLayout {
            id: contents

            width: contentsFlick.width
            spacing: 10

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 10
            }

            RowLayout {
                Layout.fillWidth: true

                spacing: 10

                Image {
                    id: icon

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
                id: reasonSuggestion

                Layout.fillWidth: true

                text: "To enable this feature, consider upgrading your plan."
                visible: text !== ""
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere

                font.pointSize: Runtime.minimumFontMetrics.font.pointSize
            }

            Link {
                Layout.alignment: Qt.AlignRight

                text: "Details »"
                enabled: Scrite.user.loggedIn && Scrite.user.info.hasActiveSubscription
                font.pointSize: Runtime.minimumFontMetrics.font.pointSize

                onClicked: {
                    UserAccountDialog.launch()
                    Utils.execLater(root, 500, () => {
                                        Announcement.shout(Runtime.announcementIds.userProfileScreenPage, "Subscriptions")
                                    })
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 10
            }
        }
    }

    VclScrollBar {
        id: vscrollBar
        flickable: contentsFlick
        orientation: Qt.Vertical
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
    }
}
