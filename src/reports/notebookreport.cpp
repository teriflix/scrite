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

#include "notebookreport.h"

#include "notes.h"
#include "scene.h"
#include "structure.h"
#include "screenplay.h"
#include "application.h"
#include "screenplaytextdocument.h"

#include <QJsonDocument>

NotebookReport::NotebookReport(QObject *parent) : AbstractReportGenerator(parent)
{
    QMetaObject::invokeMethod(this, &NotebookReport::evaluateTitleAndSubtitle,
                              Qt::QueuedConnection);
}

NotebookReport::~NotebookReport() { }

void NotebookReport::setSection(QObject *val)
{
    if (m_section == val)
        return;

    auto sectionFound = [=]() {
        return m_noteSection || m_notesSection || m_sceneSection || m_storySection
                || m_screenplaySection;
    };

    m_noteSection = nullptr;
    m_notesSection = nullptr;
    m_sceneSection = nullptr;
    m_characterSection = nullptr;
    m_storySection = nullptr;
    m_screenplaySection = nullptr;

    m_noteSection = qobject_cast<Note *>(val);
    m_notesSection = !sectionFound() ? qobject_cast<Notes *>(val) : nullptr;
    m_sceneSection = !sectionFound() ? qobject_cast<Scene *>(val) : nullptr;
    m_characterSection = !sectionFound() ? qobject_cast<Character *>(val) : nullptr;
    m_storySection = !sectionFound() ? qobject_cast<Structure *>(val) : nullptr;
    m_screenplaySection = !sectionFound() ? qobject_cast<Screenplay *>(val) : nullptr;

    QObject *val2 = sectionFound() ? val : nullptr;
    if (m_section == val2)
        return;

    m_section = val2;

    this->evaluateTitleAndSubtitle();

    emit sectionChanged();
}

void NotebookReport::setOptions(const QJsonValue &val)
{
    if (m_options == val)
        return;

    m_options = val;

    this->evaluateTitleAndSubtitle();

    emit optionsChanged();
}

QString NotebookReport::polishFileName(const QString &fileName) const
{
    if (m_section == nullptr)
        return fileName;

    const QFileInfo fi(fileName);
    const QDir folder = fi.absoluteDir();
    const QString suffix =
            this->format() == AdobePDF ? QLatin1String(".pdf") : QLatin1String(".odt");
    return folder.absoluteFilePath(m_title + QLatin1String(" - ") + m_subtitle + suffix);
}

bool NotebookReport::doGenerate(QTextDocument *doc)
{
    ScriteDocument *scriteDocument = this->document();
    Screenplay *screenplay = scriteDocument->screenplay();
    Structure *structure = scriteDocument->structure();
    const QJsonObject options = m_options.toObject();
    const QString intent = options.value("intent").toString();
    const bool charactersIntent = intent == "characters";

    doc->setDefaultFont(qApp->font());
    doc->setUseDesignMetrics(true);
    doc->clear();

    QTextCursor cursor(doc);

    QTextBlockFormat titleBlockFormat;
    titleBlockFormat.setHeadingLevel(1);
    titleBlockFormat.setAlignment(Qt::AlignHCenter);

    QTextCharFormat titleCharFormat;
    titleCharFormat.setFontWeight(QFont::Bold);
    titleCharFormat.setFontPointSize(20);
    titleBlockFormat.setTopMargin(titleCharFormat.fontPointSize() / 2);

    cursor.setBlockFormat(titleBlockFormat);
    cursor.setCharFormat(titleCharFormat);
    cursor.insertText(m_title);

    QTextBlockFormat subtitleBlockFormat;
    subtitleBlockFormat.setHeadingLevel(2);
    subtitleBlockFormat.setAlignment(Qt::AlignHCenter);

    QTextCharFormat subtitleCharFormat;
    subtitleCharFormat.setFontWeight(QFont::Bold);
    subtitleCharFormat.setFontPointSize(18);
    subtitleBlockFormat.setTopMargin(subtitleCharFormat.fontPointSize() / 2);

    cursor.insertBlock(subtitleBlockFormat, subtitleCharFormat);
    cursor.insertText(m_subtitle);

    QTextBlockFormat normalBlockFormat;
    normalBlockFormat.setHeadingLevel(0);
    normalBlockFormat.setAlignment(Qt::AlignLeft);

    QTextCharFormat normalCharFormat;
    normalCharFormat.setFontWeight(QFont::Normal);
    normalCharFormat.setFontPointSize(cursor.document()->defaultFont().pointSizeF());
    normalBlockFormat.setTopMargin(normalCharFormat.fontPointSize() / 2);

    cursor.insertBlock(normalBlockFormat, normalCharFormat);

    if (m_noteSection)
        m_noteSection->write(cursor);
    else if (m_notesSection)
        m_notesSection->write(cursor);
    else if (m_sceneSection)
        m_sceneSection->write(cursor);
    else if (m_characterSection)
        m_characterSection->write(cursor);
    else if (m_screenplaySection)
        m_screenplaySection->write(cursor);
    else if (m_storySection) {
        Structure::WriteOptions options;
        options.charactersOnly = charactersIntent;
        m_storySection->write(cursor, options);
    } else {
        auto addSection = [&cursor](const QString &title, int headingLevel = 2) {
            QTextBlockFormat sectionBlockFormat;
            sectionBlockFormat.setHeadingLevel(headingLevel);
            sectionBlockFormat.setTopMargin(10);
            sectionBlockFormat.setAlignment(Qt::AlignLeft);

            QTextCharFormat sectionCharFormat;
            sectionCharFormat.setFontWeight(QFont::Bold);
            sectionCharFormat.setFontPointSize(
                    ScreenplayTextDocument::headingFontPointSize(headingLevel - 1));
            sectionBlockFormat.setTopMargin(sectionCharFormat.fontPointSize() / 2);

            cursor.insertBlock(sectionBlockFormat, sectionCharFormat);
            cursor.insertText(title);
        };

        addSection(QLatin1String("Story Notes"));
        structure->write(cursor);

        addSection(QLatin1String("Scene Notes"));
        screenplay->write(cursor);

        addSection(QLatin1String("Character Notes"));
        const QList<Character *> characters = structure->charactersModel()->list();
        for (Character *character : characters)
            character->write(cursor);
    }

    return true;
}

void NotebookReport::evaluateTitleAndSubtitle()
{
    ScriteDocument *scriteDocument = this->document();
    Screenplay *screenplay = scriteDocument->screenplay();

    if (screenplay->title().isEmpty()) {
        if (scriteDocument->fileName().isEmpty())
            m_title = QLatin1String("Untitled Screenplay");
        else
            m_title = QFileInfo(scriteDocument->fileName()).baseName();
    } else
        m_title = screenplay->title();

    if (m_noteSection)
        m_subtitle = m_noteSection->title();
    else if (m_notesSection)
        m_subtitle = m_notesSection->title();
    else if (m_sceneSection)
        m_subtitle = m_sceneSection->title();
    else if (m_characterSection)
        m_subtitle = QLatin1String("Character");
    else if (m_storySection) {
        if (m_options.isUndefined() || m_options.isNull())
            m_subtitle = QLatin1String("Story");
        else
            m_subtitle = QLatin1String("All Character");
    } else if (m_screenplaySection)
        m_subtitle = QLatin1String("Scene");

    m_subtitle += QLatin1String(" Notes");
}

void NotebookReport::polishFormInfo(QJsonObject &formInfo) const
{
    if (m_section != nullptr) {
        formInfo.insert(QLatin1String("description"),
                        QLatin1String("Exports '") + m_subtitle
                                + QLatin1String("' into PDF or ODT."));
    }
}
