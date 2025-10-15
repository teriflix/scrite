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
import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"
import "qrc:/qml/controls"
import "qrc:/qml/structureview"

AbstractStructureElementUI {
    id: root

    readonly property alias isEditing: _private.isEditing
    readonly property alias isSelected: _private.isSelected
    readonly property alias elementStack: _private.elementStack
    readonly property alias isBeingDragged: _private.isBeingDragged
    readonly property alias isStackedOnTop: _private.isStackedOnTop
    readonly property alias isVisibleInViewport: _private.isVisibleInViewport

    signal editorRequest()
    signal requestContextMenu(StructureElement element)
    signal resetAnnotationGripRequest()

    Drag.active: _dragHandleMouseArea.drag.active
    Drag.source: root.element.scene
    Drag.dragType: Drag.Automatic
    Drag.hotSpot.x: _private.dragImageSize.width/2 // dragHandle.x + dragHandle.width/2
    Drag.hotSpot.y: _private.dragImageSize.height/2 // dragHandle.y + dragHandle.height/2
    Drag.supportedActions: Qt.LinkAction

    Drag.mimeData: {
        let md = {}
        md[Runtime.timelineViewSettings.dropAreaKey] = root.element.scene.id
        return md
    }

    BoundingBoxItem.evaluator: root.canvasItemsBoundingBox
    BoundingBoxItem.stackOrder: 3.0 + (root.elementIndex/Scrite.document.structure.elementCount)
    BoundingBoxItem.livePreview: false
    BoundingBoxItem.viewportRect: root.canvasScrollViewportRect
    BoundingBoxItem.viewportItem: root.canvasScrollViewport
    BoundingBoxItem.visibilityMode: _private.isStackedOnTop ? BoundingBoxItem.VisibleUponViewportIntersection : BoundingBoxItem.IgnoreVisibility
    BoundingBoxItem.previewFillColor: Scrite.app.translucent(root.element.scene.color, _private.isSelected ? 0.75 : 0.1)
    BoundingBoxItem.previewBorderColor: Scrite.app.isLightColor(root.element.scene.color) ? "black" : root.element.scene.color
    BoundingBoxItem.previewBorderWidth: _private.isSelected ? 3 : 1.5
    BoundingBoxItem.visibilityProperty: "isVisibleInViewport"

    Component.onCompleted: {
        root.determineElementStack()
        root.element.follow = root
    }

    function select() {
        Scrite.document.structure.currentElementIndex = root.elementIndex
    }

    function activate() {
        root.canvasTabSequence.releaseFocus()
        Scrite.document.structure.currentElementIndex = root.elementIndex

        root.resetAnnotationGripRequest()
        root.editorRequest()
    }

    function finishEditing() {
        root.canvasTabSequence.releaseFocus()
    }

    function zoomOneForFocus() {
        if(root.canvasScaleIsLessForEdit)
            root.zoomOneToItemRequest(root)
    }

    function determineElementStack() {
        if(root.element.stackId === "")
            _private.elementStack = null
        else if(_private.elementStack === null || _private.elementStack.stackId !== root.element.stackId)
            _private.elementStack = Scrite.document.structure.elementStacks.findStackById(root.element.stackId)
    }

    function confirmAndDeleteSelf() {
        _deleteConfirmationBoxLoader.active = true
    }

    x: _positionBinder.get.x
    y: _positionBinder.get.y
    z: _private.isSelected ? 1 : 0

    width: 350
    height: 300 + Scrite.document.structure.indexCardFields.length * 50

    visible: _private.isVisibleInViewport && _private.isStackedOnTop

    Rectangle {
        id: _background

        property color borderColor: Scrite.app.isLightColor(root.element.scene.color) ? Qt.rgba(0.75,0.75,0.75,1.0) : root.element.scene.color

        anchors.fill: parent

        color: Qt.tint(root.element.scene.color, _private.isSelected ? Runtime.colors.sceneControlTint : "#F0FFFFFF")
        border.width: _private.isSelected ? 2 : 1
        border.color: _private.isSelected ? borderColor : Qt.lighter(borderColor)

        // Move index-card around
        MouseArea {
            id: _moveMouseArea

            anchors.fill: parent

            drag.target: Scrite.document.readOnly || Scrite.document.structure.forceBeatBoardLayout ? null : root
            drag.axis: Drag.XAndYAxis
            drag.minimumX: 0
            drag.minimumY: 0
            drag.onActiveChanged: {
                root.canvasActiveFocusRequest()
                Scrite.document.structure.currentElementIndex = root.elementIndex
                if(drag.active === false) {
                    root.x = Scrite.document.structure.snapToGrid(root.x)
                    root.y = Scrite.document.structure.snapToGrid(root.y)
                } else
                    root.element.syncWithFollow = true
            }

            acceptedButtons: Qt.LeftButton

            onPressed: {
                root.element.undoRedoEnabled = true
                root.select()
                root.canvasActiveFocusRequest()
            }
            onReleased: {
                root.element.undoRedoEnabled = false
            }
        }

        // Context menu support for index card
        MouseArea {
            anchors.fill: parent

            acceptedButtons: Qt.RightButton

            onClicked: {
                root.canvasTabSequence.releaseFocus()
                root.canvasActiveFocusRequest()
                root.select()
                root.requestContextMenu(root.element)
            }
        }
    }

    ColumnLayout {
        id: _indexCardLayout

        readonly property real margin: 7

        anchors.fill: parent
        anchors.margins: margin

        spacing: 10

        LodLoader {
            id: _headingFieldLoader

            property bool hasFocus: false

            Layout.fillWidth: true

            TabSequenceItem.enabled: _private.isStackedOnTop
            TabSequenceItem.manager: root.canvasTabSequence
            TabSequenceItem.sequence: {
                const indexes = root.element.scene.screenplayElementIndexList
                if(indexes.length === 0)
                    return root.elementIndex * _private.nrFocusFieldCount + 0

                return (indexes[0] + Scrite.document.structure.elementCount) * _private.nrFocusFieldCount + 0
            }
            TabSequenceItem.onAboutToReceiveFocus: {
                Scrite.document.structure.currentElementIndex = root.elementIndex
                Qt.callLater(maybeAssumeFocus)
            }

            lod: _private.isSelected && !root.canvasScaleIsLessForEdit ? LodLoader.LOD.High : LodLoader.LOD.Low

            lowDetailComponent: TextEdit {
                id: _basicHeadingField

                Component.onCompleted: _headingFieldLoader.hasFocus = false

                SyntaxHighlighter.delegates: [
                    LanguageFontSyntaxHighlighterDelegate {
                        enabled: Runtime.screenplayEditorSettings.applyUserDefinedLanguageFonts
                        defaultFont: _basicHeadingField.font
                    }
                ]
                SyntaxHighlighter.textDocument: textDocument

                text: root.element.hasTitle ? root.element.title : "Index Card Title"
                color: root.element.hasTitle ? "black" : "gray"
                enabled: false
                readOnly: true

                topPadding: 4
                leftPadding: 4
                rightPadding: 4
                bottomPadding: 4

                selectByMouse: false
                selectByKeyboard: false

                font.bold: true
                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                // font.capitalization: element.hasNativeTitle ? Font.MixedCase : Font.AllUppercase

                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            }

            highDetailComponent: VclTextField {
                id: _headingField

                Component.onCompleted: _headingFieldLoader.hasFocus = activeFocus

                Keys.onEscapePressed: root.canvasTabSequence.releaseFocus()

                width: parent.width

                text: root.element.title
                label: ""
                enabled: !readOnly
                readOnly: Scrite.document.readOnly
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                maximumLength: 140
                placeholderText: "Scene Heading / Name"
                labelAlwaysVisible: true
                enableTransliteration: true

                topPadding: 4
                leftPadding: 4
                rightPadding: 4
                bottomPadding: 4

                font.bold: true
                font.pointSize: Runtime.idealFontMetrics.font.pointSize

                onEditingComplete: { root.element.title = text; TabSequenceItem.focusNext() }

                onActiveFocusChanged: {
                    if(activeFocus)
                        root.select()
                    _headingFieldLoader.hasFocus = activeFocus
                }
            }

            onItemChanged: Qt.callLater(maybeAssumeFocus)
            onFocusChanged: Qt.callLater(maybeAssumeFocus)

            function maybeAssumeFocus() {
                if(focus && lod === LodLoader.LOD.High && item) {
                    item.selectAll()
                    item.forceActiveFocus()
                }
            }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            currentIndex: Scrite.document.structure.indexCardContent === Structure.Synopsis ? 0 : 1

            ColumnLayout {
                spacing: 10
                visible: parent.currentIndex === 0

                LodLoader {
                    id: _synopsisFieldLoader

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    TabSequenceItem.enabled: _private.isStackedOnTop
                    TabSequenceItem.manager: root.canvasTabSequence
                    TabSequenceItem.sequence: {
                        const indexes = root.element.scene.screenplayElementIndexList
                        if(indexes.length === 0)
                            return root.elementIndex * _private.nrFocusFieldCount + 1

                        return (indexes[0] + Scrite.document.structure.elementCount) * _private.nrFocusFieldCount + 1
                    }
                    TabSequenceItem.onAboutToReceiveFocus: {
                        Scrite.document.structure.currentElementIndex = root.elementIndex
                        Qt.callLater(maybeAssumeFocus)
                    }

                    property bool hasFocus: false

                    lod: _private.isSelected && !root.canvasScaleIsLessForEdit ? LodLoader.LOD.High : LodLoader.LOD.Low
                    sanctioned: parent.visible
                    resetWidthBeforeLodChange: false
                    resetHeightBeforeLodChange: false

                    lowDetailComponent: Rectangle {
                        clip: true
                        height: _synopsisFieldLoader.height
                        border.width: _synopsisTextDisplay.truncated ? 1 : 0
                        border.color: Runtime.colors.primary.borderColor
                        color: _synopsisTextDisplay.truncated ? Qt.rgba(1,1,1,0.1) : Qt.rgba(0,0,0,0)

                        TextEdit {
                            id: _synopsisTextDisplay

                            anchors.fill: parent

                            SyntaxHighlighter.delegates: [
                                LanguageFontSyntaxHighlighterDelegate {
                                    enabled: Runtime.screenplayEditorSettings.applyUserDefinedLanguageFonts
                                    defaultFont: _synopsisTextDisplay.font
                                },

                                SpellCheckSyntaxHighlighterDelegate {
                                    enabled: Runtime.screenplayEditorSettings.enableSpellCheck
                                    cursorPosition: _synopsisTextDisplay.cursorPosition
                                }
                            ]
                            SyntaxHighlighter.textDocument: textDocument

                            topPadding: 4
                            leftPadding: 4
                            rightPadding: 4
                            bottomPadding: 4

                            text: root.element.scene.hasSynopsis ? root.element.scene.synopsis : "Describe what happens in this scene."
                            color: root.element.scene.hasTitle ? "black" : "gray"
                            enabled: false
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            readOnly: true
                            font.pointSize: Runtime.idealFontMetrics.font.pointSize

                            selectByMouse: false
                            selectByKeyboard: false

                            TextAreaSpellingSuggestionsMenu { }
                        }

                        Component.onCompleted: _synopsisFieldLoader.hasFocus = false
                    }

                    highDetailComponent: Item {
                        width: _synopsisFieldLoader.width
                        height: _synopsisFieldLoader.height

                        function assumeFocus() {
                            _synopsisField.forceActiveFocus()
                            _synopsisField.cursorPosition = _synopsisField.length
                        }

                        Flickable {
                            id: _synopsisFieldFlick

                            property bool scrollBarVisible: contentHeight > height

                            ScrollBar.vertical: VclScrollBar { flickable: _synopsisFieldFlick }

                            FlickScrollSpeedControl.factor: Runtime.workspaceSettings.flickScrollSpeedFactor

                            clip: true
                            width: parent.width
                            height: parent.height-5
                            interactive: _synopsisField.activeFocus && scrollBarVisible
                            contentWidth: _synopsisField.width
                            contentHeight: _synopsisField.height
                            flickableDirection: Flickable.VerticalFlick

                            TextArea {
                                id: _synopsisField

                                Component.onCompleted: _synopsisFieldLoader.hasFocus = activeFocus

                                Keys.onEscapePressed: root.canvasTabSequence.releaseFocus()

                                SyntaxHighlighter.delegates: [
                                    LanguageFontSyntaxHighlighterDelegate {
                                        enabled: Runtime.screenplayEditorSettings.applyUserDefinedLanguageFonts
                                        defaultFont: _synopsisField.font
                                    },

                                    SpellCheckSyntaxHighlighterDelegate {
                                        enabled: Runtime.screenplayEditorSettings.enableSpellCheck
                                        cursorPosition: _synopsisField.cursorPosition
                                    }
                                ]
                                SyntaxHighlighter.textDocument: textDocument

                                LanguageTransliterator.popup: LanguageTransliteratorPopup { }
                                LanguageTransliterator.option: Runtime.language.activeTransliterationOption
                                LanguageTransliterator.enabled: !readOnly

                                width: _synopsisFieldFlick.scrollBarVisible ? _synopsisFieldFlick.width-20 : _synopsisFieldFlick.width
                                height: Math.max(_synopsisFieldFlick.height, contentHeight + 100)

                                background: Item { }

                                topPadding: 4
                                leftPadding: 4
                                rightPadding: 4
                                bottomPadding: 4

                                selectByMouse: true
                                selectByKeyboard: true

                                text: root.element.scene.synopsis
                                readOnly: Scrite.document.readOnly
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                                placeholderText: "Describe what happens in this scene."

                                onTextChanged: root.element.scene.synopsis = text

                                onActiveFocusChanged: {
                                    if(activeFocus)
                                        root.select()
                                    else
                                        root.element.scene.trimSynopsis()
                                    _synopsisFieldLoader.hasFocus = activeFocus
                                }

                                onCursorRectangleChanged: {
                                    let y1 = cursorRectangle.y
                                    let y2 = cursorRectangle.y + cursorRectangle.height
                                    if(y1 < _synopsisFieldFlick.contentY)
                                        _synopsisFieldFlick.contentY = Math.max(y1-10, 0)
                                    else if(y2 > _synopsisFieldFlick.contentY + _synopsisFieldFlick.height)
                                        _synopsisFieldFlick.contentY = y2+10 - _synopsisFieldFlick.height
                                }

                                SpecialSymbolsSupport {
                                    anchors.top: parent.bottom
                                    anchors.left: parent.left
                                    textEditor: _synopsisField
                                    textEditorHasCursorInterface: true
                                    enabled: !Scrite.document.readOnly
                                }

                                TextAreaSpellingSuggestionsMenu { }

                                cursorDelegate: TextEditCursorDelegate {
                                    textEdit: _synopsisField
                                }
                            }
                        }

                        Rectangle {
                            anchors.bottom: parent.bottom
                            width: parent.width
                            height: _synopsisField.hovered || _synopsisField.activeFocus ? 2 : 1
                            color: Runtime.colors.primary.c500.background
                        }
                    }

                    onFocusChanged: Qt.callLater(maybeAssumeFocus)
                    onItemChanged: Qt.callLater(maybeAssumeFocus)

                    function maybeAssumeFocus() {
                        if(focus && lod === LodLoader.LOD.High && item)
                            item.assumeFocus()
                    }
                }

                IndexCardFields {
                    Layout.fillWidth: true

                    lod: _synopsisFieldLoader.lod
                    visible: hasFields
                    sanctioned: parent.visible

                    structureElement: root.element

                    tabSequenceEnabled: _private.isStackedOnTop
                    tabSequenceManager: root.canvasTabSequence
                    startTabSequence: {
                        const indexes = root.element.scene.screenplayElementIndexList
                        if(indexes.length === 0)
                            return root.elementIndex * _private.nrFocusFieldCount + 2

                        return (indexes[0] + Scrite.document.structure.elementCount) * _private.nrFocusFieldCount + 2
                    }

                    onFieldAboutToReceiveFocus: Scrite.document.structure.currentElementIndex = root.elementIndex
                }
            }

            LodLoader {
                id: _featuredImageFieldLoader

                lod: _synopsisFieldLoader.lod
                visible: sanctioned
                sanctioned: parent.currentIndex === 1
                resetWidthBeforeLodChange: false
                resetHeightBeforeLodChange: false

                lowDetailComponent: Image {
                    id: _lowLodfeaturedImageField

                    property int defaultFillMode: Image.PreserveAspectCrop

                    property string fillModeAttrib: "indexCardFillMode"

                    property Attachments sceneAttachments: root.element.scene.attachments
                    property Attachment featuredAttachment: sceneAttachments.featuredAttachment
                    property Attachment featuredImage: featuredAttachment && featuredAttachment.type === Attachment.Photo ? featuredAttachment : null

                    source: featuredImage ? featuredImage.fileSource : ""
                    mipmap: !(root.canvasScrollMoving || root.canvasScrollFlicking)
                    fillMode: {
                        if(!featuredImage)
                            return defaultFillMode
                        const ud = featuredImage.userData
                        if(ud[fillModeAttrib])
                            return ud[fillModeAttrib] === "fit" ? Image.PreserveAspectFit : Image.PreserveAspectCrop
                        return defaultFillMode
                    }

                    Loader {
                        anchors.fill: parent

                        active: !parent.featuredAttachment

                        sourceComponent: AttachmentsDropArea {
                            target: _lowLodfeaturedImageField.sceneAttachments
                            allowedType: Attachments.PhotosOnly
                            attachmentNoticeSuffix: "Drop this photo to tag it as featured image for this scene."

                            VclLabel {
                                anchors.centerIn: parent

                                width: parent.width

                                text: "Drag & Drop a Photo"
                                visible: !parent.active
                                wrapMode: Text.WordWrap
                                horizontalAlignment: Text.AlignHCenter

                                font.pointSize: Runtime.idealFontMetrics.font.pointSize
                            }

                            onDropped: {
                                attachment.featured = true
                                allowDrop()
                            }
                        }
                    }
                }

                highDetailComponent: SceneFeaturedImage {
                    scene: root.element.scene
                    mipmap: !(root.canvasScrollMoving || root.canvasScrollFlicking)
                    fillModeAttrib: "indexCardFillMode"
                    defaultFillMode: Image.PreserveAspectCrop
                }
            }
        }

        Item {
            id: _footerRow

            property bool lightBackground: Scrite.app.isLightColor(_footerBg.color)

            Layout.fillWidth: true
            Layout.preferredHeight: _footerRowLayout.height

            Rectangle {
                id: _footerBg

                property color baseColor: _background.border.color

                anchors.fill: parent
                anchors.margins: -5

                color: Qt.tint(baseColor, _private.isSelected ? "#70FFFFFF" : "#A0FFFFFF")
            }

            RowLayout {
                id: _footerRowLayout

                width: parent.width
                spacing: 5

                SceneTypeImage {
                    id: _sceneTypeImage

                    Layout.alignment: Qt.AlignBottom
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 24

                    opacity: 0.5
                    visible: sceneType !== Scene.Standard
                    sceneType: root.element.scene.type
                    showTooltip: false

                    lightBackground: _footerRow.lightBackground
                }

                ColumnLayout {
                    Layout.fillWidth: true

                    spacing: parent.spacing

                    VclLabel {
                        id: _groupsLabel

                        Layout.fillWidth: true

                        text: Scrite.document.structure.presentableGroupNames(root.element.scene.groups)
                        color: _footerRow.lightBackground ? "black" : "white"
                        visible: root.element.scene.groups.length > 0 || !root.element.scene.hasCharacters
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        font.pointSize: Scrite.app.idealAppFontSize - 2
                    }

                    VclLabel {
                        id: _characterList

                        Layout.fillWidth: true

                        text: {
                            if(root.element.scene.hasCharacters)
                                return "<b>Characters</b>: " + root.element.scene.characterNames.join(", ")
                            return ""
                        }
                        color: _footerRow.lightBackground ? "black" : "white"
                        visible: root.element.scene.hasCharacters
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        font.pointSize: Scrite.app.idealAppFontSize - 2
                    }
                }

                Item {
                    id: _dragHandle

                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 24
                    Layout.alignment: Qt.AlignBottom

                    Image {
                        id: _dragHandleImage

                        anchors.fill: parent

                        source: root.element.scene.addedToScreenplay || root.Drag.active ?
                                    (_footerRow.lightBackground ? "qrc:/icons/action/view_array.png" : "qrc:/icons/action/view_array_inverted.png") :
                                    (_footerRow.lightBackground ? "qrc:/icons/content/add_circle_outline.png" : "qrc:/icons/content/add_circle_outline_inverted.png")

                        scale: _dragHandleMouseArea.pressed ? 2 : 1
                        opacity: _private.isSelected ? 1 : 0.1

                        Behavior on scale {
                            enabled: Runtime.applicationSettings.enableAnimations
                            NumberAnimation { duration: Runtime.stdAnimationDuration }
                        }
                    }

                    MouseArea {
                        id: _dragHandleMouseArea

                        anchors.fill: parent

                        drag.target: _dragHandleImage
                        drag.onActiveChanged: {
                            _private.isBeingDragged = drag.active

                            if(drag.active)
                                root.canvasActiveFocusRequest()
                        }

                        hoverEnabled: !root.canvasScrollFlicking && !root.canvasScrollMoving && _private.isSelected

                        onPressed: {
                            _private.isBeingDragged = true

                            root.canvasActiveFocusRequest()
                            root.grabToImage(function(result) {
                                root.Drag.imageSource = result.url
                            }, _private.dragImageSize)
                        }
                        onReleased: { _private.isBeingDragged = false }
                        onClicked: {
                            if(!root.element.scene.addedToScreenplay)
                                Scrite.document.screenplay.addScene(root.element.scene)
                        }
                    }
                }
            }
        }
    }

    // Accept drops for stacking items on top of each other.
    Rectangle {
        property real alpha: _dropAreaForStacking.containsDrag ? 0.5 : 0

        anchors.fill: parent

        anchors.margins: -10

        border.width: 2
        border.color: Scrite.app.translucent("black", alpha)

        color: Scrite.app.translucent("#cfd8dc", alpha)
        radius: 6
        enabled: !_dragHandleMouseArea.drag.active && root.element.scene.addedToScreenplay

        DropArea {
            id: _dropAreaForStacking

            anchors.fill: parent

            keys: [Runtime.timelineViewSettings.dropAreaKey]

            onDropped: (drop) => {
                const otherScene = Scrite.app.typeName(drop.source) === "ScreenplayElement" ? drop.source.scene : drop.source
                if(Scrite.document.screenplay.firstIndexOfScene(otherScene) < 0) {
                    MessageBox.information("",
                        "Scenes must be added to the timeline before they can be stacked."
                    )
                    drop.ignore()
                    return
                }

                const otherSceneId = otherScene.id
                if(otherSceneId === root.element.scene.id) {
                    drop.ignore()
                    return
                }

                const otherElement = Scrite.document.structure.findElementBySceneID(otherSceneId)
                if(otherElement === null) {
                    drop.ignore()
                    return
                }

                if(root.element.scene.actIndex < 0 || otherElement.scene.actIndex < 0) {
                    MessageBox.information("",
                        "Scenes must be added to the timeline before they can be stacked."
                    )
                    drop.ignore()
                    return
                }

                if(root.element.scene.actIndex !== otherElement.scene.actIndex) {
                    MessageBox.information("",
                        "Scenes must belong to the same act for them to be stacked."
                    )
                    drop.ignore()
                    return
                }

                const otherElementIndex = Scrite.document.structure.indexOfElement(otherElement)
                Qt.callLater( function() { Scrite.document.structure.currentElementIndex = otherElementIndex } )

                const myStackId = root.element.stackId
                const otherStackId = otherElement.stackId
                drop.acceptProposedAction()

                if(myStackId === "") {
                    var uid = Scrite.app.createUniqueId()
                    root.element.stackId = uid
                    otherElement.stackId = uid
                } else {
                    otherElement.stackId = myStackId
                }

                Qt.callLater( function() { root.element.stackLeader = true } )
            }
        }
    }

    Loader {
        id: _deleteConfirmationBoxLoader

        anchors.fill: parent

        active: false

        sourceComponent: Rectangle {
            id: _deleteConfirmationBox

            property bool allowDeactivate: false
            property bool visibleInViewport: _private.isVisibleInViewport

            Component.onCompleted: {
                root.zoomOneForFocus()
                forceActiveFocus()
                Utils.execLater(_deleteConfirmationBox, 500, function() {
                    _deleteConfirmationBox.allowDeactivate = true
                })
            }

            color: Scrite.app.translucent(Runtime.colors.primary.c600.background,0.85)

            MouseArea {
                anchors.fill: parent
            }

            ColumnLayout {
                anchors.centerIn: parent

                width: parent.width-20

                spacing: 40

                VclLabel {
                    Layout.fillWidth: true

                    text: "Are you sure you want to delete this index card?"
                    color: Runtime.colors.primary.c600.text
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter

                    font.bold: true
                    font.pointSize: Runtime.idealFontMetrics.font.pointSize
                }

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter

                    spacing: 20

                    VclButton {
                        text: "Yes"
                        focusPolicy: Qt.NoFocus

                        onClicked: root.deleteElementRequest(root.element)
                    }

                    VclButton {
                        text: "No"
                        focusPolicy: Qt.NoFocus

                        onClicked: _deleteConfirmationBoxLoader.active = false
                    }
                }
            }

            onVisibleInViewportChanged: {
                if(!visibleInViewport && allowDeactivate)
                    _deleteConfirmationBoxLoader.active = false
            }

            onActiveFocusChanged: {
                if(!activeFocus && allowDeactivate)
                    _deleteConfirmationBoxLoader.active = false
            }
        }
    }

    TrackerPack {
        delay: 250

        TrackSignal { target: root.element; signal: "stackIdChanged()" }
        TrackSignal { target: Scrite.document.structure.elementStacks; signal: "objectCountChanged()" }
        TrackSignal { target: _private.elementStack; signal: "objectCountChanged()" }
        TrackSignal { target: _private.elementStack; signal: "stackLeaderChanged()" }
        TrackSignal { target: _private.elementStack; signal: "topmostElementChanged()" }

        onTracked: root.determineElementStack()
    }

    DelayedPropertyBinder {
        id: _positionBinder

        set: root.element.position
        initial: Qt.point(root.element.x, root.element.y)

        onGetChanged: {
            root.x = get.x
            root.y = get.y
        }
    }

    onVisibleChanged: {
        if(!visible) {
            if(Scrite.app.hasActiveFocus(Scrite.window,_indexCardLayout))
                root.canvasTabSequence.releaseFocus()
        }
    }

    onFinishEditingRequest: finishEditing()

    QtObject {
        id: _private

        readonly property size maxDragImageSize: Qt.size(36, 36)

        property size dragImageSize: {
            const s = root.width > root.height ? maxDragImageSize.width / root.width : maxDragImageSize.height / root.height
            return Qt.size( root.width*s, root.height*s )
        }

        property bool isEditing: _headingFieldLoader.hasFocus | _synopsisFieldLoader.hasFocus
        property bool isSelected: Scrite.document.structure.currentElementIndex === root.elementIndex
        property bool isStackedOnTop: (elementStack === null || elementStack.topmostElement === root.element)
        property bool isBeingDragged: false
        property bool isVisibleInViewport: true

        property StructureElementStack elementStack

        property int nrFocusFieldCount: {
            const nrHeadingFields = 1
            const nrSynopsisFields = Scrite.document.structure.indexCardContent === Structure.Synopsis ? 1 : 0
            const nrIndexCardFields = Scrite.document.structure.indexCardContent === Structure.Synopsis ? Scrite.document.structure.indexCardFields.length : 0
            return nrHeadingFields + nrSynopsisFields + nrIndexCardFields
        }

        onIsSelectedChanged: {
            if(_private.isSelected && (Runtime.undoStack.structureEditorActive || Scrite.document.structure.elementCount === 1))
                _synopsisFieldLoader.forceActiveFocus()
            else
                root.canvasTabSequence.releaseFocus()
        }
    }
}
