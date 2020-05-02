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
    Screenplay *screenplay = doc->screenplay();

    int sceneCounter = 0;
    Scene *previousScene = nullptr;
    Scene *currentScene = nullptr;
    static const QStringList headerhints = QStringList() <<
            "." << "INT" << "EXT" << "EST" << "INT./EXT" << "INT/EXT" << "I/E";
    bool inCharacter = false;
    bool hasParaBreaks = false;
    auto maybeCharacter = [](QString &text) {
        if(text.startsWith("@")) {
            text = text.mid(1).trimmed();
            return true;
        }

        for(int i=0; i<text.length(); i++) {
            const QChar ch = text.at(i);
            if(ch.isLetter()) {
                if(ch.isLower())
                    return  false;
            }
        }

        return true;
    };

    const QByteArray bytes = device->readAll();

    QTextStream ts(bytes);
    ts.setCodec("utf-8");
    ts.setAutoDetectUnicode(true);

    while(!ts.atEnd())
    {
        QString line = ts.readLine();
        line = line.trimmed();

        if(line.isEmpty())
        {
            inCharacter = false;
            hasParaBreaks = true;
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
        Q_FOREACH(QString hint, headerhints)
        {
            if(line.startsWith(hint))
            {
                if(hint == headerhints.first())
                {
                    isHeader = true;
                    break;
                }

                const QChar sep = line.at(hint.length());
                if(sep.isSpace() || sep == '.')
                {
                    isHeader = true;
                    break;
                }
            }
        }

        if(isHeader)
        {
            ++sceneCounter;
            screenplay->setCurrentElementIndex(-1);
            previousScene = currentScene;
            currentScene = doc->createNewScene();

            SceneHeading *heading = currentScene->heading();
            if(line.startsWith("."))
            {
                line = line.mid(1).trimmed();
                heading->setLocationType( previousScene ? previousScene->heading()->locationType() : "I/E" );
                heading->setLocation(line);
                heading->setMoment( previousScene ? previousScene->heading()->moment() : "DAY" );
            }
            else
                heading->parseFrom(line);

            QString locationForTitle = heading->location();
            if(locationForTitle.length() > 25)
                locationForTitle = locationForTitle.left(22) + "...";

            currentScene->setTitle("[" + QString::number(sceneCounter) + "]: Scene at " + locationForTitle);

            static const QList<QColor> sceneColors = QList<QColor>() <<
                    QColor("purple") << QColor("blue") << QColor("orange") <<
                    QColor("red") << QColor("brown") << QColor("gray");
            currentScene->setColor( sceneColors.at( sceneCounter%sceneColors.length()) );
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
                    screenplay->setSubtitle(title.mid(boIndex+1, bcIndex-boIndex-2));
                    screenplay->setTitle(title.left(boIndex).trimmed());
                }
                else
                    screenplay->setTitle(title);
            }
            else if(line.startsWith("Author:", Qt::CaseInsensitive))
                screenplay->setAuthor(line.section(':',1));
            else if(line.startsWith("Version:", Qt::CaseInsensitive))
                screenplay->setVersion(line.section(':',1));
            else if(line.startsWith("Contact:", Qt::CaseInsensitive))
                screenplay->setContact(line.section(':',1));

            continue; // ignore lines until we get atleast one heading.
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
            line = line.mid(1).trimmed();
            if(line.endsWith("<"))
                line = line.left(line.length()-1);
            para->setText(line);
            para->setType(SceneElement::Shot);
            currentScene->addElement(para);
            continue;
        }

        if(line.startsWith('(') && line.endsWith(')'))
        {
            para->setType(SceneElement::Parenthetical);
            currentScene->addElement(para);
            continue;
        }

        if(!inCharacter && maybeCharacter(line))
        {
            para->setType(SceneElement::Character);
            currentScene->addElement(para);
            inCharacter = true;
            continue;
        }

        if(inCharacter)
        {
            para->setType(SceneElement::Dialogue);
            if(!hasParaBreaks)
                inCharacter = false;
        }
        else
            para->setType(SceneElement::Action);
        currentScene->addElement(para);
    }

    screenplay->setCurrentElementIndex(0);

    return true;
}
