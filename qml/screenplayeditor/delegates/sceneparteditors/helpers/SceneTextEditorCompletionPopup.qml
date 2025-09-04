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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/helpers"
import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/structureview"
import "qrc:/qml/screenplayeditor"

Item {
    id: root

    required property TextEdit sceneTextEditor
    required property SceneDocumentBinder sceneDocumentBinder

    readonly property alias model: _private.completionModel

    signal completionRequest(string suggestion)

    Popup {
        id: _completionPopup

        x: -Scrite.app.boundingRect(_private.completionModel.completionPrefix, Runtime.sceneEditorFontMetrics.font).width
        width: Scrite.app.largestBoundingRect(_private.completionModel.strings, Runtime.sceneEditorFontMetrics.font).width + leftInset + rightInset + leftPadding + rightPadding + 30
        height: _completionView.height + topInset + bottomInset + topPadding + bottomPadding

        focus: false
        closePolicy: Popup.NoAutoClose

        contentItem: ListView {
            id: _completionView

            readonly property int maxVisibleItems: 7

            ScrollBar.vertical: VclScrollBar {
                flickable: _completionView
            }

            FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

            height: Math.min(contentHeight, maxVisibleItems*Runtime.sceneEditorFontMetrics.lineSpacing)

            clip: true
            model: model
            interactive: true
            currentIndex: model.currentRow

            highlightMoveDuration: 0
            highlightResizeDuration: 0
            highlight: Rectangle {
                color: Runtime.colors.primary.highlight.background
            }

            delegate: VclLabel {
                width: _completionView.width-(_completionView.contentHeight > _completionView.height ? 20 : 1)

                padding: 5

                text: string
                font: Runtime.sceneEditorFontMetrics.font
                color: index === _completionView.currentIndex ? Runtime.colors.primary.highlight.text : Runtime.colors.primary.c10.text

                MouseArea {
                    property bool singleClickAutoComplete: Runtime.screenplayEditorSettings.singleClickAutoComplete

                    anchors.fill: parent

                    cursorShape: singleClickAutoComplete ? Qt.PointingHandCursor : Qt.ArrowCursor
                    hoverEnabled: singleClickAutoComplete

                    onClicked: {
                        if(singleClickAutoComplete || _private.completionModel.currentRow === index)
                            _private.completionModel.requestCompletion( _private.completionModel.currentCompletion )
                        else
                            _private.completionModel.currentRow = index
                    }

                    onDoubleClicked: _private.completionModel.requestCompletion( _private.completionModel.currentCompletion )

                    onContainsMouseChanged: if(singleClickAutoComplete) _private.completionModel.currentRow = index
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
            trackChangesOn: sceneTextEditor.cursorRectangle.y
        }

        readonly property CompletionModel completionModel: CompletionModel {
            property bool completable: false
            property bool hasSuggestion: completionModelCount.value > 0

            property string suggestion: currentCompletion

            enabled: sceneTextEditor.activeFocus && _private.completionModelEnable.value && completable
            sortStrings: false
            maxVisibleItems: -1
            completionPrefix: sceneDocumentBinder.completionPrefix
            filterKeyStrokes: sceneTextEditor.activeFocus
            acceptEnglishStringsOnly: false
            minimumCompletionPrefixLength: 0

            onRequestCompletion: {
                root.completionRequest(suggestion)
                // Do we really need this anymore?
                // Announcement.shout("E69D2EA0-D26D-4C60-B551-FD3B45C5BE60", root.sceneDocumentBinder.scene.id)
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
        }
    }
}
