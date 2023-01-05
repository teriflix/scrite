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

QtObject {
    property string tipName
    property var helpTip: Scrite.user.helpTips[tipName]
    property bool tipShown: helpNotificationSettings.isTipShown(tipName)
    property bool enabled: true

    Notification.title: helpTip ? helpTip.title : ""
    Notification.image: helpTip ? helpTip.image.url : ""
    Notification.active: enabled && helpTip && !tipShown
    Notification.text: helpTip ? helpTip.text : ""
    Notification.autoClose: false
    Notification.closeOnButtonClick: false
    Notification.buttons: {
        var ret = []
        if(helpTip)
            helpTip.buttons.forEach( (item) => {
                                                ret.push(item.text)
                                             })
        return ret
    }
    Notification.onImageClicked: {
        if(helpTip) {
            if(helpTip.image.action === "$dismiss")
                markTipAsShown()
            else
                Qt.openUrlExternally(helpTip.image.action)
        }
    }

    Notification.onButtonClicked: (buttonIndex) => {
                                      if(helpTip) {
                                          const button = helpTip.buttons[buttonIndex]
                                          if(button.action === "$dismiss")
                                            markTipAsShown()
                                          else
                                            Qt.openUrlExternally(button.action)
                                      } else
                                        markTipAsShown()
                                  }

    function markTipAsShown() {
        helpNotificationSettings.markTipAsShown(tipName)
    }
}
