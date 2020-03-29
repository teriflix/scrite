/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef ABSTRACTEXPORTER_H
#define ABSTRACTEXPORTER_H

#include "abstractdeviceio.h"

class AbstractExporter : public AbstractDeviceIO
{
    Q_OBJECT

public:
    ~AbstractExporter();

    Q_INVOKABLE bool write();

protected:
    AbstractExporter(QObject *parent=nullptr);
    virtual bool doExport(QIODevice *device) = 0;
};

#endif // ABSTRACTEXPORTER_H
