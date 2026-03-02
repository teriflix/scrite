/*
 * SPDX-FileCopyrightText: 2003 Zack Rusin <zack@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "client_p.h"

using namespace Sonnet;

Client::Client(QObject *parent)
    : QObject(parent)
{
}

#include "moc_client_p.cpp"
