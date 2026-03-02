/*
 * SPDX-FileCopyrightText: 2003 Zack Rusin <zack@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef SONNET_CLIENT_P_H
#define SONNET_CLIENT_P_H

#include <QObject>
#include <QString>
#include <QStringList>

#include "sonnetcore_export.h"
/*
 * The fact that this class inherits from QObject makes me
 * hugely unhappy. The reason for as of this writing is that
 * I don't really feel like writing my own KLibFactory
 * that would load anything else then QObject derivatives.
 */
namespace Sonnet
{
class SpellerPlugin;

/*!
 * \internal
 * Client
 */
class SONNETCORE_EXPORT Client : public QObject
{
    Q_OBJECT
public:
    /*!
     */
    explicit Client(QObject *parent = nullptr);

    /*!
     * Returns how reliable the answer is (higher is better).
     */
    virtual int reliability() const = 0;

    /*!
     * Returns a dictionary for the given language.
     *
     * \a language specifies the language of the dictionary. If an
     *        empty string is passed the default language will be
     *        used. Has to be one of the values returned by
     *        languages()
     *
     * Returns a dictionary for the language or 0 if there was an error.
     */
    virtual SpellerPlugin *createSpeller(const QString &language) = 0;

    /*!
     * Returns a list of supported languages.
     */
    virtual QStringList languages() const = 0;

    /*!
     * Returns the name of the implementing class.
     */
    virtual QString name() const = 0;
};
}

Q_DECLARE_INTERFACE(Sonnet::Client, "org.kde.sonnet.Client")

#endif // SONNET_CLIENT_P_H
