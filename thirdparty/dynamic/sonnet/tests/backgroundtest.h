/*
 * backgroundtest.h
 *
 * SPDX-FileCopyrightText: 2004 Zack Rusin <zack@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef BACKGROUNDTEST_H
#define BACKGROUNDTEST_H

#include "backgroundchecker.h"
#include <QElapsedTimer>
#include <QObject>

class BackgroundTest : public QObject
{
    Q_OBJECT
public:
    BackgroundTest();

protected Q_SLOTS:
    void slotDone();
    void slotMisspelling(const QString &word, int start);

private:
    Sonnet::BackgroundChecker *m_checker;
    Sonnet::Speller m_speller;
    QElapsedTimer m_timer;
    int m_len;
};

#endif
