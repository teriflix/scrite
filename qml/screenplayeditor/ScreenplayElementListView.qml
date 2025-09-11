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
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import Qt.labs.qmlmodels 1.0

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/globals"
import "qrc:/qml/controls"
import "qrc:/qml/helpers"
import "qrc:/qml/screenplayeditor"
import "qrc:/qml/screenplayeditor/delegates"

ListView {
    id: root

    required property var pageMargins
    required property bool readOnly
    required property real zoomLevel

    required property ScreenplayAdapter screenplayAdapter

    model: screenplayAdapter
    currentIndex: screenplayAdapter.currentIndex

    highlightMoveDuration: 0
    highlightResizeDuration: 0
    highlightFollowsCurrentItem: true

    header: ScreenplayElementListViewHeader {
        width: root.width

        readOnly: root.readOnly
        zoomLevel: root.zoomLevel
        pageMargins: root.pageMargins
        screenplayAdapter: root.screenplayAdapter
    }

    footer: ScreenplayElementListViewFooter {
        width: root.width

        readOnly: root.readOnly
        zoomLevel: root.zoomLevel
        pageMargins: root.pageMargins
        screenplayAdapter: root.screenplayAdapter
    }

    /**
      Why are we doing all this circus instead of using DelegateChooser?

      Initially, I used DelegateChooser with role as "delegateKind", and one DelegateChoice for each
      potential value of delegateKind. But it breaks down when the value of the delegateKind changes
      for a row in the model, becasue DelegateChooser doesn't recreate a delegate from a new choice
      whenever that happens. That's obviously not what we want.
      */
    delegate: Loader {
        id: _delegateLoader

        required property int index
        required property int screenplayElementType
        required property int breakType
        required property string sceneID
        required property string delegateKind
        required property ScreenplayElement screenplayElement

        width: root.width

        sourceComponent: _private.pickDelegateComponent(delegateKind)

        onStatusChanged: {
            if(status === Loader.Loading)
                Scrite.app.resetObjectProperty(_delegateLoader, "height")
        }
    }

    QtObject {
        id: _private

        readonly property Component actBreakDelegate: ScreenplayActBreakDelegate {
            readonly property Loader delegateLoader: parent

            readOnly: root.readOnly
            zoomLevel: root.zoomLevel
            pageMargins: root.pageMargins

            index: delegateLoader.index
            sceneID: delegateLoader.sceneID
            breakType: delegateLoader.breakType
            screenplayElement: delegateLoader.screenplayElement
            screenplayElementType: delegateLoader.screenplayElementType
        }

        readonly property Component episodeBreakDelegate: ScreenplayEpisodeBreakDelegate {
            readonly property Loader delegateLoader: parent

            readOnly: root.readOnly
            zoomLevel: root.zoomLevel
            pageMargins: root.pageMargins

            index: delegateLoader.index
            sceneID: delegateLoader.sceneID
            breakType: delegateLoader.breakType
            screenplayElement: delegateLoader.screenplayElement
            screenplayElementType: delegateLoader.screenplayElementType
        }

        readonly property Component intervalBreakDelegate: ScreenplayIntervalBreakDelegate {
            readonly property Loader delegateLoader: parent

            readOnly: root.readOnly
            zoomLevel: root.zoomLevel
            pageMargins: root.pageMargins

            index: delegateLoader.index
            sceneID: delegateLoader.sceneID
            breakType: delegateLoader.breakType
            screenplayElement: delegateLoader.screenplayElement
            screenplayElementType: delegateLoader.screenplayElementType
        }

        readonly property Component omittedSceneDelegate: OmittedScreenplayElementDelegate {
            readonly property Loader delegateLoader: parent

            readOnly: root.readOnly
            zoomLevel: root.zoomLevel
            pageMargins: root.pageMargins

            index: delegateLoader.index
            sceneID: delegateLoader.sceneID
            breakType: delegateLoader.breakType
            screenplayElement: delegateLoader.screenplayElement
            screenplayElementType: delegateLoader.screenplayElementType
        }

        readonly property Component sceneDelegate: ScreenplayElementSceneDelegate {
            readonly property Loader delegateLoader: parent

            readOnly: root.readOnly
            zoomLevel: root.zoomLevel
            pageMargins: root.pageMargins

            index: delegateLoader.index
            sceneID: delegateLoader.sceneID
            breakType: delegateLoader.breakType
            screenplayElement: delegateLoader.screenplayElement
            screenplayElementType: delegateLoader.screenplayElementType
        }

        function pickDelegateComponent(delegateKind) {
            switch(delegateKind) {
            case "scene": return sceneDelegate
            case "actBreak": return actBreakDelegate;
            case "omittedScene": return omittedSceneDelegate;
            case "episodeBreak": return episodeBreakDelegate;
            case "invervalBreak": return intervalBreakDelegate;
            }
            return null
        }
    }
}
