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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"

// For use from within StructureView only!!!!

Item {
    id: root

    property StructureElement structureElement

    property bool tabSequenceEnabled: false
    property TabSequenceManager tabSequenceManager
    property int startTabSequence
    signal fieldAboutToReceiveFocus(int index)

    property bool hasFields: indexCardFieldsModel.count > 0

    property int lod: LodLoader.LOD.Low

    property bool sanctioned: true

    property int wrapMode: TextInput.WordWrap

    implicitHeight: hasFields ? layout.height : 0

    GridLayout {
        id: layout

        width: parent.width-20
        anchors.horizontalCenter: parent.horizontalCenter

        rows: indexCardFieldsModel.count
        flow: GridLayout.TopToBottom
        visible: rows > 0
        columns: 2
        rowSpacing: root.lod === LodLoader.LOD.Low ? 10 : 0
        columnSpacing: 10

        Repeater {
            model: indexCardFieldsModel

            delegate: VclLabel {
                required property int index
                required property string name

                Layout.topMargin: root.lod === LodLoader.LOD.Low ? 0 : 8
                Layout.alignment: Qt.AlignTop

                text: name
            }
        }

        Repeater {
            model: indexCardFieldsModel

            delegate: LodLoader {
                required property int index
                required property string description

                property string value: _private.getFieldValue(index)

                Layout.fillWidth: true

                TabSequenceItem.enabled: root.tabSequenceEnabled
                TabSequenceItem.manager: root.tabSequenceManager
                TabSequenceItem.sequence: root.startTabSequence + index
                TabSequenceItem.onAboutToReceiveFocus: {
                    root.fieldAboutToReceiveFocus(index)
                    Qt.callLater(maybeAssumeFocus)
                }

                lod: root.lod
                sanctioned: root.sanctioned
                lowDetailComponent: viewerField
                highDetailComponent: editorField

                onItemChanged: Qt.callLater(initializeItem)

                function initializeItem() {
                    if(item) {
                        item.index = index
                        item.description = description
                        item.value = value
                    }
                }

                function maybeAssumeFocus() {
                    if(focus && lod === LodLoader.LOD.High && item)
                        item.assumeFocus()
                }
            }
        }
    }

    Component {
        id: viewerField

        VclLabel {
            property int index
            property string description
            property string value

            text: value === "" ? description : value
            elide: Text.ElideRight
            opacity: value === "" ? 0.5 : 1
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            maximumLineCount: 2
        }
    }

    Component {
        id: editorField

        VclTextField {
            property int index
            property string description
            property string value

            placeholderText: text === "" ? description : ""

            text: value
            wrapMode: root.wrapMode
            readOnly: Scrite.document.readOnly
            maximumLength: 80

            onTextEdited: _private.setFieldValue(index, text)

            function assumeFocus() {
                selectAll()
                forceActiveFocus()
            }
        }
    }

    GenericArrayModel {
        id: indexCardFieldsModel
        array: Scrite.document.structure.indexCardFields
        objectMembers: ["name", "description"]
    }

    QtObject {
        id: _private

        property var fieldValues: root.structureElement ? root.structureElement.scene.indexCardFieldValues : []

        function getFieldValue(index) {
            return index < 0 || index >= fieldValues.length ? "" : fieldValues[index]
        }

        function setFieldValue(index, value) {
            if(index < 0 || index >= indexCardFieldsModel.count)
                return

            var newValues = fieldValues.length === indexCardFieldsModel.count ? fieldValues : []

            if(newValues.length === 0) {
                for(var i=0; i<indexCardFieldsModel.count; i++) {
                    if(i === index)
                        newValues.push(value)
                    else
                        newValues.push(i < fieldValues.length ? fieldValues[i] : "")
                }
            } else
                newValues[index] = value

            root.structureElement.scene.indexCardFieldValues = newValues
        }
    }
}
