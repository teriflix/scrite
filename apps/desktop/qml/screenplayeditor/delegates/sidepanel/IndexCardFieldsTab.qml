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

import QtQml
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import io.scrite.components


import "../../../globals"
import "../../../helpers"
import "../../../dialogs"
import "../../../controls"
import "../../../structureview"
import "../sceneparteditors"

AbstractScenePartEditor {
    id: root

    Flickable {
        id: _flickable

        anchors.fill: parent
        anchors.margins: 5
        anchors.rightMargin: 0

        clip: interactive
        contentY: 0
        interactive: contentHeight > height
        contentWidth: _layout.width
        contentHeight: _layout.height
        flickableDirection: Flickable.VerticalFlick

        ScrollBar.vertical: VclScrollBar { }

        ColumnLayout {
            id: _layout

            width: _flickable.ScrollBar.vertical.needed ? _flickable.width-20 : _flickable.width

            enabled: Runtime.appFeatures.structure.enabled
            opacity: enabled ? 1 : 0.5

            IndexCardFields {
                id: _indexCardFields

                Layout.fillWidth: true

                lod: LodLoader.LOD.High
                visible: hasFields
                wrapMode: TextInput.WrapAtWordBoundaryOrAnywhere
                structureElement: root.scene.structureElement
                startTabSequence: 1
                tabSequenceManager: _tabSequence
                tabSequenceEnabled: true
            }

            VclToolButton {
                id: icfEditButton

                Layout.fillWidth: true

                visible: root.screenplayAdapter.currentIndex === root.index
                toolTipText: "Edit Index Card Fields"

                icon.source: "qrc:/icons/action/edit.png"

                onClicked: StructureIndexCardFieldsDialog.launch()
            }
        }
    }

    DisabledFeatureNotice {
        anchors.fill: parent

        color: Qt.rgba(0,0,0,0)
        visible: !Runtime.appFeatures.structure.enabled

        featureName: "Index Card Fields"
    }

    TabSequenceManager {
        id: _tabSequence

        enabled: Runtime.appFeatures.structure.enabled
        wrapAround: true
    }
}
