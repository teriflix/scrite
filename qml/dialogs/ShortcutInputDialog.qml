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
import "qrc:/qml/helpers"

DialogLauncher {
    id: root

    function launch(shortcut, description, callback) {
        let dialog = doLaunch({"shortcut": shortcut, "description": description})
        if(callback) {
            dialog.shortcutEdited.connect(callback)
        }
        return dialog
    }

    parent: Scrite.window.contentItem

    name: "ShortcutInputDialog"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: _dialog

        readonly property string delimiter: " + "
        readonly property string fontFamily: {
            // We need ZERO and the letter O to be rendered distinctly
            // We also need small-L and capital-I and digit-1 to look disctinct.
            switch(Platform.type) {
            case Platform.WindowsDesktop: return "Consolas"
            case Platform.MacOSDesktop: return "Monaco"
            case Platform.LinuxDesktop: return "DejaVu Sans Mono"
            }
            return "Courier Prime"
        }

        required property string shortcut
        required property string description

        property var keyCombinations: Gui.keyCombinations(shortcut)

        signal shortcutEdited(string newShortcut)

        width: 580
        height: 400

        title: "Change Shortcut"
        closePolicy: Popup.NoAutoClose
        titleBarCloseButtonVisible: false

        content: FocusScope {
            Component.onCompleted: forceActiveFocus()

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20

                spacing: 20

                VclLabel {
                    Layout.fillWidth: true

                    font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2

                    horizontalAlignment: Text.AlignHCenter
                    text: _dialog.description
                    wrapMode: Text.WordWrap
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    color: Qt.rgba(0,0,0,0)
                    border.width: 1
                    border.color: Runtime.colors.primary.borderColor

                    ColumnLayout {
                        anchors.centerIn: parent

                        width: parent.width-40

                        spacing: 20

                        RowLayout {
                            Layout.alignment: Qt.AlignHCenter

                            VclLabel {
                                Layout.alignment: Qt.AlignBaseline

                                padding: 8
                                visible: text !== ""

                                font.family: Runtime.shortcutFontMetrics.font.family
                                font.pointSize: Runtime.shortcutFontMetrics.font.pointSize + 4

                                text: {
                                    let comps = []
                                    if(_dialog.keyCombinations.metaModifier)
                                        comps.push(Platform.modifierDescription(Qt.MetaModifier))
                                    if(_dialog.keyCombinations.controlModifier)
                                        comps.push(Platform.modifierDescription(Qt.ControlModifier))
                                    if(_dialog.keyCombinations.altModifier)
                                        comps.push(Platform.modifierDescription(Qt.AltModifier))
                                    if(_dialog.keyCombinations.shiftModifier)
                                        comps.push(Platform.modifierDescription(Qt.ShiftModifier))
                                    if(comps.length > 0) {
                                        return comps.join(_dialog.delimiter) + _dialog.delimiter
                                    }
                                    return ""
                                }
                            }

                            TextField {
                                id: _keysField

                                Layout.alignment: Qt.AlignBaseline

                                Layout.preferredWidth: Math.max(Runtime.shortcutFontMetrics.averageCharacterWidth*8, contentWidth)

                                EventFilter.target: Scrite.app
                                EventFilter.events: [EventFilter.KeyPress,EventFilter.KeyRelease]
                                EventFilter.onFilter: (object,event,result) => {
                                    if(event.type === EventFilter.KeyPress) {
                                        if(event.modifiers !== Qt.NoModifier) {
                                            if(event.key === Qt.Key_Escape) {
                                                _dialog.close()
                                            } else {
                                                event.accept = false
                                                result.acceptEvent = false
                                                result.filter = false
                                            }
                                            return
                                        }

                                        const ks = Gui.shortcut(event.key)
                                        const kc = Gui.keyCombinations(ks)
                                        text = kc.keys.join(_dialog.delimiter)

                                        let newKc = _dialog.keyCombinations
                                        newKc.keys = kc.keys
                                        newKc.keyCodes = kc.keyCodes
                                        _dialog.keyCombinations = newKc

                                        _keysFieldColorAnimation.start()
                                    }

                                    event.accept = true
                                    result.acceptEvent = true
                                    result.filter = true
                                }

                                text: _dialog.keyCombinations.keys.join(_dialog.delimiter)
                                readOnly: true
                                focus: true

                                horizontalAlignment: Text.AlignHCenter
                                font.pointSize: Runtime.shortcutFontMetrics.font.pointSize + 4
                                font.family: Runtime.shortcutFontMetrics.font.family

                                SequentialAnimation {
                                    id: _keysFieldColorAnimation

                                    ScriptAction {
                                        script: _keysField.font.bold = true
                                    }

                                    ParallelAnimation {
                                        NumberAnimation {
                                            target: _keysField
                                            properties: "font.pointSize"
                                            from: Runtime.shortcutFontMetrics.font.pointSize + 4
                                            to: Runtime.shortcutFontMetrics.font.pointSize + 8
                                        }

                                        ColorAnimation {
                                            target: _keysField
                                            properties: "color"
                                            from: "black"
                                            to: "red"
                                            duration: Runtime.stdAnimationDuration
                                        }
                                    }

                                    PauseAnimation {
                                        duration: Runtime.stdAnimationDuration/2
                                    }

                                    ParallelAnimation {
                                        NumberAnimation {
                                            target: _keysField
                                            properties: "font.pointSize"
                                            from: Runtime.shortcutFontMetrics.font.pointSize + 8
                                            to: Runtime.shortcutFontMetrics.font.pointSize + 4
                                        }

                                        ColorAnimation {
                                            target: _keysField
                                            properties: "color"
                                            from: "red"
                                            to: "black"
                                            duration: Runtime.stdAnimationDuration
                                        }
                                    }

                                    ScriptAction {
                                        script: _keysField.font.bold = false
                                    }
                                }
                            }
                        }

                        RowLayout {
                            Layout.alignment: Qt.AlignCenter

                            ModifierButton {
                                modifier: Qt.MetaModifier
                                checked: _dialog.keyCombinations.metaModifier

                                onToggled: {
                                    let newKc = _dialog.keyCombinations
                                    newKc.metaModifier = !checked
                                    _dialog.keyCombinations = newKc
                                }
                            }

                            ModifierButton {
                                modifier: Qt.ControlModifier
                                checked: _dialog.keyCombinations.controlModifier

                                onToggled: {
                                    let newKc = _dialog.keyCombinations
                                    newKc.controlModifier = !checked
                                    _dialog.keyCombinations = newKc
                                }
                            }

                            ModifierButton {
                                modifier: Qt.AltModifier
                                checked: _dialog.keyCombinations.altModifier

                                onToggled: {
                                    let newKc = _dialog.keyCombinations
                                    newKc.altModifier = !checked
                                    _dialog.keyCombinations = newKc
                                }
                            }

                            ModifierButton {
                                modifier: Qt.ShiftModifier
                                checked: _dialog.keyCombinations.shiftModifier

                                onToggled: {
                                    let newKc = _dialog.keyCombinations
                                    newKc.shiftModifier = !checked
                                    _dialog.keyCombinations = newKc
                                }
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter

                    spacing: parent.spacing

                    VclButton {
                        visible: _dialog.shortcut !== ""
                        text: "Revert"
                        toolTipText: "Reverts to " + _dialog.shortcut
                        enabled: visible && _dialog.keyCombinations.toShortcut() !== _dialog.shortcut

                        focusPolicy: Qt.NoFocus

                        onClicked: {
                            const kc = Gui.keyCombinations(_dialog.shortcut)
                            _dialog.keyCombinations = kc
                            _keysField.text = kc.keys.join(_dialog.delimiter)
                        }
                    }

                    VclButton {
                        text: "Ok"

                        focusPolicy: Qt.NoFocus

                        onClicked: {
                            const newShortcut = _dialog.keyCombinations.toShortcut()
                            _dialog.shortcutEdited(newShortcut)
                            _dialog.close()
                        }
                    }

                    VclButton {
                        text: "Cancel"

                        focusPolicy: Qt.NoFocus

                        onClicked: {
                            _dialog.close()
                        }
                    }
                }
            }
        }
    }

    component ModifierButton: Rectangle {
        required property int modifier
        property bool checked: false

        signal toggled()

        implicitWidth: Math.max(_modifierDesc.contentWidth, 100)
        implicitHeight: _modifierDesc.height

        radius: 8
        color: checked ? Runtime.colors.primary.c600.background : Runtime.colors.primary.c200.background

        border.width: 1
        border.color: Runtime.colors.primary.c700.background

        Text {
            id: _modifierDesc

            anchors.horizontalCenter: parent.horizontalCenter

            text: Platform.modifierDescription(parent.modifier) + (parent.checked ? " ✓" : "")
            color: Color.textColorFor(parent.color)
            padding: 12

            font.bold: parent.checked
            font.family: Runtime.shortcutFontMetrics.font.family
            font.pointSize: Runtime.shortcutFontMetrics.font.pointSize
        }

        MouseArea {
            anchors.fill: parent

            onClicked: parent.toggled()
        }
    }
}
