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
    id: lodLoader
    focus: true

    readonly property int eHIGH: 1
    readonly property int eLOW: 0
    property bool sanctioned: true

    property int lod: eLOW
    property Component lowDetailComponent
    property Component highDetailComponent
    property bool resetWidthBeforeLodChange: false
    property bool resetHeightBeforeLodChange: true

    Component {
        id: defaultDetailComponent

        Item { }
    }

    active: sanctioned
    sourceComponent: defaultDetailComponent

    Component.onCompleted: loadLodComponent()

    onLodChanged: loadLodComponent()

    function loadLodComponent() {
        active = false

        if(lod === eLOW)
            sourceComponent = lowDetailComponent ? lowDetailComponent : defaultDetailComponent
        else if(lod === eHIGH)
            sourceComponent = highDetailComponent ? highDetailComponent : defaultDetailComponent
        else
            sourceComponent = defaultDetailComponent

        active = Qt.binding( () => { return sanctioned } )
        if(resetWidthBeforeLodChange)
            Scrite.app.resetObjectProperty(lodLoader, "width")
        if(resetHeightBeforeLodChange)
            Scrite.app.resetObjectProperty(lodLoader, "height")
    }
}
