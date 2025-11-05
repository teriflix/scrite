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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/qml/"
import "qrc:/qml/globals"
import "qrc:/qml/helpers"
import "qrc:/qml/controls"
import "qrc:/qml/notifications"

Item {
    id: root

    Loader {
        anchors.fill: parent

        /**
          Strange issue!!!!

          If the value of active is determined using the expression
          'width > 100 && Runtime.appFeatures.screenplay.enabled'
          then, the sourceComponent is instantiated twice for some reason.

          But, if its written the way it is below, then the sourceComponent is
          instantiated once.

          It is really really strange.
          */
        active: root.width > 100 && Runtime.appFeatures.screenplay.enabled

        sourceComponent: ScreenplayEditor { }
    }

    DisabledFeatureNotice {
        anchors.fill: parent

        visible: !Runtime.appFeatures.screenplay.enabled
        featureName: "Screenplay"
    }
}

