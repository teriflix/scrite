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

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"

Item {
    id: root

    parent: Scrite.window.contentItem

    function launch() {

        var dlg = dialogComponent.createObject(root)
        if(dlg) {
            dlg.closed.connect(dlg.destroy)
            dlg.open()
            return dlg
        }

        console.log("Couldn't launch SplashScreen")
        return null
    }

    Component {
        id: dialogComponent

        Dialog {
            id: dialog

            parent: Overlay.overlay
            anchors.centerIn: parent

            modal: true
            width: Math.min(Scrite.window.width*0.7, _dialogPrivate.splashImageSize.width)
            height: width / _dialogPrivate.aspectRatio
            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

            topPadding: 0
            leftPadding: 0
            rightPadding: 0
            bottomPadding: 0

            topMargin: 0
            leftMargin: 0
            rightMargin: 0
            bottomMargin: 0

            topInset: 0
            leftInset: 0
            rightInset: 0
            bottomInset: 0

            background: Rectangle {
                color: Runtime.colors.primary.c100.background
                BoxShadow {
                    anchors.fill: parent
                }
            }

            contentItem: Image {
                id: splashImage
                source: "qrc:/images/splash.jpg"
                smooth: true; mipmap: true
                asynchronous: false

                VclText {
                    id: versionText

                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.topMargin: parent.height * (_dialogPrivate.scriteUrlTextPos.y / _dialogPrivate.splashImageSize.height)
                    anchors.rightMargin: parent.width * (_dialogPrivate.scriteUrlTextPos.x / _dialogPrivate.splashImageSize.width)

                    color: "white"
                    font.bold: true
                    font.pixelSize: parent.height * (_dialogPrivate.scriteUrlFontPixelSize / _dialogPrivate.splashImageSize.height)

                    text: Scrite.app.applicationVersion
                }

                ParallelAnimation {
                    running: Runtime.applicationSettings.enableAnimations

                    NumberAnimation {
                        target: versionText
                        property: "opacity"
                        duration: 500
                        easing.type: Easing.OutBack
                        from: 0; to: 0.8
                    }

                    NumberAnimation {
                        target: versionText
                        property: "font.letterSpacing"
                        duration: 1500
                        easing.type: Easing.OutBack
                        from: 10; to: 0
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: dialog.close()
                }

                Component.onCompleted: Utils.execLater(splashImage, 5000, dialog.close)
            }

            QtObject {
                id: _dialogPrivate

                readonly property size splashImageSize: Qt.size(1880, 800)
                readonly property real aspectRatio: splashImageSize.width / splashImageSize.height
                readonly property point scriteUrlTextPos: Qt.point(35, 750)
                readonly property real scriteUrlFontPixelSize: 24
            }
        }
    }
}
