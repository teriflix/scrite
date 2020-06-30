/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Controls.Material 2.12

import Scrite 1.0

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
        app.execLater( menuLoader, 0, function() { menuLoader.active = false } )
    }

    function close() { dismiss() }

    onActiveChanged: {
        if(active === false)
            itemInitMode = "show"
    }

    onItemChanged: {
        if( item ) {
            if( app.verifyType(item, "QQuickMenu") ) {
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
        onVisibleChanged: menuLoader.dismiss()
    }

    property string itemInitMode: "show"
}
