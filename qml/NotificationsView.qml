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
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

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
            model: Scrite.notifications.count

            Rectangle {
                required property int index
                property Notification notification: Scrite.notifications.notificationAt(index)

                width: notificationsView.width-1
                height: Math.max(100, nLayout.implicitHeight+44)
                color: notification.color
                border { width: 1; color: primaryColors.borderColor }

                RowLayout {
                    id: nLayout
                    width: parent.width-44
                    anchors.centerIn: parent
                    spacing: 30

                    Rectangle {
                        visible: notification.hasImage
                        Layout.preferredWidth: parent.width*0.25
                        Layout.preferredHeight: {
                            if(nimage.status === Image.Ready)
                                return nimage.sourceSize.height * (Layout.preferredWidth/nimage.sourceSize.width)
                            return Layout.preferredWidth*9/16
                        }
                        border.width: 1
                        border.color: primaryColors.borderColor

                        Image {
                            id: nimage
                            source: notification.image
                            fillMode: Image.PreserveAspectFit
                            anchors.fill: parent
                            anchors.margins: 1
                            mipmap: true

                            MouseArea {
                                anchors.fill: parent
                                onClicked: notification.notifyImageClick()
                            }
                        }

                        BusyIndicator {
                            anchors.centerIn: parent
                            running: nimage.status !== Image.Ready
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 20

                        Label {
                            Layout.fillWidth: true
                            text: notification.title
                            wrapMode: Text.WordWrap
                            font.pointSize: Scrite.app.idealFontPointSize + 4
                            font.bold: true
                            visible: text !== ""
                            color: notification.textColor
                        }

                        Label {
                            Layout.fillWidth: true
                            font.pointSize: Scrite.app.idealFontPointSize
                            text: notification.text
                            wrapMode: Text.WordWrap
                            color: notification.textColor
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 20

                            Repeater {
                                model: notification.buttons

                                Button2 {
                                    required property string modelData
                                    required property int index

                                    id: button
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: Math.max(75, implicitWidth)
                                    text: modelData
                                    onClicked: notification.notifyButtonClick(index)
                                }
                            }
                        }
                    }

                    Button2 {
                        id: dismissButton
                        visible: !notification.autoClose && !notification.hasButtons
                        text: "Dismiss"
                        onClicked: Scrite.notifications.dismissNotification(index)
                    }
                }
            }
        }
    }
}
