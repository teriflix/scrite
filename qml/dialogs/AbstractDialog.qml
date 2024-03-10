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

  Since all dialog box QML files are stored in this folder, it makes sense to keep
  this file in the same folder.

  Key features offered by this component

  1. Backdrop and content items are accepted as components, they are instantiated
     only when the dialog box is actually visible to the user.

  2. Dialogs are modal by default, parented to Overlay.overlay and centered in it.

  3. Title bar is standardised with a close/apply button on the top right.
  */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "../../js/utils.js" as Utils
import "../globals"

Dialog {
    id: dialog

    // Assign a component whose instance may be shown as background
    property Component backdrop: Rectangle {
        color: Runtime.colors.primary.windowColor
    }

    // Assign a component whose instance may be shown in the content
    property Component content: Item { }

    // Customise the buttons to show on the tilebar on the right side.
    // By default a check-mark is shown.
    property Component titleBarButtons: Image {
        width: 32; height: 32
        source: "qrc:/icons/action/dialog_close_button.png"
        smooth: true; mipmap: true

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: parent.scale = 1.2
            onExited: parent.scale = 1
            onClicked: dialog.close()
        }
    }

    // Configure built-in properties of the Dialog
    parent: Overlay.overlay
    anchors.centerIn: parent

    modal: true
    closePolicy: Popup.NoAutoClose
    margins: 0
    padding: 0

    background: Loader {
        width: dialog.width
        height: dialog.height
        sourceComponent: backdrop
        active: dialog.visible
    }

    contentItem: Loader {
        width: dialog.width
        height: dialog.hight - dialog.header.height
        sourceComponent: content
        active: dialog.visible
    }

    header: Rectangle {
        color: Runtime.colors.accent.c600.background
        width: dialog.width
        height: dialogHeaderLayout.height

        RowLayout {
            id: dialogHeaderLayout
            spacing: 2
            width: parent.width

            Label {
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true

                font.bold: true
                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                color: Runtime.colors.accent.c600.text
                padding: 16
                text: dialog.title
            }

            Loader {
                Layout.alignment: Qt.AlignVCenter
                Layout.rightMargin: 8
                sourceComponent: titleBarButtons
                active: dialog.visible
            }
        }
    }
}
