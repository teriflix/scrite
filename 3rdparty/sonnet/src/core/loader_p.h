/*
 * SPDX-FileCopyrightText: 2003 Zack Rusin <zack@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef SONNET_LOADER_P_H
#define SONNET_LOADER_P_H

#include "sonnetcore_export.h"

#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QStringList>

#include <memory>

class QStaticPlugin;

namespace Sonnet
{
class SettingsImpl;
class SpellerPlugin;
class LoaderPrivate;
class Client;
/*!
 * \internal
 * \brief Class used to deal with dictionaries.
 *
 * This class manages all dictionaries. It's the top level
 * Sonnet class, you can think of it as the kernel or manager
 * of the Sonnet architecture.
 */
class SONNETCORE_EXPORT Loader : public QObject
{
    Q_OBJECT
public:
    /*!
     * Constructs the loader.
     *
     * It's very important that you leave the return value in a Loader::Ptr.
     * Loader is reference counted so if you don't want to have it deleted
     * under you simply have to hold it in a Loader::Ptr for as long as
     * you're using it.
     */
    static Loader *openLoader();

public:
    /*!
     */
    Loader();
    ~Loader() override;

    /*!
     * Returns dictionary for the given language and preferred client.
     *
     * \a language specifies the language of the dictionary. If an
     *        empty string will be passed the default language will
     *        be used. Has to be one of the values returned by
     *        languages()
     * \a client specifies the preferred client. If no client is
     *               specified a client which supports the given
     *               language is picked. If a few clients supports
     *               the same language the one with the biggest
     *               reliability value is returned.
     *
     */
    SpellerPlugin *createSpeller(const QString &language = QString(), const QString &client = QString()) const;

    /*!
     * Returns a shared, cached, dictionary for the given language.
     *
     * \a language specifies the language of the dictionary. If an
     *        empty string will be passed the default language will
     *        be used. Has to be one of the values returned by
     *        languages()
     */
    QSharedPointer<SpellerPlugin> cachedSpeller(const QString &language);

    /*!
     * Returns a shared, cached, dictionary for the given language.
     *
     * \a language specifies the language of the dictionary. If an
     *        empty string will be passed the default language will
     *        be used. Has to be one of the values returned by
     *        languages()
     */
    void clearSpellerCache();

    /*!
     * Returns names of all supported clients (e.g. ISpell, ASpell)
     */
    QStringList clients() const;

    /*!
     * Returns a list of supported languages.
     */
    QStringList languages() const;

    /*!
     * Returns a localized list of names of supported languages.
     */
    QStringList languageNames() const;

    /*!
     * \a langCode the dictionary name/language code, e.g. en_gb
     * Returns the localized language name, e.g. "British English"
     * \since 4.2
     */
    QString languageNameForCode(const QString &langCode) const;

    /*!
     * Returns the SettingsImpl object used by the loader.
     */
    SettingsImpl *settings() const;
Q_SIGNALS:
    /*!
     * Signal is emitted whenever the SettingsImpl object
     * associated with this Loader changes.
     */
    void configurationChanged();

    /*!
     * Emitted when loading a dictionary fails, so that Ui parts can
     * display an appropriate error message informing the user about
     * the issue.
     * \a language the name of the dictionary that failed to be loaded
     * \since 5.56
     */
    void loadingDictionaryFailed(const QString &language) const;

protected:
    friend class SettingsImpl;
    void changed();

private:
    SONNETCORE_NO_EXPORT void loadPlugins();
    SONNETCORE_NO_EXPORT void loadPlugin(const QString &pluginPath);
    SONNETCORE_NO_EXPORT void loadPlugin(const QStaticPlugin &plugin);
    SONNETCORE_NO_EXPORT void addClient(Sonnet::Client *client);

private:
    std::unique_ptr<LoaderPrivate> const d;
};
}

#endif // SONNET_LOADER_P_H
