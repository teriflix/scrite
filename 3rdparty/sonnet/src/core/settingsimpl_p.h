/*
 * SPDX-FileCopyrightText: 2003 Zack Rusin <zack@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef SONNET_SETTINGS_IMPL_P_H
#define SONNET_SETTINGS_IMPL_P_H

#include "sonnetcore_export.h"

#include <QString>
#include <QStringList>

#include <memory>

namespace Sonnet
{
class Loader;
class SettingsImplPrivate;
/**
 * SettingsImpl class
 */
class SONNETCORE_EXPORT SettingsImpl
{
public:
    ~SettingsImpl();

    SettingsImpl(const SettingsImpl &) = delete;
    SettingsImpl &operator=(const SettingsImpl &) = delete;

    [[nodiscard]] bool modified() const;
    void setModified(bool modified);

    bool setDefaultLanguage(const QString &lang);
    [[nodiscard]] QString defaultLanguage() const;

    bool setPreferredLanguages(const QStringList &lang);
    [[nodiscard]] QStringList preferredLanguages() const;

    bool setDefaultClient(const QString &client);
    [[nodiscard]] QString defaultClient() const;

    bool setCheckUppercase(bool);
    [[nodiscard]] bool checkUppercase() const;

    bool setAutodetectLanguage(bool);
    [[nodiscard]] bool autodetectLanguage() const;

    bool setSkipRunTogether(bool);
    [[nodiscard]] bool skipRunTogether() const;

    bool setBackgroundCheckerEnabled(bool);
    [[nodiscard]] bool backgroundCheckerEnabled() const;

    bool setCheckerEnabledByDefault(bool);
    [[nodiscard]] bool checkerEnabledByDefault() const;

    bool setCurrentIgnoreList(const QStringList &ignores);
    bool addWordToIgnore(const QString &word);
    [[nodiscard]] QStringList currentIgnoreList() const;
    bool ignore(const QString &word);

    void save();
    void restore();

    int disablePercentageWordError() const;
    int disableWordErrorCount() const;

private:
    SONNETCORE_NO_EXPORT bool setQuietIgnoreList(const QStringList &ignores);

private:
    friend class Loader;
    SONNETCORE_NO_EXPORT explicit SettingsImpl(Loader *loader);

private:
    std::unique_ptr<SettingsImplPrivate> const d;
};
}

#endif // SONNET_SETTINGS_IMPL_P_H
