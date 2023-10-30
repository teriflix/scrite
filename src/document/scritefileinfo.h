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

#ifndef SCRITEFILEINFO_H
#define SCRITEFILEINFO_H

#include <QObject>
#include <QString>
#include <QImage>
#include <QFileInfo>

struct ScriteFileInfo
{
    Q_GADGET

public:
    // Test whether a file info instance is valid or not
    Q_INVOKABLE bool isValid() const
    {
        return !filePath.isEmpty() && !fileName.isEmpty() && !baseFileName.isEmpty()
                && !documentId.isEmpty();
    }

    // Absolute file path
    Q_PROPERTY(QString filePath MEMBER filePath)
    QString filePath;

    // Just file name
    Q_PROPERTY(QString fileName MEMBER fileName)
    QString fileName;

    // Base file name
    Q_PROPERTY(QString baseFileName MEMBER baseFileName)
    QString baseFileName;

    // Document ID
    Q_PROPERTY(QString documentId MEMBER documentId)
    QString documentId;

    // Screenplay title (if specified)
    Q_PROPERTY(QString title MEMBER title)
    QString title;

    // Screenplay subtitle (if specified)
    Q_PROPERTY(QString subtitle MEMBER subtitle)
    QString subtitle;

    // Screenplay author (if specified)
    Q_PROPERTY(QString author MEMBER author)
    QString author;

    // Screenplay logline (if specified)
    Q_PROPERTY(QString logline MEMBER logline)
    QString logline;

    // Screenplay cover-page-image if available (if specified)
    Q_PROPERTY(QImage coverPageImage MEMBER coverPageImage)
    QImage coverPageImage;

    // Number of scenes in the screenplay (not structure)
    Q_PROPERTY(int sceneCount MEMBER sceneCount)
    int sceneCount = 0;

    // Static method to help load a file-info from a given file
    static ScriteFileInfo load(const QString &filePath);
    static ScriteFileInfo load(const QFileInfo &fileInfo);

    bool operator==(const ScriteFileInfo &other) const { return this->filePath == other.filePath; }
};

#endif // SCRITEFILEINFO_H
