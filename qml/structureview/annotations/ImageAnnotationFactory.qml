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

import QtQml 2.15
import QtQuick 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

QtObject {
    id: root

    readonly property string type: "image"

    function create(parent, x, y, filePath) {
        if(Scrite.document.readOnly)
            return

        let w = 300
        let h = 160
        let geometry = Qt.rect(x-w/2, y-h/2, w, h)

        let annot = Utils.newAnnotation(parent, type, geometry, null)
        if(filePath && typeof filePath === "string")
            annot.setAttribute("image", annot.addImage(filePath))

        Scrite.document.structure.addAnnotation(annot)

        return annot
    }

    readonly property Component delegate: AbstractAnnotationDelegate {
        id: _d

        BoundingBoxItem.livePreview: false
        BoundingBoxItem.previewImageSource: _image.source

        clip: true
        color: _image.isSet ? (_d.annotation.attributes.fillBackground ? _d.annotation.attributes.backgroundColor : Qt.rgba(0,0,0,0)) : Runtime.colors.primary.c100.background

        Image {
            id: _image

            property bool isSet: _d.annotation.attributes.image !== "" && status === Image.Ready

            width: parent.width - 10
            height: sourceSize.height / sourceSize.width * width
            anchors.top: parent.top
            anchors.topMargin: 5
            anchors.horizontalCenter: parent.horizontalCenter

            smooth: _canvasScroll.moving || _canvasScroll.flicking ? false : true
            mipmap: smooth
            source: _d.annotation.imageUrl(_d.annotation.attributes.image)
            fillMode: Image.Stretch
            asynchronous: true

            onStatusChanged: {
                if(status === Image.Ready)
                    _d.BoundingBoxItem.markPreviewDirty()
            }
        }

        VclLabel {
            anchors.top: _image.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: 5

            width: _image.width
            height: Math.max(parent.height - _image.height - 10, 0)

            text: _image.isSet ? _d.annotation.attributes.caption : (_annotationGripLoader.annotationItem === _d ? "Set an image" : "Click to set an image")
            color: _d.annotation.attributes.captionColor
            elide: Text.ElideRight
            visible: height > 0
            wrapMode: Text.WordWrap
            font.pointSize: Runtime.idealFontMetrics.font.pointSize

            horizontalAlignment: {
                switch(_d.annotation.attributes.captionAlignment) {
                case "left": return Text.AlignLeft
                case "right": return Text.AlignRight
                }
                return Text.AlignHCenter
            }
            verticalAlignment: _image.isSet ? Text.AlignTop : Text.AlignVCenter
        }
    }
}
