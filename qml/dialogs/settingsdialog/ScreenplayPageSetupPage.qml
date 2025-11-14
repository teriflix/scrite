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
import "qrc:/qml/dialogs"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

Item {
    id: root

    height: layout.height+20

    GridLayout {
        id: layout

        y: 10
        width: parent.width-20
        columns: 2
        rowSpacing: 20
        columnSpacing: 20

        GroupBox {
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: (layout.width-layout.columnSpacing)/2

            label: VclLabel { text: "Paper Size" }

            VclComboBox {
                width: parent.width
                model: Object.typeEnumModel("ScreenplayPageLayout", "PaperSize", root)
                textRole: "enumKey"
                currentIndex: _private.pageSetupSettings.paperSize
                anchors.verticalCenter: parent.verticalCenter
                onActivated: {
                    _private.pageSetupSettings.paperSize = currentIndex
                    Scrite.document.formatting.pageLayout.paperSize = currentIndex
                    Scrite.document.printFormat.pageLayout.paperSize = currentIndex
                }
            }
        }

        GroupBox {
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: (layout.width-layout.columnSpacing)/2

            label: VclLabel { text: "Time Per Page" }

            RowLayout {
                width: parent.width
                spacing: 10

                VclTextField {
                    Layout.preferredWidth: Runtime.sceneEditorFontMetrics.averageCharacterWidth*5

                    label: "Seconds (15 - 300)"
                    labelAlwaysVisible: true
                    text: Scrite.document.printFormat.secondsPerPage
                    validator: IntValidator { bottom: 15; top: 300 }
                    onTextEdited: Scrite.document.printFormat.secondsPerPage = parseInt(text)
                }

                VclLabel {
                    text: "seconds per page."
                }
            }
        }

        GroupBox {
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: (layout.width-layout.columnSpacing)/2

            label: VclLabel { text: "Header" }

            RowLayout {
                width: parent.width
                spacing: 5

                ColumnLayout {
                    Layout.fillWidth: true

                    spacing: 10

                    VclLabel {
                        text: "Left"
                    }

                    VclComboBox {
                        Layout.fillWidth: true

                        model: _private.fieldsModel
                        textRole: "enumKey"
                        currentIndex: _private.pageSetupSettings.headerLeft
                        onActivated: _private.pageSetupSettings.headerLeft = currentIndex
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true

                    spacing: 10

                    VclLabel {
                        text: "Center"
                    }

                    VclComboBox {
                        Layout.fillWidth: true

                        model: _private.fieldsModel
                        textRole: "enumKey"
                        currentIndex: _private.pageSetupSettings.headerCenter
                        onActivated: _private.pageSetupSettings.headerCenter = currentIndex
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true

                    spacing: 10

                    VclLabel {
                        text: "Right"
                    }

                    VclComboBox {
                        Layout.fillWidth: true

                        model: _private.fieldsModel
                        textRole: "enumKey"
                        currentIndex: _private.pageSetupSettings.headerRight
                        onActivated: _private.pageSetupSettings.headerRight = currentIndex
                    }
                }
            }
        }

        GroupBox {
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: (layout.width-layout.columnSpacing)/2

            label: VclLabel { text: "Footer" }

            RowLayout {
                width: parent.width
                spacing: 5

                ColumnLayout {
                    Layout.fillWidth: true

                    spacing: 10

                    VclLabel {
                        text: "Left"
                    }

                    VclComboBox {
                        Layout.fillWidth: true

                        model: _private.fieldsModel
                        textRole: "enumKey"
                        currentIndex: _private.pageSetupSettings.footerLeft
                        onActivated: _private.pageSetupSettings.footerLeft = currentIndex
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true

                    spacing: 10

                    VclLabel {
                        text: "Center"
                    }

                    VclComboBox {
                        Layout.fillWidth: true

                        model: _private.fieldsModel
                        textRole: "enumKey"
                        currentIndex: _private.pageSetupSettings.footerCenter
                        onActivated: _private.pageSetupSettings.footerCenter = currentIndex
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true

                    spacing: 10

                    VclLabel {
                        text: "Right"
                    }

                    VclComboBox {
                        Layout.fillWidth: true

                        model: _private.fieldsModel
                        textRole: "enumKey"
                        currentIndex: _private.pageSetupSettings.footerRight
                        onActivated: _private.pageSetupSettings.footerRight = currentIndex
                    }
                }
            }
        }

        GroupBox {
            Layout.columnSpan: 2
            Layout.preferredWidth: layout.width
            Layout.preferredHeight: watermarkOptionsLayout.height+50 // !!!

            label: VclLabel { text: "Watermark" }

            GridLayout {
                id: watermarkOptionsLayout
                width: parent.width
                columns: 4
                columnSpacing: 10
                rowSpacing: 10
                enabled: Runtime.appFeatures.watermark.enabled

                VclLabel {
                    Layout.alignment: Qt.AlignRight

                    text: "Enable"
                }

                VclCheckBox {
                    text: checked ? "ON" : "OFF"
                    checked: Runtime.appFeatures.watermark.enabled ? _private.pageSetupSettings.watermarkEnabled : true
                    onToggled: _private.pageSetupSettings.watermarkEnabled = checked
                }


                VclLabel {
                    Layout.alignment: Qt.AlignRight

                    text: "Font Family"
                }

                VclButton {
                    id: fontFamilyButton
                    Layout.preferredWidth: 250

                    text: _private.pageSetupSettings.watermarkFont

                    contentItem: VclLabel {
                        text: fontFamilyButton.text
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                        font.family: _private.pageSetupSettings.watermarkFont
                        font.pointSize: Scrite.app.idealFontPointSize
                    }

                    onClicked: FontSelectionDialog.launchWithTitle("Select watermark font", (family) => {
                                                                        if(family !== "")
                                                                            _private.pageSetupSettings.watermarkFont = family
                                                                   }, _private.pageSetupSettings.watermarkFont)
                }

                VclLabel {
                    Layout.alignment: Qt.AlignRight

                    text: "Text"
                }

                VclTextField {
                    Layout.preferredWidth: 250

                    text: Runtime.appFeatures.watermark.enabled ?  _private.pageSetupSettings.watermarkText : "Scrite"
                    onTextEdited:  _private.pageSetupSettings.watermarkText = text
                    enabled: _private.pageSetupSettings.watermarkEnabled
                    enableTransliteration: true
                }

                VclLabel {
                    Layout.alignment: Qt.AlignRight

                    text: "Font Size"
                }

                SpinBox {
                    Layout.preferredWidth: 250
                    from: 9; to: 200; stepSize: 1
                    editable: true
                    value:  _private.pageSetupSettings.watermarkFontSize
                    onValueModified:  _private.pageSetupSettings.watermarkFontSize = value
                    enabled:  _private.pageSetupSettings.watermarkEnabled
                }

                VclLabel {
                    Layout.alignment: Qt.AlignRight

                    text: "Color"
                }

                Rectangle {
                    Layout.preferredWidth: 30
                    Layout.preferredHeight: 30

                    border.width: 1
                    border.color: Runtime.colors.primary.borderColor
                    color: _private.pageSetupSettings.watermarkColor
                    enabled: _private.pageSetupSettings.watermarkEnabled

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: _private.pageSetupSettings.watermarkColor = Color.pick(_private.pageSetupSettings.watermarkColor)
                    }
                }

                VclLabel {
                    Layout.alignment: Qt.AlignRight
                    text: "Rotation"
                }

                SpinBox {
                    Layout.preferredWidth: 250
                    from: -180; to: 180
                    editable: true
                    value: _private.pageSetupSettings.watermarkRotation
                    textFromValue: function(value,locale) { return value + " degrees" }
                    validator: IntValidator { top: 360; bottom: 0 }
                    onValueModified: _private.pageSetupSettings.watermarkRotation = value
                    enabled: _private.pageSetupSettings.watermarkEnabled
                }
            }

            // Refactoring QML
            DisabledFeatureNotice {
                anchors.fill: parent
                visible: !Runtime.appFeatures.watermark.enabled
                color: Qt.rgba(1,1,1,0.9)
                featureName: "Watermark Settings"
            }
        }

        RowLayout {
            Layout.columnSpan: 2
            Layout.alignment: Qt.AlignHCenter
            spacing: 10

            VclButton {
                text: "Save As Default"
                onClicked: _private.pageSetupSettings.saveAsDefaults()
                enabled: !_private.pageSetupSettings.usingSavedDefaults
            }

            VclButton {
                text: "Use Saved Defaults"
                onClicked: _private.pageSetupSettings.useSavedDefaults()
                enabled: !_private.pageSetupSettings.usingSavedDefaults
            }

            VclButton {
                text: "Use Factory Defaults"
                onClicked: _private.pageSetupSettings.useFactoryDefaults()
                enabled: !_private.pageSetupSettings.usingFactoryDefaults
            }
        }
    }

    QtObject {
        id: _private

        readonly property var fieldsModel: Object.typeEnumModel("HeaderFooter", "Field", _private)
        readonly property PageSetup pageSetupSettings: Scrite.document.pageSetup
        readonly property var systemFontInfo: Scrite.app.systemFontInfo()
    }
}
