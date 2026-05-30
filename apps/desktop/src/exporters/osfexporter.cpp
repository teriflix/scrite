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

// OSF basestyle names (in same order as SceneElement::Type enum for index-matching)
static QString osfBasestyle(SceneElement::Type type)
{
    switch (type) {
    case SceneElement::Heading:
        return QStringLiteral("Scene Heading");
    case SceneElement::Action:
        return QStringLiteral("Action");
    case SceneElement::Character:
        return QStringLiteral("Character");
    case SceneElement::Parenthetical:
        return QStringLiteral("Parenthetical");
    case SceneElement::Dialogue:
        return QStringLiteral("Dialogue");
    case SceneElement::Transition:
        return QStringLiteral("Transition");
    case SceneElement::Shot:
        return QStringLiteral("Shot");
    default:
        break;
    }
    return QStringLiteral("Action");
}

// Alignment flag → OSF string
static QString osfAlign(Qt::Alignment align)
{
    if (align & Qt::AlignRight)
        return QStringLiteral("right");
    if (align & Qt::AlignHCenter)
        return QStringLiteral("center");
    return QStringLiteral("left");
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
    QDomElement rootE = doc.createElement(QStringLiteral("document"));
    rootE.setAttribute(QStringLiteral("type"), QStringLiteral("Open Screenplay Format document"));
    rootE.setAttribute(QStringLiteral("version"), QStringLiteral("41"));
    doc.appendChild(rootE);

    // ---- <info> ----
    {
        QDomElement infoE = doc.createElement(QStringLiteral("info"));
        infoE.setAttribute(QStringLiteral("uuid"),
                           QUuid::createUuid().toString(QUuid::WithoutBraces));
        infoE.setAttribute(QStringLiteral("draft_uuid"),
                           QUuid::createUuid().toString(QUuid::WithoutBraces));
        infoE.setAttribute(QStringLiteral("pagecount"), QStringLiteral("0"));
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

        QDomElement settingsE = doc.createElement(QStringLiteral("settings"));
        settingsE.setAttribute(QStringLiteral("page_width"), qRound(paperMM.width() * 10));
        settingsE.setAttribute(QStringLiteral("page_height"), qRound(paperMM.height() * 10));
        settingsE.setAttribute(QStringLiteral("margin_top"), toTenthsMM(margins.top()));
        settingsE.setAttribute(QStringLiteral("margin_bottom"), toTenthsMM(margins.bottom()));
        settingsE.setAttribute(QStringLiteral("margin_left"), toTenthsMM(margins.left()));
        settingsE.setAttribute(QStringLiteral("margin_right"), toTenthsMM(margins.right()));
        settingsE.setAttribute(QStringLiteral("normal_linesperinch"), QStringLiteral("6.0"));
        settingsE.setAttribute(QStringLiteral("element_spacing"), QStringLiteral("1.00"));
        settingsE.setAttribute(QStringLiteral("break_on_sentences"), QStringLiteral("true"));
        settingsE.setAttribute(QStringLiteral("dialogue_continues"), QStringLiteral("true"));
        settingsE.setAttribute(QStringLiteral("dialogue_pagebreaks"), QStringLiteral("true"));
        settingsE.setAttribute(QStringLiteral("cont_text"), QStringLiteral("(cont'd)"));
        settingsE.setAttribute(QStringLiteral("more_text"), QStringLiteral("(MORE)"));
        settingsE.setAttribute(QStringLiteral("scenes_continue"), QStringLiteral("false"));
        settingsE.setAttribute(QStringLiteral("continued_text"), QStringLiteral("CONTINUED"));
        settingsE.setAttribute(QStringLiteral("number_continued"), QStringLiteral("true"));
        settingsE.setAttribute(QStringLiteral("scene_time_separator"), QStringLiteral(" - "));
        settingsE.setAttribute(QStringLiteral("page_header"), QStringLiteral("#."));
        settingsE.setAttribute(QStringLiteral("page_footer"), QString());
        settingsE.setAttribute(QStringLiteral("header_alignment"), QStringLiteral("3"));
        settingsE.setAttribute(QStringLiteral("footer_alignment"), QStringLiteral("3"));
        settingsE.setAttribute(QStringLiteral("header_first_page"), QStringLiteral("false"));
        settingsE.setAttribute(QStringLiteral("footer_first_page"), QStringLiteral("false"));
        settingsE.setAttribute(QStringLiteral("pages_locked"), QStringLiteral("false"));
        settingsE.setAttribute(QStringLiteral("pagenumber_start"), QStringLiteral("1"));
        settingsE.setAttribute(QStringLiteral("pagenumber_mode"), QStringLiteral("1AB"));
        settingsE.setAttribute(QStringLiteral("revision"), QStringLiteral("0"));
        settingsE.setAttribute(QStringLiteral("document_revision"), QStringLiteral("-1"));
        settingsE.setAttribute(QStringLiteral("revision_mode"), QStringLiteral("false"));
        settingsE.setAttribute(QStringLiteral("show_revisions"), QStringLiteral("all"));
        settingsE.setAttribute(QStringLiteral("selected_revisions"), QStringLiteral("0"));
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
            const char *baseOf;      // basestyle= on the <style> element itself
            const char *styleEnter;
            const char *styleTabBefore;
            const char *styleTabAfter;
            bool keepwithnext;
        };

        static const StyleSpec specs[] = {
            { SceneElement::Heading,      "Scene Heading", "Normal Text",
              "Action",       nullptr,       "Action",        true  },
            { SceneElement::Action,       "Action",        "Normal Text",
              nullptr,        "Character",   nullptr,         false },
            { SceneElement::Character,    "Character",     "Normal Text",
              "Dialogue",     "Action",      "Parenthetical", true  },
            { SceneElement::Parenthetical,"Parenthetical", "Normal Text",
              "Dialogue",     "Dialogue",    "Dialogue",      true  },
            { SceneElement::Dialogue,     "Dialogue",      "Normal Text",
              "Action",       "Parenthetical","Parenthetical",false },
            { SceneElement::Transition,   "Transition",    "Normal Text",
              "Scene Heading",nullptr,       "Action",        false },
            { SceneElement::Shot,         "Shot",          "Normal Text",
              "Action",       nullptr,       "Action",        true  },
        };

        // Normal Text base style (not a SceneElement type — emit from default font).
        QDomElement stylesE = doc.createElement(QStringLiteral("styles"));

        // leftMargin()/rightMargin() on SceneElementFormat are fractions of content width
        // (stored as (paragraphLeft - pageLeft) / contentWidth). Multiply by contentWidth
        // in pixels before converting to 0.1mm to get the correct absolute indent.
        const qreal contentWidthPx = pageLayout->contentWidth();

        auto addAttrIf = [](QDomElement &e, const char *attr, const char *val) {
            if (val && *val)
                e.setAttribute(QLatin1String(attr), QLatin1String(val));
        };

        {
            QDomElement normalE = doc.createElement(QStringLiteral("style"));
            normalE.setAttribute(QStringLiteral("name"), QStringLiteral("Normal Text"));
            normalE.setAttribute(QStringLiteral("builtin"), QStringLiteral("1"));
            normalE.setAttribute(QStringLiteral("builtin_index"), QStringLiteral("0"));
            normalE.setAttribute(QStringLiteral("font"),
                                 fmt->defaultFont().family().isEmpty()
                                         ? QStringLiteral("Courier Screenplay")
                                         : fmt->defaultFont().family());
            normalE.setAttribute(QStringLiteral("size"),
                                 QString::number(fmt->defaultFont().pointSize() > 0
                                                         ? fmt->defaultFont().pointSize()
                                                         : 12));
            stylesE.appendChild(normalE);
        }

        for (int si = 0; si < 7; ++si) {
            const StyleSpec &spec = specs[si];
            const SceneElementFormat *ef = fmt->elementFormat(spec.type);

            QDomElement styleE = doc.createElement(QStringLiteral("style"));
            styleE.setAttribute(QStringLiteral("name"), QLatin1String(spec.basestyle));
            styleE.setAttribute(QStringLiteral("builtin"), QStringLiteral("1"));
            styleE.setAttribute(QStringLiteral("builtin_index"), QString::number(si + 1));
            styleE.setAttribute(QStringLiteral("basestyle"), QLatin1String(spec.baseOf));

            addAttrIf(styleE, "style_enter", spec.styleEnter);
            addAttrIf(styleE, "style_tab_before", spec.styleTabBefore);
            addAttrIf(styleE, "style_tab_after", spec.styleTabAfter);

            const QFont font = ef->font();
            styleE.setAttribute(QStringLiteral("font"),
                                font.family().isEmpty() ? QStringLiteral("Courier Screenplay")
                                                        : font.family());
            const int pointSize = ef->fontPointSize() > 0 ? ef->fontPointSize() : 12;
            styleE.setAttribute(QStringLiteral("size"), QString::number(pointSize));

            if (ef->fontBold() == SceneElementFormat::Set)
                styleE.setAttribute(QStringLiteral("bold"), QStringLiteral("1"));
            if (ef->fontItalics() == SceneElementFormat::Set)
                styleE.setAttribute(QStringLiteral("italic"), QStringLiteral("1"));
            if (ef->fontUnderline() == SceneElementFormat::Set)
                styleE.setAttribute(QStringLiteral("underline"), QStringLiteral("1"));
            if (ef->fontCapitalization() == QFont::AllUppercase)
                styleE.setAttribute(QStringLiteral("allcaps"), QStringLiteral("1"));
            if (spec.keepwithnext)
                styleE.setAttribute(QStringLiteral("keepwithnext"), QStringLiteral("1"));

            const Qt::Alignment align = ef->textAlignment();
            if (align & (Qt::AlignRight | Qt::AlignHCenter))
                styleE.setAttribute(QStringLiteral("align"), osfAlign(align));

            const int leftIndent = toTenthsMM(ef->leftMargin() * contentWidthPx);
            const int rightIndent = toTenthsMM(ef->rightMargin() * contentWidthPx);
            if (leftIndent > 0)
                styleE.setAttribute(QStringLiteral("leftindent"), QString::number(leftIndent));
            if (rightIndent > 0)
                styleE.setAttribute(QStringLiteral("rightindent"), QString::number(rightIndent));

            const qreal spaceBefore = ef->lineSpacingBefore();
            if (spaceBefore > 0.0)
                styleE.setAttribute(QStringLiteral("spacebefore"),
                                    QString::number(spaceBefore, 'f', 1));

            stylesE.appendChild(styleE);
        }

        {
            QDomElement headerStyleE = doc.createElement(QStringLiteral("header_style"));
            headerStyleE.setAttribute(QStringLiteral("basestyle"), QStringLiteral("Normal Text"));
            stylesE.appendChild(headerStyleE);

            QDomElement footerStyleE = doc.createElement(QStringLiteral("footer_style"));
            footerStyleE.setAttribute(QStringLiteral("basestyle"), QStringLiteral("Normal Text"));
            stylesE.appendChild(footerStyleE);
        }

        rootE.appendChild(stylesE);
    }

    // ---- <paragraphs> ----
    {
        QDomElement paragraphsE = doc.createElement(QStringLiteral("paragraphs"));

        // Helper: emit one or more <text> children for a para, splitting on format ranges.
        auto addTextRuns = [&](QDomElement &paraE, const QString &text,
                               const QVector<QTextLayout::FormatRange> &formats) {
            auto createText = [&](const QString &snippet,
                                  const QTextLayout::FormatRange *fmt) {
                QDomElement textE = doc.createElement(QStringLiteral("text"));
                textE.setAttribute(QStringLiteral("font"), QStringLiteral("Courier Prime"));
                if (fmt) {
                    if (fmt->format.hasProperty(QTextFormat::FontWeight)
                        && fmt->format.fontWeight() == QFont::Bold)
                        textE.setAttribute(QStringLiteral("bold"), QStringLiteral("1"));
                    if (fmt->format.hasProperty(QTextFormat::FontItalic)
                        && fmt->format.fontItalic())
                        textE.setAttribute(QStringLiteral("italic"), QStringLiteral("1"));
                    if (fmt->format.hasProperty(QTextFormat::TextUnderlineStyle)
                        && fmt->format.fontUnderline())
                        textE.setAttribute(QStringLiteral("underline"), QStringLiteral("1"));
                    if (fmt->format.hasProperty(QTextFormat::FontStrikeOut)
                        && fmt->format.fontStrikeOut())
                        textE.setAttribute(QStringLiteral("strikethrough"),
                                           QStringLiteral("1"));
                    if (fmt->format.hasProperty(QTextFormat::BackgroundBrush))
                        textE.setAttribute(QStringLiteral("bgcolor"),
                                           fmt->format.background().color().name());
                    if (fmt->format.hasProperty(QTextFormat::ForegroundBrush))
                        textE.setAttribute(QStringLiteral("color"),
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
                QDomElement paraE = doc.createElement(QStringLiteral("para"));

                // Scrite-extended attribute: stable scene ID for round-trip fidelity.
                paraE.setAttribute(QStringLiteral("id"), scene->id());

                if (m_includeSceneSynopsis && scene->hasSynopsis()) {
                    // Replace newlines with OSF's &#xA; encoding for attribute values.
                    const QString synopsis = scene->synopsis()
                                                     .replace(QLatin1Char('\n'),
                                                              QStringLiteral("&#xA;"));
                    paraE.setAttribute(QStringLiteral("synopsis"), synopsis);
                    paraE.setAttribute(QStringLiteral("synopsis_color"),
                                       scene->color().name());
                }

                if (m_includeSceneNotes && scene->notes()->noteCount() > 0) {
                    // Concatenate text notes into the single note attribute.
                    QStringList noteParts;
                    const Notes *notes = scene->notes();
                    for (int ni = 0; ni < notes->noteCount(); ++ni) {
                        const Note *note = notes->noteAt(ni);
                        if (note->type() == Note::TextNoteType
                            && note->content().isString()) {
                            noteParts << note->content().toString();
                        }
                    }
                    if (!noteParts.isEmpty()) {
                        const QString noteText = noteParts.join(QStringLiteral("\n"))
                                                         .replace(QLatin1Char('\n'),
                                                                  QStringLiteral("&#xA;"));
                        paraE.setAttribute(QStringLiteral("note"), noteText);
                    }
                }

                QDomElement styleE = doc.createElement(QStringLiteral("style"));
                styleE.setAttribute(QStringLiteral("basestyle"),
                                    QStringLiteral("Scene Heading"));
                paraE.appendChild(styleE);

                if (heading->isEnabled())
                    addTextRuns(paraE, heading->text(), {});

                paragraphsE.appendChild(paraE);
            }

            // Scene element paras
            const int nrSceneElements = scene->elementCount();
            for (int j = 0; j < nrSceneElements; ++j) {
                const SceneElement *sceneElement = scene->elementAt(j);

                QDomElement paraE = doc.createElement(QStringLiteral("para"));

                QDomElement styleE = doc.createElement(QStringLiteral("style"));
                styleE.setAttribute(QStringLiteral("basestyle"),
                                    osfBasestyle(sceneElement->type()));
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
        QDomElement titlepageE = doc.createElement(QStringLiteral("titlepage"));

        auto addField = [&](const QString &bookmark, const QString &value) {
            if (value.isEmpty())
                return;
            QDomElement paraE = doc.createElement(QStringLiteral("para"));
            paraE.setAttribute(QStringLiteral("bookmark"), bookmark);
            QDomElement styleE = doc.createElement(QStringLiteral("style"));
            styleE.setAttribute(QStringLiteral("basestyle"), QStringLiteral("Normal Text"));
            styleE.setAttribute(QStringLiteral("align"), QStringLiteral("center"));
            paraE.appendChild(styleE);
            QDomElement textE = doc.createElement(QStringLiteral("text"));
            textE.setAttribute(QStringLiteral("font"), QStringLiteral("Courier Prime"));
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
    rootE.appendChild(doc.createElement(QStringLiteral("a_pages")));
    rootE.appendChild(doc.createElement(QStringLiteral("ab_pages")));
    rootE.appendChild(doc.createElement(QStringLiteral("spelling")));

    // ---- <lists> ----
    {
        QDomElement listsE = doc.createElement(QStringLiteral("lists"));

        // Characters
        {
            QDomElement charsE = doc.createElement(QStringLiteral("characters"));
            const QStringList names = structure->allCharacterNames();
            for (const QString &name : names) {
                QDomElement charE = doc.createElement(QStringLiteral("character"));
                charE.setAttribute(QStringLiteral("name"), name);
                charsE.appendChild(charE);
            }
            listsE.appendChild(charsE);
        }

        // Locations — gather from scene headings
        {
            QDomElement locsE = doc.createElement(QStringLiteral("locations"));
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
                QDomElement locE = doc.createElement(QStringLiteral("location"));
                locE.setAttribute(QStringLiteral("name"), loc);
                locsE.appendChild(locE);
            }
            listsE.appendChild(locsE);
        }

        // Scene intros (location types)
        {
            QDomElement introsE = doc.createElement(QStringLiteral("scene_intros"));
            const QStringList types = Structure::standardLocationTypes();
            for (const QString &t : types) {
                QDomElement introE = doc.createElement(QStringLiteral("scene_intro"));
                introE.setAttribute(QStringLiteral("name"), t);
                introsE.appendChild(introE);
            }
            listsE.appendChild(introsE);
        }

        // Scene times (moments)
        {
            QDomElement timesE = doc.createElement(QStringLiteral("scene_times"));
            const QStringList moments = Structure::standardMoments();
            for (const QString &m : moments) {
                QDomElement timeE = doc.createElement(QStringLiteral("scene_time"));
                timeE.setAttribute(QStringLiteral("name"), m);
                timesE.appendChild(timeE);
            }
            listsE.appendChild(timesE);
        }

        // Character name extensions
        {
            QDomElement extsE = doc.createElement(QStringLiteral("extensions"));
            const QStringList extensions({ QStringLiteral("(V.O.)"), QStringLiteral("(O.S.)"),
                                           QStringLiteral("(O.C.)"),
                                           QStringLiteral("(SUBTITLE)") });
            for (const QString &ext : extensions) {
                QDomElement extE = doc.createElement(QStringLiteral("extension"));
                extE.setAttribute(QStringLiteral("name"), ext);
                extsE.appendChild(extE);
            }
            listsE.appendChild(extsE);
        }

        // Transitions
        {
            QDomElement transitionsE = doc.createElement(QStringLiteral("transitions"));
            const QStringList transitions(
                    { QStringLiteral("CUT TO:"), QStringLiteral("FADE IN:"),
                      QStringLiteral("FADE OUT"), QStringLiteral("FADE TO:"),
                      QStringLiteral("DISSOLVE TO:"), QStringLiteral("MATCH CUT TO:"),
                      QStringLiteral("JUMP CUT TO:"), QStringLiteral("FADE TO BLACK") });
            for (const QString &t : transitions) {
                QDomElement transE = doc.createElement(QStringLiteral("transition"));
                transE.setAttribute(QStringLiteral("name"), t);
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
                { "White", 0, "White", "" },     { "Blue", 1, "Blue", "*" },
                { "Pink", 2, "Pink", "*" },       { "Yellow", 3, "Yellow", "*" },
                { "Green", 4, "Green", "*" },     { "Goldenrod", 5, "Goldenrod", "*" },
                { "Buff", 6, "Buff", "*" },        { "Salmon", 7, "Salmon", "*" },
                { "Cherry", 8, "Cherry", "*" },   { "Tan", 9, "Tan", "*" },
            };
            QDomElement revColorsE = doc.createElement(QStringLiteral("revision_colors"));
            for (const RevColor &rc : revColors) {
                QDomElement rcE = doc.createElement(QStringLiteral("revision_color"));
                rcE.setAttribute(QStringLiteral("name"), QLatin1String(rc.name));
                rcE.setAttribute(QStringLiteral("index"), rc.index);
                rcE.setAttribute(QStringLiteral("color_name"), QLatin1String(rc.colorName));
                rcE.setAttribute(QStringLiteral("color_index"), rc.index);
                rcE.setAttribute(QStringLiteral("mark"), QLatin1String(rc.mark));
                revColorsE.appendChild(rcE);
            }
            listsE.appendChild(revColorsE);
        }

        listsE.appendChild(doc.createElement(QStringLiteral("tag_categories")));
        listsE.appendChild(doc.createElement(QStringLiteral("highlight_rules")));
        rootE.appendChild(listsE);
    }

    // ---- <notes> (Scrite extension) ----
    {
        QDomElement notesContainerE = doc.createElement(QStringLiteral("notes"));
        bool hasAnyNote = false;

        auto addNoteElement = [&](const QString &type, const QString &id, const QString &ref,
                                  const QString &label, const QString &text) {
            QDomElement noteE = doc.createElement(QStringLiteral("note"));
            noteE.setAttribute(QStringLiteral("type"), type);
            if (!id.isEmpty())
                noteE.setAttribute(QStringLiteral("id"), id);
            if (!ref.isEmpty())
                noteE.setAttribute(QStringLiteral("ref"), ref);
            noteE.setAttribute(QStringLiteral("label"), label);
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
