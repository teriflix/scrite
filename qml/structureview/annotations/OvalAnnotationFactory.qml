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

import QtQml 2.15
import QtQuick 2.15

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/helpers"

QtObject {
    id: root

    readonly property string type: "oval"

    function create(parent, x, y) {
        if(Scrite.document.readOnly)
            return null

        if(Scrite.document.readOnly)
            return

        let w = 80
        let h = 80
        let geometry = Qt.rect(x-w/2, y-20, w, h)

        let annot = Runtime.newAnnotation(parent, type, geometry)
        Scrite.document.structure.addAnnotation(annot)
        return annot
    }

    readonly property Component delegate: AbstractAnnotationDelegate {
        id: _d

        color: Qt.rgba(0,0,0,0)
        border.width: 0
        border.color: Qt.rgba(0,0,0,0)

        PainterPathItem {
            id: _ovalPathItem

            anchors.fill: parent
            anchors.margins: _d.annotation.attributes.borderWidth

            fillColor: _d.annotation.attributes.color
            outlineColor: _d.annotation.attributes.borderColor
            outlineWidth: _d.annotation.attributes.borderWidth

            renderType: _d.annotation.attributes.fillBackground ? PainterPathItem.OutlineAndFill : PainterPathItem.OutlineOnly
            renderingMechanism: PainterPathItem.UseOpenGL

            painterPath: PainterPath {
                MoveTo {
                    x: _ovalPathItem.width
                    y: _ovalPathItem.height/2
                }
                ArcTo {
                    rectangle: Qt.rect(0, 0, _ovalPathItem.width, _ovalPathItem.height)
                    startAngle: 0
                    sweepLength: 360
                }
            }
        }
    }
}
