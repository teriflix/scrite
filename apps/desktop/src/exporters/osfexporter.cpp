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

#include "osfexporter.h"
#include "notes.h"
#include "structure.h"
#include "screenplay.h"
#include "scritedocument.h"
#include "screenplayformat.h"

#include <QDomDocument>
#include <QDomElement>
#include <QPageSize>
#include <QTextStream>
#include <QUuid>

// OSF XML tag names
static const QString OSF_TagDocument = QStringLiteral("document");
static const QString OSF_TagInfo = QStringLiteral("info");
static const QString OSF_TagSettings = QStringLiteral("settings");
static const QString OSF_TagStyles = QStringLiteral("styles");
static const QString OSF_TagStyle = QStringLiteral("style");
static const QString OSF_TagParagraphs = QStringLiteral("paragraphs");
static const QString OSF_TagPara = QStringLiteral("para");
static const QString OSF_TagText = QStringLiteral("text");
static const QString OSF_TagTitlepage = QStringLiteral("titlepage");
static const QString OSF_TagNotes = QStringLiteral("notes");
static const QString OSF_TagNote = QStringLiteral("note");
static const QString OSF_TagLists = QStringLiteral("lists");
static const QString OSF_TagCharacters = QStringLiteral("characters");
static const QString OSF_TagCharacter = QStringLiteral("character");
static const QString OSF_TagLocations = QStringLiteral("locations");
static const QString OSF_TagLocation = QStringLiteral("location");
static const QString OSF_TagSceneIntros = QStringLiteral("scene_intros");
static const QString OSF_TagSceneIntro = QStringLiteral("scene_intro");
static const QString OSF_TagSceneTimes = QStringLiteral("scene_times");
static const QString OSF_TagSceneTime = QStringLiteral("scene_time");
static const QString OSF_TagExtensions = QStringLiteral("extensions");
static const QString OSF_TagExtension = QStringLiteral("extension");
static const QString OSF_TagTransitions = QStringLiteral("transitions");
static const QString OSF_TagTransition = QStringLiteral("transition");
static const QString OSF_TagRevisionColors = QStringLiteral("revision_colors");
static const QString OSF_TagRevisionColor = QStringLiteral("revision_color");
static const QString OSF_TagHeaderStyle = QStringLiteral("header_style");
static const QString OSF_TagFooterStyle = QStringLiteral("footer_style");
static const QString OSF_TagAPages = QStringLiteral("a_pages");
static const QString OSF_TagAbPages = QStringLiteral("ab_pages");
static const QString OSF_TagSpelling = QStringLiteral("spelling");
static const QString OSF_TagTagCategories = QStringLiteral("tag_categories");
static const QString OSF_TagHighlightRules = QStringLiteral("highlight_rules");

// OSF attribute names
static const QString OSF_AttrType = QStringLiteral("type");
static const QString OSF_AttrVersion = QStringLiteral("version");
static const QString OSF_AttrUuid = QStringLiteral("uuid");
static const QString OSF_AttrDraftUuid = QStringLiteral("draft_uuid");
static const QString OSF_AttrPagecount = QStringLiteral("pagecount");
static const QString OSF_AttrPageWidth = QStringLiteral("page_width");
static const QString OSF_AttrPageHeight = QStringLiteral("page_height");
static const QString OSF_AttrMarginTop = QStringLiteral("margin_top");
static const QString OSF_AttrMarginBottom = QStringLiteral("margin_bottom");
static const QString OSF_AttrMarginLeft = QStringLiteral("margin_left");
static const QString OSF_AttrMarginRight = QStringLiteral("margin_right");
static const QString OSF_AttrNormalLinesPerInch = QStringLiteral("normal_linesperinch");
static const QString OSF_AttrElementSpacing = QStringLiteral("element_spacing");
static const QString OSF_AttrBreakOnSentences = QStringLiteral("break_on_sentences");
static const QString OSF_AttrDialogueContinues = QStringLiteral("dialogue_continues");
static const QString OSF_AttrDialoguePagebreaks = QStringLiteral("dialogue_pagebreaks");
static const QString OSF_AttrContText = QStringLiteral("cont_text");
static const QString OSF_AttrMoreText = QStringLiteral("more_text");
static const QString OSF_AttrScenesContinue = QStringLiteral("scenes_continue");
static const QString OSF_AttrContinuedText = QStringLiteral("continued_text");
static const QString OSF_AttrNumberContinued = QStringLiteral("number_continued");
static const QString OSF_AttrSceneTimeSeparator = QStringLiteral("scene_time_separator");
static const QString OSF_AttrPageHeader = QStringLiteral("page_header");
static const QString OSF_AttrPageFooter = QStringLiteral("page_footer");
static const QString OSF_AttrHeaderAlignment = QStringLiteral("header_alignment");
static const QString OSF_AttrFooterAlignment = QStringLiteral("footer_alignment");
static const QString OSF_AttrHeaderFirstPage = QStringLiteral("header_first_page");
static const QString OSF_AttrFooterFirstPage = QStringLiteral("footer_first_page");
static const QString OSF_AttrPagesLocked = QStringLiteral("pages_locked");
static const QString OSF_AttrPageNumberStart = QStringLiteral("pagenumber_start");
static const QString OSF_AttrPageNumberMode = QStringLiteral("pagenumber_mode");
static const QString OSF_AttrRevision = QStringLiteral("revision");
static const QString OSF_AttrDocumentRevision = QStringLiteral("document_revision");
static const QString OSF_AttrRevisionMode = QStringLiteral("revision_mode");
static const QString OSF_AttrShowRevisions = QStringLiteral("show_revisions");
static const QString OSF_AttrSelectedRevisions = QStringLiteral("selected_revisions");
static const QString OSF_AttrFont = QStringLiteral("font");
static const QString OSF_AttrSize = QStringLiteral("size");
static const QString OSF_AttrName = QStringLiteral("name");
static const QString OSF_AttrBuiltin = QStringLiteral("builtin");
static const QString OSF_AttrBuiltinIndex = QStringLiteral("builtin_index");
static const QString OSF_AttrBasestyle = QStringLiteral("basestyle");
static const QString OSF_AttrBold = QStringLiteral("bold");
static const QString OSF_AttrItalic = QStringLiteral("italic");
static const QString OSF_AttrUnderline = QStringLiteral("underline");
static const QString OSF_AttrStrikethrough = QStringLiteral("strikethrough");
static const QString OSF_AttrBgcolor = QStringLiteral("bgcolor");
static const QString OSF_AttrColor = QStringLiteral("color");
static const QString OSF_AttrAlign = QStringLiteral("align");
static const QString OSF_AttrLeftIndent = QStringLiteral("leftindent");
static const QString OSF_AttrRightIndent = QStringLiteral("rightindent");
static const QString OSF_AttrSpaceBefore = QStringLiteral("spacebefore");
static const QString OSF_AttrKeepWithNext = QStringLiteral("keepwithnext");
static const QString OSF_AttrStyleEnter = QStringLiteral("style_enter");
static const QString OSF_AttrStyleTabBefore = QStringLiteral("style_tab_before");
static const QString OSF_AttrStyleTabAfter = QStringLiteral("style_tab_after");
static const QString OSF_AttrBookmark = QStringLiteral("bookmark");
static const QString OSF_AttrAllcaps = QStringLiteral("allcaps");
static const QString OSF_AttrId = QStringLiteral("id");
static const QString OSF_AttrSynopsis = QStringLiteral("synopsis");
static const QString OSF_AttrSynopsisColor = QStringLiteral("synopsis_color");
static const QString OSF_AttrDualDialogue = QStringLiteral("dualdialogue");
static const QString OSF_AttrLabel = QStringLiteral("label");
static const QString OSF_AttrRef = QStringLiteral("ref");
static const QString OSF_AttrNote = QStringLiteral("note");
static const QString OSF_AttrIndex = QStringLiteral("index");
static const QString OSF_AttrColorName = QStringLiteral("color_name");
static const QString OSF_AttrColorIndex = QStringLiteral("color_index");
static const QString OSF_AttrMark = QStringLiteral("mark");

// OSF basestyle names
static const QString OSF_StyleNormalText = QStringLiteral("Normal Text");
static const QString OSF_StyleSceneHeading = QStringLiteral("Scene Heading");
static const QString OSF_StyleAction = QStringLiteral("Action");
static const QString OSF_StyleCharacter = QStringLiteral("Character");
static const QString OSF_StyleParenthetical = QStringLiteral("Parenthetical");
static const QString OSF_StyleDialogue = QStringLiteral("Dialogue");
static const QString OSF_StyleTransition = QStringLiteral("Transition");
static const QString OSF_StyleShot = QStringLiteral("Shot");

// Font names
static const QString OSF_FontCourierScreenplay = QStringLiteral("Courier Screenplay");
static const QString OSF_FontCourierPrime = QStringLiteral("Courier Prime");

// Attribute values
static const QString OSF_AttrDocType = QStringLiteral("Open Screenplay Format document");
static const QString OSF_AttrVersionDefault = QStringLiteral("41");
static const QString OSF_ValueOne = QStringLiteral("1");
static const QString OSF_ValueZero = QStringLiteral("0");
static const QString OSF_ValueTrue = QStringLiteral("true");
static const QString OSF_ValueFalse = QStringLiteral("false");
static const QString OSF_ValueLeftAlign = QStringLiteral("left");
static const QString OSF_ValueCenterAlign = QStringLiteral("center");
static const QString OSF_ValueRightAlign = QStringLiteral("right");
static const QString OSF_ValueContText = QStringLiteral("(cont'd)");
static const QString OSF_ValueMoreText = QStringLiteral("(MORE)");
static const QString OSF_ValueContinuedText = QStringLiteral("CONTINUED");
static const QString OSF_ValueSceneTimeSeparator = QStringLiteral(" - ");
static const QString OSF_ValuePageHeader = QStringLiteral("#.");
static const QString OSF_ValueHeaderAlignment = QStringLiteral("3");
static const QString OSF_ValuePageNumberMode = QStringLiteral("1AB");
static const QString OSF_ValueDefaultRevision = QStringLiteral("0");
static const QString OSF_ValueDocumentRevision = QStringLiteral("-1");
static const QString OSF_ValueDefaultLinesPerInch = QStringLiteral("6.0");
static const QString OSF_ValueDefaultElementSpacing = QStringLiteral("1.00");
static const QString OSF_ValueDefaultFont = QStringLiteral("12");

// Character name extensions
static const QStringList OSF_CharacterExtensions = {
    QStringLiteral("(V.O.)"), QStringLiteral("(O.S.)"), QStringLiteral("(O.C.)"),
    QStringLiteral("(SUBTITLE)")
};

// Transition names
static const QStringList OSF_Transitions = {
    QStringLiteral("CUT TO:"), QStringLiteral("FADE IN:"), QStringLiteral("FADE OUT"),
    QStringLiteral("FADE TO:"), QStringLiteral("DISSOLVE TO:"), QStringLiteral("MATCH CUT TO:"),
    QStringLiteral("JUMP CUT TO:"), QStringLiteral("FADE TO BLACK")
};

// OSF basestyle names (in same order as SceneElement::Type enum for index-matching)
static QString osfBasestyle(SceneElement::Type type)
{
    switch (type) {
    case SceneElement::Heading:
        return OSF_StyleSceneHeading;
    case SceneElement::Action:
        return OSF_StyleAction;
    case SceneElement::Character:
        return OSF_StyleCharacter;
    case SceneElement::Parenthetical:
        return OSF_StyleParenthetical;
    case SceneElement::Dialogue:
        return OSF_StyleDialogue;
    case SceneElement::Transition:
        return OSF_StyleTransition;
    case SceneElement::Shot:
        return OSF_StyleShot;
    default:
        break;
    }
    return OSF_StyleAction;
}

// Alignment flag → OSF string
static QString osfAlign(Qt::Alignment align)
{
    if (align & Qt::AlignRight)
        return OSF_ValueRightAlign;
    if (align & Qt::AlignHCenter)
        return OSF_ValueCenterAlign;
    return OSF_ValueLeftAlign;
}

OsfExporter::OsfExporter(QObject *parent) : AbstractExporter(parent) { }

OsfExporter::~OsfExporter() { }

void OsfExporter::setIncludeSceneSynopsis(bool val)
{
    if (m_includeSceneSynopsis == val)
        return;
    m_includeSceneSynopsis = val;
    emit includeSceneSynopsisChanged();
}

void OsfExporter::setIncludeSceneNotes(bool val)
{
    if (m_includeSceneNotes == val)
        return;
    m_includeSceneNotes = val;
    emit includeSceneNotesChanged();
}

bool OsfExporter::doExport(QIODevice *device)
{
    const ScriteDocument *scriteDoc = ScriteDocument::instance();
    const Screenplay *screenplay = scriteDoc->screenplay();
    const Structure *structure = scriteDoc->structure();
    const ScreenplayFormat *fmt = scriteDoc->formatting();
    const ScreenplayPageLayout *pageLayout = fmt->pageLayout();

    const int nrElements = screenplay->elementCount();
    if (nrElements == 0) {
        this->error()->setErrorMessage(
                QStringLiteral("There are no scenes in the screenplay to export."));
        return false;
    }

    this->progress()->setProgressStep(1.0 / qreal(nrElements + 1));

    // OSF page dimensions are in tenths of a millimetre (0.1 mm).
    // 1 inch = 25.4 mm = 254 × 0.1 mm; Scrite stores pixels at screen DPI.
    const qreal dpi = pageLayout->resolution() > 0 ? pageLayout->resolution() : 96.0;
    auto toTenthsMM = [dpi](qreal pixels) -> int { return qRound(pixels * 254.0 / dpi); };

    QDomDocument doc;

    // ---- <document> ----
    QDomElement rootE = doc.createElement(OSF_TagDocument);
    rootE.setAttribute(OSF_AttrType, OSF_AttrDocType);
    rootE.setAttribute(OSF_AttrVersion, OSF_AttrVersionDefault);
    doc.appendChild(rootE);

    // ---- <info> ----
    {
        QDomElement infoE = doc.createElement(OSF_TagInfo);
        infoE.setAttribute(OSF_AttrUuid, QUuid::createUuid().toString(QUuid::WithoutBraces));
        infoE.setAttribute(OSF_AttrDraftUuid,
                           QUuid::createUuid().toString(QUuid::WithoutBraces));
        infoE.setAttribute(OSF_AttrPagecount, OSF_ValueZero);
        rootE.appendChild(infoE);
    }

    // ---- <settings> ----
    {
        // Derive page dimensions from the paper size enum via QPageSize so we get
        // exact values regardless of whether paperRect() has been evaluated yet.
        const QPageSize::PageSizeId qtPageSize =
                (pageLayout->paperSize() == ScreenplayPageLayout::Letter) ? QPageSize::Letter
                                                                          : QPageSize::A4;
        const QSizeF paperMM = QPageSize(qtPageSize).size(QPageSize::Millimeter);
        const QMarginsF margins = pageLayout->margins();
        const QRectF paperRect = pageLayout->paperRect();

        // Debug: raw values read from ScreenplayPageLayout at export time.
        rootE.appendChild(doc.createComment(
                QStringLiteral(" paperSize=%1 resolution=%2"
                               " paperRect=(%3x%4)"
                               " margins(T=%5 B=%6 L=%7 R=%8)"
                               " paperMM=(%9x%10) ")
                        .arg(pageLayout->paperSize() == ScreenplayPageLayout::Letter
                                     ? QStringLiteral("Letter")
                                     : QStringLiteral("A4"))
                        .arg(pageLayout->resolution())
                        .arg(paperRect.width())
                        .arg(paperRect.height())
                        .arg(margins.top())
                        .arg(margins.bottom())
                        .arg(margins.left())
                        .arg(margins.right())
                        .arg(paperMM.width())
                        .arg(paperMM.height())));

        QDomElement settingsE = doc.createElement(OSF_TagSettings);
        settingsE.setAttribute(OSF_AttrPageWidth, qRound(paperMM.width() * 10));
        settingsE.setAttribute(OSF_AttrPageHeight, qRound(paperMM.height() * 10));
        settingsE.setAttribute(OSF_AttrMarginTop, toTenthsMM(margins.top()));
        settingsE.setAttribute(OSF_AttrMarginBottom, toTenthsMM(margins.bottom()));
        settingsE.setAttribute(OSF_AttrMarginLeft, toTenthsMM(margins.left()));
        settingsE.setAttribute(OSF_AttrMarginRight, toTenthsMM(margins.right()));
        settingsE.setAttribute(OSF_AttrNormalLinesPerInch, OSF_ValueDefaultLinesPerInch);
        settingsE.setAttribute(OSF_AttrElementSpacing, OSF_ValueDefaultElementSpacing);
        settingsE.setAttribute(OSF_AttrBreakOnSentences, OSF_ValueTrue);
        settingsE.setAttribute(OSF_AttrDialogueContinues, OSF_ValueTrue);
        settingsE.setAttribute(OSF_AttrDialoguePagebreaks, OSF_ValueTrue);
        settingsE.setAttribute(OSF_AttrContText, OSF_ValueContText);
        settingsE.setAttribute(OSF_AttrMoreText, OSF_ValueMoreText);
        settingsE.setAttribute(OSF_AttrScenesContinue, OSF_ValueFalse);
        settingsE.setAttribute(OSF_AttrContinuedText, OSF_ValueContinuedText);
        settingsE.setAttribute(OSF_AttrNumberContinued, OSF_ValueTrue);
        settingsE.setAttribute(OSF_AttrSceneTimeSeparator, OSF_ValueSceneTimeSeparator);
        settingsE.setAttribute(OSF_AttrPageHeader, OSF_ValuePageHeader);
        settingsE.setAttribute(OSF_AttrPageFooter, QString());
        settingsE.setAttribute(OSF_AttrHeaderAlignment, OSF_ValueHeaderAlignment);
        settingsE.setAttribute(OSF_AttrFooterAlignment, OSF_ValueHeaderAlignment);
        settingsE.setAttribute(OSF_AttrHeaderFirstPage, OSF_ValueFalse);
        settingsE.setAttribute(OSF_AttrFooterFirstPage, OSF_ValueFalse);
        settingsE.setAttribute(OSF_AttrPagesLocked, OSF_ValueFalse);
        settingsE.setAttribute(OSF_AttrPageNumberStart, OSF_ValueOne);
        settingsE.setAttribute(OSF_AttrPageNumberMode, OSF_ValuePageNumberMode);
        settingsE.setAttribute(OSF_AttrRevision, OSF_ValueDefaultRevision);
        settingsE.setAttribute(OSF_AttrDocumentRevision, OSF_ValueDocumentRevision);
        settingsE.setAttribute(OSF_AttrRevisionMode, OSF_ValueFalse);
        settingsE.setAttribute(OSF_AttrShowRevisions, QStringLiteral("all"));
        settingsE.setAttribute(OSF_AttrSelectedRevisions, OSF_ValueDefaultRevision);
        rootE.appendChild(settingsE);
    }

    // ---- <styles> ----
    // Editor-behavior attributes (Enter/Tab navigation) follow the OSF spec defaults.
    // Per-format attributes (font, size, bold, allcaps, align, indents, spacebefore)
    // are derived from ScreenplayFormat.
    {
        struct StyleSpec
        {
            SceneElement::Type type;
            const char *basestyle;
            const char *baseOf; // basestyle= on the <style> element itself
            const char *styleEnter;
            const char *styleTabBefore;
            const char *styleTabAfter;
            bool keepwithnext;
        };

        static const StyleSpec specs[] = {
            { SceneElement::Heading, "Scene Heading", "Normal Text", "Action", nullptr, "Action",
              true },
            { SceneElement::Action, "Action", "Normal Text", nullptr, "Character", nullptr, false },
            { SceneElement::Character, "Character", "Normal Text", "Dialogue", "Action",
              "Parenthetical", true },
            { SceneElement::Parenthetical, "Parenthetical", "Normal Text", "Dialogue", "Dialogue",
              "Dialogue", true },
            { SceneElement::Dialogue, "Dialogue", "Normal Text", "Action", "Parenthetical",
              "Parenthetical", false },
            { SceneElement::Transition, "Transition", "Normal Text", "Scene Heading", nullptr,
              "Action", false },
            { SceneElement::Shot, "Shot", "Normal Text", "Action", nullptr, "Action", true },
        };

        // Normal Text base style (not a SceneElement type — emit from default font).
        QDomElement stylesE = doc.createElement(OSF_TagStyles);

        // leftMargin()/rightMargin() on SceneElementFormat are fractions of content width
        // (stored as (paragraphLeft - pageLeft) / contentWidth). Multiply by contentWidth
        // in pixels before converting to 0.1mm to get the correct absolute indent.
        const qreal contentWidthPx = pageLayout->contentWidth();

        auto addAttrIf = [](QDomElement &e, const char *attr, const char *val) {
            if (val && *val)
                e.setAttribute(QLatin1String(attr), QLatin1String(val));
        };

        {
            QDomElement normalE = doc.createElement(OSF_TagStyle);
            normalE.setAttribute(OSF_AttrName, OSF_StyleNormalText);
            normalE.setAttribute(OSF_AttrBuiltin, OSF_ValueOne);
            normalE.setAttribute(OSF_AttrBuiltinIndex, OSF_ValueZero);
            normalE.setAttribute(OSF_AttrFont,
                                 fmt->defaultFont().family().isEmpty()
                                         ? OSF_FontCourierScreenplay
                                         : fmt->defaultFont().family());
            normalE.setAttribute(OSF_AttrSize,
                                 QString::number(fmt->defaultFont().pointSize() > 0
                                                         ? fmt->defaultFont().pointSize()
                                                         : 12));
            stylesE.appendChild(normalE);
        }

        for (int si = 0; si < 7; ++si) {
            const StyleSpec &spec = specs[si];
            const SceneElementFormat *ef = fmt->elementFormat(spec.type);

            QDomElement styleE = doc.createElement(OSF_TagStyle);
            styleE.setAttribute(OSF_AttrName, QLatin1String(spec.basestyle));
            styleE.setAttribute(OSF_AttrBuiltin, OSF_ValueOne);
            styleE.setAttribute(OSF_AttrBuiltinIndex, QString::number(si + 1));
            styleE.setAttribute(OSF_AttrBasestyle, QLatin1String(spec.baseOf));

            addAttrIf(styleE, "style_enter", spec.styleEnter);
            addAttrIf(styleE, "style_tab_before", spec.styleTabBefore);
            addAttrIf(styleE, "style_tab_after", spec.styleTabAfter);

            const QFont font = ef->font();
            styleE.setAttribute(OSF_AttrFont,
                                font.family().isEmpty() ? OSF_FontCourierScreenplay
                                                        : font.family());
            const int pointSize = ef->fontPointSize() > 0 ? ef->fontPointSize() : 12;
            styleE.setAttribute(OSF_AttrSize, QString::number(pointSize));

            if (ef->fontBold() == SceneElementFormat::Set)
                styleE.setAttribute(OSF_AttrBold, OSF_ValueOne);
            if (ef->fontItalics() == SceneElementFormat::Set)
                styleE.setAttribute(OSF_AttrItalic, OSF_ValueOne);
            if (ef->fontUnderline() == SceneElementFormat::Set)
                styleE.setAttribute(OSF_AttrUnderline, OSF_ValueOne);
            if (ef->fontCapitalization() == QFont::AllUppercase)
                styleE.setAttribute(OSF_AttrAllcaps, OSF_ValueOne);
            if (spec.keepwithnext)
                styleE.setAttribute(OSF_AttrKeepWithNext, OSF_ValueOne);

            const Qt::Alignment align = ef->textAlignment();
            if (align & (Qt::AlignRight | Qt::AlignHCenter))
                styleE.setAttribute(OSF_AttrAlign, osfAlign(align));

            const int leftIndent = toTenthsMM(ef->leftMargin() * contentWidthPx);
            const int rightIndent = toTenthsMM(ef->rightMargin() * contentWidthPx);
            if (leftIndent > 0)
                styleE.setAttribute(OSF_AttrLeftIndent, QString::number(leftIndent));
            if (rightIndent > 0)
                styleE.setAttribute(OSF_AttrRightIndent, QString::number(rightIndent));

            const qreal spaceBefore = ef->lineSpacingBefore();
            if (spaceBefore > 0.0)
                styleE.setAttribute(OSF_AttrSpaceBefore,
                                    QString::number(spaceBefore, 'f', 1));

            stylesE.appendChild(styleE);
        }

        {
            QDomElement headerStyleE = doc.createElement(OSF_TagHeaderStyle);
            headerStyleE.setAttribute(OSF_AttrBasestyle, OSF_StyleNormalText);
            stylesE.appendChild(headerStyleE);

            QDomElement footerStyleE = doc.createElement(OSF_TagFooterStyle);
            footerStyleE.setAttribute(OSF_AttrBasestyle, OSF_StyleNormalText);
            stylesE.appendChild(footerStyleE);
        }

        rootE.appendChild(stylesE);
    }

    // ---- <paragraphs> ----
    {
        QDomElement paragraphsE = doc.createElement(OSF_TagParagraphs);

        // Helper: emit one or more <text> children for a para, splitting on format ranges.
        auto addTextRuns = [&](QDomElement &paraE, const QString &text,
                               const QVector<QTextLayout::FormatRange> &formats) {
            auto createText = [&](const QString &snippet, const QTextLayout::FormatRange *fmt) {
                QDomElement textE = doc.createElement(OSF_TagText);
                textE.setAttribute(OSF_AttrFont, OSF_FontCourierPrime);
                if (fmt) {
                    if (fmt->format.hasProperty(QTextFormat::FontWeight)
                        && fmt->format.fontWeight() == QFont::Bold)
                        textE.setAttribute(OSF_AttrBold, OSF_ValueOne);
                    if (fmt->format.hasProperty(QTextFormat::FontItalic)
                        && fmt->format.fontItalic())
                        textE.setAttribute(OSF_AttrItalic, OSF_ValueOne);
                    if (fmt->format.hasProperty(QTextFormat::TextUnderlineStyle)
                        && fmt->format.fontUnderline())
                        textE.setAttribute(OSF_AttrUnderline, OSF_ValueOne);
                    if (fmt->format.hasProperty(QTextFormat::FontStrikeOut)
                        && fmt->format.fontStrikeOut())
                        textE.setAttribute(OSF_AttrStrikethrough, OSF_ValueOne);
                    if (fmt->format.hasProperty(QTextFormat::BackgroundBrush))
                        textE.setAttribute(OSF_AttrBgcolor,
                                           fmt->format.background().color().name());
                    if (fmt->format.hasProperty(QTextFormat::ForegroundBrush))
                        textE.setAttribute(OSF_AttrColor,
                                           fmt->format.foreground().color().name());
                }
                textE.appendChild(doc.createTextNode(snippet));
                paraE.appendChild(textE);
            };

            if (formats.isEmpty()) {
                createText(text, nullptr);
                return;
            }

            // Emit spans in format order; fill gaps with unstyled runs.
            int pos = 0;
            for (const QTextLayout::FormatRange &range : formats) {
                if (range.start > pos)
                    createText(text.mid(pos, range.start - pos), nullptr);
                const QString snippet = text.mid(range.start, range.length);
                if (!snippet.isEmpty())
                    createText(snippet, &range);
                pos = range.start + range.length;
            }
            if (pos < text.length())
                createText(text.mid(pos), nullptr);
        };

        for (int i = 0; i < nrElements; ++i) {
            const ScreenplayElement *element = screenplay->elementAt(i);
            if (element->elementType() != ScreenplayElement::SceneElementType) {
                this->progress()->tick();
                continue;
            }

            const Scene *scene = element->scene();
            const SceneHeading *heading = scene->heading();

            // Scene heading para
            if (heading->isEnabled() || scene->hasSynopsis()) {
                QDomElement paraE = doc.createElement(OSF_TagPara);

                // Scrite-extended attribute: stable scene ID for round-trip fidelity.
                paraE.setAttribute(OSF_AttrId, scene->id());

                if (m_includeSceneSynopsis && scene->hasSynopsis()) {
                    // Replace newlines with OSF's &#xA; encoding for attribute values.
                    const QString synopsis =
                            scene->synopsis().replace(QLatin1Char('\n'), QStringLiteral("&#xA;"));
                    paraE.setAttribute(OSF_AttrSynopsis, synopsis);
                    paraE.setAttribute(OSF_AttrSynopsisColor, scene->color().name());
                }

                if (m_includeSceneNotes && scene->notes()->noteCount() > 0) {
                    // Concatenate text notes into the single note attribute.
                    QStringList noteParts;
                    const Notes *notes = scene->notes();
                    for (int ni = 0; ni < notes->noteCount(); ++ni) {
                        const Note *note = notes->noteAt(ni);
                        if (note->type() == Note::TextNoteType && note->content().isString()) {
                            noteParts << note->content().toString();
                        }
                    }
                    if (!noteParts.isEmpty()) {
                        const QString noteText =
                                noteParts.join(QStringLiteral("\n"))
                                        .replace(QLatin1Char('\n'), QStringLiteral("&#xA;"));
                        paraE.setAttribute(OSF_AttrNote, noteText);
                    }
                }

                QDomElement styleE = doc.createElement(OSF_TagStyle);
                styleE.setAttribute(OSF_AttrBasestyle, OSF_StyleSceneHeading);
                paraE.appendChild(styleE);

                if (heading->isEnabled())
                    addTextRuns(paraE, heading->text(), {});

                paragraphsE.appendChild(paraE);
            }

            // Scene element paras
            const int nrSceneElements = scene->elementCount();
            for (int j = 0; j < nrSceneElements; ++j) {
                const SceneElement *sceneElement = scene->elementAt(j);

                QDomElement paraE = doc.createElement(OSF_TagPara);

                QDomElement styleE = doc.createElement(OSF_TagStyle);
                styleE.setAttribute(OSF_AttrBasestyle,
                                    osfBasestyle(sceneElement->type()));
                if (sceneElement->type() == SceneElement::Character) {
                    SceneDualDialogue *dualDialogue = scene->findContainingDualDialogue(
                            const_cast<SceneElement *>(sceneElement));
                    if (dualDialogue && dualDialogue->leftCharacter() == sceneElement)
                        styleE.setAttribute(OSF_AttrDualDialogue, OSF_ValueOne);
                }
                paraE.appendChild(styleE);

                QString text = sceneElement->formattedText();

                // OSF stores parentheticals without surrounding parentheses.
                if (sceneElement->type() == SceneElement::Parenthetical) {
                    if (text.startsWith(QLatin1Char('(')))
                        text = text.mid(1);
                    if (text.endsWith(QLatin1Char(')')))
                        text.chop(1);
                    text = text.trimmed();
                }

                addTextRuns(paraE, text, sceneElement->textFormats());
                paragraphsE.appendChild(paraE);
            }

            this->progress()->tick();
        }

        rootE.appendChild(paragraphsE);
    }

    // ---- <titlepage> ----
    {
        QDomElement titlepageE = doc.createElement(OSF_TagTitlepage);

        auto addField = [&](const QString &bookmark, const QString &value) {
            if (value.isEmpty())
                return;
            QDomElement paraE = doc.createElement(OSF_TagPara);
            paraE.setAttribute(OSF_AttrBookmark, bookmark);
            QDomElement styleE = doc.createElement(OSF_TagStyle);
            styleE.setAttribute(OSF_AttrBasestyle, OSF_StyleNormalText);
            styleE.setAttribute(OSF_AttrAlign, OSF_ValueCenterAlign);
            paraE.appendChild(styleE);
            QDomElement textE = doc.createElement(OSF_TagText);
            textE.setAttribute(OSF_AttrFont, OSF_FontCourierPrime);
            textE.appendChild(doc.createTextNode(value));
            paraE.appendChild(textE);
            titlepageE.appendChild(paraE);
        };

        addField(QStringLiteral("Title"), screenplay->title());
        addField(QStringLiteral("Subtitle"), screenplay->subtitle());
        addField(QStringLiteral("Author"), screenplay->author());
        addField(QStringLiteral("BasedOn"), screenplay->basedOn());
        addField(QStringLiteral("Logline"), screenplay->logline());
        addField(QStringLiteral("Contact"), screenplay->contact());
        addField(QStringLiteral("Address"), screenplay->address());
        addField(QStringLiteral("PhoneNumber"), screenplay->phoneNumber());
        addField(QStringLiteral("Email"), screenplay->email());
        addField(QStringLiteral("Website"), screenplay->website());
        addField(QStringLiteral("Version"), screenplay->version());

        rootE.appendChild(titlepageE);
    }

    // ---- <a_pages/> <ab_pages/> <spelling/> ---- (required placeholders)
    rootE.appendChild(doc.createElement(OSF_TagAPages));
    rootE.appendChild(doc.createElement(OSF_TagAbPages));
    rootE.appendChild(doc.createElement(OSF_TagSpelling));

    // ---- <lists> ----
    {
        QDomElement listsE = doc.createElement(OSF_TagLists);

        // Characters
        {
            QDomElement charsE = doc.createElement(OSF_TagCharacters);
            const QStringList names = structure->allCharacterNames();
            for (const QString &name : names) {
                QDomElement charE = doc.createElement(OSF_TagCharacter);
                charE.setAttribute(OSF_AttrName, name);
                charsE.appendChild(charE);
            }
            listsE.appendChild(charsE);
        }

        // Locations — gather from scene headings
        {
            QDomElement locsE = doc.createElement(OSF_TagLocations);
            QStringList locations;
            for (int i = 0; i < screenplay->elementCount(); ++i) {
                const ScreenplayElement *el = screenplay->elementAt(i);
                if (el->elementType() != ScreenplayElement::SceneElementType)
                    continue;
                const SceneHeading *h = el->scene()->heading();
                if (h->isEnabled() && !h->location().isEmpty()
                    && !locations.contains(h->location()))
                    locations << h->location();
            }
            std::sort(locations.begin(), locations.end());
            for (const QString &loc : std::as_const(locations)) {
                QDomElement locE = doc.createElement(OSF_TagLocation);
                locE.setAttribute(OSF_AttrName, loc);
                locsE.appendChild(locE);
            }
            listsE.appendChild(locsE);
        }

        // Scene intros (location types)
        {
            QDomElement introsE = doc.createElement(OSF_TagSceneIntros);
            const QStringList types = Structure::standardLocationTypes();
            for (const QString &t : types) {
                QDomElement introE = doc.createElement(OSF_TagSceneIntro);
                introE.setAttribute(OSF_AttrName, t);
                introsE.appendChild(introE);
            }
            listsE.appendChild(introsE);
        }

        // Scene times (moments)
        {
            QDomElement timesE = doc.createElement(OSF_TagSceneTimes);
            const QStringList moments = Structure::standardMoments();
            for (const QString &m : moments) {
                QDomElement timeE = doc.createElement(OSF_TagSceneTime);
                timeE.setAttribute(OSF_AttrName, m);
                timesE.appendChild(timeE);
            }
            listsE.appendChild(timesE);
        }

        // Character name extensions
        {
            QDomElement extsE = doc.createElement(OSF_TagExtensions);
            for (const QString &ext : OSF_CharacterExtensions) {
                QDomElement extE = doc.createElement(OSF_TagExtension);
                extE.setAttribute(OSF_AttrName, ext);
                extsE.appendChild(extE);
            }
            listsE.appendChild(extsE);
        }

        // Transitions
        {
            QDomElement transitionsE = doc.createElement(OSF_TagTransitions);
            for (const QString &t : OSF_Transitions) {
                QDomElement transE = doc.createElement(OSF_TagTransition);
                transE.setAttribute(OSF_AttrName, t);
                transitionsE.appendChild(transE);
            }
            listsE.appendChild(transitionsE);
        }

        // Revision colours (OSF-required, using spec defaults)
        {
            struct RevColor
            {
                const char *name;
                int index;
                const char *colorName;
                const char *mark;
            };
            static const RevColor revColors[] = {
                { "White", 0, "White", "" },    { "Blue", 1, "Blue", "*" },
                { "Pink", 2, "Pink", "*" },     { "Yellow", 3, "Yellow", "*" },
                { "Green", 4, "Green", "*" },   { "Goldenrod", 5, "Goldenrod", "*" },
                { "Buff", 6, "Buff", "*" },     { "Salmon", 7, "Salmon", "*" },
                { "Cherry", 8, "Cherry", "*" }, { "Tan", 9, "Tan", "*" },
            };
            QDomElement revColorsE = doc.createElement(OSF_TagRevisionColors);
            for (const RevColor &rc : revColors) {
                QDomElement rcE = doc.createElement(OSF_TagRevisionColor);
                rcE.setAttribute(OSF_AttrName, QLatin1String(rc.name));
                rcE.setAttribute(OSF_AttrIndex, rc.index);
                rcE.setAttribute(OSF_AttrColorName, QLatin1String(rc.colorName));
                rcE.setAttribute(OSF_AttrColorIndex, rc.index);
                rcE.setAttribute(OSF_AttrMark, QLatin1String(rc.mark));
                revColorsE.appendChild(rcE);
            }
            listsE.appendChild(revColorsE);
        }

        listsE.appendChild(doc.createElement(OSF_TagTagCategories));
        listsE.appendChild(doc.createElement(OSF_TagHighlightRules));
        rootE.appendChild(listsE);
    }

    // ---- <notes> (Scrite extension) ----
    {
        QDomElement notesContainerE = doc.createElement(OSF_TagNotes);
        bool hasAnyNote = false;

        auto addNoteElement = [&](const QString &type, const QString &id, const QString &ref,
                                  const QString &label, const QString &text) {
            QDomElement noteE = doc.createElement(OSF_TagNote);
            noteE.setAttribute(OSF_AttrType, type);
            if (!id.isEmpty())
                noteE.setAttribute(OSF_AttrId, id);
            if (!ref.isEmpty())
                noteE.setAttribute(OSF_AttrRef, ref);
            noteE.setAttribute(OSF_AttrLabel, label);
            noteE.appendChild(doc.createTextNode(text));
            notesContainerE.appendChild(noteE);
            hasAnyNote = true;
        };

        auto exportNotes = [&](Notes *notes, const QString &type, const QString &id,
                               const QString &ref) {
            if (!notes)
                return;
            for (int ni = 0; ni < notes->noteCount(); ++ni) {
                const Note *note = notes->noteAt(ni);
                if (note->type() != Note::TextNoteType || !note->content().isString())
                    continue;
                addNoteElement(type, id, ref, note->title(), note->content().toString());
            }
        };

        // Story-level notes
        exportNotes(structure->notes(), QStringLiteral("story"), QString(), QString());

        // Scene notes
        for (int i = 0; i < screenplay->elementCount(); ++i) {
            const ScreenplayElement *el = screenplay->elementAt(i);
            if (el->elementType() != ScreenplayElement::SceneElementType)
                continue;
            const Scene *scene = el->scene();
            exportNotes(scene->notes(), QStringLiteral("scene"), scene->id(),
                        scene->heading()->text());
        }

        // Character notes
        for (const QString &name : structure->allCharacterNames()) {
            Character *character = structure->findCharacter(name);
            if (character)
                exportNotes(character->notes(), QStringLiteral("character"), QString(), name);
        }

        if (hasAnyNote) {
            rootE.appendChild(
                    doc.createComment(QStringLiteral(" Scrite extended notes — safe to ignore "
                                                     "for other OSF readers ")));
            rootE.appendChild(notesContainerE);
        }
    }

    // ---- Write output ----
    QTextStream ts(device);
    ts.setEncoding(QStringConverter::Utf8);
    ts << QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    ts << doc.toString(2);
    ts.flush();

    return true;
}
