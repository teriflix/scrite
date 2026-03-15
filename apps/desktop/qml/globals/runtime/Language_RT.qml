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

import QtQuick

import io.scrite.components

QtObject {
    readonly property AvailableLanguages available: LanguageEngine.availableLanguages
    readonly property LanguageEngine engine: LanguageEngine
    readonly property SupportedLanguages supported: LanguageEngine.supportedLanguages

    property int activeCode: supported.activeLanguageCode

    property var active: supported.activeLanguage
    property var activeTransliterationOption: active.valid ? active.preferredTransliterationOption() : undefined

    property bool activeTransliterationIsInApp: activeTransliterationOption && activeTransliterationOption.valid ? activeTransliterationOption.inApp : false

    property AbstractTransliterationEngine activeTransliterator: activeTransliterationOption && activeTransliterationOption.valid ? activeTransliterationOption.transliterator : null

    function setActiveCode(code) {
        if(activeCode === code)
            return

        supported.activeLanguageCode = code
        Scrite.document.displayFormat.activeLanguageCode = activeCode
        logActivity("language-activate", supported.activeLanguage)
    }

    function logActivity(activity, lang) {
        if(lang && Scrite.user.info.consentToActivityLog) {
            const txOption = lang.preferredTransliterationOption()
            const portableShortcut = Gui.portableShortcut(lang.keySequence)
            const shortcut = portableShortcut === "" ? "<no-shortcut>" : portableShortcut
            const details = [lang.name, shortcut, txOption.id, txOption.name, lang.font().family, Platform.typeString].join(";")
            Scrite.user.logActivity2(activity, details)
        }
    }
}
