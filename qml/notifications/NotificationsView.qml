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
    id: root

    function init() { }

    anchors.fill: parent

    parent: NotificationsLayer.item

    color: Color.translucent(Runtime.colors.primary.borderColor, 0.6)
    visible: NotificationsLayer.valid && Scrite.notifications.count > 0
    focus: visible

    MouseArea {
        anchors.fill: parent

        enabled: parent.visible
        hoverEnabled: true
        propagateComposedEvents: false
    }

    Flickable {
        id: _flickable

        anchors.top: parent.top
        anchors.topMargin: -1
        anchors.horizontalCenter: parent.horizontalCenter

        width: parent.width * 0.7
        height: Math.min( contentHeight, parent.height*0.25 )

        visible: height > 0

        contentWidth: width
        contentHeight: _layout.implicitHeight

        Column {
            id: _layout

            width: _flickable.width

            spacing: 10

            Repeater {
                model: Scrite.notifications.count

                Rectangle {
                    id: _delegate

                    required property int index
                    property Notification notification: Scrite.notifications.notificationAt(index)

                    width: _flickable.width-1
                    height: Math.max(100, _delegateLayout.implicitHeight+44)

                    color: notification.color
                    border { width: 1; color: Runtime.colors.primary.borderColor }

                    RowLayout {
                        id: _delegateLayout

                        anchors.centerIn: parent

                        width: parent.width-44

                        spacing: 30

                        Rectangle {
                            Layout.preferredWidth: parent.width*0.25
                            Layout.preferredHeight: {
                                if(_delegateImage.status === Image.Ready) {
                                    return _delegateImage.sourceSize.height * (Layout.preferredWidth/_delegateImage.sourceSize.width)
                                }

                                return Layout.preferredWidth*9/16
                            }

                            visible: notification.hasImage
                            border.width: 1
                            border.color: Runtime.colors.primary.borderColor

                            Image {
                                id: _delegateImage

                                anchors.fill: parent
                                anchors.margins: 1

                                fillMode: Image.PreserveAspectFit
                                mipmap: true
                                source: notification.image

                                MouseArea {
                                    anchors.fill: parent

                                    onClicked: _delegate.notification.notifyImageClick()
                                }
                            }

                            BusyIndicator {
                                anchors.centerIn: parent

                                running: _delegateImage.status !== Image.Ready
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true

                            spacing: 20

                            VclLabel {
                                Layout.fillWidth: true

                                color: _delegate.notification.textColor
                                font.bold: true
                                text: _delegate.notification.title
                                visible: text !== ""
                                wrapMode: Text.WordWrap

                                font.pointSize: Runtime.idealFontMetrics.font.pointSize + 4
                            }

                            VclLabel {
                                Layout.fillWidth: true

                                color: _delegate.notification.textColor
                                text: _delegate.notification.text
                                wrapMode: Text.WordWrap

                                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                            }

                            RowLayout {
                                Layout.fillWidth: true

                                spacing: 20

                                Repeater {
                                    model: _delegate.notification.buttons

                                    VclButton {
                                        required property int index
                                        required property string modelData

                                        width: Math.max(75, implicitWidth)

                                        text: modelData

                                        onClicked: _delegate.notification.notifyButtonClick(index)
                                    }
                                }
                            }
                        }

                        VclButton {
                            visible: !_delegate.notification.autoClose && !_delegate.notification.hasButtons
                            text: "Dismiss"

                            onClicked: Scrite.notifications.dismissNotification(index)
                        }
                    }
                }
            }
        }
    }
}
