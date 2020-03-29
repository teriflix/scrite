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

#ifndef ABSTRACTIMPORTER_H
#define ABSTRACTIMPORTER_H

#include "abstractdeviceio.h"

class QIODevice;

class AbstractImporter : public AbstractDeviceIO
{
    Q_OBJECT

public:
    ~AbstractImporter();

    Q_INVOKABLE bool read();

protected:
    AbstractImporter(QObject *parent=nullptr);
    virtual bool doImport(QIODevice *device) = 0;
};

#endif // ABSTRACTIMPORTER_H
