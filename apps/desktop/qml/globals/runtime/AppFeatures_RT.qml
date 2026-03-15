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

import QtQuick

import io.scrite.components

QtObject {
    readonly property AppFeature screenplay: AppFeature {
        feature: Scrite.ScreenplayFeature
    }

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

    readonly property AppFeature emailSupport: AppFeature {
        featureName: "support/email"
    }
}
