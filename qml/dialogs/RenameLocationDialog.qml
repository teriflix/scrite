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

    function launch(location) {
        return doLaunch({"location": location})
    }

    name: "RenameLocationDialog"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: _dialog

        required property string location

        width: 680
        height: 380

        title: "Rename Location"

        content: Item {
            Component.onCompleted: {
                if(_dialog.location !== "") {
                    _originalName.text = _dialog.location
                    _originalName.readOnly = true
                    _dialog.title = "Rename Location: " + _dialog.location
                    Qt.callLater(_newName.forceActiveFocus)
                } else {
                    Qt.callLater(_originalName.forceActiveFocus)
                }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20

                spacing: 20

                ColumnLayout {
                    Layout.fillWidth: true

                    VclLabel {
                        Layout.fillWidth: true

                        text: "Original Name"
                    }

                    VclTextField {
                        id: _originalName

                        Layout.fillWidth: true

                        TabSequenceItem.manager: _tabSequenceManager
                        TabSequenceItem.sequence: 0

                        completionStrings: Scrite.document.structure.allLocations()
                        minimumCompletionPrefixLength: 0

                        font.capitalization: Font.AllUppercase
                    }
                }

                VclLabel {
                    text: " - to - "
                }

                ColumnLayout {
                    Layout.fillWidth: true

                    VclLabel {
                        Layout.fillWidth: true

                        text: "New Name"
                    }

                    VclTextField {
                        id: _newName

                        Layout.fillWidth: true

                        TabSequenceItem.manager: _tabSequenceManager
                        TabSequenceItem.sequence: 1

                        completionStrings: _originalName.completionStrings

                        font.capitalization: Font.AllUppercase
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                VclButton {
                    Layout.alignment: Qt.AlignRight

                    text: "Rename"
                    enabled: _dialog.acceptAction.enabled

                    onClicked: _dialog.acceptAction.trigger()
                }
            }

            TabSequenceManager {
                id: _tabSequenceManager

                wrapAround: true
            }

            ActionHandler {
                action: _dialog.acceptAction
                enabled: _originalName.completionStrings.indexOf(_originalName.text) >= 0 &&
                         _newName.text !== "" && _newName.text.trim() !== _originalName.text &&
                         !_originalName.completionHasSuggestions && !_newName.completionHasSuggestions

                onTriggered: (source) => {
                    const originalName = _originalName.text.toUpperCase()
                    const newName = _newName.text.toUpperCase()
                    const nrHeadings = Scrite.document.structure.renameLocation(originalName, newName)
                    if(nrHeadings > 0) {
                        MessageBox.information("Rename Successful",
                                               _originalName.text + " was changed to " + newName + " in " + nrHeadings + " scene heading(s).",
                                               _dialog.close)
                    }
                }
            }
        }
    }
}
