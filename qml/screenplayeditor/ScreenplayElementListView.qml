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

    required property real zoomLevel
    required property rect pageMargins

    property bool readOnly: Scrite.document.readOnly
    property ScreenplayAdapter screenplayAdapter: Runtime.screenplayAdapter

    ScrollBar.vertical: VclScrollBar { }

    model: screenplayAdapter

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

    delegate: DelegateChooser {
        role: "delegateKindRole"

        DelegateChoice {
            roleValue: "actBreak"

            ScreenplayActBreakDelegate {
                width: root.width

                readOnly: root.readOnly
                zoomLevel: root.zoomLevel
                pageMargins: root.pageMargins
            }
        }

        DelegateChoice {
            roleValue: "episodeBreak"

            ScreenplayEpisodeBreakDelegate {
                width: root.width

                readOnly: root.readOnly
                zoomLevel: root.zoomLevel
                pageMargins: root.pageMargins
            }
        }

        DelegateChoice {
            roleValue: "intervalBreak"

            ScreenplayIntervalBreakDelegate {
                width: root.width

                readOnly: root.readOnly
                zoomLevel: root.zoomLevel
                pageMargins: root.pageMargins
            }
        }

        DelegateChoice {
            roleValue: "omittedScene"

            OmittedScreenplayElementDelegate {
                width: root.width

                readOnly: root.readOnly
                zoomLevel: root.zoomLevel
                pageMargins: root.pageMargins
            }
        }

        DelegateChoice {
            roleValue: "scene"

            ScreenplayElementSceneDelegate {
                width: root.width

                readOnly: root.readOnly
                zoomLevel: root.zoomLevel
                pageMargins: root.pageMargins
            }
        }
    }

    QtObject {
        id: _private


    }
}
