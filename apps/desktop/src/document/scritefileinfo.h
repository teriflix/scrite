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

#ifndef SCRITEFILEINFO_H
#define SCRITEFILEINFO_H

#include <QObject>
#include <QString>
#include <QImage>
#include <QFileInfo>
#include <QQmlEngine>

struct ScriteFileInfo
{
    Q_GADGET
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    // Test whether a file info instance is valid or not
    Q_INVOKABLE bool isValid() const;

    // Absolute file path
    // clang-format off
    Q_PROPERTY(QString filePath
               MEMBER filePath)
    // clang-format on
    QString filePath;

    // Just file name
    // clang-format off
    Q_PROPERTY(QString fileName
               MEMBER fileName)
    // clang-format on
    QString fileName;

    // Base file name
    // clang-format off
    Q_PROPERTY(QString baseFileName
               MEMBER baseFileName)
    // clang-format on
    QString baseFileName;

    // File size in bytes
    // clang-format off
    Q_PROPERTY(qint64 fileSize
               MEMBER fileSize)
    // clang-format on
    qint64 fileSize = 0;

    // Qt's file info
    // clang-format off
    Q_PROPERTY(QFileInfo fileInfo
               MEMBER fileInfo)
    // clang-format on
    QFileInfo fileInfo;

    // Document ID
    // clang-format off
    Q_PROPERTY(QString documentId
               MEMBER documentId)
    // clang-format on
    QString documentId;

    // Screenplay title (if specified)
    // clang-format off
    Q_PROPERTY(QString title
               MEMBER title)
    // clang-format on
    QString title;

    // Screenplay subtitle (if specified)
    // clang-format off
    Q_PROPERTY(QString subtitle
               MEMBER subtitle)
    // clang-format on
    QString subtitle;

    // Screenplay author (if specified)
    // clang-format off
    Q_PROPERTY(QString author
               MEMBER author)
    // clang-format on
    QString author;

    // Screenplay logline (if specified)
    // clang-format off
    Q_PROPERTY(QString logline
               MEMBER logline)
    // clang-format on
    QString logline;

    // Screenplay version (if specified)
    // clang-format off
    Q_PROPERTY(QString version
               MEMBER version)
    // clang-format on
    QString version;

    // Screenplay cover-page-image if available (if specified)
    // clang-format off
    Q_PROPERTY(QImage coverPageImage
               MEMBER coverPageImage)
    // clang-format on
    QImage coverPageImage;

    // clang-format off
    Q_PROPERTY(bool hasCoverPage
               MEMBER hasCoverPage)
    // clang-format on
    bool hasCoverPage = false;

    // Number of scenes in the screenplay (not structure)
    // clang-format off
    Q_PROPERTY(int sceneCount
               MEMBER sceneCount)
    // clang-format on
    int sceneCount = 0;

    // Static method to help load a file-info from a given file
    static ScriteFileInfo quickLoad(const QString &filePath);
    static ScriteFileInfo quickLoad(const QFileInfo &filePath);

    static ScriteFileInfo load(const QString &filePath);
    static ScriteFileInfo load(const QFileInfo &fileInfo);

    bool operator==(const ScriteFileInfo &other) const { return this->filePath == other.filePath; }
};

#endif // SCRITEFILEINFO_H
