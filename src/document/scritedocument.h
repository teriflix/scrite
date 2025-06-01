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

#ifndef SCRITEDOCUMENT_H
#define SCRITEDOCUMENT_H

#include <QDir>
#include <QJsonArray>
#include <QQmlEngine>

#include "screenplay.h"
#include "structure.h"
#include "formatting.h"
#include "errorreport.h"
#include "progressreport.h"
#include "qobjectproperty.h"
#include "qobjectserializer.h"
#include "documentfilesystem.h"

class Forms;
class FileLocker;
class ScriteDocument;
class AbstractExporter;
class QFileSystemWatcher;
class AbstractReportGenerator;
class FileModificationTracker;

class StructureElementConnectors : public QAbstractListModel
{
    Q_OBJECT

public:
    ~StructureElementConnectors();

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    int count() const { return m_items.size(); }
    Q_SIGNAL void countChanged();

    StructureElement *fromElement(int row) const;
    StructureElement *toElement(int row) const;
    QString label(int row) const;

    // QAbstractItemModel interface
    enum { FromElementRole = Qt::UserRole, ToElementRole, LabelRole };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

private:
    StructureElementConnectors(ScriteDocument *parent = nullptr);
    void clear();
    void reload();

private:
    friend class ScriteDocument;
    ScriteDocument *m_document = nullptr;

    struct Item
    {
        StructureElement *from = nullptr;
        StructureElement *to = nullptr;
        QString label;
        bool operator==(const Item &other) const
        {
            return from == other.from && to == other.to && label == other.label;
        }
    };
    QList<Item> m_items;
};

class ScriteDocumentBackups : public QAbstractListModel
{
    Q_OBJECT

public:
    ~ScriteDocumentBackups();

    Q_PROPERTY(QString documentFilePath READ documentFilePath NOTIFY documentFilePathChanged)
    QString documentFilePath() const { return m_documentFilePath; }
    Q_SIGNAL void documentFilePathChanged();

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    int count() const { return m_backupFiles.size(); }
    Q_SIGNAL void countChanged();

    Q_INVOKABLE QJsonObject at(int index) const;
    Q_INVOKABLE QJsonObject latestBackup() const { return this->at(0); }
    Q_INVOKABLE QJsonObject oldestBackup() const { return this->at(m_backupFiles.size() - 1); }

    // QAbstractItemModel interface
    enum Roles {
        TimestampRole = Qt::UserRole,
        TimestampAsStringRole,
        RelativeTimeRole,
        FileNameRole,
        FilePathRole,
        FileSizeRole,
        MetaDataRole
    };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

    static QString relativeTime(const QDateTime &dt);

private:
    friend class ScriteDocument;
    ScriteDocumentBackups(QObject *parent = nullptr);
    void setDocumentFilePath(const QString &val);
    void loadBackupFileInformation();
    void reloadBackupFileInformation();
    void loadMetaData(int row);
    void clear();

private:
    struct MetaData
    {
        bool loaded = false;
        int structureElementCount = 0;
        int screenplayElementCount = 0;
        QJsonObject toJson() const;
    };

    QTimer m_reloadTimer;
    QDir m_backupFilesDir;
    QString m_documentFilePath;
    QFileInfoList m_backupFiles;
    QVector<MetaData> m_metaDataList;
    QFileSystemWatcher *m_fsWatcher = nullptr;
};

class ScriteDocumentCollaborators : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ScriteDocumentCollaborators(QObject *parent = nullptr);
    ~ScriteDocumentCollaborators();

    Q_PROPERTY(ScriteDocument* document READ document WRITE setDocument NOTIFY documentChanged)
    void setDocument(ScriteDocument *val);
    ScriteDocument *document() const { return m_document; }
    Q_SIGNAL void documentChanged();

    // QAbstractItemModel interface
    enum { CollaboratorRole = Qt::UserRole, CollaboratorEmailRole, CollaboratorNameRole };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

private:
    int updateModel();
    void fetchUsersInfo();
    void updateModelAndFetchUsersInfoIfRequired();
    void onCallFinished();

private:
    ScriteDocument *m_document = nullptr;
    QJsonObject m_usersInfoMap;
    int m_pendingFetchUsersInfoRequests = 0;
    QList<QPair<QString, QString>> m_otherCollaborators;
};

class PageSetup : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

    PageSetup(QObject *parent = nullptr);

public:
    ~PageSetup();

    Q_PROPERTY(bool usingFactoryDefaults READ usingFactoryDefaults NOTIFY usingFactoryDefaultsChanged)
    bool usingFactoryDefaults() const { return m_isFactoryDefaults; }
    Q_SIGNAL void usingFactoryDefaultsChanged();

    Q_PROPERTY(bool usingSavedDefaults READ usingSavedDefaults NOTIFY usingSavedDefaultsChanged)
    bool usingSavedDefaults() const { return m_isSavedDefaults; }
    Q_SIGNAL void usingSavedDefaultsChanged();

    // must be in one of ScreenplayPageLayout::PageSize
    Q_PROPERTY(int paperSize READ paperSize WRITE setPaperSize NOTIFY paperSizeChanged)
    void setPaperSize(int val);
    int paperSize() const { return m_paperSize; }
    Q_SIGNAL void paperSizeChanged();

    // Must be one of HeaderFooter::Field
    Q_PROPERTY(int headerLeft READ headerLeft WRITE setHeaderLeft NOTIFY headerLeftChanged)
    void setHeaderLeft(int val);
    int headerLeft() const { return m_headerLeft; }
    Q_SIGNAL void headerLeftChanged();

    Q_PROPERTY(int headerCenter READ headerCenter WRITE setHeaderCenter NOTIFY headerCenterChanged)
    void setHeaderCenter(int val);
    int headerCenter() const { return m_headerCenter; }
    Q_SIGNAL void headerCenterChanged();

    Q_PROPERTY(int headerRight READ headerRight WRITE setHeaderRight NOTIFY headerRightChanged)
    void setHeaderRight(int val);
    int headerRight() const { return m_headerRight; }
    Q_SIGNAL void headerRightChanged();

    Q_PROPERTY(qreal headerOpacity READ headerOpacity WRITE setHeaderOpacity NOTIFY headerOpacityChanged STORED false)
    void setHeaderOpacity(qreal val);
    qreal headerOpacity() const { return m_headerOpacity; }
    Q_SIGNAL void headerOpacityChanged();

    Q_PROPERTY(int footerLeft READ footerLeft WRITE setFooterLeft NOTIFY footerLeftChanged)
    void setFooterLeft(int val);
    int footerLeft() const { return m_footerLeft; }
    Q_SIGNAL void footerLeftChanged();

    Q_PROPERTY(int footerCenter READ footerCenter WRITE setFooterCenter NOTIFY footerCenterChanged)
    void setFooterCenter(int val);
    int footerCenter() const { return m_footerCenter; }
    Q_SIGNAL void footerCenterChanged();

    Q_PROPERTY(int footerRight READ footerRight WRITE setFooterRight NOTIFY footerRightChanged)
    void setFooterRight(int val);
    int footerRight() const { return m_footerRight; }
    Q_SIGNAL void footerRightChanged();

    Q_PROPERTY(qreal footerOpacity READ footerOpacity WRITE setFooterOpacity NOTIFY footerOpacityChanged STORED false)
    void setFooterOpacity(qreal val);
    qreal footerOpacity() const { return m_footerOpacity; }
    Q_SIGNAL void footerOpacityChanged();

    Q_PROPERTY(bool watermarkEnabled READ isWatermarkEnabled WRITE setWatermarkEnabled NOTIFY watermarkEnabledChanged)
    void setWatermarkEnabled(bool val);
    bool isWatermarkEnabled() const { return m_watermarkEnabled; }
    Q_SIGNAL void watermarkEnabledChanged();

    Q_PROPERTY(QString watermarkText READ watermarkText WRITE setWatermarkText NOTIFY watermarkTextChanged)
    void setWatermarkText(const QString &val);
    QString watermarkText() const { return m_watermarkText; }
    Q_SIGNAL void watermarkTextChanged();

    Q_PROPERTY(QString watermarkFont READ watermarkFont WRITE setWatermarkFont NOTIFY watermarkFontChanged)
    void setWatermarkFont(const QString &val);
    QString watermarkFont() const { return m_watermarkFont; }
    Q_SIGNAL void watermarkFontChanged();

    Q_PROPERTY(int watermarkFontSize READ watermarkFontSize WRITE setWatermarkFontSize NOTIFY watermarkFontSizeChanged)
    void setWatermarkFontSize(int val);
    int watermarkFontSize() const { return m_watermarkFontSize; }
    Q_SIGNAL void watermarkFontSizeChanged();

    Q_PROPERTY(QColor watermarkColor READ watermarkColor WRITE setWatermarkColor NOTIFY watermarkColorChanged)
    void setWatermarkColor(const QColor &val);
    QColor watermarkColor() const { return m_watermarkColor; }
    Q_SIGNAL void watermarkColorChanged();

    Q_PROPERTY(qreal watermarkOpacity READ watermarkOpacity WRITE setWatermarkOpacity NOTIFY watermarkOpacityChanged STORED false)
    void setWatermarkOpacity(qreal val);
    qreal watermarkOpacity() const { return m_watermarkOpacity; }
    Q_SIGNAL void watermarkOpacityChanged();

    Q_PROPERTY(int watermarkRotation READ watermarkRotation WRITE setWatermarkRotation NOTIFY watermarkRotationChanged)
    void setWatermarkRotation(int val);
    int watermarkRotation() const { return m_watermarkRotation; }
    Q_SIGNAL void watermarkRotationChanged();

    Q_PROPERTY(int watermarkAlignment READ watermarkAlignment WRITE setWatermarkAlignment NOTIFY watermarkAlignmentChanged)
    void setWatermarkAlignment(int val);
    int watermarkAlignment() const { return m_watermarkAlignment; }
    Q_SIGNAL void watermarkAlignmentChanged();

public slots:
    void useFactoryDefaults();
    void saveAsDefaults();
    void useSavedDefaults();

signals:
    void pageSetupChanged();

private:
    void setUsingFactoryDefaults(bool val);
    void setUsingSavedDefaults(bool val);
    void evaluateDefaultsFlags();

private:
    friend class ScriteDocument;
    int m_headerLeft;
    int m_headerCenter;
    int m_headerRight;
    qreal m_headerOpacity;
    int m_footerLeft;
    int m_footerCenter;
    int m_footerRight;
    int m_paperSize;
    qreal m_footerOpacity;
    bool m_watermarkEnabled;
    QString m_watermarkText;
    QString m_watermarkFont;
    int m_watermarkFontSize;
    QColor m_watermarkColor;
    qreal m_watermarkOpacity;
    int m_watermarkRotation;
    int m_watermarkAlignment;

    bool m_isFactoryDefaults = false;
    bool m_isSavedDefaults = false;
    static QJsonObject m_factoryDefaults;
    QJsonObject m_savedDefaults;
};

class ScriteDocument : public QObject, public QObjectSerializer::Interface
{
    Q_OBJECT
    Q_INTERFACES(QObjectSerializer::Interface)
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

    ScriteDocument(QObject *parent = nullptr);

public:
    static ScriteDocument *instance();
    ~ScriteDocument();
    Q_SIGNAL void aboutToDelete(ScriteDocument *doc);

    Q_PROPERTY(bool readOnly READ isReadOnly NOTIFY readOnlyChanged)
    bool isReadOnly() const { return m_readOnly; }
    Q_SIGNAL void readOnlyChanged();

    Q_PROPERTY(bool locked READ isLocked WRITE setLocked NOTIFY lockedChanged)
    void setLocked(bool val);
    bool isLocked() const { return m_locked; }
    Q_SIGNAL void lockedChanged();

    Q_PROPERTY(bool empty READ isEmpty NOTIFY emptyChanged)
    bool isEmpty() const;
    Q_SIGNAL void emptyChanged();

    Q_PROPERTY(QString documentId READ documentId NOTIFY documentIdChanged)
    QString documentId() const { return m_documentId; }
    Q_SIGNAL void documentIdChanged();

    // Will be true of the currently open scrite document is from Scriptalay's
    // script library.
    Q_PROPERTY(bool fromScriptalay READ isFromScriptalay NOTIFY fromScriptalayChanged)
    void setFromScriptalay(bool val);
    bool isFromScriptalay() const { return m_fromScriptalay; }
    Q_SIGNAL void fromScriptalayChanged();

    Q_PROPERTY(QString sessionId READ sessionId NOTIFY sessionIdChanged)
    QString sessionId() const { return m_sessionId; }
    Q_SIGNAL void sessionIdChanged();

    // List of email-ids of collaborators who can read/modify this document
    // When this list is empty, anybody can alter the list.
    // When not empty, it can be altered only by the first collaborator in
    // the list. This attribute cannot be altered unless there is a User login.
    Q_PROPERTY(QStringList collaborators READ collaborators WRITE setCollaborators NOTIFY collaboratorsChanged STORED false)
    void setCollaborators(const QStringList &val);
    QStringList collaborators() const { return m_collaborators; }
    Q_SIGNAL void collaboratorsChanged();

    Q_PROPERTY(bool hasCollaborators READ hasCollaborators NOTIFY collaboratorsChanged)
    bool hasCollaborators() const { return !m_collaborators.isEmpty(); }

    Q_PROPERTY(QString primaryCollaborator READ primaryCollaborator NOTIFY collaboratorsChanged)
    QString primaryCollaborator() const
    {
        return m_collaborators.isEmpty() ? QString() : m_collaborators.first();
    }

    Q_PROPERTY(QStringList otherCollaborators READ otherCollaborators NOTIFY collaboratorsChanged)
    QStringList otherCollaborators() const { return m_collaborators.mid(1); }

    Q_PROPERTY(bool canModifyCollaborators READ canModifyCollaborators NOTIFY canModifyCollaboratorsChanged)
    bool canModifyCollaborators() const;
    Q_SIGNAL void canModifyCollaboratorsChanged();

    Q_INVOKABLE void addCollaborator(const QString &email);
    Q_INVOKABLE void removeCollaborator(const QString &email);
    Q_INVOKABLE void enableCollaboration();
    Q_INVOKABLE void disableCollaboration();

    Q_PROPERTY(int autoSaveDurationInSeconds READ autoSaveDurationInSeconds WRITE setAutoSaveDurationInSeconds NOTIFY autoSaveDurationInSecondsChanged STORED false)
    void setAutoSaveDurationInSeconds(int val);
    int autoSaveDurationInSeconds() const { return m_autoSaveDurationInSeconds; }
    Q_SIGNAL void autoSaveDurationInSecondsChanged();

    Q_PROPERTY(bool autoSave READ isAutoSave WRITE setAutoSave NOTIFY autoSaveChanged STORED false)
    void setAutoSave(bool val);
    bool isAutoSave() const { return m_autoSave; }
    Q_SIGNAL void autoSaveChanged();

    Q_PROPERTY(bool busy READ isBusy NOTIFY busyChanged STORED false)
    void setBusy(bool val);
    bool isBusy() const { return m_busy; }
    Q_SIGNAL void busyChanged();

    Q_PROPERTY(QString busyMessage READ busyMessage NOTIFY busyMessageChanged STORED false)
    Q_INVOKABLE void setBusyMessage(const QString &val);
    QString busyMessage() const { return m_busyMessage; }
    Q_SIGNAL void busyMessageChanged();

    Q_INVOKABLE void clearBusyMessage() { this->setBusyMessage(QString()); }

    Q_PROPERTY(QString documentWindowTitle READ documentWindowTitle NOTIFY documentWindowTitleChanged)
    QString documentWindowTitle() const { return m_documentWindowTitle; }
    Q_SIGNAL void documentWindowTitleChanged(const QString &val);

    Q_PROPERTY(Forms* forms READ forms NOTIFY formsChanged)
    Forms *forms() const { return m_forms; }
    Q_SIGNAL void formsChanged();

    Q_PROPERTY(Structure* structure READ structure NOTIFY structureChanged)
    Structure *structure() const { return m_structure; }
    Q_SIGNAL void structureChanged();

    Q_PROPERTY(Screenplay* screenplay READ screenplay NOTIFY screenplayChanged)
    Screenplay *screenplay() const { return m_screenplay; }
    Q_SIGNAL void screenplayChanged();

    Q_PROPERTY(ScreenplayFormat* displayFormat READ formatting NOTIFY formattingChanged STORED false)
    Q_PROPERTY(ScreenplayFormat* formatting READ formatting NOTIFY formattingChanged)
    ScreenplayFormat *formatting() const { return m_formatting; }
    Q_SIGNAL void formattingChanged();

    Q_PROPERTY(ScreenplayFormat* printFormat READ printFormat NOTIFY printFormatChanged)
    ScreenplayFormat *printFormat() const { return m_printFormat; }
    Q_SIGNAL void printFormatChanged();

    Q_PROPERTY(QStringList spellCheckIgnoreList READ spellCheckIgnoreList WRITE setSpellCheckIgnoreList NOTIFY spellCheckIgnoreListChanged)
    void setSpellCheckIgnoreList(const QStringList &val);
    QStringList spellCheckIgnoreList() const { return m_spellCheckIgnoreList; }
    Q_SIGNAL void spellCheckIgnoreListChanged();

    Q_INVOKABLE void addToSpellCheckIgnoreList(const QString &word);

    Q_PROPERTY(Forms *globalForms READ globalForms CONSTANT STORED false)
    Forms *globalForms() const;

    Form *requestForm(const QString &id);
    void releaseForm(Form *form);

    Q_PROPERTY(PageSetup* pageSetup READ pageSetup NOTIFY pageSetupChanged)
    PageSetup *pageSetup() const { return m_pageSetup; }
    Q_SIGNAL void pageSetupChanged();

    // This function adds a new scene to both structure and screenplay
    // and inserts it right after the current element in both.
    Q_INVOKABLE Scene *createNewScene(bool fuzzyScreenplayInsert = true);
    Q_SIGNAL void newSceneCreated(Scene *scene, int screenplayIndex);

    Q_PROPERTY(bool modified READ isModified NOTIFY modifiedChanged)
    bool isModified() const { return m_modified; }
    Q_SIGNAL void modifiedChanged();
    Q_SIGNAL void documentChanged();

    Q_PROPERTY(QString fileName READ fileName NOTIFY fileNameChanged)
    QString fileName() const { return m_fileName; }
    Q_SIGNAL void fileNameChanged();

    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)
    bool isLoading() const { return m_loading; }
    Q_SIGNAL void loadingChanged();

    Q_PROPERTY(QJsonObject userData READ userData WRITE setUserData NOTIFY userDataChanged)
    void setUserData(const QJsonObject &val);
    QJsonObject userData() const { return m_userData; }
    Q_SIGNAL void userDataChanged();

    Q_PROPERTY(QJsonArray bookmarkedNotes READ bookmarkedNotes WRITE setBookmarkedNotes NOTIFY bookmarkedNotesChanged)
    void setBookmarkedNotes(const QJsonArray &val);
    QJsonArray bookmarkedNotes() const { return m_bookmarkedNotes; }
    Q_SIGNAL void bookmarkedNotesChanged();

    Q_PROPERTY(bool isCreatedOnThisComputer READ isCreatedOnThisComputer NOTIFY createdOnThisComputerChanged)
    bool isCreatedOnThisComputer() const { return m_createdOnThisComputer; }
    Q_SIGNAL void createdOnThisComputerChanged();

    Q_PROPERTY(int maxBackupCount READ maxBackupCount WRITE setMaxBackupCount NOTIFY maxBackupCountChanged)
    void setMaxBackupCount(int val);
    int maxBackupCount() const { return m_maxBackupCount; }
    Q_SIGNAL void maxBackupCountChanged();

    Q_PROPERTY(bool canImportFromClipboard READ canImportFromClipboard NOTIFY canImportFromClipboardChanged)
    bool canImportFromClipboard() const;
    Q_SIGNAL void canImportFromClipboardChanged();

    Q_INVOKABLE void reset();
    Q_INVOKABLE void reload();

    Q_INVOKABLE static bool canBeBackupFileName(const QString &fileName);

    Q_INVOKABLE bool openOrImport(const QString &fileName);
    Q_INVOKABLE bool importFromClipboard();

    Q_INVOKABLE bool open(const QString &fileName);
    Q_INVOKABLE bool openAnonymously(const QString &fileName);
    Q_INVOKABLE void saveAs(const QString &fileName);
    Q_INVOKABLE void save();

    Q_SIGNAL void aboutToReset();
    Q_SIGNAL void aboutToSave();
    Q_SIGNAL void justReset();
    Q_SIGNAL void justSaved();
    Q_SIGNAL void justLoaded();
    Q_SIGNAL void openedAnonymously(const QString &fileName);
    Q_SIGNAL void requiresReload();

    Q_PROPERTY(QAbstractListModel* backupFilesModel READ backupFilesModel CONSTANT STORED false)
    ScriteDocumentBackups *backupFilesModel() const
    {
        return &((const_cast<ScriteDocument *>(this))->m_documentBackupsModel);
    }

    Q_PROPERTY(QStringList supportedImportFormats READ supportedImportFormats CONSTANT)
    QStringList supportedImportFormats() const;
    Q_INVOKABLE QString importFormatFileSuffix(const QString &format) const;

    Q_PROPERTY(QJsonArray supportedExportFormats READ supportedExportFormats CONSTANT)
    QJsonArray supportedExportFormats() const;

    Q_PROPERTY(QJsonArray supportedReports READ supportedReports CONSTANT)
    QJsonArray supportedReports() const;

    Q_PROPERTY(QJsonArray characterListReports READ characterListReports CONSTANT)
    QJsonArray characterListReports() const;

    Q_PROPERTY(QJsonArray sceneListReports READ sceneListReports CONSTANT)
    QJsonArray sceneListReports() const;

    Q_INVOKABLE QString reportFileSuffix() const;

    Q_INVOKABLE bool importFile(const QString &fileName, const QString &format);
    bool importFile(AbstractImporter *importer, const QString &fileName);
    Q_INVOKABLE bool exportFile(const QString &fileName, const QString &format);
    Q_INVOKABLE bool exportToImage(int fromSceneIdx, int fromParaIdx, int toSceneIdx, int toParaIdx,
                                   const QString &imageFileName);

    Q_INVOKABLE AbstractExporter *createExporter(const QString &format);
    Q_INVOKABLE AbstractReportGenerator *createReportGenerator(const QString &report);

    void setupExporter(AbstractExporter *exporter);
    void setupReportGenerator(AbstractReportGenerator *reportGenerator);

    Q_PROPERTY(QAbstractListModel* structureElementConnectors READ structureElementConnectors CONSTANT STORED false)
    QAbstractListModel *structureElementConnectors() const;

    void clearModified();

    // Callers must be responsible for how they use this.
    DocumentFileSystem *fileSystem() { return &m_docFileSystem; }
    Q_INVOKABLE void blockUI() { this->setLoading(true); }
    Q_INVOKABLE void unblockUI() { this->setLoading(false); }

protected:
    void timerEvent(QTimerEvent *event);

private:
    bool runSaveSanityChecks(const QString &fileName);
    void setReadOnly(bool val);
    void setLoading(bool val);
    void prepareAutoSave();
    void updateDocumentWindowTitle();
    void setDocumentWindowTitle(const QString &val);
    void setStructure(Structure *val);
    void setScreenplay(Screenplay *val);
    void setFormatting(ScreenplayFormat *val);
    void setPrintFormat(ScreenplayFormat *val);
    void setForms(Forms *val);
    void setPageSetup(PageSetup *val);
    void evaluateStructureElementSequence();
    void evaluateStructureElementSequenceLater();
    void markAsModified();
    void setModified(bool val);
    void setFileName(const QString &val);
    bool load(const QString &fileName, bool anonymousLoad = false);
    bool classicLoad(const QString &fileName);
    bool modernLoad(const QString &fileName, int *format = nullptr);
    void structureElementIndexChanged();
    void screenplayElementIndexChanged();
    void setCreatedOnThisComputer(bool val);
    void screenplayElementRemoved(ScreenplayElement *ptr, int index);
    void screenplayElementMoved(ScreenplayElement *ptr, int from, int to);
    void screenplayAboutToMoveElements(int at);
    void clearModifiedLater();

public:
    // QObjectSerializer::Interface implementation
    void prepareForSerialization();
    void prepareForDeserialization();
    bool canSerialize(const QMetaObject *, const QMetaProperty &) const;
    void serializeToJson(QJsonObject &) const;
    void deserializeFromJson(const QJsonObject &);

private:
    QString polishFileName(const QString &fileName) const;
    void setSessionId(QString val);
    void setDocumentId(const QString &val);

    void initializeFileModificationTracker();
    void trackedFileModified(const QString &fileName);

private:
    bool m_busy = false;
    bool m_locked = false;
    bool m_loading = false;
    bool m_modified = false;
    bool m_autoSave = true;
    bool m_readOnly = false;
    bool m_autoSaveMode = false;
    int m_maxBackupCount = 20;
    QString m_sessionId;
    bool m_fromScriptalay = false;
    QString m_documentId;
    QJsonObject m_userData;
    QString m_fileName;
    QString m_busyMessage;
    QStringList m_collaborators;
    FileLocker *m_fileLocker = nullptr;
    bool m_inCreateNewScene = false;
    bool m_createdOnThisComputer = true;
    QJsonArray m_bookmarkedNotes;
    ExecLaterTimer m_autoSaveTimer;
    QString m_documentWindowTitle;
    ExecLaterTimer m_clearModifyTimer;
    FileModificationTracker *m_fileTracker = nullptr;
    int m_autoSaveDurationInSeconds = 60;
    DocumentFileSystem m_docFileSystem;
    QStringList m_spellCheckIgnoreList;
    QJsonArray m_structureElementSequence;
    QObjectProperty<Structure> m_structure;
    StructureElementConnectors m_connectors;
    QObjectProperty<Screenplay> m_screenplay;
    ScriteDocumentBackups m_documentBackupsModel;
    QObjectProperty<ScreenplayFormat> m_formatting;
    QObjectProperty<ScreenplayFormat> m_printFormat;
    QObjectProperty<Forms> m_forms;
    QObjectProperty<PageSetup> m_pageSetup;
    ExecLaterTimer m_evaluateStructureElementSequenceTimer;
    bool m_syncingStructureScreenplayCurrentIndex = false;

    ErrorReport *m_errorReport = new ErrorReport(this);
    ProgressReport *m_progressReport = new ProgressReport(this);
};

#endif // SCRITEDOCUMENT_H
