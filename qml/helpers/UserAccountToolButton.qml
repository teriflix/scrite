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
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"

Item {
    id: root

    width: 50+15
    height: 50

    Item {
        anchors.top: parent.top
        anchors.topMargin: height * 0.05
        anchors.left: parent.left

        width: Math.min(parent.width,parent.height)
        height: width

        Rectangle {
            anchors.centerIn: parent

            color: Runtime.colors.accent.c600.background
            width: Math.min(parent.width,parent.height)
            height: width
            radius: width/2
        }

        Image {
            id: profilePic

            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter

            width: parent.width
            source: Scrite.user.loggedIn ? Scrite.user.info.badgeImageUrl : ""
            smooth: true; mipmap: true

            visible: source != ""
            fillMode: Image.PreserveAspectFit
        }

        Loader {
            active: Scrite.user.unreadMessageCount > 0

            anchors.top: parent.top
            anchors.right: parent.right

            sourceComponent: Rectangle {
                width: Math.max(unreadMessageCountLabel.contentWidth*1.15, unreadMessageCountLabel.contentHeight*1.15)
                height: width; radius: width/2

                color: Runtime.colors.primary.a100.background

                Text {
                    id: unreadMessageCountLabel

                    font.bold: true
                    font.pixelSize: root.height * 0.25

                    anchors.centerIn: parent

                    text: {
                        const count = Scrite.user.unreadMessageCount
                        if(count >= 10)
                            return "9+"
                        return count
                    }
                    color: Runtime.colors.primary.a100.text
                }
            }
        }

        Item {
            width: 1; height: 1
            anchors.centerIn: parent

            Text {
                anchors.top: initials.top
                anchors.left: initials.left
                anchors.margins: 1

                font: initials.font
                text: initials.text
                color: Runtime.colors.primary.c600.background
                opacity: 0.5
            }

            Text {
                id: initials
                anchors.centerIn: parent

                font.bold: true
                font.pixelSize: root.height * 0.3

                color: Scrite.user.info.badgeTextColor
                text: {
                    if(Scrite.user.loggedIn) {
                        const firstName = Scrite.user.info.firstName
                        if(firstName !== "")
                            return firstName.charAt(0).toUpperCase()
                        const lastName = Scrite.user.info.lastName
                        if(lastName !== "")
                            return lastName.charAt(0).toUpperCase()
                        const email = Scrite.user.info.email
                        return email.charAt(0).toUpperCase()
                    }
                    return "S"
                }
            }
        }

        Loader {
            width: 1; height: 1

            anchors.bottom: parent.bottom
            anchors.bottomMargin: parent.height * 0.14
            anchors.horizontalCenter: parent.horizontalCenter

            active: Scrite.user.loggedIn && Scrite.user.info.hasActiveSubscription && Scrite.user.info.subscriptions[0].kind === "trial"

            sourceComponent: Item {
                Text {
                    anchors.centerIn: parent

                    font.pixelSize: root.height * 0.15

                    text: "" + Scrite.user.info.subscriptions[0].daysToUntil
                    color: Runtime.colors.primary.c600.text
                }
            }
        }

        BusyIcon {
            visible: Scrite.user.busy
            running: Scrite.user.busy
            anchors.centerIn: parent
            forDarkBackground: true
        }

        MouseArea {
            ToolTip.text: Scrite.user.loggedIn ? "Account Profile" : "Login"
            ToolTip.visible: containsMouse

            anchors.fill: parent

            cursorShape: Qt.PointingHandCursor
            hoverEnabled: true

            onClicked: {
                let screenName = undefined

                if(Scrite.user.unreadMessageCount > 0)
                    screenName = "Notifications"
                else if(Scrite.user.info.hasActiveSubscription && !Scrite.user.info.hasUpcomingSubscription && Scrite.user.info.subscriptions[0].daysToUntil < 15)
                    screenName = "Subscriptions"

                UserAccountDialog.launch(screenName)
            }
        }
    }
}
