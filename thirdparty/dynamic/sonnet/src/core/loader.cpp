/*
 * SPDX-FileCopyrightText: 2003 Zack Rusin <zack@kde.org>
 * SPDX-FileCopyrightText: 2012 Martin Sandsmark <martin.sandsmark@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "client_p.h"
#include "loader_p.h"
#include "settingsimpl_p.h"
#include "spellerplugin_p.h"

#include "core_debug.h"

#include <QCoreApplication>
#include <QDir>
#include <QHash>
#include <QList>
#include <QLocale>
#include <QMap>
#include <QPluginLoader>

#include <algorithm>

namespace Sonnet
{
class LoaderPrivate
{
public:
    SettingsImpl *settings;

    // <language, Clients with that language >
    QMap<QString, QList<Client *>> languageClients;
    QStringList clients;

    QSet<QString> loadedPlugins;

    QStringList languagesNameCache;
    QHash<QString, QSharedPointer<SpellerPlugin>> spellerCache;
};

Q_GLOBAL_STATIC(Loader, s_loader)

Loader *Loader::openLoader()
{
    if (s_loader.isDestroyed()) {
        return nullptr;
    }

    return s_loader();
}

Loader::Loader()
    : d(new LoaderPrivate)
{
    d->settings = new SettingsImpl(this);
    d->settings->restore();
    loadPlugins();
}

Loader::~Loader()
{
    qCDebug(SONNET_LOG_CORE) << "Removing loader: " << this;
    delete d->settings;
    d->settings = nullptr;
}

SpellerPlugin *Loader::createSpeller(const QString &language, const QString &clientName) const
{
    QString backend = clientName;
    QString plang = language;

    if (plang.isEmpty()) {
        plang = d->settings->defaultLanguage();
    }

    auto clientsItr = d->languageClients.constFind(plang);
    if (clientsItr == d->languageClients.constEnd()) {
        if (language.isEmpty() || language == QStringLiteral("C")) {
            qCDebug(SONNET_LOG_CORE) << "No language dictionaries for the language:" << plang << "trying to load en_US as default";
            return createSpeller(QStringLiteral("en_US"), clientName);
        }
        qCDebug(SONNET_LOG_CORE) << "No language dictionaries for the language:" << plang;
        Q_EMIT loadingDictionaryFailed(plang);
        return nullptr;
    }

    const QList<Client *> lClients = *clientsItr;

    if (backend.isEmpty()) {
        backend = d->settings->defaultClient();
        if (!backend.isEmpty()) {
            // check if the default client supports the requested language;
            // if it does it will be an element of lClients.
            bool unknown = !std::any_of(lClients.constBegin(), lClients.constEnd(), [backend](const Client *client) {
                return client->name() == backend;
            });
            if (unknown) {
                qCWarning(SONNET_LOG_CORE) << "Default client" << backend << "doesn't support language:" << plang;
                backend = QString();
            }
        }
    }

    QListIterator<Client *> itr(lClients);
    while (itr.hasNext()) {
        Client *item = itr.next();
        if (!backend.isEmpty()) {
            if (backend == item->name()) {
                SpellerPlugin *dict = item->createSpeller(plang);
                qCDebug(SONNET_LOG_CORE) << "Using the" << item->name() << "plugin for language" << plang;
                return dict;
            }
        } else {
            // the first one is the one with the highest
            // reliability
            SpellerPlugin *dict = item->createSpeller(plang);
            qCDebug(SONNET_LOG_CORE) << "Using the" << item->name() << "plugin for language" << plang;
            return dict;
        }
    }

    qCWarning(SONNET_LOG_CORE) << "The default client" << backend << "has no language dictionaries for the language:" << plang;
    return nullptr;
}

QSharedPointer<SpellerPlugin> Loader::cachedSpeller(const QString &language)
{
    auto &speller = d->spellerCache[language];
    if (!speller) {
        speller.reset(createSpeller(language));
    }
    return speller;
}

void Loader::clearSpellerCache()
{
    d->spellerCache.clear();
}

QStringList Loader::clients() const
{
    return d->clients;
}

QStringList Loader::languages() const
{
    return d->languageClients.keys();
}

QString Loader::languageNameForCode(const QString &langCode) const
{
    QString currentDictionary = langCode; // e.g. en_GB-ize-wo_accents
    QString isoCode; // locale ISO name
    QString variantName; // dictionary variant name e.g. w_accents
    QString localizedLang; // localized language
    QString localizedCountry; // localized country
    QString localizedVariant;
    QByteArray variantEnglish; // dictionary variant in English

    int minusPos; // position of "-" char
    int variantCount = 0; // used to iterate over variantList

    struct variantListType {
        const char *variantShortName;
        const char *variantEnglishName;
    };

    /*
     * This redefines the QT_TRANSLATE_NOOP3 macro provided by Qt to indicate that
     * statically initialised text should be translated so that it expands to just
     * the string that should be translated, making it possible to use it in the
     * single string construct below.
     */
#undef QT_TRANSLATE_NOOP3
#define QT_TRANSLATE_NOOP3(a, b, c) b

    const variantListType variantList[] = {{"40", QT_TRANSLATE_NOOP3("Sonnet::Loader", "40", "dictionary variant")}, // what does 40 mean?
                                           {"60", QT_TRANSLATE_NOOP3("Sonnet::Loader", "60", "dictionary variant")}, // what does 60 mean?
                                           {"80", QT_TRANSLATE_NOOP3("Sonnet::Loader", "80", "dictionary variant")}, // what does 80 mean?
                                           {"ise", QT_TRANSLATE_NOOP3("Sonnet::Loader", "-ise suffixes", "dictionary variant")},
                                           {"ize", QT_TRANSLATE_NOOP3("Sonnet::Loader", "-ize suffixes", "dictionary variant")},
                                           {"ise-w_accents", QT_TRANSLATE_NOOP3("Sonnet::Loader", "-ise suffixes and with accents", "dictionary variant")},
                                           {"ise-wo_accents", QT_TRANSLATE_NOOP3("Sonnet::Loader", "-ise suffixes and without accents", "dictionary variant")},
                                           {"ize-w_accents", QT_TRANSLATE_NOOP3("Sonnet::Loader", "-ize suffixes and with accents", "dictionary variant")},
                                           {"ize-wo_accents", QT_TRANSLATE_NOOP3("Sonnet::Loader", "-ize suffixes and without accents", "dictionary variant")},
                                           {"lrg", QT_TRANSLATE_NOOP3("Sonnet::Loader", "large", "dictionary variant")},
                                           {"med", QT_TRANSLATE_NOOP3("Sonnet::Loader", "medium", "dictionary variant")},
                                           {"sml", QT_TRANSLATE_NOOP3("Sonnet::Loader", "small", "dictionary variant")},
                                           {"variant_0", QT_TRANSLATE_NOOP3("Sonnet::Loader", "variant 0", "dictionary variant")},
                                           {"variant_1", QT_TRANSLATE_NOOP3("Sonnet::Loader", "variant 1", "dictionary variant")},
                                           {"variant_2", QT_TRANSLATE_NOOP3("Sonnet::Loader", "variant 2", "dictionary variant")},
                                           {"wo_accents", QT_TRANSLATE_NOOP3("Sonnet::Loader", "without accents", "dictionary variant")},
                                           {"w_accents", QT_TRANSLATE_NOOP3("Sonnet::Loader", "with accents", "dictionary variant")},
                                           {"ye", QT_TRANSLATE_NOOP3("Sonnet::Loader", "with ye, modern russian", "dictionary variant")},
                                           {"yeyo", QT_TRANSLATE_NOOP3("Sonnet::Loader", "with yeyo, modern and old russian", "dictionary variant")},
                                           {"yo", QT_TRANSLATE_NOOP3("Sonnet::Loader", "with yo, old russian", "dictionary variant")},
                                           {"extended", QT_TRANSLATE_NOOP3("Sonnet::Loader", "extended", "dictionary variant")},
                                           {nullptr, nullptr}};

    minusPos = currentDictionary.indexOf(QLatin1Char('-'));
    if (minusPos != -1) {
        variantName = currentDictionary.right(currentDictionary.length() - minusPos - 1);
        while (variantList[variantCount].variantShortName != nullptr) {
            if (QLatin1String(variantList[variantCount].variantShortName) == variantName) {
                break;
            } else {
                variantCount++;
            }
        }
        if (variantList[variantCount].variantShortName != nullptr) {
            variantEnglish = variantList[variantCount].variantEnglishName;
        } else {
            variantEnglish = variantName.toLatin1();
        }

        localizedVariant = tr(variantEnglish.constData(), "dictionary variant");
        isoCode = currentDictionary.left(minusPos);
    } else {
        isoCode = currentDictionary;
    }

    QLocale locale(isoCode);
    localizedCountry = locale.nativeTerritoryName();
    localizedLang = locale.nativeLanguageName();

    if (localizedLang.isEmpty() && localizedCountry.isEmpty()) {
        return isoCode; // We have nothing
    }

    if (!localizedCountry.isEmpty() && !localizedVariant.isEmpty()) { // We have both a country name and a variant
        return tr("%1 (%2) [%3]", "dictionary name; %1 = language name, %2 = country name and %3 = language variant name")
            .arg(localizedLang, localizedCountry, localizedVariant);
    } else if (!localizedCountry.isEmpty()) { // We have a country name
        return tr("%1 (%2)", "dictionary name; %1 = language name, %2 = country name").arg(localizedLang, localizedCountry);
    } else { // We only have a language name
        return localizedLang;
    }
}

QStringList Loader::languageNames() const
{
    /* For whatever reason languages() might change. So,
     * to be in sync with it let's do the following check.
     */
    if (d->languagesNameCache.count() == languages().count()) {
        return d->languagesNameCache;
    }

    QStringList allLocalizedDictionaries;
    for (const QString &langCode : languages()) {
        allLocalizedDictionaries.append(languageNameForCode(langCode));
    }
    // cache the list
    d->languagesNameCache = allLocalizedDictionaries;
    return allLocalizedDictionaries;
}

SettingsImpl *Loader::settings() const
{
    return d->settings;
}

void Loader::loadPlugins()
{
#ifndef SONNET_STATIC
    const QStringList libPaths = QCoreApplication::libraryPaths() << QStringLiteral(INSTALLATION_PLUGIN_PATH);
    const QString pathSuffix(QStringLiteral("/kf6/sonnet/"));
    for (const QString &libPath : libPaths) {
        QDir dir(libPath + pathSuffix);
        if (!dir.exists()) {
            continue;
        }
        for (const QString &fileName : dir.entryList(QDir::Files)) {
            loadPlugin(dir.absoluteFilePath(fileName));
        }
    }

    if (d->loadedPlugins.isEmpty()) {
        qCWarning(SONNET_LOG_CORE) << "Sonnet: No speller backends available!";
    }
#else
    for (auto plugin : QPluginLoader::staticPlugins()) {
        if (plugin.metaData()[QLatin1String("IID")].toString() == QLatin1String("org.kde.sonnet.Client")) {
            loadPlugin(plugin);
        }
    }
#endif
}

void Loader::loadPlugin(const QStaticPlugin &plugin)
{
    Client *client = qobject_cast<Client *>(plugin.instance());
    if (!client) {
        qCWarning(SONNET_LOG_CORE) << "Sonnet: Invalid static plugin loaded" << plugin.metaData();
        return;
    }

    addClient(client);
}

void Loader::loadPlugin(const QString &pluginPath)
{
    QPluginLoader plugin(pluginPath);
    const QString pluginId = QFileInfo(pluginPath).completeBaseName();
    if (!pluginId.isEmpty()) {
        if (d->loadedPlugins.contains(pluginId)) {
            qCDebug(SONNET_LOG_CORE) << "Skipping already loaded" << pluginPath;
            return;
        }
    }
    d->loadedPlugins.insert(pluginId);

    if (!plugin.load()) { // We do this separately for better error handling
        qCDebug(SONNET_LOG_CORE) << "Sonnet: Unable to load plugin" << pluginPath << "Error:" << plugin.errorString();
        d->loadedPlugins.remove(pluginId);
        return;
    }

    Client *client = qobject_cast<Client *>(plugin.instance());

    if (!client) {
        qCWarning(SONNET_LOG_CORE) << "Sonnet: Invalid plugin loaded" << pluginPath;
        plugin.unload(); // don't leave it in memory
        return;
    }

    addClient(client);
}

void Loader::addClient(Client *client)
{
    const QStringList languages = client->languages();
    d->clients.append(client->name());

    for (const QString &language : languages) {
        QList<Client *> &languageClients = d->languageClients[language];

        if (languageClients.isEmpty() //
            || client->reliability() < languageClients.first()->reliability()) {
            languageClients.append(client); // less reliable, to the end
        } else {
            languageClients.prepend(client); // more reliable, to the front
        }
    }
}

void Loader::changed()
{
    Q_EMIT configurationChanged();
}
}

#include "moc_loader_p.cpp"
