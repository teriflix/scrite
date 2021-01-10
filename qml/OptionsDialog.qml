/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQuick 2.13
import QtQuick.Dialogs 1.3
import Qt.labs.settings 1.0
import QtQuick.Controls 2.13
import QtQuick.Controls.Material 2.12
import Scrite 1.0

Item {
    width: 1050
    height: Math.min(documentUI.height*0.9, 750)
    readonly property color dialogColor: primaryColors.windowColor
    readonly property var systemFontInfo: app.systemFontInfo()

    Component.onCompleted: {
        tabView.currentIndex = modalDialog.arguments && modalDialog.arguments.activeTabIndex ? modalDialog.arguments.activeTabIndex : 0
        modalDialog.arguments = undefined
        // modalDialog.closeable = false
    }

    TabView2 {
        id: tabView
        anchors.fill: parent
        currentIndex: 0

        tabsArray: [
            { "title": "Application", "tooltip": "Settings in this page apply to all documents." },
            { "title": "Page Setup", "tooltip": "Settings in this page apply to all documents." },
            { "title": "Title Page", "tooltip": "Settings in this page applies only to the current document." },
            { "title": "Formatting Rules", "tooltip": "Settings in this page applies only to the current document." }
        ]

        content: {
            switch(currentIndex) {
            case 0: return applicationSettingsComponent
            case 1: return pageSetupComponent
            case 2: return titlePageSettingsComponent
            case 3: return formattingRulesSettingsComponent
            }
            return unknownSettingsComponent
        }
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
        }
    }

    Component {
        id: coreSettingsComponent

        Column {
            spacing: 20

            Item {
                height: 30
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
                        model: app.transliterationEngine.getLanguages()
                        delegate: CheckBox2 {
                            width: activeLanguagesView.width/activeLanguagesView.columns
                            checkable: true
                            checked: modelData.active
                            text: modelData.key
                            onToggled: app.transliterationEngine.markLanguage(modelData.value,checked)
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
                    label: Text { text: "Auto Save" }

                    Column {
                        width: parent.width
                        spacing: 10

                        CheckBox2 {
                            text: "Enable AutoSave"
                            checked: scriteDocument.autoSave
                            onToggled: scriteDocument.autoSave = checked
                        }

                        Text {
                            width: parent.width
                            text: "Auto Save Interval (in seconds)"
                        }

                        TextField2 {
                            width: parent.width
                            enabled: scriteDocument.autoSave
                            text: scriteDocument.autoSaveDurationInSeconds
                            validator: IntValidator {
                                bottom: 1; top: 3600
                            }
                            onTextEdited: scriteDocument.autoSaveDurationInSeconds = parseInt(text)
                        }
                    }
                }

                GroupBox {
                    width: (parent.width - parent.spacing)/2
                    label: Text {
                        text: "Screenplay Editor"
                    }

                    CheckBox2 {
                        checked: screenplayEditorSettings.enableSpellCheck
                        text: "Enable spell check"
                        onToggled: screenplayEditorSettings.enableSpellCheck = checked
                    }
                }
            }
        }
    }

    Component {
        id: fontSettingsComponent

        Column {
            id: fontSettingsUi
            spacing: 10

            readonly property var languagePreviewString: [
                "Greetings",
                "বাংলা",
                "ગુજરાતી",
                "हिन्दी",
                "ಕನ್ನಡ",
                "മലയാളം",
                "मराठी",
                "ଓଡିଆ",
                "ਪੰਜਾਬੀ",
                "संस्कृत",
                "தமிழ்",
                "తెలుగు"
            ]

            Item { width: parent.width; height: 20 }

            Repeater {
                model: app.enumerationModelForType("TransliterationEngine", "Language")

                Row {
                    spacing: 10
                    width: parent.width - 20
                    anchors.horizontalCenter: parent.horizontalCenter

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        font.pointSize: app.idealFontPointSize
                        text: modelData.key
                        width: 175
                        horizontalAlignment: Text.AlignRight
                        font.bold: fontCombo.down
                    }

                    ComboBox2 {
                        id: fontCombo
                        property var fontFamilies: app.transliterationEngine.availableLanguageFontFamilies(modelData.value)
                        model: fontFamilies.families
                        width: 400
                        currentIndex: fontFamilies.preferredFamilyIndex
                        onActivated: {
                            var family = fontFamilies.families[index]
                            app.transliterationEngine.setPreferredFontFamilyForLanguage(modelData.value, family)
                            previewText.font.family = family
                        }
                    }

                    Text {
                        id: previewText
                        anchors.verticalCenter: parent.verticalCenter
                        font.family: app.transliterationEngine.preferredFontFamilyForLanguage(modelData.value)
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
                font.pointSize: app.idealFontPointSize
                wrapMode: Text.WordWrap
                width: parent.width - 40
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Override the built-in transliterator to use any of your operating system's input methods."
            }

            Repeater {
                model: app.enumerationModelForType("TransliterationEngine", "Language")

                Row {
                    spacing: 10
                    width: parent.width - 20
                    anchors.horizontalCenter: parent.horizontalCenter

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        font.pointSize: app.idealFontPointSize
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
                            app.transliterationEngine.setTextInputSourceIdForLanguage(modelData.value, item.id)
                        }

                        Component.onCompleted: {
                            var tisSources = app.textInputManager.sourcesForLanguageJson(modelData.value)
                            var tisSourceId = app.transliterationEngine.textInputSourceIdForLanguage(modelData.value)
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
                            checked: structureCanvasSettings.showGrid
                            text: "Show Grid in Structure Tab"
                            onToggled: structureCanvasSettings.showGrid = checked
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
                                border.color: primaryColors.borderColor
                                width: 30; height: 30
                                color: structureCanvasSettings.canvasColor
                                anchors.verticalCenter: parent.verticalCenter

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: structureCanvasSettings.canvasColor = app.pickColor(structureCanvasSettings.canvasColor)
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
                                border.color: primaryColors.borderColor
                                width: 30; height: 30
                                color: structureCanvasSettings.gridColor
                                anchors.verticalCenter: parent.verticalCenter

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: structureCanvasSettings.gridColor = app.pickColor(structureCanvasSettings.gridColor)
                                }
                            }
                        }

                        Row {
                            spacing: 10
                            width: parent.width
                            visible: app.isWindowsPlatform || app.isLinuxPlatform

                            Text {
                                id: wzfText
                                text: "Zoom Speed"
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Slider {
                                from: 1
                                to: 20
                                orientation: Qt.Horizontal
                                snapMode: Slider.SnapAlways
                                value: scrollAreaSettings.zoomFactor * 100
                                anchors.verticalCenter: parent.verticalCenter
                                width: parent.width-wzfText.width-parent.spacing
                                onMoved: scrollAreaSettings.zoomFactor = value / 100
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
                            checked: scriteDocument.structure.canvasUIMode === Structure.IndexCardUI
                            onToggled: {
                                var toggleCanvasUI = function() {
                                    if(scriteDocument.structure.canvasUIMode === Structure.IndexCardUI)
                                        scriteDocument.structure.canvasUIMode = Structure.SynopsisEditorUI
                                    else
                                        scriteDocument.structure.canvasUIMode = Structure.IndexCardUI
                                }

                                if(mainTabBar.currentIndex === 0) {
                                    toggleCanvasUI()
                                } else {
                                    contentLoader.active = false
                                    app.execLater(contentLoader, 100, function() {
                                        toggleCanvasUI()
                                        contentLoader.active = true
                                    })
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
                    width: (parent.width - parent.spacing)/2
                    label: Text {
                        text: "Animations"
                    }

                    CheckBox2 {
                        checked: screenplayEditorSettings.enableAnimations
                        text: "Enable Animations"
                        onToggled: screenplayEditorSettings.enableAnimations = checked
                    }
                }

                GroupBox {
                    label: Text { text: "Page Layout (Display)" }
                    width: (parent.width - parent.spacing)/2

                    Column {
                        width: parent.width
                        spacing: 10

                        Text {
                            width: parent.width
                            wrapMode: Text.WordWrap
                            text: "Default Resolution: <strong>" + scriteDocument.displayFormat.pageLayout.defaultResolution + "</strong>"
                        }

                        TextField2 {
                            width: parent.width
                            placeholderText: "leave empty for default, or enter a custom value."
                            text: scriteDocument.displayFormat.pageLayout.customResolution > 0 ? scriteDocument.displayFormat.pageLayout.customResolution : ""
                            onEditingComplete: {
                                var value = parseFloat(text)
                                if(isNaN(value))
                                    scriteDocument.displayFormat.pageLayout.customResolution = 0
                                else
                                    scriteDocument.displayFormat.pageLayout.customResolution = value
                            }
                        }
                    }
                }
            }

            GroupBox {
                title: "Window Tabs"
                width: parent.width - 60
                anchors.horizontalCenter: parent.horizontalCenter

                Column {
                    width: parent.width
                    spacing: 5

                    Text {
                        width: parent.width
                        font.pointSize: app.idealFontPointSize
                        text: "By default Scrite shows Screenplay, Structure and Notebook in separate tabs on the main window. If you have a small display, you can move Notebook into a separate tab. Otherwise its productive to see all aspects of your screenplay within the Structure tab itself."
                        wrapMode: Text.WordWrap
                    }

                    CheckBox2 {
                        checked: workspaceSettings.showNotebookInStructure
                        text: "Move Notebook into the Structure tab"
                        onToggled: workspaceSettings.showNotebookInStructure = checked
                    }
                }
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
                        font.pointSize: app.idealFontPointSize
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
                                font.pointSize: app.idealFontPointSize
                                text: "Max Time In Milliseconds"
                                width: parent.width
                            }

                            Text {
                                font.bold: false
                                font.pointSize: app.idealFontPointSize-2
                                text: "Default: 1000"
                            }

                            TextField {
                                id: txtMaxTime
                                text: notebookSettings.graphLayoutMaxTime
                                width: parent.width
                                placeholderText: "if left empty, default of 1000 will be used"
                                validator: IntValidator {
                                    bottom: 250
                                    top: 5000
                                }
                                onTextEdited: {
                                    if(length === 0 || text.trim() === "")
                                        notebookSettings.graphLayoutMaxTime = 1000
                                    else
                                        notebookSettings.graphLayoutMaxTime = parseInt(text)
                                }
                                KeyNavigation.tab: txtMaxIterations
                            }
                        }

                        Column {
                            width: (parent.width - parent.spacing)/2

                            Text {
                                font.bold: true
                                font.pointSize: app.idealFontPointSize
                                text: "Max Iterations"
                                width: parent.width
                            }

                            Text {
                                font.bold: false
                                font.pointSize: app.idealFontPointSize-2
                                text: "Default: 50000"
                            }

                            TextField {
                                id: txtMaxIterations
                                text: notebookSettings.graphLayoutMaxIterations
                                width: parent.width
                                placeholderText: "if left empty, default of 50000 will be used"
                                validator: IntValidator {
                                    bottom: 1000
                                    top: 250000
                                }
                                onTextEdited: {
                                    if(length === 0 || text.trim() === "")
                                        notebookSettings.graphLayoutMaxIterations = 50000
                                    else
                                        notebookSettings.graphLayoutMaxIterations = parseInt(text)
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
            property var fieldsModel: app.enumerationModelForType("HeaderFooter", "Field")

            Settings {
                id: pageSetupSettings
                fileName: app.settingsFilePath
                category: "PageSetup"
                property var paperSize: ScreenplayPageLayout.Letter
                property var headerLeft: HeaderFooter.Title
                property var headerCenter: HeaderFooter.Subtitle
                property var headerRight: HeaderFooter.PageNumber
                property real headerOpacity: 0.5
                property var footerLeft: HeaderFooter.Author
                property var footerCenter: HeaderFooter.Version
                property var footerRight: HeaderFooter.Contact
                property real footerOpacity: 0.5
                property bool watermarkEnabled: false
                property string watermarkText: "Scrite"
                property string watermarkFont: "Courier Prime"
                property int watermarkFontSize: 120
                property color watermarkColor: "lightgray"
                property real watermarkOpacity: 0.5
                property real watermarkRotation: -45
                property int watermarkAlignment: Qt.AlignCenter
            }

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
                                scriteDocument.formatting.pageLayout.paperSize = currentIndex
                                scriteDocument.printFormat.pageLayout.paperSize = currentIndex
                            }
                            model: app.enumerationModelForType("ScreenplayPageLayout", "PaperSize")
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
                            text: scriteDocument.printFormat.secondsPerPage
                            validator: IntValidator { bottom: 15; top: 300 }
                            onTextEdited: scriteDocument.printFormat.secondsPerPage = parseInt(text)
                            width: sceneEditorFontMetrics.averageCharacterWidth*3
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
                    label: Text { text: "Watermark" }

                    Row {
                        spacing: 30
                        anchors.horizontalCenter: parent.horizontalCenter

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
                                checked: pageSetupSettings.watermarkEnabled
                                onToggled: pageSetupSettings.watermarkEnabled = checked
                            }

                            Text {
                                text: "Text"
                                width: labelWidth
                                horizontalAlignment: Text.AlignRight
                            }

                            TextField2 {
                                width: 300
                                text: pageSetupSettings.watermarkText
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
                                border.color: primaryColors.borderColor
                                color: pageSetupSettings.watermarkColor
                                width: 30; height: 30
                                enabled: pageSetupSettings.watermarkEnabled
                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: pageSetupSettings.watermarkColor = app.pickColor(pageSetupSettings.watermarkColor)
                                }
                            }
                        }

                        Rectangle {
                            width: 1
                            height: parent.height
                            color: primaryColors.borderColor
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
                }

                Button2 {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Restore Defaults"
                    onClicked: {
                        pageSetupSettings.headerLeft = HeaderFooter.Title
                        pageSetupSettings.headerCenter = HeaderFooter.Subtitle
                        pageSetupSettings.headerRight = HeaderFooter.PageNumber
                        pageSetupSettings.footerLeft = HeaderFooter.Author
                        pageSetupSettings.footerCenter = HeaderFooter.Version
                        pageSetupSettings.footerRight = HeaderFooter.Contact
                        pageSetupSettings.watermarkEnabled = false
                        pageSetupSettings.watermarkText = "Scrite"
                        pageSetupSettings.watermarkFont = "Courier Prime"
                        pageSetupSettings.watermarkFontSize = 120
                        pageSetupSettings.watermarkColor = "lightgray"
                        pageSetupSettings.watermarkOpacity = 0.5
                        pageSetupSettings.watermarkRotation = -45
                        pageSetupSettings.watermarkAlignment = Qt.AlignCenter
                    }
                }
            }
        }
    }

    Component {
        id: titlePageSettingsComponent

        Item {
            readonly property real labelWidth: 60

            Column {
                width: parent.width - 80
                anchors.centerIn: parent
                spacing: 20

                // Cover page photo field
                Rectangle {
                    id: coverPageEdit
                    /*
                  At best we can paint a 464x261 point photo on the cover page. Nothing more.
                  So, we need to provide a image preview in this aspect ratio.
                  */
                    width: 400; height: 225
                    border.width: scriteDocument.screenplay.coverPagePhoto === "" ? 1 : 0
                    border.color: "black"
                    anchors.horizontalCenter: parent.horizontalCenter

                    Loader {
                        anchors.fill: parent
                        active: scriteDocument.screenplay.coverPagePhoto !== "" && (coverPagePhoto.paintedWidth < parent.width || coverPagePhoto.paintedHeight < parent.height)
                        opacity: 0.1
                        sourceComponent: Item {
                            Image {
                                id: coverPageImage
                                anchors.fill: parent
                                fillMode: Image.PreserveAspectCrop
                                source: "file://" + scriteDocument.screenplay.coverPagePhoto
                                asynchronous: true
                            }
                        }
                    }

                    Image {
                        id: coverPagePhoto
                        anchors.fill: parent
                        anchors.margins: 1
                        smooth: true; mipmap: true
                        fillMode: Image.PreserveAspectFit
                        source: scriteDocument.screenplay.coverPagePhoto !== "" ? "file:///" + scriteDocument.screenplay.coverPagePhoto : ""
                        opacity: coverPagePhotoMouseArea.containsMouse ? 0.25 : 1

                        BusyIndicator {
                            anchors.centerIn: parent
                            running: parent.status === Image.Loading
                        }
                    }

                    Text {
                        anchors.fill: parent
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        opacity: coverPagePhotoMouseArea.containsMouse ? 1 : (scriteDocument.screenplay.coverPagePhoto === "" ? 0.5 : 0)
                        text: scriteDocument.screenplay.coverPagePhoto === "" ? "Click here to set the cover page photo" : "Click here to change the cover page photo"
                    }

                    MouseArea {
                        id: coverPagePhotoMouseArea
                        anchors.fill: parent
                        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                        hoverEnabled: true
                        enabled: !scriteDocument.readOnly
                        onClicked: fileDialog.open()
                    }

                    Column {
                        spacing: 0
                        anchors.left: parent.right
                        anchors.leftMargin: 20
                        visible: scriteDocument.screenplay.coverPagePhoto !== ""
                        enabled: visible && !scriteDocument.readOnly

                        Text {
                            text: "Cover Photo Size"
                            font.bold: true
                            topPadding: 5
                            bottomPadding: 5
                            color: primaryColors.c300.text
                            opacity: enabled ? 1 : 0.5
                        }

                        RadioButton2 {
                            text: "Small"
                            checked: scriteDocument.screenplay.coverPagePhotoSize === Screenplay.SmallCoverPhoto
                            onToggled: scriteDocument.screenplay.coverPagePhotoSize = Screenplay.SmallCoverPhoto
                        }

                        RadioButton2 {
                            text: "Medium"
                            checked: scriteDocument.screenplay.coverPagePhotoSize === Screenplay.MediumCoverPhoto
                            onToggled: scriteDocument.screenplay.coverPagePhotoSize = Screenplay.MediumCoverPhoto
                        }

                        RadioButton2 {
                            text: "Large"
                            checked: scriteDocument.screenplay.coverPagePhotoSize === Screenplay.LargeCoverPhoto
                            onToggled: scriteDocument.screenplay.coverPagePhotoSize = Screenplay.LargeCoverPhoto
                        }

                        Button2 {
                            text: "Remove"
                            onClicked: scriteDocument.screenplay.clearCoverPagePhoto()
                        }
                    }

                    FileDialog {
                        id: fileDialog
                        nameFilters: ["Photos (*.jpg *.png *.bmp *.jpeg)"]
                        selectFolder: false
                        selectMultiple: false
                        sidebarVisible: true
                        selectExisting: true
                        onAccepted: {
                            if(fileUrl != "")
                                scriteDocument.screenplay.setCoverPagePhoto(app.urlToLocalFile(fileUrl))
                        }
                        folder: workspaceSettings.lastOpenPhotosFolderUrl
                        onFolderChanged: workspaceSettings.lastOpenPhotosFolderUrl = folder
                    }
                }

                Row {
                    id: titlePageFields
                    width: parent.width
                    spacing: 20
                    enabled: !scriteDocument.readOnly

                    Column {
                        width: (parent.width - parent.spacing)/2

                        // Title field
                        Row {
                            spacing: 10
                            width: parent.width

                            Text {
                                width: labelWidth
                                horizontalAlignment: Text.AlignRight
                                text: "Title"
                                font.pixelSize: 14
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            TextField2 {
                                id: titleField
                                width: parent.width-parent.spacing-labelWidth
                                text: scriteDocument.screenplay.title
                                selectByMouse: true
                                onTextEdited: scriteDocument.screenplay.title = text
                                font.pixelSize: 20
                                maximumLength: 100
                                placeholderText: "(max 100 letters)"
                                tabItem: subtitleField
                                backTabItem: websiteField
                                enableTransliteration: true
                            }
                        }

                        // Subtitle field
                        Row {
                            spacing: 10
                            width: parent.width

                            Text {
                                width: labelWidth
                                horizontalAlignment: Text.AlignRight
                                text: "Subtitle"
                                font.pixelSize: 14
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            TextField2 {
                                id: subtitleField
                                width: parent.width-parent.spacing-labelWidth
                                text: scriteDocument.screenplay.subtitle
                                selectByMouse: true
                                onTextEdited: scriteDocument.screenplay.subtitle = text
                                font.pixelSize: 20
                                maximumLength: 100
                                placeholderText: "(max 100 letters)"
                                tabItem: basedOnField
                                backTabItem: titleField
                                enableTransliteration: true
                            }
                        }

                        // Based on field
                        Row {
                            spacing: 10
                            width: parent.width

                            Text {
                                width: labelWidth
                                horizontalAlignment: Text.AlignRight
                                text: "Based on"
                                font.pixelSize: 14
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            TextField2 {
                                id: basedOnField
                                width: parent.width-parent.spacing-labelWidth
                                text: scriteDocument.screenplay.basedOn
                                selectByMouse: true
                                onTextEdited: scriteDocument.screenplay.basedOn = text
                                font.pixelSize: 20
                                maximumLength: 100
                                placeholderText: "(max 100 letters)"
                                tabItem: versionField
                                backTabItem: subtitleField
                                enableTransliteration: true
                            }
                        }

                        // Version field
                        Row {
                            spacing: 10
                            width: parent.width

                            Text {
                                width: labelWidth
                                horizontalAlignment: Text.AlignRight
                                text: "Version"
                                font.pixelSize: 14
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            TextField2 {
                                id: versionField
                                width: parent.width-parent.spacing-labelWidth
                                text: scriteDocument.screenplay.version
                                selectByMouse: true
                                onTextEdited: scriteDocument.screenplay.version = text
                                font.pixelSize: 20
                                maximumLength: 20
                                placeholderText: "(max 20 letters)"
                                tabItem: authorField
                                backTabItem:  basedOnField
                                enableTransliteration: true
                            }
                        }

                        // Author field
                        Row {
                            spacing: 10
                            width: parent.width

                            Text {
                                width: labelWidth
                                horizontalAlignment: Text.AlignRight
                                text: "Written By"
                                font.pixelSize: 14
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            TextField2 {
                                id: authorField
                                width: parent.width-parent.spacing-labelWidth
                                text: scriteDocument.screenplay.author
                                selectByMouse: true
                                onTextEdited: scriteDocument.screenplay.author = text
                                font.pixelSize: 20
                                maximumLength: 100
                                placeholderText: "(max 100 letters)"
                                tabItem: contactField
                                backTabItem: versionField
                                enableTransliteration: true
                            }
                        }
                    }

                    Column {
                        width: (parent.width - parent.spacing)/2

                        // Contact field
                        Row {
                            spacing: 10
                            width: parent.width

                            Text {
                                width: labelWidth
                                horizontalAlignment: Text.AlignRight
                                text: "Contact"
                                font.pixelSize: 14
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            TextField2 {
                                id: contactField
                                width: parent.width-parent.spacing-labelWidth
                                text: scriteDocument.screenplay.contact
                                selectByMouse: true
                                onTextEdited: scriteDocument.screenplay.contact = text
                                font.pixelSize: 20
                                placeholderText: "(Optional) Company / Studio name (max 100 letters)"
                                maximumLength: 100
                                tabItem: addressField
                                backTabItem: authorField
                                enableTransliteration: true
                            }
                        }

                        // Address field
                        Row {
                            spacing: 10
                            width: parent.width

                            Text {
                                width: labelWidth
                                horizontalAlignment: Text.AlignRight
                                text: "Address"
                                font.pixelSize: 14
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            TextField2 {
                                id: addressField
                                width: parent.width-parent.spacing-labelWidth
                                text: scriteDocument.screenplay.address
                                selectByMouse: true
                                onTextEdited: scriteDocument.screenplay.address = text
                                font.pixelSize: 20
                                maximumLength: 100
                                placeholderText: "(Optional) Address (max 100 letters)"
                                tabItem: emailField
                                backTabItem: contactField
                            }
                        }

                        // Email field
                        Row {
                            spacing: 10
                            width: parent.width

                            Text {
                                width: labelWidth
                                horizontalAlignment: Text.AlignRight
                                text: "Email"
                                font.pixelSize: 14
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            TextField2 {
                                id: emailField
                                width: parent.width-parent.spacing-labelWidth
                                text: scriteDocument.screenplay.email
                                selectByMouse: true
                                onTextEdited: scriteDocument.screenplay.email = text
                                font.pixelSize: 20
                                maximumLength: 100
                                placeholderText: "(Optional) Email (max 100 letters)"
                                tabItem: phoneField
                                backTabItem: addressField
                            }
                        }

                        // Phone field
                        Row {
                            spacing: 10
                            width: parent.width

                            Text {
                                width: labelWidth
                                horizontalAlignment: Text.AlignRight
                                text: "Phone"
                                font.pixelSize: 14
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            TextField2 {
                                id: phoneField
                                width: parent.width-parent.spacing-labelWidth
                                text: scriteDocument.screenplay.phoneNumber
                                selectByMouse: true
                                onTextEdited: scriteDocument.screenplay.phoneNumber = text
                                font.pixelSize: 20
                                maximumLength: 20
                                placeholderText: "(Optional) Phone number (max 20 digits/letters)"
                                tabItem: websiteField
                                backTabItem: emailField
                            }
                        }

                        // Website field
                        Row {
                            spacing: 10
                            width: parent.width

                            Text {
                                width: labelWidth
                                horizontalAlignment: Text.AlignRight
                                text: "Website"
                                font.pixelSize: 14
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            TextField2 {
                                id: websiteField
                                width: parent.width-parent.spacing-labelWidth
                                text: scriteDocument.screenplay.website
                                selectByMouse: true
                                onTextEdited: scriteDocument.screenplay.website = text
                                font.pixelSize: 20
                                maximumLength: 150
                                placeholderText: "(Optional) Website (max 150 letters)"
                                tabItem: titleField
                                backTabItem: phoneField
                            }
                        }
                    }
                }

                Row {
                    spacing: 20
                    anchors.horizontalCenter: parent.horizontalCenter

                    CheckBox2 {
                        text: "Include Title Page In Preview"
                        checked: screenplayEditorSettings.includeTitlePageInPreview
                        onToggled: screenplayEditorSettings.includeTitlePageInPreview = checked
                    }

                    CheckBox2 {
                        text: "Center Align Title Page"
                        checked: scriteDocument.screenplay.titlePageIsCentered
                        onToggled: scriteDocument.screenplay.titlePageIsCentered = checked
                    }
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
            property SceneElementFormat displayElementFormat: scriteDocument.formatting.elementFormat(pageData.elementType)
            property SceneElementFormat printElementFormat: scriteDocument.printFormat.elementFormat(pageData.elementType)

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

            cornerContent: Item {
                Button2 {
                    text: "Reset"
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 10
                    onClicked: {
                        scriteDocument.formatting.resetToDefaults()
                        scriteDocument.printFormat.resetToDefaults()
                    }
                }
            }

            pageContent: Column {
                id: scrollViewContent
                spacing: 0
                enabled: !scriteDocument.readOnly

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
                            font: scriteDocument.formatting.defaultFont
                            readOnly: true
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            background: Rectangle {
                                color: primaryColors.c10.background
                            }

                            SceneDocumentBinder {
                                screenplayFormat: scriteDocument.formatting
                                scene: Scene {
                                    elements: [
                                        SceneElement {
                                            type: SceneElement.Heading
                                            text: "INT. SOMEPLACE - DAY"
                                        },
                                        SceneElement {
                                            type: SceneElement.Action
                                            text: "Dr. Rajkumar enters the club house like a boss. He looks around at everybody in their eyes."
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
                        property var enumModel: app.enumerationModel(displayElementFormat, "DefaultLanguage")
                        model: enumModel
                        width: 300
                        textRole: "key"
                        currentIndex: displayElementFormat.defaultLanguageInt
                        onActivated: {
                            displayElementFormat.defaultLanguageInt = enumModel[currentIndex].value
                            switch(displayElementFormat.elementType) {
                            case SceneElement.Action:
                                paragraphLanguageSettings.actionLanguage = enumModel[currentIndex].key
                                break;
                            case SceneElement.Character:
                                paragraphLanguageSettings.characterLanguage = enumModel[currentIndex].key
                                break;
                            case SceneElement.Parenthetical:
                                paragraphLanguageSettings.parentheticalLanguage = enumModel[currentIndex].key
                                break;
                            case SceneElement.Dialogue:
                                paragraphLanguageSettings.dialogueLanguage = enumModel[currentIndex].key
                                break;
                            case SceneElement.Transition:
                                paragraphLanguageSettings.transitionLanguage = enumModel[currentIndex].key
                                break;
                            case SceneElement.Shot:
                                paragraphLanguageSettings.shotLanguage = enumModel[currentIndex].key
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
                            border.color: primaryColors.borderColor
                            color: displayElementFormat.textColor
                            width: 30; height: 30
                            anchors.verticalCenter: parent.verticalCenter
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    displayElementFormat.textColor = app.pickColor(displayElementFormat.textColor)
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
                            border.color: primaryColors.borderColor
                            color: displayElementFormat.backgroundColor
                            width: 30; height: 30
                            anchors.verticalCenter: parent.verticalCenter
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    displayElementFormat.backgroundColor = app.pickColor(displayElementFormat.backgroundColor)
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
                            onCheckedChanged: {
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
                            onCheckedChanged: {
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
                            onCheckedChanged: {
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
                            onCheckedChanged: {
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
        id: unknownSettingsComponent

        Item {
            Text {
                anchors.centerIn: parent
                text: "This is embarrassing. We honestly dont know what to show here!"
            }
        }
    }
}
