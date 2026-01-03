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

import QtQuick 2.15

import io.scrite.components 1.0

import "qrc:/qml/globals"

QtObject {
    id: root

    readonly property string version: "_v2"

    property bool enabled: _private.helpTip !== undefined

    property string tipName

    Notification.active: enabled && _private.helpTip !== undefined && !_private.tipShown

    Notification.image: _private.helpTip !== undefined ? _private.helpTip.image.url : ""

    Notification.text: _private.helpTip !== undefined  ? _private.helpTip.text : ""
    Notification.title: _private.helpTip !== undefined  ? _private.helpTip.title : ""

    Notification.autoClose: false
    Notification.closeOnButtonClick: false
    Notification.buttons: {
        var ret = []
        if(_private.helpTip !== undefined)
            _private.helpTip.buttons.forEach( (item) => {
                                                ret.push(item.text)
                                             })
        return ret
    }

    Notification.onImageClicked: {
        if(_private.helpTip !== undefined) {
            if(_private.helpTip.image.action === "$dismiss") {
                markTipAsShown()
            } else {
                Qt.openUrlExternally(_private.helpTip.image.action)
            }
        }
    }

    Notification.onButtonClicked: (buttonIndex) => {
                                      if(_private.helpTip !== undefined) {
                                          const button = _private.helpTip.buttons[buttonIndex]
                                          if(button.action === "$dismiss") {
                                              markTipAsShown()
                                          } else {
                                              Qt.openUrlExternally(button.action)
                                          }
                                      } else {
                                          markTipAsShown()
                                      }
                                  }

    function markTipAsShown() {
        Runtime.helpNotificationSettings.markTipAsShown(_private.resolvedTipName)
    }

    readonly property QtObject _private: QtObject {
        property var helpTip: Runtime.helpTips === undefined || resolvedTipName === "" ? undefined : Runtime.helpTips[resolvedTipName]

        property bool tipShown: Runtime.helpNotificationSettings.isTipShown(resolvedTipName)

        property string resolvedTipName: root.tipName + root.version
    }
}
