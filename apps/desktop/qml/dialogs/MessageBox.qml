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
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import io.scrite.components

import "../globals"
import "../controls"

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
        let dlg = _dialogComponent.createObject(root, params)
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
        let dlg = _dialogComponent.createObject(root, params)
        if(dlg) {
            if(callback)
                dlg.buttonClicked.connect(callback)
            dlg.closed.connect(dlg.destroy)
            dlg.open()
        }

        return dlg
    }

    Component {
        id: _dialogComponent

        VclDialog {
            id: _dialog

            property string message
            property var buttons: ["Ok"]

            signal buttonClicked(string buttonText)

            width: Math.min(500, Scrite.window.width * 0.5)
            height: _layout.implicitHeight + 40 + header.height

            titleBarButtons: null
            titleBarCloseButtonVisible: false

            Connections {
                target: root

                function onDiscardMessageBoxes() {
                    Qt.callLater(_dialog.close)
                }
            }

            ActionHandler {
                action: _dialog.acceptAction
                enabled: _dialog.buttons.length === 1 || (_dialog.buttons.indexOf("Yes") >= 0)

                onTriggered: {
                    const buttonIndex = _dialog.buttons.length > 1 ? _dialog.buttons.indexOf("Yes") : 0
                    _dialog.buttonClicked(_dialog.buttons[buttonIndex])
                    Qt.callLater(_dialog.close)
                }
            }

            ActionHandler {
                action: Action {
                    enabled: ActionHandler.canHandle
                    shortcut: Gui.shortcut(Qt.Key_N)
                }

                enabled: _dialog.buttons.length > 1 && _dialog.buttons.indexOf("No") >= 0

                onTriggered: {
                    const buttonIndex = _dialog.buttons.indexOf("No")
                    _dialog.buttonClicked(_dialog.buttons[buttonIndex])
                    Qt.callLater(_dialog.close)
                }
            }

            ActionHandler {
                action: Action {
                    enabled: ActionHandler.canHandle
                    shortcut: Gui.shortcut(Qt.Key_Escape)
                }

                enabled: _dialog.buttons.length > 1 && _dialog.buttons.indexOf("Cancel") >= 0

                onTriggered: {
                    const buttonIndex = _dialog.buttons.indexOf("Cancel")
                    _dialog.buttonClicked(_dialog.buttons[buttonIndex])
                    Qt.callLater(_dialog.close)
                }
            }

            contentItem: Item {
                ColumnLayout {
                    id: _layout
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 20

                    VclLabel {
                        Layout.fillWidth: true
                        horizontalAlignment: lineCount > 3 ? Text.AlignLeft : Text.AlignHCenter
                        text: _dialog.message
                        wrapMode: Text.WordWrap
                    }

                    RowLayout {
                        Layout.alignment: Qt.AlignHCenter
                        spacing: 20

                        Repeater {
                            model: _dialog.buttons

                            delegate: VclButton {
                                id: _button

                                required property string index
                                required property string modelData

                                text: _button.modelData

                                onClicked: {
                                    _dialog.buttonClicked(_button.text)
                                    Qt.callLater(_dialog.close)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
