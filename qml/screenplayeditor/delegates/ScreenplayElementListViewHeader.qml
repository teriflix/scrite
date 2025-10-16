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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Layouts 1.15

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"
import "qrc:/qml/screenplayeditor"

Loader {
    id: root

    required property var pageMargins
    required property bool readOnly
    required property real zoomLevel
    required property ScreenplayAdapter screenplayAdapter

    sourceComponent: _private.showTitleCard ? _private.titleCardComponent : _private.editTitlePageButtonComponent

    QtObject {
        id: _private

        property Screenplay screenplay: screenplayAdapter.screenplay

        property bool showTitleCard: screenplayAdapter.isSourceScreenplay &&
                                          (screenplay.hasTitlePageAttributes ||
                                           Runtime.screenplayEditorSettings.showLoglineEditor ||
                                           screenplay.coverPagePhoto !== "")
        property real padding: (root.pageMargins.left+root.pageMargins.right)/2

        property FontMetrics fontMetrics: Runtime.sceneEditorFontMetrics

        function launchTitlePageEditorDialog() {
            if(root.readOnly)
                MessageBox.information("ReadOnly Mode",
                                       "Title card fields cannot be edited since the document is opened in readonly mode.")
            else
                TitlePageDialog.launch()
        }

        // This component is instantiated when no title card information exists as yet.
        property Component editTitlePageButtonComponent: Item {
            height: _editTitlePageButton.height + 40

            VclButton {
                id: _editTitlePageButton

                anchors.centerIn: parent

                text: "Edit Title Page"
                enabled: !root.readOnly

                onClicked: _private.launchTitlePageEditorDialog()
            }
        }

        // This component is instantiated only when there are atleast some title card fields available,
        // or the title page photo is available.
        property Component titleCardComponent: Item {
            id: _titleCard

            height: _titleCardLayout.height


            ColumnLayout {
                id: _titleCardLayout

                spacing: 10 * zoomLevel

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: _private.padding
                anchors.rightMargin: _private.padding

                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 35 * zoomLevel
                }

                Item {
                    Layout.alignment: Qt.AlignHCenter

                    implicitWidth: {
                        switch(_private.screenplay.coverPagePhotoSize) {
                        case Screenplay.SmallCoverPhoto:
                            return _titleCardLayout.width / 4
                        case Screenplay.MediumCoverPhoto:
                            return _titleCardLayout.width / 2
                        }
                        return _titleCardLayout.width
                    }
                    implicitHeight: _coverPicImage.sourceSize.height * (implicitWidth/_coverPicImage.sourceSize.width)

                    Image {
                        id: _coverPicImage

                        anchors.fill: parent

                        cache: false
                        source: visible ? "file:///" + _private.screenplay.coverPagePhoto : ""
                        smooth: true; mipmap: true; asynchronous: true
                        visible: _private.screenplay.coverPagePhoto !== ""
                        fillMode: Image.PreserveAspectFit

                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: -border.width - 4

                            color: Qt.rgba(1,1,1,0.1)
                            border { width: 2; color: _titleLink.hoverColor }
                            visible: _coverPicMouseArea.containsMouse
                        }

                        MouseArea {
                            id: _coverPicMouseArea

                            anchors.fill: parent

                            enabled: !root.readOnly
                            hoverEnabled: true

                            cursorShape: Qt.PointingHandCursor
                            onClicked: _private.launchTitlePageEditorDialog()
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: _private.screenplay.coverPagePhoto !== "" ? 20 * zoomLevel : 0
                }

                Link {
                    id: _titleLink

                    Layout.fillWidth: true

                    text: _private.screenplay.title === "" ? "<untitled>" : _private.screenplay.title
                    enabled: !root.readOnly
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    horizontalAlignment: Text.AlignHCenter

                    font.bold: true
                    font.family: _private.fontMetrics.font.family
                    font.pointSize: _private.fontMetrics.font.pointSize + 2
                    font.underline: containsMouse

                    onClicked: _private.launchTitlePageEditorDialog()
                }

                Link {
                    Layout.fillWidth: true

                    text: _private.screenplay.subtitle
                    enabled: !root.readOnly
                    visible: text !== ""
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    horizontalAlignment: Text.AlignHCenter

                    font.family: _private.fontMetrics.font.family
                    font.pointSize: _private.fontMetrics.font.pointSize
                    font.underline: containsMouse

                    onClicked: _private.launchTitlePageEditorDialog()
                }

                ColumnLayout {
                    Layout.fillWidth: true

                    spacing: 0

                    VclLabel {
                        Layout.fillWidth: true

                        text: "Written By"
                        font: _private.fontMetrics.font
                        width: parent.width
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Link {
                        Layout.fillWidth: true

                        text: (_private.screenplay.author === "" ? "<unknown author>" : _private.screenplay.author)
                        enabled: !root.readOnly
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        horizontalAlignment: Text.AlignHCenter

                        font.family: _private.fontMetrics.font.family
                        font.pointSize: _private.fontMetrics.font.pointSize
                        font.underline: containsMouse

                        onClicked: _private.launchTitlePageEditorDialog()
                    }
                }

                Link {
                    Layout.fillWidth: true

                    text: _private.screenplay.version
                    enabled: !root.readOnly
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    horizontalAlignment: Text.AlignHCenter

                    font.family: _private.fontMetrics.font.family
                    font.pointSize: _private.fontMetrics.font.pointSize
                    font.underline: containsMouse

                    onClicked: _private.launchTitlePageEditorDialog()
                }

                Link {
                    Layout.fillWidth: true

                    text: _private.screenplay.basedOn
                    enabled: !root.readOnly
                    visible: text !== ""
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    horizontalAlignment: Text.AlignHCenter

                    font.family: _private.fontMetrics.font.family
                    font.pointSize: _private.fontMetrics.font.pointSize
                    font.underline: containsMouse

                    onClicked: _private.launchTitlePageEditorDialog()
                }

                ColumnLayout {
                    Layout.alignment: Qt.AlignLeft
                    Layout.preferredWidth: parent.width * 0.5

                    spacing: parent.spacing/2

                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 20 * zoomLevel
                    }

                    Link {
                        Layout.fillWidth: true

                        text: _private.screenplay.contact
                        enabled: !root.readOnly
                        visible: text !== ""
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere

                        font.family: _private.fontMetrics.font.family
                        font.pointSize: _private.fontMetrics.font.pointSize
                        font.underline: containsMouse

                        onClicked: _private.launchTitlePageEditorDialog()
                    }

                    Link {
                        Layout.fillWidth: true

                        text: _private.screenplay.address
                        enabled: !root.readOnly
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        visible: text !== ""

                        font.family: _private.fontMetrics.font.family
                        font.pointSize: _private.fontMetrics.font.pointSize
                        font.underline: containsMouse

                        onClicked: _private.launchTitlePageEditorDialog()
                    }

                    Link {
                        Layout.fillWidth: true

                        text: _private.screenplay.phoneNumber
                        enabled: !root.readOnly
                        visible: text !== ""
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere

                        font.family: _private.fontMetrics.font.family
                        font.pointSize: _private.fontMetrics.font.pointSize
                        font.underline: containsMouse

                        onClicked: _private.launchTitlePageEditorDialog()
                    }

                    Link {
                        Layout.fillWidth: true

                        text: _private.screenplay.email
                        color: "blue"
                        enabled: !root.readOnly
                        visible: text !== ""
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere

                        font.family: _private.fontMetrics.font.family
                        font.pointSize: _private.fontMetrics.font.pointSize
                        font.underline: containsMouse

                        onClicked: Qt.openUrlExternally("mailto:" + text)
                    }

                    Link {
                        Layout.fillWidth: true

                        text: _private.screenplay.website
                        color: "blue"
                        elide: Text.ElideRight
                        enabled: !root.readOnly
                        visible: text !== ""

                        font.family: _private.fontMetrics.font.family
                        font.pointSize: _private.fontMetrics.font.pointSize
                        font.underline: containsMouse

                        onClicked: Qt.openUrlExternally(text)
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: (_loglineFieldLayout.visible ? 20 : 35) * zoomLevel
                }

                ColumnLayout {
                    id: _loglineFieldLayout

                    Layout.fillWidth: true

                    spacing: 13
                    visible: Runtime.screenplayEditorSettings.showLoglineEditor

                    VclLabel {
                        Layout.fillWidth: true

                        text: _logLineField.activeFocus ? ("Logline: (" + (_loglineLimiter.limitReached ? "WARNING: " : "") + _loglineLimiter.wordCount + "/" + _loglineLimiter.maxWordCount + " words, " +
                                                          _loglineLimiter.letterCount + "/" + _loglineLimiter.maxLetterCount + " letters)") : "Logline: "
                        color: _loglineLimiter.limitReached ? "darkred" : Runtime.colors.primary.a700.background
                        visible: _logLineField.length > 0

                        font.family: _private.fontMetrics.font.family
                        font.pointSize: _private.fontMetrics.font.pointSize-2
                    }

                    TextAreaInput {
                        id: _logLineField

                        Layout.fillWidth: true

                        Component.onCompleted: SyntaxHighlighter.addDelegate(_loglineLimitHighlighter)

                        text: _private.screenplay.logline
                        font: _private.fontMetrics.font
                        readOnly: root.readOnly
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        placeholderText: "Type your logline here, max " + _loglineLimiter.maxWordCount + " words or " + _loglineLimiter.maxLetterCount + " letters."

                        onTextChanged: _private.screenplay.logline = text
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 35 * zoomLevel
                    }
                }
            }

            TextLimiterSyntaxHighlighterDelegate {
                id: _loglineLimitHighlighter

                textLimiter: TextLimiter {
                    id: _loglineLimiter
                    maxWordCount: 50
                    maxLetterCount: 240
                    countMode: TextLimiter.CountInText
                }
            }
        }

        onShowTitleCardChanged: {
            Scrite.app.resetObjectProperty(root, "height")
        }
    }
}
