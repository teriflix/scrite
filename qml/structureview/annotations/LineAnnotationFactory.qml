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

    readonly property string type: "line"

    function create(parent, x, y, orientation) {
        if(Scrite.document.readOnly)
            return null

        let w = 300
        let h = 20
        let attrs = {}
        if(orientation && orientation === "Vertical") {
            attrs["orientation"] = orientation
            w = 20
            h = 300
        }

        let geometry = Qt.rect(x-w/2, y-h/2, w, h)

        let annot = Runtime.newAnnotation(parent, type, geometry, attrs)
        Scrite.document.structure.addAnnotation(annot)

        return annot
    }

    readonly property Component delegate: AbstractAnnotationDelegate {
        id: _d

        color: Qt.rgba(0,0,0,0)
        border.width: 0

        Rectangle {
            anchors.centerIn: parent

            width: _d.annotation.attributes.orientation === "Horizontal" ? parent.width : _d.annotation.attributes.lineWidth
            height: _d.annotation.attributes.orientation === "Vertical" ? parent.height : _d.annotation.attributes.lineWidth

            color: _d.annotation.attributes.lineColor
            opacity: _d.annotation.attributes.opacity
        }
    }
}
