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

#include "application.h"
#include "scritedocument.h"
#include "qobjectserializer.h"
#include "finaldraftimporter.h"
#include "finaldraftexporter.h"
#include "pdfexporter.h"
#include "htmlexporter.h"
#include "textexporter.h"
#include "aggregation.h"
#include "qobjectfactory.h"
#include "hourglass.h"

#include <QFileInfo>
#include <QJsonDocument>

class DeviceIOFactories
{
public:
    DeviceIOFactories();
    ~DeviceIOFactories();

    QObjectFactory ImporterFactory;
    QObjectFactory ExporterFactory;
};

DeviceIOFactories::DeviceIOFactories()
    : ImporterFactory("Format"),
      ExporterFactory("Format")
{
    ImporterFactory.addClass<FinalDraftImporter>();

    ExporterFactory.addClass<PdfExporter>();
    ExporterFactory.addClass<FinalDraftExporter>();
    ExporterFactory.addClass<HtmlExporter>();
    ExporterFactory.addClass<TextExporter>();
}

DeviceIOFactories::~DeviceIOFactories()
{
}

static DeviceIOFactories deviceIOFactories;

ScriteDocument::ScriteDocument(QObject *parent)
                :QObject(parent),
                  m_screenplay(nullptr),
                  m_structure(nullptr),
                  m_formatting(nullptr),
                  m_modified(false),
                  m_errorReport(new ErrorReport(this)),
                  m_progressReport(new ProgressReport(this))
{
    this->reset();
    this->updateDocumentWindowTitle();
    connect(this, &ScriteDocument::modifiedChanged, this, &ScriteDocument::updateDocumentWindowTitle);
    connect(this, &ScriteDocument::fileNameChanged, this, &ScriteDocument::updateDocumentWindowTitle);
}

ScriteDocument::~ScriteDocument()
{

}

void ScriteDocument::reset()
{
    HourGlass hourGlass;

    if(m_structure != nullptr)
    {
        disconnect(m_structure, &Structure::currentElementIndexChanged, this, &ScriteDocument::structureElementIndexChanged);
        disconnect(m_structure, &Structure::structureChanged, this, &ScriteDocument::markAsModified);
    }

    if(m_screenplay != nullptr)
    {
        disconnect(m_screenplay, &Screenplay::currentElementIndexChanged, this, &ScriteDocument::screenplayElementIndexChanged);
        disconnect(m_screenplay, &Screenplay::screenplayChanged, this, &ScriteDocument::markAsModified);
    }

    if(m_formatting != nullptr)
        disconnect(m_formatting, &ScreenplayFormat::formatChanged, this, &ScriteDocument::markAsModified);

    this->setFormatting(new ScreenplayFormat(this));
    this->setScreenplay(new Screenplay(this));
    this->setStructure(new Structure(this));
    this->setModified(false);
    this->setFileName(QString());
    this->evaluateStructureElementSequence();

    connect(m_structure, &Structure::currentElementIndexChanged, this, &ScriteDocument::structureElementIndexChanged);
    connect(m_screenplay, &Screenplay::currentElementIndexChanged, this, &ScriteDocument::screenplayElementIndexChanged);
    connect(m_screenplay, &Screenplay::screenplayChanged, this, &ScriteDocument::markAsModified);
    connect(m_screenplay, &Screenplay::screenplayChanged, this, &ScriteDocument::evaluateStructureElementSequenceLater);
    connect(m_structure, &Structure::structureChanged, this, &ScriteDocument::markAsModified);
    connect(m_formatting, &ScreenplayFormat::formatChanged, this, &ScriteDocument::markAsModified);
}

void ScriteDocument::open(const QString &fileName)
{
    HourGlass hourGlass;

    this->reset();
    if( this->load(fileName) )
        this->setFileName(fileName);
    this->setModified(false);
}

void ScriteDocument::saveAs(const QString &fileName)
{
    HourGlass hourGlass;

    m_errorReport->clear();
    if(fileName.isEmpty())
        return;

    QFile file(fileName);
    if( !file.open(QFile::WriteOnly) )
    {
        m_errorReport->setErrorMessage( QString("Cannot open %1 for writing.").arg(fileName) );
        return;
    }

    m_progressReport->start();

    QJsonObject metaInfo;
    metaInfo.insert("appName", qApp->applicationName());
    metaInfo.insert("orgName", qApp->organizationName());
    metaInfo.insert("orgDomain", qApp->organizationDomain());
    metaInfo.insert("appVersion", Application::instance()->versionNumber().toString());

    QJsonObject systemInfo;
    systemInfo.insert("machineHostName", QSysInfo::machineHostName());
    systemInfo.insert("machineUniqueId", QString::fromLatin1(QSysInfo::machineUniqueId()));
    systemInfo.insert("prettyProductName", QSysInfo::prettyProductName());
    systemInfo.insert("productType", QSysInfo::productType());
    systemInfo.insert("productVersion", QSysInfo::productVersion());
    metaInfo.insert("system", systemInfo);

    QJsonObject json = QObjectSerializer::toJson(this);
    json.insert("meta", metaInfo);

    const QByteArray bytes = QJsonDocument(json).toBinaryData();

    file.write(bytes);

    this->setFileName(fileName);
    this->setModified(false);

    m_progressReport->finish();
}

void ScriteDocument::save()
{
    this->saveAs(m_fileName);
}

QStringList ScriteDocument::supportedImportFormats() const
{
    static QList<QByteArray> keys = deviceIOFactories.ImporterFactory.keys();
    static QStringList formats;
    if(formats.isEmpty())
        Q_FOREACH(QByteArray key, keys) formats << key;
    return formats;
}

QString ScriteDocument::importFormatFileSuffix(const QString &format) const
{
    const QMetaObject *mo = deviceIOFactories.ImporterFactory.find(format.toLatin1());
    if(mo == nullptr)
        return QString();

    const int ciIndex = mo->indexOfClassInfo("NameFilters");
    if(ciIndex < 0)
        return QString();

    const QMetaClassInfo classInfo = mo->classInfo(ciIndex);
    return QString::fromLatin1(classInfo.value());
}

QStringList ScriteDocument::supportedExportFormats() const
{
    static QList<QByteArray> keys = deviceIOFactories.ExporterFactory.keys();
    static QStringList formats;
    if(formats.isEmpty())
        Q_FOREACH(QByteArray key, keys) formats << key;
    return formats;
}

QString ScriteDocument::exportFormatFileSuffix(const QString &format) const
{
    const QMetaObject *mo = deviceIOFactories.ExporterFactory.find(format.toLatin1());
    if(mo == nullptr)
        return QString();

    const int ciIndex = mo->indexOfClassInfo("NameFilters");
    if(ciIndex < 0)
        return QString();

    const QMetaClassInfo classInfo = mo->classInfo(ciIndex);
    return QString::fromLatin1(classInfo.value());
}

bool ScriteDocument::importFile(const QString &fileName, const QString &format)
{
    HourGlass hourGlass;

    m_errorReport->clear();

    const QByteArray formatKey = format.toLatin1();
    QScopedPointer<AbstractImporter> importer( deviceIOFactories.ImporterFactory.create<AbstractImporter>(formatKey, this) );

    if(importer.isNull())
    {
        m_errorReport->setErrorMessage("Cannot import from this format.");
        return false;
    }

    Aggregation aggregation;
    m_errorReport->setProxyFor(aggregation.findErrorReport(importer.data()));
    m_progressReport->setProxyFor(aggregation.findProgressReport(importer.data()));

    importer->setFileName(fileName);
    importer->setDocument(this);
    if( importer->read() )
        return true;

    return false;
}

bool ScriteDocument::exportFile(const QString &fileName, const QString &format)
{
    HourGlass hourGlass;

    m_errorReport->clear();

    const QByteArray formatKey = format.toLatin1();
    QScopedPointer<AbstractExporter> exporter( deviceIOFactories.ExporterFactory.create<AbstractExporter>(formatKey, this) );

    if(exporter.isNull())
    {
        m_errorReport->setErrorMessage("Cannot export to this format.");
        return false;
    }

    Aggregation aggregation;
    m_errorReport->setProxyFor(aggregation.findErrorReport(exporter.data()));
    m_progressReport->setProxyFor(aggregation.findProgressReport(exporter.data()));

    exporter->setFileName(fileName);
    exporter->setDocument(this);
    if( exporter->write() )
        return true;

    return false;
}

void ScriteDocument::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_evaluateStructureElementSequenceTimer.timerId())
    {
        this->evaluateStructureElementSequence();
        m_evaluateStructureElementSequenceTimer.stop();
        return;
    }

    QObject::timerEvent(event);
}

void ScriteDocument::updateDocumentWindowTitle()
{
    QString title = "[";
    if(m_fileName.isEmpty())
        title += "noname";
    else
        title += m_fileName;
    if(m_modified)
        title += " *";
    title += "] " + qApp->property("baseWindowTitle").toString();
    this->setDocumentWindowTitle(title);
}

void ScriteDocument::setDocumentWindowTitle(const QString &val)
{
    if(m_documentWindowTitle == val)
        return;

    m_documentWindowTitle = val;
    emit documentWindowTitleChanged(m_documentWindowTitle);
}

void ScriteDocument::setStructure(Structure *val)
{
    if(m_structure == val)
        return;

    if(m_structure != nullptr)
        m_structure->deleteLater();

    m_structure = val;
    m_structure->setParent(this);

    emit structureChanged();
}

void ScriteDocument::setScreenplay(Screenplay *val)
{
    if(m_screenplay == val)
        return;

    if(m_screenplay != nullptr)
        m_screenplay->deleteLater();

    m_screenplay = val;
    m_screenplay->setParent(this);

    emit screenplayChanged();
}

void ScriteDocument::setFormatting(ScreenplayFormat *val)
{
    if(m_formatting == val)
        return;

    if(m_formatting != nullptr)
        m_formatting->deleteLater();

    m_formatting = val;
    m_formatting->setParent(this);

    emit formattingChanged();
}

void ScriteDocument::evaluateStructureElementSequence()
{
    if(m_structure == nullptr || m_screenplay == nullptr)
    {
        if(!m_structureElementSequence.isEmpty())
        {
            m_structureElementSequence = QJsonArray();
            emit structureElementSequenceChanged();
        }

        return;
    }

    QJsonArray array;

    const int nrElements = m_screenplay->elementCount();
    if(nrElements == 0)
    {
        if(!m_structureElementSequence.isEmpty())
        {
            m_structureElementSequence = array;
            emit structureElementSequenceChanged();
        }

        return;
    }

    ScreenplayElement *fromElement = nullptr;
    ScreenplayElement *toElement = nullptr;
    int fromIndex = -1;
    int toIndex = -1;

    for(int i=0; i<nrElements-1; i++)
    {
        fromElement = fromElement ? fromElement : m_screenplay->elementAt(i);
        toElement = toElement ? toElement : m_screenplay->elementAt(i+1);
        fromIndex = fromIndex >= 0 ? fromIndex : m_structure->indexOfScene(fromElement->scene());
        toIndex = toIndex >= 0 ? toIndex : m_structure->indexOfScene(toElement->scene());

        if(fromIndex >= 0 && toIndex >= 0)
        {
            QJsonObject item;
            item.insert("from", fromIndex);
            item.insert("to", toIndex);
            array.append(item);
        }

        fromElement = toElement;
        fromIndex = toIndex;
        toElement = nullptr;
        toIndex = -1;
    }

    m_structureElementSequence = array;
    emit structureElementSequenceChanged();
}

void ScriteDocument::evaluateStructureElementSequenceLater()
{
    m_evaluateStructureElementSequenceTimer.start(100, this);
}

void ScriteDocument::setModified(bool val)
{
    if(m_modified == val)
        return;

    m_modified = val;
    emit modifiedChanged();
}

void ScriteDocument::setFileName(const QString &val)
{
    if(m_fileName == val)
        return;        

    m_fileName = val.trimmed();

    if(!m_fileName.isEmpty())
    {
        QFileInfo fi(m_fileName);
        if(!fi.isDir() && !fi.baseName().isEmpty() && fi.suffix().isEmpty())
            m_fileName += ".scrite";
    }

    emit fileNameChanged();
}

bool ScriteDocument::load(const QString &fileName)
{
    m_errorReport->clear();

    m_fileName = fileName;

    QFile file(m_fileName);
    if( !file.open(QFile::ReadOnly) )
    {
        m_errorReport->setErrorMessage( QString("Cannot open %1 for writing.").arg(fileName));
        return false;
    }

    const QByteArray bytes = file.readAll();
    const QJsonDocument jsonDoc = QJsonDocument::fromBinaryData(bytes);
    const QJsonObject json = jsonDoc.object();
    if(json.isEmpty())
    {
        m_errorReport->setErrorMessage(QString("Scrite document was not found in %1").arg(fileName));
        return false;
    }

    const QJsonObject metaInfo = json.value("meta").toObject();
    if(metaInfo.value("appName").toString() != qApp->applicationName())
    {
        m_errorReport->setErrorMessage(QString("Scrite document '%1' was created using an unrecognised app.").arg(fileName));
        return false;
    }

    const QVersionNumber docVersion = QVersionNumber::fromString( metaInfo.value("appVersion").toString() );
    const QVersionNumber appVersion = Application::instance()->versionNumber();
    if(appVersion < docVersion)
    {
        m_errorReport->setErrorMessage(QString("Scrite document '%1' was created using an updated version.").arg(fileName));
        return false;
    }

    m_progressReport->start();
    static QObjectFactory scriteFactory;
    if(scriteFactory.isEmpty())
        scriteFactory.addClass<Scene>();
    const bool ret = QObjectSerializer::fromJson(json, this, &scriteFactory);
    m_progressReport->finish();

    return ret;
}

void ScriteDocument::structureElementIndexChanged()
{
    if(m_screenplay == nullptr || m_structure == nullptr)
        return;

    StructureElement *element = m_structure->elementAt(m_structure->currentElementIndex());
    if(element == nullptr)
    {
        m_screenplay->setActiveScene(nullptr);
        m_screenplay->setCurrentElementIndex(-1);
    }
    else
        m_screenplay->setActiveScene(element->scene());
}

void ScriteDocument::screenplayElementIndexChanged()
{
    if(m_screenplay == nullptr || m_structure == nullptr)
        return;

    ScreenplayElement *element = m_screenplay->elementAt(m_screenplay->currentElementIndex());
    if(element == nullptr)
        m_structure->setCurrentElementIndex(-1);
    else
    {
        int index = m_structure->indexOfScene(element->scene());
        m_structure->setCurrentElementIndex(index);
    }
}
