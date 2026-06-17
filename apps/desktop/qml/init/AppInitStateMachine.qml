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

import QtQml
import QtQuick
import QtQml.StateMachine as QSM

import io.scrite.components

import "../globals"
import "../dialogs"
import "../overlays"
import "../commandcenter"
import "../notifications"
import "../floatingdockpanels"

QSM.StateMachine {
    id: root

    required property Loader contentLoader

    initialState: s1RuntimeInit
    running: false

    function start() { running = true }

    /*
     * Initialises the runtime environment: sets up the content loader reference
     * inside Runtime, resolves the default QML font point size, and raises the
     * application window to the foreground.
     */
    QSM.State {
        id: s1RuntimeInit

        signal done()

        onEntered: {
            Runtime.init(root.contentLoader)
            root._determineDefaultFontSize()
            Scrite.window.showMaximized()
            Scrite.window.raise()
            Qt.callLater(() => s1RuntimeInit.done())
        }

        QSM.SignalTransition { signal: s1RuntimeInit.done; targetState: s2Prelaunch }
    }

    /*
     * Runs platform-specific pre-launch checks. On production Windows builds
     * this guards against running alongside a legacy NSIS installation. If the
     * check fails the user is informed and the app exits; otherwise init continues.
     */
    QSM.State {
        id: s2Prelaunch

        signal passed()
        signal failed()

        onEntered: {
            if (!Scrite.prelaunchChecks()) {
                MessageBox.information(
                    "Previous Version Detected",
                    "A previous version of Scrite is installed on this computer.\n\n" +
                    "Please uninstall it via \"Add or Remove Programs\" in Windows Settings " +
                    "before running this version.",
                    () => s2Prelaunch.failed()
                )
            } else {
                Qt.callLater(() => s2Prelaunch.passed())
            }
        }

        QSM.SignalTransition { signal: s2Prelaunch.passed; targetState: s3aLicense }
        QSM.SignalTransition { signal: s2Prelaunch.failed; targetState: sQuit }
    }

    /*
     * Shows the license dialog the first time each new version is launched.
     * Skipped silently if the license for this version is already on record.
     * Accepting advances to s3bLegacyMigration; declining exits the app.
     */
    QSM.State {
        id: s3aLicense

        signal accepted()
        signal declined()

        onEntered: {
            if (!Scrite.isLicenseAccepted()) {
                let dlg = LicenseDialog.launch()
                if (dlg) {
                    dlg.accepted.connect(() => s3aLicense.accepted())
                    dlg.rejected.connect(() => s3aLicense.declined())
                } else {
                    Qt.callLater(() => s3aLicense.accepted())
                }
            } else {
                Qt.callLater(() => s3aLicense.accepted())
            }
        }

        QSM.SignalTransition { signal: s3aLicense.accepted; targetState: s3bLegacyMigration }
        QSM.SignalTransition { signal: s3aLicense.declined; targetState: sQuit }
    }

    /*
     * If Scrite recently moved the user's settings, recent files, and vault to
     * a new location on disk, this state shows a one-time notice explaining the
     * change. The dialog is purely informational — there is no decline path.
     */
    QSM.State {
        id: s3bLegacyMigration

        signal done()

        onEntered: {
            if (Scrite.app.hasLegacyDataMovedRecently()) {
                let dlg = LegacyDataMigrationDialog.launch()
                if (dlg)
                    dlg.accepted.connect(() => s3bLegacyMigration.done())
                else
                    Qt.callLater(() => s3bLegacyMigration.done())
            } else {
                Qt.callLater(() => s3bLegacyMigration.done())
            }
        }

        QSM.SignalTransition { signal: s3bLegacyMigration.done; targetState: s4AppInit }
    }

    /*
     * Initialises all application modules (ActionHub, HelpCenter, CommandCenter,
     * SubscriptionPlanOperations), activates the main content loader, then wires
     * up the UI layers (BusyOverlay, dialogs, dock panels, notifications).
     * Branches to s5Splash, s6HomeOrFile, or s7LoginRequired based on login
     * state and subscription entitlement.
     */
    QSM.State {
        id: s4AppInit

        signal done()

        onEntered: {
            ActionHub.init(root.contentLoader)
            HelpCenter.init(root.contentLoader)
            CommandCenter.init(root.contentLoader)
            SubscriptionPlanOperations.init(root.contentLoader)

            root.contentLoader.active = true

            BusyOverlay.init(root.contentLoader)
            SubscriptionDetailsDialog.init()
            SubscriptionPlanComparisonDialog.init()
            UserAccountDialog.init(root.contentLoader)
            FloatingDockLayer.init(root.contentLoader)
            OverlaysLayer.init(root.contentLoader)
            NotificationsLayer.init(root.contentLoader)

            Qt.callLater(() => s4AppInit.done())
        }

        QSM.SignalTransition {
            signal: s4AppInit.done; targetState: s5Splash
            guard: !Scrite.user.loggedIn
        }
        QSM.SignalTransition {
            signal: s4AppInit.done; targetState: s6HomeOrFile
            guard: Scrite.user.loggedIn && Runtime.allowAppUsage
        }
        QSM.SignalTransition {
            signal: s4AppInit.done; targetState: s7LoginRequired
            guard: Scrite.user.loggedIn && !Runtime.allowAppUsage
        }
    }

    /*
     * Entered when the user is not logged in. Shows the splash screen and waits
     * for it to close (either by user interaction or the 5-second auto-dismiss
     * timer). On Windows versions older than 10, an additional compatibility
     * warning is shown before proceeding. Exits to s6HomeOrFile or s7LoginRequired
     * depending on subscription entitlement at that point.
     */
    QSM.State {
        id: s5Splash

        signal closed()

        onEntered: {
            let splash = SplashScreen.launch()
            if (splash) {
                splash.closed.connect(() => {
                    if (Platform.isWindowsDesktop && Platform.osMajorVersion < 10) {
                        MessageBox.information("",
                            "The Windows version of Scrite works best on Windows 10 or higher. " +
                            "While it may work on earlier versions of Windows, we don't actively " +
                            "test on them. We recommend that you use Scrite on PCs with Windows 10 or higher.",
                            () => s5Splash.closed()
                        )
                    } else {
                        s5Splash.closed()
                    }
                })
            } else {
                Qt.callLater(() => s5Splash.closed())
            }
        }

        QSM.SignalTransition {
            signal: s5Splash.closed; targetState: s6HomeOrFile
            guard: Runtime.allowAppUsage
        }
        QSM.SignalTransition {
            signal: s5Splash.closed; targetState: s7LoginRequired
            guard: !Runtime.allowAppUsage
        }
    }

    /*
     * Normal startup destination when the user is permitted to use the app.
     * Opens a file passed on the command line, or falls back to an anonymous
     * document or the Home Screen. Also schedules the onboarding survey check
     * to run 2 seconds later.
     */
    QSM.State {
        id: s6HomeOrFile

        signal done()

        onEntered: {
            if (Scrite.fileNameToOpen === "") {
                if (!Scrite.app.maybeOpenAnonymously())
                    HomeScreen.launch()
            } else {
                Scrite.document.open(Scrite.fileNameToOpen)
            }
            Runtime.execLater(root, 2000, root._maybeOnboardUserSurvey)
            Qt.callLater(() => s6HomeOrFile.done())
        }

        QSM.SignalTransition { signal: s6HomeOrFile.done; targetState: sDone }
    }

    /*
     * Entered when the user is logged in but not entitled to use the app
     * (e.g. subscription lapsed). Launches the UserAccountDialog so the user
     * can resolve their account status.
     */
    QSM.State {
        id: s7LoginRequired

        signal done()

        onEntered: {
            UserAccountDialog.launch()
            Qt.callLater(() => s7LoginRequired.done())
        }

        QSM.SignalTransition { signal: s7LoginRequired.done; targetState: sDone }
    }

    /*
     * Terminal failure state. Entered when a pre-launch check fails or the
     * license is declined. Calls Qt.quit() to exit the application cleanly.
     */
    QSM.State {
        id: sQuit
        onEntered: Qt.quit()
    }

    /*
     * Terminal success state. Entered after s6HomeOrFile or s7LoginRequired
     * completes, signalling that the init sequence finished normally.
     */
    QSM.FinalState {
        id: sDone
    }

    // ── Helpers ──────────────────────────────────────────────────────────────

    function _determineDefaultFontSize() {
        if (Scrite.app.customFontPointSize === 0) {
            let textItemObj = Qt.createQmlObject("import QtQuick; Text { text: \"Welcome to Scrite\" }", root.contentLoader)
            let textItem = textItemObj as Text
            if (textItem) {
                Scrite.app.customFontPointSize = textItem.font.pointSize
                textItem.destroy()
            }
        }
    }

    function _maybeOnboardUserSurvey() {
        if (Runtime.userAccountDialogSettings.userOnboardingStatus === "required")
            UserOnboardingDialog.launch()
    }
}
