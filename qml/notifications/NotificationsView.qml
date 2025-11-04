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

pragma Singleton

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"

Rectangle {
    function init() { }

    parent: NotificationsLayer.item
    anchors.fill: parent

    visible: NotificationsLayer.valid && Scrite.notifications.count > 0
    color: Color.translucent(Runtime.colors.primary.borderColor, 0.6)

    focus: visible

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        propagateComposedEvents: false
        enabled: parent.visible
    }

    Flickable {
        id: notificationsFlick
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: -1

        width: parent.width * 0.7
        height: Math.min( contentHeight, parent.height*0.25 )

        visible: height > 0

        contentWidth: width
        contentHeight: notificationsLayout.implicitHeight

        Column {
            id: notificationsLayout

            spacing: 10
            width: notificationsFlick.width

            Repeater {
                model: Scrite.notifications.count

                Rectangle {
                    required property int index
                    property Notification notification: Scrite.notifications.notificationAt(index)

                    width: notificationsFlick.width-1
                    height: Math.max(100, nLayout.implicitHeight+44)
                    color: notification.color
                    border { width: 1; color: Runtime.colors.primary.borderColor }

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
                            border.color: Runtime.colors.primary.borderColor

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

                            VclLabel {
                                Layout.fillWidth: true
                                text: notification.title
                                wrapMode: Text.WordWrap
                                font.pointSize: Runtime.idealFontMetrics.font.pointSize + 4
                                font.bold: true
                                visible: text !== ""
                                color: notification.textColor
                            }

                            VclLabel {
                                Layout.fillWidth: true
                                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                                text: notification.text
                                wrapMode: Text.WordWrap
                                color: notification.textColor
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 20

                                Repeater {
                                    model: notification.buttons

                                    VclButton {
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

                        VclButton {
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
}
