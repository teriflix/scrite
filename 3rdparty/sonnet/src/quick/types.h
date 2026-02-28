// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QQmlEngine>

#include <Sonnet/Settings>

/*!
 * \qmltype Settings
 * \inqmlmodule org.kde.sonnet
 * \nativetype Sonnet::Settings
 */
struct SettingsForeign {
    Q_GADGET
    QML_ELEMENT
    QML_NAMED_ELEMENT(Settings)
    QML_FOREIGN(Sonnet::Settings)
};
