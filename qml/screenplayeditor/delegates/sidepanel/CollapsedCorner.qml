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

import QtQml 2.15
import QtQuick 2.15

import io.scrite.components 1.0


import "qrc:/qml/globals"
import "qrc:/qml/helpers"

Item {
    id: root

    required property Scene scene

    Image {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: 1.5

        width: Runtime.iconImageSize * 0.5
        height: Runtime.iconImageSize * 0.5

        y: 2
        smooth: true
        mipmap: true
        opacity: 0.75

        source: {
            if(_private.hasSceneComments)
                return "qrc:/icons/content/note.png"

            if(_private.sceneFeaturedImage)
                return "qrc:/icons/filetype/photo.png"

            if(_private.hasIndexCardFields)
                return "qrc:/icons/content/form.png"

            return ""
        }
    }

    QtObject {
        id: _private

        property bool hasSceneComments: sceneComments !== ""
        property bool hasIndexCardFields: root.scene.indexCardFieldValues.length > 0

        property string sceneComments: root.scene.comments

        property Attachment sceneFeaturedImage: sceneFeaturedAttachment && sceneFeaturedAttachment.type === Attachment.Photo ? sceneFeaturedAttachment : null
        property Attachment sceneFeaturedAttachment: sceneAttachments.featuredAttachment

        property Attachments sceneAttachments: root.scene.attachments
    }
}
