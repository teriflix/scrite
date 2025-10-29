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

import io.scrite.components 1.0

Loader {
    id: root

    enum LOD { High, Low }

    property int lod: LodLoader.LOD.Low

    property bool sanctioned: true
    property bool resetWidthBeforeLodChange: false
    property bool resetHeightBeforeLodChange: true

    property Component lowDetailComponent
    property Component highDetailComponent

    Component.onCompleted: _private.loadLodComponent()

    focus: true
    active: sanctioned
    sourceComponent: _private.defaultDetailComponent

    onLodChanged: _private.loadLodComponent()

    Component {
        id: defaultDetailComponent

        Item { }
    }

    QtObject {
        id: _private

        readonly property Component defaultDetailComponent: Item { }

        function loadLodComponent() {
            root.active = false

            if(lod === LodLoader.LOD.Low)
                root.sourceComponent = root.lowDetailComponent ? root.lowDetailComponent : defaultDetailComponent
            else if(lod === LodLoader.LOD.High)
                root.sourceComponent = root.highDetailComponent ? root.highDetailComponent : defaultDetailComponent
            else
                root.sourceComponent = defaultDetailComponent

            if(root.resetWidthBeforeLodChange) {
                Object.resetProperty(root, "width")
                Object.resetProperty(root, "implicitWidth")
            }

            if(root.resetHeightBeforeLodChange) {
                Object.resetProperty(root, "height")
                Object.resetProperty(root, "implicitHeight")
            }

            root.active = Qt.binding( () => { return root.sanctioned } )
        }
    }
}
