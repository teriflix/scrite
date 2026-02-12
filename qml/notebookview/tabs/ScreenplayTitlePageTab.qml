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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"
import "qrc:/qml/notebookview"
import "qrc:/qml/structureview"

Item {
    id: root

    required property real maxTextAreaSize
    required property real minTextAreaSize

    readonly property Screenplay screenplay: Scrite.document.screenplay

    Flickable {
        id: _flickable

        FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

        ScrollBar.vertical: _vscrollBar
        ScrollBar.horizontal: _hscrollBar

        anchors.centerIn: parent

        width: Math.max(root.minTextAreaSize, Math.min(parent.width-20, root.maxTextAreaSize))
        height: Math.min(contentHeight, parent.height)

        contentWidth: width
        contentHeight: _layout.height

        Column {
            id: _layout

            width: _flickable.width

            spacing: 10
            enabled: _private.titlePageAction.enabled

            Image {
                id: _coverPic

                anchors.horizontalCenter: parent.horizontalCenter

                width: {
                    switch(root.screenplay.coverPagePhotoSize) {
                    case Screenplay.SmallCoverPhoto:
                        return parent.width / 4
                    case Screenplay.MediumCoverPhoto:
                        return parent.width / 2
                    }
                    return parent.width
                }

                fillMode: Image.PreserveAspectFit
                smooth: true; mipmap: true
                source: visible ? "file:///" + root.screenplay.coverPagePhoto : ""
                visible: root.screenplay.coverPagePhoto !== ""

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: -border.width - 4

                    border { width: 2; color: _titleLink.hoverColor }
                    color: Qt.rgba(1,1,1,0.1)
                    visible: _coverPicMouseArea.containsMouse
                }

                MouseArea {
                    id: _coverPicMouseArea

                    anchors.fill: parent

                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor

                    onClicked: _private.titlePageAction.trigger(root)
                }
            }

            Link {
                id: _titleLink

                width: parent.width

                horizontalAlignment: Text.AlignHCenter
                text: root.screenplay.title === "" ? "<untitled>" : root.screenplay.title
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere

                font.bold: true
                font.family: _private.fontMetrics.font.family
                font.pointSize: _private.fontMetrics.font.pointSize + 2
                font.underline: containsMouse

                onClicked: _private.titlePageAction.trigger(root)
            }

            Link {
                width: parent.width

                horizontalAlignment: Text.AlignHCenter
                text: root.screenplay.subtitle
                visible: root.screenplay.subtitle !== ""
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere

                font.bold: true
                font.family: _private.fontMetrics.font.family
                font.pointSize: _private.fontMetrics.font.pointSize
                font.underline: containsMouse

                onClicked: _private.titlePageAction.trigger(root)
            }

            Column {
                width: parent.width
                spacing: 0

                VclLabel {
                    width: parent.width

                    font: _private.fontMetrics.font
                    horizontalAlignment: Text.AlignHCenter
                    text: "Written By"
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                }

                Link {
                    width: parent.width

                    horizontalAlignment: Text.AlignHCenter
                    text: (root.screenplay.author === "" ? "<unknown author>" : root.screenplay.author)
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere

                    font.family: _private.fontMetrics.font.family
                    font.pointSize: _private.fontMetrics.font.pointSize
                    font.underline: containsMouse

                    onClicked: _private.titlePageAction.trigger(root)
                }
            }

            Link {
                horizontalAlignment: Text.AlignHCenter
                text: root.screenplay.version === "" ? "Initial Version" : root.screenplay.version
                width: parent.width
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere

                font.family: _private.fontMetrics.font.family
                font.pointSize: _private.fontMetrics.font.pointSize
                font.underline: containsMouse

                onClicked: _private.titlePageAction.trigger(root)
            }

            Link {
                width: parent.width

                horizontalAlignment: Text.AlignHCenter
                text: root.screenplay.basedOn
                visible: root.screenplay.basedOn !== ""
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere

                font.family: _private.fontMetrics.font.family
                font.pointSize: _private.fontMetrics.font.pointSize
                font.underline: containsMouse

                onClicked: _private.titlePageAction.trigger(root)
            }

            Item { width: parent.width; height: parent.spacing/2 }

            Column {
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: -width*0.25

                width: parent.width * 0.5

                spacing: parent.spacing/2

                Link {
                    width: parent.width

                    text: root.screenplay.contact
                    visible: text !== ""
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere

                    font.family: _private.fontMetrics.font.family
                    font.pointSize: _private.fontMetrics.font.pointSize-2
                    font.underline: containsMouse

                    onClicked: _private.titlePageAction.trigger(root)
                }

                Link {
                    width: parent.width

                    text: root.screenplay.address
                    visible: text !== ""
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere

                    font.family: _private.fontMetrics.font.family
                    font.pointSize: _private.fontMetrics.font.pointSize-2
                    font.underline: containsMouse

                    onClicked: _private.titlePageAction.trigger(root)
                }

                Link {
                    width: parent.width

                    text: root.screenplay.phoneNumber
                    visible: text !== ""
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere

                    font.family: _private.fontMetrics.font.family
                    font.pointSize: _private.fontMetrics.font.pointSize-2
                    font.underline: containsMouse

                    onClicked: _private.titlePageAction.trigger(root)
                }

                Link {
                    width: parent.width

                    text: root.screenplay.email
                    visible: text !== ""
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere

                    font.family: _private.fontMetrics.font.family
                    font.pointSize: _private.fontMetrics.font.pointSize-2
                    font.underline: containsMouse

                    onClicked: _private.titlePageAction.trigger(root)
                }

                Link {
                    width: parent.width

                    text: root.screenplay.website
                    visible: text !== ""
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere

                    font.family: _private.fontMetrics.font.family
                    font.pointSize: _private.fontMetrics.font.pointSize-2
                    font.underline: containsMouse

                    onClicked: _private.titlePageAction.trigger(root)
                }
            }
        }
    }

    VclScrollBar {
        id: _vscrollBar

        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        orientation: Qt.Vertical
        flickable: _flickable
    }

    VclScrollBar {
        id: _hscrollBar

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        orientation: Qt.Horizontal
        flickable: _flickable
    }

    QtObject {
        id: _private

        readonly property Action titlePageAction: ActionHub.screenplayOperations.find("titlePage")

        readonly property FontMetrics fontMetrics: FontMetrics {
            font.family: Runtime.sceneEditorFontMetrics.font.family
            font.pointSize: Runtime.idealFontMetrics.font.pointSize
        }
    }
}
