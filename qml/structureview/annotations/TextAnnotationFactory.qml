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

    readonly property string type: "line"

    function create(parent, x, y) {
        if(Scrite.document.readOnly)
            return

        let w = 200
        let h = 40
        let geometry = Qt.rect(x-w/2, y-w/2, w, h)
        let annot = Utils.newAnnotation(parent, type, geometry, null)
        Scrite.document.structure.addAnnotation(annot)

        return annot
    }

    readonly property Component delegate: AbstractAnnotationDelegate {
        id: _d

        VclLabel {
            anchors.centerIn: parent
            horizontalAlignment: {
                switch(_d.annotation.attributes.hAlign) {
                case "left": return Text.AlignLeft
                case "right": return Text.AlignRight
                }
                return Text.AlignHCenter
            }
            verticalAlignment: {
                switch(_d.annotation.attributes.vAlign) {
                case "top": return Text.AlignTop
                case "bottom": return Text.AlignBottom
                }
                return Text.AlignVCenter
            }
            text: _d.annotation.attributes.text
            color: _d.annotation.attributes.textColor
            font.family: _d.annotation.attributes.fontFamily
            font.pointSize: _d.annotation.attributes.fontSize
            font.bold: _d.annotation.attributes.fontStyle.indexOf('bold') >= 0
            font.italic: _d.annotation.attributes.fontStyle.indexOf('italic') >= 0
            font.underline: _d.annotation.attributes.fontStyle.indexOf('underline') >= 0
            width: parent.width - 15
            height: parent.height - 15
            clip: true
            wrapMode: Text.WordWrap
        }
    }
}
