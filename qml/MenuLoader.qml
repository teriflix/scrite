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
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import io.scrite.components 1.0
import "../js/utils.js" as Utils

Loader {
    id: menuLoader
    active: false

    property Component menu
    sourceComponent: menu

    function show() {
        if(!enabled)
            return
        itemInitMode = "show"
        active = true
    }

    function popup() {
        if(!enabled)
            return
        itemInitMode = "popup"
        active = true
    }

    function dismiss() {
        if(item)
            item.dismiss()
        Utils.execLater( menuLoader, 0, function() { menuLoader.active = false } )
    }

    function close() { dismiss() }

    onActiveChanged: {
        if(active === false)
            itemInitMode = "show"
    }

    onItemChanged: {
        if( item ) {
            if( Scrite.app.verifyType(item, "QQuickMenu") ) {
                item.enabled = false
                if(itemInitMode === "popup")
                    item.popup()
                else if(itemInitMode === "show")
                    item.visible = true
            } else
                console.log("Using MenuLoader for anything other than Menu {} item is prohibited.")
        }
    }

    DelayedPropertyBinder {
        initial: false
        set: parent.item ? true : false
        delay: 100
        onGetChanged: {
            if(parent.item)
                parent.item.enabled = get
        }
    }

    Connections {
        target: menuLoader.item
        ignoreUnknownSignals: true
        function onVisibleChanged() { menuLoader.dismiss() }
    }

    property string itemInitMode: "show"
}
