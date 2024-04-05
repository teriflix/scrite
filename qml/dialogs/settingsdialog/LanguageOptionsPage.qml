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
    height: layout.height + 2*layout.margin

    ColumnLayout {
        id: layout

        readonly property real margin: 10

        width: parent.width-margin
        y: margin

        spacing: 10

        GroupBox {
            Layout.fillWidth: true
            Layout.preferredHeight: 250

            label: VclLabel {
                text: "Active Languages"
                font.pointSize: Runtime.idealFontMetrics.font.pointSize
            }

            GridLayout {
                id: activeLanguagesView
                anchors.fill: parent
                columns: 3

                Repeater {
                    model: GenericArrayModel {
                        array: Scrite.app.transliterationEngine.getLanguages()
                        objectMembers: ["key", "value", "active", "current"]
                    }

                    VclCheckBox {
                        required property string key
                        required property int value
                        required property bool active
                        required property bool current

                        checkable: true
                        checked: active
                        text: key
                        onToggled: Scrite.app.transliterationEngine.markLanguage(value,checked)
                    }
                }
            }
        }
    }
}
