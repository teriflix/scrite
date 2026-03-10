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

    property bool modal: !Scrite.user.info.hasActiveSubscription
    property string title: {
        if(Scrite.user.loggedIn) {
            if(Scrite.user.info.firstName && Scrite.user.info.firstName !== "")
                return "Hi, " + Scrite.user.info.firstName + "."
            if(Scrite.user.info.lastName && Scrite.user.info.lastName !== "")
                return "Hi, " + Scrite.user.info.lastName + "."
        }
        return "Hi, there."
    }

    Component.onCompleted: {
        if(Scrite.user.loggedIn)
            Runtime.showHelpTip("UserProfileDialog")
    }

    PageView {
        id: _userProfilePageView
        anchors.fill: parent

        Announcement.onIncoming: (type, data) => {
                                     if(type === Runtime.announcementIds.userProfileScreenPage) {
                                         currentIndex = Math.max(0, pagesArray.indexOf(data))
                                     }
                                 }

        pagesArray: ["Profile", "Subscriptions", "Installations", "Notifications"]
        currentIndex: Scrite.user.loggedIn && Scrite.user.info.hasActiveSubscription ? 0 : 1
        maxPageListWidth: {
            if(pagesArray.length < 2)
                return 120

            let textMetrics = Qt.createQmlObject("import QtQuick; TextMetrics { }", _userProfilePageView)
            textMetrics.font.pointSize = Runtime.idealFontMetrics.font.pointSize
            textMetrics.text = ( () => {
                                    const options = pagesArray
                                    let ret = ""
                                    options.forEach( (option) => {
                                                        if(option.length > ret.length)
                                                        ret = option
                                                    })
                                    return ret
                                })()

            const ret = textMetrics.advanceWidth + 50
            textMetrics.destroy()

            return ret
        }
        pageContent: {
            switch(_userProfilePageView.currentIndex) {
            case 0: return _userProfilePageComponent
            case 1: return _userSubscriptionsPageComponent
            case 2: return _userInstallationsPageComponent
            case 3: return _userNotificationsPageComponent
            default: break
            }
            return _userProfilePageComponent
        }
        cornerContent: Item {
            Image {
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 10
                anchors.horizontalCenter: parent.horizontalCenter

                width: parent.width-20

                source: "qrc:/images/scrite_discord_button.png"
                fillMode: Image.PreserveAspectFit
                enabled: visible
                mipmap: true

                MouseArea {
                    id: _discordButtonMouseArea
                    anchors.fill: parent

                    ToolTipPopup {
                        text: "Ask questions, post feedback, request features and connect with other Scrite users."
                        visible: _discordButtonMouseArea.hovered
                    }

                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true

                    onClicked: Qt.openUrlExternally("https://www.scrite.io/forum/")
                }
            }
        }
    }

    Component {
        id: _userProfilePageComponent

        UserProfilePage {
            id: _userProfilePage

            height: _userProfilePageView.availablePageContentHeight
        }
    }

    Component {
        id: _userNotificationsPageComponent

        UserNotificationsPage {
            id: _userNotificationsPage

            height: _userProfilePageView.availablePageContentHeight
        }
    }

    Component {
        id: _userSubscriptionsPageComponent

        UserSubscriptionsPage {
            id: _userSubscriptionsPage

            height: Math.max(implicitHeight, _userProfilePageView.availablePageContentHeight)
        }
    }

    Component {
        id: _userInstallationsPageComponent

        UserInstallationsPage {
            id: _userInstallationsPage

            height: _userProfilePageView.availablePageContentHeight
        }
    }
}
