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

    function launch(title, list, callback) {
        const props = {
            "title": title,
            "list": list
        }
        let dlg = doLaunch(props)
        if(dlg)
            dlg.itemSelected.connect(callback)
        return dlg
    }

    name: "SelectionListDialog"
    singleInstanceOnly: true

    dialogComponent: VclDialog {
        id: _dialog

        required property var list

        signal itemSelected(string item)

        width: 640
        height: 480

        content: Item {
            Component.onCompleted: {
                _listView.model.filter()
                Qt.callLater(_textField.forceActiveFocus)
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20

                VclTextField {
                    id: _textField

                    Layout.fillWidth: true

                    Keys.onUpPressed: _listView.currentIndex = Math.max(_listView.currentIndex-1, 0)
                    Keys.onDownPressed: _listView.currentIndex = Math.min(_listView.currentIndex+1, _listView.count-1)

                    onTextEdited: {
                        _listView.model.filter()
                        _listView.currentIndex = 0
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    color: Runtime.colors.primary.c10.transparent
                    border.color: Runtime.colors.primary.borderColor
                    border.width: 1

                    ListView {
                        id: _listView

                        ScrollBar.vertical: VclScrollBar { }

                        anchors.fill: parent
                        anchors.margins: 1

                        clip: true

                        highlight: Rectangle {
                            color: Runtime.colors.primary.highlight.background
                        }
                        highlightMoveDuration: 0
                        highlightResizeDuration: 0

                        keyNavigationWraps: false
                        keyNavigationEnabled: true

                        boundsBehavior: ListView.StopAtBounds
                        boundsMovement: ListView.StopAtBounds

                        model: ListModel {
                            function filter() {
                                const text = _textField.text.toLowerCase()
                                clear()

                                const list = _dialog.list
                                for(let i=0; i<list.length; i++) {
                                    const item = list[i].toLowerCase()
                                    if(item.startsWith(text)) {
                                        append({"item": list[i]})
                                    }
                                }
                            }
                        }

                        delegate: VclLabel {
                            required property int index
                            required property string item

                            width: _listView.width
                            padding: 4
                            text: item

                            MouseArea {
                                anchors.fill: parent
                                onClicked: _listView.currentIndex = index
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true

                    VclLabel {
                        Layout.alignment: Qt.AlignVCenter
                        Layout.fillWidth: true

                        text: _listView.count + " item(s)"
                    }

                    VclButton {
                        text: "Select"

                        onClicked: _dialog.acceptAction.trigger()
                    }
                }
            }

            ActionHandler {
                action: _dialog.acceptAction
                enabled: _listView.count > 0 && _listView.currentIndex >= 0

                onTriggered: (source) => {
                    const text = _listView.currentItem ? _listView.currentItem.text : ""
                    _dialog.itemSelected(text)
                    _dialog.close()
                }
            }
        }
    }
}
