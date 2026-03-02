/*
 * configdialog.cpp
 *
 * SPDX-FileCopyrightText: 2004 Zack Rusin <zack@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "configdialog.h"
#include "configwidget.h"

#include <QDialogButtonBox>
#include <QVBoxLayout>

using namespace Sonnet;

class Sonnet::ConfigDialogPrivate
{
public:
    ConfigDialogPrivate(ConfigDialog *parent)
        : q(parent)
    {
    }

    ConfigWidget *ui = nullptr;
    ConfigDialog *const q;
    void slotConfigChanged();
};

void ConfigDialogPrivate::slotConfigChanged()
{
    Q_EMIT q->languageChanged(ui->language());
}

ConfigDialog::ConfigDialog(QWidget *parent)
    : QDialog(parent)
    , d(new ConfigDialogPrivate(this))
{
    setObjectName(QStringLiteral("SonnetConfigDialog"));
    setModal(true);
    setWindowTitle(tr("Spell Checking Configuration"));

    QVBoxLayout *layout = new QVBoxLayout(this);

    d->ui = new ConfigWidget(this);
    layout->addWidget(d->ui);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &ConfigDialog::slotOk);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(d->ui, SIGNAL(configChanged()), this, SLOT(slotConfigChanged()));

    connect(d->ui, &ConfigWidget::configChanged, this, &ConfigDialog::configChanged);
}

ConfigDialog::~ConfigDialog() = default;

void ConfigDialog::slotOk()
{
    d->ui->save();
    accept();
}

void ConfigDialog::slotApply()
{
    d->ui->save();
}

void ConfigDialog::setLanguage(const QString &language)
{
    d->ui->setLanguage(language);
}

QString ConfigDialog::language() const
{
    return d->ui->language();
}

#include "moc_configdialog.cpp"
