/*
 * configwidget.cpp
 *
 * SPDX-FileCopyrightText: 2004 Zack Rusin <zack@kde.org>
 * SPDX-FileCopyrightText: 2020 Benjamin Port <benjamin.port@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "configview.h"
#include "ui_configui.h"

#include "ui_debug.h"

#include <QCheckBox>
#include <QLineEdit>
#include <QListWidget>

using namespace Sonnet;

class Sonnet::ConfigViewPrivate
{
public:
    explicit ConfigViewPrivate(ConfigView *v);
    Ui_SonnetConfigUI ui;
    QWidget *wdg = nullptr;
    QStringList ignoreList;
    ConfigView *q;
    void slotUpdateButton(const QString &text);
    void slotSelectionChanged();
    void slotIgnoreWordAdded();
    void slotIgnoreWordRemoved();
};

ConfigViewPrivate::ConfigViewPrivate(ConfigView *v)
{
    q = v;
}

void ConfigViewPrivate::slotUpdateButton(const QString &text)
{
    ui.addButton->setEnabled(!text.isEmpty());
}

void ConfigViewPrivate::slotSelectionChanged()
{
    ui.removeButton->setEnabled(!ui.ignoreListWidget->selectedItems().isEmpty());
}

void ConfigViewPrivate::slotIgnoreWordAdded()
{
    QString newWord = ui.newIgnoreEdit->text();
    ui.newIgnoreEdit->clear();
    if (newWord.isEmpty() || ignoreList.contains(newWord)) {
        return;
    }
    ignoreList.append(newWord);

    ui.ignoreListWidget->clear();
    ui.ignoreListWidget->addItems(ignoreList);

    Q_EMIT q->configChanged();
}

void ConfigViewPrivate::slotIgnoreWordRemoved()
{
    const QList<QListWidgetItem *> selectedItems = ui.ignoreListWidget->selectedItems();
    for (const QListWidgetItem *item : selectedItems) {
        ignoreList.removeAll(item->text());
    }

    ui.ignoreListWidget->clear();
    ui.ignoreListWidget->addItems(ignoreList);

    Q_EMIT q->configChanged();
}

ConfigView::ConfigView(QWidget *parent)
    : QWidget(parent)
    , d(new ConfigViewPrivate(this))
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setObjectName(QStringLiteral("SonnetConfigUILayout"));
    d->wdg = new QWidget(this);
    d->ui.setupUi(d->wdg);
    d->ui.languageList->setProperty("_breeze_force_frame", true);

    for (int i = 0; i < d->ui.m_langCombo->count(); i++) {
        const QString tag = d->ui.m_langCombo->itemData(i).toString();
        if (tag.isEmpty()) { // skip separator
            continue;
        }
        auto *item = new QListWidgetItem(d->ui.m_langCombo->itemText(i), d->ui.languageList);
        item->setData(Qt::UserRole, tag);
    }

    d->ui.kcfg_backgroundCheckerEnabled->hide(); // hidden by default

    connect(d->ui.addButton, &QAbstractButton::clicked, this, [this] {
        d->slotIgnoreWordAdded();
    });
    connect(d->ui.removeButton, &QAbstractButton::clicked, this, [this] {
        d->slotIgnoreWordRemoved();
    });

    layout->addWidget(d->wdg);
    connect(d->ui.newIgnoreEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        d->slotUpdateButton(text);
    });
    connect(d->ui.ignoreListWidget, &QListWidget::itemSelectionChanged, this, [this] {
        d->slotSelectionChanged();
    });
    d->ui.addButton->setEnabled(false);
    d->ui.removeButton->setEnabled(false);

    connect(d->ui.m_langCombo, &DictionaryComboBox::dictionaryChanged, this, &ConfigView::configChanged);
    connect(d->ui.languageList, &QListWidget::itemChanged, this, &ConfigView::configChanged);

    connect(d->ui.kcfg_backgroundCheckerEnabled, &QAbstractButton::clicked, this, &ConfigView::configChanged);
    connect(d->ui.kcfg_skipUppercase, &QAbstractButton::clicked, this, &ConfigView::configChanged);
    connect(d->ui.kcfg_skipRunTogether, &QAbstractButton::clicked, this, &ConfigView::configChanged);
    connect(d->ui.kcfg_checkerEnabledByDefault, &QAbstractButton::clicked, this, &ConfigView::configChanged);
    connect(d->ui.kcfg_autodetectLanguage, &QAbstractButton::clicked, this, &ConfigView::configChanged);
}

ConfigView::~ConfigView() = default;

void ConfigView::setNoBackendFoundVisible(bool show)
{
    d->ui.nobackendfound->setVisible(show);
}

bool ConfigView::noBackendFoundVisible() const
{
    return d->ui.nobackendfound->isVisible();
}

void ConfigView::setBackgroundCheckingButtonShown(bool b)
{
    d->ui.kcfg_backgroundCheckerEnabled->setVisible(b);
}

bool ConfigView::backgroundCheckingButtonShown() const
{
    return !d->ui.kcfg_backgroundCheckerEnabled->isHidden();
}

void ConfigView::setLanguage(const QString &language)
{
    d->ui.m_langCombo->setCurrentByDictionary(language);
}

QString ConfigView::language() const
{
    if (d->ui.m_langCombo->count()) {
        return d->ui.m_langCombo->currentDictionary();
    } else {
        return QString();
    }
}

void ConfigView::setPreferredLanguages(const QStringList &preferredLanguages)
{
    for (int i = 0; i < d->ui.languageList->count(); ++i) {
        QListWidgetItem *item = d->ui.languageList->item(i);
        QString tag = item->data(Qt::UserRole).toString();
        if (preferredLanguages.contains(tag)) {
            item->setCheckState(Qt::Checked);
        } else {
            item->setCheckState(Qt::Unchecked);
        }
    }
    Q_EMIT configChanged();
}

QStringList ConfigView::preferredLanguages() const
{
    QStringList preferredLanguages;
    for (int i = 0; i < d->ui.languageList->count(); i++) {
        if (d->ui.languageList->item(i)->checkState() == Qt::Unchecked) {
            continue;
        }
        preferredLanguages << d->ui.languageList->item(i)->data(Qt::UserRole).toString();
    }
    return preferredLanguages;
}

void ConfigView::setIgnoreList(const QStringList &ignoreList)
{
    d->ignoreList = ignoreList;
    d->ignoreList.sort();
    d->ui.ignoreListWidget->clear();
    d->ui.ignoreListWidget->addItems(d->ignoreList);
    Q_EMIT configChanged();
}

QStringList ConfigView::ignoreList() const
{
    return d->ignoreList;
}

#include "moc_configview.cpp"
