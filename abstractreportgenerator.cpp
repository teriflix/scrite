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

#include "application.h"
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

    if(m_format == AdobePDF)
    {
        if(this->canDirectPrintToPdf())
        {
            this->progress()->start();

            QPdfWriter pdfWriter(&file);
            pdfWriter.setTitle("Scrite Character Report");
            pdfWriter.setCreator(qApp->applicationName() + " " + qApp->applicationVersion());
            pdfWriter.setPageSize(QPageSize(QPageSize::Letter));
            pdfWriter.setPageMargins(QMarginsF(0.2,0.1,0.2,0.1), QPageLayout::Inch);

            const bool success = this->directPrintToPdf(&pdfWriter);

            this->progress()->finish();

            GarbageCollector::instance()->add(this);

            return success;
        }
    }

    QTextDocument textDocument;

    textDocument.setDefaultFont(format->defaultFont());
    textDocument.setProperty("#title", screenplay->title());
    textDocument.setProperty("#subtitle", screenplay->subtitle());
    textDocument.setProperty("#author", screenplay->author());
    textDocument.setProperty("#contact", screenplay->contact());
    textDocument.setProperty("#version", screenplay->version());
    textDocument.setProperty("#phone", screenplay->phoneNumber());
    textDocument.setProperty("#email", screenplay->email());
    textDocument.setProperty("#website", screenplay->website());

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
        this->configureWriter(&writer, &textDocument);
        writer.write(&textDocument);
    }
    else
    {
        QPdfWriter pdfWriter(&file);
        pdfWriter.setTitle("Scrite Character Report");
        pdfWriter.setCreator(qApp->applicationName() + " " + qApp->applicationVersion());
        pdfWriter.setPageSize(QPageSize(QPageSize::Letter));
        pdfWriter.setPageMargins(QMarginsF(0.2,0.1,0.2,0.1), QPageLayout::Inch);
        this->configureWriter(&pdfWriter, &textDocument);

        QTextDocumentPagedPrinter printer;
        printer.header()->setVisibleFromPageOne(true);
        printer.footer()->setVisibleFromPageOne(true);
        printer.watermark()->setVisibleFromPageOne(true);
        printer.print(&textDocument, &pdfWriter);
    }

    this->progress()->finish();

    GarbageCollector::instance()->add(this);

    return ret;
}

bool AbstractReportGenerator::setConfigurationValue(const QString &name, const QVariant &value)
{
    return this->setProperty(qPrintable(name),value);
}

QVariant AbstractReportGenerator::getConfigurationValue(const QString &name) const
{
    return this->property(qPrintable(name));
}

QJsonObject AbstractReportGenerator::configurationFormInfo() const
{
    return Application::instance()->objectConfigurationFormInfo(this, &AbstractReportGenerator::staticMetaObject);
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
