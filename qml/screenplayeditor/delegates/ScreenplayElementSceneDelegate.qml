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

import io.scrite.components 1.0

import "qrc:/qml/globals"
import "qrc:/qml/screenplayeditor/delegates/scenedelegate"

AbstractScreenplayElementSceneDelegate {
    id: root

    /**
      # Why two loaders, instead of one that swaps _lowResolution with _highResolution?

      ## Why two resolutions in the first place?

      First off, we need two resolutions because scene-delegate is a heavy one. It takes time
      and effort to construct the full content of this delegate, and we don't want that while
      the user is scrolling fast.

      The low resolution item quickly determines the height required for rendering the high
      resolution counterpart, and displays a blur scene image just to give an impression
      that some content will eventually show up.

      The high resolution item loads and renders the elaborate content required to furnish all
      functionality (scene heading, character list, tags, synopsis editor, side panel and the
      scene content editor with its syntax highlighter and everything).

      ## How does the loading actually happen?

      When the user is just hopping from one scene to the immediate next or previous scene,
      we simply construct the high-resolution content immediately.

      But during rapid scroll (while using trackpad flick, or by dragging the vertical scrollbar),
      we load the low resolution content, while simultaneously scheduling the load of high
      resolution content about a 500ms later. This interval is configurable in settings.ini btw.

      If the scene gets scrolled in and out within that time, the high-resolution content is not
      loaded at all. So, users will only see a blur-scene-image as rendered by the low-resolution
      content and soon ScreenplayElementListView will delete this whole thing from memory.

      But if the list-view stabilises such that some of the scenes continue to remain in visibility
      even beyond the 500ms timeout, then we swap the low-resolution content with the high-resolution
      one.

      ## Okay, if you are swapping - why not use a single Loader?

      Because we need the blur content to be visible even during those few ms it takes to load
      the high-resolution content. While its logically a swap, in practice it is hide after load.

      This also means that we have to suck up cost of two additional items (the container and
      one extra loader) in an already heavy delegate. But, that's a trade-off to keep the UI
      responsive.
      */
    content: Item {
        height: _highResLoader.active ? _highResLoader.height : _lowResLoader.height

        Loader {
            id: _lowResLoader

            z: 0
            width: parent.width

            active: !_highResLoader.active || !_highResLoader.item
            visible: active

            sourceComponent: LowResolutionSceneContent {
                  sceneDelegate: root
            }

            onLoaded: {
                Runtime.execLater(_highResLoader, Runtime.placeholderInterval, () => {
                                        _highResLoader.active = true
                                  })
            }
        }

        Loader {
            id: _highResLoader

            property bool firstLoadComplete: false

            z: 1
            width: parent.width

            active: !root.usePlaceholder || firstLoadComplete

            sourceComponent: HighResolutionSceneContent {
                  sceneDelegate: root
                  showSceneSidePanel: root.showSceneSidePanel
            }

            onLoaded: firstLoadComplete = true
        }
    }
}
