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

pragma ComponentBehavior: Bound

import QtQml
import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import io.scrite.components

import "../"
import "../../tasks"
import "../../globals"
import "../../controls"
import "../../helpers"

Item {
    id: root

    Component.onDestruction: Scrite.user.markMessagesAsRead()

    VclLabel {
        anchors.centerIn: parent

        visible: Scrite.user.totalMessageCount === 0

        text: "There are no notifications for you at the moment."
    }

    ListView {
        id: _userMessagesView

        anchors.fill: parent

        ScrollBar.vertical: VclScrollBar { }
        FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

        clip: true
        height: parent.height
        visible: Scrite.user.totalMessageCount > 0

        model: Scrite.user.messages
        spacing: 20
        boundsBehavior: Flickable.StopAtBounds

        header: VclLabel {
            width: _userMessagesView.width
            padding: 10

            font.bold: true
            font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2

            text: {
                const nrUnread = Scrite.user.unreadMessageCount
                const nrMessages = Scrite.user.totalMessageCount

                if(nrMessages === 0) {
                    return "You have no notifications right now."
                }

                if(nrUnread > 0) {
                    if(nrUnread === nrMessages)
                        return "You have " + nrUnread + " unread notification" + (nrUnread > 1 ? "s" : "") + "."
                    else
                        return "You have " + nrUnread + " of " + nrMessages + " unread notification" + (nrMessages > 1 ? "s" : "") + "."
                }

                return "You have " + nrMessages + " notification" + (nrMessages > 1 ? "s" : "") + "."
            }

            wrapMode: Text.WordWrap
        }

        footer: Item {
            width: _userMessagesView.width
            height: 20
        }

        delegate: Item {
            id: _messageItem

            required property int index
            required property scriteUserMessage modelData

            width: _userMessagesView.width
            height: Math.max(100, _contentLayout.implicitHeight + 44)

            Rectangle {
                width: parent.width - 28
                height: parent.height

                color: Runtime.colors.primary.c200.background
                border.width: _messageItem.modelData.read ? 1 : 2
                border.color: Runtime.colors.primary.borderColor

                RowLayout {
                    id: _contentLayout

                    anchors.fill: parent
                    anchors.margins: 11

                    spacing: 16

                    Item {
                        Layout.alignment: Qt.AlignTop
                        Layout.topMargin: _subject.mapToItem(_contentLayout, 0, 0).y
                        Layout.preferredWidth: parent.width * 0.25
                        Layout.fillHeight: true

                        visible: _messageImage.status === Image.Ready

                        Image {
                            id: _messageImageBg

                            anchors.fill: parent
                            anchors.margins: 1

                            fillMode: Image.PreserveAspectCrop
                            source: _messageItem.modelData.image
                        }

                        Rectangle {
                            anchors.fill: parent
                            color: Runtime.colors.primary.editor.background
                            opacity: 0.7
                        }

                        Rectangle {
                            color: "transparent"
                            width: _messageImage.paintedWidth
                            height: _messageImage.paintedHeight
                            border.width: 1
                            border.color: "darkgray"
                            anchors.centerIn: parent
                        }

                        Image {
                            id: _messageImage

                            anchors.fill: parent
                            anchors.margins: 1

                            fillMode: Image.PreserveAspectFit
                            mipmap: true
                            source: _messageItem.modelData.image

                            MouseArea {
                                anchors.fill: parent

                                enabled: _actions.count > 0
                                cursorShape: Qt.PointingHandCursor
                                onClicked: _actions.itemAt(0).activate()
                            }
                        }

                        BusyIndicator {
                            anchors.centerIn: parent
                            running: _messageImage.status !== Image.Ready
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignTop

                        spacing: 12

                        VclLabel {
                            Layout.fillWidth: true

                            text: Runtime.formatDateIncludingYear(_messageItem.modelData.timestamp)
                            color: Runtime.colors.primary.c200.text
                            opacity: 0.75
                            font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                        }

                        VclLabel {
                            id: _subject

                            Layout.fillWidth: true

                            text: _messageItem.modelData.subject
                            color: Runtime.colors.primary.c200.text
                            font.bold: true
                            font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2
                            wrapMode: Text.WordWrap
                            maximumLineCount: 2
                            elide: Text.ElideRight
                        }

                        VclLabel {
                            id: _body

                            Layout.fillWidth: true

                            text: _messageItem.modelData.body
                            color: Runtime.colors.primary.c200.text
                            font.pointSize: Runtime.idealFontMetrics.font.pointSize
                            wrapMode: Text.WordWrap
                            maximumLineCount: 6
                            elide: Text.ElideRight

                            MouseArea {
                                id: _bodyMouseArea

                                anchors.fill: parent

                                enabled: parent.truncated
                                hoverEnabled: true

                                ToolTipPopup {
                                    container: _bodyMouseArea
                                    text: _body.text
                                    visible: _bodyMouseArea.containsMouse
                                }
                            }
                        }

                        Flow {
                            Layout.fillWidth: true

                            spacing: 12

                            Repeater {
                                id: _actions

                                model: _messageItem.modelData.buttons

                                delegate: Link {
                                    required property int index
                                    required property scriteUserMessageButton modelData

                                    text: modelData.text

                                    function activate() {
                                        if(modelData.action === UserMessageButton.UrlAction) {
                                            Qt.openUrlExternally(modelData.endpoint)
                                            return
                                        }

                                        if(modelData.action === UserMessageButton.CommandAction) {
                                            UserAccountDialog.handleMessageEndpoint(modelData.endpoint)
                                            return
                                        }
                                    }

                                    onClicked: activate()
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
