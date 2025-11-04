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

import QtQuick 2.15
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"

Item {
    id: root

    property alias source: _private.posterSource // can be a string path, url or QImage
    property string logline

    visible: (_private.posterSourceUrl !== "" || _private.posterQImage !== undefined)

    // Background
    Item {
        anchors.fill: parent

        Image {
            id: posterImageBg

            anchors.fill: parent

            cache: false
            visible: _private.posterSourceKind === _private.e_ImageKind
            fillMode: Image.PreserveAspectCrop
            asynchronous: true

            source: visible ? _private.posterSourceUrl : ""
        }

        QImageItem {
            anchors.fill: parent

            visible: _private.posterSourceKind === _private.e_QImageKind
            fillMode: QImageItem.PreserveAspectCrop
            useSoftwareRenderer: Runtime.currentUseSoftwareRenderer

            image: visible ? _private.posterQImage : Gui.emptyQImage
        }

        Rectangle {
            anchors.fill: parent
            color: "black"
            opacity: 0.75
        }

        BusyIndicator {
            anchors.centerIn: parent

            Material.theme: Material.Dark

            running: posterImageBg.status === Image.Loading || posterImageFg.status === Image.Loading
        }
    }

    // Foreground
    Item {
        anchors.fill: parent

        Image {
            id: posterImageFg

            anchors.fill: parent

            cache: false
            visible: _private.posterSourceKind === _private.e_ImageKind
            fillMode: Image.PreserveAspectFit
            asynchronous: true

            source: visible ? _private.posterSourceUrl : ""
        }

        QImageItem {
            anchors.fill: parent

            visible: _private.posterSourceKind === _private.e_QImageKind
            fillMode: QImageItem.PreserveAspectFit
            useSoftwareRenderer: Runtime.currentUseSoftwareRenderer

            image: visible ? _private.posterQImage : Gui.emptyQImage
        }
    }

    VclLabel {
        width: parent.width * 0.75

        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.leftMargin: 20
        anchors.bottomMargin: 20

        color: "white"
        visible: text !== ""
        background: Rectangle {
            color: "black"
            opacity: 0.8
            radius: 5
        }

        elide: Text.ElideRight
        padding: 10
        wrapMode: Text.WordWrap
        font.pointSize: Runtime.idealFontMetrics.font.pointSize
        maximumLineCount: 4
        verticalAlignment: Text.AlignBottom
        horizontalAlignment: Text.AlignLeft

        text: root.logline
    }

    // Private stuff
    QtObject {
        id: _private

        property string posterSourceUrl
        property var posterQImage: Gui.blankQImage
        property var posterSource

        readonly property int e_ImageKind: 0
        readonly property int e_QImageKind: 1
        property int posterSourceKind: e_ImageKind

        onPosterSourceChanged: {
            posterSourceUrl = ""
            posterQImage = Gui.blankQImage

            if(typeof posterSource === "string" || typeof posterSource === "url") {
                posterSourceUrl = posterSource
                posterSourceKind = e_ImageKind
            } else if(typeof posterSource === "object" && Object.isOfType(posterSource, "QImage")) {
                posterQImage = posterSource
                posterSourceKind = e_QImageKind
            }
        }
    }
}
