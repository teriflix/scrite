/****************************************************************************
**
** Copyright (C) 2024 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef OSFIMPORTER_H
#define OSFIMPORTER_H

#include <QDomDocument>
#include "abstractimporter.h"

class OsfImporter : public AbstractImporter
{
    Q_OBJECT
    // clang-format off
    Q_CLASSINFO("Format", "Open Screenplay Format")
    Q_CLASSINFO("NameFilters", "Open Screenplay Format (*.xml)")
    // clang-format on

public:
    Q_INVOKABLE explicit OsfImporter(QObject *parent = nullptr);
    ~OsfImporter();

    bool canImport(const QString &fileName) const;

protected:
    bool doImport(QIODevice *device); // AbstractImporter interface
};

#endif // OSFIMPORTER_H
