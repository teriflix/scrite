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

    width: Runtime.bounded(600, Scrite.window.width * 0.75, 800)

    modal: false
    focus: true
    closePolicy: Popup.CloseOnPressOutside|Popup.CloseOnEscape

    contentItem: FocusScope {
        implicitHeight: _layout.height + 10

        focus: true

        ColumnLayout {
            id: _layout

            anchors.centerIn: parent

            width: parent.width - 10

            spacing: 20

            TextField {
                id: _commandText

                Layout.fillWidth: true

                Keys.onUpPressed: _actionsView.currentIndex = Math.max(_actionsView.currentIndex-1,0)
                Keys.onDownPressed: _actionsView.currentIndex = Math.min(_actionsView.currentIndex+1, _actionsView.count-1)
                Keys.onEnterPressed: _actionsView.triggerCurrentItem()
                Keys.onReturnPressed: _actionsView.triggerCurrentItem()

                DiacriticHandler.enabled: Runtime.allowDiacriticEditing && activeFocus

                focus: true
                placeholderText: "Search for a command or topic ..."

                font: Runtime.idealFontMetrics.font

                onTextEdited: {
                    if(_private.actionsModel)
                        _private.actionsModel.filter()
                }

                Connections {
                    target: root

                    function onAboutToShow() {
                        Qt.callLater(_commandText.forceActiveFocus)
                    }
                }
            }

            ListView {
                id: _actionsView

                Layout.fillWidth: true
                Layout.preferredHeight: Scrite.window.height * 0.4

                function triggerCurrentItem() {
                    if(currentItem != null) {
                        _private.trigger(currentItem.qmlAction)
                    } else {
                        root.close()
                    }
                }

                model: _private.actionsModel
                clip: true
                currentIndex: 0

                boundsBehavior: Flickable.StopAtBounds
                boundsMovement: Flickable.StopAtBounds
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

                    width: _actionsView.width
                    height: _delegateLayout.height

                    enabled: qmlAction.enabled
                    opacity: enabled ? 1 : 0.5

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
                            Layout.alignment: _descriptionLabel.visible ? Qt.AlignTop : Qt.AlignVCenter
                            Layout.topMargin: _descriptionLabel.visible ? 10 : 0
                            Layout.leftMargin: 12
                            Layout.preferredHeight: _nameLabel.height * 0.5
                            Layout.preferredWidth: _nameLabel.height * 0.5

                            fillMode: Image.PreserveAspectFit
                            source: qmlAction.icon.source !== "" ? qmlAction.icon.source : "qrc:/icons/content/blank.png"
                        }

                        ColumnLayout {
                            Layout.alignment: _descriptionLabel.visible ? Qt.AlignTop : Qt.AlignVCenter
                            Layout.fillWidth: true

                            spacing: 0

                            VclText {
                                id: _nameLabel

                                Layout.fillWidth: true

                                bottomPadding: _descriptionLabel.visible ? 2 : 10
                                padding: 10

                                color: Runtime.colors.primary.regular.text
                                elide: Text.ElideRight
                                font: Runtime.idealFontMetrics.font
                                text: "<b>" + actionManager.title + "</b>: " + qmlAction.text + (qmlAction.checkable & qmlAction.checked ? " ✔" : "")
                            }

                            VclText {
                                id: _descriptionLabel

                                Layout.fillWidth: true
                                Layout.leftMargin: 10
                                Layout.rightMargin: 10
                                Layout.bottomMargin: 10

                                color: Runtime.colors.primary.regular.text
                                opacity: 0.6

                                elide: Text.ElideRight
                                font: Runtime.minimumFontMetrics.font
                                maximumLineCount: 3
                                text: qmlAction.tooltip !== undefined ? qmlAction.tooltip : ""
                                visible: text !== ""
                                wrapMode: Text.WordWrap
                            }
                        }

                        ShortcutField {
                            Layout.alignment: Qt.AlignTop
                            Layout.topMargin: 10
                            Layout.preferredWidth: _delegateLayout.width * 0.25

                            fontMetrics: Runtime.minimumShortcutFontMetrics
                            description: actionManager.title + ": " + qmlAction.text
                            enabled: shortcutIsEditable
                            placeholderText: shortcutIsEditable ? "Assign" : ""
                            portableShortcut: qmlAction.shortcut !== undefined ? qmlAction.shortcut : ""

                            onShortcutEdited: (newShortcut) => {
                                ActionHub.assignShortcut(qmlAction, newShortcut)
                            }
                        }
                    }
                }
            }
        }

        onVisibleChanged: {
            if(visible) {
                Qt.callLater(_commandText.forceActiveFocus)
            }
        }
    }

    ActionHandler {
        action: ActionHub.appOptions.find("commandCenter")

        onTriggered: (source) => {
            root.open()
        }
    }

    onAboutToShow: {
        _private.beforeLanguage = Runtime.language.activeCode
        Runtime.language.setActiveCode(QtLocale.English)

        _commandText.text = ""
    }

    onAboutToHide: {
        if(_private.beforeLanguage > 0)
            Runtime.language.setActiveCode(_private.beforeLanguage)

        _private.beforeLanguage = -1
    }

    Loader {
        id: _actionsModelInstantiator

        active: root.visible

        sourceComponent: ActionsModelFilter {
            filters: ActionsModelFilter.CommandCenterFilters
            customFilterMode: true

            onFilterRequest: (qmlAction, actionManager, result) => {
                if(_commandText.length === 0) {
                    result.value = true
                } else {
                    const givenText = _commandText.text.toLowerCase()

                    let text = (actionManager.title + ": " + qmlAction.text)
                    if(qmlAction.keywords !== undefined) {
                        if(typeof qmlAction.keywords === "string")
                        text += ", " + qmlAction.keywords
                        else if(qmlAction.keywords.length > 0)
                        text += ", " + qmlAction.keywords.join(", ")
                    }
                    if(qmlAction.tooltip !== undefined) {
                        text += ", " + qmlAction.tooltip
                    }

                    text = text.toLowerCase()

                    result.value = (text.indexOf(givenText) >= 0)
                }
            }

            onModelReset: Qt.callLater(_private.resetCurrentActionViewItem)
            onRowsRemoved: Qt.callLater(_private.resetCurrentActionViewItem)
            onRowsInserted: Qt.callLater(_private.resetCurrentActionViewItem)
        }
    }

    QtObject {
        id: _private

        property QtObject actionsModel: _actionsModelInstantiator.item
        property int beforeLanguage: -1

        function trigger(qmlAction) {
            if(qmlAction.enabled) {
                if(ActionHub.languageOptions.contains(qmlAction))
                    beforeLanguage = -1
                qmlAction.trigger()

                if(Scrite.user.info.consentToActivityLog) {
                    const actionManager = actionsModel.actionManagerOf(qmlAction)
                    const fields = [
                                     actionManager ? actionManager.title : "<unknown action manager>",
                                     actionManager.containsUserData === true ? "<hidden>" : qmlAction.text
                                 ]
                    const activity = "command-center"
                    const details = fields.join(": ");
                    Scrite.user.logActivity2(activity, details)
                }
            }
            root.close()
        }

        function resetCurrentActionViewItem() {
            _actionsView.currentIndex = _actionsView.count > 0 ? 0 : -1
        }
    }
}
