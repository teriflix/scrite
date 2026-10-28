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

import Scrite.App

Settings {
    property int characterPageTab: 0
    property int charactersPageTab: 0
    property int screenplayPageTab: 0
    property int sceneNotesPageTab: 0
    property int sceneSynopsisTabIndex: 0
    property int attributesCategoriesWidth: 200 // Width of the list of categories in the Attributes tab
    property int graphLayoutMaxIterations: 50000
    property int graphLayoutMaxTime: 1000
    property int fontPointSize: 0 // 0 means follow Runtime.idealFontMetrics

    property bool richTextNotesEnabled: true
    property bool showAllFormQuestions: true

    // The category that the Attributes tab selects, for the notes that have it
    property string attributesCategory: ""

    location: Platform.settingsLocation
    category: "Notebook"
}
