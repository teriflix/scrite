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
import "qrc:/qml/controls"

QtObject {
    id: root

    readonly property string type: "text"

    function create(parent, x, y) {
        if(Scrite.document.readOnly)
            return

        let w = 200
        let h = 40
        let geometry = Qt.rect(x-w/2, y-h/2, w, h)
        let annot = Runtime.newAnnotation(parent, type, geometry, null)
        Scrite.document.structure.addAnnotation(annot)

        return annot
    }

    readonly property Component delegate: AbstractAnnotationDelegate {
        id: _d

        VclLabel {
            anchors.fill: parent
            anchors.margins: 8

            clip: true
            color: _d.annotation.attributes.textColor
            text: _d.annotation.attributes.text
            wrapMode: Text.WordWrap

            font.bold: _d.annotation.attributes.fontStyle.indexOf('bold') >= 0
            font.italic: _d.annotation.attributes.fontStyle.indexOf('italic') >= 0
            font.family: _d.annotation.attributes.fontFamily
            font.underline: _d.annotation.attributes.fontStyle.indexOf('underline') >= 0
            font.pointSize: _d.annotation.attributes.fontSize

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
        }
    }
}
