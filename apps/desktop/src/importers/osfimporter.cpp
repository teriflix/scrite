/****************************************************************************
**
** Copyright (C) 2024 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "osfimporter.h"
#include "notes.h"
#include "structure.h"
#include "screenplay.h"
#include "scritedocument.h"

#include <QDomDocument>

// OSF basestyle names
static const QString OSF_StyleSceneHeading = QStringLiteral("Scene Heading");
static const QString OSF_StyleAction = QStringLiteral("Action");
static const QString OSF_StyleCharacter = QStringLiteral("Character");
static const QString OSF_StyleParenthetical = QStringLiteral("Parenthetical");
static const QString OSF_StyleDialogue = QStringLiteral("Dialogue");
static const QString OSF_StyleTransition = QStringLiteral("Transition");
static const QString OSF_StyleShot = QStringLiteral("Shot");

// OSF XML tag / attribute names
static const QString OSF_TagDocument = QStringLiteral("document");
static const QString OSF_TagParagraphs = QStringLiteral("paragraphs");
static const QString OSF_TagTitlepage = QStringLiteral("titlepage");
static const QString OSF_TagNotes = QStringLiteral("notes");
static const QString OSF_TagNote = QStringLiteral("note");
static const QString OSF_TagPara = QStringLiteral("para");
static const QString OSF_TagStyle = QStringLiteral("style");
static const QString OSF_TagText = QStringLiteral("text");
static const QString OSF_AttrType = QStringLiteral("type");
static const QString OSF_AttrDocType = QStringLiteral("Open Screenplay Format document");
static const QString OSF_AttrBasestyle = QStringLiteral("basestyle");
static const QString OSF_AttrId = QStringLiteral("id");
static const QString OSF_AttrRef = QStringLiteral("ref");
static const QString OSF_AttrLabel = QStringLiteral("label");
static const QString OSF_AttrSynopsis = QStringLiteral("synopsis");
static const QString OSF_AttrSynopsisColor = QStringLiteral("synopsis_color");
static const QString OSF_AttrNote = QStringLiteral("note");
static const QString OSF_AttrBookmark = QStringLiteral("bookmark");
static const QString OSF_AttrBold = QStringLiteral("bold");
static const QString OSF_AttrItalic = QStringLiteral("italic");
static const QString OSF_AttrUnderline = QStringLiteral("underline");
static const QString OSF_AttrStrikethrough = QStringLiteral("strikethrough");
static const QString OSF_AttrBgcolor = QStringLiteral("bgcolor");
static const QString OSF_AttrColor = QStringLiteral("color");

OsfImporter::OsfImporter(QObject *parent) : AbstractImporter(parent) { }

OsfImporter::~OsfImporter() { }

bool OsfImporter::canImport(const QString &fileName) const
{
    return QFileInfo(fileName).suffix().toLower() == QStringLiteral("xml");
}

// Collect plain text and format ranges from a <para>'s <text> children.
static QPair<QString, QVector<QTextLayout::FormatRange>>
parseParagraphTexts(const QDomElement &paraE)
{
    QString text;
    QVector<QTextLayout::FormatRange> formats;

    QDomElement textE = paraE.firstChildElement(OSF_TagText);
    while (!textE.isNull()) {
        QTextLayout::FormatRange range;
        range.start = text.length();

        text += textE.text();
        range.length = text.length() - range.start;

        if (textE.attribute(OSF_AttrBold) == QStringLiteral("1"))
            range.format.setFontWeight(QFont::Bold);
        if (textE.attribute(OSF_AttrItalic) == QStringLiteral("1"))
            range.format.setFontItalic(true);
        if (textE.attribute(OSF_AttrUnderline) == QStringLiteral("1"))
            range.format.setFontUnderline(true);
        if (textE.attribute(OSF_AttrStrikethrough) == QStringLiteral("1"))
            range.format.setFontStrikeOut(true);
        if (textE.hasAttribute(OSF_AttrBgcolor))
            range.format.setBackground(QBrush(QColor(textE.attribute(OSF_AttrBgcolor))));
        if (textE.hasAttribute(OSF_AttrColor))
            range.format.setForeground(QBrush(QColor(textE.attribute(OSF_AttrColor))));

        if (!range.format.isEmpty())
            formats.append(range);

        textE = textE.nextSiblingElement(OSF_TagText);
    }

    return qMakePair(text, formats);
}

// Extract plain text from <para> children of an element (for titlepage fields).
static QString extractParaText(const QDomElement &parentE, const QString &bookmark)
{
    QDomElement paraE = parentE.firstChildElement(OSF_TagPara);
    while (!paraE.isNull()) {
        if (paraE.attribute(OSF_AttrBookmark) == bookmark)
            return parseParagraphTexts(paraE).first;
        paraE = paraE.nextSiblingElement(OSF_TagPara);
    }
    return QString();
}

bool OsfImporter::doImport(QIODevice *device)
{
    const QByteArray xml = device->readAll();

    QDomDocument doc;
    QDomDocument::ParseResult parseResult = doc.setContent(xml);
    if (!parseResult) {
        this->error()->setErrorMessage(
                QStringLiteral("Parse Error: %1 at Line %2, Column %3")
                        .arg(parseResult.errorMessage)
                        .arg(parseResult.errorLine)
                        .arg(parseResult.errorColumn));
        return false;
    }

    QDomElement rootE = doc.documentElement();
    if (rootE.tagName() != OSF_TagDocument
        || rootE.attribute(QStringLiteral("type")) != OSF_AttrDocType) {
        this->error()->setErrorMessage(QStringLiteral("Not an Open Screenplay Format file."));
        return false;
    }

    QDomElement paragraphsE = rootE.firstChildElement(OSF_TagParagraphs);
    if (paragraphsE.isNull()) {
        this->error()->setErrorMessage(QStringLiteral("No paragraphs found in OSF file."));
        return false;
    }

    // Count scenes for canvas layout.
    const int nrScenes = [&paragraphsE]() {
        int count = 0;
        QDomElement paraE = paragraphsE.firstChildElement(OSF_TagPara);
        while (!paraE.isNull()) {
            const QDomElement styleE = paraE.firstChildElement(OSF_TagStyle);
            if (!styleE.isNull()
                && styleE.attribute(OSF_AttrBasestyle) == OSF_StyleSceneHeading)
                ++count;
            paraE = paraE.nextSiblingElement(OSF_TagPara);
        }
        return count;
    }();

    this->configureCanvas(nrScenes);

    const QStringList knownStyles({ OSF_StyleSceneHeading, OSF_StyleAction, OSF_StyleCharacter,
                                    OSF_StyleParenthetical, OSF_StyleDialogue, OSF_StyleTransition,
                                    OSF_StyleShot });

    // Count total paras for progress tracking.
    const int nrParas = [&paragraphsE]() {
        int count = 0;
        QDomElement paraE = paragraphsE.firstChildElement(OSF_TagPara);
        while (!paraE.isNull()) {
            ++count;
            paraE = paraE.nextSiblingElement(OSF_TagPara);
        }
        return count;
    }();
    this->progress()->setProgressStep(1.0 / qreal(nrParas + 1));

    Scene *scene = nullptr;

    QDomElement paraE = paragraphsE.firstChildElement(OSF_TagPara);
    while (!paraE.isNull()) {
        TraverseDomElement tde(paraE, this->progress());

        const QDomElement styleE = paraE.firstChildElement(OSF_TagStyle);
        if (styleE.isNull())
            continue;

        const QString basestyle = styleE.attribute(OSF_AttrBasestyle);
        const int styleIndex = knownStyles.indexOf(basestyle);
        if (styleIndex < 0)
            continue;

        const auto [text, formats] = parseParagraphTexts(paraE);

        if (styleIndex != 0 && scene == nullptr)
            scene = this->createScene(QString());

        SceneElement *sceneElement = nullptr;

        switch (styleIndex) {
        case 0: { // Scene Heading
            scene = this->createScene(text);

            // Restore the stable Scrite scene ID if present (Scrite-exported files).
            const QString sceneId = paraE.attribute(OSF_AttrId);
            if (!sceneId.isEmpty())
                scene->setId(sceneId);

            if (paraE.hasAttribute(OSF_AttrSynopsis))
                scene->setSynopsis(paraE.attribute(OSF_AttrSynopsis));

            if (paraE.hasAttribute(OSF_AttrSynopsisColor))
                scene->setColor(QColor(paraE.attribute(OSF_AttrSynopsisColor)));

            // Inline note on the scene heading para becomes a text note on the scene.
            if (paraE.hasAttribute(OSF_AttrNote)) {
                const QString noteText = paraE.attribute(OSF_AttrNote)
                                                 .replace(QStringLiteral("&#xA;"),
                                                          QStringLiteral("\n"));
                Note *note = scene->notes()->addTextNote();
                note->setContent(QJsonValue(noteText));
            }
        } break;

        case 1: // Action
            sceneElement = this->addSceneElement(scene, SceneElement::Action, text);
            break;

        case 2: // Character
            sceneElement = this->addSceneElement(scene, SceneElement::Character, text);
            break;

        case 3: { // Parenthetical — OSF stores without parens; Scrite expects them.
            QString parenText = text.trimmed();
            if (!parenText.startsWith(QLatin1Char('(')))
                parenText.prepend(QLatin1Char('('));
            if (!parenText.endsWith(QLatin1Char(')')))
                parenText.append(QLatin1Char(')'));
            sceneElement = this->addSceneElement(scene, SceneElement::Parenthetical, parenText);
        } break;

        case 4: // Dialogue
            sceneElement = this->addSceneElement(scene, SceneElement::Dialogue, text);
            break;

        case 5: // Transition
            sceneElement = this->addSceneElement(scene, SceneElement::Transition, text);
            break;

        case 6: // Shot
            sceneElement = this->addSceneElement(scene, SceneElement::Shot, text);
            break;
        }

        if (sceneElement != nullptr && !formats.isEmpty())
            sceneElement->setTextFormats(formats);
    }

    // --- Title page ---
    const QDomElement titlepageE = rootE.firstChildElement(OSF_TagTitlepage);
    if (!titlepageE.isNull()) {
        Screenplay *sp = this->document()->screenplay();

        auto field = [&](const QString &bookmark) {
            return extractParaText(titlepageE, bookmark);
        };

        if (!field(QStringLiteral("Title")).isEmpty())
            sp->setTitle(field(QStringLiteral("Title")));
        if (!field(QStringLiteral("Subtitle")).isEmpty())
            sp->setSubtitle(field(QStringLiteral("Subtitle")));
        if (!field(QStringLiteral("Author")).isEmpty())
            sp->setAuthor(field(QStringLiteral("Author")));
        if (!field(QStringLiteral("Contact")).isEmpty())
            sp->setContact(field(QStringLiteral("Contact")));
        if (!field(QStringLiteral("Address")).isEmpty())
            sp->setAddress(field(QStringLiteral("Address")));
        if (!field(QStringLiteral("PhoneNumber")).isEmpty())
            sp->setPhoneNumber(field(QStringLiteral("PhoneNumber")));
        if (!field(QStringLiteral("Email")).isEmpty())
            sp->setEmail(field(QStringLiteral("Email")));
        if (!field(QStringLiteral("Website")).isEmpty())
            sp->setWebsite(field(QStringLiteral("Website")));
        if (!field(QStringLiteral("Version")).isEmpty())
            sp->setVersion(field(QStringLiteral("Version")));
        if (!field(QStringLiteral("BasedOn")).isEmpty())
            sp->setBasedOn(field(QStringLiteral("BasedOn")));
        if (!field(QStringLiteral("Logline")).isEmpty())
            sp->setLogline(field(QStringLiteral("Logline")));
    }

    // --- Extended notes (Scrite extension) ---
    const QDomElement notesE = rootE.firstChildElement(OSF_TagNotes);
    if (!notesE.isNull()) {
        Structure *structure = this->document()->structure();

        QDomElement noteE = notesE.firstChildElement(OSF_TagNote);
        while (!noteE.isNull()) {
            const QString type = noteE.attribute(OSF_AttrType);
            const QString id = noteE.attribute(OSF_AttrId);
            const QString ref = noteE.attribute(OSF_AttrRef);
            const QString label = noteE.attribute(OSF_AttrLabel);
            const QString text = noteE.text().trimmed();

            auto addNote = [&](Notes *notes) {
                if (!notes)
                    return;
                Note *note = notes->addTextNote();
                note->setTitle(label);
                note->setContent(QJsonValue(text));
            };

            if (type == QStringLiteral("story")) {
                addNote(structure->notes());
            } else if (type == QStringLiteral("scene")) {
                // Prefer stable ID lookup, fall back to heading-text match.
                Scene *targetScene = nullptr;
                if (!id.isEmpty()) {
                    StructureElement *se = structure->findElementBySceneID(id);
                    if (se)
                        targetScene = se->scene();
                }
                if (!targetScene && !ref.isEmpty()) {
                    for (int i = 0; i < structure->elementCount(); ++i) {
                        Scene *candidate = structure->elementAt(i)->scene();
                        if (candidate && candidate->heading()->text() == ref) {
                            targetScene = candidate;
                            break;
                        }
                    }
                }
                if (targetScene)
                    addNote(targetScene->notes());
            } else if (type == QStringLiteral("character")) {
                Character *character = structure->findCharacter(ref);
                if (character)
                    addNote(character->notes());
            }

            noteE = noteE.nextSiblingElement(OSF_TagNote);
        }
    }

    return true;
}
