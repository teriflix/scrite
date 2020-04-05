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

#ifndef ABSTRACTTEXTDOCUMENTEXPORTER_H
#define ABSTRACTTEXTDOCUMENTEXPORTER_H

#include "abstractexporter.h"

class AbstractTextDocumentExporter : public AbstractExporter
{
    Q_OBJECT

public:
    ~AbstractTextDocumentExporter();

protected:
    AbstractTextDocumentExporter(QObject *parent=nullptr);

    void generate(QTextDocument *textDocument, const qreal pageWidth);
};

#endif // ABSTRACTTEXTDOCUMENTEXPORTER_H
