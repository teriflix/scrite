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

pragma Singleton

import QtQuick 2.15

import io.scrite.components 1.0

QtObject {
    readonly property AppFeature structure: AppFeature {
        feature: Scrite.StructureFeature
    }

    readonly property AppFeature notebook: AppFeature {
        feature: Scrite.NotebookFeature
    }

    readonly property AppFeature scrited: AppFeature {
        feature: Scrite.ScritedFeature
    }

    readonly property AppFeature characterRelationshipGraph: AppFeature {
        feature: Scrite.RelationshipGraphFeature
    }

    readonly property AppFeature watermark: AppFeature {
        feature: Scrite.WatermarkFeature
    }

    readonly property AppFeature importer: AppFeature {
        feature: Scrite.ImportFeature
    }

    readonly property AppFeature exporter: AppFeature {
        feature: Scrite.ExportFeature
    }

    readonly property AppFeature scriptalay: AppFeature {
        feature: Scrite.ScriptalayFeature
    }

    readonly property AppFeature templates: AppFeature {
        feature: Scrite.TemplateFeature
    }
}
