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

/**
  This QML component extends the Dialog item from QtQuick controls to standardise
  appearence and functionality of dialog boxes in the UI across the whole application.

  Key features offered by this component

  1. Backdrop and content items are accepted as components, they are instantiated
     only when the dialog box is actually visible to the user.

  2. Dialogs are modal by default, parented to Overlay.overlay and centered in it.

  3. Title bar is standardised with a close/apply button on the top right.

  4. Content is shown in a ScrollView, whose scrollbars show up whenever size available
     is less than the implicit-size declared by the content component.
  */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/helpers"

Dialog {
    id: root

    Material.primary: Runtime.colors.primary.key
    Material.accent: Runtime.colors.accent.key
    Material.theme: Runtime.colors.theme

    // If this property is set to false, then the main-window's close button on
    // the title bar is disabled whenever the dialog box is active.
    property bool appCloseButtonVisible: true

    // If set, then it will be used as application's override cursor whenever
    // the dialog box is active. The cursor will be restored as soon as the dialog
    // box is closed.
    property int appOverrideCursor: -1

    // Assign a component whose instance may be shown as background
    property Component backdrop: Rectangle {
        color: Runtime.colors.primary.c100.background
        BoxShadow {
            anchors.fill: parent
        }
    }

    // Assign a component whose instance may be shown in the content
    property Component content: Item { }
    property alias contentImplicitWidth: contentItemLoader.implicitWidth
    property alias contentImplicitHeight: contentItemLoader.implicitHeight
    property alias contentInstance: contentItemLoader.item

    // Customise the buttons to show on the tilebar on the right side.
    // By default a check-mark is shown.
    property bool titleBarCloseButtonVisible: true
    property Component titleBarButtons: Image {
        width: 32; height: 32
        source: "qrc:/icons/action/dialog_close_button.png"
        smooth: true; mipmap: true
        enabled: visible
        visible: titleBarCloseButtonVisible

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: parent.scale = 1.2
            onExited: parent.scale = 1
            onClicked: root.close()
        }
    }

    property Component bottomBar

    // This signal is emitted after the dialog box has been dismissed.
    signal dismissed()

    // Configure built-in properties of the Dialog
    parent: Overlay.overlay
    anchors.centerIn: parent

    modal: true
    closePolicy: Popup.CloseOnEscape

    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    bottomPadding: 0

    topMargin: 0
    leftMargin: 0
    rightMargin: 0
    bottomMargin: 0

    topInset: 0
    leftInset: 0
    rightInset: 0
    bottomInset: 0

    background: Loader {
        width: root.width
        height: root.height
        sourceComponent: backdrop
        active: root.visible
    }

    contentItem: Item {
        width: root.width
        height: root.height - root.header.height
        clip: contentItemScroll.contentWidth > contentItemScroll.width ||
              contentItemScroll.contentHeight > contentItemScroll.height

        Flickable {
            id: contentItemScroll
            anchors.fill: parent
            contentWidth: contentItemLoader.width
            contentHeight: contentItemLoader.height
            ScrollBar.vertical: VclScrollBar { }
            ScrollBar.horizontal: VclScrollBar { }

            Loader {
                id: contentItemLoader
                sourceComponent: content
                active: false
                property real itemImplicitWidth: item ? item.implicitWidth : 0
                property real itemImplicitHeight: item ? item.implicitHeight : 0
                width: (itemImplicitWidth === 0) ? contentItemScroll.width : Math.max(contentItemScroll.width,itemImplicitWidth)
                height: (itemImplicitHeight === 0) ? contentItemScroll.height : Math.max(contentItemScroll.height,itemImplicitHeight)

                Connections {
                    target: root
                    function onAboutToShow() {
                        contentItemLoader.active = true
                    }
                    function onClosed() {
                        contentItemLoader.active = false
                    }
                }
            }
        }
    }

    header: Rectangle {
        color: Runtime.colors.accent.c600.background
        width: root.width
        height: dialogHeaderLayout.height

        RowLayout {
            id: dialogHeaderLayout
            spacing: 2
            width: parent.width

            VclText {
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true

                font.bold: true
                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                color: Runtime.colors.accent.c600.text
                padding: 16
                text: root.title
            }

            Loader {
                Layout.alignment: Qt.AlignVCenter
                Layout.rightMargin: 8
                sourceComponent: titleBarButtons
                active: root.visible
            }
        }
    }

    footer: Rectangle {
        color: Runtime.colors.primary.c200.background
        height: footerLoader.item ? footerLoader.height : 0
        visible: height > 0

        Loader {
            id: footerLoader
            width: parent.width
            sourceComponent: bottomBar
        }
    }

    // Private section
    QtObject {
        id: _private

        property bool overrideCursorMustBeRestored: false
    }

    onAboutToShow: {
        Scrite.window.closeButtonVisible = appCloseButtonVisible
        if(appOverrideCursor >= 0) {
            Scrite.app.installOverrideCursor(appOverrideCursor);
            _private.overrideCursorMustBeRestored = true
        }
    }
    onAboutToHide: {
        Scrite.window.closeButtonVisible = true
        if(_private.overrideCursorMustBeRestored) {
            Scrite.app.rollbackOverrideCursor()
            _private.overrideCursorMustBeRestored = false
        }
    }
    onAppCloseButtonVisibleChanged: Scrite.window.closeButtonVisible = visible && appCloseButtonVisible
    onClosed: Utils.execLater(root, 50, dismissed)
}
