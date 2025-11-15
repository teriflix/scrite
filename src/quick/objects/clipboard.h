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

#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <QQmlEngine>

class Clipboard : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    Clipboard(QObject *parent = nullptr);
    ~Clipboard();

    // clang-format off
    Q_PROPERTY(QString text
               READ text
               WRITE setText
               NOTIFY textChanged)
    // clang-format on
    void setText(const QString &val);
    QString text() const;
    Q_SIGNAL void textChanged();

    // clang-format off
    Q_PROPERTY(bool hasText
               READ hasHasText
               NOTIFY textChanged)
    // clang-format on
    bool hasHasText() const { return !this->text().isEmpty(); }
};

#endif // CLIPBOARD_H
