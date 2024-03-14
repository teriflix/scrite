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
import "qrc:/qml/helpers"

Item {
    id: root

    ColumnLayout {
        id: layout

        anchors.fill: parent
        anchors.margins: 15
        anchors.leftMargin: 0

        spacing: 10

        GroupBox {
            Layout.fillWidth: true

            label: VclText {
                text: "Screenplay Editor"
            }

            GridLayout {
                width: parent.width
                columns: 2
                rowSpacing: 10
                columnSpacing: 10

                VclCheckBox {
                    Layout.preferredWidth: (parent.width-parent.columnSpacing) / parent.columns

                    text: "Spell Check"
                    checked: Runtime.screenplayEditorSettings.enableSpellCheck
                    onToggled: Runtime.screenplayEditorSettings.enableSpellCheck = checked
                }

                VclCheckBox {
                    Layout.preferredWidth: (parent.width-parent.columnSpacing) / parent.columns

                    text: "Auto Complete on Single Click"
                    checked: Runtime.screenplayEditorSettings.singleClickAutoComplete
                    onToggled: Runtime.screenplayEditorSettings.singleClickAutoComplete = checked
                    hoverEnabled: true

                    ToolTip.text: "If checked, single click on an option in auto-complete popup will apply it in the screenplay editor."
                    ToolTip.visible: hovered
                }

                VclCheckBox {
                    Layout.preferredWidth: (parent.width-parent.columnSpacing) / parent.columns

                    text: "Capitalize Sentences"
                    checked: Runtime.screenplayEditorSettings.enableAutoCapitalizeSentences
                    onToggled: Runtime.screenplayEditorSettings.enableAutoCapitalizeSentences = checked
                    hoverEnabled: true

                    ToolTip.text: "If checked, it automatically capitalizes first letter of every sentence while typing."
                    ToolTip.visible: hovered
                }

                VclCheckBox {
                    Layout.preferredWidth: (parent.width-parent.columnSpacing) / parent.columns

                    text: "Add/Remove CONT'D"
                    checked: Runtime.screenplayEditorSettings.enableAutoPolishParagraphs
                    onToggled: Runtime.screenplayEditorSettings.enableAutoPolishParagraphs = checked
                    hoverEnabled: true

                    ToolTip.text: "If checked, CONT'D will be automatically added/removed appropriately."
                    ToolTip.visible: hovered
                }

                VclCheckBox {
                    Layout.preferredWidth: (parent.width-parent.columnSpacing) / parent.columns

                    text: "Auto Adjust Editor Width"
                    checked: Runtime.screenplayEditorSettings.autoAdjustEditorWidthInScreenplayEditor
                    onToggled: Runtime.screenplayEditorSettings.autoAdjustEditorWidthInScreenplayEditor = checked
                    hoverEnabled: true

                    ToolTip.text: "If checked, the editor width is automatically adjusted when you first launch Scrite or switch back to the screenplay tab."
                    ToolTip.visible: hovered
                }

                VclCheckBox {
                    Layout.preferredWidth: (parent.width-parent.columnSpacing) / parent.columns

                    text: "Smooth Scrolling"
                    checked: Runtime.screenplayEditorSettings.optimiseScrolling
                    onToggled: Runtime.screenplayEditorSettings.optimiseScrolling = checked
                    hoverEnabled: true

                    ToolTip.visible: hovered
                    ToolTip.text: "Checking this option will make scrolling in screenplay editor smooth, but uses a lot of RAM and can cause application to freeze at times while scrolling is being computed."
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
