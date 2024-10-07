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

#include "session.h"

Session::Session(QObject *parent) { }

Session::~Session() { }

void Session::set(const QString &name, const QVariant &value)
{
    if (m_variables.value(name) != value) {
        if (value.isValid())
            m_variables.insert(name, value);
        else
            m_variables.remove(name);
        emit changed(name, value);
    }
}

QVariant Session::get(const QString &name) const
{
    return m_variables.value(name);
}

void Session::unset(const QString &name)
{
    this->set(name, QVariant());
}
