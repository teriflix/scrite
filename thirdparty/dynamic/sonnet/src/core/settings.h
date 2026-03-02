/*
 * SPDX-FileCopyrightText: 2020 Benjamin Port <benjamin.port@enioka.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef SONNET_SETTINGS_H
#define SONNET_SETTINGS_H

#include <QAbstractListModel>
#include <QObject>
#include <QString>
#include <QStringList>

#include "sonnetcore_export.h"

#include <memory>

namespace Sonnet
{
class Loader;
class SettingsPrivate;

/*!
 * \class Sonnet::Settings
 * \inheaderfile Sonnet/Settings
 * \inmodule SonnetCore
 */
class SONNETCORE_EXPORT Settings : public QObject
{
    Q_OBJECT

    /*!
     * \property Sonnet::Settings::skipUppercase
     *
     * This property holds whether Sonnet should skip checkign words starting with an uppercase letter.
     */
    Q_PROPERTY(bool skipUppercase READ skipUppercase WRITE setSkipUppercase NOTIFY skipUppercaseChanged)

    /*!
     * \property Sonnet::Settings::autodetectLanguage
     *
     * This property holds whether Sonnet should autodetect language.
     */
    Q_PROPERTY(bool autodetectLanguage READ autodetectLanguage WRITE setAutodetectLanguage NOTIFY autodetectLanguageChanged)

    /*!
     * \property Sonnet::Settings::backgroundCheckerEnabled
     *
     * This property holds whether Sonnet should run spellchecking checks in the background.
     */
    Q_PROPERTY(bool backgroundCheckerEnabled READ backgroundCheckerEnabled WRITE setBackgroundCheckerEnabled NOTIFY backgroundCheckerEnabledChanged)

    /*!
     * \property Sonnet::Settings::checkerEnabledByDefault
     *
     * This property holds whether Sonnet should be enabled by default.
     */
    Q_PROPERTY(bool checkerEnabledByDefault READ checkerEnabledByDefault WRITE setCheckerEnabledByDefault NOTIFY checkerEnabledByDefaultChanged)

    /*!
     * \property Sonnet::Settings::skipRunTogether
     *
     * This property holds whether Sonnet should skip checking compounds words.
     */
    Q_PROPERTY(bool skipRunTogether READ skipRunTogether WRITE setSkipRunTogether NOTIFY skipRunTogetherChanged)

    /*!
     * \property Sonnet::Settings::currentIgnoreList
     *
     * This property holds the list of ignored words.
     */
    Q_PROPERTY(QStringList currentIgnoreList READ currentIgnoreList WRITE setCurrentIgnoreList NOTIFY currentIgnoreListChanged)

    /*!
     * \property Sonnet::Settings::preferredLanguages
     *
     * This property holds the list of preferred languages.
     */
    Q_PROPERTY(QStringList preferredLanguages READ preferredLanguages WRITE setPreferredLanguages NOTIFY preferredLanguagesChanged)

    /*!
     * \property Sonnet::Settings::defaultLanguage
     *
     * This property holds the default language for spell checking.
     */
    Q_PROPERTY(QString defaultLanguage READ defaultLanguage WRITE setDefaultLanguage NOTIFY defaultLanguageChanged)

    /*!
     * \property Sonnet::Settings::dictionaryModel
     *
     * This property holds a Qt Model containing all the preferred dictionaries
     * with language description and theirs codes. This model makes the
     * Qt::DisplayRole as well as the roles defined in DictionaryRoles
     * available.
     * \since 5.88
     */
    Q_PROPERTY(QAbstractListModel *dictionaryModel READ dictionaryModel CONSTANT)

    /*!
     * \property Sonnet::Settings::modified
     */
    Q_PROPERTY(bool modified READ modified NOTIFY modifiedChanged)

public:
    /*!
     * Roles for dictionaryModel
     *
     * \value LanguageCodeRole
     *        Language code of the language. This uses "languageCode" as roleNames.
     * \value PreferredRole
     *        This role holds whether the language is one of the preferred languages. This uses "isPreferred" as roleNames.
     * \value DefaultRole
     *        This role holds whether the language is the default language. This uses "isDefault" as roleNames.
     */
    enum DictionaryRoles {
        LanguageCodeRole = Qt::UserRole + 1,
        PreferredRole,
        DefaultRole
    };

    /*!
     */
    explicit Settings(QObject *parent = nullptr);
    ~Settings() override;

    void setDefaultLanguage(const QString &lang);

    [[nodiscard]] QString defaultLanguage() const;

    void setPreferredLanguages(const QStringList &lang);

    [[nodiscard]] QStringList preferredLanguages() const;

    void setDefaultClient(const QString &client);

    [[nodiscard]] QString defaultClient() const;

    void setSkipUppercase(bool);

    [[nodiscard]] bool skipUppercase() const;

    void setAutodetectLanguage(bool);

    [[nodiscard]] bool autodetectLanguage() const;

    void setSkipRunTogether(bool);

    [[nodiscard]] bool skipRunTogether() const;

    void setBackgroundCheckerEnabled(bool);

    [[nodiscard]] bool backgroundCheckerEnabled() const;

    void setCheckerEnabledByDefault(bool);

    [[nodiscard]] bool checkerEnabledByDefault() const;

    void setCurrentIgnoreList(const QStringList &ignores);

    [[nodiscard]] QStringList currentIgnoreList() const;

    [[nodiscard]] QStringList clients() const;

    [[nodiscard]] bool modified() const;

    QAbstractListModel *dictionaryModel();

    /*!
     *
     */
    Q_INVOKABLE void save();

    [[nodiscard]] static QStringList defaultIgnoreList();

    [[nodiscard]] static bool defaultSkipUppercase();

    [[nodiscard]] static bool defaultAutodetectLanguage();

    [[nodiscard]] static bool defaultBackgroundCheckerEnabled();

    [[nodiscard]] static bool defaultCheckerEnabledByDefault();

    [[nodiscard]] static bool defauktSkipRunTogether();

    [[nodiscard]] static QString defaultDefaultLanguage();

    [[nodiscard]] static QStringList defaultPreferredLanguages();

Q_SIGNALS:

    void skipUppercaseChanged();

    void autodetectLanguageChanged();

    void backgroundCheckerEnabledChanged();

    void defaultClientChanged();

    void defaultLanguageChanged();

    void preferredLanguagesChanged();

    void skipRunTogetherChanged();

    void checkerEnabledByDefaultChanged();

    void currentIgnoreListChanged();

    void modifiedChanged();

private:
    friend class Loader;
    std::unique_ptr<SettingsPrivate> const d;
};
}

#endif // SONNET_SETTINGS_H
