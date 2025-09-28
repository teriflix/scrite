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

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/controls"

Loader {
    id: root

    required property real zoomLevel
    required property TextEdit sceneTextEditor
    required property FontMetrics fontMetrics
    required property ScreenplayElement screenplayElement

    Component.onCompleted: _private.displayPageNumbersIfPossible()

    active: false

    sourceComponent: Item {
        Repeater {
            id: _pageBreaks

            model: _pageBreaksEvaluator.pageBreaks

            Item {
                id: _pageBreakItem

                required property var modelData

                property int pageNumber: modelData.pageNumber
                property int cursorPosition: modelData.position

                property rect cursorRect: _private.evaluateCursorRectAtPosition(cursorPosition)

                x: 0
                y: (cursorPosition >= 0 ? cursorRect.y : -_headingLayout.height)
                width: parent.width
                height: cursorRect.height

                PainterPathItem {
                    anchors.right: parent.left
                    anchors.rightMargin: 20
                    anchors.verticalCenter: parent.verticalCenter

                    VclLabel {
                        id: _sceneNumberText

                        anchors.centerIn: parent

                        font: root.fontMetrics.font
                        text: _pageBreakItem.pageNumber
                        color: Runtime.colors.primary.c600.text
                        topPadding: 3
                        leftPadding: 4
                        rightPadding: 4
                        bottomPadding: 1
                    }

                    width: Math.max(_sceneNumberText.contentWidth * 1.5, 30)
                    height: _sceneNumberText.height

                    fillColor: Runtime.colors.primary.c600.background
                    renderType: PainterPathItem.OutlineAndFill
                    outlineColor: Runtime.colors.primary.c600.background
                    outlineWidth: 1

                    painterPath: PainterPath {
                        id: _bubblePath

                        property real  arrowSize: _sceneNumberText.height/4
                        property point p1: Qt.point(itemRect.left, itemRect.top)
                        property point p2: Qt.point(itemRect.right, itemRect.top)
                        property point p3: Qt.point(itemRect.right, itemRect.center.y - arrowSize)
                        property point p4: Qt.point(itemRect.right+arrowSize, itemRect.center.y)
                        property point p5: Qt.point(itemRect.right, itemRect.center.y + arrowSize)
                        property point p6: Qt.point(itemRect.right, itemRect.bottom)
                        property point p7: Qt.point(itemRect.left, itemRect.bottom)

                        MoveTo { x: _bubblePath.p1.x; y: _bubblePath.p1.y }
                        LineTo { x: _bubblePath.p2.x; y: _bubblePath.p2.y }
                        LineTo { x: _bubblePath.p3.x; y: _bubblePath.p3.y }
                        LineTo { x: _bubblePath.p4.x; y: _bubblePath.p4.y }
                        LineTo { x: _bubblePath.p5.x; y: _bubblePath.p5.y }
                        LineTo { x: _bubblePath.p6.x; y: _bubblePath.p6.y }
                        LineTo { x: _bubblePath.p7.x; y: _bubblePath.p7.y }
                        CloseSubpath { }
                    }
                }
            }
        }

        ScreenplayElementPageBreaks {
            id: _pageBreaksEvaluator

            screenplayElement: _screenplayElement.value
            screenplayDocument: Scrite.document.loading ? null : Runtime.screenplayTextDocument
        }

        ResetOnChange {
            id: _screenplayElement

            from: null
            to: root.screenplayElement
            trackChangesOn: root.zoomLevel
        }
    }

    QtObject {
        id: _private

        property bool showPageNumbers: !Runtime.screenplayTextDocument.paused

        onShowPageNumbersChanged: displayPageNumbersIfPossible()

        function displayPageNumbersIfPossible() {
            if(showPageNumbers)
                Utils.execLater(root, Runtime.stdAnimationDuration/2, () => { root.active = true })
            else
                active = false
        }

        function evaluateCursorRectAtPosition(cursorPosition) {
            return cursorPosition >= 0 ? sceneTextEditor.positionToRectangle(cursorPosition) : Qt.rect(0,0,0,0)
        }
    }
}
