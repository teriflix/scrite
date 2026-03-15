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

import QtCore

import io.scrite.components

Settings {
    property string dayZero
    property string tipsShown: ""

    function daysSinceZero() {
        const today = new Date()
        const dzero = dayZero === "" ? today : new Date(dayZero + "Z")
        const days = Math.floor((today.getTime() - dzero.getTime()) / (24*60*60*1000))
        return days
    }

    function isTipShown(val) {
        const ts = tipsShown.split(",")
        return ts.indexOf(val) >= 0
    }

    function markTipAsShown(val) {
        let ts = tipsShown.length > 0 ? tipsShown.split(",") : []
        if(ts.indexOf(val) < 0)
            ts.push(val)
        tipsShown = ts.join(",")
    }

    location: Platform.settingsLocation
    category: "Help"
}
