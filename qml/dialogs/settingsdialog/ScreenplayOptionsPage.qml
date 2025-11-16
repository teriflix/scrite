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


import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"

Item {
    id: root
    height: layout.height+30

    ColumnLayout {
        id: layout

        y: 10
        width: parent.width-20

        spacing: 10

        GroupBox {
            Layout.fillWidth: true

            label: VclLabel {
                text: "Screenplay Editor"
            }

            GridLayout {
                width: parent.width
                columns: 2
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

                    ToolTipPopup {
                        text: "If checked, single click on an option in auto-complete popup will apply it in the screenplay editor."
                        visible: container.hovered
                    }

                }

                VclCheckBox {
                    Layout.preferredWidth: (parent.width-parent.columnSpacing) / parent.columns

                    text: "Capitalize Sentences"
                    checked: Runtime.screenplayEditorSettings.enableAutoCapitalizeSentences
                    onToggled: Runtime.screenplayEditorSettings.enableAutoCapitalizeSentences = checked
                    hoverEnabled: true

                    ToolTipPopup {
                        visible: container.hovered
                        text: "If checked, it automatically capitalizes first letter of every sentence while typing."
                    }
                }

                VclCheckBox {
                    Layout.preferredWidth: (parent.width-parent.columnSpacing) / parent.columns

                    text: "Add/Remove CONT'D"
                    checked: Runtime.screenplayEditorSettings.enableAutoPolishParagraphs
                    onToggled: Runtime.screenplayEditorSettings.enableAutoPolishParagraphs = checked
                    hoverEnabled: true

                    ToolTipPopup {
                        visible: container.hovered
                        text: "If checked, CONT'D will be automatically added/removed appropriately."
                    }
                }

                VclCheckBox {
                    Layout.preferredWidth: (parent.width-parent.columnSpacing) / parent.columns

                    text: "Capture Invisible Characters"
                    checked: Runtime.screenplayEditorSettings.captureInvisibleCharacters
                    onToggled: Runtime.screenplayEditorSettings.captureInvisibleCharacters = checked
                    hoverEnabled: true

                    ToolTipPopup {
                        text: "In a scene if a dialogues are only written in parenthesis (eg: VO, OS, etc..), then the character will be captured as invisible."
                        visible: container.hovered
                    }
                }

                VclCheckBox {
                    Layout.preferredWidth: (parent.width-parent.columnSpacing) / parent.columns

                    text: "Auto Adjust Editor Width"
                    checked: Runtime.screenplayEditorSettings.autoAdjustEditorWidthInScreenplayEditor
                    onToggled: Runtime.screenplayEditorSettings.autoAdjustEditorWidthInScreenplayEditor = checked
                    hoverEnabled: true

                    ToolTipPopup {
                        text: "If checked, the editor width is automatically adjusted when you first launch Scrite or switch back to the screenplay tab."
                        visible: container.hovered
                    }
                }

                RowLayout {
                    Layout.preferredWidth: (parent.width-parent.columnSpacing) / parent.columns

                    VclCheckBox {
                        text: "Long Scene Warning"

                        checked: Runtime.screenplayEditorSettings.longSceneWarningEnabled
                        onToggled: Runtime.screenplayEditorSettings.longSceneWarningEnabled = checked
                    }

                    VclTextField {
                        Layout.fillWidth: true

                        text: Runtime.screenplayEditorSettings.longSceneWordTreshold
                        onTextEdited: Runtime.screenplayEditorSettings.longSceneWordTreshold = parseInt(text)

                        placeholderText: "Words Per Scene Treshold"
                        validator: IntValidator {
                            bottom: 50; top: 1000
                        }
                    }

                    ToolButton {
                        icon.source: "qrc:/icons/action/help.png"

                        onClicked: Qt.openUrlExternally("https://www.scrite.io/advanced-editing-features/#chapter10_writing_with_scene-centric_precision_in_scrite")
                    }
                }

                RowLayout {
                    Layout.preferredWidth: (parent.width-parent.columnSpacing) / parent.columns

                    VclLabel {
                        text: "Max Synopsis Lines: "
                    }

                    SpinBox {
                        Layout.fillWidth: true

                        value: Runtime.screenplayEditorSettings.slpSynopsisLineCount
                        from: 1; to: 5
                        hoverEnabled: true

                        onValueChanged: Runtime.screenplayEditorSettings.slpSynopsisLineCount = value

                        ToolTipPopup{
                            text: "Max lines to show on the scene list panel. Range: " + from + "-" + to
                            visible: container.hovered
                        }
                    }
                }

                RowLayout {
                    Layout.preferredWidth: (parent.width-parent.columnSpacing) / parent.columns

                    VclLabel {
                        text: "Scene Loading Interval: "
                    }

                    SpinBox {
                        Layout.fillWidth: true

                        value: Runtime.screenplayEditorSettings.placeholderInterval
                        from: 50; to: 1000
                        hoverEnabled: true
                        editable: true

                        onValueChanged: Runtime.screenplayEditorSettings.placeholderInterval = Runtime.bounded(from, value, to)

                        ToolTipPopup {
                            visible: container.hovered
                            text: "Delay in ms after which scene content is loaded while scrolling. Range: " + from + "-" + to
                        }
                    }
                }
            }
        }

        GroupBox {
            Layout.fillWidth: true

            label: VclText {
                text: "Copy Options"
            }

            ColumnLayout {
                width: parent.width

                VclCheckBox {
                    Layout.fillWidth: true

                    text: "Copy text in Fountain format."
                    checked: Runtime.screenplayEditorSettings.copyAsFountain
                    onToggled: Runtime.screenplayEditorSettings.copyAsFountain = checked
                }

                VclCheckBox {
                    Layout.fillWidth: true

                    enabled: Runtime.screenplayEditorSettings.copyAsFountain
                    text: "Explicitly mark headings, character, action and transition paragraphs in copied text."
                    checked: Runtime.screenplayEditorSettings.copyFountainUsingStrictSyntax
                    onToggled: Runtime.screenplayEditorSettings.copyFountainUsingStrictSyntax = checked
                }

                VclCheckBox {
                    Layout.fillWidth: true

                    enabled: Runtime.screenplayEditorSettings.copyAsFountain
                    text: "Copy bold, italics and underline formatting."
                    checked: Runtime.screenplayEditorSettings.copyFountainWithEmphasis
                    onToggled: Runtime.screenplayEditorSettings.copyFountainWithEmphasis = checked
                }
            }
        }

        GroupBox {
            Layout.fillWidth: true

            label: VclText {
                text: "Paste Options"
            }

            ColumnLayout {
                width: parent.width

                VclCheckBox {
                    Layout.fillWidth: true

                    text: "Interpret plaintext in Fountain format."
                    checked: Runtime.screenplayEditorSettings.pasteAsFountain
                    onToggled: Runtime.screenplayEditorSettings.pasteAsFountain = checked
                }

                VclCheckBox {
                    Layout.fillWidth: true

                    enabled: Runtime.screenplayEditorSettings.pasteAsFountain
                    text: "Merge adjacent action and dialogue paragraphs."
                    checked: Runtime.screenplayEditorSettings.pasteByMergingAdjacentElements
                    onToggled: Runtime.screenplayEditorSettings.pasteByMergingAdjacentElements = checked
                }

                VclCheckBox {
                    Layout.fillWidth: true

                    enabled: Runtime.screenplayEditorSettings.pasteAsFountain
                    text: "Paste with bold, italics and underline formatting."
                    checked: Runtime.screenplayEditorSettings.pasteAfterResolvingEmphasis
                    onToggled: Runtime.screenplayEditorSettings.pasteAfterResolvingEmphasis = checked
                }

                VclCheckBox {
                    Layout.fillWidth: true

                    text: "When possible, link scenes while pasting."
                    enabled: true
                    checked: Runtime.screenplayEditorSettings.pasteByLinkingScenesWhenPossible
                    hoverEnabled: true

                    onToggled: Runtime.screenplayEditorSettings.pasteByLinkingScenesWhenPossible = checked

                    ToolTipPopup {
                        text: "Copy/pasting within the screenplay editor links scenes, without creating duplicates. Uncheck this to allow creation of duplicate scenes."
                        visible: container.hovered
                    }
                }
            }
        }
    }
}
