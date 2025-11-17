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
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"

Popup {
    id: root

    function init(_parent) {
        if( !(_parent && Object.isOfType(_parent, "QQuickItem")) )
            _parent = Scrite.window.contentItem

        parent = _parent
        visible = false
    }

    Material.primary: Runtime.colors.primary.key
    Material.accent: Runtime.colors.accent.key
    Material.theme: Runtime.colors.theme

    anchors.centerIn: parent

    parent: Scrite.window.contentItem

    width: Math.min(640, Scrite.window.width * 0.75)

    modal: false
    closePolicy: Popup.CloseOnPressOutside|Popup.CloseOnEscape

    contentItem: FocusScope {
        implicitHeight: _layout.height + 10

        focus: true

        ColumnLayout {
            id: _layout

            width: parent.width - 10

            spacing: 20

            VclLabel {
                Layout.alignment: Qt.AlignHCenter

                text: "Command Center"

                font.bold: true
                font.pointSize: Runtime.idealFontMetrics.font.pointSize + 4
            }

            TextField {
                id: _commandText

                Layout.fillWidth: true

                Keys.onUpPressed: _actionsView.currentIndex = Math.max(_actionsView.currentIndex-1,0)
                Keys.onDownPressed: _actionsView.currentIndex = Math.min(_actionsView.currentIndex+1, _actionsView.count-1)
                Keys.onEnterPressed: _actionsView.triggerCurrentItem()
                Keys.onReturnPressed: _actionsView.triggerCurrentItem()

                focus: true
                placeholderText: "Search for a command or topic ..."

                font: Runtime.idealFontMetrics.font

                onTextEdited: _actionsModel.filter()
            }

            ListView {
                id: _actionsView

                Layout.fillWidth: true
                Layout.minimumHeight: Scrite.window.height * 0.2
                Layout.maximumWidth: Scrite.window.height * 0.7

                function triggerCurrentItem() {
                    if(currentItem != null) {
                        _private.trigger(currentItem.qmlAction)
                    } else {
                        root.close()
                    }
                }

                model: _actionsModel
                currentIndex: 0

                highlightFollowsCurrentItem: true
                highlightMoveDuration: 0
                highlightResizeDuration: 0

                highlight: Rectangle {
                    color: Runtime.colors.primary.highlight.background
                }

                delegate: Item {
                    id: _delegate

                    required property int index
                    required property var qmlAction
                    required property var actionManager
                    required property bool shortcutIsEditable
                    required property string groupName

                    width: _actionsView.height < _actionsView.contentHeight ? _actionsView.width - 17 : _actionsView.width
                    height: _delegateLayout.height

                    MouseArea {
                        anchors.fill: parent

                        onClicked: {
                            _private.trigger(qmlAction)
                            root.close()
                        }
                    }

                    RowLayout {
                        id: _delegateLayout

                        width: parent.width

                        Image {
                            Layout.leftMargin: 12
                            Layout.preferredHeight: Runtime.iconImageSize
                            Layout.preferredWidth: Runtime.iconImageSize

                            fillMode: Image.PreserveAspectFit
                            source: qmlAction.icon.source !== "" ? qmlAction.icon.source : "qrc:/icons/content/blank.png"
                        }

                        VclLabel {
                            Layout.fillWidth: true

                            elide: Text.ElideRight
                            font: Runtime.idealFontMetrics.font
                            padding: 10
                            text: "<b>" + actionManager.title + "</b>: " + qmlAction.text
                        }

                        Link {
                            Layout.preferredWidth: _delegateLayout.width * 0.3

                            enabled: shortcutIsEditable
                            font: Runtime.idealFontMetrics.font
                            opacity: enabled ? 1 : 0.5
                            text: Gui.nativeShortcut(qmlAction.shortcut)

                            onClicked: {
                                ShortcutEditorDialog.launch()
                            }
                        }
                    }
                }
            }
        }
    }

    ActionsModelFilter {
        id: _actionsModel

        filters: ActionsModelFilter.CommandCenterFilters
        customFilterMode: true

        onFilterRequest: (qmlAction, actionManager, result) => {
            if(_commandText.length === 0)
                result.value = true
            else {
                const givenText = _commandText.text.toLowerCase()
                const text = (actionManager.title + ": " + qmlAction.text).toLowerCase()
                let accept = (text.indexOf(givenText) >= 0)
                if(accept || qmlAction.keywords === undefined) {
                    result.value = accept
                    return
                }

                const keywords = qmlAction.keywords.join(", ")
                result.value = keywords.indexOf(givenText) >= 0
            }
        }

        onModelReset: Qt.callLater(_private.resetCurrentActionViewItem)
        onRowsRemoved: Qt.callLater(_private.resetCurrentActionViewItem)
        onRowsInserted: Qt.callLater(_private.resetCurrentActionViewItem)
    }

    ActionHandler {
        action: ActionHub.applicationOptions.find("commandCenter")

        onTriggered: (source) => {
            root.open()
        }
    }

    onAboutToShow: {
        _commandText.text = ""
        _actionsModel.filter()
        contentItem.forceActiveFocus()
    }

    QtObject {
        id: _private

        function trigger(qmlAction) {
            if(qmlAction.enabled)
                qmlAction.trigger()
            root.close()
        }

        function resetCurrentActionViewItem() {
            _actionsView.currentIndex = _actionsView.count > 0 ? 0 : -1
        }
    }
}
