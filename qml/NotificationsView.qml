/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQuick 2.13
import QtQuick.Controls 2.13
import Scrite 1.0

Flickable {
    id: notificationsView
    height: Math.min( contentHeight, parent.height*0.25 )
    visible: height > 0
    contentWidth: width
    contentHeight: notificationsLayout.implicitHeight

    Column {
        id: notificationsLayout
        spacing: 10
        width: notificationsView.width

        Repeater {
            model: notificationManager.count

            Rectangle {
                width: notificationsView.width-1
                height: Math.max(100, ntextLayout.implicitHeight+20)
                color: notification.color
                border { width: 1; color: primaryColors.borderColor }
                property Notification notification: notificationManager.notificationAt(index)

                Column {
                    id: ntextLayout
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.right: notification.autoClose ? parent.right : dismissButton.left
                    anchors.margins: 20
                    spacing: 10

                    Text {
                        width: parent.width
                        text: notification.title
                        wrapMode: Text.WordWrap
                        font.pixelSize: 20
                        font.bold: true
                        visible: text !== ""
                        color: notification.textColor
                    }

                    Text {
                        width: parent.width
                        text: notification.text
                        wrapMode: Text.WordWrap
                        font.pixelSize: 16
                        color: notification.textColor
                    }

                    Row {
                        spacing: parent.spacing * 3
                        anchors.left: parent.left
                        anchors.leftMargin: 40

                        Repeater {
                            model: notification.buttons

                            Item {
                                width: button.width
                                height: button.height * 2

                                Button2 {
                                    id: button
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: Math.max(75, implicitWidth)
                                    text: modelData
                                    onClicked: notification.notifyButtonClick(index)
                                }
                            }
                        }
                    }
                }

                Button2 {
                    id: dismissButton
                    visible: notification.autoClose === false
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    anchors.rightMargin: 20
                    text: "Dismiss"
                    onClicked: notificationManager.dismissNotification(index)
                }
            }
        }
    }
}
