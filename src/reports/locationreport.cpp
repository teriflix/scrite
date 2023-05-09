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

#include "locationreport.h"
#include "transliteration.h"

LocationReport::LocationReport(QObject *parent) : AbstractReportGenerator(parent) { }

LocationReport::~LocationReport() { }

bool LocationReport::doGenerate(QTextDocument *textDocument)
{
    static const int snippetLength = 40;
    const Structure *structure = this->document()->structure();
    const Screenplay *screenplay = this->document()->screenplay();

    QTextDocument &document = *textDocument;
    document.setIndentWidth(20);

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
        if (title.isEmpty())
            title = "Untitled Screenplay";
        cursor.insertText(title);
        cursor.insertBlock();
        cursor.insertText("Location Report");

        blockFormat.setBottomMargin(20);

        blockFormat = defaultBlockFormat;
        blockFormat.setAlignment(Qt::AlignHCenter);
        blockFormat.setBottomMargin(20);
        charFormat = defaultCharFormat;
        cursor.insertBlock(blockFormat, charFormat);
        cursor.insertHtml("This report was generated using <strong>Scrite</strong><br/>(<a "
                          "href=\"https://www.scrite.io\">https://www.scrite.io</a>)");
    }
    this->progress()->tick();

    const QMap<QString, QList<SceneHeading *>> locationHeadingsMap =
            structure->locationHeadingsMap();
    this->progress()->setProgressStepFromCount(locationHeadingsMap.size() + 2);

    QMap<QString, QList<SceneHeading *>>::const_iterator it = locationHeadingsMap.constBegin();
    QMap<QString, QList<SceneHeading *>>::const_iterator end = locationHeadingsMap.constEnd();
    while (it != end) {
        this->progress()->tick();
        QMap<QString, QMap<QString, QList<SceneHeading *>>> map;
        QList<SceneHeading *> headings = it.value();
        for (int i = headings.size() - 1; i >= 0; i--) {
            SceneHeading *heading = headings.at(i);
            Scene *scene = heading->scene();
            if (screenplay->firstIndexOfScene(scene) < 0)
                headings.removeAt(i);
            else
                map[heading->locationType()][heading->moment()].prepend(heading);
        }

        if (headings.isEmpty()) {
            ++it;
            continue;
        }

        QTextBlockFormat blockFormat = defaultBlockFormat;
        blockFormat.setTopMargin(20);

        QTextCharFormat charFormat = defaultCharFormat;
        charFormat.setFontPointSize(defaultCharFormat.fontPointSize() + 3);
        charFormat.setFontWeight(QFont::Bold);

        cursor.insertBlock(blockFormat, charFormat);
        cursor.insertText(it.key());
        cursor.insertText(" (" + QString::number(headings.size()) + " occurrences)");

        const QStringList locTypes = map.keys();
        for (const QString &locType : locTypes) {
            const QMap<QString, QList<SceneHeading *>> momentMap = map.value(locType);
            QMap<QString, QList<SceneHeading *>>::const_iterator it2 = momentMap.constBegin();
            QMap<QString, QList<SceneHeading *>>::const_iterator end2 = momentMap.constEnd();
            int counter = 0;
            while (it2 != end2) {
                counter += it2.value().size();
                ++it2;
            }

            blockFormat = defaultBlockFormat;
            blockFormat.setIndent(1);

            charFormat = defaultCharFormat;
            charFormat.setFontWeight(QFont::Bold);

            cursor.insertBlock(blockFormat, charFormat);
            cursor.insertText(locType + " (" + QString::number(counter) + ")");

            it2 = momentMap.constBegin();
            while (it2 != end2) {
                blockFormat = defaultBlockFormat;
                blockFormat.setIndent(2);
                charFormat = defaultCharFormat;

                cursor.insertBlock(blockFormat, charFormat);
                TransliterationEngine::instance()->evaluateBoundariesAndInsertText(
                        cursor, it2.value().first()->text());
                cursor.insertText(" (" + QString::number(it.value().size()) + ")");

                for (SceneHeading *heading : qAsConst(it2.value())) {
                    Scene *scene = heading->scene();
                    int sceneNr = screenplay->firstIndexOfScene(scene) + 1;
                    ScreenplayElement *screenplayElement = screenplay->elementAt(sceneNr - 1);
                    QString snippet = scene->synopsis();
                    if (snippet.length() > snippetLength)
                        snippet = snippet.left(snippetLength - 3) + "...";

                    blockFormat = defaultBlockFormat;
                    blockFormat.setIndent(3);
                    charFormat = defaultCharFormat;

                    cursor.insertBlock(blockFormat, charFormat);
                    if (screenplayElement)
                        cursor.insertText("Scene #" + screenplayElement->resolvedSceneNumber()
                                          + +": ");
                    else
                        cursor.insertText("Scene #" + QString::number(sceneNr) + +": ");
                    TransliterationEngine::instance()->evaluateBoundariesAndInsertText(cursor,
                                                                                       snippet);
                }

                ++it2;
            }
        }

        ++it;
    }

    return true;
}
