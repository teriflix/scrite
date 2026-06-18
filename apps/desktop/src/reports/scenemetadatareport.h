/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef SCENEMETADATAREPORT_H
#define SCENEMETADATAREPORT_H

#include "abstractreportgenerator.h"

class ScreenplayElement;
class ScreenplayPaginator;

class SceneMetadataReport : public AbstractReportGenerator
{
    Q_OBJECT
    // clang-format off
    Q_CLASSINFO("Title", "Scene Metadata")
    Q_CLASSINFO("Description", "Generate a metadata report of scenes with location, time, page numbers, and characters.")
    Q_CLASSINFO("Icon", ":/icons/reports/scene_metadata_report.png")
    // clang-format on

public:
    Q_INVOKABLE explicit SceneMetadataReport(QObject *parent = nullptr);
    ~SceneMetadataReport();

    bool requiresConfiguration() const { return true; }
    bool isSinglePageReport() const { return false; }

    // clang-format off
    Q_CLASSINFO("showSynopsisColumn_FieldGroup", "Columns")
    Q_CLASSINFO("showSynopsisColumn_FieldLabel", "Include 'Synopsis' column in the report")
    Q_CLASSINFO("showSynopsisColumn_FieldEditor", "CheckBox")
    Q_PROPERTY(bool showSynopsisColumn
               READ showSynopsisColumn
               WRITE setShowSynopsisColumn
               NOTIFY showSynopsisColumnChanged)
    // clang-format on
    void setShowSynopsisColumn(bool val);
    bool showSynopsisColumn() const { return m_showSynopsisColumn; }
    Q_SIGNAL void showSynopsisColumnChanged();

    // clang-format off
    Q_CLASSINFO("showGroupsColumn_FieldGroup", "Columns")
    Q_CLASSINFO("showGroupsColumn_FieldLabel", "Include 'Formal Tags' column in the report")
    Q_CLASSINFO("showGroupsColumn_FieldEditor", "CheckBox")
    Q_PROPERTY(bool showGroupsColumn
               READ showGroupsColumn
               WRITE setShowGroupsColumn
               NOTIFY showGroupsColumnChanged)
    // clang-format on
    void setShowGroupsColumn(bool val);
    bool showGroupsColumn() const { return m_showGroupsColumn; }
    Q_SIGNAL void showGroupsColumnChanged();

    // clang-format off
    Q_CLASSINFO("showKeywordsColumn_FieldGroup", "Columns")
    Q_CLASSINFO("showKeywordsColumn_FieldLabel", "Include 'Keywords' column in the report")
    Q_CLASSINFO("showKeywordsColumn_FieldEditor", "CheckBox")
    Q_PROPERTY(bool showKeywordsColumn
               READ showKeywordsColumn
               WRITE setShowKeywordsColumn
               NOTIFY showKeywordsColumnChanged)
    // clang-format on
    void setShowKeywordsColumn(bool val);
    bool showKeywordsColumn() const { return m_showKeywordsColumn; }
    Q_SIGNAL void showKeywordsColumnChanged();

    // clang-format off
    Q_CLASSINFO("showStartPageColumn_FieldGroup", "Columns")
    Q_CLASSINFO("showStartPageColumn_FieldLabel", "Include 'Start Page' column in the report")
    Q_CLASSINFO("showStartPageColumn_FieldEditor", "CheckBox")
    Q_PROPERTY(bool showStartPageColumn
               READ showStartPageColumn
               WRITE setShowStartPageColumn
               NOTIFY showStartPageColumnChanged)
    // clang-format on
    void setShowStartPageColumn(bool val);
    bool showStartPageColumn() const { return m_showStartPageColumn; }
    Q_SIGNAL void showStartPageColumnChanged();

    // clang-format off
    Q_CLASSINFO("showPageCountColumn_FieldGroup", "Columns")
    Q_CLASSINFO("showPageCountColumn_FieldLabel", "Include 'Page Length' column in the report to show 1/8th lengths")
    Q_CLASSINFO("showPageCountColumn_FieldEditor", "CheckBox")
    Q_PROPERTY(bool showPageCountColumn
               READ showPageCountColumn
               WRITE setShowPageCountColumn
               NOTIFY showPageCountColumnChanged)
    // clang-format on
    void setShowPageCountColumn(bool val);
    bool showPageCountColumn() const { return m_showPageCountColumn; }
    Q_SIGNAL void showPageCountColumnChanged();

    // clang-format off
    Q_CLASSINFO("showSceneTimeColumn_FieldGroup", "Columns")
    Q_CLASSINFO("showSceneTimeColumn_FieldLabel", "Include 'Scene Time' column in the report")
    Q_CLASSINFO("showSceneTimeColumn_FieldEditor", "CheckBox")
    Q_PROPERTY(bool showSceneTimeColumn
               READ showSceneTimeColumn
               WRITE setShowSceneTimeColumn
               NOTIFY showSceneTimeColumnChanged)
    // clang-format on
    void setShowSceneTimeColumn(bool val);
    bool showSceneTimeColumn() const { return m_showSceneTimeColumn; }
    Q_SIGNAL void showSceneTimeColumnChanged();

    // clang-format off
    Q_CLASSINFO("showCharactersColumn_FieldGroup", "Columns")
    Q_CLASSINFO("showCharactersColumn_FieldLabel", "Include 'Characters' column in the report")
    Q_CLASSINFO("showCharactersColumn_FieldEditor", "CheckBox")
    Q_PROPERTY(bool showCharactersColumn
               READ showCharactersColumn
               WRITE setShowCharactersColumn
               NOTIFY showCharactersColumnChanged)
    // clang-format on
    void setShowCharactersColumn(bool val);
    bool showCharactersColumn() const { return m_showCharactersColumn; }
    Q_SIGNAL void showCharactersColumnChanged();

protected:
    // AbstractDeviceIO interface
    QString fileNameExtension() const;

    // AbstractReportGenerator interface
    QString personalizedFileName(const QString &fileName) const override;
    bool usePdfWriter() const { return false; }
    bool doGenerate(QTextDocument *document);
    void configureWriter(QPdfWriter *pdfWriter, const QTextDocument *document) const;
    void configureWriter(QPrinter *printer, const QTextDocument *document) const;
    virtual bool canDirectExportToOdf() const;
    virtual bool directExportToOdf(QIODevice *);

private:
    void configureWriterImpl(QPagedPaintDevice *ppd, const QTextDocument *document) const;
    QList<ScreenplayElement *> getScreenplayElements();
    bool passesFilter(const ScreenplayElement *element) const;

private:
    QList<int> m_episodeNumbers;
    QList<int> m_sceneNumbers;
    QStringList m_tags;
    QString m_keywords;
    bool m_showSynopsisColumn = true;
    bool m_showGroupsColumn = true;
    bool m_showKeywordsColumn = true;
    bool m_showStartPageColumn = true;
    bool m_showPageCountColumn = true;
    bool m_showSceneTimeColumn = true;
    bool m_showCharactersColumn = true;
};

#endif // SCENEMETADATAREPORT_H
