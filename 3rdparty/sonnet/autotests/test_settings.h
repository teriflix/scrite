/*
 * SPDX-FileCopyrightText: 2015 Kåre Särs <kare.sars@iki.fi>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef TEST_SETTINGS_H
#define TEST_SETTINGS_H

#include <QObject>

class SonnetSettingsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testRestoreDoesNotSave();
    void testSpellerAPIChangeSaves();
};

#endif
