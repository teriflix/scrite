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

#include "fountainimporter.h"
#include "application.h"

#include <QRegExp>

FountainImporter::FountainImporter(QObject *parent)
    : AbstractImporter(parent)
{

}

FountainImporter::~FountainImporter()
{

}

bool FountainImporter::doImport(QIODevice *device)
{
    // Have tried to parse the Fountain file as closely as possible to
    // the syntax described here: https://fountain.io/syntax
    ScriteDocument *doc = this->document();
    Structure *structure = doc->structure();
    Screenplay *screenplay = doc->screenplay();

    int sceneCounter = 0;
    Scene *previousScene = nullptr;
    Scene *currentScene = nullptr;
    Character *character = nullptr;
    static const QStringList headerhints = QStringList() <<
            "INT" << "EXT" << "EST" << "INT./EXT" << "INT/EXT" << "I/E";
    bool inCharacter = false;
    bool hasParaBreak = false;
    bool mergeWithLastPara = false;
    auto maybeCharacter = [](QString &text) {
        if(text.startsWith("@")) {
            text = text.remove(0, 1);
            return true;
        }

        if(text.at(0).script() != QChar::Script_Latin)
            return false;

        for(int i=0; i<text.length(); i++) {
            const QChar ch = text.at(i);
            if(ch.isLetter()) {
                if(ch.isLower())
                    return  false;
            }
        }

        return true;
    };

    const QChar space(' ');
    const QByteArray bytes = device->readAll();
    const QRegExp multipleSpaces("\\s+");
    const QString singleSpace(" ");

    QTextStream ts(bytes);
    ts.setCodec("utf-8");
    ts.setAutoDetectUnicode(true);

    while(!ts.atEnd())
    {
        QString line = ts.readLine();
        line = line.trimmed();
        line.replace(multipleSpaces, singleSpace);

        if(line.isEmpty())
        {
            inCharacter = false;
            hasParaBreak = true;
            character = nullptr;
            mergeWithLastPara = false;
            continue;
        }

        if(line.startsWith("#"))
        {
            line = line.remove("#").trimmed();
            line = line.split(" ", QString::SkipEmptyParts).first();

            ScreenplayElement *element = new ScreenplayElement(screenplay);
            element->setElementType(ScreenplayElement::BreakElementType);
            element->setSceneFromID(line);
            screenplay->addElement(element);
            continue;
        }

        QString pruned;
        if(line.startsWith('['))
        {
            // Hack to be able to import fountain files with [nnn]
            const int bcIndex = line.indexOf(']');
            pruned = line.left(bcIndex+1);
            line = line.mid(bcIndex+1).trimmed();
        }

        // We do not support other formatting features from the fountain syntax
        line = line.remove("_");
        line = line.remove("*");
        line = line.remove("^");

        // detect if ths line contains a header.
        bool isHeader = false;
        if(!inCharacter && !isHeader)
        {
            if(line.length() >= 2)
            {
                if(line.at(0) == '.' && line.at(1) != ".")
                    isHeader = true;
            }

            if(isHeader == false)
            {
                Q_FOREACH(QString hint, headerhints)
                {
                    if(line.startsWith(hint))
                    {
                        isHeader = true;
                        break;
                    }
                }
            }
        }

        if(isHeader)
        {
            ++sceneCounter;
            screenplay->setCurrentElementIndex(-1);
            previousScene = currentScene;
            currentScene = this->createScene(QString());

            SceneHeading *heading = currentScene->heading();
            if(line.at(0) == QChar('.'))
            {
                line = line.remove(0, 1);
                const int dotIndex = line.indexOf('.');
                const int dashIndex = line.indexOf('-');

                if(dashIndex >= 0)
                {
                    const QString moment = line.mid(dashIndex+1).trimmed();
                    heading->setMoment(moment);
                    line = line.left(dashIndex);
                }
                else
                    heading->setMoment( previousScene ? previousScene->heading()->moment() : "DAY" );

                if(dotIndex >= 0)
                {
                    const QString locType = line.left(dotIndex).trimmed();
                    heading->setLocationType(locType);
                    line = line.remove(0, dotIndex+1);
                }
                else
                    heading->setLocationType( previousScene ? previousScene->heading()->locationType() : "I/E" );

                heading->setLocation(line.trimmed());
            }
            else
                heading->parseFrom(line);

            QString locationForTitle = heading->location();
            if(locationForTitle.length() > 25)
                locationForTitle = locationForTitle.left(22) + "...";

            currentScene->setTitle( QString("Scene number #%1 at %2").arg(sceneCounter+1).arg(locationForTitle) );
            continue;
        }

        if(!pruned.isEmpty())
            line = pruned + " " + line;

        if(currentScene == nullptr)
        {
            if(line.startsWith("Title:", Qt::CaseInsensitive))
            {
                const QString title = line.section(':',1);
                const int boIndex = title.indexOf('(');
                const int bcIndex = title.lastIndexOf(')');
                if(boIndex >= 0 && bcIndex >= 0)
                {
                    screenplay->setSubtitle(title.mid(boIndex+1, bcIndex-boIndex-1));
                    screenplay->setTitle(title.left(boIndex).trimmed());
                }
                else
                    screenplay->setTitle(title);
            }
            else if(line.startsWith("Credit:", Qt::CaseInsensitive))
                continue;
            else if(line.startsWith("Author:", Qt::CaseInsensitive))
                screenplay->setAuthor(line.section(':',1));
            else if(line.startsWith("Version:", Qt::CaseInsensitive))
                screenplay->setVersion(line.section(':',1));
            else if(line.startsWith("Contact:", Qt::CaseInsensitive))
                screenplay->setContact(line.section(':',1));
            else if(line.at(0) == QChar('@'))
            {
                line = line.remove(0, 1).trimmed();

                character = structure->findCharacter(line);
                if(character == nullptr)
                    character = new Character(structure);
                character->setName(line.trimmed());
                structure->addCharacter(character);
            }
            else if(character != nullptr)
            {
                if(line.startsWith('(') && line.endsWith(')'))
                {
                    line.remove(0, 1);
                    line.remove(line.length()-1, 1);

                    Note *note = new Note(character);
                    note->setHeading(line);
                    character->addNote(note);
                }
                else
                {
                    Note *note = character->noteAt(character->noteCount()-1);
                    if(note == nullptr)
                    {
                        note = new Note(character);
                        note->setHeading("Note");
                        character->addNote(note);
                    }

                    note->setContent(line);
                }
            }
            else
            {
                currentScene = this->createScene(QStringLiteral("INT. SOMEWHERE - DAY"));
                currentScene->heading()->setEnabled(false);
                currentScene->setTitle(QString());

                SceneElement *para = new SceneElement;
                para->setText(line);
                para->setType(SceneElement::Action);
                currentScene->addElement(para);
            }

            continue;
        }

        SceneElement *para = new SceneElement;
        para->setText(line);

        if(line.endsWith("TO:", Qt::CaseInsensitive))
        {
            para->setType(SceneElement::Transition);
            currentScene->addElement(para);
            continue;
        }

        if(line.startsWith(">"))
        {
            line = line.remove(0, 1);
            if(line.endsWith("<"))
                line = line.remove(line.length()-1,1);
            para->setText(line);
            para->setType(SceneElement::Shot);
            currentScene->addElement(para);
            continue;
        }

        if(line.startsWith('(') && line.endsWith(')'))
        {
            if(inCharacter)
            {
                para->setType(SceneElement::Parenthetical);
                currentScene->addElement(para);
                continue;
            }

            // Parenthetical must be provided as a part of character-dialogue
            // construct only. If its free-and-floating, then we will interpret
            // it as Scene notes.
            line = line.remove(0, 1);
            line = line.remove(line.length()-1, 1);
            Note *note = new Note(currentScene);
            note->setHeading("Note #" + QString::number(currentScene->noteCount()+1));
            note->setContent(line);
            note->setColor( Application::instance()->pickStandardColor(currentScene->noteCount()) );
            currentScene->addNote(note);

            delete para;
            continue;
        }

        if(!inCharacter && maybeCharacter(line))
        {
            para->setText(line);
            para->setType(SceneElement::Character);
            currentScene->addElement(para);
            inCharacter = true;
            continue;
        }

        if(inCharacter)
        {
            para->setType(SceneElement::Dialogue);
            if(!hasParaBreak)
                inCharacter = false;
        }
        else
            para->setType(SceneElement::Action);

        SceneElement *prevPara = currentScene->elementCount() > 0 ? currentScene->elementAt( currentScene->elementCount()-1 ) : nullptr;
        if(prevPara && prevPara->type() == para->type() && mergeWithLastPara)
        {
            prevPara->setText( prevPara->text() + space + para->text() );
            delete para;
            para = nullptr;
        }
        else
        {
            currentScene->addElement(para);
            mergeWithLastPara = true;
        }
    }

    return true;
}
