import Scrite 1.0

Automation {
    id: automation

    // Capture the splash-screen
    PauseStep { duration: 1750 }
    WindowCapture {
        path: automation.pathOf(automation.fromUrl(automationScript))
        fileName: automation.fileNameOf(automation.fromUrl(automationScript), "jpg")
        format: WindowCapture.JPGFormat
        forceCounterInFileName: false
        window: qmlWindow
        replaceExistingFile: true
        maxImageSize: Qt.size(1920, 1080)
    }
}

