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
import Qt.labs.settings 1.0
import QtQuick.Controls 2.13
import QtQuick.Controls.Material 2.12
import Scrite 1.0

Item {
    width: 1050
    height: 680

    Item {
        anchors.fill: parent
        anchors.margins: 20

        Rectangle {
            anchors.fill: pageList
            anchors.margins: -4
            radius: 5
            border { width: 1; color: primaryColors.borderColor }
        }

        ListModel {
            id: pageModel
            ListElement { name: "Settings"; group: "Application"; }
            ListElement { name: "Title Page"; group: "Screenplay" }
            ListElement { name: "Page Setup"; group: "Screenplay" }

            ListElement { name: "Heading"; group: "Formatting"; elementType: SceneElement.Heading }
            ListElement { name: "Action"; group: "Formatting"; elementType: SceneElement.Action }
            ListElement { name: "Character"; group: "Formatting"; elementType: SceneElement.Character }
            ListElement { name: "Dialogue"; group: "Formatting"; elementType: SceneElement.Dialogue }
            ListElement { name: "Parenthetical"; group: "Formatting"; elementType: SceneElement.Parenthetical }
            ListElement { name: "Shot"; group: "Formatting"; elementType: SceneElement.Shot }
            ListElement { name: "Transition"; group: "Formatting"; elementType: SceneElement.Transition }
        }

        ListView {
            id: pageList
            height: parent.height
            width: 170
            model: pageModel
            spacing: 5
            clip: true
            highlightMoveDuration: 0
            section.property: "group"
            section.criteria: ViewSection.FullString
            section.delegate: Rectangle {
                width: pageList.scrollBarRequired ? pageList.width - 17 : pageList.width
                height: 30
                color: accentColors.c600.background

                Text {
                    anchors.centerIn: parent
                    font.pixelSize: 14
                    font.letterSpacing: 2
                    text: section
                    color: accentColors.c600.text
                }
            }

            property bool scrollBarRequired: pageList.height < pageList.contentHeight
            ScrollBar.vertical: ScrollBar {
                policy: pageList.scrollBarRequired ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
                minimumSize: 0.1
                palette {
                    mid: Qt.rgba(0,0,0,0.25)
                    dark: Qt.rgba(0,0,0,0.75)
                }
                opacity: active ? 1 : 0.2
                Behavior on opacity { NumberAnimation { duration: 250 } }
            }
            highlight: Rectangle {
                width: pageList.scrollBarRequired ? pageList.width - 17 : pageList.width
                color: primaryColors.highlight.background
                radius: 5
            }
            delegate: Text {
                width: pageList.scrollBarRequired ? pageList.width - 17 : pageList.width
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                height: 32
                font.pixelSize: 18
                font.bold: pageList.currentIndex === index
                text: name
                color: pageList.currentIndex === index ? primaryColors.highlight.text : primaryColors.c10.text
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        pageList.currentIndex = index
                        pageLoader.loadPage()
                    }
                }
            }
        }

        Loader {
            id: pageLoader
            anchors.left: pageList.right
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.leftMargin: 30
            active: true
            sourceComponent: applicationSettingsComponent

            function loadPage() {
                pageLoader.active = false
                if(pageList.currentIndex >= 3 && pageList.currentIndex <= 9)
                    pageLoader.sourceComponent = elementFormatOptionsComponent
                else {
                    switch(pageList.currentIndex) {
                    case 0:
                        pageLoader.sourceComponent = applicationSettingsComponent
                        break
                    case 1:
                        pageLoader.sourceComponent = screenplayOptionsComponent
                        break
                    case 2:
                        pageLoader.sourceComponent = pageSetupComponent
                        break
                    }
                }
                pageLoader.active = true
            }
        }
    }

    Component {
        id: screenplayOptionsComponent

        Item {
            property real labelWidth: 60

            Column {
                width: parent.width
                spacing: 20

                Text {
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    text: "Title Page Settings"
                    font.pixelSize: 24
                }

                Item { width: parent.width; height: 1 }

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

                    TextField {
                        width: parent.width-parent.spacing-labelWidth
                        text: scriteDocument.screenplay.title
                        onTextEdited: scriteDocument.screenplay.title = text
                        font.pixelSize: 20
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

                    TextField {
                        width: parent.width-parent.spacing-labelWidth
                        text: scriteDocument.screenplay.subtitle
                        onTextEdited: scriteDocument.screenplay.subtitle = text
                        font.pixelSize: 20
                    }
                }

                // Author field
                Row {
                    spacing: 10
                    width: parent.width

                    Text {
                        width: labelWidth
                        horizontalAlignment: Text.AlignRight
                        text: "Author"
                        font.pixelSize: 14
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    TextField {
                        width: parent.width-parent.spacing-labelWidth
                        text: scriteDocument.screenplay.author
                        onTextEdited: scriteDocument.screenplay.author = text
                        font.pixelSize: 20
                    }
                }

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

                    TextField {
                        width: parent.width-parent.spacing-labelWidth
                        text: scriteDocument.screenplay.contact
                        onTextEdited: scriteDocument.screenplay.contact = text
                        font.pixelSize: 20
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

                    TextField {
                        width: parent.width-parent.spacing-labelWidth
                        text: scriteDocument.screenplay.version
                        onTextEdited: scriteDocument.screenplay.version = text
                        font.pixelSize: 20
                    }
                }

                Column {
                    spacing: parent.spacing/2
                    width: parent.width-20
                    anchors.horizontalCenter: parent.horizontalCenter

                    CheckBox2 {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Include Title Page In Preview"
                        checked: screenplayEditorSettings.includeTitlePageInPreview
                        onToggled: screenplayEditorSettings.includeTitlePageInPreview = checked
                    }

                    Button2 {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Reset Paragraph Formats"
                        onClicked: {
                            scriteDocument.formatting.resetToDefaults()
                            scriteDocument.printFormat.resetToDefaults()
                        }
                    }

                    Text {
                        color: accentColors.c600.background
                        width: parent.width
                        font.pixelSize: 10
                        text: "<strong>NOTE:</strong> Clicking this button will reset both print and on-screen paragraph formats."
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
            }
        }
    }

    Component {
        id: pageSetupComponent

        Item {
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

            ScrollView {
                id: pageSetupScroll
                anchors.fill: parent
                ScrollBar.vertical.policy: ScrollBar.AlwaysOn
                ScrollBar.vertical.opacity: ScrollBar.vertical.active ? 1 : 0.2

                Column {
                    spacing: 20
                    width: pageSetupScroll.width-20

                    Text {
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        text: "Page Setup"
                        font.pixelSize: 24
                    }

                    Row {
                        spacing: 10

                        Text {
                            text: "Paper Size"
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        ComboBox2 {
                            width: 200
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

                    GroupBox {
                        width: parent.width
                        label: Text {
                            text: "Header"
                            font.pixelSize: 15
                            font.bold: true
                        }

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
                        width: parent.width
                        label: Text {
                            text: "Footer"
                            font.pixelSize: 15
                            font.bold: true
                        }

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

                    GroupBox {
                        width: parent.width
                        label: CheckBox2 {
                            text: "Watermark"
                            checked: pageSetupSettings.watermarkEnabled
                            onToggled: pageSetupSettings.watermarkEnabled = checked
                        }

                        Grid {
                            width: parent.width
                            columns: 2
                            spacing: 10
                            verticalItemAlignment: Grid.AlignVCenter
                            enabled: pageSetupSettings.watermarkEnabled

                            Text {
                                text: "Text"
                            }

                            TextField {
                                width: 300
                                text: pageSetupSettings.watermarkText
                                onTextEdited: pageSetupSettings.watermarkText = text
                            }

                            Text {
                                text: "Font Family"
                            }

                            ComboBox2 {
                                width: 300
                                model: systemFontInfo.families
                                currentIndex: systemFontInfo.families.indexOf(pageSetupSettings.watermarkFont)
                                onCurrentIndexChanged: pageSetupSettings.watermarkFont = systemFontInfo.families[currentIndex]
                            }

                            Text {
                                text: "Font Size"
                            }

                            SpinBox {
                                width: 300
                                from: 9; to: 200; stepSize: 1
                                editable: true
                                value: pageSetupSettings.watermarkFontSize
                                onValueModified: pageSetupSettings.watermarkFontSize = value
                            }

                            Text {
                                text: "Rotation"
                            }

                            SpinBox {
                                width: 300
                                from: -180; to: 180
                                value: pageSetupSettings.watermarkRotation
                                textFromValue: function(value,locale) { return value + " degrees" }
                                validator: IntValidator { top: 360; bottom: 0 }
                                onValueModified: pageSetupSettings.watermarkRotation = value
                            }

                            Text {
                                text: "Color"
                            }

                            Rectangle {
                                border.width: 1
                                border.color: primaryColors.borderColor
                                color: pageSetupSettings.watermarkColor
                                width: 30; height: 30
                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: pageSetupSettings.watermarkColor = app.pickColor(pageSetupSettings.watermarkColor)
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
    }

    property var systemFontInfo: app.systemFontInfo()

    /**
      We had an opportunity to discuss (over email, Messenger, Zoom calls etc..) about formatting.
      The experienced writers suggested to us that we should not allow deviation from the standard
      screenplay formatting rules.

      As of 0.3.9, we are NOT providing the following options for element formatting.
      1. Font Family
      2. Block Width & Alignment

      We now ONLY PROVIDE the following options
      1. Font Point Size & Weight
      2. Text Alignment
      3. Foreground and Background Color
      4. Line Height
      5. Line Spacing Before

      Also, since 0.3.9 we compute page numbers and count on the fly. This means that we should keep
      online and print formats in sync. So, we cannot afford to let users configure both of them
      separately. Although we need to capture print and display formats as separate ScreenplayFormat
      instances because the DPI and DPR values for printer and displays may not be the same.
      */
    Item {
        property real devicePixelRatio: 1.0

        Component.onCompleted: {
            devicePixelRatio = scriteDocument.formatting.devicePixelRatio
            scriteDocument.formatting.devicePixelRatio = app.devicePixelRatio
        }

        Component.onDestruction: {
            scriteDocument.formatting.devicePixelRatio = devicePixelRatio
        }
    }

    Component {
        id: elementFormatOptionsComponent

        ScrollView {
            id: scrollView
            property real labelWidth: 125
            property var pageData: pageModel.get(pageList.currentIndex)
            property SceneElementFormat displayElementFormat: scriteDocument.formatting.elementFormat(pageData.elementType)
            property SceneElementFormat printElementFormat: scriteDocument.printFormat.elementFormat(pageData.elementType)
            ScrollBar.vertical.opacity: ScrollBar.vertical.active ? 1 : 0.2
            ScrollBar.vertical.policy: ScrollBar.AlwaysOn

            Column {
                width: scrollView.width - 20
                spacing: 0

                Text {
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    text: "<strong>" + pageData.name
                    font.pixelSize: 24
                }

                Rectangle {
                    width: parent.width
                    height: Math.min(300, previewText.contentHeight)
                    color: primaryColors.c10.background
                    border.width: 1
                    border.color: primaryColors.borderColor
                    radius: 8

                    ScrollView {
                        anchors.fill: parent
                        anchors.margins: 8
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
                        ToolTip.text: "Apply this font size to all '" + pageData.group + "' paragraphs."
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
        id: applicationSettingsComponent

        Item {
            id: appSettingsPage

            ScrollView {
                id: appSettingsScrollView
                anchors.fill: parent
                contentWidth: width
                contentHeight: appSettingsPageContent.height
                ScrollBar.vertical.opacity: ScrollBar.vertical.active ? 1 : 0.2

                Item {
                    width: appSettingsScrollView.width
                    height: appSettingsPageContent.height

                    Column {
                        id: appSettingsPageContent
                        width: appSettingsPage.width * 0.8
                        spacing: 20
                        anchors.horizontalCenter: parent.horizontalCenter

                        GroupBox {
                            width: parent.width
                            label: CheckBox2 {
                                text: "Enable AutoSave"
                                checked: scriteDocument.autoSave
                                onToggled: scriteDocument.autoSave = checked
                            }

                            Column {
                                width: parent.width
                                spacing: 10
                                enabled: scriteDocument.autoSave

                                Text {
                                    width: parent.width
                                    text: "Auto Save Interval (in seconds)"
                                }

                                TextField {
                                    width: parent.width
                                    text: scriteDocument.autoSaveDurationInSeconds
                                    validator: IntValidator {
                                        bottom: 1; top: 3600
                                    }
                                    onTextEdited: scriteDocument.autoSaveDurationInSeconds = parseInt(text)
                                }
                            }
                        }

                        GroupBox {
                            width: parent.width

                            Column {
                                width: parent.width
                                spacing: 10

                                CheckBox2 {
                                    checkable: true
                                    checked: structureCanvasSettings.showGrid
                                    text: "Show Grid in Structure Tab"
                                    onToggled: structureCanvasSettings.showGrid = checked
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
                        }

                        GroupBox {
                            width: parent.width
                            label: Text {
                                text: "Active Languages"
                            }
                            height: activeLanguagesView.height+45

                            Grid {
                                id: activeLanguagesView
                                width: parent.width-10
                                anchors.top: parent.top
                                spacing: 5
                                columns: 3

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
                    }
                }
            }
        }
    }
}
