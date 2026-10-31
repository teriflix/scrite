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

  5. Handles language change shortcuts, provided handleLanguageShortcuts is set to true.
  */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import Scrite.App

import "../globals"
import "../helpers"

Dialog {
    id: root

    // True if the dialog box is in the main window, which is so unless it is hosted in another
    // window, such as the one of an extension.
    readonly property bool inMainWindow: {
        const window = parent ? parent.Window.window : null
        return window === null || window === Scrite.window
    }

    // True if the dialog box is one of the app, which means that it is listed in Runtime.dialogs,
    // which keeps the rest of the main window from being used while it is open, and that it hides the
    // close button of the main window, if appCloseButtonVisible is false. A dialog box in the main
    // window always is. One in another window is not, because it blocks only that window, unless
    // blocksMainWindow is true.
    readonly property bool isAppDialog: inMainWindow || blocksMainWindow

    // Only for a dialog box that is in another window than the main one, which it makes one of the
    // app anyway, because the main window must not be used or closed while it is open, for example
    // when it runs tasks. It has no effect on a dialog box in the main window.
    property bool blocksMainWindow: false

    // If this property is set to false, then the main-window's close button on
    // the title bar is disabled whenever the dialog box is active.
    property bool appCloseButtonVisible: true

    // If set, then it will be used as application's override cursor whenever
    // the dialog box is active. The cursor will be restored as soon as the dialog
    // box is closed.
    property int appOverrideCursor: -1

    // If set, then the dialog box automatically closes whenever user drags and
    // drops a file on the Scrite window to import it.
    property bool closeOnDragDrop: true

    // If set, then this dialog box handles language change shortcuts.
    property bool handleLanguageShortcuts: false

    // Assign a component whose instance may be shown as background
    property Component backdrop: null

    // Assign a component whose instance may be shown in the content
    property Component content: Item { }
    property alias contentImplicitWidth: _contentItemLoader.implicitWidth
    property alias contentImplicitHeight: _contentItemLoader.implicitHeight
    property alias contentInstance: _contentItemLoader.item

    readonly property Action acceptAction: Action {
        enabled: ActionHandler.canHandle
        shortcut: Gui.shortcut(Qt.Key_Return)
    }

    // Customise the buttons to show on the tilebar on the right side.
    // By default a check-mark is shown.
    property bool titleBarCloseButtonVisible: true
    property Component titleBarButtons: Image {
        width: 32; height: 32
        source: Runtime.themedIcon("qrc:/icons/action/dialog_close_button.png")
        smooth: true; mipmap: true
        enabled: visible
        visible: root.titleBarCloseButtonVisible

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: parent.scale = 1.2
            onExited: parent.scale = 1
            onClicked: root.close()
        }
    }

    property Component bottomBar
    property alias bottomBarInstance: _footerLoader.item

    // This signal is emitted after the dialog box has been dismissed.
    signal dismissed()

    Material.elevation: 6
    Material.containerStyle: Material.Filled
    Material.roundedScale: Material.NotRounded

    Overlay.modal: Rectangle { color: Qt.rgba(0,0,0,0.4) }

    // Configure built-in properties of the Dialog
    parent: Overlay.overlay
    anchors.centerIn: parent

    modal: true
    closePolicy: titleBarCloseButtonVisible ? Popup.CloseOnEscape : Popup.NoAutoClose

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

    contentItem: Item {
        width: root.width
        height: root.height - root.header.height

        clip: _contentItemScroll.contentWidth > _contentItemScroll.width ||
              _contentItemScroll.contentHeight > _contentItemScroll.height

        Flickable {
            id: _contentItemScroll

            ScrollBar.vertical: SctScrollBar { }
            ScrollBar.horizontal: SctScrollBar { }

            anchors.fill: parent

            clip: true
            contentWidth: _contentItemLoader.width
            contentHeight: _contentItemLoader.height

            Loader {
                id: _contentItemLoader

                property Item visualItem: item as Item

                property real itemImplicitWidth: visualItem ? visualItem.implicitWidth : 0
                property real itemImplicitHeight: visualItem ? visualItem.implicitHeight : 0

                width: (itemImplicitWidth === 0) ? _contentItemScroll.width : Math.max(_contentItemScroll.width,itemImplicitWidth)
                height: (itemImplicitHeight === 0) ? _contentItemScroll.height : Math.max(_contentItemScroll.height,itemImplicitHeight)

                active: false
                sourceComponent: root.content

                onLoaded: visualItem.focus = true

                Connections {
                    target: root

                    function onAboutToShow() {
                        _contentItemLoader.active = true
                    }

                    function onClosed() {
                        _contentItemLoader.active = false
                    }
                }
            }
        }
    }

    header: Rectangle {
        id: _header

        width: root.width
        height: _dialogHeaderLayout.height

        color: Runtime.colors.accent.c600.background

        RowLayout {
            id: _dialogHeaderLayout

            width: parent.width

            spacing: 2

            SctLabel {
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true

                color: Color.textColorFor(_header.color)
                padding: 16
                text: root.title
                elide: Text.ElideRight

                font.bold: true
                font.pointSize: Runtime.idealFontMetrics.font.pointSize
            }

            Loader {
                active: root.visible
                sourceComponent: root.titleBarButtons

                Layout.alignment: Qt.AlignVCenter
                Layout.rightMargin: 8
            }
        }
    }

    footer: Rectangle {
        visible: height > 0
        color: Runtime.colors.primary.c200.background

        height: _footerLoader.item ? _footerLoader.height : 0

        Loader {
            id: _footerLoader

            width: parent.width

            sourceComponent: root.bottomBar
        }
    }

    background: Rectangle { color: Runtime.colors.primary.c100.background }

    // Private section
    Announcement.onIncoming: (type,data) => {
                                 if(root.closeOnDragDrop && type === Runtime.announcementIds.closeDialogBoxRequest)
                                    root.close()
                             }

    QtObject {
        id: _private

        property bool overrideCursorMustBeRestored: false
    }

    Item {
        visible: false

        /**
          Language shortcuts from ActionHub.languageOptions will stop working
          whenever a dialog-box is shown. This is how Qt's Action {} items are
          designed to work. They only respond to shortcut events if they are
          created in the same overlay as the current one.

          The following repeater creates temporary shortcuts for use within the
          dialog box, only for language switching. All other actions remain
          unavailable.
          */
        Repeater {
            model: root.visible && root.modal && root.handleLanguageShortcuts ? LanguageEngine.supportedLanguages : 0

            delegate: Item {
                id: _languageDelegate

                required property int index
                required property scriteLanguage language

                Shortcut {
                    sequence: _languageDelegate.language.shortcut()

                    onActivated: Runtime.language.setActiveCode(_languageDelegate.language.code)
                }
            }
        }
    }

    onAboutToShow: {
        if(isAppDialog)
            Scrite.window.closeButtonVisible = appCloseButtonVisible
        if(appOverrideCursor >= 0) {
            MouseCursor.setShape(appOverrideCursor);
            _private.overrideCursorMustBeRestored = true
        }
        focus = true
        if(isAppDialog)
            Runtime.dialogs.include(root)
    }

    onAboutToHide: {
        if(isAppDialog) {
            if(Scrite && Scrite.window)
                Scrite.window.closeButtonVisible = true
        }
        if(_private.overrideCursorMustBeRestored) {
            MouseCursor.unsetShape()
            _private.overrideCursorMustBeRestored = false
        }
        if(isAppDialog)
            Runtime.dialogs.exclude(root)
    }

    onAppCloseButtonVisibleChanged: {
        if(isAppDialog)
            Scrite.window.closeButtonVisible = visible && appCloseButtonVisible
    }

    // A dialog box that stops, or starts, being one of the app while it is open, for example because
    // it is moved from one window to another, is listed, or not, as it is now.
    onIsAppDialogChanged: {
        if(!visible)
            return

        if(isAppDialog) {
            Scrite.window.closeButtonVisible = appCloseButtonVisible
            Runtime.dialogs.include(root)
        } else {
            Scrite.window.closeButtonVisible = true
            Runtime.dialogs.exclude(root)
        }
    }

    onClosed: {
        Runtime.execLater(root, 50, dismissed)
    }
}
