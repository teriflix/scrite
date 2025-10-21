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
import "qrc:/qml/scrited"
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

    // Refactor QML TODO: Get rid of this stuff when we move to overlays and ApplicationMainWindow
    QtObject {
        property bool overlayRefCountModified: false
        property bool requiresAppBusyOverlay: Runtime.undoStack.screenplayEditorActive || Runtime.undoStack.sceneEditorActive

        function onUpdateScheduled() {
            if(requiresAppBusyOverlay && !overlayRefCountModified) {
                appBusyOverlay.ref()
                overlayRefCountModified = true
            }
        }

        function onUpdateFinished() {
            if(overlayRefCountModified)
                appBusyOverlay.deref()
            overlayRefCountModified = false
        }

        onRequiresAppBusyOverlayChanged: {
            if(!requiresAppBusyOverlay && overlayRefCountModified) {
                appBusyOverlay.deref()
                overlayRefCountModified = false
            }
        }

        Component.onCompleted: {
            // Cannot use Connections for this, because the Connections QML item
            // does not allow usage of custom properties
            Runtime.screenplayTextDocument.onUpdateScheduled.connect(onUpdateScheduled)
            Runtime.screenplayTextDocument.onUpdateFinished.connect(onUpdateFinished)
        }
    }

    ColumnLayout {
        id: _layout

        anchors.fill: parent

        spacing: 0

        Header {
            id: _header

            Layout.fillWidth: true

            z: 1
        }

        Workspace {
            id: _workspace

            Layout.fillWidth: true
            Layout.fillHeight: true

            z: 0
        }
    }

    ReloadPromptDialog { }

    Item {
        id: closeEventHandler
        width: 100
        height: 100
        anchors.centerIn: parent

        property bool handleCloseEvent: true

        Connections {
            target: Scrite.window

            function onClosing(close) {
                if(!Scrite.window.closeButtonVisible) {
                    close.accepted = false
                    return
                }

                if(closeEventHandler.handleCloseEvent) {
                    close.accepted = false

                    Scrite.app.saveWindowGeometry(Scrite.window, "Workspace")

                    SaveFileTask.save( () => {
                                          closeEventHandler.handleCloseEvent = false
                                          if( TrialNotActivatedDialog.launch() !== null)
                                            return
                                          Scrite.window.close()
                                      } )
                } else
                    close.accepted = true
            }
        }
    }

    Component.onCompleted: {
        if(!Scrite.app.restoreWindowGeometry(Scrite.window, "Workspace"))
            Runtime.workspaceSettings.screenplayEditorWidth = -1
        Runtime.screenplayAdapter.sessionId = Scrite.document.sessionId
        _workspaceLoader.reset()
    }

    BusyOverlay {
        id: appBusyOverlay
        anchors.fill: parent
        busyMessage: "Computing Page Layout, Evaluating Page Count & Time ..."
        visible: RefCounter.isReffed
        function ref() { RefCounter.ref() }
        function deref() { RefCounter.deref() }
    }

    HelpTipNotification {
        id: htNotification
        enabled: tipName !== ""

        Component.onCompleted: {
            Qt.callLater( () => {
                             if(Runtime.helpNotificationSettings.dayZero === "")
                                Runtime.helpNotificationSettings.dayZero = new Date()

                             const days = Runtime.helpNotificationSettings.daysSinceZero()
                             if(days >= 2) {
                                 if(!Runtime.helpNotificationSettings.isTipShown("discord"))
                                     htNotification.tipName = "discord"
                             }
                         })
        }
    }

    QtObject {
        property ErrorReport applicationErrors: Aggregation.findErrorReport(Scrite.app)
        property bool errorReportHasError: applicationErrors.hasError
        onErrorReportHasErrorChanged: {
            if(errorReportHasError)
                MessageBox.information("Scrite Error", applicationErrors.errorMessage, applicationErrors.clear)
        }
    }

    QtObject {
        property ErrorReport documentErrors: Aggregation.findErrorReport(Scrite.document)
        property bool errorReportHasError: documentErrors.hasError
        onErrorReportHasErrorChanged: {
            if(errorReportHasError) {
                var msg = documentErrors.errorMessage;

                if(documentErrors.details && documentErrors.details.revealOnDesktopRequest)
                    msg += "<br/><br/>Click Ok to reveal <u>" + documentErrors.details.revealOnDesktopRequest + "</u> on your computer."

                MessageBox.information("Scrite Document Error", msg, () => {
                                           if(documentErrors.details && documentErrors.details.revealOnDesktopRequest)
                                               Scrite.app.revealFileOnDesktop(documentErrors.details.revealOnDesktopRequest)
                                           documentErrors.clear()
                                       })
            }
        }
    }

    QtObject {
        id: _private

        function handleOpenFileRequest(fileName) {
            if(Scrite.app.isMacOSPlatform) {
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
            if(Runtime.helpTips[tipName] !== undefined && !Runtime.helpNotificationSettings.isTipShown(tipName)) {
                helpTipNotification.createObject(Scrite.window.contentItem, {"tipName": tipName})
            }
        }

        Announcement.onIncoming: (type, data) => {
                                     if(type === Runtime.announcementIds.showHelpTip) {
                                         _private.showHelpTip(""+data)
                                     }
                                 }

        Component.onCompleted: {
            if(Scrite.app.isMacOSPlatform)
                Scrite.app.openFileRequest.connect(handleOpenFileRequest)
        }
    }

    Component {
        id: helpTipNotification
        HelpTipNotification {
            id: helpTip
            Notification.onDismissed: helpTip.destroy()
        }
    }
}
