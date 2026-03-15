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

import QtCore

import io.scrite.components

Settings {
    category: "Screenplay Editor"
    location: Platform.settingsLocation

    ObjectRegister.name: "screenplayEditorSettings"

    property var zoomLevelModifiers: { "tab0": 0, "tab1": 0, "tab2": 0, "tab3": 0 }

    property int commentsPanelTabIndex: 1
    property int embeddedEditorZoomValue: -1
    property int lastLanguageRefreshNoticeBoxTimestamp: 0
    property int lastSpellCheckRefreshNoticeBoxTimestamp: 0
    property int longSceneWordTreshold: 150
    property int mainEditorZoomValue: -1
    property int placeholderInterval: 250 // ms after which placeholder is swapped to content in delegates
    property int sceneSidePanelActiveTab: 0
    property int slpSynopsisLineCount: 2
    property int jumpToSceneFilterMode: 0

    property bool allowDiacriticEditing: true
    property bool allowSelectedTextTranslation: false
    property bool allowTaggingOfScenes: false
    property bool applyUserDefinedLanguageFonts: true
    property bool autoAdjustEditorWidthInScreenplayEditor: true
    property bool autoSelectSceneUnderMouse: true
    property bool captureInvisibleCharacters: false
    property bool copyAsFountain: true
    property bool copyFountainUsingStrictSyntax: true
    property bool copyFountainWithEmphasis: true
    property bool displayAddSceneBreakButtons: true
    property bool displayEmptyTitleCard: true
    property bool displayIndexCardFields: true
    property bool displayRuler: false
    property bool displaySceneCharacters: false
    property bool displaySceneComments: false
    property bool displaySceneSynopsis: false
    property bool enableAutoCapitalizeSentences: true
    property bool enableAutoPolishParagraphs: true // for automatically adding/removing CONT'D where appropriate
    property bool enableSpellCheck: true // Since this is now fixed: https://github.com/teriflix/scrite/issues/138
    property bool focusCursorOnSceneHeadingInNewScenes: false
    property bool highlightCurrentLine: true
    property bool includeTitlePageInPreview: true
    property bool languageInputPreferenceChecked: false
    property bool longSceneWarningEnabled: true
    property bool markupToolsDockVisible: false
    property bool optimiseScrolling: false
    property bool pasteAfterResolvingEmphasis: true
    property bool pasteAsFountain: true
    property bool pasteByLinkingScenesWhenPossible: true
    property bool pasteByMergingAdjacentElements: true
    property bool refreshButtonInStatsReportAnimationDone: false
    property bool restartEpisodeScenesAtOne: false // If set, each episode starts with scene number 1
    property bool sceneSidePanelOpen: false
    property bool screenplayEditorAddButtonsAnimationShown: false
    property bool showLanguageRefreshNoticeBox: true
    property bool showLoglineEditor: false
    property bool showSpellCheckRefreshNoticeBox: true
    property bool singleClickAutoComplete: true

    property real sidePanelWidth: 400
    property real spaceBetweenScenes: 0
}
