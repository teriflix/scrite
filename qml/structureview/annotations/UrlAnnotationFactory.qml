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

    readonly property string type: "url"

    function create(parent, x, y) {
        if(Scrite.document.readOnly)
            return

        let w = 300
        let h = 350 // Scrite.app.isMacOSPlatform ? 60 : 350
        let geometry = Qt.rect(x-w/2, y-20, w, h)

        let annot = Utils.newAnnotation(parent, type, geometry)
        annot.resizable = false
        Scrite.document.structure.addAnnotation(annot)
        return annot
    }

    readonly property Component delegate: AbstractAnnotationDelegate {
        id: _d

        property bool annotationHasLocalImage: _d.annotation.attributes.imageName !== undefined && _d.annotation.attributes.imageName !== ""

        color: Runtime.colors.primary.c50.background
        border {
            width: 1
            color: Runtime.colors.primary.borderColor
        }

        opacity: 1

        UrlAttributes {
            id: _urlAttribs

            url: _d.annotation.attributes.url2 !== _d.annotation.attributes.url ? _d.annotation.attributes.url : ""

            onUrlChanged: {
                if(isUrlValid) {
                    let annotAttrs = _d.annotation.attributes
                    _d.annotation.removeImage(annotAttrs.imageName)
                    annotAttrs.imageName = ""
                    _d.annotation.attributes = annotAttrs
                }
            }

            onStatusChanged: {
                if(status === UrlAttributes.Ready && isUrlValid) {
                    let annotAttrs = _d.annotation.attributes
                    let urlAttrs = attributes
                    annotAttrs.title = urlAttrs.title
                    annotAttrs.description = urlAttrs.description
                    annotAttrs.imageName = ""
                    annotAttrs.imageUrl = urlAttrs.image
                    annotAttrs.url2 = annotAttrs.url
                    _d.annotation.attributes = annotAttrs
                }
            }
        }

        Loader {
            anchors.fill: parent
            anchors.margins: 8

            clip: true
            active: _d.annotation.attributes.url !== ""

            sourceComponent: Column {
                spacing: 8

                Rectangle {
                    width: parent.width
                    height: (width/16)*9

                    color: annotationHasLocalImage ? Qt.rgba(0,0,0,0) : Runtime.colors.primary.c500.background

                    Image {
                        id: _imageItem

                        anchors.fill: parent

                        fillMode: Image.PreserveAspectCrop

                        source: {
                            if(annotationHasLocalImage)
                                _d.annotation.imageUrl(_d.annotation.attributes.imageName)
                            return Scrite.app.toHttpUrl(_d.annotation.attributes.imageUrl)
                        }

                        onStatusChanged: {
                            if(status === Image.Ready) {
                                if(!annotationHasLocalImage) {
                                    _imageItem.grabToImage(function(result) {
                                        let attrs = _d.annotation.attributes
                                        attrs.imageName = _d.annotation.addImage(result.image)
                                        _d.annotation.attributes = attrs
                                    })
                                }
                            }
                        }
                    }
                }

                VclLabel {
                    width: parent.width

                    text: _d.annotation.attributes.title
                    elide: Text.ElideRight
                    wrapMode: Text.WordWrap
                    font.bold: true
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2
                    maximumLineCount: 2
                }

                VclLabel {
                    width: parent.width

                    text: _d.annotation.attributes.description
                    elide: Text.ElideRight
                    wrapMode: Text.WordWrap
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize
                    maximumLineCount: 3
                }

                VclLabel {
                    width: parent.width

                    text: _d.annotation.attributes.url
                    color: _urlAttribs.status === UrlAttributes.Error ? "red" : "blue"
                    elide: Text.ElideRight
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize - 2
                    font.underline: _urlMouseArea.containsMouse

                    MouseArea {
                        id: _urlMouseArea

                        anchors.fill: parent

                        enabled: _urlAttribs.status !== UrlAttributes.Error
                        hoverEnabled: true

                        onClicked: Qt.openUrlExternally(_d.annotation.attributes.url)
                    }
                }
            }
        }

        BusyIcon {
            anchors.centerIn: parent

            running: _urlAttribs.status === UrlAttributes.Loading
        }

        VclLabel {
            anchors.fill: parent
            anchors.margins: 10

            text: Scrite.app.isMacOSPlatform && _annotationGripLoader.annotationItem !== _d ? "Set a URL to get a clickable link here." : "Set a URL to preview it here."
            visible: _d.annotation.attributes.url === ""
            font.pointSize: Runtime.idealFontMetrics.font.pointSize
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
