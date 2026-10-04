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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import Scrite.App

import "../../globals"
import "../../controls"
import "../../helpers"

// Each element in pagesModel describes one page, using these roles:
//   title            - title shown in the page list (required)
//   qmlSource        - URL of the page QML file (required)
//   fillHeight       - if true, the page is sized to the available height (optional)
//   noContentSpacing - if true, there is no spacing between the page list and the page (optional)
// Extensions can add their own pages by appending elements to pagesModel.

PageView {
    id: root

    required property ListModel pagesModel

    readonly property var currentPageInfo: pagesModel.count > currentIndex && currentIndex >= 0 ? pagesModel.get(currentIndex) : null

    pagesArray: _private.pageTitles
    currentIndex: 0
    pageContentSpacing: currentPageInfo && currentPageInfo.noContentSpacing === true ? 0 : 20

    pageContent: Loader {
        id: _pageLoader

        width: root.availablePageContentWidth
        source: root.currentPageInfo ? root.currentPageInfo.qmlSource : ""

        // Pages that don't fill height offer their own height, which lets PageView scroll them
        Binding {
            target: _pageLoader.item
            property: "height"
            value: root.availablePageContentHeight
            when: _pageLoader.item !== null && root.currentPageInfo !== null && root.currentPageInfo.fillHeight === true
        }
    }

    Connections {
        target: root.pagesModel

        function onCountChanged() {
            Qt.callLater(_private.evaluatePageTitles)
        }
    }

    Component.onCompleted: _private.evaluatePageTitles()

    QtObject {
        id: _private

        property var pageTitles: []

        function evaluatePageTitles() {
            let titles = []
            for(let i=0; i<root.pagesModel.count; i++) {
                titles.push(root.pagesModel.get(i).title)
            }
            pageTitles = titles
        }
    }
}
