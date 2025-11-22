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

#include "twocolumnreport.h"

#include "scene.h"
#include "screenplay.h"
#include "languageengine.h"
#include "scritedocument.h"
#include "screenplaytextdocument.h"

#include <QBrush>
#include <QTextTable>
#include <QTextBlock>
#include <QTextCursor>
#include <QDomElement>
#include <QDomDocument>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>

TwoColumnReport::TwoColumnReport(QObject *parent) : AbstractReportGenerator(parent) { }

TwoColumnReport::~TwoColumnReport() { }

void TwoColumnReport::setLayout(Layout val)
{
    if (m_layout == val)
        return;

    m_layout = val;
    emit layoutChanged();
}

void TwoColumnReport::setLeftColumnWidth(qreal val)
{
    qreal val2 = qBound(0.2, val, 0.8);
    if (qFuzzyCompare(m_leftColumnWidth, val2))
        return;

    m_leftColumnWidth = val2;
    emit leftColumnWidthChanged();
}

void TwoColumnReport::setGenerateTitlePage(bool val)
{
    if (m_generateTitlePage == val)
        return;

    m_generateTitlePage = val;
    emit generateTitlePageChanged();
}

void TwoColumnReport::setIncludeLogline(bool val)
{
    if (m_includeLogline == val)
        return;

    m_includeLogline = val;
    emit includeLoglineChanged();
}

void TwoColumnReport::setIncludeSceneNumbers(bool val)
{
    if (m_includeSceneNumbers == val)
        return;

    m_includeSceneNumbers = val;
    emit includeSceneNumbersChanged();
}

void TwoColumnReport::setIncludeSceneIcons(bool val)
{
    if (m_includeSceneIcons == val)
        return;

    m_includeSceneIcons = val;
    emit includeSceneIconsChanged();
}

void TwoColumnReport::setPrintEachSceneOnANewPage(bool val)
{
    if (m_printEachSceneOnANewPage == val)
        return;

    m_printEachSceneOnANewPage = val;
    emit printEachSceneOnANewPageChanged();
}

void TwoColumnReport::setIgnorePageBreaks(bool val)
{
    if (m_ignorePageBreaks == val)
        return;

    m_ignorePageBreaks = val;
    emit ignorePageBreaksChanged();
}

void TwoColumnReport::setPreserveMarkupFormatting(bool val)
{
    if (m_preserveMarkupFormatting == val)
        return;

    m_preserveMarkupFormatting = val;
    emit preserveMarkupFormattingChanged();
}

void TwoColumnReport::setUseSingleFont(bool val)
{
    if (m_useSingleFont == val)
        return;

    m_useSingleFont = val;
    emit useSingleFontChanged();
}

void TwoColumnReport::setCharacterNames(const QStringList &val)
{
    if (m_characterNames == val)
        return;

    m_characterNames = val;
    emit characterNamesChanged();
}

void TwoColumnReport::setHighlightCharacterDialogues(bool val)
{
    if (m_highlightCharacterDialogues == val)
        return;

    m_highlightCharacterDialogues = val;
    emit highlightCharacterDialoguesChanged();
}

void TwoColumnReport::setSceneIndexes(const QList<int> &val)
{
    if (m_sceneIndexes == val)
        return;

    m_sceneIndexes = val;
    emit sceneIndexesChanged();
}

void TwoColumnReport::setEpisodeNumbers(const QList<int> &val)
{
    if (m_episodeNumbers == val)
        return;

    m_episodeNumbers = val;
    emit episodeNumbersChanged();
}

void TwoColumnReport::setTags(const QStringList &val)
{
    if (m_tags == val)
        return;

    m_tags = val;
    emit tagsChanged();
}

void TwoColumnReport::setKeywords(const QStringList &val)
{
    if (m_keywords == val)
        return;

    m_keywords = val;
    emit keywordsChanged();
}

bool TwoColumnReport::doGenerate(QTextDocument *document)
{
    QFont defaultFont = document->defaultFont();
    defaultFont.setPointSize(10);
    document->setDefaultFont(defaultFont);

    const ScriteDocument *scriteDocument = this->document();
    const Screenplay *screenplay = scriteDocument->screenplay();
    const ScreenplayFormat *format = scriteDocument->printFormat();

    const int nrProgressSteps =
            screenplay->elementCount() + 2; // one for title-page and another for writing to file
    const qreal progressStep = 1.0 / qreal(nrProgressSteps);
    this->progress()->setProgressStep(progressStep);

    const QMap<QString, QColor> dialogueColorMap = [=]() -> QMap<QString, QColor> {
        static const QList<QColor> highlightColorOptions(
                { QColor("#FFFF00"), QColor("#FFA500"), QColor("#FF69B4"), QColor("#00FF00"),
                  QColor("#00BFFF"), QColor("#DA70D6"), QColor("#FF4500"), QColor("#40E0D0"),
                  QColor("#C8A2C8"), QColor("#FFFFE0") });

        QMap<QString, QColor> ret;
        for (int i = 0; i < m_characterNames.size(); i++) {
            ret[m_characterNames.at(i)] =
                    highlightColorOptions.at(i % highlightColorOptions.size());
        }

        return ret;
    }();

    format->pageLayout()->configure(document);
    document->setIndentWidth(10);

    QTextCursor cursor(document);

    // Title Page
    if (this->format() == AdobePDF) {
        if (m_generateTitlePage) {
            ScreenplayTitlePageObjectInterface *tpoi =
                    document->findChild<ScreenplayTitlePageObjectInterface *>();
            if (tpoi == nullptr) {
                tpoi = new ScreenplayTitlePageObjectInterface(document);
                document->documentLayout()->registerHandler(
                        ScreenplayTitlePageObjectInterface::Kind, tpoi);
            }

            document->setProperty("#includeLoglineInTitlePage",
                                  m_includeLogline ? true : QVariant());

            QTextBlockFormat pageBreakFormat;
            pageBreakFormat.setPageBreakPolicy(QTextBlockFormat::PageBreak_AlwaysAfter);
            cursor.setBlockFormat(pageBreakFormat);

            QTextCharFormat titlePageFormat;
            titlePageFormat.setObjectType(ScreenplayTitlePageObjectInterface::Kind);
            titlePageFormat.setProperty(
                    ScreenplayTitlePageObjectInterface::ScreenplayProperty,
                    QVariant::fromValue<QObject *>(scriteDocument->screenplay()));
            titlePageFormat.setProperty(ScreenplayTitlePageObjectInterface::TitlePageIsCentered,
                                        true);
            cursor.insertText(QString(QChar::ObjectReplacementCharacter), titlePageFormat);
        }

        ScreenplayTextObjectInterface *toi = document->findChild<ScreenplayTextObjectInterface *>();
        if (toi == nullptr) {
            toi = new ScreenplayTextObjectInterface(document);
            document->documentLayout()->registerHandler(ScreenplayTextObjectInterface::Kind, toi);
        }
    }

    this->progress()->tick();

    auto elementCharFormat = [format, document](SceneElement::Type type,
                                                const QColor &bgColor =
                                                        Qt::transparent) -> QTextCharFormat {
        SceneElementFormat *eformat = format->elementFormat(type);

        QTextCharFormat format = eformat->createCharFormat();
        format.clearProperty(QTextFormat::FontPointSize);
        format.setFontFamily(document->defaultFont().family());
        format.setFontPointSize(document->defaultFont().pointSize());
        if (bgColor.alpha() > 0)
            format.setBackground(QBrush(bgColor));

        switch (type) {
        case SceneElement::Heading:
            format.setFontCapitalization(QFont::AllUppercase);
            format.setFontWeight(QFont::Bold);
            break;
        case SceneElement::Shot:
        case SceneElement::Transition:
            format.setFontCapitalization(QFont::AllUppercase);
            break;
        case SceneElement::Character:
            format.setFontWeight(QFont::Bold);
            break;
        case SceneElement::Parenthetical:
            format.setFontItalic(true);
            break;
        default:
            break;
        }

        return format;
    };

    QTextTable *sceneTable = nullptr;
    int sceneCount = -1;
    int currentRow = 0;
    // int nrElements = screenplay->elementCount();

    auto includeElementInReport = [=](const ScreenplayElement *element) -> bool {
        const Scene *scene = element ? element->scene() : nullptr;
        if (scene == nullptr)
            return false;

        if (!this->includeElementByEpisodeNumber(element))
            return false;

        if (!this->includeElementByTag(element))
            return false;

        if (!this->includeElementByKeyword(element))
            return false;

        if (!this->includeElementBySceneIndex(element))
            return false;

        if (!this->includeElementByCharacter(element))
            return false;

        return true;
    };

    for (int i = 0; i < screenplay->elementCount(); i++) {
        const ScreenplayElement *element = screenplay->elementAt(i);
        // this->progress()->setProgressText(
        //         QString("Processing %1 of %2 elements ...").arg(i + 1).arg(nrElements));
        this->progress()->tick();
        if (!includeElementInReport(element))
            continue;

        const Scene *scene = element->scene();
        ++sceneCount;

        enum Column { LeftColumn, RightColumn };

        auto placeCursor = [&](Column col, bool atEnd = false) -> QTextCursor {
            QTextTableCell cell = sceneTable->cellAt(currentRow, col == LeftColumn ? 0 : 1);
            QTextTableCellFormat cellFormat;
            cellFormat.setLeftPadding(5);
            cellFormat.setTopPadding(5);
            cellFormat.setRightPadding(5);
            cellFormat.setBottomPadding(5);
            cell.setFormat(cellFormat);
            return atEnd ? cell.lastCursorPosition() : cell.firstCursorPosition();
        };

        auto moveToNextRow = [&]() -> QTextCursor {
            sceneTable->appendRows(1);
            ++currentRow;

            QTextTableCell cell = sceneTable->cellAt(currentRow, 0);
            return cell.firstCursorPosition();
        };

        auto includeText = [=](QTextCursor &cursor, const QString &text,
                               const QVector<QTextLayout::FormatRange> &formats) {
            if (m_useSingleFont)
                cursor.insertText(text);
            else
                LanguageEngine::polishFontsAndInsertTextAtCursor(
                        cursor, text,
                        m_preserveMarkupFormatting ? formats : QVector<QTextLayout::FormatRange>());
        };

        // Create a two column table
        if (sceneTable == nullptr || m_printEachSceneOnANewPage) {
            QTextTableFormat sceneTableFormat;
            sceneTableFormat.setColumns(2);

            if (this->format() == AdobePDF) {
                const int lColWidth = qRound(m_leftColumnWidth * 100.0);
                const int rColWidth = 100 - lColWidth;
                sceneTableFormat.setColumnWidthConstraints(
                        { QTextLength(QTextLength::PercentageLength, lColWidth),
                          QTextLength(QTextLength::PercentageLength, rColWidth) });
            } else {
                const qreal pageWidth = document->pageSize().width();
                const QTextFrameFormat rootFrameFormat = document->rootFrame()->frameFormat();
                const qreal availableWidth = 0.8
                        * (pageWidth - rootFrameFormat.leftMargin()
                           - rootFrameFormat.rightMargin());
                const qreal lColWidth = availableWidth * m_leftColumnWidth;
                const qreal rColWidth = availableWidth - lColWidth;
                sceneTableFormat.setColumnWidthConstraints(
                        { QTextLength(QTextLength::FixedLength, lColWidth),
                          QTextLength(QTextLength::FixedLength, rColWidth) });
            }
            sceneTableFormat.setCellPadding(5);
            sceneTableFormat.setBorder(0);
            sceneTableFormat.setBorderBrush(Qt::NoBrush);
            sceneTableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_None);
            if ((m_printEachSceneOnANewPage
                 || (!m_ignorePageBreaks && element->isPageBreakBefore()))
                && sceneCount > 0)
                sceneTableFormat.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore);
            if (!m_ignorePageBreaks && element->isPageBreakAfter())
                sceneTableFormat.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysAfter);

            sceneTable = cursor.insertTable(1, 2, sceneTableFormat);

            currentRow = 0;
        }

        if (scene->heading()->isEnabled()) {
            cursor = placeCursor(m_layout == EverythingRight ? RightColumn : LeftColumn);

            if (m_includeSceneIcons && this->format() == AdobePDF) {
                QTextCharFormat sceneIconFormat;
                sceneIconFormat.setObjectType(ScreenplayTextObjectInterface::Kind);
                sceneIconFormat.setFont(format->elementFormat(SceneElement::Heading)->font());
                sceneIconFormat.setProperty(ScreenplayTextObjectInterface::TypeProperty,
                                            ScreenplayTextObjectInterface::SceneIconType);
                sceneIconFormat.setProperty(ScreenplayTextObjectInterface::DataProperty,
                                            scene->type());
                cursor.insertText(QString(QChar::ObjectReplacementCharacter), sceneIconFormat);
            }

            const QString sceneNumber =
                    m_includeSceneNumbers ? element->resolvedSceneNumber() : QString();
            if (this->format() == AdobePDF && !sceneNumber.isEmpty()) {
                QTextCharFormat sceneNumberFormat;
                sceneNumberFormat.setObjectType(ScreenplayTextObjectInterface::Kind);
                sceneNumberFormat.setFont(document->defaultFont());
                sceneNumberFormat.setProperty(ScreenplayTextObjectInterface::TypeProperty,
                                              ScreenplayTextObjectInterface::SceneNumberType);
                const QVariantList data = QVariantList()
                        << sceneNumber << scene->heading()->text()
                        << screenplay->indexOfElement(const_cast<ScreenplayElement *>(element));
                sceneNumberFormat.setProperty(ScreenplayTextObjectInterface::DataProperty, data);
                cursor.insertText(QString(QChar::ObjectReplacementCharacter), sceneNumberFormat);
            }

            const QString headingText =
                    ((sceneNumber.isEmpty() || this->format() == AdobePDF) ? QString()
                                                                           : (sceneNumber + ". "))
                    + scene->heading()->displayText();

            QTextCharFormat headingFormat = elementCharFormat(SceneElement::Heading);
            if (element->isOmitted())
                headingFormat.setFontStrikeOut(true);

            cursor.setCharFormat(headingFormat);
            includeText(cursor, headingText, QVector<QTextLayout::FormatRange>());

            if (!element->isOmitted())
                cursor = moveToNextRow();
        }

        if (m_layout == EverythingRight || m_layout == EverythingLeft) {
            if (!element->isOmitted()) {
                QString currentCharacter;

                for (int j = 0; j < scene->elementCount(); j++) {
                    const SceneElement *sceneElement = scene->elementAt(j);

                    if (sceneElement->type() == SceneElement::Character)
                        currentCharacter =
                                sceneElement->formattedText().section('(', 0, 0).trimmed();
                    else if (QList<int>({ SceneElement::Action, SceneElement::Shot,
                                          SceneElement::Transition })
                                     .contains(sceneElement->type()))
                        currentCharacter.clear();

                    const bool dialogueOrParenthetical =
                            QVector<SceneElement::Type>(
                                    { SceneElement::Parenthetical, SceneElement::Dialogue })
                                    .contains(sceneElement->type());
                    if (!dialogueOrParenthetical) {
                        if (j > 0)
                            cursor = moveToNextRow();
                        cursor = placeCursor(m_layout == EverythingRight ? RightColumn : LeftColumn,
                                             true);
                    } else if (j > 0
                               && scene->elementAt(j - 1)->type() != SceneElement::Character) {
                        if (this->format() == AdobePDF)
                            cursor.insertBlock();
                        else
                            cursor.insertText(" ");
                    }

                    const QString suffix = sceneElement->type() == SceneElement::Character
                            ? QStringLiteral(": ")
                            : QString();
                    const QString text = sceneElement->formattedText() + suffix;
                    const QColor bgColor = !m_highlightCharacterDialogues
                                    || sceneElement->type() != SceneElement::Dialogue
                                    || currentCharacter.isEmpty() || m_characterNames.isEmpty()
                            ? Qt::transparent
                            : dialogueColorMap.value(currentCharacter, Qt::transparent);

                    if (dialogueOrParenthetical
                        || sceneElement->type() == SceneElement::Character) {
                        const qreal pageWidth = document->pageSize().width();
                        QTextBlockFormat format;
                        format.setLeftMargin(pageWidth * 0.05);
                        cursor.mergeBlockFormat(format);
                    }

                    cursor.setCharFormat(elementCharFormat(sceneElement->type(), bgColor));
                    includeText(cursor, text, sceneElement->textFormats());
                }
            }
        } else {
            if (!element->isOmitted()) {
                QString currentCharacter;

                for (int j = 0; j < scene->elementCount(); j++) {
                    const SceneElement *sceneElement = scene->elementAt(j);

                    if (sceneElement->type() == SceneElement::Character)
                        currentCharacter =
                                sceneElement->formattedText().section('(', 0, 0).trimmed();
                    else if (QList<int>({ SceneElement::Action, SceneElement::Shot,
                                          SceneElement::Transition })
                                     .contains(sceneElement->type()))
                        currentCharacter.clear();

                    switch (sceneElement->type()) {
                    case SceneElement::Action:
                    case SceneElement::Transition:
                    case SceneElement::Shot: {
                        cursor = placeCursor(LeftColumn);
                        cursor.setCharFormat(elementCharFormat(sceneElement->type()));
                        includeText(cursor, sceneElement->formattedText(),
                                    sceneElement->textFormats());
                        cursor = moveToNextRow();
                    } break;
                    case SceneElement::Character:
                    case SceneElement::Parenthetical: {
                        cursor = placeCursor(LeftColumn, this->format() == AdobePDF);

                        const bool cellIsEmpty = cursor.block().text().isEmpty();

                        QTextBlockFormat format;
                        format.setAlignment(Qt::AlignRight);
                        if (sceneElement->type() == SceneElement::Parenthetical)
                            format.setLeftMargin(100);

                        if (cellIsEmpty)
                            cursor.mergeBlockFormat(format);
                        else if (this->format() == AdobePDF)
                            cursor.insertBlock(format);

                        const QString suffix = sceneElement->type() == SceneElement::Character
                                ? QStringLiteral(":")
                                : (cellIsEmpty || this->format() == AdobePDF
                                           ? QString()
                                           : QStringLiteral(", "));
                        cursor.setCharFormat(elementCharFormat(sceneElement->type()));
                        includeText(cursor, sceneElement->formattedText() + suffix,
                                    sceneElement->textFormats());
                    } break;
                    case SceneElement::Dialogue: {
                        const QColor bgColor = !m_highlightCharacterDialogues
                                        || sceneElement->type() != SceneElement::Dialogue
                                        || currentCharacter.isEmpty() || m_characterNames.isEmpty()
                                ? Qt::transparent
                                : dialogueColorMap.value(currentCharacter, Qt::transparent);

                        cursor = placeCursor(RightColumn);
                        cursor.setCharFormat(elementCharFormat(sceneElement->type(), bgColor));
                        includeText(cursor, sceneElement->formattedText(),
                                    sceneElement->textFormats());
                        cursor = moveToNextRow();
                    } break;
                    default:
                        break;
                    }
                }
            }
        }

        QTextFrame *rootFrame = document->rootFrame();
        cursor = rootFrame->lastCursorPosition();
        sceneTable = nullptr;
    }

    return true;
}

bool TwoColumnReport::requiresOdtContentPolish() const
{
    return m_printEachSceneOnANewPage;
}

bool TwoColumnReport::polishOdtContent(QDomDocument &xmlDoc)
{
    QDomElement rootE = xmlDoc.documentElement();
    QDomElement bodyE = rootE.firstChildElement("office:body");

    const QDomNodeList tableElements = bodyE.elementsByTagName("table:table");
    QStringList tableStyles;
    for (int i = 0; i < tableElements.size(); i++) {
        const QDomElement tableE = tableElements.at(i).toElement();
        const QString styleName = tableE.attribute("table:style-name");
        if (tableStyles.isEmpty() || tableStyles.last() != styleName)
            tableStyles.append(styleName);
    }

    if (!tableStyles.isEmpty())
        tableStyles.takeFirst();

    if (tableStyles.isEmpty())
        return false;

    bool success = false;
    QDomElement stylesE = rootE.firstChildElement("office:automatic-styles");
    QDomElement styleE = stylesE.firstChildElement("style:style");
    while (!styleE.isNull()) {
        const QString styleName = styleE.attribute("style:name");
        const QString styleFamily = styleE.attribute("style:family");
        if (styleFamily == "table" && tableStyles.contains(styleName)) {
            QDomElement stylePropsE = styleE.firstChildElement("style:table-properties");
            stylePropsE.setAttribute("fo:break-before", "page");
            success |= true;
        }

        styleE = styleE.nextSiblingElement(styleE.tagName());
    }

    return success;
}

bool TwoColumnReport::includeElementByTag(const ScreenplayElement *element) const
{
    if (m_tags.isEmpty())
        return true;

    return Utils::SMath::doListsIntersect(element->scene()->groups(), m_tags);
}

bool TwoColumnReport::includeElementByKeyword(const ScreenplayElement *element) const
{
    if (m_keywords.isEmpty())
        return true;

    return Utils::SMath::doListsIntersect(element->scene()->tags(), m_keywords);
}

bool TwoColumnReport::includeElementByCharacter(const ScreenplayElement *element) const
{
    if (m_characterNames.isEmpty())
        return true;

    const Scene *scene = element->scene();
    const QStringList sceneCharacters = scene->characterNames();
    for (const QString &characterName : qAsConst(m_characterNames))
        if (sceneCharacters.contains(characterName))
            return true;

    return false;
}

bool TwoColumnReport::includeElementBySceneIndex(const ScreenplayElement *element) const
{
    if (m_sceneIndexes.isEmpty())
        return true;

    const int sceneIndex = element->elementIndex();
    return m_sceneIndexes.contains(sceneIndex);
}

bool TwoColumnReport::includeElementByEpisodeNumber(const ScreenplayElement *element) const
{
    if (m_episodeNumbers.isEmpty())
        return true;

    const Screenplay *screenplay = element->screenplay();
    if (screenplay->episodeCount() <= 0)
        return true;

    const int episodeNumber = element->episodeIndex() + 1;
    return (m_episodeNumbers.contains(episodeNumber));
}
