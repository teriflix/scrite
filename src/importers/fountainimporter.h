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

#ifndef FOUNTAINIMPORTER_H
#define FOUNTAINIMPORTER_H

#include "abstractimporter.h"

class FountainImporter : public AbstractImporter
{
    Q_OBJECT
    Q_CLASSINFO("Format", "Fountain")
    Q_CLASSINFO("NameFilters", "Fountain (*.fountain *.txt)")

public:
    Q_INVOKABLE explicit FountainImporter(QObject *parent = nullptr);
    ~FountainImporter();

    bool canImport(const QString &fileName) const;

protected:
    bool doImport(QIODevice *device); // AbstractImporter interface
    void preprocess(QByteArray &bytes);
};

#endif // FOUNTAINIMPORTER_H
