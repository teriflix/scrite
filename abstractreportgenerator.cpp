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

#include "abstractreportgenerator.h"
#include "qtextdocumentpagedprinter.h"

#include <QFileInfo>
#include <QPdfWriter>
#include <QJsonArray>
#include <QJsonObject>
#include <QMetaObject>
#include <QMetaClassInfo>
#include <QTextDocumentWriter>

AbstractReportGenerator::AbstractReportGenerator(QObject *parent)
                        :AbstractDeviceIO(parent)
{

}

AbstractReportGenerator::~AbstractReportGenerator()
{
    emit aboutToDelete(this);
}

void AbstractReportGenerator::setFormat(AbstractReportGenerator::Format val)
{
    if(m_format == val)
        return;

    m_format = val;
    emit formatChanged();
}

QString AbstractReportGenerator::name() const
{
    const int ciIndex = this->metaObject()->indexOfClassInfo("Title");
    if(ciIndex >= 0)
        return QString::fromLatin1(this->metaObject()->classInfo(ciIndex).value());

    return QString("Report");
}

bool AbstractReportGenerator::generate()
{
    QString fileName = this->fileName();
    ScriteDocument *document = this->document();
    Screenplay *screenplay = document->screenplay();
    ScreenplayFormat *format = document->printFormat();

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

    QTextDocument textDocument;

    textDocument.setDefaultFont(format->defaultFont());
    textDocument.setProperty("#title", screenplay->title());
    textDocument.setProperty("#subtitle", screenplay->subtitle());
    textDocument.setProperty("#author", screenplay->author());
    textDocument.setProperty("#contact", screenplay->contact());
    textDocument.setProperty("#version", screenplay->version());

    const QMetaObject *mo = this->metaObject();
    const QMetaClassInfo classInfo = mo->classInfo(mo->indexOfClassInfo("Title"));
    this->progress()->setProgressText( QString("Generating \"%1\"").arg(classInfo.value()));

    this->progress()->start();
    const bool ret = this->doGenerate(&textDocument);

    if(m_format == OpenDocumentFormat)
    {
        QTextDocumentWriter writer;
        writer.setFormat("ODF");
        writer.setDevice(&file);
        writer.write(&textDocument);
    }
    else
    {
        QPdfWriter pdfWriter(&file);
        pdfWriter.setTitle("Scrite Character Report");
        pdfWriter.setCreator(qApp->applicationName() + " " + qApp->applicationVersion());
        pdfWriter.setPageSize(QPageSize(QPageSize::A4));
        pdfWriter.setPageMargins(QMarginsF(0.2,0.1,0.2,0.1), QPageLayout::Inch);

        QTextDocumentPagedPrinter printer;
        printer.header()->setVisibleFromPageOne(true);
        printer.footer()->setVisibleFromPageOne(true);
        printer.watermark()->setVisibleFromPageOne(true);
        printer.print(&textDocument, &pdfWriter);
    }

    this->progress()->finish();

    m_success = true;

    GarbageCollector::instance()->add(this);

    return ret;
}

QJsonObject AbstractReportGenerator::configurationFormInfo() const
{
    const QMetaObject *mo = this->metaObject();
    auto queryClassInfo = [mo](const char *key) {
        const int ciIndex = mo->indexOfClassInfo(key);
        if(ciIndex < 0)
            return QString();
        const QMetaClassInfo ci = mo->classInfo(ciIndex);
        return QString::fromLatin1(ci.value());
    };

    auto queryPropertyInfo = [queryClassInfo](const QMetaProperty &prop, const char *key) {
        const QString ciKey = QString::fromLatin1(prop.name()) + "_" + QString::fromLatin1(key);
        return queryClassInfo(qPrintable(ciKey));
    };

    QJsonObject ret;
    ret.insert("title", queryClassInfo("Title"));

    QJsonArray fields;
    for(int i=AbstractReportGenerator::staticMetaObject.propertyOffset(); i<mo->propertyCount(); i++)
    {
        const QMetaProperty prop = mo->property(i);
        if(!prop.isWritable() || !prop.isStored())
            continue;

        QJsonObject field;
        field.insert("name", QString::fromLatin1(prop.name()));
        field.insert("label", queryPropertyInfo(prop, "FieldLabel"));
        field.insert("editor", queryPropertyInfo(prop, "FieldEditor"));
        fields.append(field);
    }

    ret.insert("fields", fields);

    return ret;
}

QString AbstractReportGenerator::polishFileName(const QString &fileName) const
{
    QFileInfo fi(fileName);
    switch(m_format)
    {
    case AdobePDF:
        if(fi.suffix().toLower() != "pdf")
            return fileName + ".pdf";
        break;
    case OpenDocumentFormat:
        if(fi.suffix().toLower() != "odt")
            return fileName + ".odt";
        break;
    }

    return fileName;
}
