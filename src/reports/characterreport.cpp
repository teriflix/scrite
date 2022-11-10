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

#include "form.h"
#include "deltadocument.h"
#include "characterreport.h"
#include "transliteration.h"

#include <QTextTable>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextCharFormat>
#include <QTextBlockFormat>

CharacterReport::CharacterReport(QObject *parent) : AbstractReportGenerator(parent) { }

CharacterReport::~CharacterReport() { }

void CharacterReport::setCharacterNames(const QStringList &val)
{
    if (m_characterNames == val)
        return;

    m_characterNames = val;
    emit characterNamesChanged();
}

void CharacterReport::setIncludeSceneHeadings(bool val)
{
    if (m_includeSceneHeadings == val)
        return;

    m_includeSceneHeadings = val;
    emit includeSceneHeadingsChanged();
}

void CharacterReport::setIncludeDialogues(bool val)
{
    if (m_includeDialogues == val)
        return;

    m_includeDialogues = val;
    emit includeDialoguesChanged();
}

void CharacterReport::setIncludeNotes(bool val)
{
    if (m_includeNotes == val)
        return;

    m_includeNotes = val;
    emit includeNotesChanged();
}

bool CharacterReport::doGenerate(QTextDocument *textDocument)
{
    if (m_characterNames.isEmpty()) {
        this->error()->setErrorMessage("No character was selected for report generation.");
        return false;
    }

    const Screenplay *screenplay = this->document()->screenplay();

    QTextDocument &document = *textDocument;
    QTextCursor cursor(&document);

    const QFont defaultFont = this->document()->printFormat()->defaultFont();

    QTextBlockFormat defaultBlockFormat;

    QTextCharFormat defaultCharFormat;
    defaultCharFormat.setFontFamily(defaultFont.family());
    defaultCharFormat.setFontPointSize(12);

    this->progress()->setProgressStepFromCount(screenplay->elementCount() + 2);

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
        TransliterationEngine::instance()->evaluateBoundariesAndInsertText(cursor, title);
        // cursor.insertText(title);
        if (!screenplay->subtitle().isEmpty()) {
            cursor.insertBlock();
            // cursor.insertText(screenplay->subtitle());
            TransliterationEngine::instance()->evaluateBoundariesAndInsertText(
                    cursor, screenplay->subtitle());
        }

        blockFormat.setBottomMargin(20);

        cursor.insertBlock(blockFormat, charFormat);
        if (m_characterNames.size() == 1)
            cursor.insertText("Character Report For \"" + m_characterNames.first() + "\"");
        else {
            cursor.insertText("Characters Report For ");
            for (int i = 0; i < m_characterNames.length(); i++) {
                if (i)
                    cursor.insertText(", ");
                cursor.insertText("\"" + m_characterNames.at(i) + "\"");
            }
        }

        blockFormat = defaultBlockFormat;
        blockFormat.setAlignment(Qt::AlignHCenter);
        blockFormat.setBottomMargin(20);
        charFormat = defaultCharFormat;
        cursor.insertBlock(blockFormat, charFormat);
        cursor.insertHtml("This report was generated using <strong>Scrite</strong><br/>(<a "
                          "href=\"https://www.scrite.io\">https://www.scrite.io</a>)");
    }
    this->progress()->tick();

    const int reportSummaryPosition = cursor.position();

    if (m_includeNotes) {
        QTextBlockFormat blockFormat = defaultBlockFormat;
        blockFormat.setAlignment(Qt::AlignLeft);
        blockFormat.setTopMargin(20);

        QTextCharFormat charFormat = defaultCharFormat;
        charFormat.setFontPointSize(20);
        charFormat.setFontCapitalization(QFont::AllUppercase);
        charFormat.setFontWeight(QFont::Bold);
        charFormat.setFontItalic(true);

        cursor.insertBlock(blockFormat, charFormat);
        cursor.insertText("NOTES:");

        const Structure *structure = this->document()->structure();
        int characterNr = 0;
        for (const QString &characterName : qAsConst(m_characterNames)) {
            ++characterNr;
            blockFormat = defaultBlockFormat;
            blockFormat.setIndent(1);

            charFormat = defaultCharFormat;
            charFormat.setFontWeight(QFont::Bold);

            cursor.insertBlock(blockFormat, charFormat);
            TransliterationEngine::instance()->evaluateBoundariesAndInsertText(cursor,
                                                                               characterName);

            charFormat.setFontWeight(QFont::Normal);
            cursor.setCharFormat(charFormat);

            const Character *character = structure->findCharacter(characterName);

            QTextTableFormat tableFormat;
            tableFormat.setCellPadding(1);
            tableFormat.setCellSpacing(0);
            tableFormat.setLeftMargin(80);
            tableFormat.setTopMargin(10);

            QTextFrame *frame = cursor.currentFrame();
            QTextTable *table =
                    cursor.insertTable(6, character->hasKeyPhoto() ? 3 : 2, tableFormat);
            QTextTableCellFormat cellFormat; // = table->cellAt(0, 0).format();
            cellFormat.setBorderStyle(QTextFrameFormat::BorderStyle_None);
            for (int tr = 0; tr < table->rows(); tr++) {
                for (int tc = 0; tc < table->columns(); tc++) {
                    table->cellAt(tr, tc).setFormat(cellFormat);
                }
            }

            if (character->hasKeyPhoto()) {
                table->mergeCells(0, 0, 6, 1);

                const QImage image(character->keyPhoto());
                const qreal imageScale = 120.0 / image.width();
                const QSizeF imageSize = image.size() * imageScale;
                const QUrl imageName(QLatin1String("character://")
                                     + characterName.toLatin1().toHex() + QLatin1String("/")
                                     + QString::number(characterNr));
                document.addResource(QTextDocument::ImageResource, imageName, image);

                QTextImageFormat imageFormat;
                imageFormat.setWidth(imageSize.width());
                imageFormat.setHeight(imageSize.height());
                imageFormat.setName(imageName.toString());

                cursor = table->cellAt(0, 0).firstCursorPosition();
                cursor.insertImage(imageFormat);
            }

            const int col = character->hasKeyPhoto() ? 1 : 0;

            table->mergeCells(0, col, 1, 2);
            cursor = table->cellAt(0, col).firstCursorPosition();
            cursor.insertText(QLatin1String("Role: ") + character->designation());

            table->mergeCells(1, col, 1, 2);
            cursor = table->cellAt(1, col).firstCursorPosition();
            cursor.insertText(QLatin1String("Tags: ")
                              + character->tags().join(QLatin1String(", ")));

            table->mergeCells(2, col, 1, 2);
            cursor = table->cellAt(2, col).firstCursorPosition();
            cursor.insertText(QLatin1String("Aliases: ")
                              + character->aliases().join(QLatin1String(", ")));

            cursor = table->cellAt(3, col).firstCursorPosition();
            cursor.insertText(QLatin1String("Type: ") + character->type());

            cursor = table->cellAt(3, col + 1).firstCursorPosition();
            cursor.insertText(QLatin1String("Gender: ") + character->gender());

            cursor = table->cellAt(4, col).firstCursorPosition();
            cursor.insertText(QLatin1String("Age: ") + character->age());

            cursor = table->cellAt(4, col + 1).firstCursorPosition();
            cursor.insertText(QLatin1String("Body Type: ") + character->bodyType());

            cursor = table->cellAt(5, col).firstCursorPosition();
            cursor.insertText(QLatin1String("Height: ") + character->height());

            cursor = table->cellAt(5, col + 1).firstCursorPosition();
            cursor.insertText(QLatin1String("Weight: ") + character->weight());

            cursor = frame->lastCursorPosition();

            const QJsonValue summaryContent = character->summary();
            if (summaryContent.isString() || summaryContent.isObject()) {
                const QString summaryText = summaryContent.toString().trimmed();
                const QJsonObject summaryDelta = summaryContent.toObject();
                if (!summaryText.isEmpty() || !summaryDelta.isEmpty()) {
                    const QString heading = QLatin1String("Character Summary:");

                    blockFormat = defaultBlockFormat;
                    blockFormat.setIndent(2);
                    blockFormat.setTopMargin(10);

                    charFormat = defaultCharFormat;
                    charFormat.setFontPointSize(charFormat.fontPointSize() + 2);
                    charFormat.setFontUnderline(true);
                    charFormat.setFontWeight(QFont::Bold);

                    cursor.insertBlock(blockFormat, charFormat);
                    TransliterationEngine::instance()->evaluateBoundariesAndInsertText(cursor,
                                                                                       heading);

                    charFormat.setFontPointSize(charFormat.fontPointSize() - 2);
                    charFormat.setFontUnderline(false);

                    blockFormat.setTopMargin(0);
                    blockFormat.setBottomMargin(0);
                    blockFormat.setAlignment(Qt::AlignJustify);

                    charFormat.setFontWeight(QFont::Normal);
                    cursor.insertBlock(blockFormat, charFormat);

                    if (summaryContent.isString())
                        TransliterationEngine::instance()->evaluateBoundariesAndInsertText(
                                cursor, summaryContent.toString());
                    else
                        DeltaDocument::blockingResolveAndInsertHtml(summaryContent.toObject(),
                                                                    cursor);
                }
            }

            blockFormat = defaultBlockFormat;
            blockFormat.setIndent(1);
            blockFormat.setTopMargin(10);

            cursor.insertBlock(blockFormat, defaultCharFormat);

            const Notes *characterNotes = character ? character->notes() : nullptr;
            if (characterNotes == nullptr || characterNotes->noteCount() == 0) {
                cursor.insertText("No notes available.");
                continue;
            }

            cursor.insertText(QString::number(characterNotes->noteCount()) + " note(s) available.");

            for (int i = 0; i < characterNotes->noteCount(); i++) {
                const Note *note = characterNotes->noteAt(i);
                QString heading = note->title().trimmed();
                if (heading.isEmpty())
                    heading = QLatin1String("Note #") + QString::number(i + 1);
                else
                    heading = QString::number(i + 1) + QLatin1String(". ") + heading;

                blockFormat = defaultBlockFormat;
                blockFormat.setIndent(2);
                blockFormat.setTopMargin(10);

                charFormat = defaultCharFormat;
                charFormat.setFontPointSize(charFormat.fontPointSize() + 2);
                charFormat.setFontUnderline(true);
                charFormat.setFontWeight(QFont::Bold);

                cursor.insertBlock(blockFormat, charFormat);
                TransliterationEngine::instance()->evaluateBoundariesAndInsertText(cursor, heading);

                charFormat.setFontPointSize(charFormat.fontPointSize() - 2);
                charFormat.setFontUnderline(false);

                blockFormat.setTopMargin(0);
                blockFormat.setBottomMargin(0);
                blockFormat.setAlignment(Qt::AlignJustify);

                charFormat.setFontWeight(QFont::Normal);
                cursor.insertBlock(blockFormat, charFormat);

                if (note->type() == Note::TextNoteType) {
                    const QJsonValue noteContent = note->content();
                    if (noteContent.isString())
                        TransliterationEngine::instance()->evaluateBoundariesAndInsertText(
                                cursor, noteContent.toString());
                    else
                        DeltaDocument::blockingResolveAndInsertHtml(noteContent.toObject(), cursor);
                }

                if (note->type() == Note::FormNoteType) {
                    Form *form = note->form();
                    if (form != nullptr) {
                        for (int i = 0; i < form->questionCount(); i++) {
                            FormQuestion *q = form->questionAt(i);
                            QTextBlockFormat qBlockFormat = blockFormat;
                            qBlockFormat.setIndent(blockFormat.indent() + q->indentation());

                            const QString questionText =
                                    q->number() + QStringLiteral(". ") + q->questionText();
                            charFormat.setFontWeight(QFont::Bold);
                            cursor.insertBlock(qBlockFormat, charFormat);
                            cursor.insertBlock();
                            TransliterationEngine::instance()->evaluateBoundariesAndInsertText(
                                    cursor, questionText);

                            const QString answerText = [q, note]() {
                                const QString ret = note->getFormData(q->id()).toString();
                                return ret.isEmpty() ? QLatin1String("-NA-") : ret;
                            }();
                            charFormat.setFontWeight(QFont::Normal);
                            cursor.insertBlock(qBlockFormat, charFormat);
                            TransliterationEngine::instance()->evaluateBoundariesAndInsertText(
                                    cursor, answerText);
                        }
                    }
                }
            }

            cursor.insertBlock();
        }
    }

    if (!m_includeDialogues && !m_includeSceneHeadings)
        return true;

    QMap<QString, int> dialogCount;
    QMap<QString, int> sceneCount;

    // Report Detail
    {
        QTextFrame *detailFrame = cursor.currentFrame();

        QTextBlockFormat blockFormat = defaultBlockFormat;
        blockFormat.setAlignment(Qt::AlignLeft);
        blockFormat.setTopMargin(20);

        QTextCharFormat charFormat = defaultCharFormat;
        charFormat.setFontPointSize(20);
        charFormat.setFontCapitalization(QFont::AllUppercase);
        charFormat.setFontWeight(QFont::Bold);
        charFormat.setFontItalic(true);

        cursor.insertBlock(blockFormat, charFormat);
        cursor.insertText("DETAIL:");

        const int nrScenes = screenplay->elementCount();
        for (int i = 0; i < nrScenes; i++) {
            QTextTable *dialogueTable = nullptr;
            bool sceneInfoWritten = false;
            ScreenplayElement *element = screenplay->elementAt(i);
            Scene *scene = element->scene();
            if (scene == nullptr)
                continue;

            bool sceneHasSaidCharacters = false;
            for (const QString &characterName : qAsConst(m_characterNames)) {
                if (scene->characterNames().contains(characterName)) {
                    sceneCount[characterName] = sceneCount.value(characterName, 0) + 1;

                    if (sceneInfoWritten == false && m_includeSceneHeadings) {
                        // Write Scene Information First
                        QTextBlockFormat blockFormat = defaultBlockFormat;
                        if (!dialogCount.isEmpty())
                            blockFormat.setTopMargin(20);

                        QTextCharFormat charFormat = defaultCharFormat;
                        charFormat.setFontPointSize(14);
                        charFormat.setFontCapitalization(QFont::AllUppercase);
                        charFormat.setFontWeight(QFont::Bold);
                        charFormat.setFontItalic(false);

                        cursor.insertBlock(blockFormat, charFormat);
                        // cursor.insertText("Scene [" + QString::number(i+1) + "]: " +
                        // scene->heading()->text());
                        TransliterationEngine::instance()->evaluateBoundariesAndInsertText(
                                cursor,
                                "Scene [" + element->resolvedSceneNumber()
                                        + "]: " + scene->heading()->text());
                        sceneInfoWritten = true;
                    }

                    sceneHasSaidCharacters = true;
                }
            }

            if (!sceneHasSaidCharacters)
                continue;

            QMap<QString, bool> characterHasDialogue;

            const int nrElements = scene->elementCount();
            for (int j = 0; j < nrElements; j++) {
                SceneElement *element = scene->elementAt(j);
                if (element->type() == SceneElement::Character) {
                    QString characterName = element->formattedText();
                    characterName = characterName.section('(', 0, 0).trimmed();
                    if (m_characterNames.contains(characterName)) {
                        characterHasDialogue[characterName] = true;

                        if (m_includeDialogues) {
                            // Write dialogue information next
                            if (dialogueTable == nullptr) {
                                QTextTableFormat tableFormat;
                                tableFormat.setLeftMargin(40);
                                tableFormat.setCellSpacing(0);
                                tableFormat.setCellPadding(5);
                                tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_None);

                                dialogueTable = cursor.insertTable(1, 2, tableFormat);
                            } else
                                dialogueTable->appendRows(1);

                            QTextBlockFormat blockFormat = defaultBlockFormat;

                            QTextCharFormat charFormat = defaultCharFormat;
                            charFormat.setFontCapitalization(QFont::AllUppercase);
                            charFormat.setFontWeight(QFont::Bold);

                            cursor = dialogueTable->cellAt(dialogueTable->rows() - 1, 0)
                                             .firstCursorPosition();
                            cursor.setCharFormat(charFormat);
                            cursor.setBlockFormat(blockFormat);
                            // cursor.insertText(characterName);
                            TransliterationEngine::instance()->evaluateBoundariesAndInsertText(
                                    cursor, characterName);

                            cursor = dialogueTable->cellAt(dialogueTable->rows() - 1, 1)
                                             .firstCursorPosition();
                            blockFormat.setAlignment(Qt::AlignJustify);
                            charFormat = defaultCharFormat;

                            int nr = 0;
                            while (1) {
                                ++j;
                                element = scene->elementAt(j);
                                if (element == nullptr)
                                    break;

                                if (element->type() != SceneElement::Parenthetical
                                    && element->type() != SceneElement::Dialogue) {
                                    --j;
                                    break;
                                }

                                charFormat.setFontItalic(element->type()
                                                         == SceneElement::Parenthetical);
                                blockFormat.setBottomMargin(
                                        element->type() == SceneElement::Dialogue ? 10 : 0);

                                if (nr == 0) {
                                    cursor.setCharFormat(charFormat);
                                    cursor.setBlockFormat(blockFormat);
                                } else
                                    cursor.insertBlock(blockFormat, charFormat);
                                // cursor.insertText(element->formattedText());
                                TransliterationEngine::instance()->evaluateBoundariesAndInsertText(
                                        cursor, element->formattedText());
                                ++nr;
                            }
                        }

                        dialogCount[characterName] = dialogCount.value(characterName, 0) + 1;
                    }
                }
            }

            cursor = detailFrame->lastCursorPosition();

            QStringList muteCharacters;
            for (const QString &characterName : qAsConst(m_characterNames)) {
                if (characterHasDialogue.value(characterName, false) == false
                    && scene->characterNames().contains(characterName)) {
                    muteCharacters << characterName;
                }
            }

            if (!muteCharacters.isEmpty()) {
                const QString isare =
                        muteCharacters.size() == 1 ? QLatin1String(" is ") : QLatin1String(" are ");
                const QString names = [&muteCharacters]() -> QString {
                    if (muteCharacters.size() == 1)
                        return muteCharacters.first();

                    QString lastName = muteCharacters.takeLast();
                    QString ret = muteCharacters.join(QLatin1String(", "));
                    ret += QLatin1String(" and ") + lastName;
                    muteCharacters.append(lastName);
                    return ret;
                }();

                QTextBlockFormat blockFormat = defaultBlockFormat;
                blockFormat.setBottomMargin(20);

                QTextCharFormat charFormat = defaultCharFormat;

                cursor.insertBlock(blockFormat, charFormat);

                TransliterationEngine::instance()->evaluateBoundariesAndInsertText(
                        cursor,
                        names + isare + QLatin1String("present in this scene and") + isare
                                + QLatin1String("mute."));
            }

            cursor = detailFrame->lastCursorPosition();

            this->progress()->tick();
        }
    }

    // Report Summary
    {
        cursor.setPosition(reportSummaryPosition);

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

        blockFormat = defaultBlockFormat;
        blockFormat.setIndent(1);

        QMap<QString, int>::const_iterator it = dialogCount.constBegin();
        QMap<QString, int>::const_iterator end = dialogCount.constEnd();
        while (it != end) {
            const int nrScenes = sceneCount.value(it.key(), 1);

            charFormat = defaultCharFormat;
            charFormat.setFontWeight(QFont::Bold);
            charFormat.setFontCapitalization(QFont::AllUppercase);

            cursor.insertBlock(blockFormat, charFormat);
            TransliterationEngine::instance()->evaluateBoundariesAndInsertText(cursor, it.key());
            // cursor.insertText(it.key());

            charFormat = defaultCharFormat;
            cursor.setCharFormat(charFormat);
            cursor.insertText(" speaks " + QString::number(it.value()) + " times and is present in "
                              + QString::number(nrScenes) + " scene(s).");

            ++it;
        }
    }
    this->progress()->tick();

    return true;
}
