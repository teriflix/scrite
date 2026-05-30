/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef BOOLEANRESULT_H
#define BOOLEANRESULT_H

#include <QObject>
#include <QQmlEngine>

class BooleanResult : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Only for accessing created instances.")

public:
    BooleanResult(QObject *parent = nullptr) : QObject(parent) { }
    ~BooleanResult() { }

    // clang-format off
    Q_PROPERTY(bool value
               READ value
               WRITE setValue
               NOTIFY valueChanged)
    // clang-format on
    void setValue(bool val)
    {
        if (m_value == val)
            return;
        m_value = val;
        emit valueChanged();
    }
    bool value() const { return m_value; }
    Q_SIGNAL void valueChanged();

private:
    bool m_value = false;
};

#endif // BOOLEANRESULT_H
