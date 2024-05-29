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

#ifndef FOUNTAIN_H
#define FOUNTAIN_H

#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QPair>
#include <QString>
#include <QTextLayout>
#include <QVector>

class Scene;
class QIODevice;
class Screenplay;
class ScreenplayElement;

namespace Fountain {

class Parser;

struct Element
{
    enum Type {
        None,
        Unknown, // Done
        SceneHeading, // Done
        Action, // Done
        Character, // Done
        Dialogue, // Done
        Parenthetical, // Done
        Lyrics, // Done
        Shot, // Done
        Transition, // Done
        PageBreak, // Done
        LineBreak, // Done
        Section, // Done
        Synopsis // Done
    };

    Type type = None;
    QString text;
    bool isCentered = false;
    QString sceneNumber;
    int sectionDepth = 0;
    QStringList notes;
    QVector<QTextLayout::FormatRange> formats;

    QJsonObject toJson() const;

private:
    // Extra data that's only useful while parsing.
    friend class Parser;
    bool containsNonLatinChars = false;
    QString simplifiedText;
    QString trimmedText;
};

typedef QPair<QString, QString> TitlePageField;
typedef QList<TitlePageField> TitlePage;
typedef QList<Element> Body;

void populateTitlePage(const Screenplay *screenplay, TitlePage &titlePage);

void populateBody(const Scene *scene, Body &body, const ScreenplayElement *element = nullptr);
void populateBody(const Screenplay *screenplay, Body &body);
void populateBody(const ScreenplayElement *element, Body &body);

void loadTitlePage(const TitlePage &titlePage, Screenplay *screenplay);

void loadIntoScene(const Body &body, Scene *scene, ScreenplayElement *element = nullptr);
bool loadIntoScene(const Element &fElement, Scene *scene, ScreenplayElement *element = nullptr);

class Parser
{
public:
    enum Options {
        NoOption = 0,
        IgnoreLeadingWhitespaceOption = 1,
        IgnoreTrailingWhiteSpaceOption = 2,
        JoinAdjacentElementOption = 4,
        ResolveEmphasisOption = 8,
        DefaultOptions = IgnoreLeadingWhitespaceOption | IgnoreTrailingWhiteSpaceOption
                | JoinAdjacentElementOption | ResolveEmphasisOption
    };

    Parser(const QString &content, int options = DefaultOptions);
    Parser(const QByteArray &content, int options = DefaultOptions);
    Parser(QIODevice *device, int options = DefaultOptions);
    ~Parser();

    QList<Element> body() const { return m_body; }
    QList<QPair<QString, QString>> titlePage() const { return m_titlePage; }

    QJsonObject toJson() const;

private:
    void parseContents(const QString &content);

    QString cleanup(const QString &content) const;

    void parseTitlePage(const QString &content);

    void parseBody(const QString &content);

    void processFormalAction();
    void processSceneHeadings();
    void processShotsAndTransitions();
    void processCharacters();
    void processDialogueAndParentheticals();
    void processLyrics();
    void processSectionsAndSynopsis();
    void processAction();

    void joinAdjacentElements();
    void processNotes();

    void processEmphasis();

    void removeEmptyLines();

private:
    int m_options = DefaultOptions;
    QList<Element> m_body;
    QList<QPair<QString, QString>> m_titlePage;
};

class Writer
{
public:
    enum Options {
        NoOption = 0,
        StrictSyntaxOption = 1,
        EmphasisOption = 2,
        DefaultOptions = StrictSyntaxOption | EmphasisOption
    };

    Writer(QList<QPair<QString, QString>> &titlePage, const QList<Element> &body,
           int options = DefaultOptions);
    Writer(const QList<Element> &body, int options = DefaultOptions);
    Writer(const Screenplay *screenplay, int options = DefaultOptions);
    Writer(const ScreenplayElement *element, int options = DefaultOptions);
    Writer(const Scene *scene, const ScreenplayElement *element, int options = DefaultOptions);
    ~Writer();

    bool write(const QString &fileName) const;
    bool write(QIODevice *device) const;

    bool writeInto(QString &text) const;
    bool writeInto(QByteArray &text) const;

    QString toString() const;
    QByteArray toByteArray() const;

private:
    void writeSceneHeading(QTextStream &ts, const Element &element) const;
    void writeAction(QTextStream &ts, const Element &element) const;
    void writeCharacter(QTextStream &ts, const Element &element) const;
    void writeParenthetical(QTextStream &ts, const Element &element) const;
    void writeDialogue(QTextStream &ts, const Element &element) const;
    void writeShotOrTransition(QTextStream &ts, const Element &element) const;
    void writeLyrics(QTextStream &ts, const Element &element) const;
    void writePageBreak(QTextStream &ts, const Element &element) const;
    void writeLineBreak(QTextStream &ts, const Element &element) const;
    void writeSection(QTextStream &ts, const Element &element) const;
    void writeSynopsis(QTextStream &ts, const Element &element) const;

private:
    QString emphasisedText(const Element &element) const;

private:
    QList<QPair<QString, QString>> m_titlePage;
    QList<Element> m_body;
    int m_options = DefaultOptions;
};

} // namespace Fountain

#endif // FOUNTAIN_H
