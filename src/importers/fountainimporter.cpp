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

#include "fountainimporter.h"
#include "application.h"

#include <QRegExp>

FountainImporter::FountainImporter(QObject *parent) : AbstractImporter(parent) { }

FountainImporter::~FountainImporter() { }

bool FountainImporter::canImport(const QString &fileName) const
{
    const QStringList suffixes = { QStringLiteral("fountain"), QStringLiteral("txt") };
    return suffixes.contains(QFileInfo(fileName).suffix().toLower());
}

bool FountainImporter::doImport(QIODevice *device)
{
    // Have tried to parse the Fountain file as closely as possible to
    // the syntax described here: https://fountain.io/syntax
    ScriteDocument *doc = this->document();
    Structure *structure = doc->structure();
    Screenplay *screenplay = doc->screenplay();

    int sceneCounter = 0;
    Scene *currentScene = nullptr;
    Character *character = nullptr;
    static const QStringList headerHints = { QStringLiteral("INT"),     QStringLiteral("EXT"),
                                             QStringLiteral("EST"),     QStringLiteral("INT./EXT"),
                                             QStringLiteral("INT/EXT"), QStringLiteral("I/E") };
    bool inCharacter = false;
    bool hasParaBreak = false;
    bool mergeWithLastPara = false;
    auto maybeCharacter = [](QString &text) {
        if (text.startsWith(QStringLiteral("@"))) {
            text = text.remove(0, 1);
            return true;
        }

        if (text.at(0).script() != QChar::Script_Latin)
            return false;

        for (int i = 0; i < text.length(); i++) {
            const QChar ch = text.at(i);
            if (ch.isLetter()) {
                if (ch.isLower())
                    return false;
            }
        }

        return true;
    };

    const QChar space(' ');
    const QByteArray bytes = device->readAll();
    const QRegExp multipleSpaces(QStringLiteral("\\s+"));
    const QString singleSpace = QStringLiteral(" ");
    const QString pound = QStringLiteral("#");
    const QString sqbo = QStringLiteral("[");
    const QString sqbc = QStringLiteral("]");
    const QString dot = QStringLiteral(".");
    const QString dash = QStringLiteral("-");
    const QString colon = QStringLiteral(":");
    const QString rbo = QStringLiteral("(");
    const QString rbc = QStringLiteral(")");
    const QString at = QStringLiteral("@");
    const QString gt = QStringLiteral(">");
    const QString lt = QStringLiteral("<");

    QTextStream ts(bytes);
    ts.setCodec("utf-8");
    ts.setAutoDetectUnicode(true);

    int lineNr = 0;
    int nrWhiteSpacesInPrevLine = -1;
    int nrWhiteSpaces = -1;

    while (!ts.atEnd()) {
        ++lineNr;

        QString line = ts.readLine();

        nrWhiteSpacesInPrevLine = nrWhiteSpaces;
        nrWhiteSpaces = 0;
        while (line.length()) {
            const QCharRef ch = line.front();
            if (ch.isSpace()) {
                line = line.mid(1);
                ++nrWhiteSpaces;
            } else
                break;
        }
        line = line.trimmed();
        line.replace(multipleSpaces, singleSpace);

        if (line.isEmpty()) {
            inCharacter = false;
            hasParaBreak = true;
            character = nullptr;
            mergeWithLastPara = false;
            continue;
        }

        if (inCharacter && nrWhiteSpacesInPrevLine > 0 && nrWhiteSpaces == 0) {
            inCharacter = false;
            character = nullptr;
        }

        if (line.startsWith(pound)) {
            line = line.remove(pound).trimmed();
            line = line.split(space, Qt::SkipEmptyParts).first();

            ScreenplayElement *element = new ScreenplayElement(screenplay);
            element->setElementType(ScreenplayElement::BreakElementType);
            element->setBreakType(Screenplay::Act);
            this->setBreakTitle(element, line);
            screenplay->addElement(element);
            continue;
        }

        QString pruned;
        if (line.startsWith(sqbo)) {
            // Hack to be able to import fountain files with [nnn]
            const int bcIndex = line.indexOf(sqbc);
            pruned = line.left(bcIndex + 1);
            line = line.mid(bcIndex + 1).trimmed();
        }

        // We do not support other formatting features from the fountain syntax
        line = line.remove(QStringLiteral("_"));
        line = line.remove(QStringLiteral("*"));
        line = line.remove(QStringLiteral("^"));

        // detect if ths line contains a header.
        bool isHeader = false;
        //        if(!inCharacter && !isHeader)
        {
            if (line.length() >= 2) {
                if (line.at(0) == dot[0] && line.at(1) != dot[0])
                    isHeader = true;
            }

            if (isHeader == false) {
                for (const QString &hint : headerHints) {
                    if (line.startsWith(hint) && line.length() > hint.length()
                        && !line.at(hint.length()).isLetterOrNumber()) {
                        if (inCharacter) {
                            QString lt, l, m;
                            isHeader = SceneHeading::parse(line, lt, l, m);
                        } else
                            isHeader = true;
                        break;
                    }
                }
            }
        }

        if (isHeader) {
            ++sceneCounter;
            screenplay->setCurrentElementIndex(-1);
            currentScene = this->createScene(QString());

            SceneHeading *heading = currentScene->heading();
            if (line.at(0) == dot[0])
                line = line.remove(0, 1);
            heading->parseFrom(line);

            QString locationForTitle = heading->location();
            if (locationForTitle.length() > 25)
                locationForTitle = locationForTitle.left(22) + "...";

            currentScene->setSynopsis(QStringLiteral("Scene number #%1 at %2")
                                           .arg(sceneCounter + 1)
                                           .arg(locationForTitle));
            continue;
        }

        if (!pruned.isEmpty())
            line = pruned + " " + line;

        if (currentScene == nullptr) {
            if (line.startsWith(QStringLiteral("Title:"), Qt::CaseInsensitive)) {
                const QString title = line.section(colon, 1);
                const int boIndex = title.indexOf(rbo);
                const int bcIndex = title.lastIndexOf(rbc);
                if (boIndex >= 0 && bcIndex >= 0) {
                    screenplay->setSubtitle(title.mid(boIndex + 1, bcIndex - boIndex - 1));
                    screenplay->setTitle(title.left(boIndex).trimmed());
                } else
                    screenplay->setTitle(title);
            } else if (line.startsWith(QStringLiteral("Credit:"), Qt::CaseInsensitive))
                continue;
            else if (line.startsWith(QStringLiteral("Author:"), Qt::CaseInsensitive))
                screenplay->setAuthor(line.section(':', 1));
            else if (line.startsWith(QStringLiteral("Version:"), Qt::CaseInsensitive))
                screenplay->setVersion(line.section(':', 1));
            else if (line.startsWith(QStringLiteral("Contact:"), Qt::CaseInsensitive))
                screenplay->setContact(line.section(':', 1));
            else if (line.at(0) == at[0]) {
                line = line.remove(0, 1).trimmed();

                character = structure->findCharacter(line);
                if (character == nullptr)
                    character = new Character(structure);
                character->setName(line.trimmed());
                structure->addCharacter(character);
            } else if (character != nullptr) {
                if (line.startsWith(rbo) && line.endsWith(rbc)) {
                    line.remove(0, 1);
                    line.remove(line.length() - 1, 1);

                    Note *note = character->notes()->addTextNote();
                    note->setTitle(line);
                } else {
                    Note *note = character->notes()->lastNote();
                    if (note == nullptr) {
                        note = character->notes()->addTextNote();
                        note->setTitle(QStringLiteral("Note"));
                    }

                    note->setContent(line);
                }
            } else {
                currentScene = this->createScene(QStringLiteral("INT. SOMEWHERE - DAY"));
                currentScene->heading()->setEnabled(false);
                currentScene->setSynopsis(QString());

                SceneElement *para = new SceneElement;
                para->setText(line);
                para->setType(SceneElement::Action);
                currentScene->addElement(para);
            }

            continue;
        }

        SceneElement *para = new SceneElement;
        para->setText(line);

        // I turns out not many writers end their transition with TO:
        // but they all by-and-large end with :
        if (line.endsWith(colon, Qt::CaseInsensitive)) {
            para->setType(SceneElement::Transition);
            currentScene->addElement(para);
            continue;
        }

        if (line.startsWith(gt)) {
            line = line.remove(0, 1);
            if (line.endsWith(lt))
                line = line.remove(line.length() - 1, 1);
            para->setText(line);
            para->setType(SceneElement::Shot);
            currentScene->addElement(para);
            continue;
        }

        if (line.startsWith(rbo) && line.endsWith(rbc)) {
            if (inCharacter) {
                para->setType(SceneElement::Parenthetical);

                while (!line.isEmpty() && line.at(0) == rbo[0])
                    line = line.mid(1);
                while (!line.isEmpty() && line.at(line.length() - 1) == rbc[0])
                    line = line.left(line.length() - 1);
                para->setText(rbo + line + rbc);

                currentScene->addElement(para);
                continue;
            }

            // Parenthetical must be provided as a part of character-dialogue
            // construct only. If its free-and-floating, then we will interpret
            // it as Scene notes.
            line = line.remove(0, 1);
            line = line.remove(line.length() - 1, 1);
            Note *note = currentScene->notes()->addTextNote();
            note->setTitle(QStringLiteral("Note #")
                           + QString::number(currentScene->notes()->noteCount() + 1));
            note->setContent(line);
            note->setColor(
                    Application::instance()->pickStandardColor(currentScene->notes()->noteCount()));

            delete para;
            continue;
        }

        if (!inCharacter && maybeCharacter(line)) {
            para->setText(line);
            para->setType(SceneElement::Character);
            currentScene->addElement(para);
            inCharacter = true;
            continue;
        }

        if (inCharacter) {
            para->setType(SceneElement::Dialogue);
            if (!hasParaBreak)
                inCharacter = false;
        } else
            para->setType(SceneElement::Action);

        SceneElement *prevPara = currentScene->elementCount() > 0
                ? currentScene->elementAt(currentScene->elementCount() - 1)
                : nullptr;
        if (prevPara && prevPara->type() == para->type() && mergeWithLastPara) {
            prevPara->setText(prevPara->text() + space + para->text());
            delete para;
            para = nullptr;
        } else {
            currentScene->addElement(para);
            mergeWithLastPara = nrWhiteSpaces == nrWhiteSpacesInPrevLine;
        }
    }

    return true;
}
