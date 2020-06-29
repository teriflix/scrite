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
#include "fountainimporter.h"
#include "fountainexporter.h"
#include "structureexporter.h"
#include "qobjectserializer.h"
#include "finaldraftimporter.h"
#include "finaldraftexporter.h"
#include "locationreportgenerator.h"
#include "characterreportgenerator.h"
#include "scenecharactermatrixreportgenerator.h"

#include <QDir>
#include <QDateTime>
#include <QFileInfo>
#include <QSettings>
#include <QDateTime>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QScopedValueRollback>

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
    ImporterFactory.addClass<FountainImporter>();

    ExporterFactory.addClass<PdfExporter>();
    ExporterFactory.addClass<FinalDraftExporter>();
    ExporterFactory.addClass<HtmlExporter>();
    ExporterFactory.addClass<TextExporter>();
    ExporterFactory.addClass<StructureExporter>();
    ExporterFactory.addClass<FountainExporter>();
    ExporterFactory.addClass<OdtExporter>();

    ReportGeneratorFactory.addClass<CharacterReportGenerator>();
    ReportGeneratorFactory.addClass<LocationReportGenerator>();
    ReportGeneratorFactory.addClass<SceneCharacterMatrixReportGenerator>();
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
                :QObject(parent),
                  m_autoSaveTimer("ScriteDocument.m_autoSaveTimer"),
                  m_clearModifyTimer("ScriteDocument.m_clearModifyTimer"),
                  m_evaluateStructureElementSequenceTimer("ScriteDocument.m_evaluateStructureElementSequenceTimer")
{

    this->reset();
    this->updateDocumentWindowTitle();

    connect(this, &ScriteDocument::spellCheckIgnoreListChanged, this, &ScriteDocument::markAsModified);
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
    this->prepareAutoSave();
    emit autoSaveDurationInSecondsChanged();
}

void ScriteDocument::setAutoSave(bool val)
{
    if(m_autoSave == val)
        return;

    m_autoSave = val;
    Application::instance()->settings()->setValue("AutoSave/autoSaveEnabled", val);
    this->prepareAutoSave();
    emit autoSaveChanged();
}

void ScriteDocument::setBusy(bool val)
{
    if(m_busy == val)
        return;

    m_busy = val;
    emit busyChanged();

    if(val)
    {
        QElapsedTimer timer;
        timer.start();
        while(timer.elapsed() < 100)
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }
}

void ScriteDocument::setBusyMessage(const QString &val)
{
    if(m_busyMessage == val)
        return;

    m_busyMessage = val;
    emit busyMessageChanged();

    this->setBusy(!m_busyMessage.isEmpty());
}

void ScriteDocument::setSpellCheckIgnoreList(const QStringList &val)
{
    QStringList val2 = val.toSet().toList(); // so that we eliminate all duplicates
    std::sort(val2.begin(), val2.end());
    if(m_spellCheckIgnoreList == val)
        return;

    m_spellCheckIgnoreList = val;
    emit spellCheckIgnoreListChanged();
}

void ScriteDocument::addToSpellCheckIgnoreList(const QString &word)
{
    if(word.isEmpty() || m_spellCheckIgnoreList.contains(word))
        return;

    m_spellCheckIgnoreList.append(word);
    std::sort(m_spellCheckIgnoreList.begin(), m_spellCheckIgnoreList.end());
    emit spellCheckIgnoreListChanged();
}

Scene *ScriteDocument::createNewScene()
{
    QScopedValueRollback<bool> createNewSceneRollback(m_inCreateNewScene, true);

    StructureElement *structureElement = nullptr;
    if(m_structure->currentElementIndex() >= 0)
        structureElement = m_structure->elementAt(m_structure->currentElementIndex());
    else
        structureElement = m_structure->elementAt(m_structure->elementCount()-1);

    const qreal xOffset = (structureElement && structureElement->x() > 275) ? -275 : 275;
    const qreal x = structureElement ? (structureElement->x() + xOffset) : 100;
    const qreal y = structureElement ? (structureElement->y() + structureElement->height() + 50) : 50;

    Scene *activeScene = structureElement ? structureElement->scene() : nullptr;

    Scene *scene = new Scene(m_structure);
    scene->setColor(activeScene ? activeScene->color() : QColor("white"));
    scene->setTitle("[" + QString::number(m_structure->elementCount()+1) + "] - Scene");
    scene->heading()->setEnabled(true);
    scene->heading()->setLocationType(activeScene ? activeScene->heading()->locationType() : "EXT");
    scene->heading()->setLocation(activeScene ? activeScene->heading()->location() : "SOMEWHERE");
    scene->heading()->setMoment(activeScene ? "LATER" : "DAY");

    SceneElement *firstPara = new SceneElement(scene);
    firstPara->setType(SceneElement::Action);
    scene->addElement(firstPara);

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
        disconnect(m_screenplay, &Screenplay::screenplayChanged, this, &ScriteDocument::evaluateStructureElementSequenceLater);
    }

    if(m_formatting != nullptr)
        disconnect(m_formatting, &ScreenplayFormat::formatChanged, this, &ScriteDocument::markAsModified);

    if(m_printFormat != nullptr)
        disconnect(m_printFormat, &ScreenplayFormat::formatChanged, this, &ScriteDocument::markAsModified);

    UndoStack::clearAllStacks();
    m_docFileSystem.reset();

    if(m_formatting == nullptr)
        this->setFormatting(new ScreenplayFormat(this));
    else
        m_formatting->resetToDefaults();

    if(m_printFormat == nullptr)
        this->setPrintFormat(new ScreenplayFormat(this));
    else
        m_printFormat->resetToDefaults();

    this->setScreenplay(new Screenplay(this));
    this->setStructure(new Structure(this));
    this->setSpellCheckIgnoreList(QStringList());
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

    this->setBusyMessage("Loading " + QFileInfo(fileName).baseName() + " ...");
    this->reset();
    if( this->load(fileName) )
        this->setFileName(fileName);
    this->setModified(false);
    this->clearBusyMessage();
}

void ScriteDocument::saveAs(const QString &givenFileName)
{
    HourGlass hourGlass;
    QString fileName = this->polishFileName(givenFileName.trimmed());

    m_errorReport->clear();
    if(fileName.isEmpty())
        return;

    QFile file(fileName);
    if( !file.open(QFile::WriteOnly) )
    {
        m_errorReport->setErrorMessage( QString("Cannot open %1 for writing.").arg(fileName) );
        return;
    }

    file.close();

    if(!m_autoSaveMode)
        this->setBusyMessage("Saving to " + QFileInfo(fileName).baseName() + " ...");

    m_progressReport->start();

    const QJsonObject json = QObjectSerializer::toJson(this);
    const QByteArray bytes = QJsonDocument(json).toBinaryData();
    m_docFileSystem.setHeader(bytes);
    m_docFileSystem.save(fileName);

    this->setFileName(fileName);
    this->setModified(false);

#ifndef QT_NO_DEBUG
    {
        const QFileInfo fi(fileName);
        const QString fileName2 = fi.absolutePath() + "/" + fi.baseName() + ".json";
        QFile file2(fileName2);
        file2.open(QFile::WriteOnly);
        file2.write(QJsonDocument(json).toJson());
    }
#endif

    m_progressReport->finish();

    if(!m_autoSaveMode)
        this->clearBusyMessage();
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
    {
        Q_FOREACH(QByteArray key, keys) formats << key;
        std::sort(formats.begin(), formats.end());

        if(formats.size() >= 2)
        {
            QList<int> seps;
            for(int i=formats.size()-2; i>=0; i--)
            {
                QString thisFormat = formats.at(i);
                QString previousFormat = formats.at(i+1);
                if(thisFormat.split("/").first() != previousFormat.split("/").first())
                    seps << i+1;
            }

            Q_FOREACH(int sep, seps)
                formats.insert(sep, QString());
        }
    }
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

    this->setLoading(true);

    Aggregation aggregation;
    m_errorReport->setProxyFor(aggregation.findErrorReport(importer.data()));
    m_progressReport->setProxyFor(aggregation.findProgressReport(importer.data()));

    importer->setFileName(fileName);
    importer->setDocument(this);
    this->setBusyMessage("Importing from " + QFileInfo(fileName).fileName() + " ...");
    const bool success = importer->read();
    this->clearBusyMessage();

    this->setLoading(false);

    return success;
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
    this->setBusyMessage("Exporting to " + QFileInfo(fileName).fileName() + " ...");
    const bool ret = exporter->write();
    this->clearBusyMessage();

    return ret;
}

AbstractExporter *ScriteDocument::createExporter(const QString &format)
{
    const QByteArray formatKey = format.toLatin1();
    AbstractExporter *exporter = deviceIOFactories->ExporterFactory.create<AbstractExporter>(formatKey, this);
    if(exporter == nullptr)
        return nullptr;

    exporter->setDocument(this);

    if(exporter->fileName().isEmpty())
    {
        QString suggestedName = m_screenplay->title();
        if(suggestedName.isEmpty())
            suggestedName = QFileInfo(m_fileName).baseName();
        else if(!m_screenplay->subtitle().isEmpty())
            suggestedName = " " + m_screenplay->subtitle();
        if(suggestedName.isEmpty())
            suggestedName = "Scrite Screenplay";
        // suggestedName += " " + QDateTime::currentDateTime().toString(dateTimeFormat);
        suggestedName += " " + QString::number(QDateTime::currentSecsSinceEpoch());

        QFileInfo fi(m_fileName);
        if(fi.exists())
            exporter->setFileName( fi.absoluteDir().absoluteFilePath(fi.baseName() + " - " + suggestedName) );
        else
            exporter->setFileName( QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/" + suggestedName );
    }

    ProgressReport *progressReport = exporter->findChild<ProgressReport*>();
    if(progressReport)
    {
        connect(progressReport, &ProgressReport::statusChanged, [progressReport,this,exporter]() {
            if(progressReport->status() == ProgressReport::Started)
                this->setBusyMessage("Exporting into \"" + exporter->fileName() + "\" ...");
            else if(progressReport->status() == ProgressReport::Finished)
                this->clearBusyMessage();
        });
    }

    return exporter;
}

AbstractReportGenerator *ScriteDocument::createReportGenerator(const QString &report)
{
    const QByteArray reportKey = report.toLatin1();
    AbstractReportGenerator *reportGenerator = deviceIOFactories->ReportGeneratorFactory.create<AbstractReportGenerator>(reportKey, this);
    if(reportGenerator == nullptr)
        return nullptr;

    reportGenerator->setDocument(this);

    if(reportGenerator->fileName().isEmpty())
    {
        const QString reportName = reportGenerator->name();
        const QString suffix = reportGenerator->format() == AbstractReportGenerator::AdobePDF ? ".pdf" : ".odt";
        // const QString suggestedName = reportName + " - " + QDateTime::currentDateTime().toString(dateTimeFormat) + suffix;
        const QString suggestedName = reportName + " - " + QString::number(QDateTime::currentSecsSinceEpoch()) + suffix;

        QFileInfo fi(m_fileName);
        if(fi.exists())
            reportGenerator->setFileName( fi.absoluteDir().absoluteFilePath(fi.baseName() + " - " + suggestedName) );
        else
            reportGenerator->setFileName( QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/" + suggestedName );
    }

    ProgressReport *progressReport = reportGenerator->findChild<ProgressReport*>();
    if(progressReport)
    {
        connect(progressReport, &ProgressReport::statusChanged, [progressReport,this,reportGenerator]() {
            if(progressReport->status() == ProgressReport::Started)
                this->setBusyMessage("Generating \"" + reportGenerator->fileName() + "\" ...");
            else if(progressReport->status() == ProgressReport::Finished)
                this->clearBusyMessage();
        });
    }

    return reportGenerator;
}

void ScriteDocument::clearModified()
{
    if(m_screenplay->elementCount() == 0 && m_structure->elementCount() == 0)
        this->setModified(false);
}

void ScriteDocument::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_evaluateStructureElementSequenceTimer.timerId())
    {
        m_evaluateStructureElementSequenceTimer.stop();
        this->evaluateStructureElementSequence();
        return;
    }

    if(event->timerId() == m_autoSaveTimer.timerId())
    {
        if(m_modified && !m_fileName.isEmpty() && QFileInfo(m_fileName).isWritable())
        {
            QScopedValueRollback<bool> autoSave(m_autoSaveMode, true);
            this->save();
        }
        return;
    }

    if(event->timerId() == m_clearModifyTimer.timerId())
    {
        m_clearModifyTimer.stop();
        this->setModified(false);
        return;
    }

    QObject::timerEvent(event);
}

void ScriteDocument::setLoading(bool val)
{
    if(m_loading == val)
        return;

    m_loading = val;
    emit loadingChanged();
}

void ScriteDocument::prepareAutoSave()
{
    if(m_autoSave)
        m_autoSaveTimer.start(m_autoSaveDurationInSeconds*1000, this);
    else
        m_autoSaveTimer.stop();
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

    if(m_formatting != nullptr)
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

    if(m_formatting != nullptr)
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

    if((m_structure && m_structure->elementCount() > 0) || (m_screenplay && m_screenplay->elementCount() > 0))
        m_modified = val;
    else
        m_modified = false;

    emit modifiedChanged();
}

void ScriteDocument::setFileName(const QString &val)
{
    if(m_fileName == val)
        return;        

    m_fileName = this->polishFileName(val);
    emit fileNameChanged();
}

bool ScriteDocument::load(const QString &fileName)
{
    m_errorReport->clear();

    if( QFile(fileName).isReadable() )
    {
        m_errorReport->setErrorMessage( QString("Cannot open %1 for reading.").arg(fileName));
        return false;
    }

    bool loaded = this->classicLoad(fileName);
    if(!loaded)
        loaded = this->modernLoad(fileName);

    if(!loaded)
    {
        m_errorReport->setErrorMessage( QString("%1 is not a Scrite document.").arg(fileName) );
        return false;
    }

    struct LoadCleanup
    {
        LoadCleanup(ScriteDocument *doc)
            : m_document(doc) {
            m_document->m_errorReport->clear();
        }

        ~LoadCleanup() {
            if(m_loadBegun) {
                m_document->m_progressReport->finish();
                m_document->setLoading(false);
            } else
                m_document->m_docFileSystem.reset();
        }

        void begin() {
            m_loadBegun = true;
            m_document->m_progressReport->start();
            m_document->setLoading(true);
        }

    private:
        bool m_loadBegun = false;
        ScriteDocument *m_document;
    } loadCleanup(this);

    const QJsonDocument jsonDoc = QJsonDocument::fromBinaryData(m_docFileSystem.header());

#ifndef QT_NO_DEBUG
    {
        const QFileInfo fi(fileName);
        const QString fileName2 = fi.absolutePath() + "/" + fi.baseName() + ".json";
        QFile file2(fileName2);
        file2.open(QFile::WriteOnly);
        file2.write(jsonDoc.toJson());
    }
#endif

    const QJsonObject json = jsonDoc.object();
    if(json.isEmpty())
    {
        m_errorReport->setErrorMessage( QString("%1 is not a Scrite document.").arg(fileName) );
        return false;
    }

    const QJsonObject metaInfo = json.value("meta").toObject();
    if(metaInfo.value("appName").toString().toLower() != qApp->applicationName().toLower())
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

    loadCleanup.begin();

    UndoStack::ignoreUndoCommands = true;
    const bool ret = QObjectSerializer::fromJson(json, this);
    UndoStack::ignoreUndoCommands = false;
    UndoStack::clearAllStacks();

    // When we finish loading, QML begins lazy initialization of the UI
    // for displaying the document. In the process even a small 1/2 pixel
    // change in element location on the structure canvas for example,
    // causes this document to marked as modified. Which is a bummer for the user
    // who will notice that a document is marked as modified immediately after
    // loading it. So, we set this timer here to ensure that modified flag is
    // set to false after the QML UI has finished its lazy loading.
    m_clearModifyTimer.start(100, this);

    return ret;
}

bool ScriteDocument::classicLoad(const QString &fileName)
{
    if(fileName.isEmpty())
        return false;

    QFile file(fileName);
    if(!file.open(QFile::ReadOnly))
        return false;

    static const QByteArray classicMarker("qbjs");
    const QByteArray marker = file.read(classicMarker.length());
    if(marker != classicMarker)
        return false;

    file.seek(0);

    const QByteArray bytes = file.readAll();
    m_docFileSystem.setHeader(bytes);
    return true;
}

bool ScriteDocument::modernLoad(const QString &fileName)
{
    return m_docFileSystem.load(fileName);
}

void ScriteDocument::structureElementIndexChanged()
{
    if(m_screenplay == nullptr || m_structure == nullptr || m_syncingStructureScreenplayCurrentIndex || m_inCreateNewScene)
        return;

    QScopedValueRollback<bool> rollback(m_syncingStructureScreenplayCurrentIndex, true);

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
    if(m_screenplay == nullptr || m_structure == nullptr || m_syncingStructureScreenplayCurrentIndex || m_inCreateNewScene)
        return;

    QScopedValueRollback<bool> rollback(m_syncingStructureScreenplayCurrentIndex, true);

    ScreenplayElement *element = m_screenplay->elementAt(m_screenplay->currentElementIndex());
    if(element != nullptr)
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

    const QVector<QColor> versionColors = Application::standardColors(version);
    const QVector<QColor> newColors     = Application::standardColors(QVersionNumber());
    if(versionColors != newColors)
    {
        auto evalNewColor = [versionColors,newColors](const QColor &color) {
            const int oldColorIndex = versionColors.indexOf(color);
            const QColor newColor = oldColorIndex < 0 ? newColors.last() : newColors.at( oldColorIndex%newColors.size() );
            return newColor;
        };

        const int nrElements = m_structure->elementCount();
        for(int i=0; i<nrElements; i++)
        {
            StructureElement *element = m_structure->elementAt(i);
            Scene *scene = element->scene();
            if(scene == nullptr)
                continue;

            scene->setColor( evalNewColor(scene->color()) );

            const int nrNotes = scene->noteCount();
            for(int n=0; n<nrNotes; n++)
            {
                Note *note = scene->noteAt(n);
                note->setColor( evalNewColor(note->color()) );
            }
        }

        const int nrNotes = m_structure->noteCount();
        for(int n=0; n<nrNotes; n++)
        {
            Note *note = m_structure->noteAt(n);
            note->setColor( evalNewColor(note->color()) );
        }

        const int nrChars = m_structure->characterCount();
        for(int c=0; c<nrChars; c++)
        {
            Character *character = m_structure->characterAt(c);

            const int nrNotes = character->noteCount();
            for(int n=0; n<nrNotes; n++)
            {
                Note *note = character->noteAt(n);
                note->setColor( evalNewColor(note->color()) );
            }
        }
    }

    // With Version 0.3.9, we have completely changed the way in which we
    // store formatting options. So, the old formatting options data doesnt
    // work anymore. We better reset to defaults in the new version and then
    // let the user alter it anyway he sees fit.
    if( version <= QVersionNumber(0,3,9) )
    {
        m_formatting->resetToDefaults();
        m_printFormat->resetToDefaults();
    }
}

QString ScriteDocument::polishFileName(const QString &givenFileName) const
{
    QString fileName = givenFileName.trimmed();

    if(!fileName.isEmpty())
    {
        QFileInfo fi(fileName);
        if(fi.isDir())
            fileName = fi.absolutePath() + "/Screenplay-" + QString::number(QDateTime::currentSecsSinceEpoch()) + ".scrite";
        else if(fi.suffix() != "scrite")
            fileName += ".scrite";
    }

    return fileName;
}
