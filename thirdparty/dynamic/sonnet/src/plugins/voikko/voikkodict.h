/*
 * voikkodict.h
 *
 * SPDX-FileCopyrightText: 2015 Jesse Jaara <jesse.jaara@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef SONNET_VOIKKODICT_H
#define SONNET_VOIKKODICT_H

#include "spellerplugin_p.h"
#include <libvoikko/voikko.h>

#include <QHash>
#include <QScopedPointer>

class VoikkoClient;
class VoikkoDictPrivate;

class VoikkoDict : public Sonnet::SpellerPlugin
{
public:
    /*!
     * Declare VoikkoClient as friend so we can use the protected constructor.
     */
    friend class VoikkoClient;

    ~VoikkoDict();

    bool isCorrect(const QString &word) const override;
    QStringList suggest(const QString &word) const override;

    bool storeReplacement(const QString &bad, const QString &good) override;
    bool addToPersonal(const QString &word) override;
    bool addToSession(const QString &word) override;

    /*!
     * Returns true if initializing Voikko backend failed.
     */
    bool initFailed() const Q_DECL_NOEXCEPT;

protected:
    /*!
     * Constructor is protected so that only spellers created
     * and validated through VoikkoClient can be used.
     */
    explicit VoikkoDict(const QString &language) Q_DECL_NOEXCEPT;

private:
    QScopedPointer<VoikkoDictPrivate> d;
};

#endif // SONNET_VOIKKODICT_H
