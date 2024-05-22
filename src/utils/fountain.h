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

class QIODevice;

namespace Fountain {

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
};

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

} // namespace Fountain

#endif // FOUNTAIN_H
