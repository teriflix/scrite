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
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"

Item {
    id: root

    readonly property int e_CurrentDocumentTarget: 0
    readonly property int e_DefaultGlobalTarget: 1
    property int target: e_DefaultGlobalTarget

    ColumnLayout {
        id: layout

        anchors.fill: parent
        anchors.margins: 15
        anchors.leftMargin: 0

        spacing: 20

        VclLabel {
            Layout.fillWidth: true

            font.bold: true
            wrapMode: Text.WordWrap

            text: target === e_CurrentDocumentTarget ? "Fields on index cards in the currently open document" : "Default fields on index cards in all new documents created in the future"
        }

        VclLabel {
            Layout.fillWidth: true

            wrapMode: Text.WordWrap
            bottomPadding: parent.spacing

            text: "Customise fields that show up on the index cards."
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: parent.spacing
            Layout.rightMargin: parent.spacing

            GridLayout {
                width: parent.width

                flow: GridLayout.TopToBottom
                rows: indexCardFieldsModel.count + 1
                columns: 3
                columnSpacing: 20

                // Column 1: labels
                VclLabel {
                    font.bold: true

                    text: "Label"
                }

                Repeater {
                    model: indexCardFieldsModel

                    VclTextField {
                        required property int index
                        required property string name

                        Layout.preferredWidth: Runtime.idealFontMetrics.averageCharacterWidth * (maximumLength+2)

                        text: name
                        onTextChanged: indexCardFieldsModel.setProperty(index, "name", text)

                        maximumLength: 5

                        TabSequenceItem.manager: tabSequenceManager
                        TabSequenceItem.sequence: index * 2
                    }
                }

                // Column 2: description
                VclLabel {
                    Layout.fillWidth: true

                    font.bold: true

                    text: "Description"
                }

                Repeater {
                    model: indexCardFieldsModel

                    VclTextField {
                        required property int index
                        required property string description

                        Layout.fillWidth: true
                        text: description
                        onTextChanged: indexCardFieldsModel.setProperty(index, "description", text)

                        TabSequenceItem.manager: tabSequenceManager
                        TabSequenceItem.sequence: index * 2 + 1
                    }
                }

                // Column 3: delete icon
                Item {
                    Layout.preferredWidth: 10
                    Layout.preferredHeight: 10
                }

                Repeater {
                    model: indexCardFieldsModel

                    VclToolButton {
                        required property int index

                        icon.source: "qrc:/icons/action/delete.png"

                        onClicked: indexCardFieldsModel.remove(index, 1)
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true

            spacing: 20

            VclButton {
                text: "Help"
                onClicked: MessageBox.information("Sorry!", "No help information available for this feature yet.")
            }

            Item {
                Layout.fillWidth: true
            }

            VclButton {
                visible: target === e_CurrentDocumentTarget
                enabled: JSON.stringify(Scrite.document.structure.defaultIndexCardFields) !== JSON.stringify(indexCardFieldsModel.array)

                text: "Use Defaults"
                ToolTip.text: "Click this button to use your global default index card fields in this document."

                onClicked: {
                    if(Scrite.document.structure.indexCardFields.length > 0) {
                        MessageBox.question("Confirmation",
                                            "Are you sure you want to replace index card fields in this document with global defaults set on this computer?",
                                            ["Yes", "No"],
                                            (answer) => {
                                                if(answer === "Yes")
                                                    useDefaults()
                                            })
                    } else
                        useDefaults()
                }


                function useDefaults() {
                    Scrite.document.structure.indexCardFields = Scrite.document.structure.defaultIndexCardFields
                    indexCardFieldsModel.reset()
                }
            }

            VclButton {
                enabled: indexCardFieldsModel.count < indexCardFieldsModel.maxCount

                text: "Add Field"

                onClicked: {
                    const newItem = {"name": "", "description": ""}
                    indexCardFieldsModel.append(newItem)
                    Utils.execLater(tabSequenceManager, 50, () => {
                                        tabSequenceManager.assumeFocusAt( (indexCardFieldsModel.count-1)*2 )
                                    } )
                }
            }

            VclButton {
                text: "Revert"
                enabled: indexCardFieldsModel.canReset
                onClicked: indexCardFieldsModel.reset()
            }

            VclButton {
                text: "Apply"
                enabled: indexCardFieldsModel.modified
                onClicked: {
                    indexCardFieldsModel.commit()

                    if(root.target === root.e_DefaultGlobalTarget) {
                        MessageBox.question("Index card fields in the current document",
                                            "Do you want to use these index card fields in the current document as well?",
                                            ["Yes", "No"],
                                            (answer) => {
                                                if(answer === "Yes") {
                                                    Scrite.document.structure.indexCardFields = Scrite.document.structure.defaultIndexCardFields
                                                }
                                            })
                    }
                }
            }
        }
    }

    // Private stuff
    TabSequenceManager {
        id: tabSequenceManager
        wrapAround: true
    }

    GenericArrayModel {
        id: indexCardFieldsModel

        readonly property int maxCount: 5

        property bool modified: false

        property var source: root.target === root.e_CurrentDocumentTarget ? Scrite.document.structure.indexCardFields : Scrite.document.structure.defaultIndexCardFields
        property bool canReset: JSON.stringify(source) !== JSON.stringify(array) // Generally this is expensive, but in here our array size is limited.

        objectMembers: ["name", "description"]

        Component.onCompleted: {
            reset()
            dataChanged.connect(markModified)
            rowsInserted.connect(markModified)
            rowsRemoved.connect(markModified)
            modelReset.connect(markModified)
        }

        function markModified() {
            modified = true
        }

        function reset() {
            array = source
            modified = false
        }

        function commit() {
            if(modified) {
                if(root.target === root.e_CurrentDocumentTarget)
                    Scrite.document.structure.indexCardFields = array
                else
                    Scrite.document.structure.defaultIndexCardFields = array
                array = source
                modified = false
            }
        }
    }
}
