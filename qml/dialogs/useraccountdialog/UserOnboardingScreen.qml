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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import Qt.labs.qmlmodels 1.0

import io.scrite.components 1.0

import "qrc:/qml/tasks"
import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"
import "qrc:/qml/dialogs"

Item {
    id: root

    readonly property bool modal: true
    readonly property string title: "User Onboarding Survey"

    property bool standalone: false

    signal formSubmitted()

    Component.onCompleted: {
        const success = _fetchFormApi.call()
        if(!success) {
            _deactivateDeviceCall.call()
        }
    }

    Image {
        anchors.fill: parent
        opacity: 0.2
        source: "qrc:/images/useraccountdialogbg.png"
        fillMode: standalone ? Image.PreserveAspectFit : Image.PreserveAspectCrop
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20

        visible: !_fetchFormApi.busy && _fetchFormApi.hasResponse

        spacing: 20

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true

            color: Runtime.colors.primary.c10.background
            border.width: 1
            border.color: Runtime.colors.primary.borderColor

            VclScrollBar {
                id: _formFlickScrollBar

                anchors.top: parent.top
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                flickable: _formFlick
            }

            Flickable {
                id: _formFlick

                anchors.fill: parent
                anchors.topMargin: 5
                anchors.leftMargin: 20
                anchors.rightMargin: 31
                anchors.bottomMargin: 5

                ScrollBar.vertical: _formFlickScrollBar

                clip: true
                contentWidth: _formLayout.width
                contentHeight: _formLayout.implicitHeight + 120

                ColumnLayout {
                    id: _formLayout

                    width: _formFlick.width - 20

                    spacing: 25

                    ColumnLayout {
                        spacing: 10

                        Layout.fillWidth: true
                        Layout.topMargin: _formLayout.spacing

                        VclLabel {
                            Layout.fillWidth: true

                            wrapMode: Text.WordWrap
                            text: _formFields.title
                            font.bold: true
                            font.pointSize: Runtime.idealFontMetrics.font.pointSize + 4
                        }

                        VclLabel {
                            Layout.fillWidth: true

                            wrapMode: Text.WordWrap
                            text: _formFields.description
                        }
                    }

                    Repeater {
                        id: _fields

                        model: _formFields

                        delegate: DelegateChooser {
                            role: "type"

                            DelegateChoice {
                                roleValue: "text"
                                delegate: TextFormField { }
                            }

                            DelegateChoice {
                                roleValue: "phone"
                                delegate: PhoneFormField { }
                            }

                            DelegateChoice {
                                roleValue: "binary"
                                delegate: BinaryFormField { }
                            }

                            DelegateChoice {
                                roleValue: "single_select"
                                delegate: SingleSelectFormField { }
                            }

                            DelegateChoice {
                                roleValue: "multi_select"
                                delegate: MultiSelectFormField { }
                            }
                        }
                    }
                }
            }
        }

        VclButton {
            id: _submit

            Layout.alignment: Qt.AlignRight

            text: "Submit »"
            enabled: false

            function determineEnabled() {
                let formData = {}
                const mustFillFields = _formFields.mandatoryFields

                for(let i=0; i<_fields.count; i++) {
                    formData = _fields.itemAt(i).updateFormData(formData)

                    const fieldName = _formFields.get(i).name
                    if(mustFillFields.indexOf(fieldName) >= 0 && formData[fieldName] === undefined) {
                        enabled = false
                        return
                    }
                }

                enabled = true
            }

            onClicked: {
                let formData = {}
                for(let i=0; i<_fields.count; i++) {
                    formData = _fields.itemAt(i).updateFormData(formData)
                }

                _submitFormApi.formData = formData
                _submitFormApi.call()
            }
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: _fetchFormApi.busy || _submitFormApi.busy
    }

    TabSequenceManager {
        id: _tabSequence
    }

    UserOnboardingFormApiCall {
        id: _fetchFormApi

        onFinished: {
            if(hasResponse && !hasError) {
                _formFields.reload()
                return
            }

            MessageBox.information("Error",
                                   "Couldn't fetch form fields. Click Ok to try again.",
                                   _fetchFormApi.call)
        }
    }

    UserSubmitOnboardingFormApiCall {
        id: _submitFormApi

        onFinished: {
            if(hasResponse && !hasError) {
                Runtime.userAccountDialogSettings.userOnboardingStatus = "completed"
                if(standalone)
                    root.formSubmitted()
                else
                    Runtime.shoutout(Runtime.announcementIds.userAccountDialogScreen, "UserProfileScreen")
                return
            }

            MessageBox.information("Error", "There was an error submitting form data. Please try again.")
        }
    }

    InstallationDeactivateRestApiCall {
        id: _deactivateDeviceCall

        onFinished: {
            if(!hasError)
                Runtime.shoutout(Runtime.announcementIds.userAccountDialogScreen, "AccountEmailScreen")
        }
    }

    ListModel {
        id: _formFields

        property var mandatoryFields: []
        property string title
        property string description

        function reload() {
            const form = _fetchFormApi.responseData.form
            const formData = _fetchFormApi.responseData.formData
            let mustFillFields = []

            title = form.title
            description = form.description

            clear()

            for(let i=0; i<form.fields.length; i++) {
                const item = {
                    "type": form.fields[i].type,
                    "name": form.fields[i].name,
                    "label": form.fields[i].label,
                    "mandatory": form.fields[i].mandatory,
                    "metaData": form.fields[i],
                    "value": formData ? formData[form.fields[i].name] : null
                }
                if(item.mandatory === true)
                    mustFillFields.push(item.name)

                append(item)
            }

            mandatoryFields = mustFillFields

            Qt.callLater(_submit.determineEnabled)
        }
    }

    component TextFormField: ColumnLayout {
        required property int index
        required property var metaData
        required property var value
        required property bool mandatory
        required property string label
        required property string name

        Layout.fillWidth: true

        VclLabel {
            Layout.fillWidth: true

            text: parent.label + (parent.mandatory ? " *" : "")
            font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2
        }

        VclTextField {
            id: _textField

            Layout.fillWidth: true

            TabSequenceItem.manager: _tabSequence

            text: parent.value !== null && typeof parent.value === "string" ? parent.value : ""
            maximumLength: 128

            onTextEdited: if(parent.mandatory) Qt.callLater(_submit.determineEnabled)
        }

        function updateFormData(formData) {
            formData[name] = _textField.text
            return formData
        }
    }

    component PhoneFormField: ColumnLayout {
        required property int index
        required property var metaData
        required property var value
        required property bool mandatory
        required property string label
        required property string name

        Layout.fillWidth: true

        VclLabel {
            Layout.fillWidth: true

            text: parent.label + (parent.mandatory ? " *" : "")
            font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2
        }

        VclTextField {
            id: _phoneField

            Layout.fillWidth: true

            TabSequenceItem.manager: _tabSequence

            text: parent.value !== null && typeof parent.value === "string" ? parent.value : ""
            maximumLength: 25
            validator: RegExpValidator {
                regExp: /^\+?(\d{1,3})?[\s\-]?\(?\d{1,4}\)?[\s\-]?\d{1,4}[\s\-]?\d{1,4}$/
            }

            onTextEdited: if(parent.mandatory) Qt.callLater(_submit.determineEnabled)
        }

        function updateFormData(formData) {
            formData[name] = _phoneField.text
            return formData
        }
    }

    component BinaryFormField: ColumnLayout {
        id: _binaryField

        required property int index
        required property var metaData
        required property var value
        required property bool mandatory
        required property string label
        required property string name

        Layout.fillWidth: true

        VclLabel {
            Layout.fillWidth: true

            text: parent.label + (parent.mandatory ? " *" : "")
            font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: parent.width * 0.25

            VclRadioButton {
                id: _option1

                TabSequenceItem.manager: _tabSequence

                text: _binaryField.metaData.options[0]

                checked: text === _binaryField.value

                onToggled: if(parent.mandatory) Qt.callLater(_submit.determineEnabled)
            }

            VclRadioButton {
                id: _option2

                TabSequenceItem.manager: _tabSequence

                text: _binaryField.metaData.options[1]

                checked: text === _binaryField.value

                onToggled: if(parent.mandatory) Qt.callLater(_submit.determineEnabled)
            }
        }

        function updateFormData(formData) {
            formData[name] = _option1.checked ? _option1.text : (_option2.checked ? _option2.text : undefined)
            return formData
        }
    }

    component SingleSelectFormField: ColumnLayout {
        required property int index
        required property var metaData
        required property var value
        required property bool mandatory
        required property string label
        required property string name

        property bool other: metaData.other
        property var choices: other ? [...metaData.choices, "Other"] : metaData.choices

        Layout.fillWidth: true

        VclLabel {
            Layout.fillWidth: true

            text: parent.label + (parent.mandatory ? " *" : "")
            wrapMode: Text.WordWrap
            font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2
        }

        VclComboBox {
            id: _ssChoices

            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: parent.width * 0.25

            TabSequenceItem.manager: _tabSequence

            model: parent.choices
            currentIndex: {
                if(parent.value !== null && typeof parent.value === "string" && parent.value !== "") {
                    const idx = parent.metaData.choices.indexOf(parent.value)
                    if(idx < 0)
                        return parent.other === true ? count-1 : -1
                    return idx
                }
                return -1
            }
            displayText: currentIndex < 0 ? "--- select ---" : currentText

            onActivated: if(parent.mandatory) Qt.callLater(_submit.determineEnabled)
        }

        VclTextField {
            id: _ssOtherText

            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: parent.width * 0.25

            TabSequenceItem.manager: _tabSequence
            TabSequenceItem.enabled: visible

            visible: parent.other && _ssChoices.currentIndex === _ssChoices.count-1
            label: ""
            text: parent.value !== null && typeof parent.value === "string" ? parent.value : ""
            maximumLength: 128
            placeholderText: "Please specify"

            onTextEdited: if(parent.mandatory) Qt.callLater(_submit.determineEnabled)
        }

        function updateFormData(formData) {
            if(_ssOtherText.visible)
                formData[name] = _ssOtherText.text
            else if(_ssChoices.currentIndex >= 0)
                formData[name] = _ssChoices.currentText
            return formData
        }
    }

    component MultiSelectFormField: ColumnLayout {
        id: _msFormField

        required property int index
        required property var metaData
        required property var value
        required property bool mandatory
        required property string label
        required property string name

        property var choices: metaData.choices
        property var checkedValues: value !== null && typeof value === "string" ? value.split(";;") : []

        Layout.fillWidth: true

        VclLabel {
            Layout.fillWidth: true

            text: parent.label + (parent.mandatory ? " *" : "")
            wrapMode: Text.WordWrap
            font.pointSize: Runtime.idealFontMetrics.font.pointSize + 2
        }

        Flow {
            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: parent.width * 0.25

            spacing: 10

            Repeater {
                id: _msCheckBoxes
                model: _msFormField.choices

                delegate: VclCheckBox {
                    required property int index
                    required property string modelData

                    TabSequenceItem.manager: _tabSequence

                    text: modelData
                    checked: _msFormField.checkedValues.indexOf(text) >= 0
                    padding: 0

                    onToggled: if(parent.mandatory) Qt.callLater(_submit.determineEnabled)
                }
            }
        }

        function updateFormData(formData) {
            let list = []
            for(let i=0; i<_msCheckBoxes.count; i++) {
                if(_msCheckBoxes.itemAt(i).checked)
                    list.push(_msCheckBoxes.itemAt(i).text)
            }
            formData[name] = list.join(";;")
        }
    }
}
