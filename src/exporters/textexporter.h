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

#ifndef TEXTEXPORTER_H
#define TEXTEXPORTER_H

#include "abstractexporter.h"

class TextExporter : public AbstractExporter
{
    Q_OBJECT
    Q_CLASSINFO("Format", "Screenplay/Text File")
    Q_CLASSINFO("NameFilters", "Text File (*.txt)")
    Q_CLASSINFO("Description", "Exports the current screenplay as a text file.")
    Q_CLASSINFO("Icon", ":/icons/exporter/text.png")

public:
    Q_INVOKABLE explicit TextExporter(QObject *parent = nullptr);
    ~TextExporter();

    bool canCopyToClipboard() const { return true; }
    bool canBundleFonts() const { return false; }
    bool requiresConfiguration() const { return true; }

    Q_CLASSINFO("maxLettersPerLine_FieldLabel", "Number of characters per line:")
    Q_CLASSINFO("maxLettersPerLine_FieldEditor", "IntegerSpinBox")
    Q_CLASSINFO("maxLettersPerLine_FieldMinValue", "30")
    Q_CLASSINFO("maxLettersPerLine_FieldMaxValue", "150")
    Q_CLASSINFO("maxLettersPerLine_FieldDefaultValue", "60")
    Q_PROPERTY(int maxLettersPerLine READ maxLettersPerLine WRITE setMaxLettersPerLine NOTIFY maxLettersPerLineChanged)
    void setMaxLettersPerLine(int val);
    int maxLettersPerLine() const { return m_maxLettersPerLine; }
    Q_SIGNAL void maxLettersPerLineChanged();

    Q_CLASSINFO("includeSceneNumbers_FieldLabel", "Include scene numbers.")
    Q_CLASSINFO("includeSceneNumbers_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeSceneNumbers READ isIncludeSceneNumbers WRITE setIncludeSceneNumbers NOTIFY includeSceneNumbersChanged)
    void setIncludeSceneNumbers(bool val);
    bool isIncludeSceneNumbers() const { return m_includeSceneNumbers; }
    Q_SIGNAL void includeSceneNumbersChanged();

    Q_CLASSINFO("includeEpisodeAndActBreaks_FieldLabel", "Include episode and act breaks.")
    Q_CLASSINFO("includeEpisodeAndActBreaks_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeEpisodeAndActBreaks READ isIncludeEpisodeAndActBreaks WRITE setIncludeEpisodeAndActBreaks NOTIFY includeEpisodeAndActBreaksChanged)
    void setIncludeEpisodeAndActBreaks(bool val);
    bool isIncludeEpisodeAndActBreaks() const { return m_includeEpisodeAndActBreaks; }
    Q_SIGNAL void includeEpisodeAndActBreaksChanged();

    Q_CLASSINFO("includeSceneSynopsis_FieldLabel", "Include scene synopsis.")
    Q_CLASSINFO("includeSceneSynopsis_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeSceneSynopsis READ isIncludeSceneSynopsis WRITE setIncludeSceneSynopsis NOTIFY includeSceneSynopsisChanged)
    void setIncludeSceneSynopsis(bool val);
    bool isIncludeSceneSynopsis() const { return m_includeSceneSynopsis; }
    Q_SIGNAL void includeSceneSynopsisChanged();

protected:
    bool doExport(QIODevice *device); // AbstractExporter interface
    QString fileNameExtension() const { return QStringLiteral("txt"); }

private:
    QString toString() const;

private:
    int m_maxLettersPerLine = 60;
    bool m_includeSceneNumbers = false;
    bool m_includeEpisodeAndActBreaks = false;
    bool m_includeSceneSynopsis = false;
};

#endif // TEXTEXPORTER_H
