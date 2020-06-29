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
#include "qobjectserializer.h"
#include "documentfilesystem.h"

class AbstractExporter;
class AbstractReportGenerator;

class ScriteDocument : public QObject, public QObjectSerializer::Interface
{
    Q_OBJECT
    Q_INTERFACES(QObjectSerializer::Interface)

    ScriteDocument(QObject *parent = nullptr);

public:
    static ScriteDocument *instance();
    ~ScriteDocument();

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

    Q_INVOKABLE void reset();

    Q_INVOKABLE void open(const QString &fileName);
    Q_INVOKABLE void saveAs(const QString &fileName);
    Q_INVOKABLE void save();

    Q_PROPERTY(QStringList supportedImportFormats READ supportedImportFormats CONSTANT)
    QStringList supportedImportFormats() const;
    Q_INVOKABLE QString importFormatFileSuffix(const QString &format) const;

    Q_PROPERTY(QStringList supportedExportFormats READ supportedExportFormats CONSTANT)
    QStringList supportedExportFormats() const;
    Q_INVOKABLE QString exportFormatFileSuffix(const QString &format) const;

    Q_PROPERTY(QStringList supportedReports READ supportedReports CONSTANT)
    QStringList supportedReports() const;

    Q_INVOKABLE QString reportFileSuffix() const;

    Q_INVOKABLE bool importFile(const QString &fileName, const QString &format);
    Q_INVOKABLE bool exportFile(const QString &fileName, const QString &format);

    Q_INVOKABLE AbstractExporter *createExporter(const QString &format);
    Q_INVOKABLE AbstractReportGenerator *createReportGenerator(const QString &report);

    Q_PROPERTY(QJsonArray structureElementSequence READ structureElementSequence NOTIFY structureElementSequenceChanged)
    QJsonArray structureElementSequence() const { return m_structureElementSequence; }
    Q_SIGNAL void structureElementSequenceChanged();

    void clearModified();

protected:
    void timerEvent(QTimerEvent *event);

private:
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
    bool modernLoad(const QString &fileName);
    void structureElementIndexChanged();
    void screenplayElementIndexChanged();

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
    bool m_loading = false;
    bool m_modified = false;
    bool m_autoSave = true;
    bool m_autoSaveMode = false;
    QString m_fileName;
    QString m_busyMessage;
    bool m_inCreateNewScene = false;
    SimpleTimer m_autoSaveTimer;
    QString m_documentWindowTitle;
    SimpleTimer m_clearModifyTimer;
    Structure* m_structure = nullptr;
    Screenplay* m_screenplay = nullptr;
    ScreenplayFormat* m_formatting = nullptr;
    ScreenplayFormat* m_printFormat = nullptr;
    int m_autoSaveDurationInSeconds = 60;
    DocumentFileSystem m_docFileSystem;
    QStringList m_spellCheckIgnoreList;
    QJsonArray m_structureElementSequence;
    SimpleTimer m_evaluateStructureElementSequenceTimer;
    bool m_syncingStructureScreenplayCurrentIndex = false;

    ErrorReport *m_errorReport = new ErrorReport(this);
    ProgressReport *m_progressReport = new ProgressReport(this);
};

#endif // SCRITEDOCUMENT_H
