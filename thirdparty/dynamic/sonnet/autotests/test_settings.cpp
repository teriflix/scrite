/**
 * test_settings.cpp
 *
 * SPDX-FileCopyrightText: 2015 Kåre Särs <kare.sars@iki.fi>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "test_settings.h"

#include "settingsimpl_p.h"
#include "speller.h"

#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <QTest>

QTEST_GUILESS_MAIN(SonnetSettingsTest)

using namespace Sonnet;

void SonnetSettingsTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void SonnetSettingsTest::testRestoreDoesNotSave()
{
    QSettings settings(QStringLiteral("KDE"), QStringLiteral("Sonnet"));
    QString fileName = settings.fileName();

    QDateTime startTime = QFileInfo(fileName).lastModified();

    // NOTE: We use new/delete to be able to test that the settings are not
    // needlessly saved on deletion of Speller
    Speller *speller = new Speller();
    // NOTE: This test works on Unix, but should _not_ fail on Windows as
    // QFileInfo::lastModified() always returns invalid QDateTime
    QCOMPARE(QFileInfo(fileName).lastModified(), startTime);

    QCOMPARE(QFileInfo(fileName).lastModified(), startTime);

    QStringList langs = speller->availableLanguages();
    for (int i = 0; i < langs.count(); ++i) {
        speller->setLanguage(langs[i]);
    }
    QCOMPARE(QFileInfo(fileName).lastModified(), startTime);

    speller->availableLanguages();
    QCOMPARE(QFileInfo(fileName).lastModified(), startTime);

    speller->restore();
    QCOMPARE(QFileInfo(fileName).lastModified(), startTime);

    // Test that the settings are not saved needlessly on delete
    delete speller;
    QCOMPARE(QFileInfo(fileName).lastModified(), startTime);
}

void SonnetSettingsTest::testSpellerAPIChangeSaves()
{
    QSettings settings(QStringLiteral("KDE"), QStringLiteral("Sonnet"));
    Speller speller;

    // Default Language
    QString defLang = speller.defaultLanguage();
    QString settingsLang = settings.value(QStringLiteral("defaultLanguage"), QLocale::system().name()).toString();
    QCOMPARE(defLang, settingsLang);

    QStringList langs = speller.availableLanguages();
    for (int i = 0; i < langs.count(); ++i) {
        if (langs[i] != defLang) {
            speller.setDefaultLanguage(langs[i]);
            settingsLang = settings.value(QStringLiteral("defaultLanguage"), QLocale::system().name()).toString();
            QCOMPARE(settingsLang, langs[i]);
            QCOMPARE(speller.defaultLanguage(), langs[i]);
            break;
        }
    }
    // set the original value
    speller.setDefaultLanguage(defLang);
    settingsLang = settings.value(QStringLiteral("defaultLanguage"), QLocale::system().name()).toString();
    QCOMPARE(settingsLang, defLang);
    QCOMPARE(speller.defaultLanguage(), defLang);

    // Default Client
    QString defClient = speller.defaultClient();
    QString settingsClient = settings.value(QStringLiteral("defaultClient"), QString()).toString();
    QCOMPARE(defClient, settingsClient);

    QStringList clients = speller.availableBackends();
    qDebug() << clients;
    for (int i = 0; i < clients.count(); ++i) {
        if (clients[i] != defLang) {
            speller.setDefaultClient(clients[i]);
            settingsClient = settings.value(QStringLiteral("defaultClient"), QString()).toString();
            QCOMPARE(settingsClient, clients[i]);
            QCOMPARE(speller.defaultClient(), clients[i]);
            break;
        }
    }
    // set the original value
    if (defClient.isEmpty()) {
        // setting default to "" does not work.
        settings.remove(QStringLiteral("defaultClient"));
    } else {
        speller.setDefaultClient(defClient);
    }
    settingsClient = settings.value(QStringLiteral("defaultClient"), QString()).toString();
    QCOMPARE(settingsClient, defClient);
    if (!defClient.isEmpty()) {
        QCOMPARE(speller.defaultClient(), defClient);
    }

    // Check uppercase
    bool checkUppercase = speller.testAttribute(Speller::CheckUppercase);
    bool settingsUppercase = settings.value(QStringLiteral("checkUppercase"), true).toBool();
    QCOMPARE(checkUppercase, settingsUppercase);
    // Change the attribute
    speller.setAttribute(Speller::CheckUppercase, !checkUppercase);
    settingsUppercase = settings.value(QStringLiteral("checkUppercase"), true).toBool();
    QCOMPARE(!checkUppercase, settingsUppercase);
    QCOMPARE(!checkUppercase, speller.testAttribute(Speller::CheckUppercase));
    // now set it back to what it was
    speller.setAttribute(Speller::CheckUppercase, checkUppercase);
    settingsUppercase = settings.value(QStringLiteral("checkUppercase"), true).toBool();
    QCOMPARE(checkUppercase, settingsUppercase);
    QCOMPARE(checkUppercase, speller.testAttribute(Speller::CheckUppercase));

    // Skip Run Together
    bool skipRunTogether = speller.testAttribute(Speller::SkipRunTogether);
    bool settingsSkipRunTogether = settings.value(QStringLiteral("skipRunTogether"), true).toBool();
    QCOMPARE(skipRunTogether, settingsSkipRunTogether);
    // Change the attribute
    speller.setAttribute(Speller::SkipRunTogether, !skipRunTogether);
    settingsSkipRunTogether = settings.value(QStringLiteral("skipRunTogether"), true).toBool();
    QCOMPARE(!skipRunTogether, settingsSkipRunTogether);
    QCOMPARE(!skipRunTogether, speller.testAttribute(Speller::SkipRunTogether));
    // now set it back to what it was
    speller.setAttribute(Speller::SkipRunTogether, skipRunTogether);
    settingsSkipRunTogether = settings.value(QStringLiteral("skipRunTogether"), true).toBool();
    QCOMPARE(skipRunTogether, settingsSkipRunTogether);
    QCOMPARE(skipRunTogether, speller.testAttribute(Speller::SkipRunTogether));

    // Auto Detect Language
    bool autodetectLanguage = speller.testAttribute(Speller::AutoDetectLanguage);
    bool settingsAutoDetectLanguage = settings.value(QStringLiteral("autodetectLanguage"), true).toBool();
    QCOMPARE(autodetectLanguage, settingsAutoDetectLanguage);
    // Change the attribute
    speller.setAttribute(Speller::AutoDetectLanguage, !autodetectLanguage);
    settingsAutoDetectLanguage = settings.value(QStringLiteral("autodetectLanguage"), true).toBool();
    QCOMPARE(!autodetectLanguage, settingsAutoDetectLanguage);
    QCOMPARE(!autodetectLanguage, speller.testAttribute(Speller::AutoDetectLanguage));
    // now set it back to what it was
    speller.setAttribute(Speller::AutoDetectLanguage, autodetectLanguage);
    settingsAutoDetectLanguage = settings.value(QStringLiteral("autodetectLanguage"), true).toBool();
    QCOMPARE(autodetectLanguage, settingsAutoDetectLanguage);
    QCOMPARE(autodetectLanguage, speller.testAttribute(Speller::AutoDetectLanguage));
}

#include "moc_test_settings.cpp"
