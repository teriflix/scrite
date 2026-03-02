/*
 * kspell_hunspellclient.cpp
 *
 * SPDX-FileCopyrightText: 2009 Montel Laurent <montel@kde.org>
 * SPDX-FileCopyrightText: 2025 Harald Sitter <sitter@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "hunspellclient.h"
#include "hunspelldebug.h"
#include "hunspelldict.h"

#include <QDir>
#include <QStandardPaths>
#include <QString>
#include <QVersionNumber>

using namespace Sonnet;
using namespace Qt::StringLiterals;

namespace
{

[[nodiscard]] QStringList flatpakSources()
{
    QMap<QVersionNumber, QString> versionedPaths;
    // TODO: could load all paths from libflatpak. For now we hardcode the known ones.
    for (const auto &flatpakPath :
         {u"/var/lib/flatpak/runtime/org.kde.Platform.Locale/"_s,
          QStandardPaths::locate(QStandardPaths::GenericDataLocation, u"flatpak/runtime/org.kde.Platform.Locale"_s, QStandardPaths::LocateDirectory)}) {
        for (const auto &architectureEntry : QDirListing(flatpakPath, QDirListing::IteratorFlag::DirsOnly)) {
            const auto &architecturePath = architectureEntry.filePath();
            for (const auto &versionEntry : QDirListing(architecturePath, QDirListing::IteratorFlag::DirsOnly)) {
                const auto filesPath = versionEntry.filePath() + "/active/files/"_L1;
                if (!QFileInfo::exists(filesPath)) {
                    continue;
                }

                versionedPaths.insert(QVersionNumber::fromString(versionEntry.fileName()), filesPath);
            }
        }
    }

    if (versionedPaths.isEmpty()) {
        qCDebug(SONNET_HUNSPELL) << "No flatpak hunspell location found";
        return {};
    }

    const QString latestActiveVersionPath = versionedPaths.values().last();
    qCDebug(SONNET_HUNSPELL) << "Found flatpak location" << latestActiveVersionPath;

    QStringList sources;
    for (const auto &parentLanguageEntry : QDirListing(latestActiveVersionPath, QDirListing::IteratorFlag::DirsOnly)) {
        const auto &parentLanguagePath = parentLanguageEntry.filePath() + "/share/"_L1;
        for (const auto &languageEntry : QDirListing(parentLanguagePath, QDirListing::IteratorFlag::DirsOnly)) {
            sources.append(languageEntry.filePath() + "/hunspell/"_L1);
        }
    }
    return sources;
}

} // namespace

HunspellClient::HunspellClient(QObject *parent)
    : Client(parent)
{
    qCDebug(SONNET_HUNSPELL) << " HunspellClient::HunspellClient";

    QStringList dirList;

    auto maybeAddPath = [&dirList](const QString &path) {
        if (QFileInfo::exists(path)) {
            dirList.append(path);

            QDir dir(path);
            for (const QString &subDir : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
                dirList.append(dir.absoluteFilePath(subDir));
            }
        }
    };

    const auto genericPaths = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("hunspell"), QStandardPaths::LocateDirectory);

    for (const auto &p : genericPaths) {
        maybeAddPath(p);
    }

    const auto appLocalPaths = QStandardPaths::locateAll(QStandardPaths::AppLocalDataLocation, QStringLiteral("hunspell"), QStandardPaths::LocateDirectory);

    for (const auto &p : appLocalPaths) {
        maybeAddPath(p);
    }

#ifdef Q_OS_WIN
    maybeAddPath(QStringLiteral(SONNET_INSTALL_PREFIX "/bin/data/hunspell/"));
#else
    maybeAddPath(QStringLiteral("/System/Library/Spelling"));
    maybeAddPath(QStringLiteral("/usr/share/hunspell/"));
    maybeAddPath(QStringLiteral("/usr/share/myspell/"));
    // Load supplementary dictionaries from our flatpak platform if available
    for (const auto &flatpakSource : flatpakSources()) {
        maybeAddPath(flatpakSource);
    }
#endif

    for (const QString &dirString : dirList) {
        QDir dir(dirString);
        const QList<QFileInfo> dicts = dir.entryInfoList({QStringLiteral("*.aff")}, QDir::Files);
        for (const QFileInfo &dict : dicts) {
            const QString language = dict.baseName();
            if (dict.isSymbolicLink()) {
                const QFileInfo actualDict(dict.canonicalFilePath());
                const QString alias = actualDict.baseName();
                if (language != alias) {
                    qCDebug(SONNET_HUNSPELL) << "Found alias" << language << "->" << alias;
                    m_languageAliases.insert(language, alias);

                    if (!m_languagePaths.contains(language)) {
                        m_languagePaths.insert(alias, actualDict.canonicalPath());
                    }

                    continue;
                }
            }
            m_languagePaths.insert(language, dict.canonicalPath());
        }
    }
}

HunspellClient::~HunspellClient()
{
}

SpellerPlugin *HunspellClient::createSpeller(const QString &inputLang)
{
    QString language = inputLang;
    if (m_languageAliases.contains(language)) {
        qCDebug(SONNET_HUNSPELL) << "Using alias" << m_languageAliases.value(language) << "for" << language;
        language = m_languageAliases.value(language);
    }
    std::shared_ptr<Hunspell> hunspell = m_hunspellCache.value(language).lock();
    if (!hunspell) {
        hunspell = HunspellDict::createHunspell(language, m_languagePaths.value(language));
        m_hunspellCache.insert(language, hunspell);
    }
    qCDebug(SONNET_HUNSPELL) << " SpellerPlugin *HunspellClient::createSpeller(const QString &language) ;" << language;
    HunspellDict *ad = new HunspellDict(inputLang, hunspell);
    return ad;
}

QStringList HunspellClient::languages() const
{
    return m_languagePaths.keys() + m_languageAliases.keys();
}

#include "moc_hunspellclient.cpp"
