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

#include "documentfilesystem.h"
#include "scritefileinfo.h"
#include "screenplay.h"

#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

ScriteFileInfo ScriteFileInfo::load(const QString &filePath)
{
    const QFileInfo fi(filePath);
    return load(fi);
}

ScriteFileInfo ScriteFileInfo::load(const QFileInfo &fileInfo)
{
    ScriteFileInfo empty;
    if (!fileInfo.exists() && fileInfo.suffix().toLower() != QLatin1String("scrite")
        && !fileInfo.isReadable())
        return empty;

    DocumentFileSystem dfs;
    if (!dfs.load(fileInfo.absoluteFilePath()))
        return empty;

    const QJsonDocument jsonDoc = QJsonDocument::fromJson(dfs.header());
    const QJsonObject docObj = jsonDoc.object();
    const QJsonObject screenplayObj = docObj.value("screenplay").toObject();
    const QJsonArray screenplayElementsArr = screenplayObj.value("elements").toArray();

    const QString documentId = docObj.value("documentId").toString();
    const QString title = screenplayObj.value("title").toString().trimmed();
    const QString subtitle = screenplayObj.value("subtitle").toString().trimmed();
    const QString author = screenplayObj.value("author").toString().trimmed();
    const QString logline = screenplayObj.value("logline").toString().trimmed();
    const int sceneCount = std::count_if(screenplayElementsArr.begin(), screenplayElementsArr.end(),
                                         [](const QJsonValue &item) {
                                             const QJsonObject &itemObj = item.toObject();
                                             return itemObj.value("elementType").toString()
                                                     == QStringLiteral("SceneElementType");
                                         });

    const QString coverPagePath = dfs.absolutePath(Screenplay::standardCoverPathPhotoPath());
    const QImage coverPageImage = QFile::exists(coverPagePath) ? QImage(coverPagePath) : QImage();

    return ScriteFileInfo { fileInfo.absoluteFilePath(),
                            fileInfo.fileName(),
                            fileInfo.completeBaseName(),
                            documentId,
                            title,
                            subtitle,
                            author,
                            logline,
                            coverPageImage,
                            sceneCount };
}
