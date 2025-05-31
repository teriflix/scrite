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

#include "scene.h"
#include "undoredo.h"
#include "hourglass.h"
#include "formatting.h"
#include "application.h"
#include "searchengine.h"
#include "timeprofiler.h"
#include "scritedocument.h"
#include "garbagecollector.h"
#include "qobjectserializer.h"
#include "screenplaytextdocument.h"

#include <QUuid>
#include <QFuture>
#include <QSGNode>
#include <QPainter>
#include <QDateTime>
#include <QByteArray>
#include <QJsonArray>
#include <QTextTable>
#include <QJsonObject>
#include <QQuickWindow>
#include <QUndoCommand>
#include <QJsonDocument>
#include <QTextDocument>
#include <QFutureWatcher>
#include <QtConcurrentRun>
#include <QTextBoundaryFinder>
#include <QScopedValueRollback>
#include <QSGOpaqueTextureMaterial>
#include <QAbstractTextDocumentLayout>

static QDataStream &operator<<(QDataStream &ds, const QTextLayout::FormatRange &formatRange)
{
    ds << formatRange.start << formatRange.length << formatRange.format;
    return ds;
}

static QDataStream &operator>>(QDataStream &ds, QTextLayout::FormatRange &formatRange)
{
    ds >> formatRange.start >> formatRange.length >> formatRange.format;
    return ds;
}

class PushSceneUndoCommand;
class SceneUndoCommand : public QUndoCommand
{
public:
    static SceneUndoCommand *current;

    explicit SceneUndoCommand(Scene *scene, bool allowMerging = true);
    ~SceneUndoCommand();

    // QUndoCommand interface
    enum { ID = 100 };
    void undo();
    void redo();
    int id() const { return ID; }
    bool mergeWith(const QUndoCommand *other);

private:
    QByteArray toByteArray(Scene *scene) const;
    Scene *fromByteArray(const QByteArray &bytes) const;

private:
    friend class PushSceneUndoCommand;
    Scene *m_scene = nullptr;
    QString m_sceneId;
    QByteArray m_after;
    QByteArray m_before;
    bool m_allowMerging = true;
    char m_padding[7];
    QDateTime m_timestamp;
};

SceneUndoCommand *SceneUndoCommand::current = nullptr;

SceneUndoCommand::SceneUndoCommand(Scene *scene, bool allowMerging)
    : m_scene(scene), m_allowMerging(allowMerging), m_timestamp(QDateTime::currentDateTime())
{
    m_padding[0] = 0; // just to get rid of the unused private variable warning.
    m_sceneId = m_scene->id();
    m_before = this->toByteArray(scene);
}

SceneUndoCommand::~SceneUndoCommand() { }

void SceneUndoCommand::undo()
{
    SceneUndoCommand::current = this;
    Scene *scene = this->fromByteArray(m_before);
    SceneUndoCommand::current = nullptr;

    if (scene == nullptr)
        this->setObsolete(true);
}

void SceneUndoCommand::redo()
{
    if (m_scene != nullptr) {
        m_after = this->toByteArray(m_scene);
        m_scene = nullptr;
        return;
    }

    SceneUndoCommand::current = this;
    Scene *scene = this->fromByteArray(m_after);
    SceneUndoCommand::current = nullptr;

    if (scene == nullptr)
        this->setObsolete(true);
}

bool SceneUndoCommand::mergeWith(const QUndoCommand *other)
{
    if (m_allowMerging && this->id() == other->id()) {
        const SceneUndoCommand *cmd = reinterpret_cast<const SceneUndoCommand *>(other);
        if (cmd->m_allowMerging == false)
            return false;

        if (cmd->m_sceneId != m_sceneId)
            return false;

        const qint64 timegap = qAbs(m_timestamp.msecsTo(cmd->m_timestamp));
        static qint64 minTimegap = 1000;
        if (timegap < minTimegap) {
            m_after = cmd->m_after;
            m_timestamp = cmd->m_timestamp;
            return true;
        }
    }

    return false;
}

QByteArray SceneUndoCommand::toByteArray(Scene *scene) const
{
    return scene->toByteArray();
}

Scene *SceneUndoCommand::fromByteArray(const QByteArray &bytes) const
{
    return Scene::fromByteArray(bytes);
}

class PushSceneUndoCommand
{
    friend class SceneElement;
    static UndoStack *allowedStack;

public:
    PushSceneUndoCommand(Scene *scene, bool allowMerging = true);
    ~PushSceneUndoCommand();

private:
    SceneUndoCommand *m_command = nullptr;
};

UndoStack *PushSceneUndoCommand::allowedStack = nullptr;

PushSceneUndoCommand::PushSceneUndoCommand(Scene *scene, bool allowMerging)
{
    if (allowedStack == nullptr)
        allowedStack = Application::instance()->findUndoStack("MainUndoStack");

    if (SceneUndoCommand::current == nullptr && allowedStack != nullptr
        && UndoStack::active() != nullptr && UndoStack::active() == allowedStack && scene != nullptr
        && scene->isUndoRedoEnabled())
        m_command = new SceneUndoCommand(scene, allowMerging);
}

PushSceneUndoCommand::~PushSceneUndoCommand()
{
    if (m_command != nullptr && SceneUndoCommand::current == nullptr && allowedStack != nullptr
        && UndoStack::active() != nullptr && UndoStack::active() == allowedStack)
        UndoStack::active()->push(m_command);
    else
        delete m_command;
}

///////////////////////////////////////////////////////////////////////////////

SceneHeading::SceneHeading(QObject *parent)
    : QObject(parent), m_scene(qobject_cast<Scene *>(parent))
{
    m_padding[0] = 0; // just to get rid of the unused private variable warning.
    connect(this, &SceneHeading::momentChanged, this, &SceneHeading::textChanged);
    connect(this, &SceneHeading::enabledChanged, this, &SceneHeading::textChanged);
    connect(this, &SceneHeading::locationChanged, this, &SceneHeading::textChanged);
    connect(this, &SceneHeading::locationTypeChanged, this, &SceneHeading::textChanged);
    connect(this, &SceneHeading::textChanged, [=]() {
        this->markAsModified();
        this->evaluateWordCountLater();
    });
    connect(this, &SceneHeading::wordCountChanged, m_scene, &Scene::evaluateWordCountLater,
            Qt::UniqueConnection);
}

SceneHeading::~SceneHeading() { }

void SceneHeading::setEnabled(bool val)
{
    if (m_enabled == val)
        return;

    PushSceneUndoCommand cmd(m_scene);

    m_enabled = val;
    emit enabledChanged();
}

void SceneHeading::setLocationType(const QString &val2)
{
    const QString val = val2.toUpper().trimmed();
    if (m_locationType == val)
        return;

    PushSceneUndoCommand cmd(m_scene);

    m_locationType = val;
    emit locationTypeChanged();
}

void SceneHeading::setLocation(const QString &val2)
{
    const QString val = val2.toUpper().trimmed();
    if (m_location == val)
        return;

    PushSceneUndoCommand cmd(m_scene);

    m_location = val;
    emit locationChanged();
}

void SceneHeading::setMoment(const QString &val2)
{
    const QString val = val2.toUpper().trimmed();
    if (m_moment == val)
        return;

    PushSceneUndoCommand cmd(m_scene);

    m_moment = val;
    emit momentChanged();
}

bool SceneHeading::parse(const QString &text, QString &locationType, QString &location,
                         QString &moment, bool strict)
{
    const Structure *structure = ScriteDocument::instance()->structure();
    const QString heading = text.toUpper().trimmed();
    const QRegularExpression fieldSep(QStringLiteral("[\\.-]"));

    const int field1SepLoc = heading.indexOf(fieldSep);
    int field2SepLoc = heading.lastIndexOf(fieldSep);
    if (field2SepLoc == field1SepLoc)
        field2SepLoc = -1;

    if (strict && (field1SepLoc < 0 || field2SepLoc < 0))
        return false;

    if (field1SepLoc < 0 && field2SepLoc < 0) {
        if (structure->standardLocationTypes().contains(heading))
            locationType = heading;
        else if (structure->standardMoments().contains(heading))
            moment = heading;
        else
            location = heading;
        return false;
    }

    if (field1SepLoc < 0) {
        moment = heading.mid(field2SepLoc + 1).trimmed();
        location = heading.mid(field1SepLoc + 1, (field2SepLoc - field1SepLoc - 1)).trimmed();
        return false;
    }

    if (field2SepLoc < 0) {
        locationType = heading.left(field1SepLoc).trimmed();
        location = heading.mid(field1SepLoc + 1, (field2SepLoc - field1SepLoc - 1)).trimmed();
        return false;
    }

    locationType = heading.left(field1SepLoc).trimmed();
    moment = heading.mid(field2SepLoc + 1).trimmed();
    location = heading.mid(field1SepLoc + 1, (field2SepLoc - field1SepLoc - 1)).trimmed();

    if (strict)
        return Structure::standardLocationTypes().contains(locationType);

    return structure->standardLocationTypes().contains(locationType)
            && structure->standardMoments().contains(moment);
}

void SceneHeading::parseFrom(const QString &text)
{
    if (!m_enabled || this->text() == text)
        return;

    QString _locationType, _location, _moment;
    parse(text, _locationType, _location, _moment);

    this->setLocationType(_locationType);
    this->setLocation(_location);
    this->setMoment(_moment);
}

void SceneHeading::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_wordCountTimer.timerId()) {
        m_wordCountTimer.stop();
        this->evaluateWordCount();
    } else
        QObject::timerEvent(event);
}

void SceneHeading::renameCharacter(const QString &from, const QString &to)
{
    int nrReplacements = 0;
    const QString newLocation =
            Application::replaceCharacterName(from, to, m_location, &nrReplacements);
    if (nrReplacements > 0) {
        m_location = newLocation.toUpper();
        emit locationChanged();
    }
}

QString SceneHeading::toString(Mode mode) const
{
    if (m_enabled) {
        const QString dot = QStringLiteral(". ");
        const QString dash = QStringLiteral(" - ");

        if (m_locationType.isEmpty())
            return m_moment.isEmpty() ? m_location : (m_location + dash + m_moment);

        if (m_moment.isEmpty())
            return m_locationType + dot + m_location + (mode == DisplayMode ? dash : QString());

        return m_locationType + dot + m_location + dash + m_moment;
    }

    return QString();
}

void SceneHeading::setWordCount(int val)
{
    if (m_wordCount == val)
        return;

    m_wordCount = val;
    emit wordCountChanged();
}

void SceneHeading::evaluateWordCount()
{
    const QString text = this->toString(DisplayMode);
    this->setWordCount(TransliterationEngine::wordCount(text));
}

void SceneHeading::evaluateWordCountLater()
{
    m_wordCountTimer.start(100, this);
}

///////////////////////////////////////////////////////////////////////////////

SceneElement::SceneElement(QObject *parent)
    : QObject(parent), m_scene(qobject_cast<Scene *>(parent))
{
    connect(this, &SceneElement::typeChanged, this, &SceneElement::elementChanged);
    connect(this, &SceneElement::textChanged, this, &SceneElement::elementChanged);
    connect(this, &SceneElement::elementChanged, [=]() { this->markAsModified(); });

    if (m_scene != nullptr)
        connect(this, &SceneElement::wordCountChanged, m_scene, &Scene::evaluateWordCountLater,
                Qt::UniqueConnection);
}

SceneElement::~SceneElement()
{
    emit aboutToDelete(this);
}

SpellCheckService *SceneElement::spellCheck() const
{
    if (m_spellCheck == nullptr) {
        m_spellCheck = new SpellCheckService(const_cast<SceneElement *>(this));
        m_spellCheck->setMethod(SpellCheckService::OnDemand);
        m_spellCheck->setAsynchronous(true);
        m_spellCheck->setText(m_text);
    }

    return m_spellCheck;
}

void SceneElement::setId(const QString &val)
{
    if (m_id == val || !m_id.isEmpty())
        return;

    m_id = val;
    emit idChanged();
}

QString SceneElement::id() const
{
    if (m_id.isEmpty())
        m_id = QUuid::createUuid().toString();

    return m_id;
}

void SceneElement::setType(SceneElement::Type val)
{
    if (m_type == val)
        return;

    PushSceneUndoCommand cmd(m_scene);

    m_type = val;
    emit typeChanged();

    this->reportSceneElementChanged(Scene::ElementTypeChange);

    /**
     * If the type of a scene element changes from action to something else, or vice-versa,
     * then the alignment flag might need to be refetched.
     */
    if (this->alignment() != m_alignment)
        emit alignmentChanged();
}

QString SceneElement::typeAsString() const
{
    switch (m_type) {
    case Action:
        return "Action";
    case Character:
        return "Character";
    case Dialogue:
        return "Dialogue";
    case Parenthetical:
        return "Parenthetical";
    case Shot:
        return "Shot";
    case Transition:
        return "Transition";
    case Heading:
        return "Scene Heading";
    default:
        break;
    }

    return "Unknown";
}

void SceneElement::setText(const QString &val)
{
    if (m_text == val)
        return;

    PushSceneUndoCommand cmd(m_scene);

    m_text = val.trimmed();
    if (m_spellCheck != nullptr)
        m_spellCheck->setText(m_text);

    emit textChanged(val);

    this->reportSceneElementChanged(Scene::ElementTextChange);
    this->evaluateWordCountLater();
}

void SceneElement::setAlignment(Qt::Alignment val)
{
    Qt::Alignment val2 = m_type == SceneElement::Action ? val : Qt::Alignment(0);
    if (m_alignment == val2)
        return;

    PushSceneUndoCommand cmd(m_scene);

    m_alignment = val2;
    emit alignmentChanged();

    this->reportSceneElementChanged(
            Scene::ElementTypeChange); // because the text hasnt technically changed.
}

Qt::Alignment SceneElement::alignment() const
{
    return m_type == Action ? m_alignment : Qt::Alignment(0);
}

void SceneElement::setCursorPosition(int val)
{
    m_scene->setCursorPosition(val);
}

int SceneElement::cursorPosition() const
{
    return m_scene->cursorPosition();
}

QString SceneElement::formattedText() const
{
    if (m_type == SceneElement::Parenthetical) {
        QString text = m_text;
        if (!text.startsWith("("))
            text.prepend("(");
        if (!text.endsWith(")"))
            text.append(")");
        return text;
    }

    switch (m_type) {
    case SceneElement::Heading:
    case SceneElement::Character:
        return m_text.toUpper();
    case SceneElement::Shot:
    case SceneElement::Transition:
        return m_text.toUpper() + (m_text.endsWith(':') ? "" : ":");
    default:
        break;
    }

    return m_text;
}

bool SceneElement::polishText(Scene *previousScene)
{
    Q_UNUSED(previousScene)

    if (m_scene == nullptr)
        return false;

    /**
     * This function is called when the user is done editing a scene element and goes on to another
     * paragraph. At this point, we can polish the text. What does polish mean?
     *
     * For instance, if we have two character paragraphs one after the other, except for the first
     * one the subsequent ones can have (CONT'D) suffix, if it doesn't already have. And if there
     * is a CONT'D added by mistake, we can remove it.
     *
     * Another instance, transition paragraphs must end with :. If they don't then we can append
     * one.
     */
    QString polishedText = m_text;

    if (m_type == SceneElement::Shot || m_type == SceneElement::Transition) {
        const QString colon = QLatin1String(":");
        polishedText = polishedText.trimmed().toUpper();
        if (!polishedText.endsWith(colon))
            polishedText += colon;
    }

    const QString openB = QLatin1String("(");
    const QString closeB = QLatin1String(")");

    if (m_type == SceneElement::Character) {
        polishedText = polishedText.simplified().toUpper();

        const QString contd = QLatin1String("CONT'D");
        const QString openB2 = QLatin1String(" (");
        const int openBIndex = polishedText.indexOf(openB);

        // There should be a space before (
        if (openBIndex > 0) {
            const QChar ch = polishedText.at(openBIndex - 1);
            if (ch.category() != QChar::Separator_Space)
                polishedText = polishedText.insert(openBIndex, QChar(' '));
        }

        // If there is ( then there must be a )
        if (openBIndex >= 0) {
            if (!polishedText.endsWith(closeB))
                polishedText += closeB;
        }

        // If previous character name is same as current one then add CONT'D
        // Otherwise, remove CONT'D if present.
        const SceneElement *prevCharElement = [&]() -> SceneElement * {
            const int myIndex = m_scene->indexOfElement(this);
            for (int i = myIndex - 1; i >= 0; i--) {
                SceneElement *prevElement = m_scene->elementAt(i);
                if (prevElement->type() == SceneElement::Character)
                    return prevElement;
            }

            // Making this work across multiple scenes is rather controversial. Let's not do this
            // right now.
            if (previousScene != nullptr && !this->scene()->heading()->isEnabled()) {
                for (int i = previousScene->elementCount() - 1; i >= 0; i--) {
                    SceneElement *prevElement = previousScene->elementAt(i);
                    if (prevElement->type() == SceneElement::Character)
                        return prevElement;
                }
            }

            return nullptr;
        }();

        const QString myCharacterName = polishedText.section('(', 0, 0).trimmed();

        auto removeContd = [&]() {
            const QString comma = QLatin1String(",");
            QString suffix = polishedText.section('(', 1, -1).trimmed();
            if (suffix.endsWith(closeB))
                suffix = suffix.left(suffix.length() - 1);
            suffix.remove(contd, Qt::CaseInsensitive);
            suffix = suffix.simplified();
            if (!suffix.isEmpty()) {
                suffix.replace(QLatin1String(",,"), comma);
                suffix.replace(QLatin1String(", ,"), comma);
            }
            if (suffix.isEmpty() || suffix == comma)
                polishedText = myCharacterName;
            else
                polishedText = myCharacterName + openB2 + suffix + closeB;
        };

        if (prevCharElement != nullptr) {
            const QString prevCharacterName = prevCharElement->text().section('(', 0, 0).trimmed();
            if (myCharacterName.compare(prevCharacterName, Qt::CaseInsensitive) == 0) {
                // Include CONT'D if not already done.
                QString suffix = polishedText.section('(', 1, -1).trimmed();
                if (suffix.isEmpty())
                    polishedText += openB2 + contd + closeB;
            } else {
                // Remove CONT'D if its there.
                removeContd();
            }
        } else
            removeContd();
    }

    if (m_type == SceneElement::Parenthetical) {
        // Parentheticals must have brackets on either end.
        polishedText = polishedText.trimmed();

        if (!polishedText.startsWith(openB))
            polishedText.prepend(openB);

        if (!polishedText.endsWith(closeB))
            polishedText += closeB;
    }

    QScopedValueRollback<UndoStack *> undoStackRollback(PushSceneUndoCommand::allowedStack,
                                                        nullptr);
    const QString oldText = m_text;
    this->setText(polishedText);
    return oldText != m_text;
}

bool SceneElement::capitalizeSentences()
{
    const QList<int> autoCapPositions = this->autoCapitalizePositions();
    if (autoCapPositions.isEmpty())
        return false;

    for (int autoCapPosition : autoCapPositions) {
        const QChar ch = m_text.at(autoCapPosition);
        m_text[autoCapPosition] = ch.toUpper();
    }

    return true;
}

QList<int> SceneElement::autoCapitalizePositions() const
{
    // Auto-capitalize needs to be done only on action and dialogue paragraphs.
    if (!QList<SceneElement::Type>({ SceneElement::Action, SceneElement::Dialogue })
                 .contains(m_type))
        return QList<int>();

    return SceneElement::autoCapitalizePositions(m_text);
}

QList<int> SceneElement::autoCapitalizePositions(const QString &text)
{
    QList<int> ret;

    static QList<QChar> sentenceBreaks(
            { '.', '!', '?' }); // I want to use QChar::isPunct(), but it
                                // returns true for non-sentence breaking
                                // punctuations also, which we don't want.
    bool needsCapping = true;
    for (int i = 0; i < text.length(); i++) {
        const QChar ch = text.at(i);
        if (ch.isSpace())
            continue;

        if (needsCapping) {
            needsCapping = false;
            const QChar uch = ch.toUpper();
            if (uch != ch)
                ret.append(i);
        }

        if (sentenceBreaks.contains(ch))
            needsCapping = true;
    }

    // Capitalize I if found anywhere in the text, except at the very end.
    // Capitalize character names, except at the very end.

    const Structure *structure = ScriteDocument::instance()->structure();
    const QStringList characterNames = structure->characterNames();

    QTextBoundaryFinder finder(QTextBoundaryFinder::Word, text);
    while (finder.position() < text.length()) {
        if (finder.boundaryReasons().testFlag(QTextBoundaryFinder::StartOfItem)) {
            const int startPos = finder.position();
            const int endPos = finder.toNextBoundary();
            if (endPos < 0)
                break;

            const int length = endPos - startPos;
            if (length == 0)
                continue;

            if (startPos == text.length() - length)
                break; // we don't want to capitalize i if its the last word, because
                       // the user maybe typing something else after that.

            const QString word = text.mid(startPos, endPos - startPos).section('\'', 0, 0);

            if (length == 1) {
                if (word == QLatin1String("i"))
                    ret.append(startPos);
            } else {
                if (word.at(0).isLower() && characterNames.contains(word, Qt::CaseInsensitive))
                    ret.append(startPos);
            }
        } else if (finder.toNextBoundary() < 0)
            break;
    }

    return ret;
}

QJsonArray SceneElement::find(const QString &text, int flags) const
{
    return SearchEngine::indexesOf(text, m_text, flags);
}

void SceneElement::serializeToJson(QJsonObject &json) const
{
    const QJsonArray jtextFormats = textFormatsToJson(m_textFormats);
    if (!jtextFormats.isEmpty())
        json.insert(QLatin1String("#textFormats"), jtextFormats);
}

void SceneElement::deserializeFromJson(const QJsonObject &json)
{
    if (m_type == SceneElement::Character) {
        const int bo = m_text.indexOf(QStringLiteral("("));
        if (bo > 0 && !m_text.at(bo - 1).isSpace()) {
            m_text.insert(bo, QChar(' '));
            emit textChanged(m_text);

            this->reportSceneElementChanged(Scene::ElementTextChange);
        }
    }

    const QJsonArray jtextFormats = json.value(QLatin1String("#textFormats")).toArray();
    this->setTextFormats(textFormatsFromJson(jtextFormats));

    this->evaluateWordCountLater();
}

void SceneElement::setTextFormats(const QVector<QTextLayout::FormatRange> &formats)
{
    if (m_textFormats == formats)
        return;

    PushSceneUndoCommand cmd(m_scene);

    m_textFormats = formats;
    emit elementChanged();

    this->reportSceneElementChanged(Scene::ElementTextChange);
}

QJsonArray SceneElement::textFormatsToJson(const QVector<QTextLayout::FormatRange> &formats)
{
    QJsonArray jtextFormats;
    for (const QTextLayout::FormatRange &formatRange : formats) {
        const QTextCharFormat format = formatRange.format;

        QJsonObject item;
        item.insert(QLatin1String("start"), formatRange.start);
        item.insert(QLatin1String("length"), formatRange.length);

        QJsonObject attribs;
        if (format.hasProperty(QTextFormat::FontWeight)) {
            if (format.fontWeight() == QFont::Bold)
                attribs.insert(QLatin1String("bold"), true);
        }

        if (format.hasProperty(QTextFormat::FontItalic)) {
            if (format.fontItalic())
                attribs.insert(QLatin1String("italics"), true);
        }

        if (format.hasProperty(QTextFormat::TextUnderlineStyle)) {
            if (format.fontUnderline())
                attribs.insert(QLatin1String("underline"), true);
        }

        if (format.hasProperty(QTextFormat::FontStrikeOut)) {
            if (format.fontStrikeOut())
                attribs.insert(QLatin1String("strikeout"), true);
        }

        if (format.hasProperty(QTextFormat::BackgroundBrush)) {
            const QColor color = format.background().color();
            if (!qFuzzyIsNull(color.alphaF()))
                attribs.insert(QLatin1String("background"), color.name());
        }

        if (format.hasProperty(QTextFormat::ForegroundBrush)) {
            const QColor color = format.foreground().color();
            if (!qFuzzyIsNull(color.alphaF()))
                attribs.insert(QLatin1String("foreground"), color.name());
        }

        if (attribs.isEmpty())
            continue;

        item.insert(QLatin1String("attribs"), attribs);
        jtextFormats.append(item);
    }

    return jtextFormats;
}

QVector<QTextLayout::FormatRange> SceneElement::textFormatsFromJson(const QJsonArray &jtextFormats)
{
    QVector<QTextLayout::FormatRange> textFormats;

    if (!jtextFormats.isEmpty()) {
        textFormats.reserve(jtextFormats.size());

        for (int i = 0; i < jtextFormats.size(); i++) {
            const QJsonObject item = jtextFormats.at(i).toObject();

            QTextLayout::FormatRange formatRange;

            formatRange.start = item.value(QLatin1String("start")).toInt();
            formatRange.length = item.value(QLatin1String("length")).toInt();

            const QJsonObject attribs = item.value(QLatin1String("attribs")).toObject();

            QTextCharFormat &format = formatRange.format;
            if (attribs.value(QLatin1String("bold")).toBool())
                format.setFontWeight(QFont::Bold);
            if (attribs.value(QLatin1String("italics")).toBool())
                format.setFontItalic(true);
            if (attribs.value(QLatin1String("underline")).toBool())
                format.setFontUnderline(true);
            if (attribs.value(QLatin1String("strikeout")).toBool())
                format.setFontStrikeOut(true);

            auto applyBrush = [&](int property, const QJsonValue &value) {
                QColor color(Qt::transparent);

                if (!value.isUndefined()) {
                    const QString valueStr = value.toString();
                    if (!valueStr.isEmpty()) {
                        color = QColor(valueStr);
                        if (!color.isValid())
                            color = Qt::transparent;
                    }
                }

                if (color != Qt::transparent)
                    format.setProperty(property, QBrush(color));
            };

            applyBrush(QTextFormat::BackgroundBrush, attribs.value(QLatin1String("background")));
            applyBrush(QTextFormat::ForegroundBrush, attribs.value(QLatin1String("foreground")));

            if (format.isEmpty())
                continue;

            textFormats.append(formatRange);
        }
    }

    return textFormats;
}

bool SceneElement::event(QEvent *event)
{
    if (event->type() == QEvent::ParentChange) {
        if (m_scene != nullptr)
            disconnect(this, &SceneElement::wordCountChanged, m_scene,
                       &Scene::evaluateWordCountLater);
        m_scene = qobject_cast<Scene *>(this->parent());
        if (m_scene != nullptr)
            connect(this, &SceneElement::wordCountChanged, m_scene, &Scene::evaluateWordCountLater,
                    Qt::UniqueConnection);
    }

    return QObject::event(event);
}

void SceneElement::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_changeTimer.timerId()) {
        m_changeTimer.stop();
        if (m_scene != nullptr) {
            if (m_changeCounters.take(Scene::ElementTypeChange) > 0)
                emit m_scene->sceneElementChanged(this, Scene::ElementTypeChange);
            if (m_changeCounters.take(Scene::ElementTextChange) > 0)
                emit m_scene->sceneElementChanged(this, Scene::ElementTextChange);
        }
        m_changeCounters.clear();
    } else if (event->timerId() == m_wordCountTimer.timerId()) {
        m_wordCountTimer.stop();
        this->evaluateWordCount();
    } else
        QObject::timerEvent(event);
}

void SceneElement::renameCharacter(const QString &from, const QString &to)
{
    /**
     * This function must be called from within Scene::renameCharacter() only.
     * Reason being, we don't check parameter sanity in this function because
     * its assumed that the calling function has already performed all sanity
     * checks. Checking for sanity in each element costs performance.
     */
    int nrReplacements = 0;
    const QString text = Application::replaceCharacterName(from, to, m_text, &nrReplacements);

    if (nrReplacements > 0) {
        switch (m_type) {
        case SceneElement::Shot:
        case SceneElement::Heading:
        case SceneElement::Character:
        case SceneElement::Transition:
            m_text = text.toUpper();
            break;
        default:
            m_text = text;
            break;
        }

        emit textChanged(m_text);
        this->reportSceneElementChanged(Scene::ElementTextChange);
    }
}

void SceneElement::reportSceneElementChanged(int type)
{
    if (m_scene != nullptr) {
        m_changeCounters[type]++;
        m_changeTimer.start(0, this);
    }
}

void SceneElement::setWordCount(int val)
{
    if (m_wordCount == val)
        return;

    m_wordCount = val;
    emit wordCountChanged();
}

void SceneElement::evaluateWordCount()
{
    this->setWordCount(TransliterationEngine::wordCount(m_text));
}

void SceneElement::evaluateWordCountLater()
{
    m_wordCountTimer.start(100, this);
}

///////////////////////////////////////////////////////////////////////////////

DistinctElementValuesMap::DistinctElementValuesMap(SceneElement::Type type) : m_type(type) { }

DistinctElementValuesMap::~DistinctElementValuesMap() { }

bool DistinctElementValuesMap::include(SceneElement *element)
{
    // This function returns true if distinctValues() would return
    // a different list after this function returns
    if (element == nullptr)
        return false;

    if (element->type() == m_type) {
        const bool ret = this->remove(element);

        QString newName = element->formattedText();
        newName = newName.section('(', 0, 0).trimmed();
        if ((m_type == SceneElement::Shot || m_type == SceneElement::Transition)
            && newName.endsWith(':'))
            newName = newName.left(newName.length() - 1);
        if (newName.isEmpty())
            return ret;

        m_forwardMap[element] = newName;
        m_reverseMap[newName].append(element);
        return true;
    }

    if (m_forwardMap.contains(element))
        return this->remove(element);

    return false;
}

bool DistinctElementValuesMap::remove(SceneElement *element)
{
    // This function returns true if distinctValues() would return
    // a different list after this function returns
    const QString oldName = m_forwardMap.take(element);
    if (!oldName.isEmpty()) {
        QList<SceneElement *> &list = m_reverseMap[oldName];
        if (list.removeOne(element)) {
            if (list.isEmpty()) {
                m_reverseMap.remove(oldName);
                return true;
            }

            if (list.size() == 1) {
                const QVariant value =
                        m_type == SceneElement::Character ? list.first()->property("#mute") : false;
                if (value.isValid() && value.toBool())
                    return true;
            }
        }
    }

    return false;
}

bool DistinctElementValuesMap::remove(const QString &name)
{
    if (name.isEmpty())
        return false;

    const QList<SceneElement *> elements = m_reverseMap.take(name);
    if (elements.isEmpty())
        return false;

    for (SceneElement *element : elements)
        m_forwardMap.take(element);

    return true;
}

QStringList DistinctElementValuesMap::distinctValues() const
{
    return m_reverseMap.keys();
}

bool DistinctElementValuesMap::containsValue(const QString &value) const
{
    return m_reverseMap.contains(value.toUpper());
}

QList<SceneElement *> DistinctElementValuesMap::elements() const
{
    return m_forwardMap.keys();
}

QList<SceneElement *> DistinctElementValuesMap::elements(const QString &value) const
{
    return m_reverseMap.value(value.toUpper());
}

void DistinctElementValuesMap::include(const DistinctElementValuesMap &other)
{
    const QList<SceneElement *> elements = other.elements();
    for (SceneElement *element : elements)
        this->include(element);
}

///////////////////////////////////////////////////////////////////////////////

Scene::Scene(QObject *parent) : QAbstractListModel(parent)
{
    m_padding[0] = 0; // just to get rid of the unused private variable warning.
    this->setStructureElement(qobject_cast<StructureElement *>(parent));

    connect(this, &Scene::synopsisChanged, this, &Scene::sceneChanged);
    connect(this, &Scene::colorChanged, this, &Scene::sceneChanged);
    connect(this, &Scene::groupsChanged, this, &Scene::sceneChanged);
    connect(m_notes, &Notes::notesModified, this, &Scene::sceneChanged);
    connect(this, &Scene::elementCountChanged, this, &Scene::sceneChanged);
    connect(this, &Scene::characterRelationshipGraphChanged, this, &Scene::sceneChanged);
    connect(this, &Scene::commentsChanged, this, &Scene::sceneChanged);
    connect(this, &Scene::indexCardFieldValuesChanged, this, &Scene::sceneChanged);
    connect(m_heading, &SceneHeading::textChanged, this, &Scene::sceneChanged);
    connect(m_heading, &SceneHeading::enabledChanged, this, &Scene::sceneChanged);
    connect(this, &Scene::sceneChanged, [=]() { this->markAsModified(); });

    connect(this, &Scene::sceneElementChanged, this, &Scene::onSceneElementChanged);
    connect(this, &Scene::aboutToRemoveSceneElement, this, &Scene::onAboutToRemoveSceneElement);

    connect(this, &Scene::sceneAboutToReset, [this]() {
        m_isBeingReset = true;
        emit resetStateChanged();
    });
    connect(this, &Scene::sceneReset, [this]() {
        m_isBeingReset = false;
        emit resetStateChanged();
    });

    connect(m_attachments, &Attachments::attachmentsModified, this, &Scene::sceneChanged);
    this->evaluateWordCountLater();

    QTimer *summaryChangeTimer = new QTimer(this);
    summaryChangeTimer->setSingleShot(true);
    summaryChangeTimer->setInterval(100);
    connect(summaryChangeTimer, &QTimer::timeout, this, &Scene::evaluateSummary);
    connect(this, &Scene::sceneChanged, summaryChangeTimer, qOverload<>(&QTimer::start));
}

Scene::~Scene()
{
    GarbageCollector::instance()->avoidChildrenOf(this);
    emit aboutToDelete(this);
}

Scene *Scene::clone(QObject *parent) const
{
    Scene *newScene = new Scene(parent);
    newScene->setSynopsis(m_synopsis + QStringLiteral(" [Copy]"));
    newScene->setColor(m_color);
    newScene->setEnabled(m_enabled);
    newScene->heading()->setMoment(m_heading->moment());
    newScene->heading()->setLocation(m_heading->location());
    newScene->heading()->setLocationType(m_heading->locationType());

    for (SceneElement *element : m_elements) {
        SceneElement *newElement = new SceneElement(newScene);
        newElement->setType(element->type());
        newElement->setText(element->text());
        newScene->addElement(newElement);
    }

    newScene->setGroups(m_groups);
    newScene->setComments(m_comments);
    newScene->setIndexCardFieldValues(m_indexCardFieldValues);
    newScene->setCharacterRelationshipGraph(m_characterRelationshipGraph);

    newScene->m_characterElementMap = m_characterElementMap;

    return newScene;
}

bool Scene::isEmpty() const
{
    const bool noNotes = (m_notes == nullptr || m_notes->noteCount() == 0);
    const bool noAttachments = (m_attachments == nullptr || m_attachments->attachmentCount() == 0);
    const bool noContent = !this->hasContent();
    const bool noSynopsis = m_synopsis.isEmpty();

    return noNotes && noAttachments && noContent && noSynopsis;
}

bool Scene::hasContent() const
{
    if (m_elements.size() > 0) {
        for (const SceneElement *element : m_elements) {
            if (!element->text().isEmpty())
                return true;
        }
    }

    return false;
}

void Scene::setId(const QString &val)
{
    if (m_id == val || !m_id.isEmpty())
        return;

    m_id = val;
    emit idChanged();
}

QString Scene::id() const
{
    if (m_id.isEmpty())
        m_id = QUuid::createUuid().toString();

    return m_id;
}

QString Scene::name() const
{
    if (m_synopsis.length() > 15)
        return QString("Scene: %1...").arg(m_synopsis.left(13));

    return QString("Scene: %1").arg(m_synopsis);
}

void Scene::setSynopsis(const QString &val)
{
    if (m_synopsis == val)
        return;

    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "title");
    QScopedPointer<PushObjectPropertyUndoCommand> cmd;
    if (!info->isLocked() && m_undoRedoEnabled)
        cmd.reset(new PushObjectPropertyUndoCommand(this, info->property));

    m_synopsis = val;
    emit synopsisChanged();
}

void Scene::inferSynopsisFromContent()
{
    /**
     * This function is called from importers to let Scenes infer their synopsis (title) from
     * scene contents.
     *
     * If a synopsis is already set (because the importer was able to parse one), then we don't
     * do anything in here.
     *
     * Here we look for the first action paragraph and extract its first sentence as synopsis.
     * If the scene doesnt have any action paragraph, we extract the first dialogue and infer from
     * it. If that is not present either, we try to use scene heading as title. In scene heading is
     * disabled, then we leave the title empty.
     */

    if (!m_synopsis.isEmpty())
        return;

    auto setTitleInternal = [=](const QString &val) {
        m_synopsis = val;
        emit synopsisChanged();
    };
    setTitleInternal(QStringLiteral("Empty Scene"));

    // First lets look for an action paragraph.
    if (m_elements.isEmpty()) {
        if (m_heading->isEnabled()) {
            setTitleInternal(m_heading->text());
            return;
        }

        return;
    }

    auto findParagraph = [=](SceneElement::Type type, SceneElement *fromElement = nullptr) {
        bool checkElement = fromElement == nullptr;
        for (SceneElement *element : qAsConst(m_elements)) {
            if (!checkElement && fromElement != nullptr && element == fromElement) {
                checkElement = true;
                continue;
            }
            if (element->type() == type && !element->text().isEmpty())
                return element;
        }
        return (SceneElement *)nullptr;
    };

    auto firstSentence = [](const QString &text) {
        QTextBoundaryFinder sentenceFinder(QTextBoundaryFinder::Sentence, text);
        const int from = sentenceFinder.position();
        const int to = sentenceFinder.toNextBoundary();
        if (from < 0 || to < 0)
            return text;
        return text.mid(from, (to - from)).trimmed();
    };

    SceneElement *firstActionPara = findParagraph(SceneElement::Action);
    if (firstActionPara != nullptr) {
        setTitleInternal(firstSentence(firstActionPara->text()));
        return;
    }

    SceneElement *firstCharacterPara = findParagraph(SceneElement::Character);
    if (firstCharacterPara != nullptr) {
        const QString name = firstCharacterPara->formattedText();

        SceneElement *firstDialoguePara = findParagraph(SceneElement::Dialogue, firstCharacterPara);
        if (firstDialoguePara != nullptr) {
            const QString dialogue = firstSentence(firstDialoguePara->text());
            setTitleInternal(name + QStringLiteral(": ") + dialogue);
            return;
        }

        setTitleInternal(name + QStringLiteral(" says something."));
        return;
    }
}

void Scene::trimSynopsis()
{
    const QString val = m_synopsis.trimmed();
    if (m_synopsis != val) {
        m_synopsis = val;
        emit synopsisChanged();
    }
}

void Scene::setIndexCardFieldValues(const QStringList &val)
{
    if (m_indexCardFieldValues == val)
        return;

    m_indexCardFieldValues = val;
    emit indexCardFieldValuesChanged();
}

void Scene::setColor(const QColor &val)
{
    if (m_color == val)
        return;

    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "color");
    QScopedPointer<PushObjectPropertyUndoCommand> cmd;
    if (!info->isLocked() && m_undoRedoEnabled)
        cmd.reset(new PushObjectPropertyUndoCommand(this, info->property));

    m_color = val;
    emit colorChanged();
}

void Scene::setEnabled(bool val)
{
    if (m_enabled == val)
        return;

    m_enabled = val;
    emit enabledChanged();
}

void Scene::setType(Scene::Type val)
{
    if (m_type == val)
        return;

    m_type = val;
    emit typeChanged();
}

void Scene::setComments(const QString &val)
{
    if (m_comments == val)
        return;

    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "comments");
    QScopedPointer<PushObjectPropertyUndoCommand> cmd;
    if (!info->isLocked() && m_undoRedoEnabled)
        cmd.reset(new PushObjectPropertyUndoCommand(this, info->property));

    m_comments = val;
    emit commentsChanged();
}

void Scene::setUndoRedoEnabled(bool val)
{
    if (m_undoRedoEnabled == val)
        return;

    m_undoRedoEnabled = val;
    emit undoRedoEnabledChanged();
}

void Scene::setCursorPosition(int val)
{
    if (m_cursorPosition == val)
        return;

    m_cursorPosition = val;
    emit cursorPositionChanged();
}

bool Scene::hasCharacter(const QString &characterName) const
{
    return m_characterElementMap.containsCharacter(characterName);
}

int Scene::characterPresence(const QString &characterName) const
{
    return m_characterElementMap.characterElements(characterName).size();
}

void Scene::addMuteCharacter(const QString &characterName)
{
    HourGlass hourGlass;

    const QStringList names = [characterName]() -> QStringList {
        QStringList ret = characterName.split(QChar(','), Qt::SkipEmptyParts);
        for (QString &name : ret)
            name = name.trimmed();
        return ret;
    }();

    int nrMuteCharactersAdded = 0;
    for (const QString &name : names) {
        const QList<SceneElement *> elements = m_characterElementMap.characterElements(name);
        if (!elements.isEmpty())
            continue;

        SceneElement *element = new SceneElement(this);
        element->setProperty("#mute", true);
        element->setType(SceneElement::Character);
        element->setText(name);
        emit sceneElementChanged(element, ElementTypeChange);

        ++nrMuteCharactersAdded;
    }

    if (nrMuteCharactersAdded > 0)
        emit sceneChanged();
}

void Scene::removeMuteCharacter(const QString &characterName)
{
    const QList<SceneElement *> elements = m_characterElementMap.characterElements(characterName);
    if (elements.isEmpty() || elements.size() > 1)
        return;

    const QVariant value = elements.first()->property("#mute");
    if (value.isValid() && value.toBool()) {
        emit aboutToRemoveSceneElement(elements.first());
        GarbageCollector::instance()->add(elements.first());
        emit sceneChanged();
    }
}

bool Scene::isCharacterMute(const QString &characterName) const
{
    const QList<SceneElement *> elements = m_characterElementMap.characterElements(characterName);
    if (elements.isEmpty() || elements.size() > 1)
        return false;

    const QVariant value = elements.first()->property("#mute");
    return (value.isValid() && value.toBool());
}

bool Scene::isCharacterVisible(const QString &characterName) const
{
    const QList<SceneElement *> elements = m_characterElementMap.characterElements(characterName);
    if (elements.isEmpty())
        return false;

    for (const SceneElement *element : elements) {
        if (element->text().indexOf('(') < 0)
            return true;
    }

    return false;
}

void Scene::scanMuteCharacters(const QStringList &characterNames)
{
    QStringList names = characterNames;
    if (names.isEmpty()) {
        Structure *structure = qobject_cast<Structure *>(this->parent());
        if (structure)
            names = structure->characterNames();
    }

    const QStringList existingCharacters = this->characterNames();
    for (const QString &existingCharacter : existingCharacters)
        names.removeAll(existingCharacter);

    const QList<SceneElement::Type> skipTypes = QList<SceneElement::Type>()
            << SceneElement::Character << SceneElement::Transition << SceneElement::Shot;

    for (SceneElement *element : qAsConst(m_elements)) {
        if (skipTypes.contains(element->type()))
            continue;

        const QString text = element->text();

        for (const QString &name : qAsConst(names)) {
            int pos = 0;
            while (pos < text.length()) {
                pos = text.indexOf(name, pos, Qt::CaseInsensitive);
                if (pos < 0)
                    break;

                if (pos > 0) {
                    const QChar ch = text.at(pos - 1);
                    if (!ch.isPunct() && !ch.isSpace()) {
                        pos += name.length();
                        continue;
                    }
                }

                bool found = false;
                if (text.length() >= pos + name.length()) {
                    const QChar ch = text.at(pos + name.length());
                    found = ch.isPunct() || ch.isSpace();
                }

                if (found)
                    this->addMuteCharacter(name);

                pos += name.length();
            }
        }
    }
}

void Scene::setAct(const QString &val)
{
    const QString val2 = val.toUpper();
    if (m_act == val2)
        return;

    m_act = val2;
    emit actChanged();
}

void Scene::setActIndex(const int &val)
{
    if (m_actIndex == val)
        return;

    m_actIndex = val;
    emit actIndexChanged();
}

void Scene::setEpisodeIndex(const int &val)
{
    if (m_episodeIndex == val)
        return;

    m_episodeIndex = val;
    emit episodeIndexChanged();
}

void Scene::setEpisode(const QString &val)
{
    if (m_episode == val)
        return;

    m_episode = val;
    emit episodeChanged();
}

void Scene::setScreenplayElementIndexList(const QList<int> &val)
{
    if (m_screenplayElementIndexList == val)
        return;

    const bool flag = m_screenplayElementIndexList.isEmpty();
    m_screenplayElementIndexList = val;
    emit screenplayElementIndexListChanged();

    if (flag != m_screenplayElementIndexList.isEmpty())
        emit addedToScreenplayChanged();
}

void Scene::setGroups(const QStringList &val)
{
    if (m_groups == val)
        return;

    m_groups = QSet<QString>(val.begin(), val.end()).values();
    emit groupsChanged();
}

void Scene::addToGroup(const QString &group)
{
    if (group.isEmpty() || this->isInGroup(group))
        return;

    m_groups.append(group);
    m_groups.sort(Qt::CaseInsensitive);
    emit groupsChanged();
}

void Scene::removeFromGroup(const QString &group)
{
    int index = -1;
    for (const QString &item : qAsConst(m_groups)) {
        ++index;
        if (!item.compare(group, Qt::CaseInsensitive))
            break;
    }

    if (index < 0)
        return;

    m_groups.removeAt(index);
    emit groupsChanged();
}

bool Scene::isInGroup(const QString &group) const
{
    return m_groups.contains(group, Qt::CaseInsensitive);
}

void Scene::verifyGroups(const QJsonArray &groupsModel)
{
    if (m_groups.isEmpty())
        return;

    if (groupsModel.isEmpty() && !m_groups.isEmpty()) {
        m_groups.clear();
        emit groupsChanged();
        return;
    }

    auto verifyGroupsImpl = [](const QJsonArray &model, const QStringList &groups) {
        QStringList ret;
        std::copy_if(groups.begin(), groups.end(), std::back_inserter(ret),
                     [model](const QString &group) {
                         for (const QJsonValue &item : model) {
                             const QJsonObject obj = item.toObject();
                             if (obj.value(QStringLiteral("name")).toString() == group)
                                 return true;
                         }
                         return false;
                     });
        ret.sort(Qt::CaseInsensitive);
        return ret;
    };

#if 0
    const QString watcherName = QStringLiteral("verifiedGroupsFutureWatcher");
    if( this->findChild<QFutureWatcherBase*>(watcherName, Qt::FindDirectChildrenOnly) != nullptr )
        return;

    QFuture<QStringList> verifiedGroupsFuture = QtConcurrent::run(verifyGroupsImpl, groupsModel, m_groups);
    QFutureWatcher<QStringList> *verifiedGroupsFutureWatcher = new QFutureWatcher<QStringList>(this);
    verifiedGroupsFutureWatcher->setObjectName(watcherName);
    connect(verifiedGroupsFutureWatcher, &QFutureWatcher<QStringList>::finished, [=]() {
         const QStringList filteredList = verifiedGroupsFutureWatcher->result();
         if(filteredList != m_groups) {
             m_groups = filteredList;
             emit groupsChanged();
         }
         verifiedGroupsFutureWatcher->deleteLater();
    });
    verifiedGroupsFutureWatcher->setFuture(verifiedGroupsFuture);
#else
    const QStringList filteredList = verifyGroupsImpl(groupsModel, m_groups);
    if (filteredList != m_groups) {
        m_groups = filteredList;
        emit groupsChanged();
    }
#endif
}

QQmlListProperty<SceneElement> Scene::elements()
{
    return QQmlListProperty<SceneElement>(reinterpret_cast<QObject *>(this),
                                          static_cast<void *>(this), &Scene::staticAppendElement,
                                          &Scene::staticElementCount, &Scene::staticElementAt,
                                          &Scene::staticClearElements);
}

SceneElement *Scene::appendElement(const QString &text, int type)
{
    SceneElement *element = new SceneElement(this);
    element->setType(SceneElement::Type(type));
    element->setText(text);
    this->addElement(element);
    return element;
}

void Scene::addElement(SceneElement *ptr)
{
    this->insertElementAt(ptr, m_elements.size());
}

void Scene::insertElementAfter(SceneElement *ptr, SceneElement *after)
{
    int index = m_elements.indexOf(after);
    if (index < 0)
        return;

    this->insertElementAt(ptr, index + 1);
}

void Scene::insertElementBefore(SceneElement *ptr, SceneElement *before)
{
    int index = m_elements.indexOf(before);
    if (index < 0)
        return;

    this->insertElementAt(ptr, index);
}

void Scene::insertElementAt(SceneElement *ptr, int index)
{
    if (ptr == nullptr || m_elements.indexOf(ptr) >= 0)
        return;

    if (index < 0 || index > m_elements.size())
        return;

    PushSceneUndoCommand cmd(this);

    if (!m_inSetElementsList)
        this->beginInsertRows(QModelIndex(), index, index);

    ptr->setParent(this);

    m_elements.insert(index, ptr);
    connect(ptr, &SceneElement::elementChanged, this, &Scene::sceneChanged);
    connect(ptr, &SceneElement::aboutToDelete, this, &Scene::removeElement);
    connect(this, &Scene::cursorPositionChanged, ptr, &SceneElement::cursorPositionChanged);

    if (!m_inSetElementsList)
        this->endInsertRows();

    emit elementCountChanged();

    // To ensure that character names are collected under all-character names
    // while an import is being done.
    if (ptr->type() == SceneElement::Character)
        emit sceneElementChanged(ptr, ElementTypeChange);
}

void Scene::removeElement(SceneElement *ptr)
{
    if (ptr == nullptr)
        return;

    const int row = m_elements.indexOf(ptr);
    if (row < 0)
        return;

    PushSceneUndoCommand cmd(this);

    if (!m_inSetElementsList)
        this->beginRemoveRows(QModelIndex(), row, row);

    emit aboutToRemoveSceneElement(ptr);
    m_elements.removeAt(row);

    disconnect(ptr, &SceneElement::elementChanged, this, &Scene::sceneChanged);
    disconnect(ptr, &SceneElement::aboutToDelete, this, &Scene::removeElement);
    disconnect(this, &Scene::cursorPositionChanged, ptr, &SceneElement::cursorPositionChanged);

    if (!m_inSetElementsList)
        this->endRemoveRows();

    emit elementCountChanged();

    if (ptr->parent() == this)
        GarbageCollector::instance()->add(ptr);
}

SceneElement *Scene::elementAt(int index) const
{
    return index < 0 || index >= m_elements.size() ? nullptr : m_elements.at(index);
}

void Scene::setElements(const QList<SceneElement *> &list)
{
    if (!m_elements.isEmpty() || list.isEmpty())
        return;

    this->beginResetModel();

    for (SceneElement *ptr : list) {
        ptr->setParent(this);
        connect(ptr, &SceneElement::elementChanged, this, &Scene::sceneChanged);
        connect(ptr, &SceneElement::aboutToDelete, this, &Scene::removeElement);
        connect(this, &Scene::cursorPositionChanged, ptr, &SceneElement::cursorPositionChanged);
        m_elements.append(ptr);
    }

    this->endResetModel();

    emit elementCountChanged();
}

int Scene::elementCount() const
{
    return m_elements.size();
}

void Scene::clearElements()
{
    while (m_elements.size())
        this->removeElement(m_elements.first());
}

void Scene::removeLastElementIfEmpty()
{
    if (m_elements.isEmpty())
        return;

    SceneElement *element = m_elements.last();
    if (element->text().isEmpty()) {
        emit sceneAboutToReset();
        this->removeElement(element);
        emit sceneReset(-1);
    }
}

void Scene::beginUndoCapture(bool allowMerging)
{
    if (m_pushUndoCommand != nullptr)
        return;

    m_pushUndoCommand = new PushSceneUndoCommand(this, allowMerging);
}

void Scene::endUndoCapture()
{
    if (m_pushUndoCommand == nullptr)
        return;

    delete m_pushUndoCommand;
    m_pushUndoCommand = nullptr;
}

bool Scene::polishText(Scene *previousScene)
{
    bool ret = false;

    for (SceneElement *para : qAsConst(m_elements))
        ret |= para->polishText(previousScene);

    if (ret)
        emit sceneRefreshed();

    return ret;
}

bool Scene::capitalizeSentences()
{
    bool ret = false;

    for (SceneElement *para : qAsConst(m_elements))
        ret |= para->capitalizeSentences();

    if (ret)
        emit sceneRefreshed();

    return ret;
}

void Scene::setSummary(const QString &val)
{
    if (m_summary == val)
        return;

    m_summary = val;
    emit summaryChanged();
}

QHash<QString, QList<SceneElement *>> Scene::dialogueElements() const
{
    QHash<QString, QList<SceneElement *>> ret;
    QString characterName;

    for (SceneElement *element : m_elements) {
        if (element->type() == SceneElement::Character)
            characterName = element->formattedText().section('(', 0, 0).trimmed();
        else if (element->type() == SceneElement::Dialogue) {
            if (!characterName.isEmpty())
                ret[characterName].append(element);
        } else if (element->type() != SceneElement::Parenthetical)
            characterName.clear();
    }

    return ret;
}

Scene *Scene::splitScene(SceneElement *element, int textPosition, QObject *parent)
{
    if (element == nullptr)
        return nullptr;

    const int index = this->indexOfElement(element);
    if (index < 0)
        return nullptr;

    // We cannot split the scene across these types.
    if (element->type() == SceneElement::Heading || element->type() == SceneElement::Parenthetical)
        return nullptr;

    PushSceneUndoCommand cmd(this);

    emit sceneAboutToReset();

    const bool splitTitleAlso = !m_synopsis.trimmed().isEmpty();

    Scene *newScene = new Scene(parent);
    if (splitTitleAlso)
        newScene->setSynopsis("2nd Part Of " + this->synopsis());
    newScene->setColor(this->color());
    newScene->heading()->setEnabled(this->heading()->isEnabled());
    newScene->heading()->setLocationType(this->heading()->locationType());
    newScene->heading()->setLocation(this->heading()->location());
    newScene->heading()->setMoment("LATER");
    newScene->id(); // trigger creation of new Scene ID

    if (splitTitleAlso)
        this->setSynopsis("1st Part Of " + this->synopsis());

    // Move all elements from index onwards to the new scene.
    for (int i = this->elementCount() - 1; i >= index; i--) {
        SceneElement *oldElement = this->elementAt(i);

        if (i == index && oldElement->type() == SceneElement::Action) {
            const QString oldElementText = oldElement->text().trimmed();
            QString locType, location, moment;
            if (SceneHeading::parse(oldElementText, locType, location, moment, true)) {
                newScene->heading()->setEnabled(true);
                newScene->heading()->setLocationType(locType);
                newScene->heading()->setLocation(location);
                newScene->heading()->setMoment(moment);
                this->removeElement(oldElement);
                continue;
            }

            bool couldBeHeading = true;
            for (const QChar ch : oldElementText) {
                if (ch.isLetter()) {
                    couldBeHeading = ch.script() == QChar::Script_Latin && ch.isUpper();
                    if (!couldBeHeading)
                        break;
                }
            }

            if (couldBeHeading) {
                newScene->heading()->setEnabled(true);
                newScene->heading()->setLocationType(this->heading()->locationType());
                newScene->heading()->setLocation(oldElementText);
                newScene->heading()->setMoment(this->heading()->moment().isEmpty()
                                                       ? QString()
                                                       : QStringLiteral("CONTINUOUS"));
                this->removeElement(oldElement);
                continue;
            }
        }

        SceneElement *newElement = new SceneElement(newScene);
        newElement->setType(oldElement->type());
        newElement->setText(oldElement->text());
        newScene->insertElementAt(newElement, 0);

        this->removeElement(oldElement);
    }

    emit sceneReset(textPosition);
    return newScene;
}

bool Scene::mergeInto(Scene *otherScene)
{
    emit otherScene->sceneAboutToReset();
    emit sceneAboutToReset();

    Scene *thisScene = this;
    SceneElement *newElement = new SceneElement(otherScene);
    newElement->setType(SceneElement::Action);
    newElement->setText(QStringLiteral("--"));
    otherScene->addElement(newElement);

    int length = 0;
    for (int i = 0; i < otherScene->elementCount(); i++)
        length += otherScene->elementAt(i)->text().length();
    length += otherScene->elementCount();

    while (thisScene->elementCount()) {
        SceneElement *element = thisScene->elementAt(0);

        newElement = new SceneElement(otherScene);
        newElement->setType(element->type());
        newElement->setText(element->text());
        otherScene->addElement(newElement);

        thisScene->removeElement(element);
    }

    otherScene->setCursorPosition(length);

    emit sceneReset(0);
    emit otherScene->sceneReset(length);

    return true;
}

int Scene::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_elements.size();
}

QVariant Scene::data(const QModelIndex &index, int role) const
{
    if (role == SceneElementRole && index.isValid())
        return QVariant::fromValue<QObject *>(this->elementAt(index.row()));

    return QVariant();
}

QHash<int, QByteArray> Scene::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[SceneElementRole] = "sceneElement";
    return roles;
}

QByteArray Scene::toByteArray() const
{
    QByteArray bytes;
    QDataStream ds(&bytes, QIODevice::WriteOnly);
    ds << m_id;
    ds << m_synopsis;
    ds << m_color;
    ds << m_cursorPosition;
    ds << m_heading->locationType();
    ds << m_heading->location();
    ds << m_heading->moment();
    ds << m_elements.size();
    for (SceneElement *element : m_elements) {
        ds << element->id();
        ds << int(element->type());
        ds << int(element->alignment());
        ds << element->text();
        ds << element->textFormats();
    }

    return bytes;
}

bool Scene::resetFromByteArray(const QByteArray &bytes)
{
    QScopedValueRollback<bool> ure(m_undoRedoEnabled, false);

    QDataStream ds(bytes);

    QString sceneID;
    ds >> sceneID;
    if (m_id.isEmpty())
        this->setId(sceneID);
    else if (sceneID != m_id)
        return false;

    emit sceneAboutToReset();

    QString title;
    ds >> title;
    this->setSynopsis(title);

    QColor color;
    ds >> color;
    this->setColor(color);

    int curPosition = -1;
    ds >> curPosition;
    QTimer::singleShot(0, this, [=]() { this->setCursorPosition(curPosition); });

    QString locType;
    ds >> locType;
    this->heading()->setLocationType(locType);

    QString loc;
    ds >> loc;
    this->heading()->setLocation(loc);

    QString moment;
    ds >> moment;
    this->heading()->setMoment(moment);

    int nrElements = 0;
    ds >> nrElements;

    struct _Paragraph
    {
        QString id;
        int type = SceneElement::Action;
        int alignment = 0;
        QString text;
        QVector<QTextLayout::FormatRange> formats;
    };
    QVector<_Paragraph> paragraphs;
    QStringList paragraphIds;

    paragraphs.reserve(nrElements);
    for (int i = 0; i < nrElements; i++) {
        _Paragraph e;
        ds >> e.id;
        ds >> e.type;
        ds >> e.alignment;
        ds >> e.text;
        ds >> e.formats;
        paragraphIds.append(e.id);
        paragraphs.append(e);
    }

    // Remove stale paragraphs
    for (int i = m_elements.size() - 1; i >= 0; i--) {
        SceneElement *para = m_elements.at(i);
        if (paragraphIds.removeOne(para->id()))
            continue;
        this->removeElement(para);
    }

    // Insert new paragraphs
    for (int i = 0; i < paragraphs.size(); i++) {
        const _Paragraph para = paragraphs.at(i);
        SceneElement *element = i <= m_elements.size() - 1 ? m_elements.at(i) : nullptr;

        const bool elementNeedsInsert = !element || element->id() != para.id;
        if (elementNeedsInsert)
            element = new SceneElement(this);

        element->setId(para.id);
        element->setType(SceneElement::Type(para.type));
        element->setAlignment(Qt::Alignment(para.alignment));
        element->setText(para.text);
        element->setTextFormats(para.formats);

        if (elementNeedsInsert)
            this->insertElementAt(element, i);
    }

    emit sceneReset(curPosition);

    return true;
}

Scene *Scene::fromByteArray(const QByteArray &bytes)
{
    QDataStream ds(bytes);

    QString sceneId;
    ds >> sceneId;

    const Structure *structure = ScriteDocument::instance()->structure();
    const StructureElement *element = structure->findElementBySceneID(sceneId);
    if (element == nullptr || element->scene() == nullptr)
        return nullptr;

    Scene *scene = element->scene();
    if (scene->resetFromByteArray(bytes))
        return scene;

    return nullptr;
}

void Scene::setCharacterRelationshipGraph(const QJsonObject &val)
{
    if (m_characterRelationshipGraph == val)
        return;

    m_characterRelationshipGraph = val;
    emit characterRelationshipGraphChanged();
}

void Scene::serializeToJson(QJsonObject &json) const
{
    const QStringList names = m_characterElementMap.characterNames();
    QJsonArray invisibleCharacters;

    for (const QString &name : names) {
        if (this->isCharacterMute(name))
            invisibleCharacters.append(name);
    }

    if (!invisibleCharacters.isEmpty())
        json.insert(QStringLiteral("#invisibleCharacters"), invisibleCharacters);
}

class AddInvisibleCharactersTimer : public QTimer
{
public:
    explicit AddInvisibleCharactersTimer(const QJsonArray &chars, Scene *parent)
        : QTimer(parent), m_scene(parent), m_characters(chars)
    {
        this->setInterval(100);
        this->setSingleShot(true);
        connect(this, &QTimer::timeout, this, &AddInvisibleCharactersTimer::itsTime);
        this->start();
    }

    ~AddInvisibleCharactersTimer() { }

private:
    void itsTime()
    {
        for (int i = 0; i < m_characters.size(); i++)
            m_scene->addMuteCharacter(m_characters.at(i).toString());
        this->deleteLater();
    }

private:
    Scene *m_scene;
    QJsonArray m_characters;
};

void Scene::deserializeFromJson(const QJsonObject &json)
{
    const QJsonArray invisibleCharacters =
            json.value(QStringLiteral("#invisibleCharacters")).toArray();
    if (!invisibleCharacters.isEmpty())
        new AddInvisibleCharactersTimer(invisibleCharacters, this);

    // Previously notes was an array, because the notes property used to be
    // a list property. Now notes is an object, because it represents Notes class.
    // So, if we are loading a notes from a file created using older versions of Scrite,
    // we have to upgrade the notes to the newer format based on the Notes class.
    const QJsonValue notes = json.value(QStringLiteral("notes"));
    if (notes.isArray())
        m_notes->loadOldNotes(notes.toArray());

    this->evaluateWordCountLater();

    // 'title' property has changed to 'synopsis' since 0.9.3.23
    const QString titleAttr = QStringLiteral("title");
    if (json.contains(titleAttr))
        this->setSynopsis(json.value(titleAttr).toString());
}

bool Scene::canSetPropertyFromObjectList(const QString &propName) const
{
    if (propName == QStringLiteral("elements"))
        return m_elements.isEmpty();

    return false;
}

void Scene::setPropertyFromObjectList(const QString &propName, const QList<QObject *> &objects)
{
    if (propName == QStringLiteral("elements")) {
        this->setElements(qobject_list_cast<SceneElement *>(objects));
        return;
    }
}

void Scene::write(QTextCursor &cursor, const WriteOptions &options) const
{
    /**
     * Although much of the code below is similar to what ScreenplayTextDocument does,
     * I want for this to be distinct - even at the cost of code duplication.
     *
     * The purpose of this method is to participate in the notebook report. So, we don't
     * need this to fit screenplay formatting as much as ScreenplayTextDocument does.
     */

    static const QRegularExpression newlinesRegEx("\n+");
    static const QString newline = QStringLiteral("\n");
    QTextDocument *textDocument = cursor.document();

    if (m_synopsis.isEmpty() && m_comments.isEmpty() && (m_notes == nullptr || m_notes->isEmpty()))
        return;

    // Scene number: heading
    if (options.includeHeading) {
        const QString heading = m_structureElement->title().trimmed();

        if (!heading.isEmpty()) {
            QTextBlockFormat headingBlockFormat;
            headingBlockFormat.setHeadingLevel(options.headingLevel);
            QTextCharFormat headingCharFormat;
            headingCharFormat.setFontWeight(QFont::Bold);
            headingCharFormat.setFontPointSize(
                    ScreenplayTextDocument::headingFontPointSize(options.headingLevel));
            headingBlockFormat.setTopMargin(headingCharFormat.fontPointSize() / 2);
            if (cursor.block().text().isEmpty()) {
                cursor.setBlockFormat(headingBlockFormat);
                cursor.setBlockCharFormat(headingCharFormat);
            } else
                cursor.insertBlock(headingBlockFormat, headingCharFormat);
            cursor.insertText(m_structureElement->title());
        }
    }

    // Featured Photo
    if (options.includeFeaturedPhoto) {
        const Attachments *sceneAttachments = m_attachments;
        const Attachment *featuredAttachment =
                sceneAttachments ? sceneAttachments->featuredAttachment() : nullptr;
        const Attachment *featuredImage =
                featuredAttachment && featuredAttachment->type() == Attachment::Photo
                ? featuredAttachment
                : nullptr;

        if (featuredImage) {
            const QUrl url(QStringLiteral("scrite://") + featuredImage->filePath());
            const QImage image(featuredImage->fileSource().toLocalFile());

            textDocument->addResource(QTextDocument::ImageResource, url,
                                      QVariant::fromValue<QImage>(image));

            const QSizeF imageSize = image.size().scaled(QSize(320, 240), Qt::KeepAspectRatio);

            QTextBlockFormat blockFormat;
            blockFormat.setTopMargin(5);
            cursor.insertBlock(blockFormat);

            QTextImageFormat imageFormat;
            imageFormat.setName(url.toString());
            imageFormat.setWidth(imageSize.width());
            imageFormat.setHeight(imageSize.height());
            cursor.insertImage(imageFormat);
        }
    }

    // Synopsis
    if (options.includeSynopsis) {
        QString synopsis = this->synopsis().trimmed();
        synopsis.replace(newlinesRegEx, newline);

        if (!synopsis.isEmpty()) {
            QColor sceneColor = this->color().lighter(175);
            sceneColor.setAlphaF(0.5);

            QTextBlockFormat blockFormat;
            blockFormat.setTopMargin(5);
            blockFormat.setIndent(2);
            blockFormat.setBackground(sceneColor);

            QTextCharFormat charFormat;
            charFormat.setFont(textDocument->defaultFont());

            cursor.insertBlock(blockFormat, charFormat);

            TransliterationUtils::polishFontsAndInsertTextAtCursor(cursor, synopsis);
        }
    }

    // Index Card Fields
    if (options.includeIndexCardFields) {
        const QJsonArray fields = m_structureElement->structure()->indexCardFields();
        if (!fields.isEmpty() && !m_indexCardFieldValues.isEmpty()) {
            QTextTableFormat tableFormat;
            tableFormat.setCellPadding(2);
            tableFormat.setCellSpacing(0);
            tableFormat.setLeftMargin(cursor.document()->indentWidth() * 2);
            tableFormat.setTopMargin(10);
            tableFormat.setBottomMargin(10);

            QTextFrame *frame = cursor.currentFrame();
            QTextTable *table = cursor.insertTable(fields.size(), 3, tableFormat);
            QTextTableCellFormat cellFormat; // = table->cellAt(0, 0).format();
            cellFormat.setBorderStyle(QTextFrameFormat::BorderStyle_None);
            for (int tr = 0; tr < table->rows(); tr++) {
                for (int tc = 0; tc < table->columns(); tc++) {
                    table->cellAt(tr, tc).setFormat(cellFormat);
                }
            }

            for (int i = 0; i < fields.size(); i++) {
                const QJsonObject field = fields.at(i).toObject();
                const QString key = field.value(QStringLiteral("name")).toString();
                const QString desc = field.value(QStringLiteral("description")).toString();
                const QString value = i < m_indexCardFieldValues.size()
                        ? m_indexCardFieldValues.at(i)
                        : QString();

                cursor = table->cellAt(i, 0).firstCursorPosition();
                cursor.insertText(key);

                cursor = table->cellAt(i, 1).firstCursorPosition();
                cursor.insertText(desc);

                cursor = table->cellAt(i, 2).firstCursorPosition();
                cursor.insertText(value);
            }

            cursor = frame->lastCursorPosition();
        }
    }

    // Comments
    if (options.includeComments) {
        QString comments = this->comments().trimmed();
        if (!comments.isEmpty()) {
            comments.replace(newlinesRegEx, newline);

            comments = QLatin1String("Comments: ") + comments;

            QColor sceneColor = this->color().lighter(175);
            sceneColor.setAlphaF(0.5);

            QTextBlockFormat blockFormat;
            blockFormat.setTopMargin(5);
            blockFormat.setLeftMargin(15);
            blockFormat.setRightMargin(15);
            blockFormat.setBackground(sceneColor);

            QTextCharFormat charFormat;
            charFormat.setFont(textDocument->defaultFont());

            cursor.insertBlock(blockFormat, charFormat);
            TransliterationUtils::polishFontsAndInsertTextAtCursor(cursor, comments);
        }
    }

    // Heading + Content
    if (options.includeContent) {
        QTextFrame *rootFrame = cursor.currentFrame();
        ScreenplayFormat *printFormat = ScriteDocument::instance()->printFormat();

        QTextFrameFormat sceneFrameFormat;
        sceneFrameFormat.setBorder(1);
        sceneFrameFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
        cursor.insertFrame(sceneFrameFormat);

        if (m_heading && m_heading->isEnabled()) {
            SceneElementFormat *headingParaFormat =
                    printFormat->elementFormat(SceneElement::Heading);
            const QTextBlockFormat headingParaBlockFormat =
                    headingParaFormat->createBlockFormat(Qt::Alignment());
            const QTextCharFormat headingParaCharFormat = headingParaFormat->createCharFormat();
            cursor.setBlockFormat(headingParaBlockFormat);
            cursor.setBlockCharFormat(headingParaCharFormat);
            TransliterationUtils::polishFontsAndInsertTextAtCursor(cursor, m_heading->text());
            cursor.insertBlock();
        }

        for (SceneElement *para : m_elements) {
            SceneElementFormat *paraFormat = printFormat->elementFormat(para->type());
            const QTextBlockFormat paraBlockFormat =
                    paraFormat->createBlockFormat(para->alignment());
            const QTextCharFormat paraCharFormat = paraFormat->createCharFormat();
            cursor.setBlockFormat(paraBlockFormat);
            cursor.setBlockCharFormat(paraCharFormat);
            TransliterationUtils::polishFontsAndInsertTextAtCursor(cursor, para->text(),
                                                                   para->textFormats());
            if (para != m_elements.last())
                cursor.insertBlock();
        }

        cursor = rootFrame->lastCursorPosition();
    }

    // Text and Form Notes
    if (options.includeTextNotes || options.includeFormNotes) {
        if (m_notes) {
            Notes::WriteOptions notesOptions;
            notesOptions.includeFormNotes = options.includeFormNotes;
            notesOptions.includeTextNotes = options.includeTextNotes;
            m_notes->write(cursor, notesOptions);
        }
    }
}

bool Scene::event(QEvent *event)
{
    if (m_structureElement == nullptr && event->type() == QEvent::ParentChange) {
        m_structureElement = qobject_cast<StructureElement *>(this->parent());
        emit structureElementChanged();
    }

    return QObject::event(event);
}

void Scene::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_wordCountTimer.timerId()) {
        m_wordCountTimer.stop();
        this->evaluateWordCount();
    } else
        QObject::timerEvent(event);
}

void Scene::setStructureElement(StructureElement *ptr)
{
    if (m_structureElement == ptr)
        return;

    m_structureElement = ptr;

    this->setParent(m_structureElement);

    if (m_structureElement->structure())
        connect(m_structureElement->structure(), &Structure::indexCardFieldsChanged, this,
                &Scene::trimIndexCardFieldValues);

    emit structureElementChanged();
}

void Scene::setElementsList(const QList<SceneElement *> &list)
{
    QScopedValueRollback<bool> isel(m_inSetElementsList, true);

    for (SceneElement *item : list) {
        if (item->scene() != this)
            return;
    }

    if (m_elements == list)
        return;

    const bool sizeChanged = m_elements.size() != list.size();
    QList<SceneElement *> oldElements = m_elements;

    this->beginResetModel();

    m_elements.clear();
    m_elements.reserve(list.size());
    for (SceneElement *item : list) {
        if (!oldElements.removeOne(item))
            this->addElement(item);
        else
            m_elements.append(item);

        if (item->type() == SceneElement::Character)
            m_characterElementMap.include(item);
    }

    while (!oldElements.isEmpty()) {
        SceneElement *ptr = oldElements.takeFirst();
        emit aboutToRemoveSceneElement(ptr);
        if (ptr->type() == SceneElement::Character)
            m_characterElementMap.remove(ptr);
        GarbageCollector::instance()->add(ptr);
    }

    this->endResetModel();

    if (sizeChanged)
        emit elementCountChanged();

    emit sceneChanged();
    emit sceneRefreshed();
}

void Scene::onSceneElementChanged(SceneElement *element, Scene::SceneElementChangeType)
{
    if (m_characterElementMap.include(element))
        this->evaluateSortedCharacterNames();
}

void Scene::onAboutToRemoveSceneElement(SceneElement *element)
{
    if (m_characterElementMap.remove(element))
        this->evaluateSortedCharacterNames();
}

void Scene::renameCharacter(const QString &from, const QString &to)
{
    emit sceneAboutToReset();

    /**
     * This function must be called from Structure::renameCharacter() only.
     *
     * This that function verifies and validates the parameters properly,
     * we don't have to repeat that step in here.
     */

    const bool isFromMute = this->isCharacterMute(from);
    int renamedElementCount = 0;

    // Rename character name in title.
    {
        int nrReplacements = 0;
        const QString newTitle =
                Application::replaceCharacterName(from, to, m_synopsis, &nrReplacements);
        if (nrReplacements > 0) {
            m_synopsis = newTitle;
            emit synopsisChanged();
        }
    }

    // Rename character name in scene heading location
    m_heading->renameCharacter(from, to);

    // Rename character name in paragraphs
    for (SceneElement *element : qAsConst(m_elements))
        element->renameCharacter(from, to);

    if (isFromMute) {
        this->removeMuteCharacter(from);
        this->addMuteCharacter(to);
    }

    // Rename in notes
    if (m_notes)
        m_notes->renameCharacter(from, to);

    // Rename in comments
    {
        int nrReplacements = 0;
        const QString newComments =
                Application::replaceCharacterName(from, to, m_comments, &nrReplacements);
        if (nrReplacements > 0) {
            m_comments = newComments;
            emit commentsChanged();
        }
    }

    if (renamedElementCount > 0 || isFromMute) {
        this->setCharacterRelationshipGraph(QJsonObject());
        this->evaluateSortedCharacterNames();
    }

    emit sceneReset(0);
}

void Scene::evaluateSortedCharacterNames()
{
    const QStringList names = m_characterElementMap.characterNames();

    if (m_structureElement && m_structureElement->structure()) {
        const Structure *structure = m_structureElement->structure();
        m_sortedCharacterNames = structure->sortCharacterNames(names);
    } else
        m_sortedCharacterNames = names;

    emit characterNamesChanged();
}

void Scene::setWordCount(int val)
{
    if (m_wordCount == val)
        return;

    m_wordCount = val;
    emit wordCountChanged();
}

void Scene::evaluateWordCount()
{
    int wordCount = 0;

    if (m_heading->isEnabled())
        wordCount += m_heading->wordCount();

    for (const SceneElement *element : qAsConst(m_elements))
        wordCount += element->wordCount();

    this->setWordCount(wordCount);
}

void Scene::evaluateWordCountLater()
{
    m_wordCountTimer.start(100, this);
}

void Scene::trimIndexCardFieldValues()
{
    if (m_indexCardFieldValues.isEmpty())
        return;

    const int length = m_structureElement && m_structureElement->structure()
            ? m_structureElement->structure()->indexCardFields().size()
            : 0;

    if (m_indexCardFieldValues.size() > length) {
        while (m_indexCardFieldValues.size() > length)
            m_indexCardFieldValues.takeLast();

        emit indexCardFieldValuesChanged();
    }
}

void Scene::evaluateSummary()
{
    QString summary;

    if (m_structureElement != nullptr)
        summary = m_structureElement->nativeTitle();

    if (summary.isEmpty())
        summary = m_synopsis;

    if (summary.isEmpty()) {
        if (!m_elements.isEmpty()) {
            // Find the first element with some text in it.
            SceneElement *element = nullptr;
            int elementIndex = 0;
            while (elementIndex < m_elements.size()) {
                element = m_elements.at(elementIndex);
                if (!element->text().isEmpty())
                    break;
                ++elementIndex;
            }

            if (element != nullptr) {
                if (element->type() == SceneElement::Character) {
                    summary = element->text();
                    if (elementIndex + 1 < m_elements.size())
                        summary += ": " + m_elements.at(elementIndex + 1)->text();
                } else
                    summary = element->text();
            }
        }
    }

    if (summary.isEmpty())
        summary = QStringLiteral("Empty scene");

    this->setSummary(summary);
}

void Scene::staticAppendElement(QQmlListProperty<SceneElement> *list, SceneElement *ptr)
{
    reinterpret_cast<Scene *>(list->data)->addElement(ptr);
}

void Scene::staticClearElements(QQmlListProperty<SceneElement> *list)
{
    reinterpret_cast<Scene *>(list->data)->clearElements();
}

SceneElement *Scene::staticElementAt(QQmlListProperty<SceneElement> *list, int index)
{
    return reinterpret_cast<Scene *>(list->data)->elementAt(index);
}

int Scene::staticElementCount(QQmlListProperty<SceneElement> *list)
{
    return reinterpret_cast<Scene *>(list->data)->elementCount();
}

///////////////////////////////////////////////////////////////////////////////

SceneSizeHintItem::SceneSizeHintItem(QQuickItem *parent)
    : QQuickItem(parent), m_scene(this, "scene"), m_format(this, "format")
{
    this->setFlag(QQuickItem::ItemHasContents, true);

    connect(this, &QQuickItem::visibleChanged, this, &SceneSizeHintItem::updateSizeAndImageLater);
}

SceneSizeHintItem::~SceneSizeHintItem() { }

void SceneSizeHintItem::setScene(Scene *val)
{
    if (m_scene == val)
        return;

    if (m_scene != nullptr) {
        disconnect(m_scene, &Scene::aboutToDelete, this, &SceneSizeHintItem::sceneReset);
        disconnect(m_scene, &Scene::sceneChanged, this, &SceneSizeHintItem::onSceneChanged);
    }

    m_scene = val;

    if (m_scene != nullptr) {
        connect(m_scene, &Scene::aboutToDelete, this, &SceneSizeHintItem::sceneReset);
        if (m_trackSceneChanges)
            connect(m_scene, &Scene::sceneChanged, this, &SceneSizeHintItem::onSceneChanged);
    }

    emit sceneChanged();

    this->updateSizeAndImageLater();
}

void SceneSizeHintItem::setTrackSceneChanges(bool val)
{
    if (m_trackSceneChanges == val)
        return;

    m_trackSceneChanges = val;
    emit trackSceneChangesChanged();

    if (val)
        connect(m_scene, &Scene::sceneChanged, this, &SceneSizeHintItem::onSceneChanged);
    else
        disconnect(m_scene, &Scene::sceneChanged, this, &SceneSizeHintItem::onSceneChanged);
}

void SceneSizeHintItem::setFormat(ScreenplayFormat *val)
{
    if (m_format == val)
        return;

    if (m_format != nullptr) {
        disconnect(m_format, &ScreenplayFormat::destroyed, this, &SceneSizeHintItem::formatReset);
        disconnect(m_format, &ScreenplayFormat::formatChanged, this,
                   &SceneSizeHintItem::onFormatChanged);
    }

    m_format = val;

    if (m_format != nullptr) {
        connect(m_format, &ScreenplayFormat::destroyed, this, &SceneSizeHintItem::formatReset);
        if (m_trackFormatChanges)
            connect(m_format, &ScreenplayFormat::formatChanged, this,
                    &SceneSizeHintItem::onFormatChanged);
    }

    emit formatChanged();

    this->updateSizeAndImageLater();
}

void SceneSizeHintItem::setTrackFormatChanges(bool val)
{
    if (m_trackFormatChanges == val)
        return;

    m_trackFormatChanges = val;
    emit trackFormatChangesChanged();

    if (val)
        connect(m_format, &ScreenplayFormat::formatChanged, this,
                &SceneSizeHintItem::onFormatChanged);
    else
        disconnect(m_format, &ScreenplayFormat::formatChanged, this,
                   &SceneSizeHintItem::onFormatChanged);
}

void SceneSizeHintItem::setAsynchronous(bool val)
{
    if (m_asynchronous == val)
        return;

    m_asynchronous = val;
    emit asynchronousChanged();
}

void SceneSizeHintItem::setActive(bool val)
{
    if (m_active == val)
        return;

    m_active = val;
    emit activeChanged();

    if (m_componentComplete) {
        if (m_active)
            this->updateSizeAndImageLater();
        else {
            m_documentImage = QImage();
            this->update();
        }
    }
}

void SceneSizeHintItem::classBegin()
{
    m_componentComplete = false;
    this->setHasPendingComputeSize(true);
}

void SceneSizeHintItem::componentComplete()
{
    QQuickItem::componentComplete();

    m_componentComplete = true;
    if (m_asynchronous)
        this->updateSizeAndImageLater();
    else
        this->updateSizeAndImageNow();
}

struct SceneSizeHintItem_TaskResult
{
    QSizeF documentSize;
    QImage documentImage;
};
Q_DECLARE_METATYPE(SceneSizeHintItem_TaskResult)

SceneSizeHintItem_TaskResult SceneSizeHintItem_Task2(const qreal devicePixelRatio,
                                                     const Scene *scene,
                                                     const ScreenplayFormat *format,
                                                     bool evaluateSize = true,
                                                     bool evaluateImage = true)
{
    SceneSizeHintItem_TaskResult result;

    const qreal pageWidth = format->pageLayout()->contentWidth();

    QTextDocument document;
    document.setTextWidth(pageWidth);
    document.setDefaultFont(format->defaultFont());

    QTextCursor cursor(&document);

    if (scene->heading()->isEnabled()) {
        const SceneElementFormat *style = format->elementFormat(SceneElement::Heading);
        QTextBlockFormat blockFormat = style->createBlockFormat(Qt::Alignment(), &pageWidth);
        QTextCharFormat charFormat = style->createCharFormat(&pageWidth);
        blockFormat.setTopMargin(0);

        cursor.setCharFormat(charFormat);
        cursor.setBlockFormat(blockFormat);
#if 0
        TransliterationUtils::polishFontsAndInsertTextAtCursor(cursor, scene->heading()->text(),
                                                               QVector<QTextLayout::FormatRange>());
#else
        cursor.insertText(scene->heading()->text());
#endif
        cursor.insertBlock();
    }

    for (int j = 0; j < scene->elementCount(); j++) {
        const SceneElement *para = scene->elementAt(j);
        const SceneElementFormat *style = format->elementFormat(para->type());
        if (j)
            cursor.insertBlock();

        QTextBlockFormat blockFormat = style->createBlockFormat(para->alignment(), &pageWidth);
        QTextCharFormat charFormat = style->createCharFormat(&pageWidth);
        if (!scene->heading()->isEnabled() && j == 0)
            blockFormat.setTopMargin(0);

        cursor.setCharFormat(charFormat);
        cursor.setBlockFormat(blockFormat);
#if 0
        TransliterationUtils::polishFontsAndInsertTextAtCursor(cursor, para->text(),
                                                               para->textFormats());
#else
        cursor.insertText(para->text());
#endif
    }

    const QSizeF docSize = document.size() * devicePixelRatio;

    if (evaluateSize)
        result.documentSize = document.size() * devicePixelRatio;

    if (evaluateImage) {
        result.documentImage = QImage(docSize.toSize(), QImage::Format_ARGB32);
        result.documentImage.fill(Qt::transparent);
        result.documentImage.setDevicePixelRatio(devicePixelRatio);

        QPainter paint(&result.documentImage);
        paint.setRenderHint(QPainter::Antialiasing);
        paint.setRenderHint(QPainter::TextAntialiasing);
        QAbstractTextDocumentLayout::PaintContext context;
        QAbstractTextDocumentLayout *layout = document.documentLayout();
        layout->draw(&paint, context);
        paint.end();
    }

    return result;
}

SceneSizeHintItem_TaskResult SceneSizeHintItem_Task(const qreal devicePixelRatio,
                                                    const QJsonObject &sceneJson,
                                                    const QJsonObject &formatJson,
                                                    bool evaluateSize = true,
                                                    bool evaluateImage = true)
{
    SceneSizeHintItem_TaskResult result;

    Scene scene;
    ScreenplayFormat format;
    format.resetToFactoryDefaults();

    if (!QObjectSerializer::fromJson(formatJson, &format))
        return result;

    if (!QObjectSerializer::fromJson(sceneJson, &scene))
        return result;

    format.pageLayout()->evaluateRectsNow();
    return SceneSizeHintItem_Task2(devicePixelRatio, &scene, &format, evaluateSize, evaluateImage);
}

void SceneSizeHintItem::timerEvent(QTimerEvent *te)
{
    if (te->timerId() == m_updateTimer.timerId()) {
        m_updateTimer.stop();

        const QString watcherName = QStringLiteral("SceneSizeHintItemFutureWatcher");

        QFutureWatcher<SceneSizeHintItem_TaskResult> *watcher =
                this->findChild<QFutureWatcher<SceneSizeHintItem_TaskResult> *>(watcherName);
        if (watcher) {
            this->updateSizeAndImageLater();
            return;
        }

        const QQuickWindow *window = this->window();
        if (!m_componentComplete || !m_active || m_scene == nullptr || m_format == nullptr
            || window == nullptr) {
            this->setContentWidth(0);
            this->setContentHeight(0);
            this->setHasPendingComputeSize(false);
            return;
        }

        if (m_asynchronous) {
            // Since the result travels from background thread to main thread
            static int taskResultTypeId = qRegisterMetaType<SceneSizeHintItem_TaskResult>();
            Q_UNUSED(taskResultTypeId)

            watcher = new QFutureWatcher<SceneSizeHintItem_TaskResult>(this);
            watcher->setObjectName(watcherName);
            connect(watcher, &QFutureWatcher<SceneSizeHintItem_TaskResult>::finished, this, [=]() {
                const SceneSizeHintItem_TaskResult result = watcher->result();
                m_documentImage = result.documentImage;
                this->updateSize(result.documentSize);
                this->update();
            });
            connect(watcher, &QFutureWatcher<SceneSizeHintItem_TaskResult>::finished, watcher,
                    &QObject::deleteLater);

            const QJsonObject sceneJson = QObjectSerializer::toJson(m_scene);
            const QJsonObject formatJson = QObjectSerializer::toJson(m_format);

            QFuture<SceneSizeHintItem_TaskResult> future =
                    QtConcurrent::run(SceneSizeHintItem_Task, window->effectiveDevicePixelRatio(),
                                      sceneJson, formatJson, true, this->isVisible());
            watcher->setFuture(future);
        } else {
            this->updateSizeAndImageNow();
        }
    }
}

QSGNode *SceneSizeHintItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    const qreal w = this->width();
    const qreal h = this->height();

    if (oldNode)
        delete oldNode;

    QSGNode *rootNode = new QSGNode;
    if (m_documentImage.isNull())
        return rootNode; // nothing to show

    QSGGeometryNode *geoNode = new QSGGeometryNode;
    geoNode->setFlags(QSGNode::OwnsGeometry | QSGNode::OwnsMaterial | QSGNode::OwnedByParent);

    QSGGeometry *rectGeo = new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 6);
    rectGeo->setDrawingMode(QSGGeometry::DrawTriangles);
    geoNode->setGeometry(rectGeo);

    QSGGeometry::TexturedPoint2D *rectPoints = rectGeo->vertexDataAsTexturedPoint2D();

    rectPoints[0].set(0.f, 0.f, 0.f, 0.f);
    rectPoints[1].set(float(w), 0.f, 1.f, 0.f);
    rectPoints[2].set(float(w), float(h), 1.f, 1.f);

    rectPoints[3].set(0.f, 0.f, 0.f, 0.f);
    rectPoints[4].set(float(w), float(h), 1.f, 1.f);
    rectPoints[5].set(0.f, float(h), 0.f, 1.f);

    QSGTexture *texture = this->window()->createTextureFromImage(m_documentImage);
    QSGOpaqueTextureMaterial *textureMaterial = new QSGOpaqueTextureMaterial;
    textureMaterial->setFlag(QSGMaterial::Blending);
    textureMaterial->setFiltering(QSGTexture::Nearest);
    textureMaterial->setTexture(texture);
    geoNode->setMaterial(textureMaterial);

    QSGOpacityNode *opacityNode = new QSGOpacityNode;
    opacityNode->setFlag(QSGNode::OwnedByParent);
    opacityNode->setOpacity(this->opacity());
    rootNode->appendChildNode(opacityNode);

    opacityNode->appendChildNode(geoNode);

    return rootNode;
}

void SceneSizeHintItem::updateSize(const QSizeF &size)
{
    this->setContentWidth(size.width());
    this->setContentHeight(size.height());

    if (this->hasPendingComputeSize())
        this->setHasPendingComputeSize(false);
}

void SceneSizeHintItem::updateSizeAndImageLater()
{
    if (m_componentComplete) {
        this->setHasPendingComputeSize(true);
        m_updateTimer.start(0, this);
    }
}

void SceneSizeHintItem::updateSizeAndImageNow()
{
    const QQuickWindow *window = this->window();
    if (!m_componentComplete || !m_active || m_scene == nullptr || m_format == nullptr
        || window == nullptr) {
        this->setContentWidth(0);
        this->setContentHeight(0);
        this->setHasPendingComputeSize(false);
        m_documentImage = QImage();
    } else {
        const SceneSizeHintItem_TaskResult result = SceneSizeHintItem_Task2(
                window->effectiveDevicePixelRatio(), m_scene, m_format, true, this->isVisible());
        m_documentImage = result.documentImage;
        this->updateSize(result.documentSize);
    }

    this->update();
}

void SceneSizeHintItem::sceneReset()
{
    m_scene = nullptr;
    emit sceneChanged();

    this->updateSizeAndImageLater();
}

void SceneSizeHintItem::onSceneChanged()
{
    if (m_trackSceneChanges)
        this->updateSizeAndImageLater();
}

void SceneSizeHintItem::formatReset()
{
    m_format = nullptr;
    emit formatChanged();

    this->updateSizeAndImageLater();
}

void SceneSizeHintItem::onFormatChanged()
{
    if (m_trackFormatChanges)
        this->updateSizeAndImageLater();
}

void SceneSizeHintItem::setContentWidth(qreal val)
{
    if (qFuzzyCompare(m_contentWidth, val))
        return;

    m_contentWidth = val;
    emit contentWidthChanged();
}

void SceneSizeHintItem::setContentHeight(qreal val)
{
    if (qFuzzyCompare(m_contentHeight, val))
        return;

    m_contentHeight = val;
    emit contentHeightChanged();
}

void SceneSizeHintItem::setHasPendingComputeSize(bool val)
{
    if (m_hasPendingComputeSize == val)
        return;

    m_hasPendingComputeSize = val;
    emit hasPendingComputeSizeChanged();
}

///////////////////////////////////////////////////////////////////////////////

SceneGroup::SceneGroup(QObject *parent)
    : GenericArrayModel(parent),
      m_groups(GenericArrayModel::internalArray()),
      m_structure(this, "structure")
{
    connect(this, &SceneGroup::sceneCountChanged, this, &SceneGroup::reevalLater);

    this->setObjectMembers({ "category", "desc", "label", "name", "type", "checked" });
}

SceneGroup::~SceneGroup() { }

void SceneGroup::toggle(int row)
{
    if (row < 0 || row >= m_groups.size() || m_scenes.isEmpty())
        return;

    const QString nameKey = QStringLiteral("name");
    const QString checkedKey = QStringLiteral("checked");
    const QString notCheckedVal = QStringLiteral("no");
    const QString fullyCheckedVal = QStringLiteral("yes");

    QJsonObject item = m_groups.at(row).toObject();
    const QString groupName = item.value(nameKey).toString();

    if (item.value(checkedKey) != fullyCheckedVal) {
        for (Scene *scene : qAsConst(m_scenes)) {
            disconnect(scene, &Scene::groupsChanged, this, &SceneGroup::reevalLater);
            scene->addToGroup(groupName);
            connect(scene, &Scene::groupsChanged, this, &SceneGroup::reevalLater);
        }

        item.insert(checkedKey, fullyCheckedVal);
    } else {
        for (Scene *scene : qAsConst(m_scenes)) {
            disconnect(scene, &Scene::groupsChanged, this, &SceneGroup::reevalLater);
            scene->removeFromGroup(groupName);
            connect(scene, &Scene::groupsChanged, this, &SceneGroup::reevalLater);
        }

        item.insert(checkedKey, notCheckedVal);
    }

    m_groups.replace(row, item);

    const QModelIndex index = this->index(row, 0);
    emit dataChanged(index, index);

    emit toggled(row);
}

void SceneGroup::setStructure(Structure *val)
{
    if (m_structure == val)
        return;

    if (!m_structure.isNull())
        m_structure->disconnect(this);

    m_structure = val;
    emit structureChanged();

    this->reload();

    if (!m_structure.isNull())
        connect(m_structure, &Structure::groupsModelChanged, this, &SceneGroup::reload);
}

QQmlListProperty<Scene> SceneGroup::scenes()
{
    return QQmlListProperty<Scene>(reinterpret_cast<QObject *>(this), static_cast<void *>(this),
                                   &SceneGroup::staticAppendScene, &SceneGroup::staticSceneCount,
                                   &SceneGroup::staticSceneAt, &SceneGroup::staticClearScenes);
}

void SceneGroup::addScene(Scene *ptr)
{
    if (ptr == nullptr || m_scenes.indexOf(ptr) >= 0)
        return;

    connect(ptr, &Scene::aboutToDelete, this, &SceneGroup::removeScene);
    connect(ptr, &Scene::groupsChanged, this, &SceneGroup::reevalLater);
    m_scenes.append(ptr);
    emit sceneCountChanged();
}

void SceneGroup::removeScene(Scene *ptr)
{
    if (ptr == nullptr)
        return;

    const int index = m_scenes.indexOf(ptr);
    if (index < 0)
        return;

    disconnect(ptr, &Scene::aboutToDelete, this, &SceneGroup::removeScene);
    disconnect(ptr, &Scene::groupsChanged, this, &SceneGroup::reevalLater);
    m_scenes.removeAt(index);
    emit sceneCountChanged();
}

Scene *SceneGroup::sceneAt(int index) const
{
    return index < 0 || index >= m_scenes.size() ? nullptr : m_scenes.at(index);
}

void SceneGroup::clearScenes()
{
    while (m_scenes.size())
        this->removeScene(m_scenes.first());
}

void SceneGroup::timerEvent(QTimerEvent *te)
{
    if (te->timerId() == m_reevalTimer.timerId()) {
        m_reevalTimer.stop();
        this->reeval();
    } else
        GenericArrayModel::timerEvent(te);
}

void SceneGroup::setSceneActs(const QStringList &val)
{
    if (m_sceneActs == val)
        return;

    m_sceneActs = val;
    emit sceneActsChanged();
}

void SceneGroup::setGroupActs(const QStringList &val)
{
    if (m_groupActs == val)
        return;

    m_groupActs = val;
    emit groupActsChanged();
}

void SceneGroup::setSceneStackIds(const QStringList &val)
{
    if (m_sceneStackIds == val)
        return;

    m_sceneStackIds = val;
    emit sceneStackIdsChanged();
}

void SceneGroup::reload()
{
    this->beginResetModel();

    QStringList acts;

    m_groups = QJsonArray();
    if (!m_structure.isNull()) {
        const QJsonArray array = m_structure->groupsModel();
        for (int i = 0; i < array.size(); i++) {
            QJsonObject item = array.at(i).toObject();
            item.insert(QStringLiteral("checked"), QStringLiteral("no"));

            const QString act = item.value(QStringLiteral("act")).toString();
            if (!acts.contains(act))
                acts.append(act);

            m_groups.append(item);
        }
    }

    this->endResetModel();

    this->setGroupActs(acts);

    this->reeval();
}

void SceneGroup::reeval()
{
    QMap<QString, int> groupCounter;
    QStringList acts;
    QSet<QString> stackIds;

    for (Scene *scene : qAsConst(m_scenes)) {
        const QStringList groups = scene->groups();
        for (const QString &group : groups)
            groupCounter[group] = groupCounter.value(group, 0) + 1;

        const QString sceneAct = scene->act();
        if (!sceneAct.isEmpty() && m_groupActs.contains(sceneAct) && !acts.contains(sceneAct))
            acts.append(sceneAct);

        if (m_structure != nullptr) {
            const int eindex = m_structure->indexOfScene(scene);
            const StructureElement *element = m_structure->elementAt(eindex);
            if (element != nullptr) {
                const QString stackId = element->stackId();
                if (!stackId.isEmpty())
                    stackIds += stackId;
            }
        }
    }

    const QString nameKey = QStringLiteral("name");
    const QString checkedKey = QStringLiteral("checked");
    const QString notCheckedVal = QStringLiteral("no");
    const QString partiallyCheckedVal = QStringLiteral("partial");
    const QString fullyCheckedVal = QStringLiteral("yes");

    for (int i = 0; i < m_groups.size(); i++) {
        const QModelIndex index = this->index(i, 0);

        QJsonObject item = m_groups.at(i).toObject();

        const QString name = item.value(nameKey).toString();
        QString checkedVal = notCheckedVal;

        if (groupCounter.contains(name))
            checkedVal = groupCounter.value(name) == m_scenes.size() ? fullyCheckedVal
                                                                     : partiallyCheckedVal;

        if (item.value(checkedKey) != checkedVal) {
            item.insert(checkedKey, checkedVal);
            m_groups.replace(i, item);
            emit dataChanged(index, index);
        }
    }

    this->setSceneActs(acts);
    this->setSceneStackIds(stackIds.values());
}

void SceneGroup::reevalLater()
{
    m_reevalTimer.start(0, this);
}

void SceneGroup::staticAppendScene(QQmlListProperty<Scene> *list, Scene *ptr)
{
    reinterpret_cast<SceneGroup *>(list->data)->addScene(ptr);
}

void SceneGroup::staticClearScenes(QQmlListProperty<Scene> *list)
{
    reinterpret_cast<SceneGroup *>(list->data)->clearScenes();
}

Scene *SceneGroup::staticSceneAt(QQmlListProperty<Scene> *list, int index)
{
    return reinterpret_cast<SceneGroup *>(list->data)->sceneAt(index);
}

int SceneGroup::staticSceneCount(QQmlListProperty<Scene> *list)
{
    return reinterpret_cast<SceneGroup *>(list->data)->sceneCount();
}
