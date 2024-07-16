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

#include "abstractexporter.h"
#include "application.h"
#include "scrite.h"
#include "user.h"

#include <QBuffer>
#include <QClipboard>
#include <QScopeGuard>

AbstractExporter::AbstractExporter(QObject *parent) : AbstractDeviceIO(parent)
{
    m_languageBundleMap = TransliterationEngine::instance()->activeLanguages();
    connect(User::instance(), &User::infoChanged, this, &AbstractExporter::featureEnabledChanged);
}

AbstractExporter::~AbstractExporter()
{
    emit aboutToDelete(this);
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

bool AbstractExporter::isFeatureEnabled() const
{
    if (User::instance()->isLoggedIn()) {
        const bool allReportsEnabled = User::instance()->isFeatureEnabled(Scrite::ExportFeature);
        const bool thisSpecificReportEnabled = allReportsEnabled
                ? User::instance()->isFeatureNameEnabled(QStringLiteral("export/")
                                                         + this->formatName())
                : false;
        return allReportsEnabled && thisSpecificReportEnabled;
    }

    return this->formatName() == QStringLiteral("Adobe PDF"); // this is the only exporter we enable
                                                              // by default when not logged in.
}

QJsonObject AbstractExporter::configurationFormInfo() const
{
    return Application::instance()->objectConfigurationFormInfo(
            this, &AbstractExporter::staticMetaObject);
}

bool AbstractExporter::write(AbstractExporter::Target target)
{
    auto cleanup = qScopeGuard([=]() { GarbageCollector::instance()->add(this); });

    QString fileName = this->fileName();
    ScriteDocument *document = this->document();

    this->error()->clear();

    if (!this->isFeatureEnabled()) {
        this->error()->setErrorMessage(QStringLiteral("This exporter is not enabled."));
        return false;
    }

    QScopedPointer<QIODevice> device;

    if (target == FileTarget) {
        if (fileName.isEmpty()) {
            this->error()->setErrorMessage(QStringLiteral("Cannot export to an empty file."));
            return false;
        }

        if (document == nullptr) {
            this->error()->setErrorMessage(QStringLiteral("No document available to export."));
            return false;
        }

        device.reset(new QFile(fileName));
        if (!device->open(QFile::WriteOnly)) {
            this->error()->setErrorMessage(
                    QStringLiteral("Could not open file '%1' for writing.").arg(fileName));
            return false;
        }
    } else if (target == ClipboardTarget) {
        if (!this->canCopyToClipboard()) {
            this->error()->setErrorMessage(QStringLiteral("Export to clipboard is not supported."));
            return false;
        }

        device.reset(new QBuffer);
        if (!device->open(QBuffer::WriteOnly)) {
            this->error()->setErrorMessage(
                    QStringLiteral("Could not open clipboard for writing!."));
            return false;
        }
    }

    auto guard = qScopeGuard([=]() {
        const QString exporterName = QString::fromLatin1(this->metaObject()->className());
        User::instance()->logActivity2(QStringLiteral("export"), exporterName);
    });

    const QMetaObject *mo = this->metaObject();
    const QMetaClassInfo classInfo = mo->classInfo(mo->indexOfClassInfo("Format"));
    this->progress()->setProgressText(QStringLiteral("Generating \"%1\"").arg(classInfo.value()));

    this->progress()->start();
    const bool ret = this->doExport(device.get());
    this->progress()->finish();

    if (target == ClipboardTarget) {
        QBuffer *buffer = qobject_cast<QBuffer *>(device.get());
        if (buffer) {

            const QByteArray bytes = buffer->data();
            const QString text = QString::fromUtf8(bytes);

            QClipboard *clipboard = qApp->clipboard();
            clipboard->setText(text);
        } else {
            if (!device->open(QBuffer::WriteOnly)) {
                this->error()->setErrorMessage(
                        QStringLiteral("Could not copy exported contents to clipboard!."));
                return false;
            }
        }
    }

    return ret;
}
