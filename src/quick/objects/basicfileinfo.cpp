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

#include "basicfileinfo.h"
#include "application.h"

BasicFileInfo::BasicFileInfo(QObject *parent) : QObject(parent) { }

BasicFileInfo::~BasicFileInfo() { }

void BasicFileInfo::setAbsoluteFilePath(const QString &val)
{
    if (val.isEmpty())
        this->setFileInfo(QFileInfo());
    else
        this->setFileInfo(QFileInfo(val));
}

void BasicFileInfo::setAbsolutePath(const QString &val)
{
    if (m_fileInfo == QFileInfo() || val.isEmpty() || val == m_fileInfo.absolutePath())
        return;

    const QString afp = val + QStringLiteral("/") + m_fileInfo.fileName();
    this->setFileInfo(QFileInfo(afp));
}

void BasicFileInfo::setFileName(const QString &val)
{
    if (m_fileInfo == QFileInfo() || val.isEmpty() || val == m_fileInfo.fileName())
        return;

    const QString afp = m_fileInfo.absolutePath() + QStringLiteral("/") + val;
    this->setFileInfo(QFileInfo(afp));
}

void BasicFileInfo::setSuffix(const QString &val)
{
    if (m_fileInfo == QFileInfo() || val.isEmpty() || val == m_fileInfo.suffix())
        return;

    QString ext = val.trimmed();
    if (ext.isEmpty())
        return;

    if (ext.startsWith(QStringLiteral(".")))
        ext = ext.mid(1);

    const QString afp = m_fileInfo.absolutePath() + QStringLiteral("/")
            + m_fileInfo.completeBaseName() + QStringLiteral(".") + ext;
    this->setFileInfo(QFileInfo(afp));
}

void BasicFileInfo::setBaseName(const QString &val)
{
    if (m_fileInfo == QFileInfo() || val.isEmpty() || val == m_fileInfo.completeBaseName())
        return;

    const QString afp = m_fileInfo.absolutePath() + QStringLiteral("/") + val + QStringLiteral(".")
            + m_fileInfo.suffix();

    this->setFileInfo(QFileInfo(afp));
}

void BasicFileInfo::setFileInfo(const QFileInfo &val)
{
#ifdef Q_OS_WIN
    const bool fileNamesAreCaseSensitive = false;
#else
    const bool fileNamesAreCaseSensitive = true;
#endif
    const QString currentFilePath =
            m_fileInfo.exists() ? m_fileInfo.canonicalFilePath() : m_fileInfo.absoluteFilePath();
    const QString newFilePath = val.exists() ? val.canonicalFilePath() : val.absoluteFilePath();
    const int comparison = currentFilePath.compare(
            newFilePath, fileNamesAreCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
    if (comparison == 0)
        return;

    m_fileInfo = val;
    emit fileInfoChanged();
}
