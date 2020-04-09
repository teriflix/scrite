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

#ifndef SCRITEDOCUMENT_H
#define SCRITEDOCUMENT_H

#include <QObject>
#include <QJsonArray>

#include "screenplay.h"
#include "structure.h"
#include "formatting.h"
#include "errorreport.h"
#include "progressreport.h"

class AbstractReportGenerator;

class ScriteDocument : public QObject
{
    Q_OBJECT

public:
    ScriteDocument(QObject *parent = nullptr);
    ~ScriteDocument();

    Q_PROPERTY(int autoSaveDurationInSeconds READ autoSaveDurationInSeconds WRITE setAutoSaveDurationInSeconds NOTIFY autoSaveDurationInSecondsChanged STORED false)
    void setAutoSaveDurationInSeconds(int val);
    int autoSaveDurationInSeconds() const { return m_autoSaveDurationInSeconds; }
    Q_SIGNAL void autoSaveDurationInSecondsChanged();

    Q_PROPERTY(bool autoSave READ isAutoSave WRITE setAutoSave NOTIFY autoSaveChanged STORED false)
    void setAutoSave(bool val);
    bool isAutoSave() const { return m_autoSave; }
    Q_SIGNAL void autoSaveChanged();

    Q_PROPERTY(QString documentWindowTitle READ documentWindowTitle NOTIFY documentWindowTitleChanged)
    QString documentWindowTitle() const { return m_documentWindowTitle; }
    Q_SIGNAL void documentWindowTitleChanged(const QString &val);

    Q_PROPERTY(Structure* structure READ structure NOTIFY structureChanged)
    Structure* structure() const { return m_structure; }
    Q_SIGNAL void structureChanged();

    Q_PROPERTY(Screenplay* screenplay READ screenplay NOTIFY screenplayChanged)
    Screenplay* screenplay() const { return m_screenplay; }
    Q_SIGNAL void screenplayChanged();

    Q_PROPERTY(ScreenplayFormat* formatting READ formatting NOTIFY formattingChanged)
    ScreenplayFormat* formatting() const { return m_formatting; }
    Q_SIGNAL void formattingChanged();

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
    Q_INVOKABLE bool generateReport(const QString &fileName, const QString &report);
    Q_SIGNAL void requestReportGeneratorConfiguration(AbstractReportGenerator *reportGenerator);

    Q_PROPERTY(QJsonArray structureElementSequence READ structureElementSequence NOTIFY structureElementSequenceChanged)
    QJsonArray structureElementSequence() const { return m_structureElementSequence; }
    Q_SIGNAL void structureElementSequenceChanged();

protected:
    void timerEvent(QTimerEvent *event);

private:
    void prepareAutoSave();
    void updateDocumentWindowTitle();
    void setDocumentWindowTitle(const QString &val);
    void setStructure(Structure* val);
    void setScreenplay(Screenplay* val);
    void setFormatting(ScreenplayFormat* val);
    void evaluateStructureElementSequence();
    void evaluateStructureElementSequenceLater();
    void markAsModified() { this->setModified(true); }
    void setModified(bool val);
    void setFileName(const QString &val);
    bool load(const QString &fileName);
    void structureElementIndexChanged();
    void screenplayElementIndexChanged();

private:
    bool m_autoSave;
    int m_autoSaveDurationInSeconds;
    QBasicTimer m_autoSaveTimer;
    Screenplay* m_screenplay;
    Structure* m_structure;
    ScreenplayFormat* m_formatting;
    bool m_modified;
    QString m_fileName;
    QString m_documentWindowTitle;
    QJsonArray m_structureElementSequence;
    QBasicTimer m_evaluateStructureElementSequenceTimer;

    ErrorReport *m_errorReport;
    ProgressReport *m_progressReport;
};

#endif // SCRITEDOCUMENT_H
