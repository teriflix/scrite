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

pragma Singleton
pragma ComponentBehavior: Bound

import QtQml
import QtQuick
import QtQuick.Window
import QtQuick.Controls

import io.scrite.components

import "../../globals"
import "../../helpers"
import "../../controls"

QtObject {
    id: root

    readonly property string type: "image"

    function create(parent, x, y, filePath) {
        if(Scrite.document.readOnly)
            return

        let w = 300
        let h = 160
        let geometry = Qt.rect(x-w/2, y-h/2, w, h)

        let annot = Runtime.newAnnotation(parent, type, geometry, null)
        if(filePath && typeof filePath === "string")
            annot.setAttribute("image", annot.addImage(filePath))

        Scrite.document.structure.addAnnotation(annot)

        return annot
    }

    readonly property Component delegate: AbstractAnnotationDelegate {
        id: _d

        BoundingBoxItem.livePreview: false
        BoundingBoxItem.previewImageSource: _d.annotation.imageUrl(_image.imageName)

        clip: true
        color: _image.isSet ? (_d.annotation.attributes.fillBackground ? _d.annotation.attributes.backgroundColor : Qt.rgba(0,0,0,0)) : Runtime.colors.primary.c100.background

        Rectangle {
            anchors.fill: _image
            visible: _image.isSet && _image.status === Image.Loading
            color: Runtime.colors.primary.editor.background

            BusyIndicator {
                anchors.centerIn: parent
                scale: _d.canvasScale > 1 ? (1/_d.canvasScale) : 1
                running: parent.visible
            }
        }

        Image {
            id: _image

            property bool isSet: imageName !== "" && naturalSize !== Qt.size(0,0)
            property size naturalSize: imageName !== ""
                ? _d.annotation.estimateImageSize(imageName)
                : Qt.size(0, 0)
            property real desiredWidth: naturalSize.width > 0
                ? Math.min((_d.width - 10) * _d.canvasScale * Screen.devicePixelRatio, naturalSize.width)
                : 0
            property real lastCommittedWidth: 0
            property string imageName: _d.annotation.attributes.image

            onNaturalSizeChanged: {
                lastCommittedWidth = naturalSize.width > 0 ? desiredWidth : 0
            }

            Connections {
                target: _d
                function onCanvasScaleSettled() {
                    if (_image.desiredWidth <= 0) {
                        _image.lastCommittedWidth = 0
                    } else if (_image.lastCommittedWidth <= 0 ||
                               Math.abs(_image.desiredWidth - _image.lastCommittedWidth) / _image.lastCommittedWidth > 0.25) {
                        _image.lastCommittedWidth = _image.desiredWidth
                    }
                }
            }

            width: _d.width - 10
            height: naturalSize.width > 0 ? naturalSize.height / naturalSize.width * width : 0
            anchors.top: _d.top
            anchors.topMargin: 5
            anchors.horizontalCenter: _d.horizontalCenter

            smooth: _d.canvasScrollMoving || _d.canvasScrollFlicking ? false : true
            mipmap: smooth
            source: lastCommittedWidth > 0
                ? "image://annotation-image/" + imageName + "/" + Math.round(lastCommittedWidth)
                : ""
            fillMode: Image.Stretch
            asynchronous: true

            onStatusChanged: _d.BoundingBoxItem.livePreview = false
        }

        VclLabel {
            anchors.top: _image.bottom
            anchors.horizontalCenter: _d.horizontalCenter
            anchors.topMargin: 5

            width: _image.width
            height: Math.max(_d.height - _image.height - 10, 0)

            text: _image.isSet ? _d.annotation.attributes.caption : (_d.currentAnnotationItem === _d ? "Set an image" : "Click to set an image")
            color: Runtime.colors.tx(_d.annotation.attributes.captionColor)
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
