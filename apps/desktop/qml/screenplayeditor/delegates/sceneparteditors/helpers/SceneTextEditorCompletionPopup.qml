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

pragma ComponentBehavior: Bound

import QtQml
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import io.scrite.components

import "../../../"
import "../../../../helpers"
import "../../../../globals"
import "../../../../controls"
import "../../../../structureview"

Item {
    id: root

    required property TextEdit sceneTextEditor
    required property FontMetrics fontMetrics
    required property SceneDocumentBinder sceneDocumentBinder

    readonly property alias model: _private.completionModel

    signal completionRequest(string suggestion)

    Popup {
        id: _completionPopup

        Material.elevation: 6
        Material.containerStyle: Material.Filled
        Material.roundedScale: Material.NotRounded

        x: -GMath.boundingRect(_private.completionModel.completionPrefix, root.fontMetrics.font).width
        width: root.fontMetrics.averageCharacterWidth + GMath.largestBoundingRect(_private.completionModel.strings, root.fontMetrics.font).width + leftMargin + leftPadding + rightMargin + rightPadding
        height: _completionView.count > 0 ? (_completionView.implicitHeight + topMargin + topPadding + bottomMargin + bottomPadding) : 0

        focus: false
        closePolicy: Popup.NoAutoClose

        contentItem: ListView {
            id: _completionView

            readonly property int maxVisibleItems: 7

            FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

            implicitHeight: (1+Math.min(maxVisibleItems,count)) * root.fontMetrics.lineSpacing

            clip: true
            model: _private.completionModel
            interactive: true
            currentIndex: _private.completionModel.currentRow

            highlightMoveDuration: 0
            highlightResizeDuration: 0
            highlight: Rectangle {
                color: Runtime.colors.primary.highlight.background
            }

            delegate: VclLabel {
                id: _completionDelegate

                required property int index
                required property string completionString

                width: _completionView.width

                padding: 5

                text: completionString
                font: root.fontMetrics.font
                color: index === _completionView.currentIndex ? Runtime.colors.primary.highlight.text : Runtime.colors.primary.editor.text

                MouseArea {
                    anchors.fill: parent

                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true

                    onClicked: {
                        if(_private.completionModel.currentRow === _completionDelegate.index)
                            _private.completionModel.requestCompletion( _private.completionModel.currentCompletion )
                        else
                            _private.completionModel.currentRow = _completionDelegate.index
                    }

                    onDoubleClicked: _private.completionModel.requestCompletion( _private.completionModel.currentCompletion )

                    onContainsMouseChanged: {
                        if(containsMouse)
                            _private.completionModel.currentRow = _completionDelegate.index
                    }
                }
            }
        }
    }

    QtObject {
        id: _private

        readonly property ResetOnChange completionModelEnable : ResetOnChange {
            to: true
            from: false
            delay: 250
            trackChangesOn: root.sceneTextEditor.cursorRectangle.y
        }

        readonly property CompletionModel completionModel: CompletionModel {
            property bool completable: false
            property bool hasSuggestion: count > 0

            property string suggestion: currentCompletion


            enabled: root.sceneTextEditor.activeFocus && _private.completionModelEnable.value && completable
            sortStrings: false
            maxVisibleItems: -1
            completionPrefix: root.sceneDocumentBinder.completionPrefix
            filterKeyStrokes: root.sceneTextEditor.activeFocus
            acceptEnglishStringsOnly: false
            minimumCompletionPrefixLength: 0

            onRequestCompletion: {
                root.completionRequest(suggestion)
                // Do we really need this anymore?
                // Runtime.shoutout("E69D2EA0-D26D-4C60-B551-FD3B45C5BE60", root.sceneDocumentBinder.scene.id)
            }

            onHasSuggestionChanged: {
                if(hasSuggestion) {
                    if(!_completionPopup.visible)
                        _completionPopup.open()
                } else {
                    if(_completionPopup.visible)
                        _completionPopup.close()
                }
            }

            property int binderCompletionMode: root.sceneDocumentBinder.completionMode

            onBinderCompletionModeChanged: {
                completable = false
                Runtime.execLater(_private, 250, updateModel)
            }

            function updateModel() {
                strings = root.sceneDocumentBinder.autoCompleteHints
                priorityStrings = root.sceneDocumentBinder.priorityAutoCompleteHints
                completable = root.sceneDocumentBinder.completionMode !== SceneDocumentBinder.NoCompletionMode
            }
        }
    }
}
