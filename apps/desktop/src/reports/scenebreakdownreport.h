/****************************************************************************
**
** Copyright (C) 2024 Prashanth N Udupa
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

#ifndef SCENEBREAKDOWNREPORT_H
#define SCENEBREAKDOWNREPORT_H

#include "abstractreportgenerator.h"

class ScreenplayElement;
class ScreenplayPaginator;

class SceneBreakdownReport : public AbstractReportGenerator
{
    Q_OBJECT
    // clang-format off
    Q_CLASSINFO("Title", "Scene Breakdown")
    Q_CLASSINFO("Description", "Generate a breakdown report of scenes with location, time, page numbers, and characters.")
    Q_CLASSINFO("Icon", ":/icons/reports/scene_breakdown_report.png")
    // clang-format on

public:
    Q_INVOKABLE explicit SceneBreakdownReport(QObject *parent = nullptr);
    ~SceneBreakdownReport();

    bool requiresConfiguration() const { return true; }
    bool isSinglePageReport() const { return false; }

    // clang-format off
    Q_CLASSINFO("episodeNumbers_FieldGroup", "Episodes")
    Q_CLASSINFO("episodeNumbers_FieldLabel", "Episodes to include in the report")
    Q_CLASSINFO("episodeNumbers_FieldEditor", "MultipleEpisodeSelector")
    Q_CLASSINFO("episodeNumbers_FieldNote", "If no episodes are selected, then the report is generated for all episodes in the screenplay.")
    Q_CLASSINFO("episodeNumbers_IsPersistent", "false")
    Q_PROPERTY(QList<int> episodeNumbers
               READ episodeNumbers
               WRITE setEpisodeNumbers
               NOTIFY episodeNumbersChanged)
    // clang-format on
    void setEpisodeNumbers(const QList<int> &val);
    QList<int> episodeNumbers() const { return m_episodeNumbers; }
    Q_SIGNAL void episodeNumbersChanged();

    // clang-format off
    Q_CLASSINFO("sceneNumbers_FieldGroup", "Scenes")
    Q_CLASSINFO("sceneNumbers_FieldLabel", "Scenes to include in the report")
    Q_CLASSINFO("sceneNumbers_FieldEditor", "MultipleSceneSelector")
    Q_CLASSINFO("sceneNumbers_FieldNote", "If no scenes are selected, then the report is generated for all scenes in the screenplay.")
    Q_CLASSINFO("sceneNumbers_IsPersistent", "false")
    Q_PROPERTY(QList<int> sceneNumbers
               READ sceneNumbers
               WRITE setSceneNumbers
               NOTIFY sceneNumbersChanged)
    // clang-format on
    void setSceneNumbers(const QList<int> &val);
    QList<int> sceneNumbers() const { return m_sceneNumbers; }
    Q_SIGNAL void sceneNumbersChanged();

    // clang-format off
    Q_CLASSINFO("tags_FieldGroup", "Tags")
    Q_CLASSINFO("tags_FieldLabel", "Groups/Tags to include in the report")
    Q_CLASSINFO("tags_FieldEditor", "MultipleTagGroupSelector")
    Q_CLASSINFO("tags_FieldNote", "If no tags are selected, then the report is generated for all tags in the screenplay.")
    Q_CLASSINFO("tags_IsPersistent", "false")
    Q_PROPERTY(QStringList tags
               READ tags
               WRITE setTags
               NOTIFY tagsChanged)
    // clang-format on
    void setTags(const QStringList &val);
    QStringList tags() const { return m_tags; }
    Q_SIGNAL void tagsChanged();

    // clang-format off
    Q_CLASSINFO("keywords_FieldGroup", "Keywords")
    Q_CLASSINFO("keywords_FieldLabel", "Keywords to filter scenes")
    Q_CLASSINFO("keywords_FieldEditor", "MultipleKeywordsSelector")
    Q_CLASSINFO("keywords_FieldNote", "Comma-separated keywords to search in scene text.")
    Q_CLASSINFO("keywords_IsPersistent", "false")
    Q_PROPERTY(QString keywords
               READ keywords
               WRITE setKeywords
               NOTIFY keywordsChanged)
    // clang-format on
    void setKeywords(const QString &val);
    QString keywords() const { return m_keywords; }
    Q_SIGNAL void keywordsChanged();

protected:
    // AbstractDeviceIO interface
    QString fileNameExtension() const;

    // AbstractReportGenerator interface
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
};

#endif // SCENEBREAKDOWNREPORT_H
