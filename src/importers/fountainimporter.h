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

#ifndef FOUNTAINIMPORTER_H
#define FOUNTAINIMPORTER_H

#include "abstractimporter.h"

namespace Fountain {
class Parser;
}

class FountainImporter : public AbstractImporter
{
    Q_OBJECT
    // clang-format off
    Q_CLASSINFO("Format", "Fountain")
    Q_CLASSINFO("NameFilters", "Fountain (*.fountain *.txt)")
    // clang-format on

public:
    Q_INVOKABLE explicit FountainImporter(QObject *parent = nullptr);
    ~FountainImporter();

    bool canImport(const QString &fileName) const;

    bool importFromClipboard();

protected:
    bool doImport(QIODevice *device); // AbstractImporter interface
    bool doImport(const Fountain::Parser &parser);
};

#endif // FOUNTAINIMPORTER_H
