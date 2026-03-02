/*
 * configwidget.cpp
 *
 * SPDX-FileCopyrightText: 2004 Zack Rusin <zack@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "configwidget.h"
#include "ui_configui.h"

#include "loader_p.h"
#include "settings.h"
#include "settingsimpl_p.h"

#include "ui_debug.h"

#include <QCheckBox>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>

using namespace Sonnet;

class Sonnet::ConfigWidgetPrivate
{
public:
    Ui_SonnetConfigUI ui;
    Settings *settings = nullptr;
    QWidget *wdg = nullptr;
};

ConfigWidget::ConfigWidget(QWidget *parent)
    : QWidget(parent)
    , d(new ConfigWidgetPrivate)
{
    d->settings = new Settings(this);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setObjectName(QStringLiteral("SonnetConfigUILayout"));
    d->wdg = new QWidget(this);
    d->ui.setupUi(d->wdg);
    d->ui.languageList->setProperty("_breeze_force_frame", true);

    d->ui.m_langCombo->setCurrentByDictionary(d->settings->defaultLanguage());

    QStringList preferredLanguages = d->settings->preferredLanguages();
    for (int i = 0; i < d->ui.m_langCombo->count(); i++) {
        const QString tag = d->ui.m_langCombo->itemData(i).toString();
        if (tag.isEmpty()) { // skip separator
            continue;
        }

        QListWidgetItem *item = new QListWidgetItem(d->ui.m_langCombo->itemText(i), d->ui.languageList);
        item->setData(Qt::UserRole, tag);
        if (preferredLanguages.contains(tag)) {
            item->setCheckState(Qt::Checked);
        } else {
            item->setCheckState(Qt::Unchecked);
        }
    }

    d->ui.kcfg_skipUppercase->setChecked(d->settings->skipUppercase());
    d->ui.kcfg_skipRunTogether->setChecked(d->settings->skipRunTogether());
    d->ui.kcfg_checkerEnabledByDefault->setChecked(d->settings->checkerEnabledByDefault());
    d->ui.kcfg_autodetectLanguage->setChecked(d->settings->autodetectLanguage());
    QStringList ignoreList = d->settings->currentIgnoreList();
    ignoreList.sort();
    d->ui.ignoreListWidget->addItems(ignoreList);
    d->ui.kcfg_backgroundCheckerEnabled->setChecked(d->settings->backgroundCheckerEnabled());
    d->ui.kcfg_backgroundCheckerEnabled->hide(); // hidden by default
    connect(d->ui.addButton, &QAbstractButton::clicked, this, &ConfigWidget::slotIgnoreWordAdded);
    connect(d->ui.removeButton, &QAbstractButton::clicked, this, &ConfigWidget::slotIgnoreWordRemoved);

    layout->addWidget(d->wdg);
    connect(d->ui.m_langCombo, &DictionaryComboBox::dictionaryChanged, this, &ConfigWidget::configChanged);
    connect(d->ui.languageList, &QListWidget::itemChanged, this, &ConfigWidget::configChanged);

    connect(d->ui.kcfg_backgroundCheckerEnabled, &QAbstractButton::clicked, this, &ConfigWidget::configChanged);
    connect(d->ui.kcfg_skipUppercase, &QAbstractButton::clicked, this, &ConfigWidget::configChanged);
    connect(d->ui.kcfg_skipRunTogether, &QAbstractButton::clicked, this, &ConfigWidget::configChanged);
    connect(d->ui.kcfg_checkerEnabledByDefault, &QAbstractButton::clicked, this, &ConfigWidget::configChanged);
    connect(d->ui.kcfg_autodetectLanguage, &QAbstractButton::clicked, this, &ConfigWidget::configChanged);
    connect(d->ui.newIgnoreEdit, &QLineEdit::textChanged, this, &ConfigWidget::slotUpdateButton);
    connect(d->ui.ignoreListWidget, &QListWidget::itemSelectionChanged, this, &ConfigWidget::slotSelectionChanged);
    d->ui.nobackendfound->setVisible(d->settings->clients().isEmpty());
    d->ui.addButton->setEnabled(false);
    d->ui.removeButton->setEnabled(false);
}

ConfigWidget::~ConfigWidget() = default;

void ConfigWidget::slotUpdateButton(const QString &text)
{
    d->ui.addButton->setEnabled(!text.isEmpty());
}

void ConfigWidget::slotSelectionChanged()
{
    d->ui.removeButton->setEnabled(!d->ui.ignoreListWidget->selectedItems().isEmpty());
}

void ConfigWidget::save()
{
    setFromGui();
}

void ConfigWidget::setFromGui()
{
    if (d->ui.m_langCombo->count()) {
        d->settings->setDefaultLanguage(d->ui.m_langCombo->currentDictionary());
    }

    QStringList preferredLanguages;
    for (int i = 0; i < d->ui.languageList->count(); i++) {
        if (d->ui.languageList->item(i)->checkState() == Qt::Unchecked) {
            continue;
        }
        preferredLanguages << d->ui.languageList->item(i)->data(Qt::UserRole).toString();
    }
    d->settings->setPreferredLanguages(preferredLanguages);

    d->settings->setSkipUppercase(d->ui.kcfg_skipUppercase->isChecked());
    d->settings->setSkipRunTogether(d->ui.kcfg_skipRunTogether->isChecked());
    d->settings->setBackgroundCheckerEnabled(d->ui.kcfg_backgroundCheckerEnabled->isChecked());
    d->settings->setCheckerEnabledByDefault(d->ui.kcfg_checkerEnabledByDefault->isChecked());
    d->settings->setAutodetectLanguage(d->ui.kcfg_autodetectLanguage->isChecked());

    if (d->settings->modified()) {
        d->settings->save();
    }
}

void ConfigWidget::slotIgnoreWordAdded()
{
    QStringList ignoreList = d->settings->currentIgnoreList();
    QString newWord = d->ui.newIgnoreEdit->text();
    d->ui.newIgnoreEdit->clear();
    if (newWord.isEmpty() || ignoreList.contains(newWord)) {
        return;
    }
    ignoreList.append(newWord);
    d->settings->setCurrentIgnoreList(ignoreList);

    d->ui.ignoreListWidget->clear();
    d->ui.ignoreListWidget->addItems(ignoreList);

    Q_EMIT configChanged();
}

void ConfigWidget::slotIgnoreWordRemoved()
{
    QStringList ignoreList = d->settings->currentIgnoreList();
    const QList<QListWidgetItem *> selectedItems = d->ui.ignoreListWidget->selectedItems();
    for (const QListWidgetItem *item : selectedItems) {
        ignoreList.removeAll(item->text());
    }
    d->settings->setCurrentIgnoreList(ignoreList);

    d->ui.ignoreListWidget->clear();
    d->ui.ignoreListWidget->addItems(ignoreList);

    Q_EMIT configChanged();
}

void ConfigWidget::setBackgroundCheckingButtonShown(bool b)
{
    d->ui.kcfg_backgroundCheckerEnabled->setVisible(b);
}

bool ConfigWidget::backgroundCheckingButtonShown() const
{
    return !d->ui.kcfg_backgroundCheckerEnabled->isHidden();
}

void ConfigWidget::slotDefault()
{
    d->ui.kcfg_autodetectLanguage->setChecked(Settings::defaultAutodetectLanguage());
    d->ui.kcfg_skipUppercase->setChecked(Settings::defaultSkipUppercase());
    d->ui.kcfg_skipRunTogether->setChecked(Settings::defauktSkipRunTogether());
    d->ui.kcfg_checkerEnabledByDefault->setChecked(Settings::defaultCheckerEnabledByDefault());
    d->ui.kcfg_backgroundCheckerEnabled->setChecked(Settings::defaultBackgroundCheckerEnabled());
    d->ui.ignoreListWidget->clear();
    d->ui.ignoreListWidget->addItems(Settings::defaultIgnoreList());
    d->ui.m_langCombo->setCurrentByDictionary(d->settings->defaultLanguage());
}

void ConfigWidget::setLanguage(const QString &language)
{
    d->ui.m_langCombo->setCurrentByDictionary(language);
}

QString ConfigWidget::language() const
{
    if (d->ui.m_langCombo->count()) {
        return d->ui.m_langCombo->currentDictionary();
    } else {
        return QString();
    }
}

#include "moc_configwidget.cpp"
