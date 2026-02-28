/*
    SPDX-FileCopyrightText: 2019 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSPELL_ISPELLCHECKDICT_H
#define KSPELL_ISPELLCHECKDICT_H

#include "spellerplugin_p.h"

#include "ispellcheckerclient.h"

class ISpellCheckerDict : public Sonnet::SpellerPlugin
{
public:
    explicit ISpellCheckerDict(ISpellChecker *spellChecker, const QString &language);
    ~ISpellCheckerDict() override;
    bool isCorrect(const QString &word) const override;

    QStringList suggest(const QString &word) const override;

    bool storeReplacement(const QString &bad, const QString &good) override;

    bool addToPersonal(const QString &word) override;
    bool addToSession(const QString &word) override;

private:
    // spell checker com object, we don't own this
    ISpellChecker *const m_spellChecker;
};

#endif
