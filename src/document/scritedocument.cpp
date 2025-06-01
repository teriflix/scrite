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

#include "scritedocument.h"

#include "form.h"
#include "user.h"
#include "scrite.h"
#include "undoredo.h"
#include "fountain.h"
#include "hourglass.h"
#include "callgraph.h"
#include "filelocker.h"
#include "restapicall.h"
#include "aggregation.h"
#include "application.h"
#include "pdfexporter.h"
#include "odtexporter.h"
#include "localstorage.h"
#include "htmlexporter.h"
#include "textexporter.h"
#include "notification.h"
#include "htmlimporter.h"
#include "notebookreport.h"
#include "qobjectfactory.h"
#include "locationreport.h"
#include "twocolumnreport.h"
#include "characterreport.h"
#include "statisticsreport.h"
#include "fountainimporter.h"
#include "fountainexporter.h"
#include "qobjectserializer.h"
#include "finaldraftimporter.h"
#include "finaldraftexporter.h"
#include "screenplaysubsetreport.h"
#include "filemodificationtracker.h"
#include "qtextdocumentpagedprinter.h"
#include "characterscreenplayreport.h"
#include "scenecharactermatrixreport.h"

#include <QDir>
#include <QUuid>
#include <QFuture>
#include <QPainter>
#include <QMimeData>
#include <QDateTime>
#include <QFileInfo>
#include <QSettings>
#include <QDateTime>
#include <QClipboard>
#include <QScopeGuard>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QFutureWatcher>
#include <QStandardPaths>
#include <QtConcurrentRun>
#include <QRandomGenerator>
#include <QFileSystemWatcher>
#include <QScopedValueRollback>

ScriteDocumentBackups::ScriteDocumentBackups(QObject *parent) : QAbstractListModel(parent)
{
    m_reloadTimer.setSingleShot(true);
    m_reloadTimer.setInterval(50);
    connect(&m_reloadTimer, &QTimer::timeout, this,
            &ScriteDocumentBackups::reloadBackupFileInformation);
}

ScriteDocumentBackups::~ScriteDocumentBackups() { }

QJsonObject ScriteDocumentBackups::at(int index) const
{
    QJsonObject ret;

    if (index < 0 || index >= m_backupFiles.size())
        return ret;

    const QModelIndex idx = this->index(index, 0, QModelIndex());

    const QHash<int, QByteArray> roles = this->roleNames();
    auto it = roles.begin();
    auto end = roles.end();
    while (it != end) {
        ret.insert(QString::fromLatin1(it.value()),
                   QJsonValue::fromVariant(this->data(idx, it.key())));
        ++it;
    }

    return ret;
}

int ScriteDocumentBackups::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_backupFiles.size();
}

QVariant ScriteDocumentBackups::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_backupFiles.size())
        return QVariant();

    const QFileInfo fi = m_backupFiles.at(index.row());
    switch (role) {
    case TimestampRole:
        return fi.birthTime().toMSecsSinceEpoch();
    case TimestampAsStringRole:
        return fi.birthTime().toString();
    case Qt::DisplayRole:
    case FileNameRole:
        return fi.completeBaseName();
    case FilePathRole:
        return fi.absoluteFilePath();
    case RelativeTimeRole:
        return relativeTime(fi.birthTime());
    case FileSizeRole:
        return fi.size();
    case MetaDataRole:
        if (!m_metaDataList.at(index.row()).loaded)
            (const_cast<ScriteDocumentBackups *>(this))->loadMetaData(index.row());
        return m_metaDataList.at(index.row()).toJson();
    }

    return QVariant();
}

QHash<int, QByteArray> ScriteDocumentBackups::roleNames() const
{
    static QHash<int, QByteArray> roles = { { TimestampRole, QByteArrayLiteral("timestamp") },
                                            { TimestampAsStringRole,
                                              QByteArrayLiteral("timestampAsString") },
                                            { RelativeTimeRole, QByteArrayLiteral("relativeTime") },
                                            { FileNameRole, QByteArrayLiteral("fileName") },
                                            { FilePathRole, QByteArrayLiteral("filePath") },
                                            { FileSizeRole, QByteArrayLiteral("fileSize") },
                                            { MetaDataRole, QByteArrayLiteral("metaData") } };
    return roles;
}

QString ScriteDocumentBackups::relativeTime(const QDateTime &dt)
{
    return Application::relativeTime(dt);
}

void ScriteDocumentBackups::setDocumentFilePath(const QString &val)
{
    if (m_documentFilePath == val)
        return;

    m_documentFilePath = val;
    emit documentFilePathChanged();

    this->loadBackupFileInformation();
}

void ScriteDocumentBackups::loadBackupFileInformation()
{
    if (m_documentFilePath.isEmpty()) {
        this->clear();
        return;
    }

    const QFileInfo fi(m_documentFilePath);
    if (!fi.exists() || fi.suffix() != QStringLiteral("scrite")) {
        this->clear();
        return;
    }

    const QString backupDirPath(fi.absolutePath() + QStringLiteral("/") + fi.completeBaseName()
                                + QStringLiteral(" Backups"));
    QDir backupDir(backupDirPath);
    if (!backupDir.exists()) {
        this->clear();
        return;
    }

    if (m_fsWatcher == nullptr) {
        m_fsWatcher = new QFileSystemWatcher(this);
        connect(m_fsWatcher, SIGNAL(directoryChanged(QString)), &m_reloadTimer, SLOT(start()));
        m_fsWatcher->addPath(backupDirPath);
    }

    m_backupFilesDir = backupDir;
    this->reloadBackupFileInformation();
}

void ScriteDocumentBackups::reloadBackupFileInformation()
{
    const QString futureWatcherName = QStringLiteral("ReloadFutureWatcher");
    if (this->findChild<QFutureWatcherBase *>(futureWatcherName, Qt::FindDirectChildrenOnly)) {
        m_reloadTimer.start();
        return;
    }

    /**
     * Why all this circus?
     *
     * Depending on the kind of OS and hard-disk used, querying directory information may take
     * a few milliseconds or maybe even up to a few seconds. We shouldn't however freeze the
     * UI or even the business logic layer during that time. Updating the model is not a critical
     * activity, so it can take its own sweet time.
     *
     * We push directory query to a separate thread and update the model whenever its job is
     * done.
     */
    QFutureWatcher<QFileInfoList> *futureWatcher = new QFutureWatcher<QFileInfoList>(this);
    futureWatcher->setObjectName(futureWatcherName);
    connect(futureWatcher, &QFutureWatcher<QFileInfoList>::finished, this, [=]() {
        futureWatcher->deleteLater();

        this->beginResetModel();
        m_backupFiles = futureWatcher->result();
        m_metaDataList.resize(m_backupFiles.size());
        this->endResetModel();

        emit countChanged();
    });
    QFuture<QFileInfoList> future = QtConcurrent::run([=]() -> QFileInfoList {
        const QFileInfoList ret = m_backupFilesDir.entryInfoList({ QStringLiteral("*.scrite") },
                                                                 QDir::Files, QDir::Time);
        for (const QFileInfo &fi : ret) {
            const QString absPath = fi.absoluteFilePath();
            QFileDevice::Permissions permissions = QFile::permissions(absPath);
            if (int(permissions) != int(QFileDevice::ReadUser | QFileDevice::ReadOwner))
                QFile::setPermissions(absPath, QFileDevice::ReadUser | QFileDevice::ReadOwner);
        }
        return ret;
    });
    futureWatcher->setFuture(future);
}

void ScriteDocumentBackups::loadMetaData(int row)
{
    if (row < 0 || row >= m_backupFiles.size())
        return;

    const QString futureWatcherName = QStringLiteral("loadMetaDataFuture");

    const QFileInfo fi = m_backupFiles.at(row);
    const QString fileName = fi.absoluteFilePath();

    QFuture<MetaData> future = QtConcurrent::run(
            [](const QString &fileName) -> MetaData {
                MetaData ret;

                DocumentFileSystem dfs;
                if (!dfs.load(fileName)) {
                    ret.loaded = true;
                    return ret;
                }

                const QJsonDocument jsonDoc = QJsonDocument::fromJson(dfs.header());
                const QJsonObject docObj = jsonDoc.object();

                const QJsonObject structure = docObj.value(QStringLiteral("structure")).toObject();
                ret.structureElementCount =
                        structure.value(QStringLiteral("elements")).toArray().size();

                const QJsonObject screenplay =
                        docObj.value(QStringLiteral("screenplay")).toObject();
                ret.screenplayElementCount =
                        screenplay.value(QStringLiteral("elements")).toArray().size();

                ret.loaded = true;

                return ret;
            },
            fileName);

    QFutureWatcher<MetaData> *futureWatcher = new QFutureWatcher<MetaData>(this);
    futureWatcher->setObjectName(futureWatcherName);
    connect(futureWatcher, &QFutureWatcher<MetaData>::finished, [=]() {
        if (row < 0 || row >= m_metaDataList.size())
            return;

        m_metaDataList.replace(row, future.result());

        const QModelIndex index = this->index(row, 0);
        emit dataChanged(index, index);
    });
    futureWatcher->setFuture(future);

    connect(this, &ScriteDocumentBackups::modelAboutToBeReset, futureWatcher,
            &QObject::deleteLater);
}

void ScriteDocumentBackups::clear()
{
    delete m_fsWatcher;
    m_fsWatcher = nullptr;

    m_backupFilesDir = QDir();

    if (m_backupFiles.isEmpty())
        return;

    this->beginResetModel();
    m_backupFiles.clear();
    m_metaDataList.clear();
    m_metaDataList.squeeze();
    this->endResetModel();

    emit countChanged();
}

///////////////////////////////////////////////////////////////////////////////

QJsonObject PageSetup::m_factoryDefaults;

PageSetup::PageSetup(QObject *parent) : QObject(parent)
{
    connect(this, &PageSetup::paperSizeChanged, this, &PageSetup::pageSetupChanged);

    connect(this, &PageSetup::headerLeftChanged, this, &PageSetup::pageSetupChanged);
    connect(this, &PageSetup::headerCenterChanged, this, &PageSetup::pageSetupChanged);
    connect(this, &PageSetup::headerRightChanged, this, &PageSetup::pageSetupChanged);
    connect(this, &PageSetup::headerOpacityChanged, this, &PageSetup::pageSetupChanged);

    connect(this, &PageSetup::footerLeftChanged, this, &PageSetup::pageSetupChanged);
    connect(this, &PageSetup::footerCenterChanged, this, &PageSetup::pageSetupChanged);
    connect(this, &PageSetup::footerRightChanged, this, &PageSetup::pageSetupChanged);
    connect(this, &PageSetup::footerOpacityChanged, this, &PageSetup::pageSetupChanged);

    connect(this, &PageSetup::watermarkAlignmentChanged, this, &PageSetup::pageSetupChanged);
    connect(this, &PageSetup::watermarkColorChanged, this, &PageSetup::pageSetupChanged);
    connect(this, &PageSetup::watermarkEnabledChanged, this, &PageSetup::pageSetupChanged);
    connect(this, &PageSetup::watermarkFontChanged, this, &PageSetup::pageSetupChanged);
    connect(this, &PageSetup::watermarkFontSizeChanged, this, &PageSetup::pageSetupChanged);
    connect(this, &PageSetup::watermarkOpacityChanged, this, &PageSetup::pageSetupChanged);
    connect(this, &PageSetup::watermarkRotationChanged, this, &PageSetup::pageSetupChanged);
    connect(this, &PageSetup::watermarkTextChanged, this, &PageSetup::pageSetupChanged);

    QSignalBlocker signalBlocker(this);
    this->useSavedDefaults();
    m_savedDefaults = QObjectSerializer::toJson(this);

    this->evaluateDefaultsFlags();

    connect(this, &PageSetup::pageSetupChanged, this, &PageSetup::evaluateDefaultsFlags);
}

PageSetup::~PageSetup() { }

void PageSetup::setPaperSize(int val)
{
    if (m_paperSize == val)
        return;

    m_paperSize = val;
    emit paperSizeChanged();
}

void PageSetup::setHeaderLeft(int val)
{
    if (m_headerLeft == val)
        return;

    m_headerLeft = val;
    emit headerLeftChanged();
}

void PageSetup::setHeaderCenter(int val)
{
    if (m_headerCenter == val)
        return;

    m_headerCenter = val;
    emit headerCenterChanged();
}

void PageSetup::setHeaderRight(int val)
{
    if (m_headerRight == val)
        return;

    m_headerRight = val;
    emit headerRightChanged();
}

void PageSetup::setHeaderOpacity(qreal val)
{
    if (qFuzzyCompare(m_headerOpacity, val))
        return;

    m_headerOpacity = val;
    emit headerOpacityChanged();
}

void PageSetup::setFooterLeft(int val)
{
    if (m_footerLeft == val)
        return;

    m_footerLeft = val;
    emit footerLeftChanged();
}

void PageSetup::setFooterCenter(int val)
{
    if (m_footerCenter == val)
        return;

    m_footerCenter = val;
    emit footerCenterChanged();
}

void PageSetup::setFooterRight(int val)
{
    if (m_footerRight == val)
        return;

    m_footerRight = val;
    emit footerRightChanged();
}

void PageSetup::setFooterOpacity(qreal val)
{
    if (qFuzzyCompare(m_footerOpacity, val))
        return;

    m_footerOpacity = val;
    emit footerOpacityChanged();
}

void PageSetup::setWatermarkEnabled(bool val)
{
    if (m_watermarkEnabled == val)
        return;

    m_watermarkEnabled = val;
    emit watermarkEnabledChanged();
}

void PageSetup::setWatermarkText(const QString &val)
{
    if (m_watermarkText == val)
        return;

    m_watermarkText = val;
    emit watermarkTextChanged();
}

void PageSetup::setWatermarkFont(const QString &val)
{
    if (m_watermarkFont == val)
        return;

    m_watermarkFont = val;
    emit watermarkFontChanged();
}

void PageSetup::setWatermarkFontSize(int val)
{
    if (m_watermarkFontSize == val)
        return;

    m_watermarkFontSize = val;
    emit watermarkFontSizeChanged();
}

void PageSetup::setWatermarkColor(const QColor &val)
{
    if (m_watermarkColor == val)
        return;

    m_watermarkColor = val;
    emit watermarkColorChanged();
}

void PageSetup::setWatermarkOpacity(qreal val)
{
    if (qFuzzyCompare(m_watermarkOpacity, val))
        return;

    m_watermarkOpacity = val;
    emit watermarkOpacityChanged();
}

void PageSetup::setWatermarkRotation(int val)
{
    if (m_watermarkRotation == val)
        return;

    m_watermarkRotation = val;
    emit watermarkRotationChanged();
}

void PageSetup::setWatermarkAlignment(int val)
{
    if (m_watermarkAlignment == val)
        return;

    m_watermarkAlignment = val;
    emit watermarkAlignmentChanged();
}

void PageSetup::useFactoryDefaults()
{
    m_headerLeft = HeaderFooter::Title;
    emit headerLeftChanged();

    m_headerCenter = HeaderFooter::Subtitle;
    emit headerCenterChanged();

    m_headerRight = HeaderFooter::PageNumber;
    emit headerRightChanged();

    m_headerOpacity = 0.5;
    emit headerOpacityChanged();

    m_footerLeft = HeaderFooter::Author;
    emit footerLeftChanged();

    m_footerCenter = HeaderFooter::Version;
    emit footerCenterChanged();

    m_footerRight = HeaderFooter::Contact;
    emit footerRightChanged();

    m_footerOpacity = 0.5;
    emit footerOpacityChanged();

    m_paperSize = ScreenplayPageLayout::Letter;
    emit paperSizeChanged();

    m_watermarkEnabled = true;
    emit watermarkEnabledChanged();

    m_watermarkText = QLatin1String("Scrite");
    emit watermarkTextChanged();

    m_watermarkFont = QLatin1String("Courier Prime");
    emit watermarkFontChanged();

    m_watermarkFontSize = 120;
    emit watermarkFontSizeChanged();

    m_watermarkColor = QColor(Qt::lightGray);
    emit watermarkColorChanged();

    m_watermarkOpacity = 0.5;
    emit watermarkOpacityChanged();

    m_watermarkRotation = -45;
    emit watermarkRotationChanged();

    m_watermarkAlignment = Qt::AlignCenter;
    emit watermarkAlignmentChanged();

    if (m_factoryDefaults.isEmpty())
        m_factoryDefaults = QObjectSerializer::toJson(this);
}

void PageSetup::saveAsDefaults()
{
    const QString group = QLatin1String("PageSetup/");
    QSettings *settings = Application::instance()->settings();

    settings->setValue(group + QLatin1String("paperSize"), m_paperSize);

    settings->setValue(group + QLatin1String("headerLeft"), m_headerLeft);
    settings->setValue(group + QLatin1String("headerCenter"), m_headerCenter);
    settings->setValue(group + QLatin1String("headerRight"), m_headerRight);
    settings->setValue(group + QLatin1String("headerOpacity"), m_headerOpacity);

    settings->setValue(group + QLatin1String("footerLeft"), m_footerLeft);
    settings->setValue(group + QLatin1String("footerCenter"), m_footerCenter);
    settings->setValue(group + QLatin1String("footerRight"), m_footerRight);
    settings->setValue(group + QLatin1String("footerOpacity"), m_footerOpacity);

    settings->setValue(group + QLatin1String("watermarkEnabled"), m_watermarkEnabled);
    settings->setValue(group + QLatin1String("watermarkText"), m_watermarkText);
    settings->setValue(group + QLatin1String("watermarkFont"), m_watermarkFont);
    settings->setValue(group + QLatin1String("watermarkFontSize"), m_watermarkFontSize);
    settings->setValue(group + QLatin1String("watermarkColor"), m_watermarkColor);
    settings->setValue(group + QLatin1String("watermarkOpacity"), m_watermarkOpacity);
    settings->setValue(group + QLatin1String("watermarkRotation"), m_watermarkRotation);
    settings->setValue(group + QLatin1String("watermarkAlignment"), m_watermarkAlignment);

    m_savedDefaults = QObjectSerializer::toJson(this);

    this->evaluateDefaultsFlags();
}

void PageSetup::useSavedDefaults()
{
    const QString group = QLatin1String("PageSetup/");
    const QSettings *settings = Application::instance()->settings();

    this->useFactoryDefaults();

    this->setPaperSize(settings->value(group + QLatin1String("paperSize"), m_paperSize).toInt());

    this->setHeaderLeft(settings->value(group + QLatin1String("headerLeft"), m_headerLeft).toInt());
    this->setHeaderCenter(
            settings->value(group + QLatin1String("headerCenter"), m_headerCenter).toInt());
    this->setHeaderRight(
            settings->value(group + QLatin1String("headerRight"), m_headerRight).toInt());
    this->setHeaderOpacity(
            settings->value(group + QLatin1String("headerOpacity"), m_headerOpacity).toReal());

    this->setFooterLeft(settings->value(group + QLatin1String("footerLeft"), m_footerLeft).toInt());
    this->setFooterCenter(
            settings->value(group + QLatin1String("footerCenter"), m_footerCenter).toInt());
    this->setFooterRight(
            settings->value(group + QLatin1String("footerRight"), m_footerRight).toInt());
    this->setFooterOpacity(
            settings->value(group + QLatin1String("footerOpacity"), m_footerOpacity).toReal());

    this->setWatermarkEnabled(
            settings->value(group + QLatin1String("watermarkEnabled"), m_watermarkEnabled)
                    .toBool());
    this->setWatermarkText(
            settings->value(group + QLatin1String("watermarkText"), m_watermarkText).toString());
    this->setWatermarkFont(
            settings->value(group + QLatin1String("watermarkFont"), m_watermarkFont).toString());
    this->setWatermarkFontSize(
            settings->value(group + QLatin1String("watermarkFontSize"), m_watermarkFontSize)
                    .toInt());
    this->setWatermarkColor(
            settings->value(group + QLatin1String("watermarkColor"), m_watermarkColor)
                    .value<QColor>());
    this->setWatermarkOpacity(
            settings->value(group + QLatin1String("watermarkOpacity"), m_watermarkOpacity)
                    .toReal());
    this->setWatermarkRotation(
            settings->value(group + QLatin1String("watermarkRotation"), m_watermarkRotation)
                    .toInt());
    this->setWatermarkAlignment(
            settings->value(group + QLatin1String("watermarkAlignment"), m_watermarkAlignment)
                    .toInt());
}

void PageSetup::setUsingFactoryDefaults(bool val)
{
    if (m_isFactoryDefaults == val)
        return;

    m_isFactoryDefaults = val;
    emit usingFactoryDefaultsChanged();
}

void PageSetup::setUsingSavedDefaults(bool val)
{
    if (m_isSavedDefaults == val)
        return;

    m_isSavedDefaults = val;
    emit usingSavedDefaultsChanged();
}

void PageSetup::evaluateDefaultsFlags()
{
    const QJsonObject json = QObjectSerializer::toJson(this);
    this->setUsingFactoryDefaults(json == m_factoryDefaults);
    this->setUsingSavedDefaults(json == m_savedDefaults);
}

///////////////////////////////////////////////////////////////////////////////

class DeviceIOFactories
{
public:
    DeviceIOFactories();
    ~DeviceIOFactories();

    QObjectFactory ImporterFactory;
    QObjectFactory ExporterFactory;
    QObjectFactory ReportsFactory;
};

DeviceIOFactories::DeviceIOFactories()
    : ImporterFactory(QByteArrayLiteral("Format")),
      ExporterFactory(QByteArrayLiteral("Format")),
      ReportsFactory(QByteArrayLiteral("Title"))
{
    ImporterFactory.addClass<HtmlImporter>();
    ImporterFactory.addClass<FountainImporter>();
    ImporterFactory.addClass<FinalDraftImporter>();

    ExporterFactory.addClass<OdtExporter>();
    ExporterFactory.addClass<PdfExporter>();
    ExporterFactory.addClass<HtmlExporter>();
    ExporterFactory.addClass<TextExporter>();
    ExporterFactory.addClass<FountainExporter>();
    ExporterFactory.addClass<FinalDraftExporter>();

    ReportsFactory.addClass<ScreenplaySubsetReport>();
    ReportsFactory.addClass<LocationReport>();
    ReportsFactory.addClass<CharacterReport>();
    ReportsFactory.addClass<CharacterScreenplayReport>();
    ReportsFactory.addClass<SceneCharacterMatrixReport>();
    ReportsFactory.addClass<StatisticsReport>();
    ReportsFactory.addClass<NotebookReport>();
    ReportsFactory.addClass<TwoColumnReport>();
}

DeviceIOFactories::~DeviceIOFactories() { }

Q_GLOBAL_STATIC(DeviceIOFactories, deviceIOFactories)

ScriteDocument *ScriteDocument::instance()
{
    // CAPTURE_FIRST_CALL_GRAPH;
    static ScriteDocument *theInstance = new ScriteDocument(qApp);
    return theInstance;
}

ScriteDocument::ScriteDocument(QObject *parent)
    : QObject(parent),
      m_autoSaveTimer("ScriteDocument.m_autoSaveTimer"),
      m_clearModifyTimer("ScriteDocument.m_clearModifyTimer"),
      m_structure(this, "structure"),
      m_connectors(this),
      m_screenplay(this, "screenplay"),
      m_formatting(this, "formatting"),
      m_printFormat(this, "printFormat"),
      m_forms(this, "forms"),
      m_pageSetup(this, "pageSetup"),
      m_evaluateStructureElementSequenceTimer(
              "ScriteDocument.m_evaluateStructureElementSequenceTimer")
{
    // CAPTURE_CALL_GRAPH;
    m_fileLocker = new FileLocker(this);

    this->reset();
    this->updateDocumentWindowTitle();

    connect(this, &ScriteDocument::collaboratorsChanged, this, &ScriteDocument::markAsModified);
    connect(this, &ScriteDocument::spellCheckIgnoreListChanged, this,
            &ScriteDocument::markAsModified);
    connect(this, &ScriteDocument::userDataChanged, this, &ScriteDocument::markAsModified);
    connect(this, &ScriteDocument::modifiedChanged, this,
            &ScriteDocument::updateDocumentWindowTitle);
    connect(this, &ScriteDocument::fileNameChanged, this,
            &ScriteDocument::updateDocumentWindowTitle);
    connect(this, &ScriteDocument::readOnlyChanged, this,
            &ScriteDocument::updateDocumentWindowTitle);
    connect(this, &ScriteDocument::fileNameChanged,
            [=]() { m_documentBackupsModel.setDocumentFilePath(m_fileName); });
    connect(qApp->clipboard(), &QClipboard::dataChanged, this,
            &ScriteDocument::canImportFromClipboardChanged);

    const QVariant ase = Application::instance()->settings()->value("AutoSave/autoSaveEnabled");
    this->setAutoSave(ase.isValid() ? ase.toBool() : m_autoSave);

    const QVariant asd = Application::instance()->settings()->value("AutoSave/autoSaveInterval");
    this->setAutoSaveDurationInSeconds(asd.isValid() ? asd.toInt() : m_autoSaveDurationInSeconds);

    m_autoSaveTimer.setRepeat(true);
    this->prepareAutoSave();

    QSettings *settings = Application::instance()->settings();
    const QVariant mbc = settings->value(QStringLiteral("Installation/maxBackupCount"));
    if (!mbc.isNull())
        m_maxBackupCount = mbc.toInt();

    connect(this, &ScriteDocument::collaboratorsChanged, this,
            &ScriteDocument::canModifyCollaboratorsChanged);

    // We cannot use User::instance() right now, because it will
    // refer back to ScriteDocument::instance() and cause a recusrion,
    // leading to crash.
    QTimer::singleShot(0, this, [=]() {
        connect(User::instance(), &User::loggedInChanged, this,
                &ScriteDocument::canModifyCollaboratorsChanged);
        connect(User::instance(), &User::infoChanged, this,
                &ScriteDocument::updateDocumentWindowTitle);

        this->updateDocumentWindowTitle();
    });

    connect(qApp, &QApplication::aboutToQuit, this, [=]() {
        if (m_autoSave && !m_fileName.isEmpty())
            this->save();
    });

    this->initializeFileModificationTracker();
}

ScriteDocument::~ScriteDocument()
{
    emit aboutToDelete(this);
}

void ScriteDocument::setLocked(bool val)
{
    if (m_locked == val)
        return;

    m_locked = val;
    emit lockedChanged();

    this->markAsModified();
}

bool ScriteDocument::isEmpty() const
{
    const int objectCount = qMax(
            0,
            m_structure->elementCount() + m_structure->annotationCount()
                    + m_screenplay->elementCount() + m_structure->notes()->noteCount()
                    + m_structure->characterCount() + m_structure->attachments()->attachmentCount()
                    + m_collaborators.size() - (m_screenplay->isEmpty() ? 2 : 0));

    return objectCount == 0;
}

void ScriteDocument::setFromScriptalay(bool val)
{
    if (m_fromScriptalay == val)
        return;

    m_fromScriptalay = val;
    emit fromScriptalayChanged();
}

void ScriteDocument::setCollaborators(const QStringList &val)
{
    if (m_collaborators == val || !User::instance()->isLoggedIn()
        || !this->canModifyCollaborators())
        return;

    if (val.isEmpty())
        m_collaborators = val;
    else {
        QStringList newCollaborators({ User::instance()->info().email });
        for (const QString &item : val) {
            const QString item2 = item.trimmed().toLower();
            if (item2.isEmpty())
                continue;

            if (!newCollaborators.contains(item2))
                newCollaborators.append(item2);
        }

        m_collaborators = newCollaborators;
    }

    emit collaboratorsChanged();
}

bool ScriteDocument::canModifyCollaborators() const
{
    if (!User::instance()->isLoggedIn())
        return false;

    return m_collaborators.isEmpty()
            || m_collaborators.first().compare(User::instance()->info().email, Qt::CaseInsensitive)
            == 0;
}

void ScriteDocument::addCollaborator(const QString &email)
{
    if (this->hasCollaborators()) {
        QStringList collabs = m_collaborators;
        if (collabs.contains(email))
            return;

        collabs.append(email.toLower());
        this->setCollaborators(collabs);

        User::instance()->logActivity2(
                QStringLiteral("collaboration"),
                QJsonObject({
                        { QStringLiteral("action"), QStringLiteral("add") },
                        { QStringLiteral("size"), QString::number(m_collaborators.size()) },
                }));
    }
}

void ScriteDocument::removeCollaborator(const QString &email)
{
    if (this->hasCollaborators()) {
        QStringList collabs = m_collaborators;
        const int idx = collabs.indexOf(email);
        if (idx == 0)
            return;

        collabs.removeAt(idx);
        this->setCollaborators(collabs);

        User::instance()->logActivity2(
                QStringLiteral("collaboration"),
                QJsonObject({
                        { QStringLiteral("action"), QStringLiteral("remove") },
                        { QStringLiteral("size"), QString::number(m_collaborators.size()) },
                }));
    }
}

void ScriteDocument::enableCollaboration()
{
    if (this->hasCollaborators())
        return;

    if (User::instance()->isLoggedIn()) {
        this->setCollaborators(QStringList({ User::instance()->info().email }));
        User::instance()->logActivity2(
                QStringLiteral("collaboration"),
                QJsonObject({
                        { QStringLiteral("action"), QStringLiteral("enable") },
                        { QStringLiteral("size"), QString::number(m_collaborators.size()) },
                }));
    }
}

void ScriteDocument::disableCollaboration()
{
    User::instance()->logActivity2(
            QStringLiteral("collaboration"),
            QJsonObject({
                    { QStringLiteral("action"), QStringLiteral("disable") },
                    { QStringLiteral("size"), QString::number(m_collaborators.size()) },
            }));
    this->setCollaborators(QStringList());
}

void ScriteDocument::setAutoSaveDurationInSeconds(int val)
{
    val = qBound(30, val, 3600);
    if (m_autoSaveDurationInSeconds == val)
        return;

    m_autoSaveDurationInSeconds = val;
    Application::instance()->settings()->setValue("AutoSave/autoSaveInterval", val);
    this->prepareAutoSave();
    emit autoSaveDurationInSecondsChanged();
}

void ScriteDocument::setAutoSave(bool val)
{
    if (m_autoSave == val)
        return;

    m_autoSave = val;
    Application::instance()->settings()->setValue("AutoSave/autoSaveEnabled", val);
    this->prepareAutoSave();
    emit autoSaveChanged();
}

void ScriteDocument::setBusy(bool val)
{
    if (m_busy == val)
        return;

    m_busy = val;
    emit busyChanged();

    if (val)
        qApp->setOverrideCursor(Qt::WaitCursor);
    else
        qApp->restoreOverrideCursor();
}

void ScriteDocument::setBusyMessage(const QString &val)
{
    if (m_busyMessage == val)
        return;

    m_busyMessage = val;
    emit busyMessageChanged();

    this->setBusy(!m_busyMessage.isEmpty());
}

void ScriteDocument::setSpellCheckIgnoreList(const QStringList &val)
{
    QStringList val2 =
            QSet<QString>(val.begin(), val.end()).values(); // so that we eliminate all duplicates
    std::sort(val2.begin(), val2.end());
    if (m_spellCheckIgnoreList == val)
        return;

    m_spellCheckIgnoreList = val;
    emit spellCheckIgnoreListChanged();
}

void ScriteDocument::addToSpellCheckIgnoreList(const QString &word)
{
    if (word.isEmpty() || m_spellCheckIgnoreList.contains(word))
        return;

    m_spellCheckIgnoreList.append(word);
    std::sort(m_spellCheckIgnoreList.begin(), m_spellCheckIgnoreList.end());
    emit spellCheckIgnoreListChanged();
}

Forms *ScriteDocument::globalForms() const
{
    return Forms::global();
}

Form *ScriteDocument::requestForm(const QString &id)
{
    Form *ret = m_forms->findForm(id);
    if (ret) {
        ret->ref();
        return ret;
    }

    ret = Forms::global()->findForm(id);
    if (ret) {
        const QJsonObject fjs = QObjectSerializer::toJson(ret);
        ret = m_forms->addForm(fjs);
        ret->ref();
        m_forms->append(ret);
    }

    return ret;
}

void ScriteDocument::releaseForm(Form *form)
{
    const int index = form == nullptr ? -1 : m_forms->indexOf(form);
    if (index < 0)
        return;

    if (form->deref() <= 0) {
        m_forms->removeAt(index);
        form->deleteLater();
    }
}

/**
When createNewScene() is called as a because of Ctrl+Shift+N shortcut key then
a new scene must be created after all act-breaks. (fuzzyScreenplayInsert = true)

Otherwise it must add a new scene after the current scene in screenplay.
*/
Scene *ScriteDocument::createNewScene(bool fuzzyScreenplayInsert)
{
    QScopedValueRollback<bool> createNewSceneRollback(m_inCreateNewScene, true);

    StructureElement *structureElement = nullptr;
    int structureElementIndex = m_structure->elementCount() - 1;
    if (m_structure->currentElementIndex() >= 0)
        structureElementIndex = m_structure->currentElementIndex();

    structureElement = m_structure->elementAt(structureElementIndex);

    Scene *activeScene = structureElement ? structureElement->scene() : nullptr;

    const QSettings *settings = Application::instance()->settings();
    const QString defaultSceneColor =
            settings->value(QStringLiteral("Workspace/defaultSceneColor")).toString();

    const QVector<QColor> standardColors = Application::standardColors(QVersionNumber());
    const QColor defaultColor =
            defaultSceneColor.isEmpty() ? standardColors.first() : QColor(defaultSceneColor);

    Scene *scene = new Scene(m_structure);
    scene->setColor(activeScene ? activeScene->color() : defaultColor);
    if (m_structure->canvasUIMode() != Structure::IndexCardUI)
        scene->setSynopsis(QStringLiteral("New Scene"));
    scene->heading()->setEnabled(true);
    scene->heading()->setLocationType(activeScene ? activeScene->heading()->locationType()
                                                  : QStringLiteral("EXT"));
    scene->heading()->setLocation(activeScene ? activeScene->heading()->location()
                                              : QStringLiteral("SOMEWHERE"));
    scene->heading()->setMoment(activeScene ? QStringLiteral("LATER") : QStringLiteral("DAY"));

    SceneElement *firstPara = new SceneElement(scene);
    firstPara->setType(SceneElement::Action);
    scene->addElement(firstPara);

    StructureElement *newStructureElement = new StructureElement(m_structure);
    newStructureElement->setScene(scene);
    m_structure->addElement(newStructureElement);

    const bool asLastScene = m_screenplay->currentElementIndex() < 0
            || (fuzzyScreenplayInsert
                && m_screenplay->currentElementIndex() == m_screenplay->lastSceneIndex());

    ScreenplayElement *newScreenplayElement = new ScreenplayElement(m_screenplay);
    newScreenplayElement->setScene(scene);
    int newScreenplayElementIndex = -1;
    if (asLastScene) {
        newScreenplayElementIndex = m_screenplay->elementCount();
        m_screenplay->addElement(newScreenplayElement);
    } else {
        newScreenplayElementIndex = m_screenplay->currentElementIndex() + 1;
        m_screenplay->insertElementAt(newScreenplayElement,
                                      m_screenplay->currentElementIndex() + 1);
    }

    if (m_screenplay->elementAt(newScreenplayElementIndex) != newScreenplayElement)
        newScreenplayElementIndex = m_screenplay->indexOfElement(newScreenplayElement);

    m_structure->placeElement(newStructureElement, m_screenplay);
    m_structure->setCurrentElementIndex(m_structure->elementCount() - 1);
    m_screenplay->setCurrentElementIndex(newScreenplayElementIndex);

    if (structureElement && !structureElement->stackId().isEmpty()) {
        ScreenplayElement *spe_before = m_screenplay->elementAt(newScreenplayElementIndex - 1);
        ScreenplayElement *spe_after = m_screenplay->elementAt(newScreenplayElementIndex + 1);
        if (spe_before && spe_after) {
            StructureElement *ste_before =
                    m_structure->elementAt(m_structure->indexOfScene(spe_before->scene()));
            StructureElement *ste_after =
                    m_structure->elementAt(m_structure->indexOfScene(spe_after->scene()));
            if (ste_before && ste_after) {
                if (ste_before->stackId() == ste_after->stackId())
                    newStructureElement->setStackId(ste_before->stackId());
            }
        }
    }

    if (newScreenplayElementIndex > 0
        && newScreenplayElementIndex == m_screenplay->elementCount() - 1) {
        ScreenplayElement *prevElement = m_screenplay->elementAt(newScreenplayElementIndex - 1);
        if (prevElement->elementType() == ScreenplayElement::BreakElementType)
            scene->setColor(defaultColor);
    }

    emit newSceneCreated(scene, newScreenplayElementIndex);

    scene->setUndoRedoEnabled(true);
    return scene;
}

void ScriteDocument::setUserData(const QJsonObject &val)
{
    if (m_userData == val)
        return;

    m_userData = val;
    emit userDataChanged();
}

void ScriteDocument::setBookmarkedNotes(const QJsonArray &val)
{
    if (m_bookmarkedNotes == val)
        return;

    m_bookmarkedNotes = val;
    emit bookmarkedNotesChanged();
}

void ScriteDocument::setMaxBackupCount(int val)
{
    if (m_maxBackupCount == val)
        return;

    m_maxBackupCount = val;
    emit maxBackupCountChanged();

    QSettings *settings = Application::instance()->settings();
    settings->setValue(QStringLiteral("Installation/maxBackupCount"), m_maxBackupCount);
}

bool ScriteDocument::canImportFromClipboard() const
{
    const QClipboard *clipboard = qApp->clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    const QString clipboardText = mimeData->text();
    const int nrLines = clipboardText.isEmpty() ? 0 : clipboardText.count("\n");

    if (nrLines >= 2) {
        const Fountain::Parser parser(mimeData->text(), Screenplay::fountainPasteOptions());
        return !parser.body().isEmpty();
    }

    return false;
}

void ScriteDocument::reset()
{
    HourGlass hourGlass;

    if (m_autoSave && !m_fileName.isEmpty())
        this->save();

    emit aboutToReset();

    m_connectors.clear();

    if (m_structure != nullptr) {
        disconnect(m_structure, &Structure::currentElementIndexChanged, this,
                   &ScriteDocument::structureElementIndexChanged);
        disconnect(m_structure, &Structure::structureChanged, this,
                   &ScriteDocument::markAsModified);
        disconnect(m_structure, &Structure::elementCountChanged, this,
                   &ScriteDocument::emptyChanged);
        disconnect(m_structure, &Structure::annotationCountChanged, this,
                   &ScriteDocument::emptyChanged);
        disconnect(m_structure->notes(), &Notes::notesModified, this,
                   &ScriteDocument::emptyChanged);
        disconnect(m_structure, &Structure::preferredGroupCategoryChanged, m_screenplay,
                   &Screenplay::updateBreakTitlesLater);
        disconnect(m_structure, &Structure::groupsModelChanged, m_screenplay,
                   &Screenplay::updateBreakTitlesLater);
    }

    if (m_screenplay != nullptr) {
        disconnect(m_screenplay, &Screenplay::currentElementIndexChanged, this,
                   &ScriteDocument::screenplayElementIndexChanged);
        disconnect(m_screenplay, &Screenplay::screenplayChanged, this,
                   &ScriteDocument::markAsModified);
        disconnect(m_screenplay, &Screenplay::screenplayChanged, this,
                   &ScriteDocument::evaluateStructureElementSequenceLater);
        disconnect(m_screenplay, &Screenplay::elementRemoved, this,
                   &ScriteDocument::screenplayElementRemoved);
        disconnect(m_screenplay, &Screenplay::elementMoved, this,
                   &ScriteDocument::screenplayElementMoved);
        disconnect(m_screenplay, &Screenplay::aboutToMoveElements, this,
                   &ScriteDocument::screenplayAboutToMoveElements);
        disconnect(m_screenplay, &Screenplay::emptyChanged, this, &ScriteDocument::emptyChanged);
        disconnect(m_screenplay, &Screenplay::elementCountChanged, this,
                   &ScriteDocument::emptyChanged);
    }

    if (m_formatting != nullptr)
        disconnect(m_formatting, &ScreenplayFormat::formatChanged, this,
                   &ScriteDocument::markAsModified);

    if (m_printFormat != nullptr)
        disconnect(m_printFormat, &ScreenplayFormat::formatChanged, this,
                   &ScriteDocument::markAsModified);

    UndoStack::clearAllStacks();
    m_docFileSystem.hardReset();

    this->setSessionId(QUuid::createUuid().toString());
    this->setDocumentId(QUuid::createUuid().toString());
    this->setFromScriptalay(false);
    this->setReadOnly(false);
    this->setLocked(false);

    m_collaborators = QStringList();
    emit collaboratorsChanged();

    if (m_formatting == nullptr)
        this->setFormatting(new ScreenplayFormat(this));
    else
        m_formatting->resetToUserDefaults();

    if (m_printFormat == nullptr)
        this->setPrintFormat(new ScreenplayFormat(this));
    else
        m_printFormat->resetToUserDefaults();

    this->setPageSetup(new PageSetup(this));
    this->setForms(new Forms(this));
    this->setScreenplay(new Screenplay(this));
    this->setStructure(new Structure(this));
    this->setBookmarkedNotes(QJsonArray());
    this->setSpellCheckIgnoreList(QStringList());
    this->setFileName(QString());
    this->setUserData(QJsonObject());
    this->evaluateStructureElementSequence();
    this->createNewScene(); // Create a blank scene in new documents.
    this->setModified(false);
    this->clearModifiedLater();
    emit emptyChanged();

    connect(m_structure, &Structure::currentElementIndexChanged, this,
            &ScriteDocument::structureElementIndexChanged);
    connect(m_structure, &Structure::structureChanged, this, &ScriteDocument::markAsModified);
    connect(m_structure, &Structure::elementCountChanged, this, &ScriteDocument::emptyChanged);
    connect(m_structure, &Structure::annotationCountChanged, this, &ScriteDocument::emptyChanged);
    connect(m_structure->notes(), &Notes::notesModified, this, &ScriteDocument::emptyChanged);
    connect(m_structure, &Structure::preferredGroupCategoryChanged, m_screenplay,
            &Screenplay::updateBreakTitlesLater);
    connect(m_structure, &Structure::groupsModelChanged, m_screenplay,
            &Screenplay::updateBreakTitlesLater);

    connect(m_screenplay, &Screenplay::currentElementIndexChanged, this,
            &ScriteDocument::screenplayElementIndexChanged);
    connect(m_screenplay, &Screenplay::screenplayChanged, this, &ScriteDocument::markAsModified);
    connect(m_screenplay, &Screenplay::screenplayChanged, this,
            &ScriteDocument::evaluateStructureElementSequenceLater);
    connect(m_screenplay, &Screenplay::elementRemoved, this,
            &ScriteDocument::screenplayElementRemoved);
    connect(m_screenplay, &Screenplay::aboutToMoveElements, this,
            &ScriteDocument::screenplayAboutToMoveElements);
    connect(m_screenplay, &Screenplay::elementMoved, this, &ScriteDocument::screenplayElementMoved);
    connect(m_screenplay, &Screenplay::emptyChanged, this, &ScriteDocument::emptyChanged);
    connect(m_screenplay, &Screenplay::elementCountChanged, this, &ScriteDocument::emptyChanged);

    connect(m_formatting, &ScreenplayFormat::formatChanged, this, &ScriteDocument::markAsModified);
    connect(m_printFormat, &ScreenplayFormat::formatChanged, this, &ScriteDocument::markAsModified);

    emit justReset();

    ExecLaterTimer::call(
            "ScriteDocument::clearModified", this, [=]() { this->setModified(false); }, 250);
}

void ScriteDocument::reload()
{
    // TODO:
    // The idea is to not save changes, but just reload the file afresh.
    const QString fileName = m_fileName;
    m_fileName.clear();
    this->load(fileName);
}

bool ScriteDocument::canBeBackupFileName(const QString &fileName)
{
    const QFileInfo fi(fileName);
    const QString resolvedAbsFileName = fi.absoluteFilePath();

    static QRegularExpression re(R"(.*[/\\](.+) Backups[/\\]\1 \[(\d+)\]\.([a-zA-Z0-9]+)$)");
    const QRegularExpressionMatch match = re.match(resolvedAbsFileName);

    if (match.hasMatch()) {
        const qint64 timestamp = match.captured(2).toLong();
        const QString extension = match.captured(3).toLower();

        if (extension != "scrite")
            return false;

        if (timestamp >= QDateTime(QDate(2020, 3, 20), QTime(0, 0, 0, 0)).toSecsSinceEpoch())
            return true;
    }

    return false;
}

bool ScriteDocument::openOrImport(const QString &fileName)
{
    if (fileName.isEmpty())
        return false;

    const QFileInfo fi(fileName);
    const QString absFileName = fi.absoluteFilePath();

    if (fi.suffix() == QStringLiteral("scrite"))
        return this->open(absFileName);

    const QList<QByteArray> keys = ::deviceIOFactories->ImporterFactory.keys();
    for (const QByteArray &key : keys) {
        QScopedPointer<AbstractImporter> importer(
                ::deviceIOFactories->ImporterFactory.create<AbstractImporter>(key, this));
        if (importer->canImport(absFileName))
            return this->importFile(importer.data(), fileName);
    }

    return false;
}

bool ScriteDocument::importFromClipboard()
{
    HourGlass hourGlass;

    m_errorReport->clear();

    if (!this->canImportFromClipboard()) {
        m_errorReport->setErrorMessage("No text in clipboard to import from.");
        return false;
    }

    QScopedPointer<FountainImporter> importer(new FountainImporter(this));

    if (importer.isNull()) {
        m_errorReport->setErrorMessage("Couldn't load fountain importer.");
        return false;
    }

    this->setLoading(true);

    m_errorReport->setProxyFor(Aggregation::findErrorReport(importer.get()));
    m_progressReport->setProxyFor(Aggregation::findProgressReport(importer.get()));

    importer->setDocument(this);
    this->setBusyMessage("Importing from clipboard ...");
    const bool success = importer->importFromClipboard();
    this->clearBusyMessage();

    this->setLoading(false);

    return success;
}

bool ScriteDocument::open(const QString &fileName)
{
    if (this->canBeBackupFileName(fileName)) {
        const bool ret = this->openAnonymously(fileName);
        if (ret)
            QTimer::singleShot(500, this, [=]() { emit openedAnonymously(fileName); });
        return ret;
    }

    if (fileName == m_fileName)
        return false;

    HourGlass hourGlass;

    this->setBusyMessage("Loading " + QFileInfo(fileName).completeBaseName() + " ...");
    this->reset();
    const bool ret = this->load(fileName);
    if (ret)
        this->setFileName(fileName);
    this->setModified(false);
    this->clearModifiedLater();
    this->clearBusyMessage();

    return ret;
}

bool ScriteDocument::openAnonymously(const QString &fileName)
{
    HourGlass hourGlass;

    this->setBusyMessage("Loading ...");
    this->reset();
    const bool ret = this->load(fileName, true);
    this->setModified(false);
    this->clearBusyMessage();

    m_fileLocker->setFilePath(QString());
    m_fileName.clear();
    emit fileNameChanged();

    return ret;
}

void ScriteDocument::saveAs(const QString &givenFileName)
{
    HourGlass hourGlass;
    QString fileName = this->polishFileName(givenFileName.trimmed());
    fileName = Application::instance()->sanitiseFileName(fileName);

    m_errorReport->clear();

    if (!this->runSaveSanityChecks(fileName))
        return;

    if (QFile::exists(fileName)) {
        const QString lockFilePath = m_fileLocker->filePath();
        m_fileLocker->setFilePath(fileName);
        if (!m_fileLocker->isClaimed() && !m_fileLocker->canWrite())
            m_fileLocker->claim();

        if (!m_fileLocker->canWrite()) {
            QJsonObject details;
            details.insert(QStringLiteral("revealOnDesktopRequest"), m_fileLocker->lockFilePath());

            m_fileLocker->setFilePath(lockFilePath);
            m_errorReport->setErrorMessage(
                    QStringLiteral("File '%1' is locked by another Scrite instance on this "
                                   "computer or elsewhere. Please "
                                   "close other Scrite instances using this file, or manually "
                                   "delete the lock file.")
                            .arg(fileName),
                    details);
            return;
        }
    }

    if (!m_autoSaveMode)
        this->setBusyMessage("Saving to " + QFileInfo(fileName).completeBaseName() + " ...");

    m_progressReport->start();

    emit aboutToSave();

    const QJsonObject json = QObjectSerializer::toJson(this);
    const QByteArray bytes = QJsonDocument(json).toJson();
    m_docFileSystem.setHeader(bytes);

#ifndef QT_NO_DEBUG_OUTPUT
    const bool saveJson = true;
#else
    const bool saveJson = qgetenv("SCRITE_SAVE_JSON").toUpper() == QByteArrayLiteral("YES");
#endif
    if (saveJson) {
        const QFileInfo fi(fileName);
        const QString fileName2 = fi.absolutePath() + "/" + fi.completeBaseName() + ".json";
        QFile file2(fileName2);
        file2.open(QFile::WriteOnly);
        file2.write(bytes);
    }

    if (m_autoSaveMode) {
        QObject *autoSaveContext = new QObject(this);
        connect(&m_docFileSystem, &DocumentFileSystem::saveFinished, autoSaveContext,
                [=](bool success) {
                    autoSaveContext->deleteLater();
                    if (!success) {
                        m_errorReport->setErrorMessage(QStringLiteral("Auto Save Failed."));

                        m_modified = true;
                        emit modifiedChanged();
                    }

                    emit justSaved();

                    m_progressReport->finish();
                });

        m_docFileSystem.save(fileName, !m_collaborators.isEmpty(),
                             DocumentFileSystem::NonBlockingSaveMode);
        m_modified = false;
        emit modifiedChanged();
    } else {
        const bool success = m_docFileSystem.save(fileName, !m_collaborators.isEmpty());

        if (!success) {
            m_errorReport->setErrorMessage(QStringLiteral("Couldn't save document \"") + fileName
                                           + QStringLiteral("\""));
            emit justSaved();
            m_progressReport->finish();
            this->clearBusyMessage();
            return;
        }

        this->setFileName(fileName);
        this->setCreatedOnThisComputer(true);

        emit justSaved();

        m_modified = false;
        emit modifiedChanged();

        m_progressReport->finish();

        this->setReadOnly(false);

        this->clearBusyMessage();
    }
}

void ScriteDocument::save()
{
    HourGlass hourGlass;

    if (m_readOnly)
        return;

    if (!this->runSaveSanityChecks(m_fileName))
        return;

    QFileInfo fi(m_fileName);
    if (fi.exists()) {
        const QString backupDirPath(fi.absolutePath() + "/" + fi.completeBaseName() + " Backups");
        QDir().mkpath(backupDirPath);

        const qint64 now = QDateTime::currentSecsSinceEpoch();

        auto timeGapInSeconds = [now](const QFileInfo &fi) {
            const QString baseName = fi.completeBaseName();
            const QString thenStr = baseName.section('[', 1).section(']', 0, 0);
            const qint64 then = thenStr.toLongLong();
            return now - then;
        };

        const QDir backupDir(backupDirPath);
        QFileInfoList backupEntries = backupDir.entryInfoList(
                QStringList() << QStringLiteral("*.scrite"), QDir::Files, QDir::Name);
        const bool firstBackup = backupEntries.isEmpty();
        if (!backupEntries.isEmpty()) {
            const int maxBackups = m_maxBackupCount;
            if (maxBackups > 0) {
                while (backupEntries.size() > maxBackups - 1) {
                    const QFileInfo oldestEntry = backupEntries.takeFirst();
                    QFile::remove(oldestEntry.absoluteFilePath());
                }
            }

            const QFileInfo latestEntry = backupEntries.takeLast();
            if (latestEntry.suffix() == QStringLiteral("scrite")) {
                if (timeGapInSeconds(latestEntry) < 60)
                    QFile::remove(latestEntry.absoluteFilePath());
            }
        }

        const QString backupFileName = backupDirPath + "/" + fi.completeBaseName() + " ["
                + QString::number(now) + "].scrite";
        const bool backupSuccessful = QFile::copy(m_fileName, backupFileName);
        if (backupSuccessful)
            QFile::setPermissions(backupFileName, QFileDevice::ReadOwner | QFileDevice::ReadUser);

        if (firstBackup && backupSuccessful)
            m_documentBackupsModel.loadBackupFileInformation();
    }

    this->saveAs(m_fileName);
}

QStringList ScriteDocument::supportedImportFormats() const
{
    static const QList<QByteArray> keys = deviceIOFactories->ImporterFactory.keys();
    static QStringList formats;
    if (formats.isEmpty())
        for (const QByteArray &key : keys)
            formats << key;
    return formats;
}

QString ScriteDocument::importFormatFileSuffix(const QString &format) const
{
    const QMetaObject *mo = deviceIOFactories->ImporterFactory.find(format.toLatin1());
    if (mo == nullptr)
        return QString();

    const int ciIndex = mo->indexOfClassInfo("NameFilters");
    if (ciIndex < 0)
        return QString();

    const QMetaClassInfo classInfo = mo->classInfo(ciIndex);
    return QString::fromLatin1(classInfo.value());
}

QJsonArray ScriteDocument::supportedExportFormats() const
{
    static QJsonArray exporters;

    if (exporters.isEmpty()) {
        QList<QByteArray> keys = deviceIOFactories->ExporterFactory.keys();
        std::sort(keys.begin(), keys.end());

        for (const QByteArray &key : qAsConst(keys)) {
            const QString skey = QString::fromLatin1(key);
            const QStringList fields = skey.split("/");

            QJsonObject item;
            item.insert("key", skey);
            item.insert("category", fields.first());
            item.insert("name", fields.last());

            const QMetaObject *mo = deviceIOFactories->ExporterFactory.find(key);
            const int d_cii = mo->indexOfClassInfo("Description");
            item.insert("description",
                        d_cii >= 0 ? QString::fromLatin1(mo->classInfo(d_cii).value())
                                   : QString::fromLatin1(key));
            const int i_cii = mo->indexOfClassInfo("Icon");
            item.insert("icon",
                        i_cii >= 0 ? QString::fromLatin1(mo->classInfo(i_cii).value())
                                   : QStringLiteral(":/icons/exporters/exporters_menu_item.png"));
            const int n_cii = mo->indexOfClassInfo("NameFilters");
            item.insert("nameFilters",
                        n_cii >= 0 ? QString::fromLatin1(mo->classInfo(n_cii).value())
                                   : QStringLiteral("Scrite Document (*.scrite)"));

            exporters.append(item);
        }
    }

    return exporters;
}

QJsonArray ScriteDocument::supportedReports() const
{
    static QJsonArray reports;

    if (reports.isEmpty()) {
        const QList<QByteArray> keys = deviceIOFactories->ReportsFactory.keys();

        for (const QByteArray &key : keys) {
            QJsonObject item;
            item.insert("name", QString::fromLatin1(key));

            const QMetaObject *mo = deviceIOFactories->ReportsFactory.find(key);
            const int d_cii = mo->indexOfClassInfo("Description");
            item.insert("description",
                        d_cii >= 0 ? QString::fromLatin1(mo->classInfo(d_cii).value())
                                   : QString::fromLatin1(key));
            const int i_cii = mo->indexOfClassInfo("Icon");
            item.insert("icon",
                        i_cii >= 0 ? QString::fromLatin1(mo->classInfo(i_cii).value())
                                   : QStringLiteral(":/icons/reports/reports_menu_item.png"));

            reports.append(item);
        }
    }

    return reports;
}

QJsonArray ScriteDocument::characterListReports() const
{
    static QJsonArray ret;

    if (ret.isEmpty()) {
        const QList<QByteArray> keys = deviceIOFactories->ReportsFactory.keys();

        for (const QByteArray &key : keys) {
            const QMetaObject *mo = deviceIOFactories->ReportsFactory.find(key);
            const QByteArray propName = QByteArrayLiteral("characterNames");
            if (mo->indexOfProperty(propName) >= 0) {
                QJsonObject item;
                item.insert("name", QString::fromLatin1(key));

                const QMetaObject *mo = deviceIOFactories->ReportsFactory.find(key);
                const int d_cii = mo->indexOfClassInfo("Description");
                item.insert("description",
                            d_cii >= 0 ? QString::fromLatin1(mo->classInfo(d_cii).value())
                                       : QString::fromLatin1(key));
                const int i_cii = mo->indexOfClassInfo("Icon");
                item.insert("icon",
                            i_cii >= 0 ? QString::fromLatin1(mo->classInfo(i_cii).value())
                                       : QStringLiteral(":/icons/reports/reports_menu_item.png"));

                const int p_cii = mo->indexOfClassInfo(propName + "_FieldGroup");
                item.insert("group", QString::fromLatin1(mo->classInfo(p_cii).value()));

                ret.append(item);
            }
        }
    }

    return ret;
}

QJsonArray ScriteDocument::sceneListReports() const
{
    static QJsonArray ret;

    if (ret.isEmpty()) {
        const QList<QByteArray> keys = deviceIOFactories->ReportsFactory.keys();

        for (const QByteArray &key : keys) {
            const QMetaObject *mo = deviceIOFactories->ReportsFactory.find(key);
            const QByteArray propName = QByteArrayLiteral("sceneNumbers");
            if (mo->indexOfProperty(propName) >= 0) {
                QJsonObject item;
                item.insert("name", QString::fromLatin1(key));

                const QMetaObject *mo = deviceIOFactories->ReportsFactory.find(key);
                const int d_cii = mo->indexOfClassInfo("Description");
                item.insert("description",
                            d_cii >= 0 ? QString::fromLatin1(mo->classInfo(d_cii).value())
                                       : QString::fromLatin1(key));
                const int i_cii = mo->indexOfClassInfo("Icon");
                item.insert("icon",
                            i_cii >= 0 ? QString::fromLatin1(mo->classInfo(i_cii).value())
                                       : QStringLiteral(":/icons/reports/reports_menu_item.png"));

                const int p_cii = mo->indexOfClassInfo(propName + "_FieldGroup");
                item.insert("group", QString::fromLatin1(mo->classInfo(p_cii).value()));

                ret.append(item);
            }
        }
    }

    return ret;
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
    QScopedPointer<AbstractImporter> importer(
            deviceIOFactories->ImporterFactory.create<AbstractImporter>(formatKey, this));

    if (importer.isNull()) {
        m_errorReport->setErrorMessage("Cannot import from this format.");
        return false;
    }

    return this->importFile(importer.data(), fileName);
}

bool ScriteDocument::importFile(AbstractImporter *importer, const QString &fileName)
{
    this->setLoading(true);

    Aggregation aggregation;
    m_errorReport->setProxyFor(aggregation.findErrorReport(importer));
    m_progressReport->setProxyFor(aggregation.findProgressReport(importer));

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
    QScopedPointer<AbstractExporter> exporter(
            deviceIOFactories->ExporterFactory.create<AbstractExporter>(formatKey, this));

    if (exporter.isNull()) {
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

bool ScriteDocument::exportToImage(int fromSceneIdx, int fromParaIdx, int toSceneIdx, int toParaIdx,
                                   const QString &imageFileName)
{
    const int nrScenes = m_screenplay->elementCount();
    if (fromSceneIdx < 0 || fromSceneIdx >= nrScenes)
        return false;

    if (toSceneIdx < 0 || toSceneIdx >= nrScenes)
        return false;

    QTextDocument document;
    // m_printFormat->pageLayout()->configure(&document);
    document.setTextWidth(m_printFormat->pageLayout()->contentWidth());

    QTextCursor cursor(&document);

    auto prepareCursor = [=](QTextCursor &cursor, SceneElement::Type paraType,
                             Qt::Alignment overrideAlignment) {
        const qreal pageWidth = m_printFormat->pageLayout()->contentWidth();
        const SceneElementFormat *format = m_printFormat->elementFormat(paraType);
        QTextBlockFormat blockFormat = format->createBlockFormat(overrideAlignment, &pageWidth);
        QTextCharFormat charFormat = format->createCharFormat(&pageWidth);
        cursor.setCharFormat(charFormat);
        cursor.setBlockFormat(blockFormat);
    };

    for (int i = fromSceneIdx; i <= toSceneIdx; i++) {
        const ScreenplayElement *element = m_screenplay->elementAt(i);
        if (element->scene() == nullptr)
            continue;

        const Scene *scene = element->scene();
        int startParaIdx = -1, endParaIdx = -1;

        if (cursor.position() > 0) {
            cursor.insertBlock();
            startParaIdx = qMax(fromParaIdx, 0);
        } else
            startParaIdx = 0;

        endParaIdx = (i == toSceneIdx) ? qMin(toParaIdx, scene->elementCount() - 1) : toParaIdx;
        if (endParaIdx < 0)
            endParaIdx = scene->elementCount() - 1;

        if (startParaIdx == 0 && scene->heading()->isEnabled()) {
            prepareCursor(cursor, SceneElement::Heading, Qt::Alignment());
            cursor.insertText(QStringLiteral("[") + element->resolvedSceneNumber()
                              + QStringLiteral("] "));
            cursor.insertText(scene->heading()->text());
            cursor.insertBlock();
        }

        for (int p = startParaIdx; p <= endParaIdx; p++) {
            const SceneElement *para = scene->elementAt(p);
            prepareCursor(cursor, para->type(), para->alignment());
            cursor.insertText(para->text());
            if (p < endParaIdx)
                cursor.insertBlock();
        }
    }

    const QSizeF docSize = document.documentLayout()->documentSize() * 2.0;

    QImage image(docSize.toSize(), QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    QPainter paint(&image);
    paint.scale(2.0, 2.0);
    document.drawContents(&paint, QRectF(QPointF(0, 0), docSize));
    paint.end();

    const QString format = QFileInfo(imageFileName).suffix().toUpper();

    return image.save(imageFileName, qPrintable(format));
}

inline QString createTimestampString(const QDateTime &dt = QDateTime::currentDateTime())
{
    static const QString format = QStringLiteral("MMM dd yyyy h.m AP");
    return dt.toString(format);
}

AbstractExporter *ScriteDocument::createExporter(const QString &format)
{
    const QByteArray formatKey = format.toLatin1();
    AbstractExporter *exporter =
            deviceIOFactories->ExporterFactory.create<AbstractExporter>(formatKey, this);
    if (exporter == nullptr)
        return nullptr;

    this->setupExporter(exporter);

    return exporter;
}

AbstractReportGenerator *ScriteDocument::createReportGenerator(const QString &report)
{
    const QByteArray reportKey = report.toLatin1();
    AbstractReportGenerator *reportGenerator =
            deviceIOFactories->ReportsFactory.create<AbstractReportGenerator>(reportKey, this);
    if (reportGenerator == nullptr)
        return nullptr;

    this->setupReportGenerator(reportGenerator);

    return reportGenerator;
}

void ScriteDocument::setupExporter(AbstractExporter *exporter)
{
    if (exporter == nullptr)
        return;

    exporter->setDocument(this);

    if (exporter->fileName().isEmpty()) {
        QString suggestedName = QFileInfo(m_fileName).completeBaseName();
        if (suggestedName.isEmpty())
            suggestedName = m_screenplay->title();
        if (suggestedName.isEmpty())
            suggestedName = QStringLiteral("Scrite Screenplay");

        suggestedName += QStringLiteral(" - ") + exporter->formatName();
        suggestedName += QStringLiteral(" - ") + createTimestampString();

        // Insert a dummy extension, so that exporters can correct it.
        suggestedName += QStringLiteral(".ext");

#if 0
        QFileInfo fi(m_fileName);
        if (fi.exists())
            exporter->setFileName(fi.absoluteDir().absoluteFilePath(suggestedName));
        else {
            const QUrl folderUrl(
                    Application::instance()
                            ->settings()
                            ->value(QStringLiteral("Workspace/lastOpenExportFolderUrl"))
                            .toString());
            const QString path = folderUrl.isEmpty()
                    ? QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                    : folderUrl.toLocalFile();
        }
#else
        const QString path = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
#endif
        exporter->setFileName(path + QStringLiteral("/") + suggestedName);
    }

#if 0
    ProgressReport *progressReport = exporter->findChild<ProgressReport *>();
    if (progressReport) {
        connect(progressReport, &ProgressReport::statusChanged, this,
                [progressReport, this, exporter]() {
                    if (progressReport->status() == ProgressReport::Started)
                        this->setBusyMessage("Exporting into \"" + exporter->fileName() + "\" ...");
                    else if (progressReport->status() == ProgressReport::Finished)
                        this->clearBusyMessage();
                });
    }
#endif
}

void ScriteDocument::setupReportGenerator(AbstractReportGenerator *reportGenerator)
{
    if (reportGenerator == nullptr)
        return;

    reportGenerator->setDocument(this);

    if (reportGenerator->fileName().isEmpty()) {
        QString suggestedName = QFileInfo(m_fileName).completeBaseName();
        if (suggestedName.isEmpty())
            suggestedName = m_screenplay->title();
        if (suggestedName.isEmpty())
            suggestedName = QStringLiteral("Scrite");

        const QString reportName = reportGenerator->name();
        const QString suffix = reportGenerator->format() == AbstractReportGenerator::AdobePDF
                ? QStringLiteral(".pdf")
                : QStringLiteral(".odt");
        suggestedName = suggestedName + QStringLiteral(" - ") + reportName + QStringLiteral(" - ")
                + createTimestampString() + suffix;

#if 0
        QFileInfo fi(m_fileName);
        if (fi.exists())
            reportGenerator->setFileName(fi.absoluteDir().absoluteFilePath(suggestedName));
        else {
            const QUrl folderUrl(
                    Application::instance()
                            ->settings()
                            ->value(QStringLiteral("Workspace/lastOpenReportsFolderUrl"))
                            .toString());
            const QString path = folderUrl.isEmpty()
                    ? QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                    : folderUrl.toLocalFile();
        }
#else
        const QString path = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
#endif
        reportGenerator->setFileName(path + QStringLiteral("/") + suggestedName);
    }

#if 0
    ProgressReport *progressReport = reportGenerator->findChild<ProgressReport *>();
    if (progressReport) {
        connect(progressReport, &ProgressReport::statusChanged, this,
                [progressReport, this, reportGenerator]() {
                    if (progressReport->status() == ProgressReport::Started)
                        this->setBusyMessage("Generating \"" + reportGenerator->fileName()
                                             + "\" ...");
                    else if (progressReport->status() == ProgressReport::Finished)
                        this->clearBusyMessage();
                });
    }
#endif
}

QAbstractListModel *ScriteDocument::structureElementConnectors() const
{
    ScriteDocument *that = const_cast<ScriteDocument *>(this);
    return &(that->m_connectors);
}

void ScriteDocument::clearModified()
{
    if (m_screenplay->elementCount() == 0 && m_structure->elementCount() == 0)
        this->setModified(false);
}

void ScriteDocument::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_evaluateStructureElementSequenceTimer.timerId()) {
        m_evaluateStructureElementSequenceTimer.stop();
        this->evaluateStructureElementSequence();
        return;
    }

    if (event->timerId() == m_autoSaveTimer.timerId()) {
        if (m_modified && !m_fileName.isEmpty() && QFileInfo(m_fileName).isWritable()) {
            QScopedValueRollback<bool> autoSave(m_autoSaveMode, true);
            this->save();
        }
        return;
    }

    if (event->timerId() == m_clearModifyTimer.timerId()) {
        m_clearModifyTimer.stop();
        this->setModified(false);
        return;
    }

    QObject::timerEvent(event);
}

bool ScriteDocument::runSaveSanityChecks(const QString &givenFileName)
{
    const QString fileName = givenFileName.trimmed();

    // Multiple things could go wrong while saving a file.
    // 1. File name is empty.
    if (fileName.isEmpty()) {
        m_errorReport->setErrorMessage(QStringLiteral("File name cannot be empty"));
        return false;
    }

    // 2. Filename must not contain special characters
    // It is true that file names will have already been sanitized using
    // Application::sanitiseFileName() But we double check it here anyway.
    QSet<QChar> purgedChars;
    const QString sanitisedFileName = Application::sanitiseFileName(fileName, &purgedChars);
    if (sanitisedFileName != fileName || !purgedChars.isEmpty()) {
        if (purgedChars.isEmpty())
            m_errorReport->setErrorMessage(
                    QStringLiteral("File name contains invalid characters."));
        else
            m_errorReport->setErrorMessage(
                    QStringLiteral("File name cannot contain special character '%1'")
                            .arg(*purgedChars.begin()));
        return false;
    }

    // 3. File already exists, but has become readonly now.
    QFileInfo fi(fileName);
    if (fi.exists() && !fi.isWritable()) {
        m_errorReport->setErrorMessage(
                QStringLiteral("Cannot open '%1' for writing.").arg(fileName));
        return false;
    }

    // 4. Folder in which the file exists seems have become readonly
    QDir dir = fi.absoluteDir();
    {
        // Try to write something in this folder.
        const QString tmpFile = dir.absoluteFilePath(
                QStringLiteral("scrite_tmp_") + QString::number(QDateTime::currentMSecsSinceEpoch())
                + QStringLiteral(".dat"));
        QFile file(tmpFile);
        if (!file.open(QFile::WriteOnly)) {
            m_errorReport->setErrorMessage(
                    QStringLiteral("Cannot write into folder '%1'").arg(dir.absolutePath()));
            return false;
        }

        file.close();
        QFile::remove(tmpFile);
    }

    // 5. If the scrite document is locked for whatever reason
    if (m_locked && !m_createdOnThisComputer) {
        m_errorReport->setErrorMessage(
                QStringLiteral("Cannot write into '%1' because it is locked").arg(fileName));
        return false;
    }

    return true;
}

void ScriteDocument::setReadOnly(bool val)
{
    if (m_readOnly == val)
        return;

    m_readOnly = val;
    emit readOnlyChanged();
}

void ScriteDocument::setLoading(bool val)
{
    if (m_loading == val)
        return;

    m_loading = val;
    emit loadingChanged();
}

void ScriteDocument::prepareAutoSave()
{
    if (m_autoSave)
        m_autoSaveTimer.start(m_autoSaveDurationInSeconds * 1000, this);
    else
        m_autoSaveTimer.stop();
}

void ScriteDocument::updateDocumentWindowTitle()
{
    QString title;

    if (m_readOnly)
        title += "READ ONLY: ";

    if (m_modified)
        title += QStringLiteral("* ");

    if (m_fileName.isEmpty())
        title += QStringLiteral("[noname]");
    else
        title += QFileInfo(m_fileName).completeBaseName();

    title += QStringLiteral(" - ") + qApp->property("baseWindowTitle").toString();

    if (User::instance()->isLoggedIn() && User::instance()->info().hasActiveSubscription) {
        const UserSubscriptionInfo activeSub = User::instance()->info().subscriptions.first();
        title += " [" + activeSub.description() + "]";
    }

    this->setDocumentWindowTitle(title);
}

void ScriteDocument::setDocumentWindowTitle(const QString &val)
{
    if (m_documentWindowTitle == val)
        return;

    m_documentWindowTitle = val;
    emit documentWindowTitleChanged(m_documentWindowTitle);
}

void ScriteDocument::setStructure(Structure *val)
{
    if (m_structure == val)
        return;

    if (m_structure != nullptr)
        GarbageCollector::instance()->add(m_structure);

    m_structure = val;
    m_structure->setParent(this);
    m_structure->setObjectName("Document Structure");

    emit structureChanged();
}

void ScriteDocument::setScreenplay(Screenplay *val)
{
    if (m_screenplay == val)
        return;

    if (m_screenplay != nullptr)
        GarbageCollector::instance()->add(m_screenplay);

    m_screenplay = val;
    m_screenplay->setParent(this);
    m_screenplay->setObjectName("Document Screenplay");

    emit screenplayChanged();
}

void ScriteDocument::setFormatting(ScreenplayFormat *val)
{
    if (m_formatting == val)
        return;

    if (m_formatting != nullptr)
        GarbageCollector::instance()->add(m_formatting);

    m_formatting = val;
    m_formatting->setSreeenFromWindow(Scrite::window());

    if (m_formatting != nullptr)
        m_formatting->setParent(this);

    emit formattingChanged();
}

void ScriteDocument::setPrintFormat(ScreenplayFormat *val)
{
    if (m_printFormat == val)
        return;

    if (m_printFormat != nullptr)
        GarbageCollector::instance()->add(m_printFormat);

    m_printFormat = val;

    if (m_formatting != nullptr)
        m_printFormat->setParent(this);

    emit printFormatChanged();
}

void ScriteDocument::setForms(Forms *val)
{
    if (m_forms == val)
        return;

    if (m_forms != nullptr) {
        GarbageCollector::instance()->add(m_forms);
        disconnect(m_forms, &Forms::formCountChanged, this, &ScriteDocument::markAsModified);
    }

    m_forms = val;

    if (m_forms != nullptr) {
        m_forms->setParent(this);
        connect(m_forms, &Forms::formCountChanged, this, &ScriteDocument::markAsModified);
    }

    emit formsChanged();
}

void ScriteDocument::setPageSetup(PageSetup *val)
{
    if (m_pageSetup == val)
        return;

    if (m_pageSetup != nullptr) {
        disconnect(m_pageSetup, &PageSetup::pageSetupChanged, this,
                   &ScriteDocument::markAsModified);
        GarbageCollector::instance()->add(m_pageSetup);
    }

    m_pageSetup = val;

    if (m_pageSetup != nullptr) {
        m_pageSetup->setParent(this);
        connect(m_pageSetup, &PageSetup::pageSetupChanged, this, &ScriteDocument::markAsModified);
    }

    emit pageSetupChanged();
}

void ScriteDocument::evaluateStructureElementSequence()
{
    m_connectors.reload();
}

void ScriteDocument::evaluateStructureElementSequenceLater()
{
    m_evaluateStructureElementSequenceTimer.start(0, this);
}

void ScriteDocument::markAsModified()
{
    this->setModified(m_loading ? true : !this->isEmpty());
    emit documentChanged();
}

void ScriteDocument::setModified(bool val)
{
    if (m_readOnly)
        val = false;

    if (m_modified == val)
        return;

    if (m_structure == nullptr || m_screenplay == nullptr)
        return;

    m_modified = val;

    emit modifiedChanged();
}

void ScriteDocument::setFileName(const QString &val)
{
    if (m_fileName == val)
        return;

    m_fileName = this->polishFileName(val);
    emit fileNameChanged();

    m_fileLocker->setFilePath(m_fileName);
}

bool ScriteDocument::load(const QString &fileName, bool anonymousLoad)
{
    m_errorReport->clear();

    QJsonObject details;
    details.insert(QStringLiteral("revealOnDesktopRequest"), fileName);

    if (!QFileInfo(fileName).isReadable()) {
        m_errorReport->setErrorMessage(QStringLiteral("Cannot open %1 for reading.").arg(fileName),
                                       details);
        return false;
    }

    m_fileLocker->setFilePath(fileName);
    if (m_fileLocker->isClaimed() && !m_fileLocker->canRead()) {
        details.insert(QStringLiteral("revealOnDesktopRequest"), m_fileLocker->lockFilePath());
        m_fileLocker->setFilePath(QString());
        m_errorReport->setErrorMessage(
                QStringLiteral(
                        "File '%1' is locked by another Scrite instance on this computer or "
                        "elsewhere. Please close "
                        "other Scrite instances using this file, or manually delete the lock file.")
                        .arg(fileName),
                details);
        return false;
    }

    struct LoadCleanup
    {
        LoadCleanup(ScriteDocument *doc) : m_document(doc) { m_document->m_errorReport->clear(); }

        ~LoadCleanup()
        {
            if (m_loadBegun) {
                m_document->m_progressReport->finish();
                m_document->setLoading(false);
            } else
                m_document->m_docFileSystem.hardReset();
        }

        void begin()
        {
            m_loadBegun = true;
            m_document->m_progressReport->start();
            m_document->setLoading(true);
        }

    private:
        bool m_loadBegun = false;
        ScriteDocument *m_document;
    } loadCleanup(this);

    int format = DocumentFileSystem::ScriteFormat;
    bool loaded = this->classicLoad(fileName);
    if (!loaded)
        loaded = this->modernLoad(fileName, &format);

    if (!loaded) {
        m_errorReport->setErrorMessage(QStringLiteral("%1 is not a Scrite document.").arg(fileName),
                                       details);
        return false;
    }

    const QJsonDocument jsonDoc = format == DocumentFileSystem::ZipFormat
            ? QJsonDocument::fromJson(m_docFileSystem.header())
            : QJsonDocument::fromBinaryData(m_docFileSystem.header());

#ifndef QT_NO_DEBUG_OUTPUT
    {
        const QFileInfo fi(fileName);
        const QString fileName2 = fi.absolutePath() + "/" + fi.completeBaseName() + ".json";
        QFile file2(fileName2);
        file2.open(QFile::WriteOnly);
        file2.write(jsonDoc.toJson());
    }
#endif

    const QJsonObject json = jsonDoc.object();
    if (json.isEmpty()) {
        m_errorReport->setErrorMessage(QStringLiteral("%1 is not a Scrite document.").arg(fileName),
                                       details);
        return false;
    }

    const QJsonObject metaInfo = json.value("meta").toObject();
    if (metaInfo.value("appName").toString().toLower() != qApp->applicationName().toLower()) {
        m_errorReport->setErrorMessage(
                QStringLiteral("Scrite document '%1' was created using an unrecognised app.")
                        .arg(fileName),
                details);
        ;
        return false;
    }

    const QVersionNumber docVersion =
            QVersionNumber::fromString(metaInfo.value(QStringLiteral("appVersion")).toString());
    const QVersionNumber appVersion = Application::instance()->versionNumber();
    if (appVersion < docVersion) {
        m_errorReport->setErrorMessage(
                QStringLiteral("Scrite document '%1' was created using an updated version.")
                        .arg(fileName),
                details);
        return false;
    }

    {
        const QJsonArray jsCollaborators = json.value(QStringLiteral("collaborators")).toArray();
        for (const QJsonValue &item : jsCollaborators) {
            const QString collaborator = item.toString();
            if (collaborator.isEmpty())
                continue;

            m_collaborators << collaborator;
        }

        if (!m_collaborators.isEmpty()) {
            if (!User::instance()->isLoggedIn()) {
                m_collaborators.clear();
                m_errorReport->setErrorMessage(QStringLiteral(
                        "This document is protected. Please sign-up/login to open it."));
                return false;
            }

            const QString infoEmail = User::instance()->info().email;
            const QString lsEmail = LocalStorage::load("email").toString();
            if (infoEmail.isEmpty() || lsEmail.isEmpty() || infoEmail != lsEmail
                || !m_collaborators.contains(lsEmail, Qt::CaseInsensitive)) {
                m_collaborators.clear();
                m_errorReport->setErrorMessage(QStringLiteral(
                        "This document is protected. You are not authorized to view it."));
                return false;
            }
        }
    }

    m_fileName = fileName;
    emit fileNameChanged();

    const bool ro = QFileInfo(fileName).permission(QFile::WriteUser) == false;
    this->setReadOnly(ro);
    this->setModified(false);

    loadCleanup.begin();

    UndoStack::ignoreUndoCommands = true;
    const bool ret = QObjectSerializer::fromJson(json, this);
    if (m_screenplay->currentElementIndex() == 0)
        m_screenplay->setCurrentElementIndex(-1);
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

    if (ro && !anonymousLoad) {
        Notification *notification = new Notification(this);
        connect(notification, &Notification::dismissed, &Notification::deleteLater);

        notification->setTitle(QStringLiteral("File only has read permission."));
        notification->setText(QStringLiteral("This document is being opened in read only mode.\nTo "
                                             "edit this document, please apply "
                                             "write-permissions for the file in your computer."));
        notification->setAutoClose(false);
        notification->setActive(true);
    }

    emit collaboratorsChanged();
    emit justLoaded();

    return ret;
}

bool ScriteDocument::classicLoad(const QString &fileName)
{
    if (fileName.isEmpty())
        return false;

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        return false;

    static const QByteArray classicMarker("qbjs");
    const QByteArray marker = file.read(classicMarker.length());
    if (marker != classicMarker)
        return false;

    file.seek(0);

    const QByteArray bytes = file.readAll();
    m_docFileSystem.setHeader(bytes);
    return true;
}

bool ScriteDocument::modernLoad(const QString &fileName, int *format)
{
    DocumentFileSystem::Format dfsFormat;
    const bool ret = m_docFileSystem.load(fileName, &dfsFormat);
    if (format)
        *format = dfsFormat;
    return ret;
}

void ScriteDocument::structureElementIndexChanged()
{
    if (m_screenplay == nullptr || m_structure == nullptr
        || m_syncingStructureScreenplayCurrentIndex || m_inCreateNewScene)
        return;

    QScopedValueRollback<bool> rollback(m_syncingStructureScreenplayCurrentIndex, true);

    StructureElement *element = m_structure->elementAt(m_structure->currentElementIndex());
    if (element == nullptr) {
        m_screenplay->setActiveScene(nullptr);
        m_screenplay->setCurrentElementIndex(-1);
    } else
        m_screenplay->setActiveScene(element->scene());
}

void ScriteDocument::screenplayElementIndexChanged()
{
    if (m_screenplay == nullptr || m_structure == nullptr
        || m_syncingStructureScreenplayCurrentIndex || m_inCreateNewScene)
        return;

    QScopedValueRollback<bool> rollback(m_syncingStructureScreenplayCurrentIndex, true);

    ScreenplayElement *element = m_screenplay->elementAt(m_screenplay->currentElementIndex());
    if (element != nullptr) {
        int index = m_structure->indexOfScene(element->scene());
        m_structure->setCurrentElementIndex(index);
    }
}

void ScriteDocument::setCreatedOnThisComputer(bool val)
{
    if (m_createdOnThisComputer == val)
        return;

    m_createdOnThisComputer = val;
    emit createdOnThisComputerChanged();
}

void ScriteDocument::screenplayElementRemoved(ScreenplayElement *ptr, int)
{
    Scene *scene = ptr->scene();
    int index = m_screenplay->firstIndexOfScene(scene);
    if (index < 0) {
        index = m_structure->indexOfScene(scene);
        if (index >= 0) {
            StructureElement *element = m_structure->elementAt(index);
            element->setStackId(QString());
        }
    }
}

void ScriteDocument::screenplayElementMoved(ScreenplayElement *ptr, int from, int to)
{
    Q_UNUSED(from);
    Q_UNUSED(to);

    /**
     * When a screenplay element is moved, in other words when a scenes are
     * resequenced in the screenplay, the corresponding structure element may
     * be part of a stack. Simply moving that one structure element out of the
     * stack can leave the structure canvas with overlapping loops.
     *
     * So, it is best if we completely unstack the whole thing and then restack
     * them as needed.
     */

    StructureElement *element = ptr->scene() ? ptr->scene()->structureElement() : nullptr;
    if (element == nullptr)
        return;

    // Split the stack from which the element was moved
    const QString stackId = element->stackId();
    if (!stackId.isEmpty()) {
        auto unstackElement = qScopeGuard([=] { element->setStackId(QString()); });
        Q_UNUSED(unstackElement)

        StructureElementStack *stack = m_structure->elementStacks()->findStackById(stackId);

        if (stack != nullptr && stack->indexOf(element) >= 0) {
            stack->sortByScreenplayOccurance(m_screenplay);

            const QList<StructureElement *> stackElements = stack->constList();
            const int elementIndex = stackElements.indexOf(element);
            StructureElementStack::stackEm(stackElements.mid(0, elementIndex));
            StructureElementStack::stackEm(stackElements.mid(elementIndex + 1));
        }
    }
}

void ScriteDocument::screenplayAboutToMoveElements(int at)
{
    const ScreenplayElement *previousElementAfterMove = m_screenplay->elementAt(at - 1);
    const ScreenplayElement *nextElementAfterMove = m_screenplay->elementAt(at);
    if (previousElementAfterMove == nullptr || nextElementAfterMove == nullptr)
        return;

    if (previousElementAfterMove->scene() == nullptr || nextElementAfterMove->scene() == nullptr)
        return;

    StructureElement *previousStructureElementAfterMove =
            previousElementAfterMove->scene()->structureElement();
    StructureElement *nextStructureElementAfterMove =
            nextElementAfterMove->scene()->structureElement();
    if (previousStructureElementAfterMove->stackId() != nextStructureElementAfterMove->stackId())
        return;

    if (previousStructureElementAfterMove->stackId().isEmpty())
        return;

    // Split the stack into which the element is moved.
    StructureElementStack *stack = m_structure->elementStacks()->findStackById(
            previousStructureElementAfterMove->stackId());
    if (stack == nullptr)
        return;

    stack->sortByScreenplayOccurance(m_screenplay);

    const int previousElementIndex = stack->constList().indexOf(previousStructureElementAfterMove);
    const int nextElementIndex = stack->constList().indexOf(nextStructureElementAfterMove);
    StructureElementStack::stackEm(stack->constList().mid(0, previousElementIndex + 1));
    StructureElementStack::stackEm(stack->constList().mid(nextElementIndex));
}

void ScriteDocument::clearModifiedLater()
{
    ExecLaterTimer::call(
            "ScriteDocument::clearModified", this, [=]() { this->setModified(false); }, 250);
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
    json.insert(QStringLiteral("collaborators"), QJsonValue::fromVariant(m_collaborators));
    json.insert(QStringLiteral("documentId"), m_documentId);

    QJsonObject metaInfo;
    metaInfo.insert(QStringLiteral("appName"), qApp->applicationName());
    metaInfo.insert(QStringLiteral("orgName"), qApp->organizationName());
    metaInfo.insert(QStringLiteral("orgDomain"), qApp->organizationDomain());
    metaInfo.insert(QStringLiteral("appVersion"), QStringLiteral("0.9.9"));

    QVersionNumber appVersion =
            QVersionNumber::fromString(Application::instance()->versionNumber().toString());
    if (appVersion.microVersion() % 2)
        appVersion = QVersionNumber(appVersion.majorVersion(), appVersion.minorVersion(),
                                    appVersion.microVersion() - 1);
    metaInfo.insert(QStringLiteral("createdWith"), appVersion.toString());

    QJsonObject systemInfo;
    systemInfo.insert(QStringLiteral("machineHostName"), QSysInfo::machineHostName());
    systemInfo.insert(QStringLiteral("machineUniqueId"),
                      QString::fromLatin1(QSysInfo::machineUniqueId()));
    systemInfo.insert(QStringLiteral("prettyProductName"), QSysInfo::prettyProductName());
    systemInfo.insert(QStringLiteral("productType"), QSysInfo::productType());
    systemInfo.insert(QStringLiteral("productVersion"), QSysInfo::productVersion());
    metaInfo.insert(QStringLiteral("system"), systemInfo);

    QJsonObject installationInfo;
    installationInfo.insert(QStringLiteral("id"), Application::instance()->installationId());
    installationInfo.insert(QStringLiteral("since"),
                            Application::instance()->installationTimestamp().toMSecsSinceEpoch());
    installationInfo.insert(QStringLiteral("launchCount"),
                            Application::instance()->launchCounter());
    metaInfo.insert(QStringLiteral("installation"), installationInfo);

    json.insert(QStringLiteral("meta"), metaInfo);
}

void ScriteDocument::deserializeFromJson(const QJsonObject &json)
{
    const QString storedDocumentId = json.value(QStringLiteral("documentId")).toString();
    if (storedDocumentId.isEmpty()) {
        m_documentId = QUuid::createUuid().toString();

        // Without a proper document-id, we will end up having too many restore
        // points of the same doucment.
        QTimer::singleShot(0, this, [=]() {
            if (!m_fileName.isEmpty())
                this->save();
        });
    } else
        m_documentId = storedDocumentId;
    emit documentIdChanged();

    const QJsonObject metaInfo = json.value(QStringLiteral("meta")).toObject();
    const QJsonObject systemInfo = metaInfo.value(QStringLiteral("system")).toObject();

    const QString thisMachineId = QString::fromLatin1(QSysInfo::machineUniqueId());
    const QString jsonMachineId = systemInfo.value(QStringLiteral("machineUniqueId")).toString();
    this->setCreatedOnThisComputer(jsonMachineId == thisMachineId);

    const QString appVersion = metaInfo.value(QStringLiteral("appVersion")).toString();
    const QVersionNumber version = QVersionNumber::fromString(appVersion);
    if (version <= QVersionNumber(0, 1, 9)) {
        const qreal dx = -130;
        const qreal dy = -22;

        const int nrElements = m_structure->elementCount();
        for (int i = 0; i < nrElements; i++) {
            StructureElement *element = m_structure->elementAt(i);
            element->setX(element->x() + dx);
            element->setY(element->y() + dy);
        }
    }

    if (version <= QVersionNumber(0, 2, 6)) {
        const int nrElements = m_structure->elementCount();
        for (int i = 0; i < nrElements; i++) {
            StructureElement *element = m_structure->elementAt(i);
            Scene *scene = element->scene();
            if (scene == nullptr)
                continue;

            SceneHeading *heading = scene->heading();
            QString val = heading->locationType();
            if (val == QStringLiteral("INTERIOR"))
                val = QStringLiteral("INT");
            if (val == QStringLiteral("EXTERIOR"))
                val = QStringLiteral("EXT");
            if (val == QStringLiteral("BOTH"))
                val = "I/E";
            heading->setLocationType(val);
        }
    }

    if (m_screenplay->currentElementIndex() < 0) {
        if (m_screenplay->elementCount() > 0)
            m_screenplay->setCurrentElementIndex(0);
        else if (m_structure->elementCount() > 0)
            m_structure->setCurrentElementIndex(0);
    }

    const QVector<QColor> versionColors = Application::standardColors(version);
    const QVector<QColor> newColors = Application::standardColors(QVersionNumber());
    if (versionColors != newColors) {
        auto evalNewColor = [versionColors, newColors](const QColor &color) {
            const int oldColorIndex = versionColors.indexOf(color);
            const QColor newColor = oldColorIndex < 0
                    ? newColors.last()
                    : newColors.at(oldColorIndex % newColors.size());
            return newColor;
        };

        const int nrElements = m_structure->elementCount();
        for (int i = 0; i < nrElements; i++) {
            StructureElement *element = m_structure->elementAt(i);
            Scene *scene = element->scene();
            if (scene == nullptr)
                continue;

            scene->setColor(evalNewColor(scene->color()));

            const int nrNotes = scene->notes()->noteCount();
            for (int n = 0; n < nrNotes; n++) {
                Note *note = scene->notes()->noteAt(n);
                note->setColor(evalNewColor(note->color()));
            }
        }

        const int nrNotes = m_structure->notes()->noteCount();
        for (int n = 0; n < nrNotes; n++) {
            Note *note = m_structure->notes()->noteAt(n);
            note->setColor(evalNewColor(note->color()));
        }

        const int nrChars = m_structure->characterCount();
        for (int c = 0; c < nrChars; c++) {
            Character *character = m_structure->characterAt(c);

            const int nrNotes = character->notes()->noteCount();
            for (int n = 0; n < nrNotes; n++) {
                Note *note = character->notes()->noteAt(n);
                note->setColor(evalNewColor(note->color()));
            }
        }
    }

    // With Version 0.3.9, we have completely changed the way in which we
    // store formatting options. So, the old formatting options data doesnt
    // work anymore. We better reset to defaults in the new version and then
    // let the user alter it anyway he sees fit.
    if (version <= QVersionNumber(0, 3, 9)) {
        m_formatting->resetToFactoryDefaults();
        m_printFormat->resetToFactoryDefaults();
    }

    // Starting with 0.4.5, it is possible for users to lock a document
    // such that it is editable only the system in which it was created.
    if (m_locked && !m_readOnly) {
        const QJsonObject installationInfo =
                metaInfo.value(QStringLiteral("installation")).toObject();
        const QString docClientId = installationInfo.value(QStringLiteral("id")).toString();
        const QString myClientId = Application::instance()->installationId();
        if (!myClientId.isEmpty() && !docClientId.isEmpty()) {
            const bool ro = myClientId != docClientId;
            this->setReadOnly(ro);
            if (ro) {
                Notification *notification = new Notification(this);
                connect(notification, &Notification::dismissed, &Notification::deleteLater);

                notification->setTitle(QStringLiteral("Document is locked for edit."));
                notification->setText(QStringLiteral(
                        "This document is being opened in read only mode.\nYou cannot edit "
                        "this "
                        "document on your "
                        "computer, because it has been locked for edit on another "
                        "computer.\nYou "
                        "can however save a "
                        "copy using the 'Save As' option and edit the copy on your computer."));
                notification->setAutoClose(false);
                notification->setActive(true);
            }
        }
    }

    if (version <= QVersionNumber(0, 4, 7)) {
        for (int i = SceneElement::Min; i <= SceneElement::Max; i++) {
            SceneElementFormat *format = m_formatting->elementFormat(SceneElement::Type(i));
            if (qFuzzyCompare(format->lineHeight(), 1.0))
                format->setLineHeight(0.85);
            const qreal lineHeight = format->lineHeight();

            format = m_printFormat->elementFormat(SceneElement::Type(i));
            format->setLineHeight(lineHeight);
        }
    }

    // From version 0.4.14 onwards we allow users to set their own custom fonts
    // for each language. This is a deviation from using "Courier Prime" as the
    // default Latin font.
    m_formatting->useUserSpecifiedFonts();
    m_printFormat->useUserSpecifiedFonts();

    // Although its not specified anywhere that transitions must be right aligned,
    // many writers who are early adopters of Scrite are insisting on it.
    // So, going forward transition paragraphs will be right aligned by default.
    if (version <= QVersionNumber(0, 5, 1)) {
        SceneElementFormat *format = m_formatting->elementFormat(SceneElement::Transition);
        format->setTextAlignment(Qt::AlignRight);

        format = m_printFormat->elementFormat(SceneElement::Transition);
        format->setTextAlignment(Qt::AlignRight);
    }

    // Documents created using Scrite version 0.5.2 or before use SynopsisEditorUI
    // by default.
    if (version <= QVersionNumber(0, 5, 2))
        m_structure->setCanvasUIMode(Structure::SynopsisEditorUI);
}

QString ScriteDocument::polishFileName(const QString &givenFileName) const
{
    QString fileName = givenFileName.trimmed();

    if (!fileName.isEmpty()) {
        QFileInfo fi(fileName);
        if (fi.isDir())
            fileName = fi.absolutePath() + QStringLiteral("/Screenplay-")
                    + QString::number(QDateTime::currentSecsSinceEpoch())
                    + QStringLiteral(".scrite");
        else if (fi.suffix() != QStringLiteral("scrite"))
            fileName += QStringLiteral(".scrite");
    }

    return fileName;
}

void ScriteDocument::setSessionId(QString val)
{
    if (m_sessionId == val)
        return;

    m_sessionId = val;
    emit sessionIdChanged();
}

void ScriteDocument::setDocumentId(const QString &val)
{
    if (m_documentId == val)
        return;

    m_documentId = val;
    emit documentIdChanged();
}

void ScriteDocument::initializeFileModificationTracker()
{
    m_fileTracker = new FileModificationTracker(this);

    connect(this, &ScriteDocument::aboutToSave, m_fileTracker,
            [=]() { m_fileTracker->pauseTracking(250); });

    auto watchCurrentFile = [=]() { m_fileTracker->setFilePath(m_fileName); };

    connect(this, &ScriteDocument::fileNameChanged, m_fileTracker, watchCurrentFile);
    connect(this, &ScriteDocument::justLoaded, m_fileTracker, watchCurrentFile);
    connect(this, &ScriteDocument::documentWindowTitleChanged, m_fileTracker, watchCurrentFile);
    connect(m_fileTracker, &FileModificationTracker::fileModified, this,
            &ScriteDocument::trackedFileModified);
}

void ScriteDocument::trackedFileModified(const QString &fileName)
{
    if (!m_fileName.isEmpty() && m_fileName == fileName && QFileInfo(m_fileName).isWritable()) {
        emit requiresReload();
    }
}

///////////////////////////////////////////////////////////////////////////////

StructureElementConnectors::StructureElementConnectors(ScriteDocument *parent)
    : QAbstractListModel(parent), m_document(parent)
{
}

StructureElementConnectors::~StructureElementConnectors() { }

StructureElement *StructureElementConnectors::fromElement(int row) const
{
    if (row < 0 || row >= m_items.size())
        return nullptr;

    const Item item = m_items.at(row);
    return item.from;
}

StructureElement *StructureElementConnectors::toElement(int row) const
{
    if (row < 0 || row >= m_items.size())
        return nullptr;

    const Item item = m_items.at(row);
    return item.to;
}

QString StructureElementConnectors::label(int row) const
{
    if (row < 0 || row >= m_items.size())
        return nullptr;

    const Item item = m_items.at(row);
    return item.label;
}

int StructureElementConnectors::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_items.size();
}

QVariant StructureElementConnectors::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_items.size())
        return QVariant();

    const Item item = m_items.at(index.row());
    switch (role) {
    case FromElementRole:
        return QVariant::fromValue<QObject *>(item.from);
    case ToElementRole:
        return QVariant::fromValue<QObject *>(item.to);
    case LabelRole:
        return item.label;
    default:
        break;
    }

    return QVariant();
}

QHash<int, QByteArray> StructureElementConnectors::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[FromElementRole] = QByteArrayLiteral("connectorFromElement");
    roles[ToElementRole] = QByteArrayLiteral("connectorToElement");
    roles[LabelRole] = QByteArrayLiteral("connectorLabel");
    return roles;
}

void StructureElementConnectors::clear()
{
    if (m_items.isEmpty())
        return;

    this->beginResetModel();
    m_items.clear();
    this->endResetModel();
    emit countChanged();
}

void StructureElementConnectors::reload()
{
    const Structure *structure = m_document->structure();
    const Screenplay *screenplay = m_document->screenplay();
    if (structure == nullptr || screenplay == nullptr) {
        this->clear();
        return;
    }

    const int nrElements = screenplay->elementCount();
    if (nrElements <= 1) {
        this->clear();
        return;
    }

    ScreenplayElement *fromElement = nullptr;
    ScreenplayElement *toElement = nullptr;
    int fromIndex = -1;
    int toIndex = -1;
    int itemIndex = 0;

    const bool itemsWasEmpty = m_items.isEmpty();
    if (itemsWasEmpty)
        this->beginResetModel();

    for (int i = 0; i < nrElements - 1; i++) {
        fromElement = fromElement ? fromElement : screenplay->elementAt(i);
        toElement = toElement ? toElement : screenplay->elementAt(i + 1);
        fromIndex = fromIndex >= 0 ? fromIndex : structure->indexOfScene(fromElement->scene());
        toIndex = toIndex >= 0 ? toIndex : structure->indexOfScene(toElement->scene());

        if (fromIndex >= 0 && toIndex >= 0) {
            Item item;
            item.from = structure->elementAt(fromIndex);
            item.to = structure->elementAt(toIndex);
            item.label = QString::number(itemIndex + 1);

            if (itemsWasEmpty)
                m_items.append(item);
            else {
                if (itemIndex < m_items.size()) {
                    if (!(m_items.at(itemIndex) == item)) {
                        this->beginRemoveRows(QModelIndex(), itemIndex, itemIndex);
                        m_items.removeAt(itemIndex);
                        this->endRemoveRows();

                        this->beginInsertRows(QModelIndex(), itemIndex, itemIndex);
                        m_items.insert(itemIndex, item);
                        this->endInsertRows();
                    }
                } else {
                    this->beginInsertRows(QModelIndex(), itemIndex, itemIndex);
                    m_items.append(item);
                    this->endInsertRows();
                    emit countChanged();
                }
            }

            ++itemIndex;

            fromElement = toElement;
            fromIndex = toIndex;
        } else {
            fromElement = nullptr;
            fromIndex = -1;
        }

        toElement = nullptr;
        toIndex = -1;
    }

    if (itemsWasEmpty) {
        this->endResetModel();
        emit countChanged();
    } else {
        const int expectedCount = itemIndex;
        if (m_items.size() > expectedCount) {
            this->beginRemoveRows(QModelIndex(), expectedCount, m_items.size() - 1);
            while (m_items.size() != expectedCount)
                m_items.removeLast();
            this->endRemoveRows();
        }
    }
}

QJsonObject ScriteDocumentBackups::MetaData::toJson() const
{
    QJsonObject ret;
    ret.insert(QStringLiteral("loaded"), this->loaded);
    ret.insert(QStringLiteral("structureElementCount"), this->structureElementCount);
    ret.insert(QStringLiteral("screenplayElementCount"), this->screenplayElementCount);
    ret.insert(QStringLiteral("sceneCount"),
               qMax(this->structureElementCount, this->screenplayElementCount));
    return ret;
}

///////////////////////////////////////////////////////////////////////////////

ScriteDocumentCollaborators::ScriteDocumentCollaborators(QObject *parent)
    : QAbstractListModel(parent)
{
    this->setDocument(ScriteDocument::instance());
}

ScriteDocumentCollaborators::~ScriteDocumentCollaborators() { }

void ScriteDocumentCollaborators::setDocument(ScriteDocument *val)
{
    if (m_document == val)
        return;

    if (m_document)
        disconnect(m_document, nullptr, this, nullptr);

    if (!m_otherCollaborators.isEmpty()) {
        this->beginRemoveRows(QModelIndex(), 0, m_otherCollaborators.size());
        m_otherCollaborators.clear();
        this->endRemoveRows();
    }

    m_document = val;

    if (m_document) {
        connect(m_document, &ScriteDocument::collaboratorsChanged, this,
                &ScriteDocumentCollaborators::updateModelAndFetchUsersInfoIfRequired);
        this->updateModelAndFetchUsersInfoIfRequired();
    }

    emit documentChanged();
}

int ScriteDocumentCollaborators::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_otherCollaborators.size();
}

QVariant ScriteDocumentCollaborators::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_otherCollaborators.size())
        return QVariant();

    const auto item = m_otherCollaborators.at(index.row());
    switch (role) {
    case CollaboratorNameRole:
        return item.second;
    case CollaboratorEmailRole:
        return item.first;
    case CollaboratorRole:
        return item.second.isEmpty()
                ? item.first
                : (item.second + QStringLiteral(" <") + item.first + QStringLiteral(">"));
    }

    return QVariant();
}

QHash<int, QByteArray> ScriteDocumentCollaborators::roleNames() const
{
    return QHash<int, QByteArray>(
            { { CollaboratorRole, QByteArrayLiteral("collaborator") },
              { CollaboratorEmailRole, QByteArrayLiteral("collaboratorEmail") },
              { CollaboratorNameRole, QByteArrayLiteral("collaboratorName") } });
}

int ScriteDocumentCollaborators::updateModel()
{
    int nrUnknownEmails = 0;

    QMap<QString, QString> infoMap;
    for (auto &item : m_otherCollaborators)
        infoMap[item.first] = item.second;

    this->beginResetModel();

    const QStringList collaborators = m_document ? m_document->otherCollaborators() : QStringList();
    m_otherCollaborators.clear();

    for (const QString &collaborator : collaborators) {
        auto item = qMakePair(collaborator, infoMap.value(collaborator));
        if (item.second.isEmpty()) {
            const QJsonObject userInfo = m_usersInfoMap.value(item.first).toObject();
            if (!userInfo.isEmpty()) {
                const QString firstName = userInfo.value(QStringLiteral("firstName")).toString();
                const QString lastName = userInfo.value(QStringLiteral("lastName")).toString();
                item.second =
                        QStringList({ firstName, lastName }).join(QStringLiteral(" ")).trimmed();
                if (item.second.isEmpty())
                    item.second = QStringLiteral("Registered User");
            }
        }
        m_otherCollaborators.append(item);

        if (item.second.isEmpty())
            ++nrUnknownEmails;
    }

    this->endResetModel();

    return nrUnknownEmails;
}

void ScriteDocumentCollaborators::fetchUsersInfo()
{
    if (!m_document)
        return;

    const QStringList collaborators = m_document->otherCollaborators();
    if (collaborators.isEmpty())
        return;

    UserCheckRestApiCall *call =
            this->findChild<UserCheckRestApiCall *>(QString(), Qt::FindDirectChildrenOnly);
    if (call != nullptr) {
        ++m_pendingFetchUsersInfoRequests;
        return;
    }

    QStringList pendingCollaborators;
    for (const QString &collaborator : collaborators) {
        if (m_usersInfoMap.contains(collaborator))
            continue;
        pendingCollaborators << collaborator;
    }

    if (pendingCollaborators.isEmpty())
        return;

    call = new UserCheckRestApiCall(this);
    call->setEmails(pendingCollaborators);
    connect(call, &UserCheckRestApiCall::finished, this,
            &ScriteDocumentCollaborators::onCallFinished);
    call->call();
}

void ScriteDocumentCollaborators::updateModelAndFetchUsersInfoIfRequired()
{
    if (this->updateModel() > 0)
        this->fetchUsersInfo();
}

void ScriteDocumentCollaborators::onCallFinished()
{
    UserCheckRestApiCall *call = qobject_cast<UserCheckRestApiCall *>(this->sender());
    if (call == nullptr)
        call = this->findChild<UserCheckRestApiCall *>(QString(), Qt::FindDirectChildrenOnly);
    if (call == nullptr)
        return;

    if (call->hasError() || !call->hasResponse())
        return;

    const QJsonObject response = call->responseData();
    auto it = response.begin();
    auto end = response.end();
    while (it != end) {
        m_usersInfoMap.insert(it.key(), it.value());
        ++it;
    }

    this->updateModel();

    if (m_pendingFetchUsersInfoRequests > 0)
        QTimer::singleShot(100, this, &ScriteDocumentCollaborators::fetchUsersInfo);
}
