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

#include "documentfilesystementry.h"
#include "documentfilesystem.h"
#include "scritedocument.h"

#include <QDir>

static const QString extensionsFolder = QStringLiteral("extensions/");

DocumentFileSystemEntry::DocumentFileSystemEntry(QObject *parent) : QObject(parent)
{
    ScriteDocument *doc = ScriteDocument::instance();
    connect(doc->fileSystem(), &DocumentFileSystem::auction, this,
            &DocumentFileSystemEntry::onDfsAuction);
    connect(doc, &ScriteDocument::justReset, this,
            &DocumentFileSystemEntry::absoluteFilePathChanged);
    connect(doc, &ScriteDocument::justLoaded, this,
            &DocumentFileSystemEntry::absoluteFilePathChanged);
    connect(doc, &ScriteDocument::aboutToSave, this, &DocumentFileSystemEntry::aboutToSave);
    connect(doc, &ScriteDocument::justSaved, this, &DocumentFileSystemEntry::justSaved);
    connect(doc, &ScriteDocument::aboutToReset, this, &DocumentFileSystemEntry::aboutToReset);
}

DocumentFileSystemEntry::~DocumentFileSystemEntry() { }

void DocumentFileSystemEntry::setPath(const QString &val)
{
    QString val2 = QDir::cleanPath(QDir::fromNativeSeparators(val.trimmed()));
    if (val2 == QStringLiteral(".") || QDir::isAbsolutePath(val2) || val2 == QStringLiteral("..")
        || val2.startsWith(QStringLiteral("../")))
        val2.clear();

    if (m_path == val2)
        return;

    m_path = val2;
    emit pathChanged();
    emit absoluteFilePathChanged();
}

QString DocumentFileSystemEntry::absoluteFilePath() const
{
    if (m_path.isEmpty())
        return QString();

    return ScriteDocument::instance()->fileSystem()->absolutePath(this->dfsPath(), true);
}

bool DocumentFileSystemEntry::exists() const
{
    if (m_path.isEmpty())
        return false;

    return ScriteDocument::instance()->fileSystem()->exists(this->dfsPath());
}

bool DocumentFileSystemEntry::remove()
{
    if (m_path.isEmpty())
        return false;

    return ScriteDocument::instance()->fileSystem()->remove(this->dfsPath());
}

QString DocumentFileSystemEntry::dfsPath() const
{
    return extensionsFolder + m_path;
}

void DocumentFileSystemEntry::onDfsAuction(const QString &filePath, int *claims)
{
    if (!m_path.isEmpty() && filePath == this->dfsPath())
        *claims = *claims + 1;
}
