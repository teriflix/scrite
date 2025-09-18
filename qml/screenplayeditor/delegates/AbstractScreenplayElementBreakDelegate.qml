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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

AbstractScreenplayElementDelegate {
    id: root

    content: Rectangle {
        height: _layout.height * 1.2

        color: Runtime.colors.primary.c10.background

        Connections {
            target: root

            function on__focusIn(cursorPosition) {
                _titleField.cursorPosition = cursorPosition < 0 ? _titleField.length : 0
                _titleField.forceActiveFocus()
            }

            function on__focusOut() {
                _titleField.focus = false
            }
        }

        /**
          Not using Row here on purpose.

          The Layout.fillWidth attached property in _titleField makes this part of the code looks
          so much cleaner and maintainable than having to calculate width manually.

          Besides, we won't have too many break delegates anyway.
          */
        RowLayout {
            id: _layout

            anchors.verticalCenter: parent.verticalCenter

            width: parent.width

            spacing: 10

            TextField {
                Layout.preferredWidth: root.pageLeftMargin

                text: {
                    if(root.screenplayElement.breakType === Screenplay.Episode)
                        return "Ep " + (root.screenplayElement.episodeIndex+1)
                    return root.screenplayElement.breakTitle
                }
                font: root.font
                opacity: root.screenplayElement.breakSubtitle.length > 0 ? 1 : 0
                readOnly: true
                maximumLength: 5
                horizontalAlignment: Text.AlignRight

                background: Item { }
            }

            VclTextField {
                id: _titleField

                Layout.fillWidth: true

                text: root.screenplayElement.breakSubtitle
                font: root.font
                label: ""
                focus: true
                placeholderText: root.screenplayElement.breakTitle
                enableTransliteration: true

                onTextEdited: root.screenplayElement.breakSubtitle = text
                onEditingComplete: root.screenplayElement.breakSubtitle = text
            }

            Item {
                Layout.preferredWidth: root.pageRightMargin

                FlatToolButton {
                    ToolTip.text: {
                        let ret = "Deletes this "
                        switch(root.screenplayElement.breakType) {
                        case Screenplay.Act: ret += "act"; break;
                        case Screenplay.Episode: ret += "episode"; break;
                        case Screenplay.Interval: ret += "interval"; break
                        }
                        ret += " break."
                        return ret
                    }

                    anchors.verticalCenter: parent.verticalCenter

                    width: root.fontMetrics.lineSpacing
                    height: root.fontMetrics.lineSpacing

                    iconSource: "qrc:/icons/action/delete.png"

                    onClicked: Runtime.screenplayAdapter.screenplay.removeElement(root.screenplayElement)
                }
            }
        }
    }

    on__FocusIn: () => { }     // TODO
    on__FocusOut: () => { }    // TODO
}
