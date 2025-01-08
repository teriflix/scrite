/****************************************************************************
**
** Copyright (C) VCreate Logic Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth@scrite.io)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "clipboard.h"

#include <QClipboard>
#include <QGuiApplication>

Clipboard::Clipboard(QObject *parent) : QObject(parent)
{
    QClipboard *systemClipboard = qApp->clipboard();
    connect(systemClipboard, &QClipboard::dataChanged, this, &Clipboard::textChanged);
}

Clipboard::~Clipboard() { }

void Clipboard::setText(const QString &val)
{
    QClipboard *systemClipboard = qApp->clipboard();
    systemClipboard->setText(val);
}

QString Clipboard::text() const
{
    QClipboard *systemClipboard = qApp->clipboard();
    return systemClipboard->text();
}
