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

#ifndef SCENECHARACTERMATRIXREPORT_H
#define SCENECHARACTERMATRIXREPORT_H

#include "abstractreportgenerator.h"

class ScreenplayElement;

class SceneCharacterMatrixReport : public AbstractReportGenerator
{
    Q_OBJECT
    Q_CLASSINFO("Title", "Scene Character Matrix")
    Q_CLASSINFO("Description", "Generate a table of scene names and characters.")

public:
    Q_INVOKABLE explicit SceneCharacterMatrixReport(QObject *parent = nullptr);
    ~SceneCharacterMatrixReport();

    bool requiresConfiguration() const { return true; }
    bool isSinglePageReport() const { return true; }

    enum Type { SceneVsCharacter, CharacterVsScene };
    Q_ENUM(Type)
    Q_CLASSINFO("Type_CharacterVsScene", "Character vs Scene")
    Q_CLASSINFO("Type_SceneVsCharacter", "Scene Vs Character")

    Q_CLASSINFO("type_FieldGroup", "Characters")
    Q_CLASSINFO("type_FieldLabel", "Select type of matrix to generate")
    Q_CLASSINFO("type_FieldEditor", "EnumSelector")
    Q_CLASSINFO("type_FieldEnum", "Type")
    Q_PROPERTY(int type READ type WRITE setType NOTIFY typeChanged)
    void setType(int val);
    int type() const { return m_type; }
    Q_SIGNAL void typeChanged();

    Q_CLASSINFO("marker_FieldGroup", "Characters")
    Q_CLASSINFO("marker_FieldLabel", "Marker (CSV/ODF Only)")
    Q_CLASSINFO("marker_FieldNote", "Marker text to use while generating CSV.")
    Q_CLASSINFO("marker_FieldEditor", "TextBox")
    Q_PROPERTY(QString marker READ marker WRITE setMarker NOTIFY markerChanged)
    void setMarker(const QString &val);
    QString marker() const { return m_marker; }
    Q_SIGNAL void markerChanged();

    Q_CLASSINFO("characterNames_FieldGroup", "Characters")
    Q_CLASSINFO("characterNames_FieldLabel", "Characters to include in the report")
    Q_CLASSINFO("characterNames_FieldNote", "If no characters are selected, then the report is generted for all characters in the screenplay.")
    Q_CLASSINFO("characterNames_FieldEditor", "MultipleCharacterNameSelector")
    Q_PROPERTY(QStringList characterNames READ characterNames WRITE setCharacterNames NOTIFY characterNamesChanged)
    void setCharacterNames(const QStringList &val);
    QStringList characterNames() const { return m_characterNames; }
    Q_SIGNAL void characterNamesChanged();

    Q_CLASSINFO("episodeNumbers_FieldGroup", "Episodes")
    Q_CLASSINFO("episodeNumbers_FieldLabel", "Episodes to include in the report")
    Q_CLASSINFO("episodeNumbers_FieldEditor", "MultipleEpisodeSelector")
    Q_CLASSINFO("episodeNumbers_FieldNote", "If no episodes are selected, then the report is generted for all episodes in the screenplay.")
    Q_PROPERTY(QList<int> episodeNumbers READ episodeNumbers WRITE setEpisodeNumbers NOTIFY episodeNumbersChanged)
    void setEpisodeNumbers(const QList<int> &val);
    QList<int> episodeNumbers() const { return m_episodeNumbers; }
    Q_SIGNAL void episodeNumbersChanged();

    Q_CLASSINFO("tags_FieldGroup", "Tags")
    Q_CLASSINFO("tags_FieldLabel", "Groups/Tags to include in the report")
    Q_CLASSINFO("tags_FieldEditor", "MultipleTagGroupSelector")
    Q_CLASSINFO("tags_FieldNote", "If no tags are selected, then the report is generated for all tags in the screenplay.")
    Q_PROPERTY(QStringList tags READ tags WRITE setTags NOTIFY tagsChanged)
    void setTags(const QStringList &val);
    QStringList tags() const { return m_tags; }
    Q_SIGNAL void tagsChanged();

protected:
    // AbstractDeviceIO interface
    QString polishFileName(const QString &fileName) const;

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
    void finalizeCharacterNames();

private:
    QStringList m_tags;
    QStringList m_characterNames;
    int m_type = SceneVsCharacter;
    QList<int> m_episodeNumbers;
    QString m_marker = QStringLiteral("YES");
};

#endif // SCENECHARACTERMATRIXREPORT_H
