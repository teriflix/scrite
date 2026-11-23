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

import Scrite.App

/**
  Declares one tab of a page that derives from AbstractNotebookTabPage. Scrite declares its own tabs
  as children of the page. Extensions offer theirs through a FeatureProvider, whose hostUri is the
  hostUri of the page.

  The content of the tab is the one item declared inside it, which is made when the tab is shown for
  the first time, and not before.
  */
QtObject { 
    // Tabs are sorted by order, left to right. Scrite spaces its own tabs 100 apart, so that tabs
    // offered by extensions can go in between.
    property int order: 0

    property bool visible: true

    // The name of this tab among the tabs of its page. Stable, because it is what the page remembers
    // as its current tab. Use the form "<Scrite|Extension>.NotebookTab.<Page>.<Name>Tab".
    property string uri
    property string title

    // Optional icon shown before the title in the tab bar. Leave empty for a tab without an icon.
    property url iconSource

    default property Component component

    ObjectRegister.name: "Scrite.App.Notebook.NotebookPageTab"
}
