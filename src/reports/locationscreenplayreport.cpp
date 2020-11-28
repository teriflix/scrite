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

#include "locationscreenplayreport.h"
#include "scene.h"

LocationScreenplayReport::LocationScreenplayReport(QObject *parent)
    : AbstractScreenplaySubsetReport(parent)
{

}

LocationScreenplayReport::~LocationScreenplayReport()
{

}

void LocationScreenplayReport::setLocations(const QStringList &val)
{
    if(m_locations == val)
        return;

    m_locations = val;
    emit locationsChanged();
}

void LocationScreenplayReport::setGenerateSummary(bool val)
{
    if(m_generateSummary == val)
        return;

    m_generateSummary = val;
    emit generateSummaryChanged();
}

bool LocationScreenplayReport::includeScreenplayElement(const ScreenplayElement *element) const
{
    const Scene *scene = element->scene();
    if(scene == nullptr)
        return false;

    if(m_locations.isEmpty())
        return true;

    if(!scene->heading()->isEnabled())
        return false;

    const bool ret = m_locations.contains(scene->heading()->location(), Qt::CaseInsensitive);
    if(ret)
    {
        const QString loc = scene->heading()->location().toUpper();
        m_locationSceneNumberList[loc] << element;
    }

    return ret;
}

QString LocationScreenplayReport::screenplaySubtitle() const
{
    if(m_locations.isEmpty())
        return QStringLiteral("Location Screenplay of: ALL LOCATIONS");

    const QString subtitle = QStringLiteral("Location Screenplay of: ") + m_locations.join(", ");
    if(subtitle.length() > 60)
        return  m_locations.first() + QStringLiteral(" and ") +
                QString::number(m_locations.size()-1) + QStringLiteral(" other locations(s).");

    return subtitle;
}

void LocationScreenplayReport::configureScreenplayTextDocument(ScreenplayTextDocument &stDoc)
{
    Q_UNUSED(stDoc);
}

void LocationScreenplayReport::inject(QTextCursor &cursor, AbstractScreenplayTextDocumentInjectionInterface::InjectLocation location)
{
    AbstractScreenplaySubsetReport::inject(cursor, location);

    if(!m_generateSummary)
        return;

    if(location == AfterTitlePage)
    {
        m_summaryLocation = cursor.position();
        return;
    }

    if(location == AfterLastScene)
    {
        if(m_locationSceneNumberList.isEmpty())
            return;

        cursor.setPosition(m_summaryLocation);

        const QFont defaultFont = this->document()->printFormat()->defaultFont();

        QTextBlockFormat defaultBlockFormat;

        QTextCharFormat defaultCharFormat;
        defaultCharFormat.setFontFamily(defaultFont.family());
        defaultCharFormat.setFontPointSize(12);

        QTextBlockFormat blockFormat = defaultBlockFormat;
        blockFormat.setAlignment(Qt::AlignLeft);
        blockFormat.setTopMargin(20);

        QTextCharFormat charFormat = defaultCharFormat;
        charFormat.setFontPointSize(20);
        charFormat.setFontCapitalization(QFont::AllUppercase);
        charFormat.setFontWeight(QFont::Bold);
        charFormat.setFontItalic(true);

        cursor.insertBlock(blockFormat, charFormat);
        cursor.insertText("SUMMARY:");

        QMap< QString, QList<const ScreenplayElement *> >::const_iterator it = m_locationSceneNumberList.begin();
        QMap< QString, QList<const ScreenplayElement *> >::const_iterator end = m_locationSceneNumberList.end();
        while(it != end)
        {
            blockFormat = defaultBlockFormat;
            blockFormat.setIndent(1);

            charFormat = defaultCharFormat;
            charFormat.setFontWeight(QFont::Bold);

            cursor.insertBlock(blockFormat, charFormat);
            cursor.insertText(it.key().toUpper() + QStringLiteral(": "));

            blockFormat.setIndent(2);
            charFormat.setFontWeight(QFont::Normal);

            cursor.insertBlock(blockFormat, charFormat);
            cursor.insertText( QStringLiteral("This location is referenced in ") + QString::number(it.value().size()) + QStringLiteral(" scene(s).") );

            blockFormat.setIndent(3);

            Q_FOREACH(const ScreenplayElement *element, it.value())
            {
                cursor.insertBlock(blockFormat, charFormat);
                cursor.insertText( QStringLiteral("[") + element->resolvedSceneNumber() + QStringLiteral("] - ") + element->scene()->heading()->text() );
            }

            cursor.insertBlock(blockFormat, charFormat);
            cursor.insertText(QString());

            ++it;
        }

        blockFormat = defaultBlockFormat;
        blockFormat.setPageBreakPolicy(QTextBlockFormat::PageBreak_AlwaysAfter);
        charFormat = defaultCharFormat;
        cursor.insertBlock(blockFormat, charFormat);
        cursor.insertText(QStringLiteral("-- end of notes --"));

        m_summaryLocation = -1;
        m_locationSceneNumberList.clear();
    }
}
