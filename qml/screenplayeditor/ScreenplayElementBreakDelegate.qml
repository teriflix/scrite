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

Rectangle {
    id: root

    property alias font: _titleField.font

    required property rect documentMargins
    required property ScreenplayElement screenplayElement

    // width is supplied by the ListView in which this delegate is shown
    height: _layout.height + _private.fontMetrics.lineSpacing*0.1

    color: Runtime.colors.primary.c10.background

    RowLayout {
        id: _layout

        anchors.verticalCenter: parent

        width: parent.width

        spacing: 10

        TextField {
            Layout.preferredWidth: documentMargins.left

            text: {
                if(screenplayElement.breakType === Screenplay.Episode)
                    return "Ep " + (screenplayElement.episodeIndex+1)
                return screenplayElement.breakTitle
            }
            font: root.font
            opacity: screenplayElement.breakSubtitle.length > 0 ? 1 : 0
            readOnly: true
            maximumLength: 5
            horizontalAlignment: Text.AlignRight

            background: Item { }
        }

        VclTextField {
            id: _titleField

            Layout.fillWidth: true

            text: screenplayElement.breakSubtitle
            label: ""
            placeholderText: screenplayElement.breakTitle
            enableTransliteration: true

            onTextEdited: screenplayElement.breakSubtitle = text
            onEditingComplete: screenplayElement.breakSubtitle = text
        }

        Item {
            Layout.preferredWidth: documentMargins.right

            FlatToolButton {
                ToolTip.text: {
                    if(screenplayElement.breakType === Screenplay.Episode)
                        return "Deletes this episode break."
                    return "Deletes this act break."
                }

                anchors.verticalCenter: parent.verticalCenter

                width: _private.fontMetrics.lineSpacing
                height: _private.fontMetrics.lineSpacing

                iconSource: "qrc:/icons/action/delete.png"

                onClicked: Runtime.screenplayAdapter.screenplay.removeElement(screenplayElement)
            }
        }
    }

    QtObject {
        id: _private

        property real paperWidth: (root.width - documentMargins.left - documentMargins.right)

        property FontMetrics fontMetrics: FontMetrics {
            font: _titleField.font
        }
    }
}
