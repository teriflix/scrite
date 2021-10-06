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

#include "statisticsreport.h"
#include "scritedocument.h"
#include "screenplay.h"
#include "scene.h"

#include <QTextTable>
#include <QTextCursor>
#include <QTextDocument>

StatisticsReport::StatisticsReport(QObject *parent)
    : AbstractReportGenerator(parent)
{

}

StatisticsReport::~StatisticsReport()
{

}

bool StatisticsReport::doGenerate(QTextDocument *textDocument)
{
    Screenplay *screenplay = ScriteDocument::instance()->screenplay();
    Structure *structure = ScriteDocument::instance()->structure();

    QTextDocument &document = *textDocument;
    QTextCursor cursor(&document);

    const QFont defaultFont = this->document()->printFormat()->defaultFont();

    QTextBlockFormat defaultBlockFormat;

    QTextCharFormat defaultCharFormat;
    defaultCharFormat.setFontFamily(defaultFont.family());
    defaultCharFormat.setFontPointSize(12);

    // Report Title
    {
        QTextBlockFormat blockFormat = defaultBlockFormat;
        blockFormat.setAlignment(Qt::AlignHCenter);
        cursor.setBlockFormat(blockFormat);

        QTextCharFormat charFormat = defaultCharFormat;
        charFormat.setFontPointSize(24);
        charFormat.setFontCapitalization(QFont::AllUppercase);
        charFormat.setFontWeight(QFont::Bold);
        charFormat.setFontUnderline(true);
        cursor.setCharFormat(charFormat);

        QString title = screenplay->title();
        if(title.isEmpty())
            title = QStringLiteral("Untitled Screenplay");
        cursor.insertText(title);
        cursor.insertBlock();
        cursor.insertText(QStringLiteral("Key Statistics"));

        blockFormat = defaultBlockFormat;
        blockFormat.setAlignment(Qt::AlignHCenter);
        blockFormat.setBottomMargin(20);
        charFormat = defaultCharFormat;
        cursor.insertBlock(blockFormat, charFormat);
        cursor.insertHtml("This report was generated using <strong>Scrite</strong><br/>(<a href=\"https://www.scrite.io\">https://www.scrite.io</a>)");
    }

    QTextTableFormat tableFormat;
    tableFormat.setCellSpacing(0);
    tableFormat.setCellPadding(5);
    tableFormat.setBorder(3);
    tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
    tableFormat.setHeaderRowCount(1);

    cursor.insertBlock();
    cursor.insertBlock();
    QTextTable *table = cursor.insertTable(3, 2, tableFormat);

    // Number of scenes
    cursor = table->cellAt(0, 0).firstCursorPosition();
    cursor.insertText( QStringLiteral("Number Of Scenes: ") );

    int nrScenes = 0;
    for(int i=0; i<screenplay->elementCount(); i++)
    {
        ScreenplayElement *element = screenplay->elementAt(i);
        if(element->elementType() == ScreenplayElement::SceneElementType)
        {
            Scene *scene = element->scene();
            if(scene->heading() && scene->heading()->isEnabled())
                ++nrScenes;
        }
    }
    cursor = table->cellAt(0, 1).firstCursorPosition();
    cursor.insertText( QString::number(nrScenes) );

    // Percentage of dialogues & actions

    qreal totalLength = 0, dialogueLength = 0, actionLength = 0;
    for(int i=0; i<screenplay->elementCount(); i++)
    {
        ScreenplayElement *element = screenplay->elementAt(i);
        if(element->elementType() == ScreenplayElement::SceneElementType)
        {
            Scene *scene = element->scene();
            const QMap<int,int> lengths = scene->totalTextLengths();
            totalLength += lengths[SceneElement::All];
            dialogueLength += lengths[SceneElement::Dialogue];
            actionLength += lengths[SceneElement::Action];
        }
    }

    const int actionPercent = qRound(100*actionLength/totalLength);
    const int dialoguePercent = qRound(100*dialogueLength/totalLength);

    cursor = table->cellAt(1, 0).firstCursorPosition();
    cursor.insertText( QStringLiteral("Action: ") );
    cursor = table->cellAt(1, 1).firstCursorPosition();
    cursor.insertText( QString::number(actionPercent) + QStringLiteral("%") );

    cursor = table->cellAt(2, 0).firstCursorPosition();
    cursor.insertText( QStringLiteral("Dialogue: ") );
    cursor = table->cellAt(2, 1).firstCursorPosition();
    cursor.insertText( QString::number(dialoguePercent) + QStringLiteral("%") );

    // Dialogue screen time for each character
    cursor = table->lastCursorPosition();
    cursor.movePosition(QTextCursor::Down);

    cursor.insertBlock();
    cursor.insertBlock();

    const QStringList characters = structure->allCharacterNames();
    QMap<QString,QPair<int,int> > dialogueLengths;
    for(int i=0; i<screenplay->elementCount(); i++)
    {
        const ScreenplayElement *element = screenplay->elementAt(i);
        const Scene *scene = element->scene();
        if(scene == nullptr)
            continue;

        auto dl = scene->dialogueTextLengths();
        auto it = dl.begin();
        auto end = dl.end();
        while(it != end)
        {
            if(it.value().first > 0)
            {
                auto len = dialogueLengths.value(it.key());
                len.first += it.value().first;
                len.second += it.value().second;
                dialogueLengths[it.key()] = len;
            }

            ++it;
        }
    }

    table = cursor.insertTable(dialogueLengths.size()+1, 3, tableFormat);
    cursor = table->cellAt(0, 0).firstCursorPosition();
    cursor.insertHtml( QStringLiteral("<b>Character</b>") );
    cursor = table->cellAt(0, 1).firstCursorPosition();
    cursor.insertHtml( QStringLiteral("<b>Dialogue Count</b>") );
    cursor = table->cellAt(0, 2).firstCursorPosition();
    cursor.insertHtml( QStringLiteral("<b>Dialogue Percent</b>") );

    QTextBlockFormat cellFormat;
    cellFormat.setAlignment(Qt::AlignRight);

    auto it = dialogueLengths.begin();
    auto end = dialogueLengths.end();
    int row = 1;
    while(it != end)
    {
        const QString character = it.key();

        cursor = table->cellAt(row, 0).firstCursorPosition();
        cursor.insertText(character);

        cursor = table->cellAt(row, 1).firstCursorPosition();
        cursor.mergeBlockFormat(cellFormat);
        cursor.insertText( QString::number(it.value().first) );

        const qreal percent = qRound(10000*it.value().second / dialogueLength);
        cursor = table->cellAt(row, 2).firstCursorPosition();
        cursor.mergeBlockFormat(cellFormat);
        cursor.insertText( QString::number(percent/100) + QStringLiteral("%") );

        ++it;
        ++row;
    }

    return true;
}
