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

#ifndef HTMLIMPORTER_H
#define HTMLIMPORTER_H

#include <QDomDocument>
#include "abstractimporter.h"

class HtmlImporter : public AbstractImporter
{
    Q_OBJECT
    Q_CLASSINFO("Format", "HTML")
    Q_CLASSINFO("NameFilters", "HTML (*.html)")

public:
    Q_INVOKABLE explicit HtmlImporter(QObject *parent = nullptr);
    ~HtmlImporter();

    bool canImport(const QString &fileName) const;

protected:
    bool doImport(QIODevice *device); // AbstractImporter interface
    QByteArray preprocess(QIODevice *device) const;
    bool importFrom(const QByteArray &bytes);
};

#endif // CELTXHTMLIMPORTER_H
