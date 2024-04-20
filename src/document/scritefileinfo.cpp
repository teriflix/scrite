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

bool ScriteFileInfo::isValid() const
{
    return !filePath.isEmpty() && !fileName.isEmpty() && !baseFileName.isEmpty()
            && fileInfo.exists() && !documentId.isEmpty();
}

ScriteFileInfo ScriteFileInfo::quickLoad(const QString &filePath)
{
    const QFileInfo fi(filePath);
    return quickLoad(fi);
}

ScriteFileInfo ScriteFileInfo::quickLoad(const QFileInfo &fileInfo)
{
    ScriteFileInfo ret;
    if (!fileInfo.exists() && fileInfo.suffix().toLower() != QLatin1String("scrite")
        && !fileInfo.isReadable())
        return ret;

    ret.filePath = fileInfo.absoluteFilePath();
    ret.fileName = fileInfo.fileName();
    ret.baseFileName = fileInfo.completeBaseName();
    ret.fileSize = fileInfo.size();
    ret.fileInfo = fileInfo;

    return ret;
}

ScriteFileInfo ScriteFileInfo::load(const QString &filePath)
{
    const QFileInfo fi(filePath);
    return load(fi);
}

ScriteFileInfo ScriteFileInfo::load(const QFileInfo &fileInfo)
{
    ScriteFileInfo ret;
    if (!fileInfo.exists() && fileInfo.suffix().toLower() != QLatin1String("scrite")
        && !fileInfo.isReadable())
        return ret;

    DocumentFileSystem dfs;
    if (!dfs.load(fileInfo.absoluteFilePath()))
        return ret;

    const QJsonDocument jsonDoc = QJsonDocument::fromJson(dfs.header());
    const QJsonObject docObj = jsonDoc.object();
    const QJsonObject screenplayObj = docObj.value("screenplay").toObject();
    const QJsonArray screenplayElementsArr = screenplayObj.value("elements").toArray();

    ret.filePath = fileInfo.absoluteFilePath();
    ret.fileName = fileInfo.fileName();
    ret.baseFileName = fileInfo.completeBaseName();
    ret.fileSize = fileInfo.size();
    ret.fileInfo = fileInfo;
    ret.documentId = docObj.value("documentId").toString();
    ret.title = screenplayObj.value("title").toString().trimmed();
    ret.subtitle = screenplayObj.value("subtitle").toString().trimmed();
    ret.author = screenplayObj.value("author").toString().trimmed();
    ret.logline = screenplayObj.value("logline").toString().trimmed();
    ret.version = screenplayObj.value("version").toString().trimmed();
    ret.sceneCount = std::count_if(screenplayElementsArr.begin(), screenplayElementsArr.end(),
                                   [](const QJsonValue &item) {
                                       const QJsonObject &itemObj = item.toObject();
                                       return itemObj.value("elementType").toString()
                                               == QStringLiteral("SceneElementType");
                                   });

    const QString coverPagePath = dfs.absolutePath(Screenplay::standardCoverPathPhotoPath());
    ret.coverPageImage = QFile::exists(coverPagePath)
            ? QImage(coverPagePath).scaled(512, 512, Qt::KeepAspectRatio, Qt::SmoothTransformation)
            : QImage();
    ret.hasCoverPage = !ret.coverPageImage.isNull();

    return ret;
}
