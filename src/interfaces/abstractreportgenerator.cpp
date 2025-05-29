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

#include "abstractreportgenerator.h"

#include "user.h"
#include "scrite.h"
#include "application.h"
#include "qtextdocumentpagedprinter.h"

#include <QDir>
#include <QPrinter>
#include <QFileInfo>
#include <QSettings>
#include <QPdfWriter>
#include <QJsonArray>
#include <QScopeGuard>
#include <QJsonObject>
#include <QMetaObject>
#include <QDomDocument>
#include <QMetaClassInfo>
#include <QTextDocumentWriter>

AbstractReportGenerator::AbstractReportGenerator(QObject *parent) : AbstractDeviceIO(parent)
{
    connect(User::instance(), &User::infoChanged, this,
            &AbstractReportGenerator::featureEnabledChanged);
}

AbstractReportGenerator::~AbstractReportGenerator()
{
    emit aboutToDelete(this);
}

void AbstractReportGenerator::setFormat(AbstractReportGenerator::Format val)
{
    if (m_format == val)
        return;

    m_format = val;
    emit formatChanged();

    if (!this->fileName().isEmpty()) {
        const QString suffix =
                m_format == AdobePDF ? QStringLiteral(".pdf") : QStringLiteral(".odt");
        const QFileInfo fileInfo(this->fileName());
        this->setFileName(
                fileInfo.absoluteDir().absoluteFilePath(fileInfo.completeBaseName() + suffix));
    }
}

QString AbstractReportGenerator::title() const
{
    const int cii = this->metaObject()->indexOfClassInfo("Title");
    return cii >= 0 ? QString::fromLatin1(this->metaObject()->classInfo(cii).value()) : QString();
}

QString AbstractReportGenerator::description() const
{
    const int cii = this->metaObject()->indexOfClassInfo("Description");
    return cii >= 0 ? QString::fromLatin1(this->metaObject()->classInfo(cii).value()) : QString();
}

QIcon AbstractReportGenerator::icon() const
{
    const int cii = this->metaObject()->indexOfClassInfo("Icon");
    return cii >= 0 ? QIcon(QString::fromLatin1(this->metaObject()->classInfo(cii).value()))
                    : QIcon();
}

bool AbstractReportGenerator::isFeatureEnabled() const
{
    if (User::instance()->isLoggedIn()) {
        const bool allReportsEnabled = AppFeature::isEnabled(Scrite::ReportFeature);
        const bool thisSpecificImporterEnabled = allReportsEnabled
                ? AppFeature::isEnabled(QStringLiteral("report/") + this->title())
                : false;
        return allReportsEnabled && thisSpecificImporterEnabled;
    }

    return false;
}

void AbstractReportGenerator::setWatermark(const QString &val)
{
    if (m_watermark == val)
        return;

    m_watermark = val;
    emit watermarkChanged();
}

void AbstractReportGenerator::setComment(const QString &val)
{
    if (m_comment == val)
        return;

    m_comment = val;
    emit commentChanged();
}

QString AbstractReportGenerator::name() const
{
    const int ciIndex = this->metaObject()->indexOfClassInfo("Title");
    if (ciIndex >= 0)
        return QString::fromLatin1(this->metaObject()->classInfo(ciIndex).value());

    return QString("Report");
}

bool AbstractReportGenerator::generate()
{
    auto cleanup = qScopeGuard([=]() { GarbageCollector::instance()->add(this); });

    QString fileName = this->fileName();
    ScriteDocument *document = this->document();
    Screenplay *screenplay = document->screenplay();
    ScreenplayFormat *format = document->printFormat();

    this->error()->clear();

    if (!this->isFeatureEnabled()) {
        this->error()->setErrorMessage(this->title() + QStringLiteral(" is disabled."));
        return false;
    }

    if (fileName.isEmpty()) {
        this->error()->setErrorMessage("Cannot export to an empty file.");
        return false;
    }

    if (document == nullptr) {
        this->error()->setErrorMessage("No document available to export.");
        return false;
    }

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        this->error()->setErrorMessage(
                QString("Could not open file '%1' for writing.").arg(fileName));
        return false;
    }

    auto guard = qScopeGuard([=]() {
        const QString reportName = QString::fromLatin1(this->metaObject()->className());
        User::instance()->logActivity2(QStringLiteral("report"), reportName);
    });

    const bool usePdfWriter = this->usePdfWriter();

    if (m_format == AdobePDF) {
        if (this->canDirectPrintToPdf()) {
            QScopedPointer<QPdfWriter> qpdfWriter;
            QScopedPointer<QPrinter> qprinter;
            bool success = false;

            this->progress()->start();

            if (usePdfWriter) {
                qpdfWriter.reset(new QPdfWriter(&file));
                qpdfWriter->setPdfVersion(QPagedPaintDevice::PdfVersion_1_6);
                qpdfWriter->setTitle(screenplay->title() + QStringLiteral(" - ") + this->name());
                qpdfWriter->setCreator(qApp->applicationName() + QStringLiteral(" ")
                                       + qApp->applicationVersion() + QStringLiteral(" PdfWriter"));
                format->pageLayout()->configure(qpdfWriter.data());
                qpdfWriter->setPageMargins(QMarginsF(0.2, 0.1, 0.2, 0.1), QPageLayout::Inch);
                success = this->directPrintToPdf(qpdfWriter.data());
            } else {
                file.close();

                qprinter.reset(new QPrinter);
                qprinter->setOutputFormat(QPrinter::PdfFormat);
                qprinter->setOutputFileName(fileName);
                qprinter->setPdfVersion(QPagedPaintDevice::PdfVersion_1_6);
                qprinter->setDocName(screenplay->title() + QStringLiteral(" - ") + this->name());
                qprinter->setCreator(qApp->applicationName() + QStringLiteral(" ")
                                     + qApp->applicationVersion() + QStringLiteral(" Printer"));
                format->pageLayout()->configure(qprinter.data());
                qprinter->setPageMargins(QMarginsF(0.2, 0.1, 0.2, 0.1), QPageLayout::Inch);
                success = this->directPrintToPdf(qprinter.data());
            }

            this->progress()->finish();

            return success;
        }
    }

    if (m_format == OpenDocumentFormat) {
        if (this->canDirectExportToOdf()) {
            this->progress()->start();
            bool success = this->directExportToOdf(&file);
            this->progress()->finish();
            return success;
        }
    }

    QTextDocument textDocument;

    textDocument.setDefaultFont(format->defaultFont());
    textDocument.setUseDesignMetrics(true);
    textDocument.setProperty("#title", screenplay->title());
    textDocument.setProperty("#subtitle", screenplay->subtitle());
    textDocument.setProperty("#author", screenplay->author());
    textDocument.setProperty("#contact", screenplay->contact());
    textDocument.setProperty("#version", screenplay->version());
    textDocument.setProperty("#phone", screenplay->phoneNumber());
    textDocument.setProperty("#email", screenplay->email());
    textDocument.setProperty("#website", screenplay->website());
    textDocument.setProperty("#comment", m_comment);
    textDocument.setProperty("#watermark", m_watermark);

    const QMetaObject *mo = this->metaObject();
    const QMetaClassInfo classInfo = mo->classInfo(mo->indexOfClassInfo("Title"));
    this->progress()->setProgressText(QString("Generating \"%1\"").arg(classInfo.value()));

    this->progress()->start();
    const bool ret = this->doGenerate(&textDocument);

    if (!ret) {
        this->progress()->finish();
        return ret;
    }

    if (m_format == OpenDocumentFormat) {
        QTextDocumentWriter writer;
        writer.setFormat("ODF");
        writer.setDevice(&file);
        this->configureWriter(&writer, &textDocument);
        writer.write(&textDocument);
        file.close();
        this->polishOdtContent(fileName);
    } else {
        QScopedPointer<QPdfWriter> qpdfWriter;
        QScopedPointer<QPrinter> qprinter;
        QPagedPaintDevice *pdfDevice = nullptr;

        if (usePdfWriter) {
            qpdfWriter.reset(new QPdfWriter(&file));
            qpdfWriter->setPdfVersion(QPagedPaintDevice::PdfVersion_1_6);
            qpdfWriter->setTitle(screenplay->title() + QStringLiteral(" - ") + this->name());
            qpdfWriter->setCreator(qApp->applicationName() + QStringLiteral(" ")
                                   + qApp->applicationVersion() + QStringLiteral(" PdfWriter"));
            format->pageLayout()->configure(qpdfWriter.data());
            qpdfWriter->setPageMargins(QMarginsF(0.2, 0.1, 0.2, 0.1), QPageLayout::Inch);
            this->configureWriter(qpdfWriter.data(), &textDocument);

            pdfDevice = qpdfWriter.data();
        } else {
            file.close();

            qprinter.reset(new QPrinter);
            qprinter->setOutputFormat(QPrinter::PdfFormat);
            qprinter->setOutputFileName(fileName);
            qprinter->setPdfVersion(QPagedPaintDevice::PdfVersion_1_6);
            qprinter->setDocName(screenplay->title() + QStringLiteral(" - ") + this->name());
            qprinter->setCreator(qApp->applicationName() + QStringLiteral(" ")
                                 + qApp->applicationVersion() + QStringLiteral(" Printer"));
            format->pageLayout()->configure(qprinter.data());
            qprinter->setPageMargins(QMarginsF(0.2, 0.1, 0.2, 0.1), QPageLayout::Inch);
            this->configureWriter(qprinter.data(), &textDocument);

            pdfDevice = qprinter.data();
        }

        QTextDocumentPagedPrinter printer;
        printer.header()->setVisibleFromPageOne(true);
        printer.footer()->setVisibleFromPageOne(true);
        printer.watermark()->setVisibleFromPageOne(true);
        this->configureTextDocumentPrinter(&printer, &textDocument);
        printer.print(&textDocument, pdfDevice);
    }

    this->progress()->finish();

    return ret;
}

bool AbstractReportGenerator::setConfigurationValue(const QString &name, const QVariant &value)
{
    const QMetaProperty prop =
            this->metaObject()->property(this->metaObject()->indexOfProperty(qPrintable(name)));
    if (!prop.isValid())
        return false;

    QVariant value2 = value;
    if (value2.userType() != prop.userType())
        value2.convert(prop.userType());

    return prop.write(this, value2);
}

QVariant AbstractReportGenerator::getConfigurationValue(const QString &name) const
{
    return this->property(qPrintable(name));
}

QJsonObject AbstractReportGenerator::configurationFormInfo() const
{
    QJsonObject formInfo = Application::instance()->objectConfigurationFormInfo(
            this, &AbstractReportGenerator::staticMetaObject);
    this->polishFormInfo(formInfo);
    return formInfo;
}

QString AbstractReportGenerator::fileNameExtension() const
{
    return m_format == AdobePDF ? QStringLiteral("pdf") : QStringLiteral("odt");
}

bool AbstractReportGenerator::usePdfWriter() const
{
#if 0
    const bool val = Application::instance()
                             ->settings()
                             ->value(QStringLiteral("PdfExport/usePdfDriver"), true)
                             .toBool();
    return val;
#else
    return false; // Qt 5.15.7's PdfWriter is broken!
#endif
}

void AbstractReportGenerator::polishOdtContent(const QString &fileName)
{
    if (m_format == OpenDocumentFormat && this->requiresOdtContentPolish()) {
        // Unzip contents
        const QFileInfo fi(fileName);

        QTemporaryDir tmpDir;
        tmpDir.setAutoRemove(true);

        if (!Scrite::doUnzip(fi, tmpDir)) {
            return;
        }

        // Load XML
        const QString contentXmlFilePath = tmpDir.filePath("content.xml");
        if (!QFile::exists(contentXmlFilePath))
            return;

        QFile contentXmlFile(contentXmlFilePath);
        if (!contentXmlFile.open(QFile::ReadOnly))
            return;

        QDomDocument contentXmlDom;

        QString errMsg;
        int errLine = -1, errCol = -1;
        if (!contentXmlDom.setContent(&contentXmlFile, &errMsg, &errLine, &errCol)) {
            return;
        }

        contentXmlFile.close();

        // Polish & Save
        if (!this->polishOdtContent(contentXmlDom))
            return;

        if (!contentXmlFile.open(QFile::WriteOnly))
            return;

        QTextStream ts(&contentXmlFile);
        ts.setCodec("utf-8");
        ts.setAutoDetectUnicode(true);
        ts << contentXmlDom.toString();
        ts.flush();

        contentXmlFile.close();

        // Zip Contents
        Scrite::doZip(fi, QDir(tmpDir.path()),
                      { qMakePair<QString, int>("mimetype", 0),
                        qMakePair<QString, int>("content.xml", -1),
                        qMakePair<QString, int>("META-INF/manifest.xml", -1) });
    }
}
