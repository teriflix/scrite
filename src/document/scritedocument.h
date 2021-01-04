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

#ifndef SCRITEDOCUMENT_H
#define SCRITEDOCUMENT_H

#include <QObject>
#include <QJsonArray>

#include "screenplay.h"
#include "structure.h"
#include "formatting.h"
#include "errorreport.h"
#include "progressreport.h"
#include "qobjectproperty.h"
#include "qobjectserializer.h"
#include "documentfilesystem.h"

class AbstractExporter;
class AbstractReportGenerator;

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
    enum  {  FromElementRole = Qt::UserRole, ToElementRole, LabelRole };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

private:
    StructureElementConnectors(ScriteDocument *parent=nullptr);
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
        bool operator == (const Item &other) const {
            return from == other.from && to == other.to && label == other.label;
        }
    };
    QList<Item> m_items;
};

class ScriteDocument : public QObject, public QObjectSerializer::Interface
{
    Q_OBJECT
    Q_INTERFACES(QObjectSerializer::Interface)

    ScriteDocument(QObject *parent = nullptr);

public:
    static ScriteDocument *instance();
    ~ScriteDocument();

    Q_PROPERTY(bool readOnly READ isReadOnly NOTIFY readOnlyChanged)
    bool isReadOnly() const { return m_readOnly; }
    Q_SIGNAL void readOnlyChanged();

    Q_PROPERTY(bool locked READ isLocked WRITE setLocked NOTIFY lockedChanged)
    void setLocked(bool val);
    bool isLocked() const { return m_locked; }
    Q_SIGNAL void lockedChanged();

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
    void setBusyMessage(const QString &val);
    QString busyMessage() const { return m_busyMessage; }
    Q_SIGNAL void busyMessageChanged();

    void clearBusyMessage() { this->setBusyMessage(QString()); }

    Q_PROPERTY(QString documentWindowTitle READ documentWindowTitle NOTIFY documentWindowTitleChanged)
    QString documentWindowTitle() const { return m_documentWindowTitle; }
    Q_SIGNAL void documentWindowTitleChanged(const QString &val);

    Q_PROPERTY(Structure* structure READ structure NOTIFY structureChanged)
    Structure* structure() const { return m_structure; }
    Q_SIGNAL void structureChanged();

    Q_PROPERTY(Screenplay* screenplay READ screenplay NOTIFY screenplayChanged)
    Screenplay* screenplay() const { return m_screenplay; }
    Q_SIGNAL void screenplayChanged();

    Q_PROPERTY(ScreenplayFormat* displayFormat READ formatting NOTIFY formattingChanged STORED false)
    Q_PROPERTY(ScreenplayFormat* formatting READ formatting NOTIFY formattingChanged)
    ScreenplayFormat* formatting() const { return m_formatting; }
    Q_SIGNAL void formattingChanged();

    Q_PROPERTY(ScreenplayFormat* printFormat READ printFormat NOTIFY printFormatChanged)
    ScreenplayFormat* printFormat() const { return m_printFormat; }
    Q_SIGNAL void printFormatChanged();

    Q_PROPERTY(QStringList spellCheckIgnoreList READ spellCheckIgnoreList WRITE setSpellCheckIgnoreList NOTIFY spellCheckIgnoreListChanged)
    void setSpellCheckIgnoreList(const QStringList &val);
    QStringList spellCheckIgnoreList() const { return m_spellCheckIgnoreList; }
    Q_SIGNAL void spellCheckIgnoreListChanged();

    Q_INVOKABLE void addToSpellCheckIgnoreList(const QString &word);

    // This function adds a new scene to both structure and screenplay
    // and inserts it right after the current element in both.
    Q_INVOKABLE Scene *createNewScene();
    Q_SIGNAL void newSceneCreated(Scene *scene, int screenplayIndex);

    Q_PROPERTY(bool modified READ isModified NOTIFY modifiedChanged)
    bool isModified() const { return m_modified; }
    Q_SIGNAL void modifiedChanged();

    Q_PROPERTY(QString fileName READ fileName NOTIFY fileNameChanged)
    QString fileName() const { return m_fileName; }
    Q_SIGNAL void fileNameChanged();

    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)
    bool isLoading() const { return m_loading; }
    Q_SIGNAL void loadingChanged();

    Q_PROPERTY(bool isCreatedOnThisComputer READ isCreatedOnThisComputer NOTIFY createdOnThisComputerChanged)
    bool isCreatedOnThisComputer() const { return m_createdOnThisComputer; }
    Q_SIGNAL void createdOnThisComputerChanged();

    Q_INVOKABLE void reset();

    Q_INVOKABLE void open(const QString &fileName);
    Q_INVOKABLE void openAnonymously(const QString &fileName);
    Q_INVOKABLE void saveAs(const QString &fileName);
    Q_INVOKABLE void save();

    Q_SIGNAL void aboutToSave();
    Q_SIGNAL void justSaved();

    Q_PROPERTY(QStringList supportedImportFormats READ supportedImportFormats CONSTANT)
    QStringList supportedImportFormats() const;
    Q_INVOKABLE QString importFormatFileSuffix(const QString &format) const;

    Q_PROPERTY(QStringList supportedExportFormats READ supportedExportFormats CONSTANT)
    QStringList supportedExportFormats() const;
    Q_INVOKABLE QString exportFormatFileSuffix(const QString &format) const;

    Q_PROPERTY(QJsonArray supportedReports READ supportedReports CONSTANT)
    QJsonArray supportedReports() const;

    Q_INVOKABLE QString reportFileSuffix() const;

    Q_INVOKABLE bool importFile(const QString &fileName, const QString &format);
    Q_INVOKABLE bool exportFile(const QString &fileName, const QString &format);
    Q_INVOKABLE bool exportToImage(int fromSceneIdx, int fromParaIdx, int toSceneIdx, int toParaIdx, const QString &imageFileName);

    Q_INVOKABLE AbstractExporter *createExporter(const QString &format);
    Q_INVOKABLE AbstractReportGenerator *createReportGenerator(const QString &report);

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
    void setReadOnly(bool val);
    void setLoading(bool val);
    void prepareAutoSave();
    void updateDocumentWindowTitle();
    void setDocumentWindowTitle(const QString &val);
    void setStructure(Structure* val);
    void setScreenplay(Screenplay* val);
    void setFormatting(ScreenplayFormat* val);
    void setPrintFormat(ScreenplayFormat *val);
    void evaluateStructureElementSequence();
    void evaluateStructureElementSequenceLater();
    void markAsModified() { this->setModified(true); }
    void setModified(bool val);
    void setFileName(const QString &val);
    bool load(const QString &fileName);
    bool classicLoad(const QString &fileName);
    bool modernLoad(const QString &fileName, int *format=nullptr);
    void structureElementIndexChanged();
    void screenplayElementIndexChanged();
    void setCreatedOnThisComputer(bool val);

public:
    // QObjectSerializer::Interface implementation
    void prepareForSerialization();
    void prepareForDeserialization();
    bool canSerialize(const QMetaObject *, const QMetaProperty &) const;
    void serializeToJson(QJsonObject &) const;
    void deserializeFromJson(const QJsonObject &);

private:
    QString polishFileName(const QString &fileName) const;

private:
    bool m_busy = false;
    bool m_locked = false;
    bool m_loading = false;
    bool m_modified = false;
    bool m_autoSave = true;
    bool m_readOnly = false;
    bool m_autoSaveMode = false;
    QString m_fileName;
    QString m_busyMessage;
    bool m_inCreateNewScene = false;
    bool m_createdOnThisComputer = true;
    ExecLaterTimer m_autoSaveTimer;
    QString m_documentWindowTitle;
    ExecLaterTimer m_clearModifyTimer;
    int m_autoSaveDurationInSeconds = 60;
    DocumentFileSystem m_docFileSystem;
    QStringList m_spellCheckIgnoreList;
    QJsonArray m_structureElementSequence;
    QObjectProperty<Structure> m_structure;
    StructureElementConnectors m_connectors;
    QObjectProperty<Screenplay> m_screenplay;
    QObjectProperty<ScreenplayFormat> m_formatting;
    QObjectProperty<ScreenplayFormat> m_printFormat;
    ExecLaterTimer m_evaluateStructureElementSequenceTimer;
    bool m_syncingStructureScreenplayCurrentIndex = false;

    ErrorReport *m_errorReport = new ErrorReport(this);
    ProgressReport *m_progressReport = new ProgressReport(this);
};

#endif // SCRITEDOCUMENT_H
