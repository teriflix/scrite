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
import QtQuick.Window 2.13
import QtQuick.Dialogs 1.3
import QtQuick.Controls 2.13
import Scrite 1.0

Item {
    property Character character

    FileDialog {
        id: fileDialog
        nameFilters: ["Photos (*.jpg *.png *.bmp *.jpeg)"]
        selectFolder: false
        selectMultiple: false
        sidebarVisible: true
        selectExisting: true
        onAccepted: {
            if(fileUrl != "")
                character.addPhoto(app.urlToLocalFile(fileUrl))
        }
    }

    ScrollView {
        id: characterQuickInfoView
        width: 300
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 5

        Column {
            width: characterQuickInfoView.width - 10
            spacing: 10

            Rectangle {
                width: parent.width
                height: parent.width
                border.width: 1
                border.color: primaryColors.borderColor

                SwipeView {
                    id: photoSlides
                    anchors.fill: parent
                    anchors.margins: 2

                    Repeater {
                        model: character.photos

                        Image {
                            width: photoSlides.width
                            height: photoSlides.height
                            fillMode: Image.PreserveAspectFit
                            source: "file:///" + modelData
                        }

                        Item {
                            width: photoSlides.width
                            height: photoSlides.height

                            Button2 {
                                anchors.centerIn: parent
                                text: "Add Photo"
                                onClicked: fileDialog.open()
                                enabled: !scriteDocument.readOnly
                            }
                        }
                    }
                }

                PageIndicator {
                    count: photoSlides.count
                    currentIndex: photoSlides.currentIndex
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }

            TextField2 {
                id: nameField
                label: "Name:"
                labelAlwaysVisible: true
                placeholderText: "<max 256 letters>"
                maximumLength: 256
                text: character.name
                tabItem: typeField
                readOnly: true
                enableTransliteration: true
            }

            TextField2 {
                id: typeField
                label: "Type:"
                labelAlwaysVisible: true
                placeholderText: "Human/Animal/Robot <max 25 letters>"
                maximumLength: 25
                text: character.type
                backTabItem: nameField
                tabItem: designationField
                onEditingComplete: character.type = text
                enableTransliteration: true
                readOnly: scriteDocument.readOnly
            }

            TextField2 {
                id: designationField
                label: "Designation:"
                labelAlwaysVisible: true
                placeholderText: "Hero/Heroine/Villian/Other <max 256 letters>"
                maximumLength: 256
                text: character.designation
                backTabItem: typeField
                tabItem: genderField
                onEditingComplete: character.designation = text
                enableTransliteration: true
                readOnly: scriteDocument.readOnly
            }

            TextField2 {
                id: genderField
                label: "Gender:"
                placeholderText: "Gender <max 20 letters>"
                maximumLength: 20
                text: character.gender
                backTabItem: designationField
                tabItem: ageField
                onEditingComplete: character.gender = text
                enableTransliteration: true
                readOnly: scriteDocument.readOnly
            }

            TextField2 {
                id: ageField
                label: "Age:"
                placeholderText: "Age <max 20 letters>"
                maximumLength: 20
                text: character.age
                backTabItem: genderField
                tabItem: heightField
                onEditingComplete: character.age = text
                enableTransliteration: true
                readOnly: scriteDocument.readOnly
            }

            TextField2 {
                id: heightField
                label: "Height:"
                placeholderText: "Height <max 20 letters>"
                maximumLength: 20
                text: character.age
                backTabItem: ageField
                tabItem: nameField
                onEditingComplete: character.height = text
                enableTransliteration: true
                readOnly: scriteDocument.readOnly
            }
        }
    }

    Item {
        anchors.left: characterQuickInfoView.right
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 5

        Row {
            id: tabBar
            anchors.horizontalCenter: parent.horizontalCenter
            property int currentIndex: 0

            TabBarTab {
                text: "Relationships"
                tabIndex: 0
                tabCount: 2
                currentTabIndex: tabBar.currentIndex
                onRequestActivation: tabBar.currentIndex = tabIndex
                font.pixelSize: active ? 20 : 16
            }

            TabBarTab {
                text: "Notes"
                tabIndex: 1
                tabCount: 2
                currentTabIndex: tabBar.currentIndex
                onRequestActivation: tabBar.currentIndex = tabIndex
                font.pixelSize: active ? 20 : 16
            }
        }

        Loader {
            anchors.left: parent.left
            anchors.top: tabBar.bottom
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            sourceComponent: {
                if(tabBar.currentIndex === 0)
                    return relationshipsComponent
                return notesComponent
            }
        }
    }

    Component {
        id: relationshipsComponent
    }

    Component {
        id: notesComponent
    }
}
