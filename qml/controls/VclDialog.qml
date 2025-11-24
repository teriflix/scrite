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

  5. Handles language change shortcuts, provided handleLanguageShortcuts is set to true.
  */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

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
        enabled: ActionHandler.enabled
    }

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
    property alias bottomBarInstance: _footerLoader.item

    // This signal is emitted after the dialog box has been dismissed.
    signal dismissed()

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
        EventFilter.target: Scrite.app
        EventFilter.active: root.acceptAction.enabled && _contentItemLoader.item && root.visible
        EventFilter.events: [EventFilter.KeyPress]
        EventFilter.onFilter: (object, event, result) => {
                                  if(event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                                      result.filter = true
                                      result.accepted = true
                                      root.acceptAction.trigger()
                                  }
                              }

        width: root.width
        height: root.height - root.header.height

        clip: _contentItemScroll.contentWidth > _contentItemScroll.width ||
              _contentItemScroll.contentHeight > _contentItemScroll.height

        Flickable {
            id: _contentItemScroll

            ScrollBar.vertical: VclScrollBar { }
            ScrollBar.horizontal: VclScrollBar { }

            anchors.fill: parent

            contentWidth: _contentItemLoader.width
            contentHeight: _contentItemLoader.height

            Loader {
                id: _contentItemLoader

                property real itemImplicitWidth: item ? item.implicitWidth : 0
                property real itemImplicitHeight: item ? item.implicitHeight : 0

                width: (itemImplicitWidth === 0) ? _contentItemScroll.width : Math.max(_contentItemScroll.width,itemImplicitWidth)
                height: (itemImplicitHeight === 0) ? _contentItemScroll.height : Math.max(_contentItemScroll.height,itemImplicitHeight)

                active: false
                sourceComponent: content

                onLoaded: item.focus = true

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
        width: root.width
        height: _dialogHeaderLayout.height

        color: Runtime.colors.accent.c600.background

        RowLayout {
            id: _dialogHeaderLayout

            width: parent.width

            spacing: 2

            VclLabel {
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true

                color: Runtime.colors.accent.c600.text
                padding: 16
                text: root.title
                elide: Text.ElideRight

                font.bold: true
                font.pointSize: Runtime.idealFontMetrics.font.pointSize
            }

            Loader {
                active: root.visible
                sourceComponent: titleBarButtons

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

            sourceComponent: bottomBar
        }
    }

    // Private section
    Announcement.onIncoming: (type,data) => {
                                 if(root.closeOnDragDrop && type === Runtime.announcementIds.closeDialogBoxRequest)
                                    root.close()
                             }

    Component.onCompleted: {
        if(root.backdrop) {
            background = _customBackground.createObject(root)
        }
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

            Item {
                required property var language // This is of type Language, but we have to use var here.
                // You cannot use Q_GADGET struct names as type names in QML
                // that privilege is only reserved for QObject types.

                Shortcut {
                    sequence: language.shortcut()

                    onActivated: Runtime.language.setActiveCode(language.code)
                }
            }
        }
    }

    Component {
        id: _customBackground

        Loader {
            width: root.width
            height: root.height

            sourceComponent: root.backdrop
            active: root.visible

            BoxShadow {
                anchors.fill: parent
            }
        }
    }

    onAboutToShow: {
        Scrite.window.closeButtonVisible = appCloseButtonVisible
        if(appOverrideCursor >= 0) {
            MouseCursor.setShape(appOverrideCursor);
            _private.overrideCursorMustBeRestored = true
        }
        focus = true
        Runtime.dialogs.include(root)
    }

    onAboutToHide: {
        Scrite.window.closeButtonVisible = true
        if(_private.overrideCursorMustBeRestored) {
            MouseCursor.unsetShape()
            _private.overrideCursorMustBeRestored = false
        }
        Runtime.dialogs.exclude(root)
    }

    onAppCloseButtonVisibleChanged: {
        Scrite.window.closeButtonVisible = visible && appCloseButtonVisible
    }

    onClosed: {
        Runtime.execLater(root, 50, dismissed)
    }
}
