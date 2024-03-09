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
import QtQuick.Dialogs 1.3
import Qt.labs.settings 1.0
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "../js/utils.js" as Utils
import "./globals"

Item {
    id: optionsDialog
    width: Math.min(scriteDocumentViewItem.width-80, 1050)
    height: Math.min(scriteDocumentViewItem.height-80, 750)
    readonly property color dialogColor: ScritePrimaryColors.windowColor
    readonly property var systemFontInfo: Scrite.app.systemFontInfo()

    Component.onCompleted: {
        tabView.currentIndex = modalDialog.arguments && modalDialog.arguments.activeTabIndex ? modalDialog.arguments.activeTabIndex : 0
        modalDialog.arguments = undefined
        modalDialog.closeable = true

        Scrite.document.formatting.beginTransaction();
        Scrite.document.printFormat.beginTransaction();
    }

    Connections {
        target: modalDialog
        function onAboutToClose() {
            if(Scrite.document.formatting.hasChangesToCommit()) {
                busyOverlay.visible = true
                Scrite.app.sleep(100)
            }

            Scrite.document.formatting.commitTransaction();
            Scrite.document.printFormat.commitTransaction();
        }
    }

    TabView2 {
        id: tabView
        anchors.fill: parent
        currentIndex: 0
        onCurrentIndexChanged: {
            modalDialog.closeOnEscape = true
            modalDialog.closeable = currentIndex === 4 ? !ScriteAppFeatures.structure.enabled : true
        }

        tabsArray: [
            { "title": "Application", "tooltip": "Settings in this page apply to all documents." },
            { "title": "Page Setup", "tooltip": "Settings in this page apply to all documents." },
            { "title": "Title Page", "tooltip": "Settings in this page applies only to the current document." },
            { "title": "Formatting Rules", "tooltip": "Settings in this page applies only to the current document." },
            { "title": "Structure", "tooltip": "Edit categories and groups used for tagging scenes." }
        ]

        content: {
            switch(currentIndex) {
            case 0: return applicationSettingsComponent
            case 1: return pageSetupComponent
            case 2: return titlePageSettingsComponent
            case 3: return formattingRulesSettingsComponent
            case 4: return categoriesAndStructureComponent
            }
            return unknownSettingsComponent
        }
    }

    BusyOverlay {
        id: busyOverlay
        anchors.fill: parent
        busyMessage: "Applying changes..."
    }

    Component {
        id: applicationSettingsComponent

        PageView {
            pagesArray: ["Settings", "Fonts", "Transliteration", "Rel. Graph", "Additional"]
            currentIndex: 0
            pageContent: {
                switch(currentIndex) {
                case 1: return fontSettingsComponent
                case 2: return transliterationSettingsComponent
                case 3: return relationshipGraphSettingsComponent
                case 4: return additionalSettingsComponent
                }
                return coreSettingsComponent
            }
            pageContentSpacing: 0
            cornerContent: currentIndex === 1 ? fontSettingsCornerComponent : null
        }
    }

    Component {
        id: coreSettingsComponent

        Column {
            spacing: 16

            Item {
                height: 1
                width: parent.width - 60
                anchors.horizontalCenter: parent.horizontalCenter
            }

            GroupBox {
                label: Text { text: "Active Languages" }
                width: parent.width - 60
                anchors.horizontalCenter: parent.horizontalCenter

                Grid {
                    id: activeLanguagesView
                    width: parent.width
                    spacing: 5
                    columns: 4

                    Repeater {
                        model: Scrite.app.transliterationEngine.getLanguages()
                        delegate: CheckBox2 {
                            width: activeLanguagesView.width/activeLanguagesView.columns
                            checkable: true
                            checked: modelData.active
                            text: modelData.key
                            onToggled: Scrite.app.transliterationEngine.markLanguage(modelData.value,checked)
                            padding: 0
                        }
                    }
                }
            }

            Row {
                spacing: 20
                width: parent.width - 60
                anchors.horizontalCenter: parent.horizontalCenter

                GroupBox {
                    id: fileSaveSettingsGroupBox
                    width: (parent.width - parent.spacing)/2
                    label: Text { text: "Saving Files" }
                    clip: true

                    Column {
                        width: parent.width
                        spacing: 0
                        anchors.centerIn: parent

                        Row {
                            width: parent.width

                            CheckBox2 {
                                text: "Auto Save"
                                checked: Scrite.document.autoSave
                                onToggled: Scrite.document.autoSave = checked
                                anchors.verticalCenter: parent.verticalCenter
                                width: parent.width/2
                            }

                            TextField2 {
                                label: enabled ? "Interval in seconds:" : ""
                                enabled: Scrite.document.autoSave
                                text: enabled ? Scrite.document.autoSaveDurationInSeconds : "No Auto Save"
                                width: parent.width/2
                                validator: IntValidator {
                                    bottom: 1; top: 3600
                                }
                                onTextEdited: Scrite.document.autoSaveDurationInSeconds = parseInt(text)
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        Row {
                            width: parent.width

                            CheckBox2 {
                                text: "Limit Backups"
                                checked: Scrite.document.maxBackupCount > 0
                                onToggled: Scrite.document.maxBackupCount = checked ? 20 : 0
                                anchors.verticalCenter: parent.verticalCenter
                                width: parent.width/2
                            }

                            TextField2 {
                                label: enabled ? "Number of backups to retain:" : ""
                                enabled: Scrite.document.maxBackupCount > 0
                                text: enabled ? Scrite.document.maxBackupCount : "Unlimited Backups"
                                width: parent.width/2
                                validator: IntValidator {
                                    bottom: 1; top: 3600
                                }
                                onTextEdited: Scrite.document.maxBackupCount = parseInt(text)
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        CheckBox2 {
                            text: "Enable Restore (" + (Scrite.document.autoSave ? "New Files Only" : "All Files") + ")"
                            width: parent.width
                            checked: Scrite.vault.enabled
                            onToggled: Scrite.vault.enabled = checked
                        }
                    }
                }

                GroupBox {
                    id: screenplayEditorSettingsGroupBox
                    width: (parent.width - parent.spacing)/2
                    height: fileSaveSettingsGroupBox.height
                    label: Text {
                        text: "Screenplay Editor"
                    }

                    GridLayout {
                        id: screenplayEditorSettingsLayout
                        width: parent.width-20
                        columns: 2
                        anchors.centerIn: parent
                        readonly property int _padding: 4

                        CheckBox2 {
                            checked: ScriteRuntime.screenplayEditorSettings.enableSpellCheck
                            text: "Spell Check"
                            onToggled: ScriteRuntime.screenplayEditorSettings.enableSpellCheck = checked
                            padding: parent._padding
                            Layout.preferredWidth: parent.width / parent.columns
                        }

                        CheckBox2 {
                            checked: ScriteRuntime.screenplayEditorSettings.singleClickAutoComplete
                            text: "Auto Complete on Single Click"
                            onToggled: ScriteRuntime.screenplayEditorSettings.singleClickAutoComplete = checked
                            padding: parent._padding
                            ToolTip.text: "If checked, single click on an option in auto-complete popup will apply it in the screenplay editor."
                            ToolTip.visible: hovered
                            Layout.preferredWidth: parent.width / parent.columns
                        }

                        CheckBox2 {
                            checked: ScriteRuntime.screenplayEditorSettings.enableAutoCapitalizeSentences
                            text: "Capitalize Sentences"
                            ToolTip.text: "If checked, it automatically capitalizes first letter of every sentence while typing."
                            ToolTip.visible: hovered
                            hoverEnabled: true
                            onToggled: ScriteRuntime.screenplayEditorSettings.enableAutoCapitalizeSentences = checked
                            padding: parent._padding
                            Layout.preferredWidth: parent.width / parent.columns
                        }

                        CheckBox2 {
                            checked: ScriteRuntime.screenplayEditorSettings.enableAutoPolishParagraphs
                            text: "Add/Remove CONT'D"
                            ToolTip.text: "If checked, CONT'D will be automatically added/removed appropriately."
                            ToolTip.visible: hovered
                            hoverEnabled: true
                            onToggled: ScriteRuntime.screenplayEditorSettings.enableAutoPolishParagraphs = checked
                            padding: parent._padding
                            Layout.preferredWidth: parent.width / parent.columns
                        }

                        CheckBox2 {
                            checked: ScriteRuntime.screenplayEditorSettings.autoAdjustEditorWidthInScreenplayEditor
                            text: "Auto Adjust Editor Width"
                            ToolTip.text: "If checked, the editor width is automatically adjusted when you first launch Scrite or switch back to the screenplay tab."
                            ToolTip.visible: hovered
                            hoverEnabled: true
                            onToggled: ScriteRuntime.screenplayEditorSettings.autoAdjustEditorWidthInScreenplayEditor = checked
                            padding: parent._padding
                            Layout.preferredWidth: parent.width / parent.columns
                        }

                        CheckBox2 {
                            checked: ScriteRuntime.screenplayEditorSettings.optimiseScrolling
                            text: "Smooth Scrolling"
                            ToolTip.visible: hovered
                            ToolTip.text: "Checking this option will make scrolling in screenplay editor smooth, but uses a lot of RAM and can cause application to freeze at times while scrolling is being computed."
                            hoverEnabled: true
                            onToggled: ScriteRuntime.screenplayEditorSettings.optimiseScrolling = checked
                            padding: parent._padding
                            Layout.preferredWidth: parent.width / parent.columns
                        }
                    }
                }
            }

            Row {
                spacing: 20
                width: parent.width - 60
                anchors.horizontalCenter: parent.horizontalCenter

                GroupBox {
                    id: timelineSettingsGroupBox
                    width: (parent.width - parent.spacing)/2
                    label: Text { text: "Timeline" }

                    Column {
                        spacing: 20
                        width: parent.width

                        Text {
                            width: parent.width
                            text: "What text do you want to display on cards in the timeline?"
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        }

                        ComboBox2 {
                            width: parent.width
                            model: [
                                { "label": "Scene Heading Or Title", "value": "HeadingOrTitle" },
                                { "label": "Scene Synopsis", "value": "Synopsis" }
                            ]
                            textRole: "label"
                            currentIndex: ScriteRuntime.timelineViewSettings.textMode === "HeadingOrTitle" ? 0 : 1
                            onActivated: ScriteRuntime.timelineViewSettings.textMode = model[currentIndex].value
                        }
                    }
                }

                GroupBox {
                    width: (parent.width - parent.spacing)/2
                    label: Text { text: "Structure" }

                    Column {
                        spacing: 20
                        width: parent.width

                        Text {
                            width: parent.width
                            text: "Turn on/off pull handle animations on the structure canvas."
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        }

                        CheckBox2 {
                            checked: ScriteRuntime.structureCanvasSettings.showPullHandleAnimation
                            text: "Show Pull Handle Animation"
                            onToggled: ScriteRuntime.structureCanvasSettings.showPullHandleAnimation = checked
                        }
                    }
                }
            }

            Row {
                spacing: 20
                width: parent.width - 60
                anchors.horizontalCenter: parent.horizontalCenter

                GroupBox {
                    width: (parent.width - parent.spacing)/2
                    label: Text { text: "Normal Font Size" }

                    Row {
                        spacing: 10

                        TextField {
                            id: appFontSizeField
                            text: Scrite.app.customFontPointSize === 0 ? ScriteRuntime.idealFontMetrics.font.pointSize : Scrite.app.customFontPointSize
                            width: ScriteRuntime.idealFontMetrics.averageCharacterWidth*5
                            selectByMouse: true
                            onActiveFocusChanged: {
                                if(activeFocus)
                                    selectAll()
                            }
                            validator: IntValidator {
                                bottom: 0; top: 100
                            }
                            anchors.verticalCenter: parent.verticalCenter
                            Component.onDestruction: applyCustomFontSize()

                            function applyCustomFontSize() {
                                if(length > 0)
                                    Scrite.app.customFontPointSize = parseInt(text)
                                else
                                    Scrite.app.customFontPointSize = 0
                            }
                        }

                        Text {
                            text: "pt"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }

                GroupBox {
                    width: (parent.width - parent.spacing)/2
                    label: Text { text: Scrite.app.isMacOSPlatform ? "Scroll/Flick Speed (Windows/Linux Only)" : "Scroll/Flick Speed" }
                    enabled: !Scrite.app.isMacOSPlatform
                    opacity: enabled ? 1 : 0.5

                    Row {
                        width: parent.width

                        Slider {
                            id: flickSpeedSlider
                            from: 0.1
                            to: 3
                            value: ScriteRuntime.workspaceSettings.flickScrollSpeedFactor
                            stepSize: 0.1
                            width: parent.width - flickSpeedLabel.width - resetFlickSpeedButton.width - 2*parent.spacing
                            anchors.verticalCenter: parent.verticalCenter
                            snapMode: Slider.SnapAlways
                            onMoved: ScriteRuntime.workspaceSettings.flickScrollSpeedFactor = value
                            ToolTip.text: "Configure the scroll sensitivity of your mouse and trackpad."
                        }

                        Label {
                            id: flickSpeedLabel
                            text: Math.round( flickSpeedSlider.value*100 ) + "%"
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        ToolButton3 {
                            id: resetFlickSpeedButton
                            iconSource: "../icons/action/reset.png"
                            anchors.verticalCenter: parent.verticalCenter
                            onClicked: ScriteRuntime.workspaceSettings.flickScrollSpeedFactor = 1
                            ToolTip.text: "Reset flick/scroll speed to 100%"
                        }
                    }
                }
            }

            Item {
                height: 1
                width: parent.width - 60
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }

    Component {
        id: fontSettingsCornerComponent

        Item {

            Text {
                id: fontSettingsTip
                width: parent.width-20
                anchors.bottom: fontSettingsComboBox.top
                anchors.bottomMargin: 20
                anchors.horizontalCenter: parent.horizontalCenter
                font.pointSize: ScriteRuntime.idealFontMetrics.font.pointSize-2
                horizontalAlignment: Text.AlignLeft
                wrapMode: Text.WordWrap
                color: ScritePrimaryColors.c100.background
                property string englishFontFamily: Scrite.app.transliterationEngine.languageFont(TransliterationEngine.English).family
                text: {
                    if(ScriteRuntime.screenplayEditorSettings.applyUserDefinedLanguageFonts)
                        return "Custom fonts are used for both display and in PDF & HTML."
                    return "For display, only '" + englishFontFamily + "' will be used. Custom fonts for languages are used only in exported PDF & HTML files."
                }

                Announcement.onIncoming: (type,data) => {
                                             const stype = "" + type
                                             const sdata = "" + data
                                             if(stype === "763E8FAD-8681-4F64-B574-F9BB7CF8A7F1") {
                                                 fontSettingsTip.englishFontFamily = sdata
                                             }
                                         }
            }

            ComboBox2 {
                id: fontSettingsComboBox
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 20
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width-20
                model: ["PDF, HTML Only", "Display, PDF, HTML"]
                currentIndex: ScriteRuntime.screenplayEditorSettings.applyUserDefinedLanguageFonts ? 1 : 0
                onActivated: (index) => {
                                 ScriteRuntime.screenplayEditorSettings.applyUserDefinedLanguageFonts = (index === 1)
                             }
            }
        }
    }

    Component {
        id: fontSettingsComponent

        Column {
            id: fontSettingsUi
            spacing: 10

            readonly property var languagePreviewString: [ "Greetings", "বাংলা", "ગુજરાતી", "हिन्दी", "ಕನ್ನಡ", "മലയാളം", "मराठी", "ଓଡିଆ", "ਪੰਜਾਬੀ", "संस्कृत", "தமிழ்", "తెలుగు" ]

            Item { width: parent.width; height: 20 }

            Repeater {
                model: Scrite.app.enumerationModelForType("TransliterationEngine", "Language")

                Row {
                    readonly property int languageIndex: index
                    spacing: 10
                    width: parent.width - 20
                    anchors.horizontalCenter: parent.horizontalCenter

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        font.pointSize: ScriteRuntime.idealFontMetrics.font.pointSize
                        text: modelData.key
                        width: 175
                        horizontalAlignment: Text.AlignRight
                        font.bold: fontCombo.down
                    }

                    ComboBox2 {
                        id: fontCombo
                        property var fontFamilies: Scrite.app.transliterationEngine.availableLanguageFontFamilies(modelData.value)
                        model: fontFamilies.families
                        width: 400
                        currentIndex: fontFamilies.preferredFamilyIndex
                        onActivated: {
                            var family = fontFamilies.families[index]
                            Scrite.app.transliterationEngine.setPreferredFontFamilyForLanguage(modelData.value, family)
                            previewText.font.family = family
                            if(languageIndex === 0)
                                Announcement.shout("763E8FAD-8681-4F64-B574-F9BB7CF8A7F1", family)
                        }
                    }

                    Text {
                        id: previewText
                        anchors.verticalCenter: parent.verticalCenter
                        font.family: Scrite.app.transliterationEngine.preferredFontFamilyForLanguage(modelData.value)
                        font.pixelSize: fontCombo.font.pixelSize * 1.2
                        text: fontSettingsUi.languagePreviewString[modelData.value]
                        width: 150
                        font.bold: fontCombo.down
                    }
                }
            }

            Item { width: parent.width; height: 20 }
        }
    }

    Component {
        id: transliterationSettingsComponent

        Column {
            spacing: 10

            Item { width: parent.width; height: 10 }

            Text {
                id: titleText
                font.pointSize: ScriteRuntime.idealFontMetrics.font.pointSize
                wrapMode: Text.WordWrap
                width: parent.width - 40
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Override the built-in transliterator to use any of your operating system's input methods."
            }

            Repeater {
                model: Scrite.app.enumerationModelForType("TransliterationEngine", "Language")

                Row {
                    spacing: 10
                    width: parent.width - 20
                    anchors.horizontalCenter: parent.horizontalCenter

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        font.pointSize: ScriteRuntime.idealFontMetrics.font.pointSize
                        text: modelData.key + ": "
                        width: 175
                        horizontalAlignment: Text.AlignRight
                        font.bold: tisSourceCombo.down
                    }

                    ComboBox2 {
                        id: tisSourceCombo
                        property var sources: []
                        model: sources
                        width: 400
                        textRole: "title"
                        onActivated: {
                            var item = sources[currentIndex]
                            Scrite.app.transliterationEngine.setTextInputSourceIdForLanguage(modelData.value, item.id)
                        }

                        Component.onCompleted: {
                            var tisSources = Scrite.app.textInputManager.sourcesForLanguageJson(modelData.value)
                            var tisSourceId = Scrite.app.transliterationEngine.textInputSourceIdForLanguage(modelData.value)
                            tisSources.unshift({"id": "", "title": "Default (Inbuilt Scrite Transliterator)"})
                            enabled = tisSources.length > 1
                            sources = tisSources
                            for(var i=0; i<sources.length; i++) {
                                if(sources[i].id === tisSourceId) {
                                    currentIndex = i
                                    break
                                }
                            }
                        }
                    }
                }
            }

            Item { width: parent.width; height: 10 }
        }
    }

    Component {
        id: additionalSettingsComponent

        Column {
            spacing: 20

            Item {
                height: 30
                width: parent.width - 60
                anchors.horizontalCenter: parent.horizontalCenter
            }

            GroupBox {
                width: parent.width - 60
                anchors.horizontalCenter: parent.horizontalCenter
                title: "Structure Canvas"

                Row {
                    width: parent.width
                    spacing: 20

                    Column {
                        width: (parent.width-parent.spacing)/2
                        spacing: 10

                        CheckBox2 {
                            checkable: true
                            checked: ScriteRuntime.structureCanvasSettings.showGrid
                            text: "Show Grid in Structure Tab"
                            onToggled: ScriteRuntime.structureCanvasSettings.showGrid = checked
                            width: parent.width
                        }

                        // Colors
                        Row {
                            spacing: 10
                            width: parent.width

                            Text {
                                font.pixelSize: 14
                                text: "Background Color"
                                horizontalAlignment: Text.AlignRight
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Rectangle {
                                border.width: 1
                                border.color: ScritePrimaryColors.borderColor
                                width: 30; height: 30
                                color: ScriteRuntime.structureCanvasSettings.canvasColor
                                anchors.verticalCenter: parent.verticalCenter

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: ScriteRuntime.structureCanvasSettings.canvasColor = Scrite.app.pickColor(ScriteRuntime.structureCanvasSettings.canvasColor)
                                }
                            }

                            Text {
                                text: "Grid Color"
                                font.pixelSize: 14
                                horizontalAlignment: Text.AlignRight
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Rectangle {
                                border.width: 1
                                border.color: ScritePrimaryColors.borderColor
                                width: 30; height: 30
                                color: ScriteRuntime.structureCanvasSettings.gridColor
                                anchors.verticalCenter: parent.verticalCenter

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: ScriteRuntime.structureCanvasSettings.gridColor = Scrite.app.pickColor(ScriteRuntime.structureCanvasSettings.gridColor)
                                }
                            }
                        }

                        Row {
                            spacing: 10
                            width: parent.width
                            visible: Scrite.app.isWindowsPlatform || Scrite.app.isLinuxPlatform

                            Text {
                                id: wzfText
                                text: "Zoom Speed"
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Slider {
                                from: 1
                                to: 100
                                orientation: Qt.Horizontal
                                snapMode: Slider.SnapAlways
                                value: ScriteRuntime.scrollAreaSettings.zoomFactor * 100
                                anchors.verticalCenter: parent.verticalCenter
                                width: parent.width-wzfText.width-parent.spacing
                                onMoved: ScriteRuntime.scrollAreaSettings.zoomFactor = value / 100
                            }
                        }
                    }

                    Column {
                        width: (parent.width-parent.spacing)/2

                        Text {
                            width: parent.width
                            wrapMode: Text.WordWrap
                            text: "Starting with version 0.5.5, Scrite documents use Index Card UI by default. Older projects continue to use synopsis editor as before."
                        }

                        CheckBox2 {
                            text: "Use Index Card UI On Canvas"
                            checkable: true
                            enabled: Scrite.document.structure.elementStacks.objectCount === 0
                            checked: Scrite.document.structure.canvasUIMode === Structure.IndexCardUI
                            onToggled: mainUiContentLoader.reset( () => {
                                                               Scrite.document.structure.canvasUIMode = Structure.IndexCardUI
                                                               Scrite.document.structure.indexCardContent = Structure.Synopsis
                                                           } )
                        }
                    }
                }
            }

            Row {
                spacing: 20
                width: parent.width - 60
                anchors.horizontalCenter: parent.horizontalCenter

                GroupBox {
                    id: graphicsGroup
                    width: (parent.width - parent.spacing)/2
                    label: Text {
                        text: "Graphics"
                    }

                    Column {
                        width: graphicsGroup.availableWidth

                        CheckBox2 {
                            checked: ScriteRuntime.applicationSettings.enableAnimations
                            text: "Enable Animations"
                            onToggled: ScriteRuntime.applicationSettings.enableAnimations = checked
                        }

                        CheckBox2 {
                            checked: ScriteRuntime.applicationSettings.useSoftwareRenderer
                            text: "Use Software Renderer"
                            onToggled: {
                                ScriteRuntime.applicationSettings.useSoftwareRenderer = checked
                                Notification.active = true
                            }
                            Notification.title: "Requires Restart"
                            Notification.text: checked ? "Software renderer will be used when you restart Scrite." : "Accelerated graphics renderer will be used when you restart Scrite."
                            Notification.autoClose: false
                            ToolTip.text: "If you feel that Scrite is not responding fast enough, then you may want to switch to using a Software Renderer to speed things up. Otherwise, keep this option unchecked for best experience."
                            ToolTip.visible: hovered
                            ToolTip.delay: 1000
                        }

                        Row {
                            width: parent.width
                            spacing: 10

                            Text {
                                id: themeLabel
                                text: "Theme: "
                                leftPadding: 10
                                font.pointSize: ScriteRuntime.idealFontMetrics.font.pointSize-2
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            ComboBox2 {
                                width: parent.width - themeLabel.width - parent.spacing
                                anchors.verticalCenter: parent.verticalCenter
                                model: Scrite.app.availableThemes
                                readonly property int materialStyleIndex: Scrite.app.availableThemes.indexOf("Material");
                                currentIndex: {
                                    const idx = Scrite.app.availableThemes.indexOf(ScriteRuntime.applicationSettings.theme)
                                    if(idx < 0)
                                        return materialStyleIndex
                                    return idx
                                }
                                onCurrentTextChanged: {
                                    const oldTheme = ScriteRuntime.applicationSettings.theme
                                    ScriteRuntime.applicationSettings.theme = currentText
                                    Notification.active = oldTheme !== currentText
                                }
                                Notification.title: "Requires Restart"
                                Notification.text: "\"" + currentText + "\" theme will be used when you restart Scrite."
                                Notification.autoClose: false

                                ToolTip.text: "Scrite's UI is designed for use with Material theme and with software rendering disabled. If the UI is not rendering properly on your computer, then switching to a different theme may help."
                                ToolTip.visible: hovered
                                ToolTip.delay: 1000
                            }
                        }
                    }
                }

                GroupBox {
                    label: Text { text: "Display" }
                    width: (parent.width - parent.spacing)/2
                    clip: true

                    Column {
                        width: parent.width
                        spacing: 10

                        Row {
                            width: parent.width

                            Text {
                                width: parent.width * 0.2
                                wrapMode: Text.WordWrap
                                padding: 5
                                text: "DPI:"
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            TextField2 {
                                width: parent.width * 0.8
                                placeholderText: "leave empty for default (" + Math.round(Scrite.document.displayFormat.pageLayout.defaultResolution) + "), or enter a custom value."
                                text: Scrite.document.displayFormat.pageLayout.customResolution > 0 ? Scrite.document.displayFormat.pageLayout.customResolution : ""
                                onEditingComplete: {
                                    var value = parseFloat(text)
                                    if(isNaN(value))
                                        Scrite.document.displayFormat.pageLayout.customResolution = 0
                                    else
                                        Scrite.document.displayFormat.pageLayout.customResolution = value
                                }
                            }
                        }

                        Row {
                            width: parent.width
                            enabled: Scrite.app.isWindowsPlatform

                            Text {
                                width: parent.width * 0.2
                                wrapMode: Text.WordWrap
                                padding: 5
                                text: "Scale:"
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            TextField2 {
                                width: parent.width * 0.8
                                placeholderText: "Default: 1.0. Requires restart if changed."
                                text: Scrite.app.isWindowsPlatform ? Scrite.app.getWindowsEnvironmentVariable("SCRITE_UI_SCALE_FACTOR", "1.0") : "1.0"
                                onEditingComplete: {
                                    var value = parseFloat(text)
                                    if(isNaN(value))
                                        value = 1.0

                                    value = Math.min(Math.max(0.1,value),10)
                                    value = Math.round(value*100)/100

                                    Scrite.app.removeWindowsEnvironmentVariable("SCRITE_DPI_MODE")
                                    Scrite.app.changeWindowsEnvironmentVariable("SCRITE_UI_SCALE_FACTOR", ""+value)
                                }
                            }
                        }
                    }
                }
            }

            Row {
                spacing: 20
                width: parent.width - 60
                anchors.horizontalCenter: parent.horizontalCenter

                GroupBox {
                    title: "Window Tabs"
                    width: (parent.width - parent.spacing)/2

                    Column {
                        width: parent.width
                        spacing: 5

                        Text {
                            id: windowTabsExplainerText
                            width: parent.width
                            font.pointSize: ScriteRuntime.idealFontMetrics.font.pointSize-2
                            text: "Move Notebook into the Structure tab to see all three aspects of your screenplay in a single view. (Note: This works when Scrite window size is atleast 1600 px wide.)"
                            wrapMode: Text.WordWrap
                        }

                        CheckBox2 {
                            checked: mainScriteDocumentView.showNotebookInStructure
                            enabled: mainScriteDocumentView.canShowNotebookInStructure
                            text: "Move Notebook into the Structure tab"
                            onToggled: {
                                ScriteRuntime.workspaceSettings.showNotebookInStructure = checked
                                if(checked) {
                                    ScriteRuntime.workspaceSettings.animateStructureIcon = true
                                    ScriteRuntime.workspaceSettings.animateNotebookIcon = true
                                }
                            }
                        }

                        CheckBox2 {
                            checked: ScriteRuntime.workspaceSettings.showScritedTab
                            text: "Show Scrited Tab"
                            onToggled: {
                                ScriteRuntime.workspaceSettings.showScritedTab = checked
                                if(!checked && ScriteRuntime.mainWindowTab === ScriteRuntime.e_ScritedTab) {
                                    try {
                                        ScriteRuntime.activateMainWindowTab(ScriteRuntime.e_ScreenplayTab)
                                    } catch(e) {
                                        Scrite.app.log(e)
                                    }
                                }
                            }
                        }
                    }
                }

                GroupBox {
                    title: "PDF Export"
                    width: (parent.width - parent.spacing)/2

                    Column {
                        width: parent.width
                        spacing: 5

                        Text {
                            width: parent.width
                            height: windowTabsExplainerText.height
                            font.pointSize: ScriteRuntime.idealFontMetrics.font.pointSize-2
                            text: "If you are facing issues with PDF export, then choose Printer Driver in the combo-box below. Otherwise we strongly advise you to use PDF Driver."
                            wrapMode: Text.WordWrap
                        }

                        ComboBox2 {
                            enabled: false // Qt 5.15.7's PdfWriter is broken!
                            width: parent.width
                            model: [
                                { "key": "PDF Driver", "value": true },
                                { "key": "Printer Driver", "value": false }
                            ]
                            textRole: "key"
                            // currentIndex: ScriteRuntime.pdfExportSettings.usePdfDriver ? 0 : 1
                            currentIndex: 1
                            onCurrentIndexChanged: ScriteRuntime.pdfExportSettings.usePdfDriver = (currentIndex === 0)
                        }
                    }
                }
            }

            Item {
                height: 30
                width: parent.width - 60
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }

    Component {
        id: relationshipGraphSettingsComponent

        Item {
            GroupBox {
                width: parent.width-60
                anchors.top: parent.top
                anchors.topMargin: 40
                anchors.horizontalCenter: parent.horizontalCenter
                label: Text {
                    text: "Relationship Graph"
                }

                Column {
                    spacing: 10
                    width: parent.width

                    TextArea {
                        font.pointSize: ScriteRuntime.idealFontMetrics.font.pointSize
                        width: parent.width
                        wrapMode: Text.WordWrap
                        textFormat: TextArea.RichText
                        readOnly: true
                        background: Item { }
                        text: "<p>Relationship graphs are automatically constructed using the Force Directed Graph algorithm. You can configure attributes of the algorithm using the fields below. The default values work for most cases.</p>" +
                              "<font size=\"-1\"><ul><li><strong>Max Time</strong> is the number of milliseconds the algorithm can take to compute the graph.</li><li><strong>Max Iterations</strong> is the number of times within max-time the graph can go over each character to determine the ideal placement of nodes and edges in the graph.</li></ul></font>"
                    }

                    Row {
                        width: parent.width
                        spacing: parent.spacing

                        Column {
                            width: (parent.width - parent.spacing)/2

                            Text {
                                font.bold: true
                                font.pointSize: ScriteRuntime.idealFontMetrics.font.pointSize
                                text: "Max Time In Milliseconds"
                                width: parent.width
                            }

                            Text {
                                font.bold: false
                                font.pointSize: ScriteRuntime.idealFontMetrics.font.pointSize-2
                                text: "Default: 1000"
                            }

                            TextField {
                                id: txtMaxTime
                                text: ScriteRuntime.notebookSettings.graphLayoutMaxTime
                                width: parent.width
                                placeholderText: "if left empty, default of 1000 will be used"
                                validator: IntValidator {
                                    bottom: 250
                                    top: 5000
                                }
                                onTextEdited: {
                                    if(length === 0 || text.trim() === "")
                                        ScriteRuntime.notebookSettings.graphLayoutMaxTime = 1000
                                    else
                                        ScriteRuntime.notebookSettings.graphLayoutMaxTime = parseInt(text)
                                }
                                KeyNavigation.tab: txtMaxIterations
                            }
                        }

                        Column {
                            width: (parent.width - parent.spacing)/2

                            Text {
                                font.bold: true
                                font.pointSize: ScriteRuntime.idealFontMetrics.font.pointSize
                                text: "Max Iterations"
                                width: parent.width
                            }

                            Text {
                                font.bold: false
                                font.pointSize: ScriteRuntime.idealFontMetrics.font.pointSize-2
                                text: "Default: 50000"
                            }

                            TextField {
                                id: txtMaxIterations
                                text: ScriteRuntime.notebookSettings.graphLayoutMaxIterations
                                width: parent.width
                                placeholderText: "if left empty, default of 50000 will be used"
                                validator: IntValidator {
                                    bottom: 1000
                                    top: 250000
                                }
                                onTextEdited: {
                                    if(length === 0 || text.trim() === "")
                                        ScriteRuntime.notebookSettings.graphLayoutMaxIterations = 50000
                                    else
                                        ScriteRuntime.notebookSettings.graphLayoutMaxIterations = parseInt(text)
                                }
                                KeyNavigation.tab: txtMaxTime
                            }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: pageSetupComponent

        Item {
            property real labelWidth: 60
            property var fieldsModel: Scrite.app.enumerationModelForType("HeaderFooter", "Field")
            property PageSetup pageSetupSettings: Scrite.document.pageSetup

            Column {
                width: parent.width - 60
                spacing: 20
                anchors.centerIn: parent

                Row {
                    spacing: 20
                    width: parent.width

                    Row {
                        spacing: 20
                        width: (parent.width-parent.spacing)/2

                        Text {
                            id: paperSizeLabel
                            text: "Paper Size"
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        ComboBox2 {
                            width: parent.width - parent.spacing - paperSizeLabel.width
                            textRole: "key"
                            currentIndex: pageSetupSettings.paperSize
                            anchors.verticalCenter: parent.verticalCenter
                            onActivated: {
                                pageSetupSettings.paperSize = currentIndex
                                Scrite.document.formatting.pageLayout.paperSize = currentIndex
                                Scrite.document.printFormat.pageLayout.paperSize = currentIndex
                            }
                            model: Scrite.app.enumerationModelForType("ScreenplayPageLayout", "PaperSize")
                        }
                    }

                    Row {
                        spacing: 20
                        width: (parent.width-parent.spacing)/2

                        Text {
                            id: timePerPageLabel
                            text: "Time Per Page:"
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        TextField2 {
                            label: "Seconds (15 - 300)"
                            labelAlwaysVisible: true
                            text: Scrite.document.printFormat.secondsPerPage
                            validator: IntValidator { bottom: 15; top: 300 }
                            onTextEdited: Scrite.document.printFormat.secondsPerPage = parseInt(text)
                            width: ScriteRuntime.sceneEditorFontMetrics.averageCharacterWidth*3
                        }

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: "seconds per page."
                        }
                    }
                }

                Row {
                    width: parent.width
                    spacing: 10

                    GroupBox {
                        width: (parent.width-parent.spacing)/2
                        label: Text { text: "Header" }

                        Row {
                            width: parent.width
                            height: 80

                            Item {
                                width: parent.width/3
                                height: childrenRect.height

                                Column {
                                    width: parent.width-10
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    spacing: 10

                                    Text {
                                        text: "Left"
                                    }

                                    ComboBox2 {
                                        width: parent.width
                                        model: fieldsModel
                                        textRole: "key"
                                        currentIndex: pageSetupSettings.headerLeft
                                        onActivated: pageSetupSettings.headerLeft = currentIndex
                                    }
                                }
                            }

                            Item {
                                width: parent.width/3
                                height: childrenRect.height

                                Column {
                                    width: parent.width-10
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    spacing: 10

                                    Text {
                                        text: "Center"
                                    }

                                    ComboBox2 {
                                        width: parent.width
                                        model: fieldsModel
                                        textRole: "key"
                                        currentIndex: pageSetupSettings.headerCenter
                                        onActivated: pageSetupSettings.headerCenter = currentIndex
                                    }
                                }
                            }

                            Item {
                                width: parent.width/3
                                height: childrenRect.height

                                Column {
                                    width: parent.width-10
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    spacing: 10

                                    Text {
                                        text: "Right"
                                    }

                                    ComboBox2 {
                                        width: parent.width
                                        model: fieldsModel
                                        textRole: "key"
                                        currentIndex: pageSetupSettings.headerRight
                                        onActivated: pageSetupSettings.headerRight = currentIndex
                                    }
                                }
                            }
                        }
                    }

                    GroupBox {
                        width: (parent.width-parent.spacing)/2
                        label: Text { text: "Footer" }

                        Row {
                            width: parent.width
                            height: 80

                            Item {
                                width: parent.width/3
                                height: childrenRect.height

                                Column {
                                    width: parent.width-10
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    spacing: 10

                                    Text {
                                        text: "Left"
                                    }

                                    ComboBox2 {
                                        width: parent.width
                                        model: fieldsModel
                                        textRole: "key"
                                        currentIndex: pageSetupSettings.footerLeft
                                        onActivated: pageSetupSettings.footerLeft = currentIndex
                                    }
                                }
                            }

                            Item {
                                width: parent.width/3
                                height: childrenRect.height

                                Column {
                                    width: parent.width-10
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    spacing: 10

                                    Text {
                                        text: "Center"
                                    }

                                    ComboBox2 {
                                        width: parent.width
                                        model: fieldsModel
                                        textRole: "key"
                                        currentIndex: pageSetupSettings.footerCenter
                                        onActivated: pageSetupSettings.footerCenter = currentIndex
                                    }
                                }
                            }

                            Item {
                                width: parent.width/3
                                height: childrenRect.height

                                Column {
                                    width: parent.width-10
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    spacing: 10

                                    Text {
                                        text: "Right"
                                    }

                                    ComboBox2 {
                                        width: parent.width
                                        model: fieldsModel
                                        textRole: "key"
                                        currentIndex: pageSetupSettings.footerRight
                                        onActivated: pageSetupSettings.footerRight = currentIndex
                                    }
                                }
                            }
                        }
                    }
                }

                GroupBox {
                    width: parent.width
                    height: watermarkSettingsLayout.height + 40
                    label: Text { text: "Watermark" }

                    Row {
                        id: watermarkSettingsLayout
                        spacing: 30
                        anchors.horizontalCenter: parent.horizontalCenter
                        enabled: ScriteAppFeatures.watermark.enabled

                        Grid {
                            columns: 2
                            spacing: 10
                            verticalItemAlignment: Grid.AlignVCenter

                            Text {
                                text: "Enable"
                                width: labelWidth
                                horizontalAlignment: Text.AlignRight
                            }

                            CheckBox2 {
                                text: checked ? "ON" : "OFF"
                                checked: ScriteAppFeatures.watermark.enabled ? pageSetupSettings.watermarkEnabled : true
                                onToggled: pageSetupSettings.watermarkEnabled = checked
                            }

                            Text {
                                text: "Text"
                                width: labelWidth
                                horizontalAlignment: Text.AlignRight
                            }

                            TextField2 {
                                width: 300
                                text: ScriteAppFeatures.watermark.enabled ? pageSetupSettings.watermarkText : "Scrite"
                                onTextEdited: pageSetupSettings.watermarkText = text
                                enabled: pageSetupSettings.watermarkEnabled
                                enableTransliteration: true
                            }

                            Text {
                                text: "Color"
                                width: labelWidth
                                horizontalAlignment: Text.AlignRight
                            }

                            Rectangle {
                                border.width: 1
                                border.color: ScritePrimaryColors.borderColor
                                color: pageSetupSettings.watermarkColor
                                width: 30; height: 30
                                enabled: pageSetupSettings.watermarkEnabled
                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: pageSetupSettings.watermarkColor = Scrite.app.pickColor(pageSetupSettings.watermarkColor)
                                }
                            }
                        }

                        Rectangle {
                            width: 1
                            height: parent.height
                            color: ScritePrimaryColors.borderColor
                        }

                        Grid {
                            columns: 2
                            spacing: 10
                            verticalItemAlignment: Grid.AlignVCenter

                            Text {
                                text: "Font Family"
                                width: labelWidth
                                horizontalAlignment: Text.AlignRight
                            }

                            ComboBox2 {
                                width: 300
                                model: systemFontInfo.families
                                currentIndex: systemFontInfo.families.indexOf(pageSetupSettings.watermarkFont)
                                onCurrentIndexChanged: pageSetupSettings.watermarkFont = systemFontInfo.families[currentIndex]
                                enabled: pageSetupSettings.watermarkEnabled
                            }

                            Text {
                                text: "Font Size"
                                width: labelWidth
                                horizontalAlignment: Text.AlignRight
                            }

                            SpinBox {
                                width: 300
                                from: 9; to: 200; stepSize: 1
                                editable: true
                                value: pageSetupSettings.watermarkFontSize
                                onValueModified: pageSetupSettings.watermarkFontSize = value
                                enabled: pageSetupSettings.watermarkEnabled
                            }

                            Text {
                                text: "Rotation"
                                width: labelWidth
                                horizontalAlignment: Text.AlignRight
                            }

                            SpinBox {
                                width: 300
                                from: -180; to: 180
                                editable: true
                                value: pageSetupSettings.watermarkRotation
                                textFromValue: function(value,locale) { return value + " degrees" }
                                validator: IntValidator { top: 360; bottom: 0 }
                                onValueModified: pageSetupSettings.watermarkRotation = value
                                enabled: pageSetupSettings.watermarkEnabled
                            }
                        }
                    }

                    DisabledFeatureNotice {
                        anchors.fill: parent
                        visible: !ScriteAppFeatures.watermark.enabled
                        color: Qt.rgba(1,1,1,0.9)
                        featureName: "Watermark Settings"
                    }
                }

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20

                    Button2 {
                        text: "Save As Default"
                        onClicked: pageSetupSettings.saveAsDefaults()
                        enabled: !pageSetupSettings.usingSavedDefaults
                    }

                    Button2 {
                        text: "Use Saved Defaults"
                        onClicked: pageSetupSettings.useSavedDefaults()
                        enabled: !pageSetupSettings.usingSavedDefaults
                    }

                    Button2 {
                        text: "Use Factory Defaults"
                        onClicked: pageSetupSettings.useFactoryDefaults()
                        enabled: !pageSetupSettings.usingFactoryDefaults
                    }
                }
            }
        }
    }

    Component {
        id: titlePageSettingsComponent

        Item {
            id: titlePageContainer
            readonly property real fieldLabelWidth: 60

            TabSequenceManager {
                id: titlePageFieldsTabSequence
                wrapAround: true
            }

            ColumnLayout {
                id: titlePageSettingsLayout
                width: titlePageContainer.width-160
                spacing: 30
                anchors.centerIn: parent

                // Cover page photo field
                Rectangle {
                    id: coverPageEdit
                    /*
                      At best we can paint a 464x261 point photo on the cover page. Nothing more.
                      So, we need to provide a image preview in this aspect ratio.
                      */
                    border.width: Scrite.document.screenplay.coverPagePhoto === "" ? 1 : 0
                    border.color: "black"
                    Layout.preferredWidth: 400
                    Layout.preferredHeight: 255
                    Layout.alignment: Qt.AlignHCenter

                    Loader {
                        id: coverPagePhotoLoader
                        anchors.fill: parent
                        anchors.margins: 1
                        active: Scrite.document.screenplay.coverPagePhoto !== ""
                        sourceComponent: Item {
                            Image {
                                anchors.fill: parent
                                fillMode: Image.PreserveAspectCrop
                                asynchronous: true
                                source: coverPagePhoto.source
                                visible: (coverPagePhoto.go && coverPagePhoto.status === Image.Ready) && (coverPagePhoto.paintedWidth < width || coverPagePhoto.paintedHeight < height)
                                opacity: 0.1 * coverPagePhoto.opacity
                                cache: false
                            }

                            Image {
                                id: coverPagePhoto
                                anchors.fill: parent
                                smooth: true; mipmap: true
                                asynchronous: true
                                fillMode: Image.PreserveAspectFit
                                source: go ? "file:///" + Scrite.document.screenplay.coverPagePhoto : ""
                                opacity: coverPagePhotoMouseArea.containsMouse ? 0.25 : 1
                                cache: false
                                property bool go: false

                                BusyIcon {
                                    anchors.centerIn: parent
                                    running: parent.status === Image.Loading || !parent.go
                                }

                                Component.onCompleted: Utils.execLater(coverPagePhoto, 400, () => {
                                                                           coverPagePhoto.go = true
                                                                       } )
                            }
                        }
                    }

                    Text {
                        anchors.fill: parent
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        opacity: coverPagePhotoMouseArea.containsMouse ? 1 : (Scrite.document.screenplay.coverPagePhoto === "" ? 0.5 : 0)
                        text: Scrite.document.screenplay.coverPagePhoto === "" ? "Click here to set the cover page photo" : "Click here to change the cover page photo"
                    }

                    MouseArea {
                        id: coverPagePhotoMouseArea
                        anchors.fill: parent
                        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                        hoverEnabled: true
                        enabled: !Scrite.document.readOnly
                        onClicked: fileDialog.open()
                    }

                    AttachmentsDropArea2 {
                        anchors.fill: parent
                        attachmentNoticeSuffix: "Drop to set as cover page photo."
                        visible: !Scrite.document.readOnly
                        allowedType: Attachments.PhotosOnly
                        onDropped: {
                            Scrite.document.screenplay.clearCoverPagePhoto()
                            var filePath = attachment.filePath
                            Qt.callLater( function(fp) {
                                Scrite.document.screenplay.setCoverPagePhoto(fp)
                            }, filePath)
                        }
                    }

                    Column {
                        spacing: 0
                        anchors.left: parent.right
                        anchors.leftMargin: 20
                        visible: Scrite.document.screenplay.coverPagePhoto !== ""
                        enabled: visible && !Scrite.document.readOnly

                        Text {
                            text: "Cover Photo Size"
                            font.bold: true
                            topPadding: 5
                            bottomPadding: 5
                            color: ScritePrimaryColors.c300.text
                            opacity: enabled ? 1 : 0.5
                        }

                        RadioButton2 {
                            text: "Small"
                            checked: Scrite.document.screenplay.coverPagePhotoSize === Screenplay.SmallCoverPhoto
                            onToggled: Scrite.document.screenplay.coverPagePhotoSize = Screenplay.SmallCoverPhoto
                        }

                        RadioButton2 {
                            text: "Medium"
                            checked: Scrite.document.screenplay.coverPagePhotoSize === Screenplay.MediumCoverPhoto
                            onToggled: Scrite.document.screenplay.coverPagePhotoSize = Screenplay.MediumCoverPhoto
                        }

                        RadioButton2 {
                            text: "Large"
                            checked: Scrite.document.screenplay.coverPagePhotoSize === Screenplay.LargeCoverPhoto
                            onToggled: Scrite.document.screenplay.coverPagePhotoSize = Screenplay.LargeCoverPhoto
                        }

                        Button2 {
                            text: "Remove"
                            onClicked: Scrite.document.screenplay.clearCoverPagePhoto()
                        }
                    }

                    FileDialog {
                        id: fileDialog
                        nameFilters: ["Photos (*.jpg *.png *.bmp *.jpeg)"]
                        selectFolder: false
                        selectMultiple: false
                        sidebarVisible: true
                        selectExisting: true
                        dirUpAction.shortcut: "Ctrl+Shift+U" // The default Ctrl+U interfers with underline
                        onAccepted: {
                            if(fileUrl != "")
                                Scrite.document.screenplay.setCoverPagePhoto(Scrite.app.urlToLocalFile(fileUrl))
                        }
                        folder: ScriteRuntime.workspaceSettings.lastOpenPhotosFolderUrl
                        onFolderChanged: ScriteRuntime.workspaceSettings.lastOpenPhotosFolderUrl = folder
                    }
                }

                Grid {
                    id: titlePageFields
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    rowSpacing: 5
                    columnSpacing: 40
                    enabled: !Scrite.document.readOnly
                    flow: Flow.TopToBottom
                    columns: 2

                    Repeater {
                        model: ListModel {
                            ListElement { name: "Title";      fieldSize: 100;    key: "title"       }
                            ListElement { name: "Subtitle";   fieldSize: 100;    key: "subtitle"    }
                            ListElement { name: "Based on";   fieldSize: 100;    key: "basedOn"     }
                            ListElement { name: "Version";    fieldSize: 20 ;    key: "version"     }
                            ListElement { name: "Written by"; fieldSize: 100;    key: "authorValue" }
                            ListElement { name: "Contact";    fieldSize: 100;    key: "contact"     }
                            ListElement { name: "Address";    fieldSize: 100;    key: "address"     }
                            ListElement { name: "Email";      fieldSize: 100;    key: "email"       }
                            ListElement { name: "Phone";      fieldSize: 100;    key: "phone"       }
                            ListElement { name: "Website";    fieldSize: 100;    key: "website"     }
                        }

                        Item {
                            required property int index
                            required property string name
                            required property int fieldSize
                            required property string key

                            width: (titlePageFields.width-titlePageFields.columnSpacing)/2
                            height: _tpfRow.height

                            TextLimiter {
                                id: _tpfLimiter
                                maxWordCount: fieldSize
                                maxLetterCount: fieldSize
                                countMode: TextLimiter.CountInText
                                text: _tpfScreenplayProperty.value
                            }

                            PropertyAlias {
                                id: _tpfScreenplayProperty
                                sourceObject: Scrite.document.screenplay
                                sourceProperty: key
                            }

                            Row {
                                id: _tpfRow
                                width: parent.width
                                spacing: 10

                                Text {
                                    width: fieldLabelWidth
                                    horizontalAlignment: Text.AlignRight
                                    text: name
                                    font.pointSize: ScriteRuntime.idealFontMetrics.font.pointSize-2
                                    anchors.verticalCenter: parent.verticalCenter
                                    color: ScritePrimaryColors.c800.background
                                }

                                TextField2 {
                                    id: _tpfField
                                    width: parent.width-parent.spacing-fieldLabelWidth
                                    text: _tpfScreenplayProperty.value
                                    selectByMouse: true
                                    placeholderText: activeFocus ?
                                                     (text === "" ? ("(max " + _tpfLimiter.maxLetterCount + " letters)") : (_tpfLimiter.letterCount + "/" + _tpfLimiter.maxLetterCount)) :
                                                     ""
                                    labelColor: _tpfLimiter.limitReached ? "red" : "gray"
                                    labelTextAlign: Text.AlignRight
                                    onTextEdited: {
                                        _tpfLimiter.text = text
                                        _tpfScreenplayProperty.value = text
                                    }
                                    font.pointSize: ScriteRuntime.idealFontMetrics.font.pointSize+1
                                    enableTransliteration: true
                                    TabSequenceItem.manager: titlePageFieldsTabSequence
                                    TabSequenceItem.sequence: index
                                }
                            }
                        }
                    }
                }

                Item {
                    id: titlePageOptions

                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true
                    Layout.preferredHeight: useAsDefaultsButton.height

                    CheckBox2 {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter

                        text: "Include Title Page In Preview"
                        checked: ScriteRuntime.screenplayEditorSettings.includeTitlePageInPreview
                        onToggled: ScriteRuntime.screenplayEditorSettings.includeTitlePageInPreview = checked
                    }

                    Button2 {
                        id: useAsDefaultsButton
                        anchors.centerIn: parent

                        text: "Use As Defaults"
                        hoverEnabled: true
                        ToolTip.visible: hovered && defaultsSavedNotice.opacity === 0
                        ToolTip.text: "Click this button to use Address, Author, Contact, Email, Phone and Website field values from this dialogue as default from now on."
                        ToolTip.delay: 1000
                        onClicked: {
                            ScriteRuntime.titlePageSettings.author = Scrite.document.screenplay.author
                            ScriteRuntime.titlePageSettings.contact = Scrite.document.screenplay.contact
                            ScriteRuntime.titlePageSettings.address = Scrite.document.screenplay.address
                            ScriteRuntime.titlePageSettings.email = Scrite.document.screenplay.email
                            ScriteRuntime.titlePageSettings.phone = Scrite.document.screenplay.phoneNumber
                            ScriteRuntime.titlePageSettings.website = Scrite.document.screenplay.website
                            defaultsSavedNotice.opacity = 1
                        }
                    }

                    CheckBox2 {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter

                        text: "Include Timestamp"
                        checked: ScriteRuntime.titlePageSettings.includeTimestamp
                        onToggled: ScriteRuntime.titlePageSettings.includeTimestamp = checked
                    }
                }

            }

            Text {
                id: defaultsSavedNotice
                anchors.top: titlePageSettingsLayout.bottom
                anchors.topMargin: 10
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Address, Author, Contact, Email, Phone and Website field values saved as default."
                opacity: 0
                onOpacityChanged: {
                    if(opacity > 0)
                        Utils.execLater(defaultsSavedNotice, 2500, function() { defaultsSavedNotice.opacity = 0 })
                }
            }
        }
    }

    Component {
        id: formattingRulesSettingsComponent

        PageView {
            id: formattingRulesPageView
            readonly property real labelWidth: 125
            property var pageData: pagesArray[currentIndex]
            property SceneElementFormat printElementFormat: Scrite.document.printFormat.elementFormat(pageData.elementType)
            property SceneElementFormat displayElementFormat: Scrite.document.formatting.elementFormat(pageData.elementType)

            pagesArray: [
                { "elementName": "Heading", "elementType": SceneElement.Heading },
                { "elementName": "Action", "elementType": SceneElement.Action },
                { "elementName": "Character", "elementType": SceneElement.Character },
                { "elementName": "Dialogue", "elementType": SceneElement.Dialogue },
                { "elementName": "Parenthetical", "elementType": SceneElement.Parenthetical },
                { "elementName": "Shot", "elementType": SceneElement.Shot },
                { "elementName": "Transition", "elementType": SceneElement.Transition },
            ]

            pageTitleRole: "elementName"

            currentIndex: 0

            Component.onCompleted: Announcement.shout("DF77A452-FDB2-405C-8A0F-E48982012D36", "save")
            Component.onDestruction: Announcement.shout("DF77A452-FDB2-405C-8A0F-E48982012D36", "restore")

            cornerContent: Item {
                Column {
                    width: parent.width-40
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 15
                    spacing: 10

                    Button2 {
                        text: "Make Default"
                        width: parent.width
                        ToolTip.text: "Saves current formatting options as default for all current and future documents."
                        ToolTip.visible: hovered
                        onClicked: {
                            Scrite.document.formatting.saveAsUserDefaults()
                        }
                    }

                    Button2 {
                        text: "Factory Reset"
                        width: parent.width
                        ToolTip.text: "Restores formatting options to defaults for current document only."
                        ToolTip.visible: hovered
                        onClicked: {
                            Scrite.document.formatting.resetToFactoryDefaults()
                            Scrite.document.printFormat.resetToFactoryDefaults()
                        }
                    }
                }

            }

            pageContent: Column {
                id: scrollViewContent
                spacing: 0
                enabled: !Scrite.document.readOnly

                Item { width: parent.width; height: 10 }

                Item {
                    width: parent.width
                    height: Math.min(340, previewText.contentHeight + previewText.topPadding + previewText.bottomPadding)

                    ScrollView {
                        anchors.fill: parent
                        ScrollBar.vertical.opacity: ScrollBar.vertical.active ? 1 : 0.2
                        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                        Component.onCompleted: ScrollBar.vertical.position = (displayElementFormat.elementType === SceneElement.Shot || displayElementFormat.elementType === SceneElement.Transition) ? 0.2 : 0

                        TextArea {
                            id: previewText
                            font: Scrite.document.formatting.defaultFont
                            readOnly: true
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            background: Rectangle {
                                color: ScritePrimaryColors.c10.background
                            }

                            SceneDocumentBinder {
                                screenplayFormat: Scrite.document.formatting
                                applyFormattingEvenInTransaction: true
                                scene: Scene {
                                    elements: [
                                        SceneElement {
                                            type: SceneElement.Heading
                                            text: "INT. SOMEPLACE - DAY"
                                        },
                                        SceneElement {
                                            type: SceneElement.Action
                                            text: "Dr. Rajkumar enters the club house like a boss. He makes eye contact with everybody in the room."
                                        },
                                        SceneElement {
                                            type: SceneElement.Character
                                            text: "Dr. Rajkumar"
                                        },
                                        SceneElement {
                                            type: SceneElement.Parenthetical
                                            text: "(singing)"
                                        },
                                        SceneElement {
                                            type: SceneElement.Dialogue
                                            text: "If you come today, its too early. If you come tomorrow, its too late."
                                        },
                                        SceneElement {
                                            type: SceneElement.Shot
                                            text: "EXTREME CLOSEUP on Dr. Rajkumar's smiling face."
                                        },
                                        SceneElement {
                                            type: SceneElement.Transition
                                            text: "CUT TO"
                                        }
                                    ]
                                }
                                textDocument: previewText.textDocument
                                cursorPosition: -1
                                forceSyncDocument: true
                            }
                        }
                    }
                }

                Item { width: parent.width; height: 10 }

                // Default Language
                Row {
                    spacing: 10
                    width: parent.width
                    visible: pageData.elementType !== SceneElement.Heading

                    Text {
                        width: labelWidth
                        horizontalAlignment: Text.AlignRight
                        text: "Language"
                        font.pixelSize: 14
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    ComboBox2 {
                        property var enumModel: Scrite.app.enumerationModel(displayElementFormat, "DefaultLanguage")
                        model: enumModel
                        width: 300
                        textRole: "key"
                        currentIndex: displayElementFormat.defaultLanguageInt
                        onActivated: {
                            displayElementFormat.defaultLanguageInt = enumModel[currentIndex].value
                            switch(displayElementFormat.elementType) {
                            case SceneElement.Action:
                                ScriteRuntime.paragraphLanguageSettings.actionLanguage = enumModel[currentIndex].key
                                break;
                            case SceneElement.Character:
                                ScriteRuntime.paragraphLanguageSettings.characterLanguage = enumModel[currentIndex].key
                                break;
                            case SceneElement.Parenthetical:
                                ScriteRuntime.paragraphLanguageSettings.parentheticalLanguage = enumModel[currentIndex].key
                                break;
                            case SceneElement.Dialogue:
                                ScriteRuntime.paragraphLanguageSettings.dialogueLanguage = enumModel[currentIndex].key
                                break;
                            case SceneElement.Transition:
                                ScriteRuntime.paragraphLanguageSettings.transitionLanguage = enumModel[currentIndex].key
                                break;
                            case SceneElement.Shot:
                                ScriteRuntime.paragraphLanguageSettings.shotLanguage = enumModel[currentIndex].key
                                break;
                            }
                        }
                    }
                }

                // Font Size
                Row {
                    spacing: 10
                    width: parent.width

                    Text {
                        width: labelWidth
                        horizontalAlignment: Text.AlignRight
                        text: "Font Size"
                        font.pixelSize: 14
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    SpinBox {
                        width: parent.width-2*parent.spacing-labelWidth-parent.height
                        from: 6
                        to: 62
                        stepSize: 1
                        editable: true
                        value: displayElementFormat.font.pointSize
                        onValueModified: {
                            displayElementFormat.setFontPointSize(value)
                            printElementFormat.setFontPointSize(value)
                        }
                    }

                    ToolButton2 {
                        icon.source: "../icons/action/done_all.png"
                        anchors.verticalCenter: parent.verticalCenter
                        ToolTip.text: "Apply this font size to all '" + pageData.elementName + "' paragraphs."
                        ToolTip.delay: 1000
                        onClicked: {
                            displayElementFormat.applyToAll(SceneElementFormat.FontSize)
                            printElementFormat.applyToAll(SceneElementFormat.FontSize)
                        }
                    }
                }

                // Font Style
                Row {
                    spacing: 10
                    width: parent.width

                    Text {
                        width: labelWidth
                        horizontalAlignment: Text.AlignRight
                        text: "Font Style"
                        font.pixelSize: 14
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Row {
                        width: parent.width-2*parent.spacing-labelWidth-parent.height
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 5

                        CheckBox2 {
                            text: "Bold"
                            font.bold: true
                            checkable: true
                            checked: displayElementFormat.font.bold
                            onToggled: {
                                displayElementFormat.setFontBold(checked)
                                printElementFormat.setFontBold(checked)
                            }
                        }

                        CheckBox2 {
                            text: "Italics"
                            font.italic: true
                            checkable: true
                            checked: displayElementFormat.font.italic
                            onToggled: {
                                displayElementFormat.setFontItalics(checked)
                                printElementFormat.setFontItalics(checked)
                            }
                        }

                        CheckBox2 {
                            text: "Underline"
                            font.underline: true
                            checkable: true
                            checked: displayElementFormat.font.underline
                            onToggled: {
                                displayElementFormat.setFontUnderline(checked)
                                printElementFormat.setFontUnderline(checked)
                            }
                        }
                    }

                    ToolButton2 {
                        icon.source: "../icons/action/done_all.png"
                        anchors.verticalCenter: parent.verticalCenter
                        ToolTip.text: "Apply this font style to all '" + pageData.group + "' paragraphs."
                        ToolTip.delay: 1000
                        onClicked: {
                            displayElementFormat.applyToAll(SceneElementFormat.FontStyle)
                            printElementFormat.applyToAll(SceneElementFormat.FontStyle)
                        }
                    }
                }

                // Line Height
                Row {
                    spacing: 10
                    width: parent.width

                    Text {
                        width: labelWidth
                        horizontalAlignment: Text.AlignRight
                        text: "Line Height"
                        font.pixelSize: 14
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    SpinBox {
                        width: parent.width-2*parent.spacing-labelWidth-parent.height
                        from: 25
                        to: 300
                        stepSize: 5
                        editable: true
                        value: displayElementFormat.lineHeight * 100
                        onValueModified: {
                            displayElementFormat.lineHeight = value/100
                            printElementFormat.lineHeight = value/100
                        }
                        textFromValue: function(value,locale) {
                            return value + "%"
                        }
                    }

                    ToolButton2 {
                        icon.source: "../icons/action/done_all.png"
                        anchors.verticalCenter: parent.verticalCenter
                        ToolTip.text: "Apply this line height to all '" + pageData.group + "' paragraphs."
                        ToolTip.delay: 1000
                        onClicked: {
                            displayElementFormat.applyToAll(SceneElementFormat.LineHeight)
                            printElementFormat.applyToAll(SceneElementFormat.LineHeight)
                        }
                    }
                }

                // Colors
                Row {
                    spacing: 10
                    width: parent.width

                    Text {
                        width: labelWidth
                        horizontalAlignment: Text.AlignRight
                        text: "Text Color"
                        font.pixelSize: 14
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Row {
                        spacing: parent.spacing
                        width: parent.width - 2*parent.spacing - labelWidth - parent.height
                        anchors.verticalCenter: parent.verticalCenter

                        Rectangle {
                            border.width: 1
                            border.color: ScritePrimaryColors.borderColor
                            color: displayElementFormat.textColor
                            width: 30; height: 30
                            anchors.verticalCenter: parent.verticalCenter
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    displayElementFormat.textColor = Scrite.app.pickColor(displayElementFormat.textColor)
                                    printElementFormat.textColor = displayElementFormat.textColor
                                }
                            }
                        }

                        Text {
                            horizontalAlignment: Text.AlignRight
                            text: "Background Color"
                            font.pixelSize: 14
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Rectangle {
                            border.width: 1
                            border.color: ScritePrimaryColors.borderColor
                            color: displayElementFormat.backgroundColor
                            width: 30; height: 30
                            anchors.verticalCenter: parent.verticalCenter
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    displayElementFormat.backgroundColor = Scrite.app.pickColor(displayElementFormat.backgroundColor)
                                    printElementFormat.backgroundColor = displayElementFormat.backgroundColor
                                }
                            }
                        }
                    }

                    ToolButton2 {
                        icon.source: "../icons/action/done_all.png"
                        anchors.verticalCenter: parent.verticalCenter
                        ToolTip.text: "Apply these colors to all '" + pageData.group + "' paragraphs."
                        ToolTip.delay: 1000
                        onClicked: {
                            displayElementFormat.applyToAll(SceneElementFormat.TextAndBackgroundColors)
                            printElementFormat.applyToAll(SceneElementFormat.TextAndBackgroundColors)
                        }
                    }
                }

                // Text Alignment
                Row {
                    spacing: 10
                    width: parent.width

                    Text {
                        width: labelWidth
                        horizontalAlignment: Text.AlignRight
                        text: "Text Alignment"
                        font.pixelSize: 14
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Row {
                        width: parent.width - 2*parent.spacing - labelWidth - parent.height
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 5

                        RadioButton2 {
                            text: "Left"
                            checkable: true
                            checked: displayElementFormat.textAlignment === Qt.AlignLeft
                            onToggled: {
                                if(checked) {
                                    displayElementFormat.textAlignment = Qt.AlignLeft
                                    printElementFormat.textAlignment = Qt.AlignLeft
                                }
                            }
                        }

                        RadioButton2 {
                            text: "Center"
                            checkable: true
                            checked: displayElementFormat.textAlignment === Qt.AlignHCenter
                            onToggled: {
                                if(checked) {
                                    displayElementFormat.textAlignment = Qt.AlignHCenter
                                    printElementFormat.textAlignment = Qt.AlignHCenter
                                }
                            }
                        }

                        RadioButton2 {
                            text: "Right"
                            checkable: true
                            checked: displayElementFormat.textAlignment === Qt.AlignRight
                            onToggled: {
                                if(checked) {
                                    displayElementFormat.textAlignment = Qt.AlignRight
                                    printElementFormat.textAlignment = Qt.AlignRight
                                }
                            }
                        }

                        RadioButton2 {
                            text: "Justify"
                            checkable: true
                            checked: displayElementFormat.textAlignment === Qt.AlignJustify
                            onToggled: {
                                if(checked) {
                                    displayElementFormat.textAlignment = Qt.AlignJustify
                                    printElementFormat.textAlignment = Qt.AlignJustify
                                }
                            }
                        }
                    }

                    ToolButton2 {
                        icon.source: "../icons/action/done_all.png"
                        anchors.verticalCenter: parent.verticalCenter
                        ToolTip.text: "Apply this alignment to all '" + pageData.group + "' paragraphs."
                        ToolTip.delay: 1000
                        onClicked: {
                            displayElementFormat.applyToAll(SceneElementFormat.TextAlignment)
                            printElementFormat.applyToAll(SceneElementFormat.TextAlignment)
                        }
                    }
                }
            }
        }
    }

    Component {
        id: categoriesAndStructureComponent

        Item {
            PageView {
                id: categoriesAndStructurePages
                anchors.fill: parent
                enabled: ScriteAppFeatures.structure.enabled
                opacity: enabled ? 1 : 0.5
                pagesArray: [
                    { "title": "This Document" },
                    { "title": "Default Global" }
                ]
                pageTitleRole:  "title"
                currentIndex: 0
                cornerContent: Item {
                    Text {
                        width: parent.width-20
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 20
                        anchors.horizontalCenter: parent.horizontalCenter
                        font.pointSize: ScriteRuntime.idealFontMetrics.font.pointSize-2
                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.WordWrap
                        color: ScritePrimaryColors.c100.background
                        text: "Customize categories & groups you use for tagging index cards on the structure canvas. Each document has its own groups, there is a system wide copy as well."
                    }
                }
                pageContent: Loader {
                    height: categoriesAndStructurePages.height
                    sourceComponent: {
                        switch(categoriesAndStructurePages.currentIndex) {
                        case 0: return currentDocumentGroupsEditor
                        case 1: return defaultGlobalGroupsEditor
                        }
                        return unknownSettingsComponent
                    }
                }
            }

            DisabledFeatureNotice {
                visible: !ScriteAppFeatures.structure.enabled
                anchors.fill: parent
                featureName: "Structure Settings"
                color: Qt.rgba(1,1,1,0.9)
            }
        }
    }

    Component {
        id: currentDocumentGroupsEditor

        Item {
            Column {
                anchors.fill: parent
                anchors.margins: 10
                anchors.rightMargin: 20
                spacing: 20

                FlickableTextArea {
                    id: groupsDataEdit
                    width: parent.width
                    height: parent.height - parent.spacing - buttonsRow.height
                    clip: true
                    font.family: "Courier Prime"
                    font.pointSize: ScriteRuntime.idealFontMetrics.font.pointSize
                    text: Scrite.document.structure.groupsData
                    color: ScritePrimaryColors.c50.text
                    background: Rectangle {
                        color: ScritePrimaryColors.c50.background
                        border.width: 1
                        border.color: ScritePrimaryColors.borderColor
                    }
                }

                Row {
                    id: buttonsRow
                    anchors.right: parent.right
                    spacing: 20

                    Button2 {
                        text: "Cancel"
                        onClicked: modalDialog.close()
                    }

                    Button2 {
                        text: "Apply"
                        onClicked: {
                            Scrite.document.structure.groupsData = groupsDataEdit.text
                            modalDialog.close()
                        }
                    }
                }
            }
        }
    }

    Component {
        id: defaultGlobalGroupsEditor

        Item {
            Column {
                anchors.fill: parent
                anchors.margins: 10
                anchors.rightMargin: 20
                spacing: 20

                FlickableTextArea {
                    id: groupsDataEdit
                    width: parent.width
                    height: parent.height - parent.spacing - buttonsRow.height
                    clip: true
                    font.family: "Courier Prime"
                    font.pointSize: ScriteRuntime.idealFontMetrics.font.pointSize
                    text: Scrite.app.fileContents(Scrite.document.structure.defaultGroupsDataFile)
                    color: ScriteAccentColors.c50.text
                    background: Rectangle {
                        color: ScriteAccentColors.c50.background
                        border.width: 1
                        border.color: ScriteAccentColors.borderColor
                    }
                }

                Row {
                    id: buttonsRow
                    anchors.right: parent.right
                    spacing: 20

                    Button2 {
                        text: "Cancel"
                        onClicked: modalDialog.close()
                    }

                    Button2 {
                        text: "Apply"
                        onClicked: {
                            Scrite.app.writeToFile(Scrite.document.structure.defaultGroupsDataFile, groupsDataEdit.text)
                            modalDialog.close()
                        }
                    }
                }
            }
        }
    }

    Component {
        id: unknownSettingsComponent

        Item {
            Text {
                anchors.centerIn: parent
                text: "This is embarrassing. We honestly dont know what to show here!"
            }
        }
    }
}
