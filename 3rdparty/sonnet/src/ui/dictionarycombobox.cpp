/*
 * SPDX-FileCopyrightText: 2003 Ingo Kloecker <kloecker@kde.org>
 * SPDX-FileCopyrightText: 2008 Tom Albers <tomalbers@kde.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "dictionarycombobox.h"

#include "ui_debug.h"
#include <speller.h>

namespace Sonnet
{
//@cond PRIVATE
class DictionaryComboBoxPrivate
{
public:
    explicit DictionaryComboBoxPrivate(DictionaryComboBox *combo)
        : q(combo)
    {
    }

    DictionaryComboBox *const q;
    void slotDictionaryChanged(int idx);
};

void DictionaryComboBoxPrivate::slotDictionaryChanged(int idx)
{
    Q_EMIT q->dictionaryChanged(q->itemData(idx).toString());
    Q_EMIT q->dictionaryNameChanged(q->itemText(idx));
}

//@endcon

DictionaryComboBox::DictionaryComboBox(QWidget *parent)
    : QComboBox(parent)
    , d(new DictionaryComboBoxPrivate(this))
{
    reloadCombo();
    connect(this, SIGNAL(activated(int)), SLOT(slotDictionaryChanged(int)));
}

DictionaryComboBox::~DictionaryComboBox() = default;

QString DictionaryComboBox::currentDictionaryName() const
{
    return currentText();
}

QString DictionaryComboBox::currentDictionary() const
{
    return itemData(currentIndex()).toString();
}

bool DictionaryComboBox::assignDictionnaryName(const QString &name)
{
    if (name.isEmpty() || name == currentText()) {
        return false;
    }

    int idx = findText(name);
    if (idx == -1) {
        qCDebug(SONNET_LOG_UI) << "name not found" << name;
        return false;
    }

    setCurrentIndex(idx);
    d->slotDictionaryChanged(idx);
    return true;
}

void DictionaryComboBox::setCurrentByDictionaryName(const QString &name)
{
    assignDictionnaryName(name);
}

bool DictionaryComboBox::assignByDictionnary(const QString &dictionary)
{
    if (dictionary.isEmpty()) {
        return false;
    }
    if (dictionary == itemData(currentIndex()).toString()) {
        return true;
    }

    int idx = findData(dictionary);
    if (idx == -1) {
        qCDebug(SONNET_LOG_UI) << "dictionary not found" << dictionary;
        return false;
    }

    setCurrentIndex(idx);
    d->slotDictionaryChanged(idx);
    return true;
}

void DictionaryComboBox::setCurrentByDictionary(const QString &dictionary)
{
    assignByDictionnary(dictionary);
}

void DictionaryComboBox::reloadCombo()
{
    clear();
    Sonnet::Speller speller;
    QMap<QString, QString> preferredDictionaries = speller.preferredDictionaries();
    QMapIterator<QString, QString> i(preferredDictionaries);
    while (i.hasNext()) {
        i.next();
        addItem(i.key(), i.value());
    }
    if (count()) {
        insertSeparator(count());
    }

    QMap<QString, QString> dictionaries = speller.availableDictionaries();
    i = dictionaries;
    while (i.hasNext()) {
        i.next();
        if (preferredDictionaries.contains(i.key())) {
            continue;
        }
        addItem(i.key(), i.value());
    }
}

} // namespace Sonnet

#include "moc_dictionarycombobox.cpp"
