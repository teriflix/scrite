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

#include "abstractimporter.h"
#include <QFile>

AbstractImporter::AbstractImporter(QObject *parent)
                 :AbstractDeviceIO(parent)
{

}

AbstractImporter::~AbstractImporter()
{

}

bool AbstractImporter::read()
{
    QString fileName = this->fileName();
    ScriteDocument *document = this->document();

    this->error()->clear();

    if(fileName.isEmpty())
    {
        this->error()->setErrorMessage("Nothing to import.");
        return false;
    }

    if(document == nullptr)
    {
        this->error()->setErrorMessage("No document available to import into.");
        return false;
    }

    document->reset();

    QFile file(fileName);
    if( !file.open(QFile::ReadOnly) )
    {
        this->error()->setErrorMessage( QString("Could not open file '%1' for reading.").arg(fileName) );
        return false;
    }

    const QMetaObject *mo = this->metaObject();
    const QMetaClassInfo classInfo = mo->classInfo(mo->indexOfClassInfo("Format"));
    this->progress()->setProgressText( QString("Importing from \"%1\"").arg(classInfo.value()));

    this->progress()->start();
    const bool ret = this->doImport(&file);
    this->progress()->finish();

    return ret;
}
