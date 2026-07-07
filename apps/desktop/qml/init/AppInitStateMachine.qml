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
     * check fails the user is informed, the legacy uninstaller is launched, and
     * the app exits; otherwise init continues.
     */
    QSM.State {
        id: s2Prelaunch

        signal passed()
        signal failed()

        onEntered: {
            root._deferredLicenseCheck = Scrite.user.loggedIn
            const info = Scrite.prelaunchChecks()
            if (info) {
                if (info.uninstaller !== "YES") {
                    const versionLabel = info.version ? ("v" + info.version + " ") : ""
                    const launchButton = "Launch " + versionLabel + "Uninstaller"
                    MessageBox.question(
                        "Previous Version Detected",
                        "A previous version of Scrite is installed on this computer.\n\n" +
                        "Please uninstall it before running this version. Would you like to launch the uninstaller now?",
                        [launchButton, "No, I Will Uninstall Myself"],
                        (button) => {
                            if (button === launchButton)
                                Scrite.launchLegacyUninstaller(info.uninstaller)
                            s2Prelaunch.failed()
                        }
                    )
                } else {
                    MessageBox.information(
                        "Previous Version Detected",
                        "A previous version of Scrite is installed on this computer.\n\n" +
                        "Please uninstall it via \"Add or Remove Programs\" in Windows Settings before running this version.",
                        () => s2Prelaunch.failed()
                    )
                }
            } else {
                Qt.callLater(() => s2Prelaunch.passed())
            }
        }

        QSM.SignalTransition {
            signal: s2Prelaunch.passed; targetState: s3License
            guard: !root._deferredLicenseCheck
        }
        QSM.SignalTransition {
            signal: s2Prelaunch.passed; targetState: s4AppInit
            guard: root._deferredLicenseCheck
        }
        QSM.SignalTransition { signal: s2Prelaunch.failed; targetState: sQuit }
    }

    /*
     * Shows the license dialog the first time each new version is launched.
     * Skipped silently if the license for this version is already on record.
     * Accepting advances to s4AppInit; declining exits the app.
     */
    QSM.State {
        id: s3License

        signal accepted()
        signal declined()

        onEntered: {
            if (!Scrite.isLicenseAccepted()) {
                let dlg = LicenseDialog.launch()
                if (dlg) {
                    dlg.accepted.connect(() => s3License.accepted())
                    dlg.rejected.connect(() => s3License.declined())
                } else {
                    Qt.callLater(() => s3License.accepted())
                }
            } else {
                Qt.callLater(() => s3License.accepted())
            }
        }

        QSM.SignalTransition {
            signal: s3License.accepted; targetState: s4AppInit
            guard: !root._deferredLicenseCheck
        }
        QSM.SignalTransition {
            signal: s3License.accepted; targetState: sDone
            guard: root._deferredLicenseCheck
        }
        QSM.SignalTransition { signal: s3License.declined; targetState: sQuit }
    }

    /*
     * Initialises all application modules (ActionHub, HelpCenter, CommandCenter,
     * SubscriptionPlanOperations), activates the main content loader, then wires
     * up the UI layers (BusyOverlay, dialogs, dock panels, notifications).
     * This runs after the license dialog to ensure the user sees the license
     * before any potential initialization delays from loading ScriteMainWindowContent.
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

        QSM.SignalTransition { signal: s4AppInit.done; targetState: s5LegacyMigration }
    }

    /*
     * If Scrite recently moved the user's settings, recent files, and vault to
     * a new location on disk, this state shows a one-time notice explaining the
     * change. The dialog is purely informational — there is no decline path.
     * After this, the state machine branches based on login and subscription status.
     */
    QSM.State {
        id: s5LegacyMigration

        signal done()

        onEntered: {
            if (Scrite.app.hasLegacyDataMovedRecently()) {
                let dlg = LegacyDataMigrationDialog.launch()
                if (dlg)
                    dlg.accepted.connect(() => s5LegacyMigration.done())
                else
                    Qt.callLater(() => s5LegacyMigration.done())
            } else {
                Qt.callLater(() => s5LegacyMigration.done())
            }
        }

        QSM.SignalTransition {
            signal: s5LegacyMigration.done; targetState: s6Splash
            guard: !Scrite.user.loggedIn
        }
        QSM.SignalTransition {
            signal: s5LegacyMigration.done; targetState: s7HomeOrFile
            guard: Scrite.user.loggedIn && Runtime.allowAppUsage
        }
        QSM.SignalTransition {
            signal: s5LegacyMigration.done; targetState: s8LoginRequired
            guard: Scrite.user.loggedIn && !Runtime.allowAppUsage
        }
    }

    /*
     * Entered when the user is not logged in. Shows the splash screen and waits
     * for it to close (either by user interaction or the 5-second auto-dismiss
     * timer). On Windows versions older than 10, an additional compatibility
     * warning is shown before proceeding. Exits to s7HomeOrFile or s8LoginRequired
     * depending on subscription entitlement at that point.
     */
    QSM.State {
        id: s6Splash

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
                            () => s6Splash.closed()
                        )
                    } else {
                        s6Splash.closed()
                    }
                })
            } else {
                Qt.callLater(() => s6Splash.closed())
            }
        }

        QSM.SignalTransition {
            signal: s6Splash.closed; targetState: s7HomeOrFile
            guard: Runtime.allowAppUsage
        }
        QSM.SignalTransition {
            signal: s6Splash.closed; targetState: s8LoginRequired
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
        id: s7HomeOrFile

        signal done()

        onEntered: {
            if (Scrite.fileNameToOpen === "") {
                if (!Scrite.app.maybeOpenAnonymously())
                    HomeScreen.launch()
            } else {
                Scrite.document.open(Scrite.fileNameToOpen)
            }
            Qt.callLater(() => s7HomeOrFile.done())
        }

        QSM.SignalTransition {
            signal: s7HomeOrFile.done; targetState: s3License
            guard: root._deferredLicenseCheck
        }
        QSM.SignalTransition {
            signal: s7HomeOrFile.done; targetState: sDone
            guard: !root._deferredLicenseCheck
        }
    }

    /*
     * Entered when the user is logged in but not entitled to use the app
     * (e.g. subscription lapsed). Launches the UserAccountDialog so the user
     * can resolve their account status.
     */
    QSM.State {
        id: s8LoginRequired

        signal done()

        onEntered: {
            UserAccountDialog.launch()
            Qt.callLater(() => s8LoginRequired.done())
        }

        QSM.SignalTransition { signal: s8LoginRequired.done; targetState: sDone }
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
     * Terminal success state. Entered after s7HomeOrFile or s8LoginRequired
     * completes, signalling that the init sequence finished normally.
     */
    QSM.FinalState {
        id: sDone

        onEntered: {
            Runtime.execLater(root, 2000, root._maybeOnboardUserSurvey)
        }
    }

    // ── Helpers ──────────────────────────────────────────────────────────────

    property bool _deferredLicenseCheck: false

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
