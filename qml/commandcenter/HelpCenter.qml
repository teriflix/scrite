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

            VclLabel {
                Layout.fillWidth: true

                padding: 10

                background: Rectangle {
                    color: Runtime.colors.accent.c500.background
                }
                color: Runtime.colors.accent.c500.text
                text: "Help Center"
                verticalAlignment: Text.AlignHCenter
                horizontalAlignment: Text.AlignHCenter

                font.bold: true
                font.family: Runtime.idealFontMetrics.font.family
                font.pointSize: Runtime.idealFontMetrics.font.pointSize + 3
            }

            TextField {
                id: _helpText

                Layout.fillWidth: true

                Keys.onUpPressed: _helpTopics.currentIndex = Math.max(_helpTopics.currentIndex-1,0)
                Keys.onDownPressed: _helpTopics.currentIndex = Math.min(_helpTopics.currentIndex+1, _helpTopics.count-1)
                Keys.onEnterPressed: _helpTopics.triggerCurrentItem()
                Keys.onReturnPressed: _helpTopics.triggerCurrentItem()

                DiacriticHandler.enabled: Runtime.allowDiacriticEditing && activeFocus

                focus: true
                placeholderText: "Search for a help topic ..."

                font: Runtime.idealFontMetrics.font

                Connections {
                    target: root

                    function onAboutToShow() {
                        Qt.callLater(_helpText.forceActiveFocus)
                    }
                }
            }

            ListView {
                id: _helpTopics

                Layout.fillWidth: true
                Layout.preferredHeight: Scrite.window.height * 0.4

                function triggerCurrentItem() {
                    if(currentItem != null) {
                        Qt.openUrlExternally(currentItem.location)
                    }

                    root.close()
                }

                model: _private.searchIndex
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
                    required property url location
                    required property string fullTitle
                    required property string plainText

                    width: _helpTopics.width
                    height: _delegateLayout.height

                    MouseArea {
                        anchors.fill: parent

                        onClicked: {
                            Qt.openUrlExternally(location)
                            root.close()
                        }
                    }

                    ColumnLayout {
                        id: _delegateLayout

                        width: parent.width

                        spacing: 0

                        VclText {
                            id: _titleLabel

                            Layout.fillWidth: true

                            padding: 10

                            color: Runtime.colors.primary.regular.text
                            elide: Text.ElideRight
                            text: fullTitle
                            font.bold: _helpText.length > 0
                            font.family: Runtime.idealFontMetrics.font.family
                            font.pixelSize: Runtime.idealFontMetrics.font.pointSize
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
                            text: _private.searchIndex.highlightFilter(plainText, _helpText.text, 240)
                            textFormat: Text.RichText
                            visible: text !== ""
                            wrapMode: Text.WordWrap
                        }
                    }
                }
            }
        }

        onVisibleChanged: {
            if(visible) {
                Qt.callLater(_helpText.forceActiveFocus)
            }
        }
    }

    ActionHandler {
        action: ActionHub.appOptions.find("helpCenter")

        onTriggered: (source) => {
            root.open()
        }
    }

    onAboutToShow: {
        _private.beforeLanguage = Runtime.language.activeCode
        Runtime.language.setActiveCode(QtLocale.English)

        _helpText.text = ""
    }

    onAboutToHide: {
        if(_private.beforeLanguage > 0)
            Runtime.language.setActiveCode(_private.beforeLanguage)

        _private.beforeLanguage = -1
    }

    QtObject {
        id: _private

        property UserGuideSearchIndexFilter searchIndex: UserGuideSearchIndexFilter {
            filter: _helpText.text

            onModelReset: Qt.callLater(_private.resetCurrentActionViewItem)
            onRowsRemoved: Qt.callLater(_private.resetCurrentActionViewItem)
            onRowsInserted: Qt.callLater(_private.resetCurrentActionViewItem)
            onFilterChanged: Qt.callLater(_private.resetCurrentActionViewItem)
        }

        property int beforeLanguage: -1

        function resetCurrentActionViewItem() {
            _helpTopics.currentIndex = _helpTopics.count > 0 ? 0 : -1
        }
    }
}
