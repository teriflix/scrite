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
            id: _userMessageDelegate

            required property int index
            required property var modelData

            width: _userMessagesView.width
            height: _messageRect.height

            Rectangle {
                id: _messageRect

                anchors.centerIn: parent

                width: 450
                height: _messageLayout.implicitHeight + 30
                border {
                    width: _userMessageDelegate.modelData.read ? 1 : 2
                    color: Runtime.colors.primary.borderColor
                }

                color: Runtime.colors.primary.c200.background

                ColumnLayout {
                    id: _messageLayout

                    anchors.centerIn: parent

                    width: parent.width - 20
                    spacing: 10

                    VclLabel {
                        Layout.fillWidth: true

                        text: Runtime.formatDateIncludingYear(_userMessageDelegate.modelData.timestamp)
                        color: Runtime.colors.primary.c200.text
                        opacity: 0.75
                        font.pointSize: Runtime.minimumFontMetrics.font.pointSize
                    }

                    VclLabel {
                        Layout.fillWidth: true

                        text: _userMessageDelegate.modelData.subject
                        color: Runtime.colors.primary.c200.text
                        wrapMode: Text.WordWrap
                        font.bold: true
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize
                    }

                    Image {
                        Layout.fillWidth: true
                        Layout.preferredHeight: sourceSize.height * (width/sourceSize.width)

                        source: _userMessageDelegate.modelData.image
                        mipmap: true
                        visible: source !== ""
                        fillMode: Image.PreserveAspectFit

                        MouseArea {
                            anchors.fill: parent

                            enabled: _buttonsRepeater.count === 1
                            cursorShape: Qt.PointingHandCursor
                            onClicked: _buttonsRepeater.itemAt(0).handleClick()
                        }
                    }

                    VclLabel {
                        Layout.fillWidth: true

                        text: _userMessageDelegate.modelData.body
                        color: Runtime.colors.primary.c200.text
                        wrapMode: Text.WordWrap
                        font.pointSize: Runtime.idealFontMetrics.font.pointSize
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1

                        color: Runtime.colors.primary.borderColor
                    }

                    Repeater {
                        id: _buttonsRepeater
                        model: _userMessageDelegate.modelData.buttons

                        delegate: Link {
                            id: _buttonDelegate

                            required property int index
                            required property var modelData

                            Layout.fillWidth: true

                            padding: 4
                            text: _buttonDelegate.modelData.text
                            horizontalAlignment: Text.AlignHCenter

                            function handleClick() {
                                if(_buttonDelegate.modelData.action === UserMessageButton.UrlAction) {
                                    Qt.openUrlExternally(_buttonDelegate.modelData.endpoint)
                                    return
                                }

                                if(_buttonDelegate.modelData.action === UserMessageButton.CommandAction) {
                                    switch(_buttonDelegate.modelData.endpoint) {
                                    case "$subscribe":
                                        Runtime.shoutout(Runtime.announcementIds.userProfileScreenPage, "Subscriptions")
                                        return
                                    case "$profile":
                                        Runtime.shoutout(Runtime.announcementIds.userProfileScreenPage, "Profile")
                                        return
                                    case "$installations":
                                        Runtime.shoutout(Runtime.announcementIds.userProfileScreenPage, "Installations")
                                        return
                                    case "$homescreen":
                                        HomeScreen.launch()
                                        return
                                    }
                                }

                                // Implement API and Code in a future update
                                enabled = false
                            }

                            onClicked: handleClick()
                        }
                    }
                }
            }
        }
    }
}
