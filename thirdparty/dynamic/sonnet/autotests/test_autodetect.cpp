/**
 * Copyright (C)  2019 Waqar Ahmed <waqar.17a@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#include "guesslanguage.h"
#include "speller.h"

#include <QObject>
#include <QStandardPaths>
#include <QTest>

class SonnetAutoDetectTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void autodetect_data();
    void autodetect();
    void benchDistance_data();
    void benchDistance();
};

using namespace Sonnet;

void SonnetAutoDetectTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);

    Speller speller(QStringLiteral("en_US"));

    if (speller.availableBackends().empty()) {
        QSKIP("No backends available");
    }

    if (!speller.availableBackends().contains(QLatin1String("Hunspell"))) {
        QSKIP("Hunspell not available");
    }

    speller.setDefaultClient(QStringLiteral("Hunspell"));
    speller.setAttribute(Speller::AutoDetectLanguage, true);
}

void SonnetAutoDetectTest::autodetect_data()
{
    QTest::addColumn<QString>("sentence");
    QTest::addColumn<QString>("correct_lang");
    QTest::addColumn<QStringList>("suggested_langs");

    QTest::newRow("English") << QStringLiteral("This is an English sentence.") << QStringLiteral("en_US")
                             << QStringList{QLatin1String("en_US"), QLatin1String("de_DE"), QLatin1String("ur_PK")};
    QTest::newRow("German") << QStringLiteral("Dies ist ein deutscher Satz.") << QStringLiteral("de_DE")
                            << QStringList{QLatin1String("hi_IN"), QLatin1String("pl_PL"), QLatin1String("de_DE_frami")};
    QTest::newRow("Malayam") << QStringLiteral("ഇന്ത്യയുടെ തെക്കു ഭാഗത്തു സ്ഥിതി ചെയ്യുന്ന ഒരു സംസ്ഥാനമാണ് കേരളം.") << QStringLiteral("ml_IN")
                             << QStringList{QLatin1String("ml_IN"), QLatin1String("ur_PK"), QLatin1String("en_US-large")};
}

void SonnetAutoDetectTest::autodetect()
{
    QFETCH(QString, sentence);
    QFETCH(QString, correct_lang);
    QFETCH(QStringList, suggested_langs);

    // skip if the language is not available
    Speller speller;
    if (!speller.availableLanguages().contains(correct_lang)) {
        const QString msg = correct_lang + QStringLiteral(" not available");
        QSKIP(msg.toLocal8Bit().constData());
    }

    Sonnet::GuessLanguage gl;
    QString actual_lang = gl.identify(sentence, suggested_langs);

    // get chars till _, get the language code
    int us = correct_lang.indexOf(QLatin1Char('_'));
    const QString correctLangCode = correct_lang.left(us);

    us = actual_lang.indexOf(QLatin1Char('_'));
    const QString actualLangCode = actual_lang.left(us);

    qDebug() << "Actual: " << actual_lang;
    qDebug() << "Expected: " << correct_lang;

    QCOMPARE(actualLangCode, correctLangCode);
}

void SonnetAutoDetectTest::benchDistance_data()
{
    QTest::addColumn<QString>("sentence");
    QTest::addColumn<QString>("correct_lang");
    QTest::addColumn<QStringList>("suggested_langs");

    QTest::newRow("English") << QStringLiteral("This is an English sentence.") << QStringLiteral("en_US")
                             << QStringList{QLatin1String("en_US"), QLatin1String("de_DE")};
    QTest::newRow("German") << QStringLiteral("Dies ist ein deutscher Satz.") << QStringLiteral("de_DE")
                            << QStringList{QLatin1String("pl_PL"), QLatin1String("de_DE_frami")};
    QTest::newRow("Malayam") << QStringLiteral("ഇന്ത്യയുടെ തെക്കു ഭാഗത്തു സ്ഥിതി ചെയ്യുന്ന ഒരു സംസ്ഥാനമാണ് കേരളം.") << QStringLiteral("ml_IN")
                             << QStringList{QLatin1String("ml_IN"), QLatin1String("en_US-large")};
}

void SonnetAutoDetectTest::benchDistance()
{
    QFETCH(QString, sentence);
    QFETCH(QString, correct_lang);
    QFETCH(QStringList, suggested_langs);

    Sonnet::GuessLanguage gl;

    QBENCHMARK {
        QString actual_lang = gl.identify(sentence, suggested_langs);
    }
}

QTEST_GUILESS_MAIN(SonnetAutoDetectTest)

#include "test_autodetect.moc"
