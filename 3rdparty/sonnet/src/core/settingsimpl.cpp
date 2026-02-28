/*
 * SPDX-FileCopyrightText: 2003 Zack Rusin <zack@kde.org>
 * SPDX-FileCopyrightText: 2006 Laurent Montel <montel@kde.org>
 * SPDX-FileCopyrightText: 2013 Martin Sandsmark <martin.sandsmark@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "settingsimpl_p.h"

#include "loader_p.h"

#include <QMap>
#include <QSettings>

#include "settings.h"

namespace Sonnet
{
class SettingsImplPrivate
{
public:
    Loader *loader = nullptr; // can't be a Ptr since we don't want to hold a ref on it
    bool modified = false;

    QString defaultLanguage;
    QStringList preferredLanguages;
    QString defaultClient;

    bool checkUppercase = false;
    bool skipRunTogether = false;
    bool backgroundCheckerEnabled = false;
    bool checkerEnabledByDefault = false;
    bool autodetectLanguage = false;

    int disablePercentage;
    int disableWordCount;

    QMap<QString, bool> ignore;
};

SettingsImpl::SettingsImpl(Loader *loader)
    : d(new SettingsImplPrivate)
{
    d->loader = loader;

    d->modified = false;
    d->checkerEnabledByDefault = false;
    restore();
}

SettingsImpl::~SettingsImpl() = default;

bool SettingsImpl::setDefaultLanguage(const QString &lang)
{
    const QStringList cs = d->loader->languages();
    if (cs.indexOf(lang) != -1 && d->defaultLanguage != lang) {
        d->defaultLanguage = lang;
        d->modified = true;
        d->loader->changed();
        return true;
    }
    return false;
}

QString SettingsImpl::defaultLanguage() const
{
    return d->defaultLanguage;
}

bool SettingsImpl::setPreferredLanguages(const QStringList &lang)
{
    if (d->preferredLanguages != lang) {
        d->modified = true;
        d->preferredLanguages = lang;
        return true;
    }

    return false;
}

QStringList SettingsImpl::preferredLanguages() const
{
    return d->preferredLanguages;
}

bool SettingsImpl::setDefaultClient(const QString &client)
{
    // Different from setDefaultLanguage because
    // the number of clients can't be even close
    // as big as the number of languages
    if (d->loader->clients().contains(client)) {
        d->defaultClient = client;
        d->modified = true;
        d->loader->changed();
        return true;
    }
    return false;
}

QString SettingsImpl::defaultClient() const
{
    return d->defaultClient;
}

bool SettingsImpl::setCheckUppercase(bool check)
{
    if (d->checkUppercase != check) {
        d->modified = true;
        d->checkUppercase = check;
        return true;
    }
    return false;
}

bool SettingsImpl::checkUppercase() const
{
    return d->checkUppercase;
}

bool SettingsImpl::setAutodetectLanguage(bool detect)
{
    if (d->autodetectLanguage != detect) {
        d->modified = true;
        d->autodetectLanguage = detect;
        return true;
    }
    return false;
}

bool SettingsImpl::autodetectLanguage() const
{
    return d->autodetectLanguage;
}

bool SettingsImpl::setSkipRunTogether(bool skip)
{
    if (d->skipRunTogether != skip) {
        d->modified = true;
        d->skipRunTogether = skip;
        return true;
    }
    return false;
}

bool SettingsImpl::skipRunTogether() const
{
    return d->skipRunTogether;
}

bool SettingsImpl::setCheckerEnabledByDefault(bool check)
{
    if (d->checkerEnabledByDefault != check) {
        d->modified = true;
        d->checkerEnabledByDefault = check;
        return true;
    }
    return false;
}

bool SettingsImpl::checkerEnabledByDefault() const
{
    return d->checkerEnabledByDefault;
}

bool SettingsImpl::setBackgroundCheckerEnabled(bool enable)
{
    if (d->backgroundCheckerEnabled != enable) {
        d->modified = true;
        d->backgroundCheckerEnabled = enable;
        return true;
    }
    return false;
}

bool SettingsImpl::backgroundCheckerEnabled() const
{
    return d->backgroundCheckerEnabled;
}

bool SettingsImpl::setCurrentIgnoreList(const QStringList &ignores)
{
    bool changed = setQuietIgnoreList(ignores);
    d->modified = true;
    return changed;
}

bool SettingsImpl::setQuietIgnoreList(const QStringList &ignores)
{
    bool changed = false;
    d->ignore = QMap<QString, bool>(); // clear out
    for (QStringList::const_iterator itr = ignores.begin(); itr != ignores.end(); ++itr) {
        d->ignore.insert(*itr, true);
        changed = true;
    }
    return changed;
}

QStringList SettingsImpl::currentIgnoreList() const
{
    return d->ignore.keys();
}

bool SettingsImpl::addWordToIgnore(const QString &word)
{
    if (!d->ignore.contains(word)) {
        d->modified = true;
        d->ignore.insert(word, true);
        return true;
    }
    return false;
}

bool SettingsImpl::ignore(const QString &word)
{
    return d->ignore.contains(word);
}

int SettingsImpl::disablePercentageWordError() const
{
    return d->disablePercentage;
}

int SettingsImpl::disableWordErrorCount() const
{
    return d->disableWordCount;
}

void SettingsImpl::save()
{
    QSettings settings(QStringLiteral("KDE"), QStringLiteral("Sonnet"));
    settings.setValue(QStringLiteral("defaultClient"), d->defaultClient);
    settings.setValue(QStringLiteral("defaultLanguage"), d->defaultLanguage);
    settings.setValue(QStringLiteral("preferredLanguages"), d->preferredLanguages);
    settings.setValue(QStringLiteral("checkUppercase"), d->checkUppercase);
    settings.setValue(QStringLiteral("skipRunTogether"), d->skipRunTogether);
    settings.setValue(QStringLiteral("backgroundCheckerEnabled"), d->backgroundCheckerEnabled);
    settings.setValue(QStringLiteral("checkerEnabledByDefault"), d->checkerEnabledByDefault);
    settings.setValue(QStringLiteral("autodetectLanguage"), d->autodetectLanguage);
    QString defaultLanguage = QStringLiteral("ignore_%1").arg(d->defaultLanguage);
    if (settings.contains(defaultLanguage) && d->ignore.isEmpty()) {
        settings.remove(defaultLanguage);
    } else if (!d->ignore.isEmpty()) {
        settings.setValue(defaultLanguage, QStringList(d->ignore.keys()));
    }
    d->modified = false;
}

void SettingsImpl::restore()
{
    QSettings settings(QStringLiteral("KDE"), QStringLiteral("Sonnet"));
    d->defaultClient = settings.value(QStringLiteral("defaultClient"), QString()).toString();
    d->defaultLanguage = settings.value(QStringLiteral("defaultLanguage"), Settings::defaultDefaultLanguage()).toString();
    d->preferredLanguages = settings.value(QStringLiteral("preferredLanguages"), Settings::defaultPreferredLanguages()).toStringList();

    // same defaults are in the default filter (filter.cpp)
    d->checkUppercase = settings.value(QStringLiteral("checkUppercase"), !Settings::defaultSkipUppercase()).toBool();
    d->skipRunTogether = settings.value(QStringLiteral("skipRunTogether"), Settings::defauktSkipRunTogether()).toBool();
    d->backgroundCheckerEnabled = settings.value(QStringLiteral("backgroundCheckerEnabled"), Settings::defaultBackgroundCheckerEnabled()).toBool();
    d->checkerEnabledByDefault = settings.value(QStringLiteral("checkerEnabledByDefault"), Settings::defaultCheckerEnabledByDefault()).toBool();
    d->disablePercentage = settings.value(QStringLiteral("Sonnet_AsYouTypeDisablePercentage"), 90).toInt();
    d->disableWordCount = settings.value(QStringLiteral("Sonnet_AsYouTypeDisableWordCount"), 100).toInt();
    d->autodetectLanguage = settings.value(QStringLiteral("autodetectLanguage"), Settings::defaultAutodetectLanguage()).toBool();

    const QString ignoreEntry = QStringLiteral("ignore_%1").arg(d->defaultLanguage);
    const QStringList ignores = settings.value(ignoreEntry, Settings::defaultIgnoreList()).toStringList();
    setQuietIgnoreList(ignores);
}

bool SettingsImpl::modified() const
{
    return d->modified;
}

void SettingsImpl::setModified(bool modified)
{
    d->modified = modified;
}
} // namespace Sonnet
