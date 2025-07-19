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

import "qrc:/js/utils.js" as Utils

Loader {
    id: root

    property Component menu

    active: false
    sourceComponent: menu

    function show() {
        if(!enabled)
            return

        _private.itemInitMode = "show"
        active = true
    }

    function popup() {
        if(!enabled)
            return

        _private.itemInitMode = "popup"
        active = true
    }

    function dismiss() {
        if(item)
            item.dismiss()

        Utils.execLater( root, 0, function() { root.active = false } )
    }

    function close() { dismiss() }

    onActiveChanged: {
        if(active === false)
            _private.itemInitMode = "show"
    }

    onItemChanged: {
        if( item ) {
            if( Scrite.app.verifyType(item, "QQuickMenu") ) {
                item.enabled = false
                if(_private.itemInitMode === "popup")
                    item.popup()
                else if(_private.itemInitMode === "show")
                    item.visible = true
            } else
                console.log("Using MenuLoader for anything other than Menu {} item is prohibited.")
        }
    }

    DelayedPropertyBinder {
        set: parent.item ? true : false
        delay: 100
        initial: false

        onGetChanged: {
            if(parent.item)
                parent.item.enabled = get
        }
    }

    Connections {
        target: root.item
        ignoreUnknownSignals: true

        function onVisibleChanged() { root.dismiss() }
    }

    QtObject {
        id: _private

        property string itemInitMode: "show"
    }
}
