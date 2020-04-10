/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "logger.h"
#include "formatting.h"
#include "application.h"
#include "scritedocument.h"
#include "qobjectserializer.h"
#include "qobjectserializer.h"

#include <QPointer>
#include <QMetaEnum>
#include <QTextCursor>
#include <QTextBlockUserData>

SceneElementFormat::SceneElementFormat(SceneElement::Type type, ScreenplayFormat *parent)
                   : QObject(parent),
                     m_font(parent->defaultFont()),
                     m_topMargin(20),
                     m_blockWidth(1.0),
                     m_lineHeight(1.0),
                     m_textColor(Qt::black),
                     m_bottomMargin(0),
                     m_backgroundColor(Qt::transparent),
                     m_format(parent),
                     m_textAlignment(Qt::AlignLeft),
                     m_blockAlignment(Qt::AlignHCenter),
                     m_elementType(type)
{
    CACHE_DEFAULT_PROPERTY_VALUES
}

SceneElementFormat::~SceneElementFormat()
{

}

void SceneElementFormat::setFont(const QFont &val)
{
    if(m_font == val)
        return;

    m_font = val;
    emit fontChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setTextColor(const QColor &val)
{
    if(m_textColor == val)
        return;

    m_textColor = val;
    emit textColorChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setBackgroundColor(const QColor &val)
{
    if(m_backgroundColor == val)
        return;

    QColor val2 = val;
    val2.setAlphaF(1);
    if(val2 == Qt::black || val2 == Qt::white)
        val2 = Qt::transparent;
    else
        val2.setAlphaF(0.25);

    m_backgroundColor = val2;
    emit backgroundColorChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setTextAlignment(Qt::Alignment val)
{
    if(m_textAlignment == val)
        return;

    m_textAlignment = val;
    emit textAlignmentChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setBlockWidth(qreal val)
{
    val = qBound(0.1, val, 1.0);
    if( qFuzzyCompare(m_blockWidth, val) )
        return;

    m_blockWidth = val;
    emit blockWidthChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setBlockAlignment(Qt::Alignment val)
{
    if(m_blockAlignment == val)
        return;

    m_blockAlignment = val;
    emit blockAlignmentChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setTopMargin(qreal val)
{
    if( qFuzzyCompare(m_topMargin, val) )
        return;

    m_topMargin = val;
    emit topMarginChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setBottomMargin(qreal val)
{
    if( qFuzzyCompare(m_bottomMargin, val) )
        return;

    m_bottomMargin = val;
    emit bottomMarginChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setLineHeight(qreal val)
{
    if( qFuzzyCompare(m_lineHeight, val) )
        return;

    m_lineHeight = val;
    emit lineHeightChanged();
    emit elementFormatChanged();
}

QTextBlockFormat SceneElementFormat::createBlockFormat(const qreal *givenPageWidth) const
{
    const qreal pageWidth = givenPageWidth ? *givenPageWidth : m_format->pageWidth();
    const qreal blockWidthInPixels = pageWidth * m_blockWidth;
    const qreal fullMargin = (pageWidth - blockWidthInPixels);
    const qreal halfMargin = fullMargin*0.5;
    const qreal leftMargin = m_blockAlignment.testFlag(Qt::AlignLeft) ? 0 : (m_blockAlignment.testFlag(Qt::AlignHCenter) ? halfMargin : fullMargin);
    const qreal rightMargin = pageWidth - blockWidthInPixels - leftMargin;

    QTextBlockFormat format;
    format.setLeftMargin(leftMargin);
    format.setRightMargin(rightMargin);
    format.setTopMargin(m_topMargin);
    format.setBottomMargin(m_bottomMargin);
    format.setLineHeight(m_lineHeight*100, QTextBlockFormat::ProportionalHeight);
    format.setAlignment(m_textAlignment);
    format.setBackground(QBrush(m_backgroundColor));
    format.setForeground(QBrush(m_textColor));

    return format;
}

QTextCharFormat SceneElementFormat::createCharFormat(const qreal *givenPageWidth) const
{
    Q_UNUSED(givenPageWidth)

    QTextCharFormat format;

    // It turns out that format.setFont()
    // doesnt actually do all of the below.
    // So, we will have to do it explicitly
    format.setFontFamily(m_font.family());
    format.setFontItalic(m_font.italic());
    format.setFontWeight(m_font.weight());
    // format.setFontKerning(m_font.kerning());
    format.setFontStretch(m_font.stretch());
    format.setFontOverline(m_font.overline());
    format.setFontPointSize(m_font.pointSize());
    format.setFontStrikeOut(m_font.strikeOut());
    // format.setFontStyleHint(m_font.styleHint());
    // format.setFontStyleName(m_font.styleName());
    format.setFontUnderline(m_font.underline());
    // format.setFontFixedPitch(m_font.fixedPitch());
    format.setFontWordSpacing(m_font.wordSpacing());
    format.setFontLetterSpacing(m_font.letterSpacing());
    // format.setFontStyleStrategy(m_font.styleStrategy());
    format.setFontCapitalization(m_font.capitalization());
    // format.setFontHintingPreference(m_font.hintingPreference());
    format.setFontLetterSpacingType(m_font.letterSpacingType());

    format.setBackground(QBrush(m_backgroundColor));
    format.setForeground(QBrush(m_textColor));

    return format;
}

///////////////////////////////////////////////////////////////////////////////

ScreenplayFormat::ScreenplayFormat(QObject *parent)
    : QAbstractListModel(parent),
      m_screen(nullptr),
      m_pageWidth(750),
      m_defaultFont(QFont("Courier New", 10)),
      m_scriteDocument(qobject_cast<ScriteDocument*>(parent))
{
    for(int i=SceneElement::Min; i<=SceneElement::Max; i++)
    {
        SceneElementFormat *elementFormat = new SceneElementFormat(SceneElement::Type(i), this);
        connect(elementFormat, &SceneElementFormat::elementFormatChanged, this, &ScreenplayFormat::formatChanged);
        m_elementFormats.append(elementFormat);
    }

    m_elementFormats[SceneElement::Action]->setTextAlignment(Qt::AlignJustify);

    m_elementFormats[SceneElement::Character]->setBlockWidth(0.6);
    m_elementFormats[SceneElement::Character]->setTextAlignment(Qt::AlignHCenter);
    m_elementFormats[SceneElement::Character]->setTopMargin(30);
    m_elementFormats[SceneElement::Character]->fontRef().setBold(true);
    m_elementFormats[SceneElement::Character]->fontRef().setCapitalization(QFont::AllUppercase);

    m_elementFormats[SceneElement::Dialogue]->setBlockWidth(0.6);
    m_elementFormats[SceneElement::Dialogue]->setTextAlignment(Qt::AlignJustify);
    m_elementFormats[SceneElement::Dialogue]->setTopMargin(0);

    m_elementFormats[SceneElement::Parenthetical]->setBlockWidth(0.5);
    m_elementFormats[SceneElement::Parenthetical]->setTextAlignment(Qt::AlignHCenter);
    m_elementFormats[SceneElement::Parenthetical]->fontRef().setItalic(true);
    m_elementFormats[SceneElement::Parenthetical]->setTopMargin(0);

    m_elementFormats[SceneElement::Shot]->setTextAlignment(Qt::AlignRight);
    m_elementFormats[SceneElement::Shot]->fontRef().setCapitalization(QFont::AllUppercase);

    m_elementFormats[SceneElement::Transition]->setTextAlignment(Qt::AlignRight);
    m_elementFormats[SceneElement::Transition]->fontRef().setCapitalization(QFont::AllUppercase);

    m_elementFormats[SceneElement::Heading]->fontRef().setBold(true);
    m_elementFormats[SceneElement::Heading]->fontRef().setPointSize(m_defaultFont.pointSize()+2);
    m_elementFormats[SceneElement::Heading]->fontRef().setCapitalization(QFont::AllUppercase);
}

ScreenplayFormat::~ScreenplayFormat()
{

}

void ScreenplayFormat::setScreen(QScreen *val)
{
    if(m_screen == val || m_screen != nullptr)
        return;

    const qreal a4PageWidthInInches = 8.3;

    m_screen = val;
    m_pageWidth = m_screen->logicalDotsPerInchX() * a4PageWidthInInches;
    emit screenChanged();
}

void ScreenplayFormat::setDefaultFont(const QFont &val)
{
    if(m_defaultFont == val)
        return;

    m_defaultFont = val;
    emit defaultFontChanged();
}

SceneElementFormat *ScreenplayFormat::elementFormat(SceneElement::Type type) const
{
    return m_elementFormats.at(int(type));
}

int ScreenplayFormat::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_elementFormats.size();
}

QVariant ScreenplayFormat::data(const QModelIndex &index, int role) const
{
    if(role == SceneElementFomat && index.isValid())
        return QVariant::fromValue<QObject*>( qobject_cast<QObject*>(m_elementFormats.at(index.row())) );

    return QVariant();
}

QHash<int, QByteArray> ScreenplayFormat::roleNames() const
{
    QHash<int,QByteArray> roles;
    roles[SceneElementFomat] = "sceneElementFormat";
    return roles;
}

///////////////////////////////////////////////////////////////////////////////

class SceneDocumentBlockUserData : public QTextBlockUserData
{
public:
    SceneDocumentBlockUserData(SceneElement *element);
    ~SceneDocumentBlockUserData();

    SceneElement *sceneElement() const { return m_sceneElement; }

    static SceneDocumentBlockUserData *get(const QTextBlock &block);
    static SceneDocumentBlockUserData *get(QTextBlockUserData *userData);

private:
    QPointer<SceneElement> m_sceneElement;
};

SceneDocumentBlockUserData::SceneDocumentBlockUserData(SceneElement *element)
    : m_sceneElement(element) { }
SceneDocumentBlockUserData::~SceneDocumentBlockUserData() { }

SceneDocumentBlockUserData *SceneDocumentBlockUserData::get(const QTextBlock &block)
{
    return get(block.userData());
}

SceneDocumentBlockUserData *SceneDocumentBlockUserData::get(QTextBlockUserData *userData)
{
    if(userData == nullptr)
        return nullptr;

    SceneDocumentBlockUserData *userData2 = reinterpret_cast<SceneDocumentBlockUserData*>(userData);
    return userData2;
}

SceneDocumentBinder::SceneDocumentBinder(QObject *parent)
    : QSyntaxHighlighter(parent),
      m_scene(nullptr),
      m_textWidth(0),
      m_cursorPosition(-1),
      m_documentLoadCount(0),
      m_forceSyncDocument(false),
      m_initializingDocument(false),
      m_currentElement(nullptr),
      m_textDocument(nullptr),
      m_screenplayFormat(nullptr)
{

}

SceneDocumentBinder::~SceneDocumentBinder()
{

}

void SceneDocumentBinder::setScreenplayFormat(ScreenplayFormat *val)
{
    if(m_screenplayFormat == val)
        return;

    if(m_screenplayFormat != nullptr)
        disconnect(m_screenplayFormat, &ScreenplayFormat::formatChanged,
                this, &QSyntaxHighlighter::rehighlight);

    m_screenplayFormat = val;
    if(m_screenplayFormat != nullptr)
    {
        connect(m_screenplayFormat, &ScreenplayFormat::formatChanged,
                this, &QSyntaxHighlighter::rehighlight);

        if( qFuzzyCompare(m_textWidth,0.0) )
            this->setTextWidth(m_screenplayFormat->pageWidth());
    }

    emit screenplayFormatChanged();

    this->initializeDocument();
}

void SceneDocumentBinder::setScene(Scene *val)
{
    if(m_scene == val)
        return;

    if(m_scene != nullptr)
    {
        disconnect(m_scene, &Scene::sceneElementChanged,
                   this, &SceneDocumentBinder::onSceneElementChanged);
        disconnect(m_scene, &Scene::sceneAboutToReset,
                   this, &SceneDocumentBinder::onSceneAboutToReset);
        disconnect(m_scene, &Scene::sceneReset,
                   this, &SceneDocumentBinder::onSceneReset);
    }

    m_scene = val;

    if(m_scene != nullptr && this->document() != nullptr)
    {
        connect(m_scene, &Scene::sceneElementChanged,
                this, &SceneDocumentBinder::onSceneElementChanged);
        connect(m_scene, &Scene::sceneAboutToReset,
                this, &SceneDocumentBinder::onSceneAboutToReset);
        connect(m_scene, &Scene::sceneReset,
                this, &SceneDocumentBinder::onSceneReset);
    }

    emit sceneChanged();

    this->initializeDocument();
}

void SceneDocumentBinder::setTextDocument(QQuickTextDocument *val)
{
    if(m_textDocument == val)
        return;

    if(this->document() != nullptr)
    {
        this->document()->setUndoRedoEnabled(true);

        disconnect( this->document(), &QTextDocument::contentsChange,
                    this, &SceneDocumentBinder::onContentsChange);
        disconnect( this->document(), &QTextDocument::blockCountChanged,
                    this, &SceneDocumentBinder::syncSceneFromDocument);

        if(m_scene != nullptr)
            disconnect(m_scene, &Scene::sceneElementChanged,
                       this, &SceneDocumentBinder::onSceneElementChanged);

        this->setCurrentElement(nullptr);
    }

    m_textDocument = val;
    if(m_textDocument != nullptr)
        this->QSyntaxHighlighter::setDocument(m_textDocument->textDocument());
    else
        this->QSyntaxHighlighter::setDocument(nullptr);

    this->evaluateAutoCompleteHints();

    emit textDocumentChanged();

    this->initializeDocument();

    if(m_textDocument != nullptr)
    {
        this->document()->setUndoRedoEnabled(false);

        connect(this->document(), &QTextDocument::contentsChange,
                this, &SceneDocumentBinder::onContentsChange);
        connect(this->document(), &QTextDocument::blockCountChanged,
                    this, &SceneDocumentBinder::syncSceneFromDocument);

        if(m_scene != nullptr)
            connect(m_scene, &Scene::sceneElementChanged,
                    this, &SceneDocumentBinder::onSceneElementChanged);

#if 0 // At the moment, this seems to be causing more trouble than help.
        this->document()->setTextWidth(m_textWidth);
#endif

        this->setCursorPosition(0);
    }
    else
        this->setCursorPosition(-1);
}

void SceneDocumentBinder::setTextWidth(qreal val)
{
    if( qFuzzyCompare(m_textWidth, val) )
        return;

    m_textWidth = val;
#if 0 // At the moment, this seems to be causing more trouble than help.
    if(this->document() != nullptr)
        this->document()->setTextWidth(m_textWidth);
#endif

    emit textWidthChanged();
}

void SceneDocumentBinder::setCursorPosition(int val)
{
    if(m_initializingDocument)
        return;

    if(m_cursorPosition == val || m_textDocument == nullptr || this->document() == nullptr)
        return;

    m_cursorPosition = val;

    QTextCursor cursor(this->document());
    cursor.setPosition(val);

    QTextBlock block = cursor.block();
    if(!block.isValid())
    {
        qDebug("[%d] There is no block at the cursor position.", __LINE__);
        return;
    }

    SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
    if(userData == nullptr)
    {
        this->syncSceneFromDocument();
        userData = SceneDocumentBlockUserData::get(block);
    }

    if(userData == nullptr)
    {
        this->setCurrentElement(nullptr);
        qWarning("[%d] TextDocument has a block that isnt backed by a SceneElement!!", __LINE__);
    }
    else
    {
        this->setCurrentElement(userData->sceneElement());
        if(!m_autoCompleteHints.isEmpty())
            this->setCompletionPrefix(block.text());
    }

    emit cursorPositionChanged();
}

void SceneDocumentBinder::setCharacterNames(const QStringList &val)
{
    if(m_characterNames == val)
        return;

    m_characterNames = val;
    emit characterNamesChanged();
}

void SceneDocumentBinder::setForceSyncDocument(bool val)
{
    if(m_forceSyncDocument == val)
        return;

    m_forceSyncDocument = val;
    emit forceSyncDocumentChanged();
}

void SceneDocumentBinder::tab()
{
    if(m_cursorPosition < 0 || m_textDocument == nullptr || m_currentElement == nullptr || this->document() == nullptr)
        return;

    const int elementNr = m_scene->indexOfElement(m_currentElement);
    if(elementNr < 0)
        return;

    switch(m_currentElement->type())
    {
    case SceneElement::Action:
        m_currentElement->setType(SceneElement::Character);
        break;
    case SceneElement::Character:
        if(m_tabHistory.isEmpty())
            m_currentElement->setType(SceneElement::Action);
        else
            m_currentElement->setType(SceneElement::Transition);
        break;
    case SceneElement::Dialogue:
        m_currentElement->setType(SceneElement::Parenthetical);
        break;
    case SceneElement::Parenthetical:
        m_currentElement->setType(SceneElement::Dialogue);
        break;
    case SceneElement::Shot:
        m_currentElement->setType(SceneElement::Transition);
        break;
    case SceneElement::Transition:
        m_currentElement->setType(SceneElement::Action);
        break;
    default:
        break;
    }

    m_tabHistory.append(m_currentElement->type());
}

void SceneDocumentBinder::backtab()
{
    // Do nothing. It doesnt work anyway!
}

bool SceneDocumentBinder::canGoUp()
{
    if(m_cursorPosition < 0 || this->document() == nullptr)
        return false;

    QTextCursor cursor(this->document());
    cursor.setPosition(m_cursorPosition);
    return cursor.movePosition(QTextCursor::Up);
}

bool SceneDocumentBinder::canGoDown()
{
    if(m_cursorPosition < 0 || this->document() == nullptr)
        return false;

    QTextCursor cursor(this->document());
    cursor.setPosition(m_cursorPosition);
    return cursor.movePosition(QTextCursor::Down);
}

int SceneDocumentBinder::lastCursorPosition() const
{
    if(m_cursorPosition < 0 || this->document() == nullptr)
        return 0;

    QTextCursor cursor(this->document());
    cursor.setPosition(m_cursorPosition);
    cursor.movePosition(QTextCursor::End);
    return cursor.position();
}

int SceneDocumentBinder::cursorPositionAtBlock(int blockNumber) const
{
    if(this->document() != nullptr)
    {
        const QTextBlock block = this->document()->findBlockByNumber(blockNumber);
        if( m_cursorPosition >= block.position() && m_cursorPosition < block.position()+block.length() )
            return m_cursorPosition;

        return block.position()+block.length()-1;
    }

    return -1;
}

QFont SceneDocumentBinder::currentFont() const
{
    if(this->document() == nullptr)
        return QFont();

    QTextCursor cursor(this->document());
    cursor.setPosition(m_cursorPosition);

    QTextCharFormat format = cursor.charFormat();
    return format.font();
}

void SceneDocumentBinder::highlightBlock(const QString &text)
{
    Q_UNUSED(text)

    if(m_initializingDocument)
        return;

    if(m_screenplayFormat == nullptr)
        return;

    QTextBlock block = this->QSyntaxHighlighter::currentBlock();
    SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
    if(userData == nullptr)
    {
        this->syncSceneFromDocument();
        userData = SceneDocumentBlockUserData::get(block);
    }

    if(userData == nullptr)
    {
        qWarning("[%d] TextDocument has a block that isnt backed by a SceneElement!!", __LINE__);
        return;
    }

    SceneElement *element = userData->sceneElement();
    if(element == nullptr)
    {
        qWarning("[%d] TextDocument has a block that isnt backed by a SceneElement!!", __LINE__);
        return;
    }

    SceneElementFormat *format = m_screenplayFormat->elementFormat(element->type());
    QTextBlockFormat blkFormat = format->createBlockFormat();
    QTextCharFormat chrFormat = format->createCharFormat();
    chrFormat.setFontPointSize(format->font().pointSize()+8);

    QTextCursor cursor(block);
    cursor.setBlockFormat(blkFormat);
    cursor.setPosition(block.position(), QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    cursor.setCharFormat(chrFormat);

    if(m_currentElement == element)
        emit currentFontChanged();
}

void SceneDocumentBinder::timerEvent(QTimerEvent *te)
{
    if(te->timerId() == m_initializeDocumentTimer.timerId())
    {
        this->initializeDocument();
        m_initializeDocumentTimer.stop();
    }
}

void SceneDocumentBinder::initializeDocument()
{
    if(m_textDocument == nullptr || m_scene == nullptr || m_screenplayFormat == nullptr)
        return;

    m_initializingDocument = true;

    m_tabHistory.clear();

    const QFont defaultFont = m_screenplayFormat->defaultFont();

    QTextDocument *document = m_textDocument->textDocument();
    document->blockSignals(true);
    document->clear();
    document->setDefaultFont(defaultFont);

    const int nrElements = m_scene->elementCount();

    QTextCursor cursor(document);
    for(int i=0; i<nrElements; i++)
    {
        SceneElement *element = m_scene->elementAt(i);
        if(i > 0)
            cursor.insertBlock();

        QTextBlock block = cursor.block();
        block.setUserData(new SceneDocumentBlockUserData(element));
        cursor.insertText(element->text());
    }
    document->blockSignals(false);

    this->setDocumentLoadCount(m_documentLoadCount+1);
    m_initializingDocument = false;
    this->QSyntaxHighlighter::rehighlight();
    emit documentInitialized();
}

void SceneDocumentBinder::initializeDocumentLater()
{
    m_initializeDocumentTimer.start(100, this);
}

void SceneDocumentBinder::setDocumentLoadCount(int val)
{
    if(m_documentLoadCount == val)
        return;

    m_documentLoadCount = val;
    emit documentLoadCountChanged();
}

void SceneDocumentBinder::setCurrentElement(SceneElement *val)
{
    if(m_currentElement == val)
        return;

    m_currentElement = val;
    emit currentElementChanged();

    m_tabHistory.clear();
    this->evaluateAutoCompleteHints();

    emit currentFontChanged();
}

void SceneDocumentBinder::onSceneElementChanged(SceneElement *element, Scene::SceneElementChangeType type)
{
    if(m_initializingDocument)
        return;

    if(m_textDocument == nullptr || this->document() == nullptr || m_scene == nullptr || element->scene() != m_scene)
        return;

    if(m_forceSyncDocument)
        this->initializeDocumentLater();

    if(type != Scene::ElementTypeChange)
        return;

    this->evaluateAutoCompleteHints();

    auto updateBlock = [=](const QTextBlock &block) {
        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
        if(userData != nullptr && userData->sceneElement() == element) {
            // Text changes from scene element to block are not applied
            // Only element type changes can be applied.
            this->rehighlightBlock(block);
            return true;
        }
        return false;
    };

    const int elementNr = m_scene->indexOfElement(element);
    QTextBlock block;

    if(elementNr >= 0)
    {
        block = this->document()->findBlockByNumber(elementNr);
        if(updateBlock(block))
            return;
    }

    block = this->document()->firstBlock();
    while(block.isValid())
    {
        if( updateBlock(block) )
            return;

        block = block.next();
    }
}

void SceneDocumentBinder::onContentsChange(int from, int charsRemoved, int charsAdded)
{
    if(m_initializingDocument || m_sceneIsBeingReset)
        return;

    Q_UNUSED(charsRemoved)
    Q_UNUSED(charsAdded)

    if(m_textDocument == nullptr || m_scene == nullptr || this->document() == nullptr)
        return;

    QTextCursor cursor(this->document());
    cursor.setPosition(from);

    QTextBlock block = cursor.block();
    SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
    if(userData == nullptr)
    {
        this->syncSceneFromDocument();
        return;
    }

    if(userData == nullptr)
    {
        qWarning("[%d] TextDocument has a block that isnt backed by a SceneElement!!", __LINE__);
        return;
    }

    SceneElement *sceneElement = userData->sceneElement();
    if(sceneElement == nullptr)
    {
        qWarning("[%d] TextDocument has a block that isnt backed by a SceneElement!!", __LINE__);
        return;
    }

    sceneElement->setText(block.text());
    m_tabHistory.clear();
}

void SceneDocumentBinder::syncSceneFromDocument(int nrBlocks)
{
    if(m_initializingDocument || m_sceneIsBeingReset)
        return;

    if(m_textDocument == nullptr || m_scene == nullptr)
        return;

    if(nrBlocks < 0)
        nrBlocks = this->document()->blockCount();

    /*
     * Ensure that blocks on the QTextDocument are in sync with
     * SceneElements in the Scene. I know that we are using a for loop
     * to make this happen, so we are (many-times) needlessly looping
     * over blocks that have already been touched, thereby making
     * this function slow. Still, I feel that this is better. A scene
     * would not have more than a few blocks, atbest 100 blocks.
     * So its better we sync it like this.
     */

    QList<SceneElement*> elementList;
    elementList.reserve(nrBlocks);

    QTextBlock block = this->document()->begin();
    QTextBlock previousBlock;
    while(block.isValid())
    {
        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
        if(userData == nullptr)
        {
            SceneElement *newElement = new SceneElement(m_scene);

            if(previousBlock.isValid())
            {
                SceneDocumentBlockUserData *prevUserData = SceneDocumentBlockUserData::get(previousBlock);
                SceneElement *prevElement = prevUserData->sceneElement();
                switch(prevElement->type())
                {
                case SceneElement::Action:
                    newElement->setType(SceneElement::Action);
                    break;
                case SceneElement::Character:
                    newElement->setType(SceneElement::Dialogue);
                    break;
                case SceneElement::Dialogue:
                    newElement->setType(SceneElement::Character);
                    break;
                case SceneElement::Parenthetical:
                    newElement->setType(SceneElement::Dialogue);
                    break;
                case SceneElement::Shot:
                    newElement->setType(SceneElement::Action);
                    break;
                case SceneElement::Transition:
                    newElement->setType(SceneElement::Action);
                    break;
                default:
                    newElement->setType(SceneElement::Action);
                    break;
                }

                m_scene->insertElementAfter(newElement, prevElement);
            }
            else
            {
                newElement->setType(SceneElement::Action);
                m_scene->insertElementAt(newElement, 0);
            }

            userData = new SceneDocumentBlockUserData(newElement);
            block.setUserData(userData);
        }

        elementList.append(userData->sceneElement());
        userData->sceneElement()->setText(block.text());

        previousBlock = block;
        block = block.next();
    }

    m_scene->setElementsList(elementList);
}

void SceneDocumentBinder::evaluateAutoCompleteHints()
{
    QStringList hints;

    if(m_currentElement == nullptr)
    {
        this->setAutoCompleteHints(hints);
        return;
    }

    static QStringList transitions = QStringList() <<
            "CUT TO" <<
            "DISSOLVE TO" <<
            "FADE IN" <<
            "FADE OUT" <<
            "FADE TO" <<
            "FLASH CUT TO" <<
            "FREEZE FRAME" <<
            "IRIS IN" <<
            "IRIS OUT" <<
            "JUMP CUT TO" <<
            "MATCH CUT TO" <<
            "MATCH DISSOLVE TO" <<
            "SMASH CUT TO" <<
            "STOCK SHOT" <<
            "TIME CUT" <<
            "WIPE TO";

    static QStringList shots = QStringList() <<
            "AIR" <<
            "CLOSE ON" <<
            "CLOSER ON" <<
            "CLOSEUP" <<
            "ESTABLISHING" <<
            "EXTREME CLOSEUP" <<
            "INSERT" <<
            "POV" <<
            "SURFACE" <<
            "THREE SHOT" <<
            "TWO SHOT" <<
            "UNDERWATER" <<
            "WIDE" <<
            "WIDE ON" <<
            "WIDER ANGLE";

    switch(m_currentElement->type())
    {
    case SceneElement::Character:
        hints = m_characterNames;
        break;
    case SceneElement::Transition:
        hints = transitions;
        break;
    case SceneElement::Shot:
        hints = shots;
        break;
    default:
        break;
    }

    this->setAutoCompleteHints(hints);
}

void SceneDocumentBinder::setAutoCompleteHints(const QStringList &val)
{
    if(m_autoCompleteHints == val)
        return;

    m_autoCompleteHints = val;
    emit autoCompleteHintsChanged();
}

void SceneDocumentBinder::setCompletionPrefix(const QString &val)
{
    if(m_completionPrefix == val)
        return;

    m_completionPrefix = val;
    emit completionPrefixChanged();
}

void SceneDocumentBinder::onSceneAboutToReset()
{
    m_sceneIsBeingReset = true;
}

void SceneDocumentBinder::onSceneReset(int)
{
    m_initializeDocumentTimer.start(0, this);

#if 0
    if(this->document() != nullptr)
    {
        const QTextBlock block = this->document()->findBlockByNumber(elementIndex);
        if( m_cursorPosition >= block.position() && m_cursorPosition < block.position()+block.length() )
            emit requestCursorPosition(m_cursorPosition);
        else
            emit requestCursorPosition(block.position()+block.length()-1);
    }
#else
#endif

    m_sceneIsBeingReset = false;
}

