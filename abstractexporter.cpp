/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "abstractexporter.h"

AbstractExporter::AbstractExporter(QObject *parent)
                 :AbstractDeviceIO(parent)
{
    m_languageBundleMap = TransliterationEngine::instance()->activeLanguages();
}

AbstractExporter::~AbstractExporter()
{

}

QString AbstractExporter::format() const
{
    const int cii = this->metaObject()->indexOfClassInfo("Format");
    return QString::fromLatin1(this->metaObject()->classInfo(cii).value());
}

QString AbstractExporter::formatName() const
{
    const QStringList fields = this->format().split("/");
    return fields.last();
}

QString AbstractExporter::nameFilters() const
{
    const int cii = this->metaObject()->indexOfClassInfo("NameFilters");
    return QString::fromLatin1(this->metaObject()->classInfo(cii).value());
}

bool AbstractExporter::write()
{
    QString fileName = this->fileName();
    ScriteDocument *document = this->document();

    this->error()->clear();

    if(fileName.isEmpty())
    {
        this->error()->setErrorMessage("Cannot export to an empty file.");
        return false;
    }

    if(document == nullptr)
    {
        this->error()->setErrorMessage("No document available to export.");
        return false;
    }

    QFile file(fileName);
    if( !file.open(QFile::WriteOnly) )
    {
        this->error()->setErrorMessage( QString("Could not open file '%1' for writing.").arg(fileName) );
        return false;
    }

    const QMetaObject *mo = this->metaObject();
    const QMetaClassInfo classInfo = mo->classInfo(mo->indexOfClassInfo("Format"));
    this->progress()->setProgressText( QString("Generating \"%1\"").arg(classInfo.value()));

    this->progress()->start();
    const bool ret = this->doExport(&file);
    this->progress()->finish();

    GarbageCollector::instance()->add(this);

    return ret;
}
