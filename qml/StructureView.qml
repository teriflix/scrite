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
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/overlays"
import "qrc:/qml/structureview"

Item {
    id: root

    signal editorRequest()
    signal releaseEditorRequest()

    Rectangle {
        anchors.fill: _canvasScroll

        color: Runtime.structureCanvasSettings.canvasColor
        opacity: Runtime.applicationSettings.colorIntensity
    }

    StructureCanvasScrollArea {
        id: _canvasScroll

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: _statusBar.top

        canvasPreviewInteracting: _canvasPreview.interacting
        canvasPreviewUpdatingThumbnail: _canvasPreview.updatingThumbnail

        newSceneColor: _actionHandlers.newSceneColor

        onEditorRequest: root.editorRequest()
        onReleaseEditorRequest: root.releaseEditorRequest()

        onSelectionModeOffRequest: { _actionHandlers.selectionMode = false }
        onDenyCanvasPreviewRequest: { _canvasPreview.allowed = false }
        onAllowCanvasPreviewRequest: { _canvasPreview.allowed = true }
    }

    StructureCanvasPreview {
        id: _canvasPreview

        anchors.right: _canvasScroll.right
        anchors.bottom: _canvasScroll.bottom
        anchors.margins: 30

        canvasScroll: _canvasScroll

        visible: allowed && Runtime.structureCanvasSettings.showPreview && parent.width > 400 && isContentOverflowing
    }

    BasicAttachmentsDropArea {
        id: _annotationAttachmentDropArea

        anchors.fill: _canvasScroll

        z: -10
        allowedType: Attachments.PhotosOnly

        onDropped: () => {
                           const pos = _canvasScroll.canvas.mapFromItem(_canvasScroll, mouse.x, mouse.y)
                           _canvasScroll.canvas.annotationLayer.dropImageAnnotation(pos.x, pos.y, attachment.filePath)
                   }
    }

    StructureViewStatusBar {
        id: _statusBar

        canvasScale: _canvasScroll.canvasScale
        canvasPreviewVisible: Runtime.structureCanvasSettings.showPreview
        canvasEpisodeBoxCount: _canvasScroll.canvasEpisodeBoxCount
        canvasItemsBoundingBox: _canvasScroll.itemsBoundingBox
        canvasScrollInteractive: _canvasScroll.interactive
        canvasScrollMinimumScale: _canvasScroll.minimumScale
        canvasScrollMaximumScale: _canvasScroll.maximumScale

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        onZoomInRequest: () => { _canvasScroll.zoomIn() }
        onZoomOutRequest: () => { _canvasScroll.zoomOut() }
        onZoomToRequest: (zoomValue) => { _canvasScroll.zoomTo(zoomValue) }
        onZoomFitRequest: (boundingBox) => { _canvasScroll.zoomFit(boundingBox) }
        onZoomOneRequest: () => {
                                  if(_canvasScroll.currentElementItem)
                                        _canvasScroll.zoomOneToItem(_canvasScroll.currentElementItem)
                                  else
                                        _canvasScroll.zoomOne()
                          }
    }

    Rectangle {
        id: _annotationAttachmentNotice

        anchors.fill: parent

        color: Color.translucent(Runtime.colors.primary.c500.background, 0.5)
        visible: _annotationAttachmentDropArea.active

        Rectangle {
            anchors.fill: _attachmentNotice
            anchors.margins: -30

            color: Runtime.colors.primary.c700.background
            radius: 4
        }

        VclLabel {
            id: _attachmentNotice

            anchors.centerIn: parent

            width: parent.width * 0.5 /* noticeWidthFactor */

            text: parent.visible ? "<b>" + _annotationAttachmentDropArea.attachment.title + "</b><br/><br/>" + "Drop here as an annotation." : ""
            color: Runtime.colors.primary.c700.text
            wrapMode: Text.WordWrap
            font.pointSize: Runtime.idealFontMetrics.font.pointSize
            horizontalAlignment: Text.AlignHCenter
        }
    }

    StructureViewActionHandlers {
        id: _actionHandlers

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: _statusBar.top

        canvasScroll: _canvasScroll
        canvasPreview: _canvasPreview

        onZoomOneRequest: () => {
                              _canvasScroll.zoomOne()
                          }

        onNewSceneRequest: () => {
                               _canvasScroll.createNewScene(Runtime.workspaceSettings.defaultSceneColor)
                           }

        onSelectAllRequest: () => {
                                _canvasScroll.canvas.selectAllElements()
                            }

        onNewAnnotationRequest: (annotationType) => {
                                    _canvasScroll.createNewAnnotation(annotationType)
                                }

        onGroupCategoryRequest: (groupCategory) => {
                                    Scrite.document.structure.preferredGroupCategory = groupCategory
                                }

        onNewColoredSceneRequest: (sceneColor) => {
                                      _canvasScroll.createNewScene(sceneColor)
                                  }

        onSelectionLayoutRequest: (layout) => {
                                      _canvasScroll.canvas.selection.layout(layout)
                                  }

        onSelectionModeChanged: () => {
                                    _canvasScroll.canvas.rubberbandSelectionMode = selectionMode
                                }

        onDeleteSelectionRequest: () => {
                                      _canvasScroll.canvas.selection.confirmDelete()
                                  }

        onDeleteElementRequest: (element) => {
                                    _canvasScroll.confirmAndDeleteElement(element)
                                }
    }

    QtObject {
        id: _private

        readonly property SequentialAnimation initSequence: SequentialAnimation {
            PauseAnimation { duration: 50 }

            ScriptAction {
                script: _canvasScroll.zoomOne()
            }

            PauseAnimation { duration: 10 }

            ScriptAction {
                script: {
                    const item = _canvasScroll.currentElementItem
                    _canvasScroll.contentX = item ? item.x - 50 : 4950
                    _canvasScroll.contentY = item ? item.y - 100 : 4950
                }
            }
        }

        Component.onCompleted: {
            Runtime.structureView = root
            Scrite.user.logActivity1("structure")
            initSequence.start()
        }

        Component.onDestruction: {
            Runtime.structureView = null
        }
    }
}
