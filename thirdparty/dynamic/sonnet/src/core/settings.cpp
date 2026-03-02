/*
 * SPDX-FileCopyrightText: 2020 Benjamin Port <benjamin.port@enioka.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "settingsimpl_p.h"

#include <QLocale>

#include "loader_p.h"
#include "settings.h"
#include <QDebug>
#include <speller.h>

namespace Sonnet
{
class DictionaryModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit DictionaryModel(QObject *parent = nullptr)
        : QAbstractListModel(parent)
    {
        reload();
    }

    ~DictionaryModel() override
    {
    }

    void reload()
    {
        beginResetModel();
        Sonnet::Speller speller;
        m_preferredDictionaries = speller.preferredDictionaries();
        m_availableDictionaries = speller.availableDictionaries();
        endResetModel();
    }

    void setDefaultLanguage(const QString &language)
    {
        m_defaultDictionary = language;
        Q_EMIT dataChanged(index(0, 0), index(rowCount(QModelIndex()) - 1, 0), {Settings::DefaultRole});
    }

    bool setData(const QModelIndex &idx, const QVariant &value, int role = Qt::EditRole) override
    {
        Q_UNUSED(value)

        if (!checkIndex(idx) || role != Qt::CheckStateRole) {
            return false;
        }
        const int row = idx.row();
        const auto language = m_availableDictionaries.keys().at(row);
        const auto inPreferredDictionaries = m_preferredDictionaries.contains(m_availableDictionaries.keys().at(row));

        if (inPreferredDictionaries) {
            m_preferredDictionaries.remove(language);
        } else {
            m_preferredDictionaries[language] = m_availableDictionaries.values().at(row);
        }
        qobject_cast<Settings *>(parent())->setPreferredLanguages(m_preferredDictionaries.values());
        Q_EMIT dataChanged(index(row, 0), index(row, 0), {Qt::CheckStateRole});
        return true;
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (!checkIndex(index)) {
            return {};
        }
        const int row = index.row();

        switch (role) {
        case Qt::DisplayRole:
            return m_availableDictionaries.keys().at(row);
        case Settings::LanguageCodeRole:
            return m_availableDictionaries.values().at(row);
        case Qt::CheckStateRole:
            return m_preferredDictionaries.contains(m_availableDictionaries.keys().at(row));
        case Settings::DefaultRole:
            return data(index, Settings::LanguageCodeRole) == m_defaultDictionary;
        }
        return {};
    }

    int rowCount(const QModelIndex &parent) const override
    {
        Q_UNUSED(parent)
        return m_availableDictionaries.count();
    }

    QHash<int, QByteArray> roleNames() const override
    {
        return {
            {Qt::DisplayRole, QByteArrayLiteral("display")},
            {Qt::CheckStateRole, QByteArrayLiteral("checked")},
            {Settings::PreferredRole, QByteArrayLiteral("isPreferred")},
            {Settings::LanguageCodeRole, QByteArrayLiteral("languageCode")},
            {Settings::DefaultRole, QByteArrayLiteral("isDefault")},
        };
    }

private:
    QMap<QString, QString> m_preferredDictionaries;
    QMap<QString, QString> m_availableDictionaries;
    QString m_defaultDictionary;
};

class SettingsPrivate
{
public:
    Loader *loader = nullptr;
    DictionaryModel *dictionaryModel = nullptr;
};

Settings::Settings(QObject *parent)
    : QObject(parent)
    , d(new SettingsPrivate)
{
    d->loader = Loader::openLoader();
}

Settings::~Settings() = default;

void Settings::setDefaultLanguage(const QString &lang)
{
    if (defaultLanguage() == lang) {
        return;
    }
    d->loader->settings()->setDefaultLanguage(lang);
    Q_EMIT defaultLanguageChanged();
    Q_EMIT modifiedChanged();

    if (d->dictionaryModel) {
        d->dictionaryModel->setDefaultLanguage(lang);
    }
}

QString Settings::defaultLanguage() const
{
    return d->loader->settings()->defaultLanguage();
}

void Settings::setPreferredLanguages(const QStringList &lang)
{
    if (!d->loader->settings()->setPreferredLanguages(lang)) {
        return;
    }
    Q_EMIT modifiedChanged();
    Q_EMIT preferredLanguagesChanged();
}

QStringList Settings::preferredLanguages() const
{
    return d->loader->settings()->preferredLanguages();
}

void Settings::setDefaultClient(const QString &client)
{
    if (!d->loader->settings()->setDefaultClient(client)) {
        return;
    }
    Q_EMIT defaultClientChanged();
    Q_EMIT modifiedChanged();
}

QString Settings::defaultClient() const
{
    return d->loader->settings()->defaultClient();
}

void Settings::setSkipUppercase(bool skip)
{
    if (!d->loader->settings()->setCheckUppercase(!skip)) {
        return;
    }
    Q_EMIT skipUppercaseChanged();
    Q_EMIT modifiedChanged();
}

bool Settings::skipUppercase() const
{
    return !d->loader->settings()->checkUppercase();
}

void Settings::setAutodetectLanguage(bool detect)
{
    if (!d->loader->settings()->setAutodetectLanguage(detect)) {
        return;
    }
    Q_EMIT autodetectLanguageChanged();
    Q_EMIT modifiedChanged();
}

bool Settings::autodetectLanguage() const
{
    return d->loader->settings()->autodetectLanguage();
}

void Settings::setSkipRunTogether(bool skip)
{
    if (skipRunTogether() == skip) {
        return;
    }
    d->loader->settings()->setSkipRunTogether(skip);
    Q_EMIT skipRunTogetherChanged();
    Q_EMIT modifiedChanged();
}

bool Settings::skipRunTogether() const
{
    return d->loader->settings()->skipRunTogether();
}

void Settings::setCheckerEnabledByDefault(bool check)
{
    if (checkerEnabledByDefault() == check) {
        return;
    }
    d->loader->settings()->setCheckerEnabledByDefault(check);
    Q_EMIT checkerEnabledByDefaultChanged();
    Q_EMIT modifiedChanged();
}

bool Settings::checkerEnabledByDefault() const
{
    return d->loader->settings()->checkerEnabledByDefault();
}

void Settings::setBackgroundCheckerEnabled(bool enable)
{
    if (backgroundCheckerEnabled() == enable) {
        return;
    }
    d->loader->settings()->setBackgroundCheckerEnabled(enable);
    Q_EMIT backgroundCheckerEnabledChanged();
    Q_EMIT modifiedChanged();
}

bool Settings::backgroundCheckerEnabled() const
{
    return d->loader->settings()->backgroundCheckerEnabled();
}

void Settings::setCurrentIgnoreList(const QStringList &ignores)
{
    if (currentIgnoreList() == ignores) {
        return;
    }
    d->loader->settings()->setCurrentIgnoreList(ignores);
    Q_EMIT currentIgnoreListChanged();
    Q_EMIT modifiedChanged();
}

QStringList Settings::currentIgnoreList() const
{
    return d->loader->settings()->currentIgnoreList();
}

QStringList Settings::clients() const
{
    return d->loader->clients();
}

void Settings::save()
{
    d->loader->settings()->save();
    Q_EMIT modifiedChanged();
}

bool Settings::modified() const
{
    return d->loader->settings()->modified();
}

// default values
// A static list of KDE specific words that we want to recognize
QStringList Settings::defaultIgnoreList()
{
    QStringList l;
    l.append(QStringLiteral("KMail"));
    l.append(QStringLiteral("KOrganizer"));
    l.append(QStringLiteral("KAddressBook"));
    l.append(QStringLiteral("KHTML"));
    l.append(QStringLiteral("KIO"));
    l.append(QStringLiteral("KJS"));
    l.append(QStringLiteral("Konqueror"));
    l.append(QStringLiteral("Sonnet"));
    l.append(QStringLiteral("Kontact"));
    l.append(QStringLiteral("Qt"));
    l.append(QStringLiteral("Okular"));
    l.append(QStringLiteral("KMix"));
    l.append(QStringLiteral("Amarok"));
    l.append(QStringLiteral("KDevelop"));
    return l;
}

bool Settings::defaultSkipUppercase()
{
    return false;
}

bool Settings::defaultAutodetectLanguage()
{
    return true;
}

bool Settings::defaultBackgroundCheckerEnabled()
{
    return true;
}

bool Settings::defaultCheckerEnabledByDefault()
{
    return false;
}

bool Settings::defauktSkipRunTogether()
{
    return true;
}

QString Settings::defaultDefaultLanguage()
{
    return QLocale::system().name();
}

QStringList Settings::defaultPreferredLanguages()
{
    return QStringList();
}

QAbstractListModel *Settings::dictionaryModel()
{
    // Lazy loading
    if (d->dictionaryModel) {
        return d->dictionaryModel;
    }

    d->dictionaryModel = new DictionaryModel(this);
    d->dictionaryModel->setDefaultLanguage(defaultLanguage());
    return d->dictionaryModel;
}
}

#include "moc_settings.cpp"
#include "settings.moc"
