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

#include "abstractdeviceio.h"
#include "application.h"

#include <QFile>

AbstractDeviceIO::AbstractDeviceIO(QObject *parent) : QObject(parent), m_document(this, "document")
{
}

AbstractDeviceIO::~AbstractDeviceIO() { }

void AbstractDeviceIO::setFileName(const QString &val)
{
    QString val2 = val.trimmed();
    if (m_fileName == val2 || val2.isEmpty())
        return;

    m_fileName = this->polishFileName(val2);
    m_fileName = Application::instance()->sanitiseFileName(m_fileName);

    emit fileNameChanged();
}

void AbstractDeviceIO::setDocument(ScriteDocument *val)
{
    if (m_document == val)
        return;

    m_document = val;
    emit documentChanged();
}

void AbstractDeviceIO::resetDocument()
{
    this->setDocument(nullptr);
}
