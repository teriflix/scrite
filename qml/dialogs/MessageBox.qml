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

Item {
    id: root

    parent: Scrite.window.contentItem

    /**
      Usage:

      MessageBox.information("some title ...", "some message ....", () => { ... })

      This function shows a dialog box with title, message and Ok button. The callback
      will be invoked whenever the user discards the dialog box by clicking on the Ok
      button.
      */
    function information(title, message, callback) {
        const params = {
            "title": title,
            "message": message
        }
        var msgDlg = messageDialogComponent.createObject(root, params)
        if(msgDlg) {
            if(callback)
                msgDlg.buttonClicked.connect(callback)
            msgDlg.closed.connect(msgDlg.destroy)
            msgDlg.open()
        }
    }

    /**
      Usage:

      MessageBox.question("some title ...", "some question ...?", ["Yes", "No", "Cancel"], (buttonText) => {
            if(buttonText === "Yes")
                ...
            else if(buttonText === "No")
                ...
            else if(buttonText === "Cancel")
                ...
        })
      */
    function question(title, question, answerButtons, callback) {
        const params = {
            "title": title,
            "message": question,
            "buttons": answerButtons ? answerButtons : ["Yes", "No", "Cancel"]
        }
        var msgDlg = messageDialogComponent.createObject(root, params)
        if(msgDlg) {
            if(callback)
                msgDlg.buttonClicked.connect(callback)
            msgDlg.closed.connect(msgDlg.destroy)
            msgDlg.open()
        }
    }

    Component {
        id: messageDialogComponent

        VclDialog {
            id: messageDialog

            property string message
            property var buttons: ["Ok"]

            signal buttonClicked(string buttonText)

            width: Math.min(500, Scrite.window.width * 0.5)
            height: layout.implicitHeight + 40 + header.height

            titleBarButtons: null

            contentItem: Item {
                ColumnLayout {
                    id: layout
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 20

                    VclText {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        text: messageDialog.message
                        wrapMode: Text.WordWrap
                    }

                    RowLayout {
                        Layout.alignment: Qt.AlignHCenter
                        spacing: 20

                        Repeater {
                            model: messageDialog.buttons

                            VclButton {
                                id: button

                                required property string modelData
                                text: modelData

                                onClicked: {
                                    messageDialog.buttonClicked(text)
                                    Qt.callLater(messageDialog.close)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
