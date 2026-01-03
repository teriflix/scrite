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
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.15
import Qt.labs.settings 1.0
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/tasks"

import "qrc:/qml/globals"
import "qrc:/qml/dialogs"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/mainwindow"
import "qrc:/qml/notifications"
import "qrc:/qml/screenplayeditor"
import "qrc:/qml/floatingdockpanels"

Item {
    id: root

    width: 1350
    height: 700

    enabled: !Scrite.document.loading

    ColumnLayout {
        id: _layout

        anchors.fill: parent

        spacing: 0

        Header {
            id: _header

            Layout.fillWidth: true

            z: 1

            ActionHandler {
                action: ActionHub.applicationOptions.find("toggleToolbarVisibility")

                checked: _header.visible
                onToggled: (source) => {
                               Qt.callLater(_header.toggleVisibility)
                           }
            }

            function toggleVisibility() {
                visible = !visible
            }
        }

        Workspace {
            id: _workspace

            Layout.fillWidth: true
            Layout.fillHeight: true

            z: 0
        }
    }

    ReloadPromptDialog { }

    QtObject {
        id: _private

        readonly property Component helpTipNotification: HelpTipNotification {
            id: _helpTip

            Notification.onDismissed: _helpTip.destroy()
        }

        readonly property Connections userConnections: Connections {
            target: Scrite.user

            function onRequestVersionTypeAccess() {
                RequestVersionTypeAccess.launch()
            }
        }

        property bool handleCloseEvent: true
        property bool hasDocumentErrors: documentErrors.hasError
        property bool hasApplicationErrors: applicationErrors.hasError

        property ErrorReport documentErrors: Aggregation.errorReport(Scrite.document)
        property ErrorReport applicationErrors: Aggregation.errorReport(Scrite.app)

        function handleOpenFileRequest(fileName) {
            if(Platform.isMacOSDesktop) {
                if(Scrite.document.empty) {
                    Runtime.shoutout(Runtime.announcementIds.closeHomeScreenRequest, undefined)
                    OpenFileTask.open(fileName)
                } else {
                    let fileInfo = Qt.createQmlObject("import io.scrite.components 1.0; BasicFileInfo { }", _private)
                    fileInfo.absoluteFilePath = fileName

                    const justFileName = fileInfo.baseName
                    fileInfo.destroy()

                    MessageBox.question("Open Options",
                                        "How do you want to open <b>" + justFileName + "</b>?",
                                        ["This Window", "New Window"], (answer) => {
                                            if(answer === "This Window")
                                                OpenFileTask.open(fileName)
                                            else
                                                Scrite.app.launchNewInstanceAndOpen(Scrite.window, fileName);
                                        })
                }
            }
        }

        function showHelpTip(tipName) {
            if(Runtime.helpTips && Runtime.helpTips[tipName] !== undefined && !Runtime.helpNotificationSettings.isTipShown(tipName)) {
                helpTipNotification.createObject(Scrite.window.contentItem, {"tipName": tipName})
            }
        }

        function maybeShowDiscordHelpTip() {
            if(Runtime.helpNotificationSettings.dayZero === "")
               Runtime.helpNotificationSettings.dayZero = new Date()

            const days = Runtime.helpNotificationSettings.daysSinceZero()
            if(days >= 2) {
                if(!Runtime.helpNotificationSettings.isTipShown("discord"))
                    showHelpTip("discord")
            }
        }

        function handleWindowClosing(close) {
            if(!Scrite.window.closeButtonVisible) {
                close.accepted = false
                return
            }

            if(!Scrite.user.canUseAppVersionType) {
                close.accepted = true
                return
            }

            if(handleCloseEvent) {
                close.accepted = false

                Scrite.app.saveWindowGeometry(Scrite.window, "Workspace")

                SaveFileTask.save( () => {
                                      _private.handleCloseEvent = false
                                      if( TrialNotActivatedDialog.launch() !== null)
                                        return

                                      Scrite.window.close()
                                  } )
            } else
                close.accepted = true
        }

        Announcement.onIncoming: (type, data) => {
                                     if(type === Runtime.announcementIds.showHelpTip) {
                                         _private.showHelpTip(""+data)
                                     }
                                 }

        Component.onCompleted: {
            Scrite.window.closing.connect(handleWindowClosing)

            if(Platform.isMacOSDesktop)
                Scrite.app.openFileRequest.connect(handleOpenFileRequest)

            Qt.callLater(maybeShowDiscordHelpTip)

            if(!Scrite.app.restoreWindowGeometry(Scrite.window, "Workspace"))
                Runtime.workspaceSettings.screenplayEditorWidth = -1

            _workspace.reset()
        }

        onHasApplicationErrorsChanged: {
            if(hasApplicationErrors)
                MessageBox.information("Scrite Error", applicationErrors.errorMessage, applicationErrors.clear)
        }

        onHasDocumentErrorsChanged: {
            if(hasDocumentErrors) {
                var msg = documentErrors.errorMessage;

                if(documentErrors.details && documentErrors.details.revealOnDesktopRequest)
                    msg += "<br/><br/>Click Ok to reveal <u>" + documentErrors.details.revealOnDesktopRequest + "</u> on your computer."

                MessageBox.information("Scrite Document Error", msg, () => {
                                           if(documentErrors.details && documentErrors.details.revealOnDesktopRequest)
                                               File.revealOnDesktop(documentErrors.details.revealOnDesktopRequest)
                                           documentErrors.clear()
                                       })
            }
        }

    }
}
