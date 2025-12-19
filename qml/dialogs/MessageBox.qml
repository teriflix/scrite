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

Item {
    id: root

    parent: Scrite.window.contentItem

    signal discardMessageBoxes()

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
        let dlg = dialogComponent.createObject(root, params)
        if(dlg) {
            if(callback)
                dlg.buttonClicked.connect(callback)
            dlg.closed.connect(dlg.destroy)
            dlg.open()
        }

        return dlg
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
        let dlg = dialogComponent.createObject(root, params)
        if(dlg) {
            if(callback)
                dlg.buttonClicked.connect(callback)
            dlg.closed.connect(dlg.destroy)
            dlg.open()
        }

        return dlg
    }

    Component {
        id: dialogComponent

        VclDialog {
            id: dialog

            property string message
            property var buttons: ["Ok"]

            signal buttonClicked(string buttonText)

            width: Math.min(500, Scrite.window.width * 0.5)
            height: layout.implicitHeight + 40 + header.height

            titleBarButtons: null
            titleBarCloseButtonVisible: false

            Connections {
                target: root

                function onDiscardMessageBoxes() {
                    Qt.callLater(dialog.close)
                }
            }

            ActionHandler {
                action: dialog.acceptAction
                enabled: dialog.buttons.length === 1 || (dialog.buttons.indexOf("Yes") >= 0)

                onTriggered: {
                    const buttonIndex = dialog.buttons.length > 1 ? dialog.buttons.indexOf("Yes") : 0
                    dialog.buttonClicked(buttons[buttonIndex])
                    Qt.callLater(dialog.close)
                }
            }

            ActionHandler {
                action: Action {
                    enabled: ActionHandler.canHandle
                    shortcut: Gui.shortcut(Qt.Key_N)
                }

                enabled: dialog.buttons.length > 1 && dialog.buttons.indexOf("No") >= 0

                onTriggered: {
                    const buttonIndex = dialog.buttons.indexOf("No")
                    dialog.buttonClicked(buttons[buttonIndex])
                    Qt.callLater(dialog.close)
                }
            }

            ActionHandler {
                action: Action {
                    enabled: ActionHandler.canHandle
                    shortcut: Gui.shortcut(Qt.Key_Escape)
                }

                enabled: dialog.buttons.length > 1 && dialog.buttons.indexOf("Cancel") >= 0

                onTriggered: {
                    const buttonIndex = dialog.buttons.indexOf("Cancel")
                    dialog.buttonClicked(buttons[buttonIndex])
                    Qt.callLater(dialog.close)
                }
            }

            contentItem: Item {
                ColumnLayout {
                    id: layout
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 20

                    VclLabel {
                        Layout.fillWidth: true
                        horizontalAlignment: lineCount > 3 ? Text.AlignLeft : Text.AlignHCenter
                        text: dialog.message
                        wrapMode: Text.WordWrap
                    }

                    RowLayout {
                        Layout.alignment: Qt.AlignHCenter
                        spacing: 20

                        Repeater {
                            model: dialog.buttons

                            delegate: VclButton {
                                id: button

                                required property string index
                                required property string modelData

                                text: modelData

                                onClicked: {
                                    dialog.buttonClicked(text)
                                    Qt.callLater(dialog.close)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
