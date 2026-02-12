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

pragma Singleton

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"

DialogLauncher {
    id: root

    function launch() {
        if(Scrite.user.canUseAppVersionType)
            return null

        return doLaunch()
    }

    name: "RequestVersionTypeAccess"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: _dialog

        title: "Request Access"

        width: 500
        height: 400
        titleBarCloseButtonVisible: false

        content: Item {
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20

                enabled: !_apiCall.busy
                opacity: enabled ? 1 : 0.5
                spacing: 20

                VclLabel {
                    Layout.fillWidth: true

                    text: "To use this <b>" + Scrite.app.versionType + "</b> version of Scrite, you will need to request access."
                    font.bold: true
                    wrapMode: Text.WordWrap
                }

                VclLabel {
                    Layout.fillWidth: true

                    text: "Please note that access will be granted on a case-by-case basis, only at the discretion of the Scrite team and may be subject to additional terms. You may have to wait for up to 72 hours before you hear from us."
                    wrapMode: Text.WordWrap
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                RowLayout {
                    Layout.alignment: Qt.AlignRight

                    spacing: 20

                    VclButton {
                        text: "Request Access"

                        onClicked: {
                            if(!_apiCall.call()) {
                                _private.showSentMessage()
                            }
                        }
                    }

                    VclButton {
                        text: "Quit"

                        onClicked: {
                            _private.closeMainWindow()
                        }
                    }
                }
            }

            BusyIndicator {
                anchors.centerIn: parent

                running: _apiCall.busy
            }

            UserRequestVersionTypeApiCall {
                id: _apiCall

                onBusyChanged: {
                    Scrite.window.closeButtonVisible = !busy
                }

                onFinished: {
                    if(hasError)
                        _private.showErrorMessage()
                    else
                        _private.showSentMessage()
                }
            }
        }
    }

    QtObject {
        id: _private

        function closeMainWindow() {
            Qt.callLater(() => { Scrite.window.close() })
        }

        function showErrorMessage() {
            MessageBox.information("Error",
                                   "There was an error sending your access request. Please ensure that you have a working Internet connection, and try again later.",
                                   _private.closeMainWindow)
        }

        function showSentMessage() {
            MessageBox.information("Request Sent",
                                   "Your access request was sent. Please allow us up to 72 hours to get back to you. If you don't hear back from us, it probably means that your request was declined.",
                                   _private.closeMainWindow)
        }
    }
}
