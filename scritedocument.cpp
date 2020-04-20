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

#include "scritedocument.h"

#include "logger.h"
#include "undoredo.h"
#include "hourglass.h"
#include "aggregation.h"
#include "application.h"
#include "pdfexporter.h"
#include "odtexporter.h"
#include "htmlexporter.h"
#include "textexporter.h"
#include "htmlimporter.h"
#include "qobjectfactory.h"
#include "structureexporter.h"
#include "qobjectserializer.h"
#include "finaldraftimporter.h"
#include "finaldraftexporter.h"
#include "locationreportgenerator.h"
#include "characterreportgenerator.h"

#include <QDir>
#include <QDateTime>
#include <QFileInfo>
#include <QSettings>
#include <QDateTime>
#include <QJsonDocument>
#include <QStandardPaths>

class DeviceIOFactories
{
public:
    DeviceIOFactories();
    ~DeviceIOFactories();

    QObjectFactory ImporterFactory;
    QObjectFactory ExporterFactory;
    QObjectFactory ReportGeneratorFactory;
};

DeviceIOFactories::DeviceIOFactories()
    : ImporterFactory("Format"),
      ExporterFactory("Format"),
      ReportGeneratorFactory("Title")
{
    ImporterFactory.addClass<FinalDraftImporter>();
    ImporterFactory.addClass<HtmlImporter>();

    ExporterFactory.addClass<PdfExporter>();
    ExporterFactory.addClass<FinalDraftExporter>();
    ExporterFactory.addClass<HtmlExporter>();
    ExporterFactory.addClass<TextExporter>();
    ExporterFactory.addClass<StructureExporter>();
    // ExporterFactory.addClass<OdtExporter>();

    ReportGeneratorFactory.addClass<CharacterReportGenerator>();
    ReportGeneratorFactory.addClass<LocationReportGenerator>();
}

DeviceIOFactories::~DeviceIOFactories()
{
}

Q_GLOBAL_STATIC(DeviceIOFactories, deviceIOFactories)

ScriteDocument *ScriteDocument::instance()
{
    static ScriteDocument *theInstance = new ScriteDocument(qApp);
    return theInstance;
}

ScriteDocument::ScriteDocument(QObject *parent)
                :QObject(parent)
{
    this->reset();
    this->updateDocumentWindowTitle();
    connect(this, &ScriteDocument::modifiedChanged, this, &ScriteDocument::updateDocumentWindowTitle);
    connect(this, &ScriteDocument::fileNameChanged, this, &ScriteDocument::updateDocumentWindowTitle);

    const QVariant ase = Application::instance()->settings()->value("AutoSave/autoSaveEnabled");
    this->setAutoSave( ase.isValid() ? ase.toBool() : m_autoSave );

    const QVariant asd = Application::instance()->settings()->value("AutoSave/autoSaveInterval");
    this->setAutoSaveDurationInSeconds( asd.isValid() ? asd.toInt() : m_autoSaveDurationInSeconds );

    this->prepareAutoSave();
}

ScriteDocument::~ScriteDocument()
{

}

void ScriteDocument::setAutoSaveDurationInSeconds(int val)
{
    val = qBound(1, val, 3600);
    if(m_autoSaveDurationInSeconds == val)
        return;

    m_autoSaveDurationInSeconds = val;
    Application::instance()->settings()->setValue("AutoSave/autoSaveInterval", val);
    emit autoSaveDurationInSecondsChanged();

    Logger::qtPropertyInfo(this, "autoSaveDurationInSeconds");
}

void ScriteDocument::setAutoSave(bool val)
{
    if(m_autoSave == val)
        return;

    m_autoSave = val;
    Application::instance()->settings()->setValue("AutoSave/autoSaveEnabled", val);
    emit autoSaveChanged();

    Logger::qtPropertyInfo(this, "autoSave");
}

Scene *ScriteDocument::createNewScene()
{
    StructureElement *structureElement = nullptr;
    if(m_structure->currentElementIndex() > 0)
        structureElement = m_structure->elementAt(m_structure->currentElementIndex());
    else
        structureElement = m_structure->elementAt(m_structure->elementCount()-1);

    const qreal xOffset = m_structure->elementCount()%2 ? 275 : -275;
    const qreal x = structureElement ? (structureElement->x() + xOffset) : 225;
    const qreal y = structureElement ? (structureElement->y() + structureElement->height() + 100) : 100;

    Scene *activeScene = structureElement ? structureElement->scene() : nullptr;

    Scene *scene = new Scene(m_structure);
    scene->setColor(activeScene ? activeScene->color() : QColor("blue"));
    scene->setTitle("[" + QString::number(m_structure->elementCount()+1) + "] - Scene");
    scene->heading()->setEnabled(true);
    scene->heading()->setLocationType(activeScene ? activeScene->heading()->locationType() : "EXT");
    scene->heading()->setLocation(activeScene ? activeScene->heading()->location() : "SOMEWHERE");
    scene->heading()->setMoment(activeScene ? activeScene->heading()->moment() : "DAY");

    StructureElement *newStructureElement = new StructureElement(m_structure);
    newStructureElement->setScene(scene);
    newStructureElement->setX(x);
    newStructureElement->setY(y);
    m_structure->addElement(newStructureElement);

    ScreenplayElement *newScreenplayElement = new ScreenplayElement(m_screenplay);
    newScreenplayElement->setScene(scene);
    int newScreenplayElementIndex = -1;
    if(m_screenplay->currentElementIndex() >= 0)
    {
        newScreenplayElementIndex = m_screenplay->currentElementIndex()+1;
        m_screenplay->insertElementAt(newScreenplayElement, m_screenplay->currentElementIndex()+1);
    }
    else
    {
        newScreenplayElementIndex = m_screenplay->elementCount();
        m_screenplay->addElement(newScreenplayElement);
    }

    if(m_screenplay->elementAt(newScreenplayElementIndex) != newScreenplayElement)
        newScreenplayElementIndex = m_screenplay->indexOfElement(newScreenplayElement);

    m_structure->setCurrentElementIndex(m_structure->elementCount()-1);
    m_screenplay->setCurrentElementIndex(newScreenplayElementIndex);

    emit newSceneCreated(scene, newScreenplayElementIndex);

    scene->setUndoRedoEnabled(true);
    return scene;
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

    if(m_printFormat != nullptr)
        disconnect(m_printFormat, &ScreenplayFormat::formatChanged, this, &ScriteDocument::markAsModified);

    UndoStack::clearAllStacks();

    this->setFormatting(new ScreenplayFormat(this));
    this->setPrintFormat(new ScreenplayFormat(this));
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
    connect(m_printFormat, &ScreenplayFormat::formatChanged, this, &ScriteDocument::markAsModified);
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

    const QJsonObject json = QObjectSerializer::toJson(this);
    const QByteArray bytes = QJsonDocument(json).toBinaryData();

    file.write(bytes);

    this->setFileName(fileName);
    this->setModified(false);

    m_progressReport->finish();
}

void ScriteDocument::save()
{
    QFileInfo fi(m_fileName);
    if(fi.exists())
    {
        const QString backupDirPath(fi.absolutePath() + "/" + fi.baseName() + " Backups");
        QDir().mkpath(backupDirPath);
        const QString backupFileName = backupDirPath + "/" + fi.baseName() + " [" + QString::number(QDateTime::currentSecsSinceEpoch()) + "].scrite";
        QFile::copy(m_fileName, backupFileName);
    }

    this->saveAs(m_fileName);
}

QStringList ScriteDocument::supportedImportFormats() const
{
    static QList<QByteArray> keys = deviceIOFactories->ImporterFactory.keys();
    static QStringList formats;
    if(formats.isEmpty())
        Q_FOREACH(QByteArray key, keys) formats << key;
    return formats;
}

QString ScriteDocument::importFormatFileSuffix(const QString &format) const
{
    const QMetaObject *mo = deviceIOFactories->ImporterFactory.find(format.toLatin1());
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
    static QList<QByteArray> keys = deviceIOFactories->ExporterFactory.keys();
    static QStringList formats;
    if(formats.isEmpty())
        Q_FOREACH(QByteArray key, keys) formats << key;
    return formats;
}

QString ScriteDocument::exportFormatFileSuffix(const QString &format) const
{
    const QMetaObject *mo = deviceIOFactories->ExporterFactory.find(format.toLatin1());
    if(mo == nullptr)
        return QString();

    const int ciIndex = mo->indexOfClassInfo("NameFilters");
    if(ciIndex < 0)
        return QString();

    const QMetaClassInfo classInfo = mo->classInfo(ciIndex);
    return QString::fromLatin1(classInfo.value());
}

QStringList ScriteDocument::supportedReports() const
{
    static QList<QByteArray> keys = deviceIOFactories->ReportGeneratorFactory.keys();
    static QStringList reports;
    if(reports.isEmpty())
        Q_FOREACH(QByteArray key, keys) reports << key;
    return reports;
}

QString ScriteDocument::reportFileSuffix() const
{
    return QString("Adobe PDF (*.pdf)");
}

bool ScriteDocument::importFile(const QString &fileName, const QString &format)
{
    HourGlass hourGlass;

    m_errorReport->clear();

    const QByteArray formatKey = format.toLatin1();
    QScopedPointer<AbstractImporter> importer( deviceIOFactories->ImporterFactory.create<AbstractImporter>(formatKey, this) );

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
    QScopedPointer<AbstractExporter> exporter( deviceIOFactories->ExporterFactory.create<AbstractExporter>(formatKey, this) );

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

AbstractReportGenerator *ScriteDocument::createReportGenerator(const QString &report)
{
    const QByteArray reportKey = report.toLatin1();
    AbstractReportGenerator *reportGenerator = deviceIOFactories->ReportGeneratorFactory.create<AbstractReportGenerator>(reportKey, this);
    if(reportGenerator && reportGenerator->fileName().isEmpty())
    {
        reportGenerator->setDocument(this);

        const QString reportName = reportGenerator->name();
        const QString suffix = reportGenerator->format() == AbstractReportGenerator::AdobePDF ? ".pdf" : ".odt";
        const QString suggestedName = reportName + " - " + QDateTime::currentDateTime().toString("d MMM yyyy, hh:mm") + suffix;

        QFileInfo fi(m_fileName);
        if(fi.exists())
            reportGenerator->setFileName( fi.absoluteDir().absoluteFilePath(fi.baseName() + " - " + suggestedName) );
        else
            reportGenerator->setFileName( QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/" + suggestedName );
    }

    return reportGenerator;
}

void ScriteDocument::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_evaluateStructureElementSequenceTimer.timerId())
    {
        this->evaluateStructureElementSequence();
        m_evaluateStructureElementSequenceTimer.stop();
        return;
    }

    if(event->timerId() == m_autoSaveTimer.timerId())
    {
        if(m_modified && !m_fileName.isEmpty())
        {
            Logger::qtInfo(this, QString("Auto saving to %1").arg(m_fileName));
            this->save();
        }
    }

    QObject::timerEvent(event);
}

void ScriteDocument::prepareAutoSave()
{
    if(m_autoSave)
        m_autoSaveTimer.start(m_autoSaveDurationInSeconds*1000, this);
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

void ScriteDocument::setPrintFormat(ScreenplayFormat *val)
{
    if(m_printFormat == val)
        return;

    if(m_printFormat != nullptr)
        m_printFormat->deleteLater();

    m_printFormat = val;
    m_printFormat->setParent(this);

    emit printFormatChanged();
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

            fromElement = toElement;
            fromIndex = toIndex;
        }
        else
        {
            fromElement = nullptr;
            fromIndex = -1;
        }

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

    QFile file(fileName);
    if( !file.open(QFile::ReadOnly) )
    {
        m_errorReport->setErrorMessage( QString("Cannot open %1 for reading.").arg(fileName));
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

    m_fileName = fileName;
    emit fileNameChanged();

    m_progressReport->start();

    UndoStack::ignoreUndoCommands = true;
    const bool ret = QObjectSerializer::fromJson(json, this);
    UndoStack::ignoreUndoCommands = false;
    UndoStack::clearAllStacks();

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

void ScriteDocument::prepareForSerialization()
{
    // Nothing to do
}

void ScriteDocument::prepareForDeserialization()
{
    // Nothing to do
}

bool ScriteDocument::canSerialize(const QMetaObject *, const QMetaProperty &) const
{
    return true;
}

void ScriteDocument::serializeToJson(QJsonObject &json) const
{
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

    json.insert("meta", metaInfo);
}

void ScriteDocument::deserializeFromJson(const QJsonObject &json)
{
    const QJsonObject metaInfo = json.value("meta").toObject();
    const QString appVersion = metaInfo.value("appVersion").toString();
    const QVersionNumber version = QVersionNumber::fromString(appVersion);
    if( version <= QVersionNumber(0,1,9) )
    {
        const qreal dx = -130;
        const qreal dy = -22;

        const int nrElements = m_structure->elementCount();
        for(int i=0; i<nrElements; i++)
        {
            StructureElement *element = m_structure->elementAt(i);
            element->setX( element->x()+dx );
            element->setY( element->y()+dy );
        }
    }

    if( version <= QVersionNumber(0,2,6) )
    {
        const int nrElements = m_structure->elementCount();
        for(int i=0; i<nrElements; i++)
        {
            StructureElement *element = m_structure->elementAt(i);
            Scene *scene = element->scene();
            if(scene == nullptr)
                continue;

            SceneHeading *heading = scene->heading();
            QString val = heading->locationType();
            if(val == "INTERIOR")
                val = "INT";
            if(val == "EXTERIOR")
                val = "EXT";
            if(val == "BOTH")
                val = "I/E";
            heading->setLocationType(val);
        }
    }

    if(m_screenplay->currentElementIndex() < 0)
    {
        if(m_screenplay->elementCount() > 0)
            m_screenplay->setCurrentElementIndex(0);
        else if(m_structure->elementCount() > 0)
            m_structure->setCurrentElementIndex(0);
    }
}
