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

#include "fountain.h"
#include "appwindow.h"
#include "formatting.h"
#include "application.h"
#include "scritedocument.h"
#include "qobjectserializer.h"
#include "qobjectserializer.h"

#include <QPointer>
#include <QMarginsF>
#include <QSettings>
#include <QMetaEnum>
#include <QMimeData>
#include <QClipboard>
#include <QPdfWriter>
#include <QScopeGuard>
#include <QTextCursor>
#include <QPageLayout>
#include <QFontDatabase>
#include <QJsonDocument>
#include <QTextBlockUserData>
#include <QTextBoundaryFinder>
#include <QScopedValueRollback>
#include <QTextDocumentFragment>
#include <QAbstractTextDocumentLayout>

Q_DECLARE_METATYPE(QTextCharFormat)

class BlockKeyStrokes : public QObject
{
public:
    BlockKeyStrokes() { qApp->installEventFilter(this); }
    ~BlockKeyStrokes() { qApp->removeEventFilter(this); }

protected:
    bool eventFilter(QObject *watched, QEvent *event)
    {
        Q_UNUSED(watched);
        if (QList<int>({ QEvent::KeyPress, QEvent::KeyRelease, QEvent::ShortcutOverride,
                         QEvent::Shortcut })
                    .contains(event->type()))
            return true;
        return false;
    }
};

struct ParagraphMetrics
{
    // The following metrics are picked up from FinalDraft 12
    const qreal leftMargin = 1.5; // inches
    const qreal rightMargin = 7.69; // inches
    const qreal topMargin = 0.50; // inches
    const qreal bottomMargin = 0.50; // inches
    const qreal contentWidth = rightMargin - leftMargin;
    const QList<QVariantList> paragraphMetrics = {
        { 1.50, 7.69, 1, QFont::MixedCase, Qt::AlignLeft }, // SceneElement::Action,
        { 3.56, 7.41, 1, QFont::AllUppercase, Qt::AlignLeft }, // SceneElement::Character,
        { 2.56, 6.13, 0, QFont::MixedCase, Qt::AlignLeft }, // SceneElement::Dialogue,
        { 3.06, 5.63, 0, QFont::MixedCase, Qt::AlignLeft }, // SceneElement::Parenthetical,
        { 1.50, 7.69, 1, QFont::AllUppercase, Qt::AlignLeft }, // SceneElement::Shot,
        { 5.63, 7.25, 1, QFont::AllUppercase, Qt::AlignRight }, // SceneElement::Transition,
        { 1.50, 7.69, 1, QFont::AllUppercase, Qt::AlignLeft } // SceneElement::Heading,
    };

    qreal leftMarginOf(int type) const { return paragraphMetrics[type][0].toDouble(); }
    qreal rightMarginOf(int type) const { return paragraphMetrics[type][1].toDouble(); }
    int linesBefore(int type) const { return paragraphMetrics[type][2].toInt(); }
    QFont::Capitalization fontCappingOf(int type) const
    {
        return QFont::Capitalization(paragraphMetrics[type][3].toInt());
    }
    Qt::Alignment textAlignOf(int type) const
    {
        return Qt::Alignment(paragraphMetrics[type][4].toInt());
    }
};

SceneElementFormat::SceneElementFormat(SceneElement::Type type, ScreenplayFormat *parent)
    : QObject(parent), m_font(parent->defaultFont()), m_format(parent), m_elementType(type)
{
    QObject::connect(this, &SceneElementFormat::elementFormatChanged, [this]() {
        this->markAsModified();
        m_lastCreatedBlockFormatPageWidth = -1;
        m_lastCreatedCharFormatPageWidth = -1;
    });
    QObject::connect(this, &SceneElementFormat::fontChanged, this,
                     &SceneElementFormat::font2Changed);
    QObject::connect(m_format, &ScreenplayFormat::fontPointSizeDeltaChanged, this,
                     &SceneElementFormat::font2Changed);
    QObject::connect(m_format, &ScreenplayFormat::fontPointSizeDeltaChanged, this,
                     &SceneElementFormat::elementFormatChanged);
}

SceneElementFormat::~SceneElementFormat() { }

void SceneElementFormat::setFont(const QFont &val)
{
    if (m_font == val)
        return;

    m_font = val;
    emit fontChanged();
    emit elementFormatChanged();
}

QFont SceneElementFormat::font2() const
{
    QFont font = m_font;
    font.setPointSize(font.pointSize() + m_format->fontPointSizeDelta());
    return font;
}

void SceneElementFormat::setFontFamily(const QString &val)
{
    if (m_font.family() == val)
        return;

    m_font.setFamily(val);
    emit fontChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setFontBold(bool val)
{
    if (m_font.bold() == val)
        return;

    m_font.setBold(val);
    emit fontChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setFontItalics(bool val)
{
    if (m_font.italic() == val)
        return;

    m_font.setItalic(val);
    emit fontChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setFontUnderline(bool val)
{
    if (m_font.underline() == val)
        return;

    m_font.setUnderline(val);
    emit fontChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setFontPointSize(int val)
{
    if (m_font.pointSize() == val)
        return;

    m_font.setPointSize(val);
    emit fontChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setFontCapitalization(QFont::Capitalization caps)
{
    if (m_font.capitalization() == caps)
        return;

    m_font.setCapitalization(caps);
    emit fontChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setTextColor(const QColor &val)
{
    if (m_textColor == val)
        return;

    m_textColor = val;
    emit textColorChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setTextAlignment(Qt::Alignment val)
{
    if (m_textAlignment == val)
        return;

    m_textAlignment = val;
    emit textAlignmentChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setBackgroundColor(const QColor &val)
{
    if (m_backgroundColor == val)
        return;

    QColor val2 = val;
    val2.setAlphaF(1);
    if (val2 == Qt::black || val2 == Qt::white)
        val2 = Qt::transparent;
    else
        val2.setAlphaF(0.25);

    m_backgroundColor = val2;
    emit backgroundColorChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setTextIndent(qreal val)
{
    qreal val2 = qBound(0.0, val, 100.0);
    if (qFuzzyCompare(m_textIndent, val2))
        return;

    m_textIndent = val2;
    emit textIndentChanged();

    emit elementFormatChanged();
}

void SceneElementFormat::setLineHeight(qreal val)
{
    qreal val2 = qBound(0.1, val, 2.0);
    if (qFuzzyCompare(m_lineHeight, val2))
        return;

    m_lineHeight = val2;

    emit lineHeightChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setLineSpacingBefore(qreal val)
{
    val = qBound(0.0, val, 2.0);
    if (qFuzzyCompare(m_lineSpacingBefore, val))
        return;

    m_lineSpacingBefore = val;
    emit lineSpacingBeforeChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setLeftMargin(qreal val)
{
    val = qBound(0.0, val, 1.0);
    if (qFuzzyCompare(m_leftMargin, val))
        return;

    m_leftMargin = val;
    emit leftMarginChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setRightMargin(qreal val)
{
    val = qBound(0.0, val, 1.0);
    if (qFuzzyCompare(m_rightMargin, val))
        return;

    m_rightMargin = val;
    emit rightMarginChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setDefaultLanguage(SceneElementFormat::DefaultLanguage val)
{
    if (m_defaultLanguage == val)
        return;

    m_defaultLanguage = val;
    emit defaultLanguageChanged();

#if 0
    Application::log("SceneElementFormat(" + QString::number(m_elementType)
                     + ") changed default language to " + QString::number(int(val) - 1));
#endif
}

void SceneElementFormat::activateDefaultLanguage()
{
    if (m_defaultLanguage == Default) {
#if 0
        Application::log("SceneElementFormat(" + QString::number(m_elementType)
                         + ") activating default " + QString::number(m_format->defaultLanguage()));
#endif
        TransliterationEngine::instance()->setLanguage(m_format->defaultLanguage());
        return;
    }

    TransliterationEngine::Language language =
            TransliterationEngine::Language(int(m_defaultLanguage) - 1);
#if 0
    Application::log("SceneElementFormat(" + QString::number(m_elementType)
                     + ") activating explicitly " + QString::number(language));
#endif
    TransliterationEngine::instance()->setLanguage(language);
}

QTextBlockFormat SceneElementFormat::createBlockFormat(Qt::Alignment overrideAlignment,
                                                       const qreal *givenContentWidth) const
{
    if (m_lastCreatedBlockFormatPageWidth > 0 && givenContentWidth
        && *givenContentWidth == m_lastCreatedBlockFormatPageWidth
        && overrideAlignment == m_lastCreatedBlockAlignment)
        return m_lastCreatedBlockFormat;

    const qreal dpr = m_format->devicePixelRatio();
    const QFontMetrics fm =
            m_format->screen() ? m_format->defaultFont2Metrics() : m_format->defaultFontMetrics();
    const qreal contentWidth =
            givenContentWidth ? *givenContentWidth : m_format->pageLayout()->contentWidth();
    const qreal leftMargin = contentWidth * m_leftMargin * dpr;
    const qreal rightMargin = contentWidth * m_rightMargin * dpr;
    const qreal topMargin = fm.lineSpacing() * m_lineSpacingBefore;

    QTextBlockFormat format;
    format.setLeftMargin(leftMargin);
    format.setRightMargin(rightMargin);
    format.setTopMargin(topMargin);
    format.setLineHeight(m_lineHeight * 100, QTextBlockFormat::ProportionalHeight);

    if (m_textIndent > 0.0)
        format.setTextIndent(m_textIndent);

    if (overrideAlignment != 0)
        format.setAlignment(overrideAlignment);
    else
        format.setAlignment(m_textAlignment);

    if (!qFuzzyIsNull(m_backgroundColor.alphaF()))
        format.setBackground(QBrush(m_backgroundColor));

    if (givenContentWidth) {
        m_lastCreatedBlockFormatPageWidth = *givenContentWidth;
        m_lastCreatedBlockAlignment = overrideAlignment;
        m_lastCreatedBlockFormat = format;
    }

    return format;
}

QTextCharFormat SceneElementFormat::createCharFormat(const qreal *givenPageWidth) const
{
    if (m_lastCreatedCharFormatPageWidth > 0 && givenPageWidth
        && *givenPageWidth == m_lastCreatedCharFormatPageWidth)
        return m_lastCreatedCharFormat;

    QTextCharFormat format;

    const QFont font = this->font2();

    // It turns out that format.setFont()
    // doesnt actually do all of the below.
    // So, we will have to do it explicitly
    format.setFontFamily(font.family());
    format.setFontStretch(font.stretch());
    format.setFontOverline(font.overline());
    format.setFontPointSize(font.pointSize());
    // format.setFontStrikeOut(font.strikeOut());
    format.setFontFixedPitch(font.fixedPitch());
    format.setFontWordSpacing(font.wordSpacing());
    format.setFontLetterSpacing(font.letterSpacing());
    format.setFontCapitalization(font.capitalization());
    format.setFontLetterSpacingType(font.letterSpacingType());

    // Following properties clash with custom text-formatting that user can
    // apply to selected text fragments. So, we insert these font properties
    // only if they are not default.
    if (font.italic())
        format.setFontItalic(font.italic());
    if (font.weight() != format.fontWeight())
        format.setFontWeight(font.weight());
    if (font.underline() != font.underline())
        format.setFontUnderline(font.underline());
    if (m_textColor != Qt::black)
        format.setForeground(QBrush(m_textColor));

    if (givenPageWidth) {
        m_lastCreatedCharFormatPageWidth = *givenPageWidth;
        m_lastCreatedCharFormat = format;
    }

    return format;
}

void SceneElementFormat::applyToAll(SceneElementFormat::Properties properties)
{
    if (properties == AllProperties) {
        for (int i = FontFamily; i <= TextAndBackgroundColors; i++)
            m_format->applyToAll(this, SceneElementFormat::Properties(i));
    } else
        m_format->applyToAll(this, properties);
}

void SceneElementFormat::beginTransaction()
{
    if (m_inTransaction)
        return;

    m_inTransaction = true;
    emit inTransactionChanged();

    m_nrChangesDuringTransation = 0;
    connect(this, &SceneElementFormat::elementFormatChanged, this,
            &SceneElementFormat::countTransactionChange);
}

void SceneElementFormat::commitTransaction()
{
    if (!m_inTransaction)
        return;

    m_inTransaction = false;
    emit inTransactionChanged();

    disconnect(this, &SceneElementFormat::elementFormatChanged, this,
               &SceneElementFormat::countTransactionChange);
    if (m_nrChangesDuringTransation > 0)
        emit elementFormatChanged();

    m_nrChangesDuringTransation = 0;
}

void SceneElementFormat::resetToFactoryDefaults()
{
    this->setFont(m_format->defaultFont());
    this->setLineHeight(0.85);
    this->setLeftMargin(0);
    this->setRightMargin(0);
    this->setLineSpacingBefore(0);
    this->setTextIndent(0);
    this->setTextColor(Qt::black);
    this->setBackgroundColor(Qt::transparent);
    this->setTextAlignment(Qt::AlignLeft);
    this->setDefaultLanguage(m_elementType == SceneElement::Heading ? English : Default);

    QSettings *settings = Application::instance()->settings();
    QString defaultLanguage = QStringLiteral("Default");
    switch (m_elementType) {
    case SceneElement::Action:
        settings->setValue(QStringLiteral("Paragraph Language/actionLanguage"), defaultLanguage);
        break;
    case SceneElement::Character:
        settings->setValue(QStringLiteral("Paragraph Language/characterLanguage"), defaultLanguage);
        break;
    case SceneElement::Parenthetical:
        settings->setValue(QStringLiteral("Paragraph Language/parentheticalLanguage"),
                           defaultLanguage);
        break;
    case SceneElement::Dialogue:
        settings->setValue(QStringLiteral("Paragraph Language/dialogueLanguage"), defaultLanguage);
        break;
    case SceneElement::Shot:
        settings->setValue(QStringLiteral("Paragraph Language/shotLanguage"), defaultLanguage);
        break;
    case SceneElement::Transition:
        settings->setValue(QStringLiteral("Paragraph Language/transitionLanguage"),
                           defaultLanguage);
        break;
    case SceneElement::Heading:
        settings->setValue(QStringLiteral("Paragraph Language/headingLanguage"), defaultLanguage);
        break;
    default:
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////

Q_DECL_IMPORT int qt_defaultDpi();

ScreenplayPageLayout::ScreenplayPageLayout(ScreenplayFormat *parent)
    : QObject(parent), m_format(parent)
{
    m_resolution = qt_defaultDpi();
    m_padding[0] = 0; // just to get rid of the unused private variable warning.

    connect(m_format, &ScreenplayFormat::screenChanged, this,
            &ScreenplayPageLayout::evaluateRectsLater);
    this->evaluateRectsLater();
}

ScreenplayPageLayout::~ScreenplayPageLayout() { }

void ScreenplayPageLayout::setPaperSize(ScreenplayPageLayout::PaperSize val)
{
    if (m_paperSize == val)
        return;

    m_paperSize = val;
    this->evaluateRectsLater();

    emit paperSizeChanged();
}

void ScreenplayPageLayout::setResolution(qreal val)
{
    if (qFuzzyCompare(m_resolution, val))
        return;

    m_resolution = val;
    emit resolutionChanged();
}

void ScreenplayPageLayout::setDefaultResolution(qreal val)
{
    if (qFuzzyCompare(m_defaultResolution, val))
        return;

    m_defaultResolution = val;

    QSettings *settings = Application::instance()->settings();
    settings->setValue("ScreenplayPageLayout/defaultResolution", val);

    emit defaultResolutionChanged();
}

void ScreenplayPageLayout::loadCustomResolutionFromSettings()
{
    if (qFuzzyIsNull(m_customResolution)) {
        QSettings *settings = Application::instance()->settings();
#ifdef Q_OS_MAC
        const qreal fallback = 72.0;
#else
        const qreal fallback = 0;
#endif
        this->setCustomResolution(
                settings->value("ScreenplayPageLayout/customResolution", fallback).toDouble());
    }
}

void ScreenplayPageLayout::setCustomResolution(qreal val)
{
    if (m_customResolution == val)
        return;

    m_customResolution = val;

    QSettings *settings = Application::instance()->settings();
    settings->setValue("ScreenplayPageLayout/customResolution", val);

    emit customResolutionChanged();

    this->evaluateRectsLater();
}

void ScreenplayPageLayout::configure(QTextDocument *document) const
{
    const bool stdResolution = qFuzzyCompare(m_resolution, qt_defaultDpi());
    const QMarginsF pixelMargins =
            stdResolution ? m_margins : m_pageLayout.marginsPixels(qt_defaultDpi());
    const QSizeF pageSize = stdResolution ? m_paperRect.size()
                                          : m_pageLayout.pageSize().sizePixels(qt_defaultDpi());

    document->setUseDesignMetrics(true);
    document->setPageSize(pageSize);

    QTextFrameFormat format;
    format.setTopMargin(pixelMargins.top());
    format.setBottomMargin(pixelMargins.bottom());
    format.setLeftMargin(pixelMargins.left());
    format.setRightMargin(pixelMargins.right());
    document->rootFrame()->setFrameFormat(format);
}

void ScreenplayPageLayout::configure(QPagedPaintDevice *printer) const
{
    printer->setPageLayout(m_pageLayout);
}

void ScreenplayPageLayout::evaluateRectsNow()
{
    if (m_evaluateRectsTimer.isActive()) {
        m_evaluateRectsTimer.stop();
        this->evaluateRects();
    }
}

void ScreenplayPageLayout::evaluateRects()
{
    if (m_format->screen()) {
        const qreal pdpi = m_format->screen()->physicalDotsPerInch();
        const qreal dpr = m_format->screen()->devicePixelRatio();
        const qreal dres =
                !qFuzzyIsNull(pdpi) && !qFuzzyIsNull(dpr) ? (pdpi / dpr) : qt_defaultDpi();
        this->setDefaultResolution(dres);
        this->loadCustomResolutionFromSettings();
        this->setResolution(qFuzzyIsNull(m_customResolution) ? m_defaultResolution
                                                             : m_customResolution);
    } else
        this->setResolution(qt_defaultDpi());

    // Page margins
    const ParagraphMetrics paraMetrics;
    const qreal leftMargin = paraMetrics.leftMargin; // inches
    const qreal topMargin = paraMetrics.topMargin; // inches
    const qreal bottomMargin = paraMetrics.bottomMargin; // inches
    const qreal contentWidth = paraMetrics.contentWidth; // inches

    const QPageSize pageSize(m_paperSize == A4 ? QPageSize::A4 : QPageSize::Letter);
    const QRectF paperRectIn = pageSize.rect(QPageSize::Inch);

    const qreal rightMargin = paperRectIn.width() - contentWidth - leftMargin;

    const QMarginsF margins(leftMargin, topMargin, rightMargin, bottomMargin);

    QPageLayout pageLayout(pageSize, QPageLayout::Portrait, margins, QPageLayout::Inch);
    pageLayout.setMode(QPageLayout::StandardMode);

    m_paperRect = pageLayout.fullRectPixels(int(m_resolution));
    m_paintRect = pageLayout.paintRectPixels(int(m_resolution));
    m_margins = pageLayout.marginsPixels(int(m_resolution));

    m_headerRect = m_paperRect;
    m_headerRect.setLeft(m_paintRect.left());
    m_headerRect.setRight(m_paintRect.right());
    m_headerRect.setBottom(m_paintRect.top());

    m_footerRect = m_paperRect;
    m_footerRect.setLeft(m_paintRect.left());
    m_footerRect.setRight(m_paintRect.right());
    m_footerRect.setTop(m_paintRect.bottom());

    m_pageLayout = pageLayout;

    emit rectsChanged();
}

void ScreenplayPageLayout::evaluateRectsLater()
{
    m_evaluateRectsTimer.start(0, this);
}

void ScreenplayPageLayout::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_evaluateRectsTimer.timerId()) {
        m_evaluateRectsTimer.stop();
        this->evaluateRects();
    }
}

///////////////////////////////////////////////////////////////////////////////

ScreenplayFormat::ScreenplayFormat(QObject *parent)
    : QAbstractListModel(parent),
      m_pageWidth(750),
      m_screen(this, "screen"),
      m_scriteDocument(qobject_cast<ScriteDocument *>(parent)),
      m_defaultFontMetrics(m_defaultFont),
      m_defaultFont2Metrics(m_defaultFont)
{
    m_padding[0] = 0; // just to get rid of the unused private variable warning.

    for (int i = SceneElement::Min; i <= SceneElement::Max; i++) {
        SceneElementFormat *elementFormat = new SceneElementFormat(SceneElement::Type(i), this);
        connect(elementFormat, &SceneElementFormat::elementFormatChanged, this,
                &ScreenplayFormat::formatChanged);
        m_elementFormats.append(elementFormat);
    }

    connect(this, &ScreenplayFormat::formatChanged, [this]() { this->markAsModified(); });

    connect(TransliterationEngine::instance(),
            &TransliterationEngine::preferredFontFamilyForLanguageChanged, this, [=] {
                this->useUserSpecifiedFonts();
                emit formatChanged();
            });

    QTimer::singleShot(0, this, &ScreenplayFormat::resetToUserDefaults);
}

ScreenplayFormat::~ScreenplayFormat() { }

void ScreenplayFormat::setScreen(QScreen *val)
{
    if (m_screen == val)
        return;

    if (m_screen != nullptr && val == nullptr)
        return; // Happens when Scrite is running and user switches to Login Screen on macOS.

    m_screen = val;

    this->evaluateFontZoomLevels();
    this->evaluateFontPointSizeDelta();

    emit screenChanged();
}

void ScreenplayFormat::setSreeenFromWindow(QObject *windowObject)
{
    QScreen *screen = Application::instance()->windowScreen(windowObject);
    if (screen) {
        this->setScreen(screen);
        connect(windowObject, SIGNAL(screenChanged(QScreen *)), this, SLOT(setScreen(QScreen *)),
                Qt::UniqueConnection);
    }
}

qreal ScreenplayFormat::devicePixelRatio() const
{
    Q_ASSERT_X(m_fontPointSizes.size() == m_fontZoomLevels.size(), "ScreenplayFormat",
               "Font sizes and zoom levels are out of sync.");

    const int index = m_fontPointSizes.indexOf(this->defaultFont2().pointSize());
    if (index < 0 || index >= m_fontPointSizes.size())
        return 1.0; // FIXME

    return m_fontZoomLevels.at(index).toDouble() * this->screenDevicePixelRatio();
}

void ScreenplayFormat::setDefaultLanguage(TransliterationEngine::Language val)
{
#if 0
    Application::log("ScreenplayFormat Default Language: " + QString::number(val));
#endif
    if (m_defaultLanguage == val)
        return;

    m_defaultLanguage = val;
    emit defaultLanguageChanged();
}

void ScreenplayFormat::setDefaultFont(const QFont &val)
{
    if (m_defaultFont == val)
        return;

    m_defaultFont = val;

    this->evaluateFontZoomLevels();
    this->evaluateFontPointSizeDelta();

    emit defaultFontChanged();
    emit formatChanged();
}

QFont ScreenplayFormat::defaultFont2() const
{
    QFont font = m_defaultFont;
    font.setPointSize(font.pointSize() + m_fontPointSizeDelta);
    return font;
}

void ScreenplayFormat::setFontZoomLevelIndex(int val)
{
    val = qBound(0, val, m_fontZoomLevels.size() - 1);
    if (m_fontZoomLevelIndex == val)
        return;

    m_fontZoomLevelIndex = val;

    this->evaluateFontPointSizeDelta();

    emit fontZoomLevelIndexChanged();
}

SceneElementFormat *ScreenplayFormat::elementFormat(SceneElement::Type type) const
{
    int itype = int(type);
    itype = itype % (SceneElement::Max + 1);
    return m_elementFormats.at(itype);
}

SceneElementFormat *ScreenplayFormat::elementFormat(int type) const
{
    type = type % (SceneElement::Max + 1);
    return this->elementFormat(SceneElement::Type(type));
}

QQmlListProperty<SceneElementFormat> ScreenplayFormat::elementFormats()
{
    return QQmlListProperty<SceneElementFormat>(
            reinterpret_cast<QObject *>(this), static_cast<void *>(this),
            &ScreenplayFormat::staticElementFormatCount, &ScreenplayFormat::staticElementFormatAt);
}

void ScreenplayFormat::applyToAll(const SceneElementFormat *from,
                                  SceneElementFormat::Properties properties)
{
    if (from == nullptr)
        return;

    for (SceneElementFormat *format : qAsConst(m_elementFormats)) {
        if (from == format)
            continue;

        switch (properties) {
        case SceneElementFormat::FontFamily:
            format->setFontFamily(from->font().family());
            break;
        case SceneElementFormat::FontSize:
            format->setFontPointSize(from->font().pointSize());
            break;
        case SceneElementFormat::FontStyle:
            format->setFontBold(from->font().bold());
            format->setFontItalics(from->font().italic());
            format->setFontUnderline(from->font().underline());
            break;
        case SceneElementFormat::LineHeight:
            format->setLineHeight(from->lineHeight());
            break;
        case SceneElementFormat::LineSpacingBefore:
            format->setLineSpacingBefore(from->lineSpacingBefore());
            break;
        case SceneElementFormat::TextAlignment:
            format->setTextAlignment(from->textAlignment());
            break;
        case SceneElementFormat::TextAndBackgroundColors:
            format->setTextColor(from->textColor());
            format->setBackgroundColor(from->backgroundColor());
            break;
        case SceneElementFormat::TextIndent:
            format->setTextIndent(from->textIndent());
            break;
        default:
            break;
        }
    }
}

int ScreenplayFormat::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_elementFormats.size();
}

QVariant ScreenplayFormat::data(const QModelIndex &index, int role) const
{
    if (role == SceneElementFomat && index.isValid())
        return QVariant::fromValue<QObject *>(
                qobject_cast<QObject *>(m_elementFormats.at(index.row())));

    return QVariant();
}

QHash<int, QByteArray> ScreenplayFormat::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[SceneElementFomat] = "sceneElementFormat";
    return roles;
}

void ScreenplayFormat::setSecondsPerPage(int val)
{
    if (m_secondsPerPage == val)
        return;

    m_secondsPerPage = val;
    emit secondsPerPageChanged();

    emit formatChanged();
}

void ScreenplayFormat::resetToFactoryDefaults()
{
    QSettings *settings = Application::instance()->settings();
    const int iPaperSize = settings->value("PageSetup/paperSize").toInt();
    if (iPaperSize == ScreenplayPageLayout::A4)
        this->pageLayout()->setPaperSize(ScreenplayPageLayout::A4);
    else
        this->pageLayout()->setPaperSize(ScreenplayPageLayout::Letter);

    this->setSecondsPerPage(60);

    const QString fontFamily = TransliterationEngine::instance()->preferredFontFamilyForLanguage(
            TransliterationEngine::English);
    this->setDefaultFont(QFont(fontFamily, 12));
    if (m_screen != nullptr) {
        const int index = m_fontZoomLevels.indexOf(QVariant(1.0));
        this->setFontZoomLevelIndex(index);
    }

    const QString defLanguage =
            settings->value(QStringLiteral("Paragraph Language/defaultLanguage"),
                            QStringLiteral("English"))
                    .toString();
    const QMetaObject *mo = &SceneElement::staticMetaObject;
    const QMetaEnum enumerator = mo->enumerator(mo->indexOfEnumerator("Language"));
    if (enumerator.isValid()) {
        bool ok = false;
        const int value = enumerator.keyToValue(qPrintable(defLanguage), &ok);
        if (ok)
            this->setDefaultLanguageInt(value);
    }

    for (int i = SceneElement::Min; i <= SceneElement::Max; i++)
        m_elementFormats.at(i)->resetToFactoryDefaults();

    const ParagraphMetrics paraMetrics;
    const qreal lm = paraMetrics.leftMargin;
    const qreal rm = paraMetrics.rightMargin;
    const qreal cw = paraMetrics.contentWidth;

    for (int i = SceneElement::Min; i <= SceneElement::Max; i++) {
        const qreal plm = paraMetrics.leftMarginOf(i);
        const qreal prm = paraMetrics.rightMarginOf(i);

        m_elementFormats[i]->setLeftMargin((plm - lm) / cw);
        m_elementFormats[i]->setRightMargin((rm - prm) / cw);
        m_elementFormats[i]->setLineSpacingBefore(paraMetrics.linesBefore(i));
        m_elementFormats[i]->setFontCapitalization(paraMetrics.fontCappingOf(i));
        m_elementFormats[i]->setTextAlignment(paraMetrics.textAlignOf(i));
    }
}

bool ScreenplayFormat::saveAsUserDefaults()
{
    const QJsonObject json = QObjectSerializer::toJson(this);
    const QJsonDocument jsonDoc(json);
    const QByteArray jsonStr = jsonDoc.toJson();

    const QFileInfo settingsPath(Application::instance()->settingsFilePath());
    const QString formatFile =
            settingsPath.absoluteDir().absoluteFilePath(QStringLiteral("formatting.json"));

    QFile file(formatFile);
    if (!file.open(QFile::WriteOnly))
        return false;

    file.write(jsonStr);
    return true;
}

void ScreenplayFormat::resetToUserDefaults()
{
    const QFileInfo settingsPath(Application::instance()->settingsFilePath());
    const QString formatFile =
            settingsPath.absoluteDir().absoluteFilePath(QStringLiteral("formatting.json"));

    this->resetToFactoryDefaults();

    if (!QFile::exists(formatFile))
        return;

    QFile file(formatFile);
    if (!file.open(QFile::ReadOnly))
        return;

    const QByteArray jsonStr = file.readAll();
    if (jsonStr.isEmpty())
        return;

    QJsonParseError jsonError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr, &jsonError);
    if (jsonError.error != QJsonParseError::NoError)
        return;

    if (!jsonDoc.isObject())
        return;

    const QJsonObject json = jsonDoc.object();
    if (json.isEmpty())
        return;

    QObjectSerializer::fromJson(json, this);
}

void ScreenplayFormat::beginTransaction()
{
    if (m_inTransaction)
        return;

    m_inTransaction = true;
    emit inTransactionChanged();

    m_nrChangesDuringTransation = 0;
    connect(this, &ScreenplayFormat::formatChanged, this,
            &ScreenplayFormat::countTransactionChange);

    for (int i = SceneElement::Min; i <= SceneElement::Max; i++)
        m_elementFormats.at(i)->beginTransaction();
}

void ScreenplayFormat::commitTransaction()
{
    if (!m_inTransaction)
        return;

    for (int i = SceneElement::Min; i <= SceneElement::Max; i++)
        m_elementFormats.at(i)->commitTransaction();

    m_inTransaction = false;
    emit inTransactionChanged();

    disconnect(this, &ScreenplayFormat::formatChanged, this,
               &ScreenplayFormat::countTransactionChange);
    if (m_nrChangesDuringTransation > 0)
        emit formatChanged();

    m_nrChangesDuringTransation = 0;
}

void ScreenplayFormat::useUserSpecifiedFonts()
{
    const QString englishFont = TransliterationEngine::instance()->preferredFontFamilyForLanguage(
            TransliterationEngine::English);
    QFont defFont = this->defaultFont();
    if (defFont.family() != englishFont) {
        defFont.setFamily(englishFont);
        for (int i = SceneElement::Min; i <= SceneElement::Max; i++) {
            SceneElementFormat *format = this->elementFormat(i);
            QFont formatFont = format->font();
            formatFont.setFamily(englishFont);
            format->setFont(formatFont);
        }
        this->setDefaultFont(defFont);
    }
}

void ScreenplayFormat::deserializeFromJson(const QJsonObject &)
{
    SceneElementFormat *headingFormat = this->elementFormat(SceneElement::Heading);
    if (headingFormat && headingFormat->defaultLanguage() == SceneElementFormat::Default)
        headingFormat->setDefaultLanguage(SceneElementFormat::English);
}

void ScreenplayFormat::resetScreen()
{
    m_screen = nullptr;
    emit screenChanged();

    this->evaluateFontZoomLevels();
    this->evaluateFontPointSizeDelta();
}

void ScreenplayFormat::evaluateFontPointSizeDelta()
{
    Q_ASSERT_X(m_fontPointSizes.size() == m_fontZoomLevels.size(), "ScreenplayFormat",
               "Font sizes and zoom levels are out of sync.");

    int fontPointSize = 0;
    if (m_fontZoomLevelIndex < 0) {
        const int index =
                qBound(0, m_fontZoomLevels.indexOf(QVariant(1.0)), m_fontZoomLevels.size() - 1);
        fontPointSize = m_fontPointSizes.at(index);
    } else
        fontPointSize =
                m_fontPointSizes.at(qBound(0, m_fontZoomLevelIndex, m_fontZoomLevels.size() - 1));

    const int val = fontPointSize - m_defaultFont.pointSize();
    if (m_fontPointSizeDelta == val)
        return;

    m_fontPointSizeDelta = val;
    emit fontPointSizeDeltaChanged();
    emit formatChanged();
}

void ScreenplayFormat::evaluateFontZoomLevels()
{
    const QList<int> defaultFontPointSizes = QFontDatabase().pointSizes(m_defaultFont.family());

    QFont font2 = m_defaultFont;
    font2.setPointSize([=]() {
        const int ps = int(font2.pointSize() * this->screenDevicePixelRatio());
        int i = 0;
        for (i = 0; i < defaultFontPointSizes.size(); i++) {
            const int dps = defaultFontPointSizes.at(i);
            if (dps == ps)
                break;
            if (dps > ps) {
                --i;
                break;
            }
        }
        return defaultFontPointSizes.at(i);
    }());

    QFontInfo defaultFontInfo(font2);
    font2.setPointSize(defaultFontInfo.pointSize());

    QFontMetricsF fm2(font2);
    const qreal zoomOneACW = fm2.averageCharWidth();
    const int maxPointSize = int(2.0 * qreal(defaultFontInfo.pointSize()));
    const int minPointSize = 8;

    QVariantList zoomLevels = QVariantList() << QVariant(1.0);
    QList<int> selectedPointSizes = QList<int>() << defaultFontInfo.pointSize();

    const int start = defaultFontPointSizes.indexOf(defaultFontInfo.pointSize());
    for (int i = start + 1; i < defaultFontPointSizes.size(); i++) {
        const int fontSize = defaultFontPointSizes.at(i);
        if (fontSize > maxPointSize)
            break;

        font2.setPointSize(fontSize);
        fm2 = QFontMetricsF(font2);
        const qreal zoomLevel = fm2.averageCharWidth() / zoomOneACW;
        const qreal lastZoomLevel = zoomLevels.last().toDouble();
        const qreal zoomScale = zoomLevel / lastZoomLevel;
        if (zoomScale > 0.1) {
            zoomLevels.append(zoomLevel);
            selectedPointSizes.append(fontSize);
        }
    }

    for (int i = start - 1; i >= 0; i--) {
        const int fontSize = defaultFontPointSizes.at(i);
        if (fontSize <= minPointSize)
            break;

        font2.setPointSize(fontSize);
        fm2 = QFontMetricsF(font2);
        const qreal zoomLevel = fm2.averageCharWidth() / zoomOneACW;
        const qreal lastZoomLevel = zoomLevels.first().toDouble();
        const qreal zoomScale = zoomLevel / lastZoomLevel;
        if (zoomScale < 0.9) {
            zoomLevels.prepend(zoomLevel);
            selectedPointSizes.prepend(fontSize);
        }
    }

    m_fontZoomLevels = zoomLevels;
    m_fontPointSizes = selectedPointSizes;
    emit fontZoomLevelsChanged();

    m_fontZoomLevelIndex = m_fontPointSizes.indexOf(defaultFontInfo.pointSize());
    emit fontZoomLevelIndexChanged();
}

SceneElementFormat *
ScreenplayFormat::staticElementFormatAt(QQmlListProperty<SceneElementFormat> *list, int index)
{
    index = index % (SceneElement::Max + 1);
    return reinterpret_cast<ScreenplayFormat *>(list->data)->m_elementFormats.at(index);
}

int ScreenplayFormat::staticElementFormatCount(QQmlListProperty<SceneElementFormat> *list)
{
    return reinterpret_cast<ScreenplayFormat *>(list->data)->m_elementFormats.size();
}

///////////////////////////////////////////////////////////////////////////////

TextFormat::TextFormat(QObject *parent) : QObject(parent) { }

TextFormat::~TextFormat() { }

void TextFormat::setBold(bool val)
{
    if (m_bold == val)
        return;

    m_bold = val;
    emit boldChanged();

    if (!m_updatingFromFormat)
        emit formatChanged({ QTextFormat::FontWeight });
}

void TextFormat::setItalics(bool val)
{
    if (m_italics == val)
        return;

    m_italics = val;
    emit italicsChanged();

    if (!m_updatingFromFormat)
        emit formatChanged({ QTextFormat::FontItalic });
}

void TextFormat::setUnderline(bool val)
{
    if (m_underline == val)
        return;

    m_underline = val;
    emit underlineChanged();

    if (!m_updatingFromFormat)
        emit formatChanged({ QTextFormat::FontUnderline, QTextFormat::TextUnderlineStyle });
}

void TextFormat::setStrikeout(bool val)
{
    if (m_strikeout == val)
        return;

    m_strikeout = val;
    emit strikeoutChanged();

    if (!m_updatingFromFormat)
        emit formatChanged({ QTextFormat::FontStrikeOut });
}

void TextFormat::setTextColor(const QColor &val)
{
    if (m_textColor == val)
        return;

    m_textColor = val;
    emit textColorChanged();

    if (!m_updatingFromFormat)
        emit formatChanged({ QTextFormat::ForegroundBrush });
}

void TextFormat::setBackgroundColor(const QColor &val)
{
    if (m_backgroundColor == val)
        return;

    m_backgroundColor = val;
    emit backgroundColorChanged();

    if (!m_updatingFromFormat)
        emit formatChanged({ QTextFormat::BackgroundBrush });
}

void TextFormat::reset()
{
    m_updatingFromFormat = true;

    this->resetTextColor();
    this->resetBackgroundColor();
    this->setBold(false);
    this->setItalics(false);
    this->setUnderline(false);

    m_updatingFromFormat = false;

    emit formatChanged(allProperties());
}

void TextFormat::updateFromCharFormat(const QTextCharFormat &format)
{
    if (m_updatingFromFormat)
        return;

    QScopedValueRollback<bool> rollback(m_updatingFromFormat);
    m_updatingFromFormat = true;

    if (format.hasProperty(QTextFormat::ForegroundBrush))
        this->setTextColor(format.foreground().color());
    else
        this->resetTextColor();

    if (format.hasProperty(QTextFormat::BackgroundBrush))
        this->setBackgroundColor(format.background().color());
    else
        this->resetBackgroundColor();

    if (format.hasProperty(QTextFormat::FontWeight))
        this->setBold(format.fontWeight() == QFont::Bold);
    else
        this->setBold(false);

    if (format.hasProperty(QTextFormat::FontItalic))
        this->setItalics(format.fontItalic());
    else
        this->setItalics(false);

    if (format.hasProperty(QTextFormat::TextUnderlineStyle))
        this->setUnderline(format.fontUnderline());
    else
        this->setUnderline(false);

    if (format.hasProperty(QTextFormat::FontStrikeOut))
        this->setStrikeout(format.fontStrikeOut());
    else
        this->setStrikeout(false);
}

QTextCharFormat TextFormat::toCharFormat(const QList<int> &properties) const
{
    QTextCharFormat format;

    if (properties.isEmpty() || properties.contains(QTextFormat::ForegroundBrush))
        if (m_textColor != Qt::transparent)
            format.setForeground(m_textColor);

    if (properties.isEmpty() || properties.contains(QTextFormat::BackgroundBrush))
        if (m_backgroundColor != Qt::transparent)
            format.setBackground(m_backgroundColor);

    if (properties.isEmpty() || properties.contains(QTextFormat::FontWeight))
        if (m_bold)
            format.setFontWeight(QFont::Bold);

    if (properties.isEmpty() || properties.contains(QTextFormat::FontItalic))
        if (m_italics)
            format.setFontItalic(m_italics);

    if (properties.isEmpty() || properties.contains(QTextFormat::FontUnderline)
        || properties.contains(QTextFormat::TextUnderlineStyle))
        if (m_underline)
            format.setFontUnderline(m_underline);

    if (properties.isEmpty() || properties.contains(QTextFormat::FontStrikeOut))
        if (m_strikeout)
            format.setFontStrikeOut(m_strikeout);

    return format;
}

QList<int> TextFormat::allProperties()
{
    return { QTextFormat::ForegroundBrush,   QTextFormat::BackgroundBrush,
             QTextFormat::FontWeight,        QTextFormat::FontItalic,
             QTextFormat::FontUnderline,     QTextFormat::FontStrikeOut,
             QTextFormat::TextUnderlineStyle };
}

///////////////////////////////////////////////////////////////////////////////

class SceneDocumentBlockUserData : public QTextBlockUserData
{
public:
    enum { Type = 1001 };
    const int type = Type;

    explicit SceneDocumentBlockUserData(const QTextBlock &block, SceneElement *element,
                                        SceneDocumentBinder *binder);
    ~SceneDocumentBlockUserData();

    QTextBlockFormat blockFormat;
    QTextCharFormat charFormat;

    SceneElement *sceneElement() const { return m_sceneElement; }

    void resetFormat();
    bool shouldUpdateFromFormat(const SceneElementFormat *format);

    void initializeSpellCheck(SceneDocumentBinder *binder);
    bool shouldUpdateFromSpellCheck();
    void scheduleSpellCheckUpdate();
    QList<TextFragment> misspelledFragments() const;
    TextFragment findMisspelledFragment(int start, int end) const;

    void polishTextLater();
    void autoCapitalizeLater();

    static SceneDocumentBlockUserData *get(const QTextBlock &block);
    static SceneDocumentBlockUserData *get(QTextBlockUserData *userData);

private:
    enum Tasks { PolishTextTask, AutoCapitalizeTask };
    void polishTextNow();
    void autoCapitalizeNow();
    void performPendingTasks();
    bool markCursorPosition();
    int markedCursorPosition(bool removeMarker = true);

private:
    friend class SceneDocumentBinder;
    QTextBlock m_textBlock;
    QSet<int> m_pendingTasks;
    QPointer<SpellCheckService> m_spellCheck;
    QPointer<SceneElement> m_sceneElement;
    QPointer<SceneDocumentBinder> m_binder;
    QString m_highlightedText;
    int m_formatMTime = -1;
    int m_spellCheckMTime = -1;
    QMetaObject::Connection m_spellCheckConnection;
};

SceneDocumentBlockUserData::SceneDocumentBlockUserData(const QTextBlock &textBlock,
                                                       SceneElement *element,
                                                       SceneDocumentBinder *binder)
    : m_textBlock(textBlock), m_sceneElement(element), m_binder(binder)
{
    if (m_binder->isSpellCheckEnabled()) {
        m_spellCheck = element->spellCheck();
        m_spellCheckConnection =
                QObject::connect(m_spellCheck, SIGNAL(misspelledFragmentsChanged()), m_binder,
                                 SLOT(onSpellCheckUpdated()), Qt::UniqueConnection);
        m_spellCheck->scheduleUpdate();
    }

    if (m_textBlock.isValid())
        m_textBlock.setUserData(this);
}

SceneDocumentBlockUserData::~SceneDocumentBlockUserData()
{
    if (m_spellCheckConnection)
        QObject::disconnect(m_spellCheckConnection);
}

void SceneDocumentBlockUserData::resetFormat()
{
    m_formatMTime = -1;
    blockFormat = QTextBlockFormat();
    charFormat = QTextCharFormat();
}

bool SceneDocumentBlockUserData::shouldUpdateFromFormat(const SceneElementFormat *format)
{
    return format->isModified(&m_formatMTime);
}

void SceneDocumentBlockUserData::initializeSpellCheck(SceneDocumentBinder *binder)
{
    if (binder->isSpellCheckEnabled()) {
        m_spellCheck = m_sceneElement->spellCheck();
        if (!m_spellCheckConnection)
            m_spellCheckConnection =
                    QObject::connect(m_spellCheck, SIGNAL(misspelledFragmentsChanged()), binder,
                                     SLOT(onSpellCheckUpdated()), Qt::UniqueConnection);
        m_spellCheck->scheduleUpdate();
    } else {
        if (m_spellCheckConnection)
            QObject::disconnect(m_spellCheckConnection);

        m_spellCheck = nullptr;
    }
}

bool SceneDocumentBlockUserData::shouldUpdateFromSpellCheck()
{
    return !m_spellCheck.isNull() && m_spellCheck->isModified(&m_spellCheckMTime);
}

void SceneDocumentBlockUserData::scheduleSpellCheckUpdate()
{
    if (!m_spellCheck.isNull())
        m_spellCheck->scheduleUpdate();
}

QList<TextFragment> SceneDocumentBlockUserData::misspelledFragments() const
{
    if (!m_spellCheck.isNull())
        return m_spellCheck->misspelledFragments();
    return QList<TextFragment>();
}

TextFragment SceneDocumentBlockUserData::findMisspelledFragment(int start, int end) const
{
    const QList<TextFragment> fragments = this->misspelledFragments();
    for (const TextFragment &fragment : fragments) {
        if ((fragment.start() >= start && fragment.start() < end)
            || (fragment.end() > start && fragment.end() <= end))
            return fragment;
    }
    return TextFragment();
}

void SceneDocumentBlockUserData::polishTextLater()
{
    if (m_binder->m_autoPolishParagraphs) {
        m_pendingTasks += PolishTextTask;
        m_binder->m_sceneElementTaskTimer.start(500, m_binder);
    }
}

void SceneDocumentBlockUserData::autoCapitalizeLater()
{
    if (m_binder->m_autoCapitalizeSentences) {
        m_pendingTasks += AutoCapitalizeTask;
        m_binder->m_sceneElementTaskTimer.start(500, m_binder);
    }
}

SceneDocumentBlockUserData *SceneDocumentBlockUserData::get(const QTextBlock &block)
{
    SceneDocumentBlockUserData *ret = get(block.userData());
    if (ret && ret->m_textBlock != block)
        ret->m_textBlock = block;

    return ret;
}

SceneDocumentBlockUserData *SceneDocumentBlockUserData::get(QTextBlockUserData *userData)
{
    if (userData == nullptr)
        return nullptr;

    SceneDocumentBlockUserData *userData2 =
            reinterpret_cast<SceneDocumentBlockUserData *>(userData);
    return userData2->type == SceneDocumentBlockUserData::Type ? userData2 : nullptr;
}

void SceneDocumentBlockUserData::polishTextNow()
{
    BlockKeyStrokes blockKeyStrokes;

    if (m_binder.isNull() || !m_textBlock.isValid())
        return;

    if (!m_binder->m_autoPolishParagraphs)
        return;

    // If the block that this object represents is currently being edited by the user
    // then lets not polish it right now.
    if (m_binder->m_cursorPosition >= 0) {
        QTextCursor cursor(m_textBlock);
        cursor.setPosition(m_binder->m_cursorPosition);
        if (cursor.block() == m_textBlock)
            return;
    }

    {
        // This is to avoid recursive edits
        QScopedValueRollback<bool> rollback(m_binder->m_sceneElementTaskIsRunning, true);

        // If the block has no text, then there is no point in polishing text
        if (m_textBlock.text().isEmpty())
            return;

        // Find out the previous scene, so that polishing of text may be done keeping
        // previous scene element in context.
        ScreenplayElement *spElement = m_binder->m_screenplayElement;
        ScreenplayElement *prevSpElement = nullptr;
        if (spElement) {
            const Screenplay *screenplay = spElement->screenplay();
            const int spElementIndex = screenplay->indexOfElement(spElement);
            for (int i = spElementIndex - 1; i >= 0; i--) {
                ScreenplayElement *spe = screenplay->elementAt(i);
                if (spe->elementType() == ScreenplayElement::SceneElementType) {
                    prevSpElement = spe;
                    break;
                }
            }
        }
        Scene *previousScene = prevSpElement ? prevSpElement->scene() : nullptr;

        // If the scene element that this object represents has no polish to apply, then
        // we can simply quit. Polishing means adding/removing CONT'D, : etc..
        if (m_sceneElement == nullptr || !m_sceneElement->polishText(previousScene))
            return;

        // Mark current cursor position if required, so that we can get back to it
        // once we are done applying all edits done as a part of the polish operation.
        bool cursorPositionMarked = false;
        if (m_binder->m_cursorPosition > m_textBlock.position())
            cursorPositionMarked = this->markCursorPosition();

        // Reset format so that its applied by the highlighter again
        this->resetFormat();

        // Apply the polished text
        QTextCursor cursor(m_textBlock);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        const QString text = m_sceneElement->text();
        if (text.isEmpty())
            cursor.removeSelectedText();
        else
            cursor.insertText(text);

        // Restore cursor position
        if (cursorPositionMarked) {
            const int cp = this->markedCursorPosition(true);
            emit m_binder->requestCursorPosition(cp);
        }
    }

    // Rehighlight the block
    m_binder->rehighlightBlock(m_textBlock);
}

void SceneDocumentBlockUserData::autoCapitalizeNow()
{
    BlockKeyStrokes blockKeyStrokes;

    // If the block has no text, then there is no point in polishing text
    if (m_textBlock.text().isEmpty() || m_sceneElement == nullptr)
        return;

    if (!m_binder->m_autoCapitalizeSentences
        || TransliterationEngine::instance()->language() != TransliterationEngine::English)
        return;

    // Auto-capitalize needs to be done only on action and dialogue paragraphs.
    const QList<int> capitalizePositions = m_sceneElement->autoCapitalizePositions();
    if (capitalizePositions.isEmpty())
        return;

    {
        // This is to avoid recursive edits
        QScopedValueRollback<bool> rollback(m_binder->m_sceneElementTaskIsRunning, true);

        // Mark current cursor position if required, so that we can get back to it
        // once we are done applying all edits done as a part of the capitalize
        // operation
        bool cursorPositionMarked = false;
        if (m_binder->m_cursorPosition > m_textBlock.position())
            cursorPositionMarked = this->markCursorPosition();

        // Reset format so that its applied by the highlighter again
        this->resetFormat();

        // Capitalize letters as determined.
        QTextCursor cursor(m_textBlock);
        for (int pos : capitalizePositions) {
            cursor.setPosition(m_textBlock.position() + pos);
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            cursor.insertText(cursor.selectedText().toUpper());
            cursor.clearSelection();
            cursor.setPosition(m_textBlock.position() + pos);
        }

        // Store changes into the element.
        m_sceneElement->setText(m_textBlock.text());
        m_sceneElement->setTextFormats(m_textBlock.textFormats());

        // Restore cursor position
        if (cursorPositionMarked) {
            const int cp = this->markedCursorPosition(true);
            emit m_binder->requestCursorPosition(cp);
        }
    }

    // Rehighlight the block
    m_binder->rehighlightBlock(m_textBlock);
}

void SceneDocumentBlockUserData::performPendingTasks()
{
    for (int task : qAsConst(m_pendingTasks)) {
        if (task == PolishTextTask)
            this->polishTextNow();
        else if (task == AutoCapitalizeTask)
            this->autoCapitalizeNow();
    }

    m_pendingTasks.clear();
}

bool SceneDocumentBlockUserData::markCursorPosition()
{
    if (m_binder.isNull() || !m_textBlock.isValid())
        return false;

    if (m_binder->m_cursorPosition < 0
        || m_binder->m_cursorPosition > m_textBlock.document()->characterCount())
        return false;

    QTextCursor cursor(m_textBlock);
    cursor.setPosition(m_binder->m_cursorPosition);
    cursor.insertText(QString(QChar::LastValidCodePoint));
    return true;
}

int SceneDocumentBlockUserData::markedCursorPosition(bool removeMarker)
{
    if (m_binder.isNull() || !m_textBlock.isValid())
        return 0;

    QTextCursor cursor = m_textBlock.document()->find(QString(QChar::LastValidCodePoint));
    if (cursor.isNull())
        return 0;

    const int cp = cursor.hasSelection() ? cursor.selectionStart() : cursor.position();
    if (removeMarker) {
        if (cursor.hasSelection())
            cursor.removeSelectedText();
        else
            cursor.deleteChar();
    }
    return cp;
}

class SpellCheckCursor : public QTextCursor
{
public:
    explicit SpellCheckCursor(QTextDocument *document, int position) : QTextCursor(document)
    {
        this->setPosition(position);
        this->select(QTextCursor::WordUnderCursor);
        m_format = this->charFormat();

        m_blockData = SceneDocumentBlockUserData::get(this->block());
        if (m_blockData != nullptr) {
            const int start = this->selectionStart() - this->block().position();
            const int end = this->selectionEnd() - this->block().position();
            m_misspelledFragment = m_blockData->findMisspelledFragment(start, end);
        }
    }
    ~SpellCheckCursor() { }

    QString word() const { return this->selectedText(); }
    bool isMisspelled() const { return m_misspelledFragment.isValid(); }
    QStringList suggestions() const { return m_misspelledFragment.suggestions(); }

    void replace(const QString &word)
    {
        if (this->word().isEmpty())
            return;

        this->removeSelectedText();
        const int start = this->position();
        this->insertText(word);
        const int end = this->position();
        this->setPosition(start);
        this->setPosition(end, QTextCursor::KeepAnchor);
        this->setCharFormat(m_format);
        this->setPosition(end);
    }

    void resetCharFormat() { this->replace(this->word()); }

private:
    SceneDocumentBlockUserData *m_blockData = nullptr;
    TextFragment m_misspelledFragment;
    QTextCharFormat m_format;
};

SceneDocumentBinder::SceneDocumentBinder(QObject *parent)
    : QSyntaxHighlighter(parent),
      m_scene(this, "scene"),
      m_rehighlightTimer("SceneDocumentBinder.m_rehighlightTimer"),
      m_initializeDocumentTimer("SceneDocumentBinder.m_initializeDocumentTimer"),
      m_sceneElementTaskTimer("SceneDocumentBinder.m_sceneElementTaskTimer"),
      m_currentElement(this, "currentElement"),
      m_textDocument(this, "textDocument"),
      m_screenplayFormat(this, "screenplayFormat"),
      m_screenplayElement(this, "screenplayElement")
{
    connect(this, &SceneDocumentBinder::currentElementChanged, this,
            &SceneDocumentBinder::nextTabFormatChanged);
    connect(m_textFormat, &TextFormat::formatChanged, this,
            &SceneDocumentBinder::onTextFormatChanged);
    connect(this, &SceneDocumentBinder::currentElementChanged, this,
            &SceneDocumentBinder::activateCurrentElementDefaultLanguage);
    connect(m_textFormat, &TextFormat::formatChanged, this,
            &SceneDocumentBinder::activateCurrentElementDefaultLanguage);
    connect(this, &SceneDocumentBinder::selectionStartPositionChanged, this,
            &SceneDocumentBinder::selectedElementsChanged);
    connect(this, &SceneDocumentBinder::selectionEndPositionChanged, this,
            &SceneDocumentBinder::selectedElementsChanged);
}

SceneDocumentBinder::~SceneDocumentBinder() { }

void SceneDocumentBinder::setScreenplayFormat(ScreenplayFormat *val)
{
    if (m_screenplayFormat == val)
        return;

    if (m_screenplayFormat != nullptr) {
        disconnect(m_screenplayFormat, &ScreenplayFormat::formatChanged, this,
                   &SceneDocumentBinder::refresh);
        disconnect(m_screenplayFormat, &ScreenplayFormat::inTransactionChanged, this,
                   &SceneDocumentBinder::rehighlightLater);
    }

    m_screenplayFormat = val;
    if (m_screenplayFormat != nullptr) {
        connect(m_screenplayFormat, &ScreenplayFormat::formatChanged, this,
                &SceneDocumentBinder::refresh);
        connect(m_screenplayFormat, &ScreenplayFormat::inTransactionChanged, this,
                &SceneDocumentBinder::rehighlightLater);

        if (qFuzzyCompare(m_textWidth, 0.0))
            this->setTextWidth(m_screenplayFormat->pageLayout()->contentWidth());
    }

    emit screenplayFormatChanged();

    this->initializeDocumentLater();
}

void SceneDocumentBinder::setScene(Scene *val)
{
    if (m_scene == val)
        return;

    if (m_scene != nullptr) {
        disconnect(m_scene, &Scene::sceneElementChanged, this,
                   &SceneDocumentBinder::onSceneElementChanged);
        disconnect(m_scene, &Scene::sceneAboutToReset, this,
                   &SceneDocumentBinder::onSceneAboutToReset);
        disconnect(m_scene, &Scene::sceneReset, this, &SceneDocumentBinder::onSceneReset);
        disconnect(m_scene, &Scene::sceneRefreshed, this, &SceneDocumentBinder::onSceneRefreshed);
    }

    m_scene = val;

    if (m_scene != nullptr) {
        connect(m_scene, &Scene::sceneElementChanged, this,
                &SceneDocumentBinder::onSceneElementChanged);
        connect(m_scene, &Scene::sceneAboutToReset, this,
                &SceneDocumentBinder::onSceneAboutToReset);
        connect(m_scene, &Scene::sceneReset, this, &SceneDocumentBinder::onSceneReset);
        connect(m_scene, &Scene::sceneRefreshed, this, &SceneDocumentBinder::onSceneRefreshed);
    }

    emit sceneChanged();

    this->initializeDocumentLater();
}

void SceneDocumentBinder::setScreenplayElement(ScreenplayElement *val)
{
    if (m_screenplayElement == val)
        return;

    if (m_screenplayElement != nullptr) {
        Screenplay *screenplay = m_screenplayElement->screenplay();
        disconnect(screenplay, &Screenplay::elementMoved, this,
                   &SceneDocumentBinder::polishAllSceneElements);
    }

    m_screenplayElement = val;

    if (m_screenplayElement != nullptr) {
        Screenplay *screenplay = m_screenplayElement->screenplay();
        connect(screenplay, &Screenplay::elementMoved, this,
                &SceneDocumentBinder::polishAllSceneElements, Qt::UniqueConnection);
    }

    emit screenplayElementChanged();

    this->polishAllSceneElements();
}

void SceneDocumentBinder::setTextDocument(QQuickTextDocument *val)
{
    if (m_textDocument == val)
        return;

    if (this->document() != nullptr) {
        this->document()->setUndoRedoEnabled(true);

        disconnect(this->document(), &QTextDocument::contentsChange, this,
                   &SceneDocumentBinder::onContentsChange);
        disconnect(this->document(), &QTextDocument::blockCountChanged, this,
                   &SceneDocumentBinder::syncSceneFromDocument);

        if (m_scene != nullptr)
            disconnect(m_scene, &Scene::sceneElementChanged, this,
                       &SceneDocumentBinder::onSceneElementChanged);

        this->setCurrentElement(nullptr);
        this->setCursorPosition(-1);
    }

    m_textDocument = val;
    if (m_textDocument != nullptr)
        this->QSyntaxHighlighter::setDocument(m_textDocument->textDocument());
    else
        this->QSyntaxHighlighter::setDocument(nullptr);
    this->setDocumentLoadCount(0);

    this->evaluateAutoCompleteHintsAndCompletionPrefix();

    emit textDocumentChanged();

    this->initializeDocumentLater();

    if (m_textDocument != nullptr) {
        this->document()->setUndoRedoEnabled(false);

        connect(this->document(), &QTextDocument::contentsChange, this,
                &SceneDocumentBinder::onContentsChange, Qt::UniqueConnection);
        connect(this->document(), &QTextDocument::blockCountChanged, this,
                &SceneDocumentBinder::syncSceneFromDocument, Qt::UniqueConnection);

        if (m_scene != nullptr)
            connect(m_scene, &Scene::sceneElementChanged, this,
                    &SceneDocumentBinder::onSceneElementChanged, Qt::UniqueConnection);

#if 0 // At the moment, this seems to be causing more trouble than help.
        this->document()->setTextWidth(m_textWidth);
#endif

        this->setCursorPosition(0);
    } else
        this->setCursorPosition(-1);
}

void SceneDocumentBinder::setSpellCheckEnabled(bool val)
{
    if (m_spellCheckEnabled == val)
        return;

    m_spellCheckEnabled = val;
    emit spellCheckEnabledChanged();

    this->refresh();
}

void SceneDocumentBinder::setLiveSpellCheckEnabled(bool val)
{
    if (m_liveSpellCheckEnabled == val)
        return;

    m_liveSpellCheckEnabled = val;
    emit liveSpellCheckEnabledChanged();
}

void SceneDocumentBinder::setAutoCapitalizeSentences(bool val)
{
    if (m_autoCapitalizeSentences == val)
        return;

    m_autoCapitalizeSentences = val;
    emit autoCapitalizeSentencesChanged();
}

void SceneDocumentBinder::setAutoPolishParagraphs(bool val)
{
    if (m_autoPolishParagraphs == val)
        return;

    m_autoPolishParagraphs = val;
    emit autoPolishParagraphsChanged();
}

void SceneDocumentBinder::setTextWidth(qreal val)
{
    if (qFuzzyCompare(m_textWidth, val))
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
#if 0
    Application::log("SceneDocumentBinder(" + this->objectName() + ") cursorPosition: "
                     + QString::number(m_cursorPosition) + " to " + QString::number(val));
#endif
    if (m_initializingDocument || m_pastingContent || m_cursorPosition == val)
        return;

    QScopedValueRollback<bool> rollbackAcceptTextFormatChanges(m_acceptTextFormatChanges, false);
    auto cleanup = qScopeGuard([=]() {
        this->evaluateAutoCompleteHintsAndCompletionPrefix();
        if (m_cursorPosition >= 0)
            qApp->installEventFilter(this);
        else
            qApp->removeEventFilter(this);
    });

    if (m_textDocument == nullptr || this->document() == nullptr) {
        m_cursorPosition = -1;
        m_currentElementCursorPosition = -1;
        m_textFormat->reset();
        emit cursorPositionChanged();
        return;
    }

    m_cursorPosition = val;
    m_currentElementCursorPosition = -1;
    if (m_scene != nullptr)
        m_scene->setCursorPosition(m_cursorPosition);

    if (m_cursorPosition < 0) {
        m_currentElementCursorPosition = -1;
        m_textFormat->reset();
        emit cursorPositionChanged();
        this->setCurrentElement(nullptr);
        return;
    }

#if 1
    // Even if the document is empty, it should have one block for action or character
    // or whatever else default paragraph type, unless the document associated with
    // the scene is yet to be loaded from the scene content itself.
    if ((this->document()->isEmpty() || m_cursorPosition > this->document()->characterCount())
        && m_initializeDocumentTimer.isActive()) {
        m_textFormat->reset();
        emit cursorPositionChanged();
        return;
    }
#endif

    this->setWordUnderCursorIsMisspelled(false);
    this->setSpellingSuggestions(QStringList());

    SpellCheckCursor cursor(this->document(), val);

    QTextBlock block = cursor.block();
    if (!block.isValid()) {
        qDebug("[%d] There is no block at the cursor position %d.", __LINE__, val);
        emit cursorPositionChanged();
        m_textFormat->reset();
        return;
    }

    SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
    if (userData == nullptr) {
        this->syncSceneFromDocument();
        userData = SceneDocumentBlockUserData::get(block);
    }

    if (userData == nullptr) {
        this->setCurrentElement(nullptr);
        m_textFormat->reset();
        qWarning("[%d] TextDocument has a block at %d that isnt backed by a SceneElement!!",
                 __LINE__, val);
    } else {
        this->setCurrentElement(userData->sceneElement());
        this->setWordUnderCursorIsMisspelled(cursor.isMisspelled());
        this->setSpellingSuggestions(cursor.suggestions());

        if (m_selectionStartPosition >= 0 && m_selectionEndPosition > 0
            && m_selectionStartPosition != m_selectionEndPosition) {
            cursor.setPosition(m_selectionStartPosition);
            cursor.setPosition(m_selectionEndPosition, QTextCursor::KeepAnchor);
        }
        const QTextCharFormat format = cursor.charFormat();
        m_textFormat->updateFromCharFormat(format);
    }

    m_currentElementCursorPosition = m_cursorPosition - block.position();
    emit cursorPositionChanged();
}

void SceneDocumentBinder::setSelectionStartPosition(int val)
{
    if (m_selectionStartPosition == val)
        return;

    m_selectionStartPosition = val;
    emit selectionStartPositionChanged();
}

void SceneDocumentBinder::setSelectionEndPosition(int val)
{
    if (m_selectionEndPosition == val)
        return;

    m_selectionEndPosition = val;
    emit selectionEndPositionChanged();
}

bool SceneDocumentBinder::changeTextCase(TextCasing casing)
{
    struct Fragment
    {
        int start = -1;
        int end = -1;
        QTextBlock block;
    };
    QVector<Fragment> fragments;

    if (m_selectionStartPosition >= 0 && m_selectionEndPosition > 0
        && m_selectionEndPosition > m_selectionStartPosition) {
        QTextCursor cursor(this->document());
        cursor.setPosition(m_selectionStartPosition);
        while (1) {
            Fragment fragment;
            fragment.block = cursor.block();
            fragment.start = qMax(m_selectionStartPosition, fragment.block.position());
            fragment.end = qMin(m_selectionEndPosition, [](const QTextBlock &block) {
                QTextCursor c(block);
                c.movePosition(QTextCursor::EndOfBlock);
                return c.position();
            }(fragment.block));
            fragments.append(fragment);
            if (!cursor.movePosition(QTextCursor::NextBlock))
                break;
            if (cursor.atEnd() || cursor.position() > m_selectionEndPosition)
                break;
        }
    } else if (m_cursorPosition >= 0) {
        QTextCursor cursor(this->document());
        cursor.setPosition(m_cursorPosition);
        cursor.select(QTextCursor::WordUnderCursor);

        if (cursor.hasSelection()) {
            Fragment fragment;
            fragment.end = cursor.selectionEnd();
            fragment.start = cursor.selectionStart();
            fragment.block = cursor.block();
            fragments.append(fragment);
        }
    }

    if (fragments.isEmpty())
        return false;

    auto changeTextCase = [casing](const QString &text) {
        switch (casing) {
        case LowerCase:
            return text.toLower();
        case UpperCase:
            return text.toUpper();
        }
        return text;
    };

    QTextCursor cursor(this->document());

    for (int i = fragments.length() - 1; i >= 0; i--) {
        const Fragment fragment = fragments.at(i);

        cursor.setPosition(fragment.start);
        cursor.setPosition(fragment.end, QTextCursor::KeepAnchor);
        cursor.insertText(changeTextCase(cursor.selectedText()));
        cursor.clearSelection();

        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(fragment.block);
        if (userData)
            userData->autoCapitalizeLater();
    }

    return true;
}

void SceneDocumentBinder::setApplyTextFormat(bool val)
{
    if (m_applyTextFormat == val)
        return;

    m_applyTextFormat = val;
    emit applyTextFormatChanged();
}

void SceneDocumentBinder::setCharacterNames(const QStringList &val)
{
    if (m_characterNames == val)
        return;

    m_characterNames = val;
    emit characterNamesChanged();
}

void SceneDocumentBinder::setTransitions(const QStringList &val)
{
    if (m_transitions == val)
        return;

    m_transitions = val;
    emit transitionsChanged();
}

void SceneDocumentBinder::setShots(const QStringList &val)
{
    if (m_shots == val)
        return;

    m_shots = val;
    emit shotsChanged();
}

QList<SceneElement *> SceneDocumentBinder::selectedElements() const
{
    QList<SceneElement *> ret;

    if (m_selectionStartPosition < 0 || m_selectionEndPosition < 0
        || m_selectionEndPosition < m_selectionStartPosition)
        return ret;

    QTextDocument *doc = m_textDocument ? m_textDocument->textDocument() : nullptr;

    QTextCursor cursor(doc);
    cursor.setPosition(m_selectionStartPosition);

    while (1) {
        QTextBlock block = cursor.block();
        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
        if (userData)
            ret << userData->sceneElement();

        if (!cursor.movePosition(QTextCursor::NextBlock))
            break;

        if (cursor.position() > m_selectionEndPosition)
            break;
    }

    return ret;
}

SceneElement *SceneDocumentBinder::sceneElementAt(int cursorPosition) const
{
    QTextDocument *doc = m_textDocument ? m_textDocument->textDocument() : nullptr;
    if (doc == nullptr || cursorPosition < 0)
        return nullptr;

    QTextCursor cursor(doc);
    cursor.setPosition(cursorPosition);

    QTextBlock block = cursor.block();
    SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
    if (userData)
        return userData->sceneElement();

    return nullptr;
}

QRectF SceneDocumentBinder::sceneElementBoundingRect(SceneElement *sceneElement) const
{
    if (sceneElement == nullptr)
        return QRectF();

    QTextDocument *doc = m_textDocument->textDocument();

    QTextBlock block = doc->firstBlock();
    while (block.isValid()) {
        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
        if (userData && userData->sceneElement() == sceneElement) {
            QAbstractTextDocumentLayout *layout = doc->documentLayout();
            QRectF blockRect = layout->blockBoundingRect(block);

            SceneElementFormat *elementFormat =
                    m_screenplayFormat->elementFormat(sceneElement->type());

            const qreal dpr = m_screenplayFormat->devicePixelRatio();
            const qreal contentWidth = doc->textWidth();
            const qreal leftMargin = contentWidth * elementFormat->leftMargin() * dpr;
            blockRect.moveLeft(blockRect.left() + leftMargin);

            return blockRect;
        }

        block = block.next();
    }

    return QRectF();
}

void SceneDocumentBinder::setForceSyncDocument(bool val)
{
    if (m_forceSyncDocument == val)
        return;

    m_forceSyncDocument = val;
    emit forceSyncDocumentChanged();
}

void SceneDocumentBinder::setApplyLanguageFonts(bool val)
{
    if (m_applyLanguageFonts == val)
        return;

    m_applyLanguageFonts = val;
    emit applyLanguageFontsChanged();

    this->refresh();
}

QString SceneDocumentBinder::nextTabFormatAsString() const
{
    auto typeToString = [](int type) {
        switch (type) {
        case SceneElement::Action:
            return QStringLiteral("Action");
        case SceneElement::Character:
            return QStringLiteral("Character");
        case SceneElement::Dialogue:
            return QStringLiteral("Dialogue");
        case SceneElement::Parenthetical:
            return QStringLiteral("Parenthetical");
        case SceneElement::Shot:
            return QStringLiteral("Shot");
        case SceneElement::Transition:
            return QStringLiteral("Transition");
        case SceneElement::Heading:
            return QStringLiteral("Scene Heading");
        }
        return QStringLiteral("Unknown");
    };

    const int ntf = this->nextTabFormat();
    if (ntf < 0)
        return QStringLiteral("Change Format");

    const QString current =
            m_currentElement ? typeToString(m_currentElement->type()) : typeToString(-1);
    const QString next = typeToString(ntf);
    return current + QString(QChar(0x2192)) + next;
}

int SceneDocumentBinder::nextTabFormat() const
{
    if (m_cursorPosition < 0 || m_textDocument == nullptr || m_currentElement == nullptr
        || this->document() == nullptr)
        return -1;

    const int elementNr = m_scene->indexOfElement(m_currentElement);
    if (elementNr < 0)
        return -1;

    switch (m_currentElement->type()) {
    case SceneElement::Action:
        return SceneElement::Character;
    case SceneElement::Character:
        if (m_tabHistory.isEmpty())
            return SceneElement::Action;
        return SceneElement::Transition;
    case SceneElement::Dialogue:
        return SceneElement::Parenthetical;
    case SceneElement::Parenthetical:
        return SceneElement::Dialogue;
    case SceneElement::Shot:
        return SceneElement::Transition;
    case SceneElement::Transition:
        return SceneElement::Action;
    default:
        break;
    }

    return m_currentElement->type();
}

void SceneDocumentBinder::tab()
{
    const int ntf = this->nextTabFormat();
    if (ntf < 0)
        return;

    m_currentElement->setType(SceneElement::Type(ntf));
    m_tabHistory.append(m_currentElement->type());
    emit nextTabFormatChanged();
}

void SceneDocumentBinder::backtab()
{
    // Do nothing. It doesnt work anyway!
}

bool SceneDocumentBinder::canGoUp()
{
    if (m_cursorPosition < 0 || this->document() == nullptr)
        return false;

    QTextCursor cursor(this->document());
    cursor.setPosition(qMax(m_cursorPosition, 0));
    return cursor.movePosition(QTextCursor::Up);
}

bool SceneDocumentBinder::canGoDown()
{
    if (m_cursorPosition < 0 || this->document() == nullptr)
        return false;

    QTextCursor cursor(this->document());
    cursor.setPosition(qMax(m_cursorPosition, 0));
    return cursor.movePosition(QTextCursor::Down);
}

void SceneDocumentBinder::refresh()
{
    if (this->document()) {
        QTextBlock block = this->document()->firstBlock();
        while (block.isValid()) {
            SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
            if (userData) {
                userData->resetFormat();
                userData->initializeSpellCheck(this);
                userData->autoCapitalizeLater();
                userData->polishTextLater();
            }

            block = block.next();
        }

        this->rehighlightLater();
    }
}

void SceneDocumentBinder::reload()
{
    this->initializeDocument();
}

QStringList SceneDocumentBinder::spellingSuggestionsForWordAt(int position) const
{
    if (this->document() == nullptr || m_initializingDocument || position < 0)
        return QStringList();

    SpellCheckCursor cursor(this->document(), position);
    if (cursor.isMisspelled())
        return cursor.suggestions();

    return QStringList();
}

void SceneDocumentBinder::replaceWordAt(int position, const QString &with)
{
    if (this->document() == nullptr || m_initializingDocument || position < 0)
        return;

    SpellCheckCursor cursor(this->document(), position);
    if (!cursor.isMisspelled())
        return;

    cursor.replace(with);
    this->setSpellingSuggestions(QStringList());
    this->setWordUnderCursorIsMisspelled(false);
}

void SceneDocumentBinder::addWordAtPositionToDictionary(int position)
{
    if (this->document() == nullptr || m_initializingDocument || position < 0)
        return;

    SpellCheckCursor cursor(this->document(), position);
    if (!cursor.isMisspelled())
        return;

    if (SpellCheckService::addToDictionary(cursor.word())) {
        cursor.resetCharFormat();
        this->setSpellingSuggestions(QStringList());
        this->setWordUnderCursorIsMisspelled(false);
    }
}

void SceneDocumentBinder::addWordAtPositionToIgnoreList(int position)
{
    if (this->document() == nullptr || m_initializingDocument || position < 0)
        return;

    SpellCheckCursor cursor(this->document(), position);
    if (!cursor.isMisspelled())
        return;

    ScriteDocument::instance()->addToSpellCheckIgnoreList(cursor.word());
    cursor.resetCharFormat();
    this->setSpellingSuggestions(QStringList());
    this->setWordUnderCursorIsMisspelled(false);
}

void SceneDocumentBinder::setCompletionMode(CompletionMode val)
{
    if (m_completionMode == val)
        return;

    m_completionMode = val;
    emit completionModeChanged();
}

int SceneDocumentBinder::lastCursorPosition() const
{
    if (m_cursorPosition < 0 || this->document() == nullptr)
        return 0;

    QTextCursor cursor(this->document());
    cursor.setPosition(qMax(m_cursorPosition, 0));
    cursor.movePosition(QTextCursor::End);
    return cursor.position();
}

int SceneDocumentBinder::cursorPositionAtBlock(int blockNumber) const
{
    if (this->document() != nullptr) {
        const QTextBlock block = this->document()->findBlockByNumber(blockNumber);
        if (m_cursorPosition >= block.position()
            && m_cursorPosition < block.position() + block.length())
            return m_cursorPosition;

        return block.position() + block.length() - 1;
    }

    return -1;
}

int SceneDocumentBinder::currentBlockPosition() const
{
    QTextCursor cursor(this->document());
    cursor.setPosition(m_cursorPosition);
    return cursor.block().position();
}

QFont SceneDocumentBinder::currentFont() const
{
    if (this->document() == nullptr)
        return QFont();

    QTextCursor cursor(this->document());
    cursor.setPosition(qMax(m_cursorPosition, 0));

    QTextCharFormat format = cursor.charFormat();
    return format.font();
}

void SceneDocumentBinder::copy(int fromPosition, int toPosition)
{
    if (this->document() == nullptr)
        return;

    const bool allTextSelected = [=]() -> bool {
        if (fromPosition > 0)
            return false;

        QTextCursor cursor(m_textDocument->textDocument());
        cursor.movePosition(QTextCursor::End);
        return toPosition == cursor.position();
    }();

    QJsonArray content;

    auto addParaToContent = [&content](int type, int alignment, const QString &text,
                                       const QVector<QTextLayout::FormatRange> &formats =
                                               QVector<QTextLayout::FormatRange>()) {
        QJsonObject para;
        para.insert(QStringLiteral("type"), type);
        if (alignment >= 0)
            para.insert(QStringLiteral("alignment"), alignment);
        para.insert(QStringLiteral("text"), text);

        if (!formats.isEmpty()) {
            const QJsonArray jformats = SceneElement::textFormatsToJson(formats);
            para.insert(QStringLiteral("formats"), jformats);
        }

        content.append(para);
    };

    Fountain::Body fBody;

    if (allTextSelected && m_scene->heading()->isEnabled()) {
        // Copy the scene heading and synopsis to both fountain and JSON representations
        Fountain::Element fElement;
        fElement.text = m_scene->heading()->displayText();
        fElement.sceneNumber =
                m_screenplayElement ? m_screenplayElement->userSceneNumber() : QString();
        fElement.type = Fountain::Element::SceneHeading;
        fBody.append(fElement);

        fElement = Fountain::Element();
        if (!m_scene->synopsis().isEmpty()) {
            fElement.text = m_scene->synopsis();
            fElement.type = Fountain::Element::Synopsis;
            fBody.append(fElement);
        }

        addParaToContent(SceneElement::Heading, 0, m_scene->heading()->displayText());

        // Add scene number and synopsis to scene heading itself.
        QJsonObject headingPara = content.last().toObject();
        headingPara.insert(QStringLiteral("sceneNumber"), m_screenplayElement->userSceneNumber());
        headingPara.insert(QStringLiteral("synopsis"), m_scene->synopsis());
        content[content.size() - 1] = headingPara;
    }

    QTextCursor cursor(this->document());
    cursor.setPosition(fromPosition);

    QTextBlock block = cursor.block();
    while (block.isValid() && toPosition > block.position()) {
        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
        if (userData == nullptr) {
            block = block.next();
            continue;
        }

        const int bstart = block.position();
        const int bend = [=]() {
            QTextCursor c(block);
            c.movePosition(QTextCursor::EndOfBlock);
            return c.position();
        }();
        cursor.setPosition(qMax(fromPosition, bstart));
        cursor.setPosition(qMin(toPosition, bend), QTextCursor::KeepAnchor);

        SceneElement *element = userData->sceneElement();

        const QVector<QTextLayout::FormatRange> blockFormats = block.textFormats();
        QVector<QTextLayout::FormatRange> formatsToCopy;
        formatsToCopy.reserve(blockFormats.size());
        for (const QTextLayout::FormatRange &format : blockFormats) {
            const int fstart = format.start - (fromPosition <= bstart ? 0 : fromPosition - bstart);
            const int flength = format.length + qMin(fstart, 0);

            QTextLayout::FormatRange fmt;
            fmt.start = qMax(fstart, 0);
            fmt.length = flength;
            fmt.format = format.format;
            formatsToCopy.append(fmt);
        }

        addParaToContent(element->type(), element->alignment(), cursor.selectedText(),
                         formatsToCopy);

        Fountain::Element fElement;
        fElement.text = cursor.selectedText();
        fElement.formats = formatsToCopy;
        switch (element->type()) {
        default:
        case SceneElement::Action:
            fElement.type = Fountain::Element::Action;
            break;
        case SceneElement::Character:
            fElement.type = Fountain::Element::Character;
            break;
        case SceneElement::Dialogue:
            fElement.type = Fountain::Element::Dialogue;
            break;
        case SceneElement::Parenthetical:
            fElement.type = Fountain::Element::Parenthetical;
            break;
        case SceneElement::Shot:
            fElement.type = Fountain::Element::Shot;
            break;
        case SceneElement::Transition:
            fElement.type = Fountain::Element::Transition;
            break;
        }

        fBody.append(fElement);

        block = block.next();
    }

    const QByteArray contentJson = QJsonDocument(content).toJson(QJsonDocument::Compact);

    QClipboard *clipboard = Application::instance()->clipboard();
    QMimeData *mimeData = new QMimeData;
    mimeData->setData(QStringLiteral("scrite/scene"), contentJson);
    mimeData->setText(Fountain::Writer(fBody,
                                       fBody.size() > 1 ? Screenplay::fountainCopyOptions()
                                                        : Fountain::Writer::NoOption)
                              .toString());
    clipboard->setMimeData(mimeData);
}

int SceneDocumentBinder::paste(int fromPosition)
{
    if (this->document() == nullptr || m_pastingContent)
        return -1;

    QScopedValueRollback<bool> pastingContentRollback(m_pastingContent, true);

    struct Paragraph
    {
        Paragraph() { }
        Paragraph(const QString &_text, SceneElement::Type _type = SceneElement::Action)
            : text(_text), type(_type)
        {
        }

        QString text;
        SceneElement::Type type = SceneElement::Action;
        Qt::Alignment alignment;
        QVector<QTextLayout::FormatRange> formats;
    };

    QVector<Paragraph> paragraphs;

    const QClipboard *clipboard = Application::instance()->clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    const QByteArray contentJson = mimeData->data(QStringLiteral("scrite/scene"));
    if (contentJson.isEmpty()) {
        if (mimeData->hasText()) {
            const QString text = mimeData->text();
            if (text.contains('\n')) {
                Fountain::Parser parser(text, Screenplay::fountainPasteOptions());

                bool applySceneHeading = fromPosition == 0 && m_scene->isEmpty();

                const Fountain::Body fBody = parser.body();
                if (fBody.size() == 1 && fBody.first().type == Fountain::Element::Action) {
                    const QStringList lines = text.split('\n');
                    for (const QString &line : lines) {
                        Paragraph paragraph;
                        paragraph.text = line;
                        paragraphs.append(paragraph);
                    }
                } else {
                    for (const Fountain::Element &element : fBody) {
                        Paragraph paragraph;
                        paragraph.text = element.text;
                        paragraph.formats = element.formats;

                        bool includeParagraph = true;
                        switch (element.type) {
                        case Fountain::Element::SceneHeading:
                            if (applySceneHeading) {
                                m_scene->heading()->parseFrom(element.text);
                                if (!element.sceneNumber.isEmpty()
                                    && m_screenplayElement != nullptr)
                                    m_screenplayElement->setUserSceneNumber(element.sceneNumber);
                                applySceneHeading = false;
                                includeParagraph = false;
                            } else {
                                paragraph.type = SceneElement::Action;
                                applySceneHeading = false;
                            }
                            break;
                        case Fountain::Element::Action:
                            paragraph.type = SceneElement::Action;
                            applySceneHeading = false;
                            break;
                        case Fountain::Element::Character:
                            paragraph.type = SceneElement::Character;
                            applySceneHeading = false;
                            break;
                        case Fountain::Element::Parenthetical:
                            paragraph.type = SceneElement::Parenthetical;
                            applySceneHeading = false;
                            break;
                        case Fountain::Element::Dialogue:
                            paragraph.type = SceneElement::Dialogue;
                            applySceneHeading = false;
                            break;
                        case Fountain::Element::Shot:
                            paragraph.type = SceneElement::Shot;
                            applySceneHeading = false;
                            break;
                        case Fountain::Element::Transition:
                            paragraph.type = SceneElement::Transition;
                            applySceneHeading = false;
                            break;
                        case Fountain::Element::Synopsis:
                            includeParagraph = false;
                            if (!element.text.isEmpty()) {
                                QString synopsis = m_scene->synopsis();
                                if (!synopsis.isEmpty())
                                    synopsis += "\n\n";
                                synopsis += element.text;
                                m_scene->setSynopsis(element.text);
                            }
                            break;
                        default:
                            includeParagraph = false;
                            break;
                        }

                        if (includeParagraph)
                            paragraphs.append(paragraph);
                    }
                }
            } else {
                Paragraph paragraph;
                paragraph.text = text;
                paragraphs.append(paragraph);
            }
        }
    } else {
        const QJsonArray content = QJsonDocument::fromJson(contentJson).array();
        if (content.isEmpty())
            return -1;

        bool applySceneHeading = fromPosition == 0 && m_scene->isEmpty();

        for (const QJsonValue &item : content) {
            const QJsonObject itemObject = item.toObject();
            const int type = itemObject.value(QStringLiteral("type")).toInt();
            const int alignment = itemObject.value(QStringLiteral("alignment")).toInt();
            const QString text = itemObject.value(QStringLiteral("text")).toString();

            if (applySceneHeading && type == SceneElement::Heading) {
                m_scene->heading()->parseFrom(text);
                m_screenplayElement->setUserSceneNumber(
                        itemObject.value(QStringLiteral("sceneNumber")).toString());
                m_scene->setSynopsis(itemObject.value(QStringLiteral("synopsis")).toString());
                applySceneHeading = false;
                continue;
            }

            Paragraph paragraph;
            paragraph.type = (type < SceneElement::Min || type > SceneElement::Max
                              || type == SceneElement::Heading)
                    ? SceneElement::Action
                    : SceneElement::Type(type);
            paragraph.text = text;
            paragraph.alignment = alignment == 0 ? Qt::Alignment() : Qt::Alignment(alignment);
            paragraph.formats = SceneElement::textFormatsFromJson(
                    itemObject.value(QStringLiteral("formats")).toArray());
            paragraphs.append(paragraph);

            applySceneHeading = false;
        }
    }

    fromPosition = fromPosition >= 0 ? fromPosition : m_cursorPosition;

    QTextCursor cursor(this->document());
    cursor.setPosition(fromPosition);

    const bool pasteFormatting = paragraphs.size() > 1;
    QTextBlock lastPastedBlock;

    int finalCursorPos = fromPosition;

    for (int i = 0; i < paragraphs.size(); i++) {
        const Paragraph paragraph = paragraphs.at(i);
        if (i > 0)
            cursor.insertBlock(QTextBlockFormat(), QTextCharFormat());

        const int bstart = cursor.position();
        cursor.insertText(paragraph.text);
        const int bend = cursor.position();
        finalCursorPos = bend;

        lastPastedBlock = cursor.block();
        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(lastPastedBlock);
        if (userData && userData->sceneElement())
            userData->sceneElement()->setText(lastPastedBlock.text());

        if (userData && pasteFormatting) {
            if (userData->sceneElement()) {
                userData->sceneElement()->setType(paragraph.type);
                userData->sceneElement()->setAlignment(paragraph.alignment);
            }
            userData->resetFormat();
        }

        if (!paragraph.formats.isEmpty()) {
            cursor.setPosition(bstart);
            for (const QTextLayout::FormatRange &format : paragraph.formats) {
                cursor.setPosition(bstart + format.start);
                cursor.setPosition(bstart + format.start + format.length, QTextCursor::KeepAnchor);
                cursor.setCharFormat(format.format);
                cursor.clearSelection();
            }
            cursor.setPosition(bend);
        }
    }

    const int cp = finalCursorPos;
    emit requestCursorPosition(cp);

    m_sceneElementTaskTimer.stop();
    this->performAllSceneElementTasks();

    QTimer::singleShot(50, this, [this, cp]() {
        this->refresh();
        emit requestCursorPosition(cp);
    });

    return cp;
}

void SceneDocumentBinder::setApplyFormattingEvenInTransaction(bool val)
{
    if (m_applyFormattingEvenInTransaction == val)
        return;

    m_applyFormattingEvenInTransaction = val;
    emit applyFormattingEvenInTransactionChanged();
}

void SceneDocumentBinder::classBegin() { }

void SceneDocumentBinder::componentComplete()
{
    m_initializeDocumentTimer.stop();
    this->initializeDocument();
}

void SceneDocumentBinder::highlightBlock(const QString &text)
{
    if (m_initializingDocument || m_sceneElementTaskIsRunning)
        return;

    if (m_screenplayFormat == nullptr)
        return;

    QTextBlock block = this->QSyntaxHighlighter::currentBlock();
    SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
    if (userData == nullptr) {
        this->syncSceneFromDocument();
        userData = SceneDocumentBlockUserData::get(block);
    }

    if (userData == nullptr) {
        qWarning("[%d] TextDocument has a block that isnt backed by a SceneElement!!", __LINE__);
        return;
    }

    SceneElement *element = userData->sceneElement();
    if (element == nullptr) {
        qWarning("[%d] TextDocument has a block that isnt backed by a SceneElement!!", __LINE__);
        return;
    }

    // Basic formatting
    const SceneElementFormat *format = m_screenplayFormat->elementFormat(element->type());
    if (userData->shouldUpdateFromFormat(format)) {
        userData->blockFormat = format->createBlockFormat(element->alignment());
        userData->charFormat = format->createCharFormat();
        this->applyBlockFormatLater(block);
    }

    this->mergeFormat(0, block.length(), userData->charFormat);

    // Per-language fonts.
    if (m_applyLanguageFonts) {
        const QList<TransliterationEngine::Boundary> boundaries =
                TransliterationEngine::instance()->evaluateBoundaries(text);

        for (const TransliterationEngine::Boundary &boundary : boundaries) {
            if (boundary.isEmpty() || boundary.language == TransliterationEngine::English)
                continue;

            QTextCharFormat format;
            format.setFontFamily(boundary.font.family());
            this->mergeFormat(boundary.start, boundary.end - boundary.start + 1, format);
        }

        if (m_currentElement == element)
            emit currentFontChanged();
    }

    // Spelling mistakes.
    const QList<TextFragment> fragments = userData->misspelledFragments();
    if (!fragments.isEmpty()) {
        for (const TextFragment &fragment : fragments) {
            if (!fragment.isValid())
                continue;

            const QString word = text.mid(fragment.start(), fragment.length());
            const QChar::Script script = TransliterationEngine::determineScript(word);
            if (script != QChar::Script_Latin)
                continue;

            QTextCharFormat spellingErrorFormat;
#if 0
            spellingErrorFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
            spellingErrorFormat.setUnderlineColor(Qt::red);
            spellingErrorFormat.setFontUnderline(true);
#else
            spellingErrorFormat.setBackground(QColor(255, 0, 0, 32));
#endif
            this->mergeFormat(fragment.start(), fragment.length(), spellingErrorFormat);
        }

        /*
        Suppose that a paragraph of text has custom text and/or background
        colors applied to one or more words in it. When spell check is enabled
        and a paragraph of text has spelling mistakes, then the custom colors
        are not rendered until all the spelling mistakes are fixed.

        It appears that setting background color to light-red (as we do here)
        for misspelled words, causes all colors to disappear in that paragraph.
        I am unable to reproduce this issue on a separate sample Qt app; and I
        am unable to find out why its causing this issue here.

        Until we figure out why this is happening, we reapply background and
        foreground colors whenever spelling mistakes are detected.
        */

        const QVector<QTextLayout::FormatRange> formats = block.textFormats();
        for (const QTextLayout::FormatRange &format : formats) {
            if (format.format.hasProperty(QTextFormat::BackgroundBrush)
                || format.format.hasProperty(QTextFormat::ForegroundBrush)) {
                QTextCharFormat charFormat;
                if (format.format.hasProperty(QTextFormat::BackgroundBrush))
                    charFormat.setBackground(format.format.background());
                if (format.format.hasProperty(QTextFormat::ForegroundBrush))
                    charFormat.setForeground(format.format.foreground());
                this->mergeFormat(format.start, format.length, charFormat);
            }
        }

        emit spellingMistakesDetected();
    }
}

void SceneDocumentBinder::timerEvent(QTimerEvent *te)
{
    if (te->timerId() == m_initializeDocumentTimer.timerId()) {
        m_initializeDocumentTimer.stop();
        this->initializeDocument();
    } else if (te->timerId() == m_rehighlightTimer.timerId()) {
        m_rehighlightTimer.stop();

        const int nrBlocks = this->document()->blockCount();
        const int nrTresholdBlocks = nrBlocks >> 1;
        const QList<QTextBlock> queue = m_rehighlightBlockQueue;
        m_rehighlightBlockQueue.clear();

        if (queue.size() > nrTresholdBlocks || queue.isEmpty()) {
            this->QSyntaxHighlighter::rehighlight();
        } else {
            for (const QTextBlock &block : queue)
                this->rehighlightBlock(block);
        }
    } else if (te->timerId() == m_sceneElementTaskTimer.timerId()) {
        m_sceneElementTaskTimer.stop();
        this->performAllSceneElementTasks();
    } else if (te->timerId() == m_applyBlockFormatTimer.timerId()) {
        m_applyBlockFormatTimer.stop();

        const QList<QTextBlock> queue = m_applyBlockFormatQueue;
        m_applyBlockFormatQueue.clear();
        for (const QTextBlock &block : queue) {
            SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
            if (userData) {
                QTextCursor cursor(block);
                cursor.setBlockFormat(userData->blockFormat);
            }
        }
    }
}

bool SceneDocumentBinder::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)

    if (m_cursorPosition >= 0
        && QList<int>({ QEvent::KeyPress, QEvent::KeyRelease, QEvent::Shortcut,
                        QEvent::ShortcutOverride })
                   .contains(event->type())) {
        if (m_sceneElementTaskTimer.isActive())
            m_sceneElementTaskTimer.start(500, this);
    }

    return false;
}

void SceneDocumentBinder::mergeFormat(int start, int count, const QTextCharFormat &format)
{
    for (int i = start; i < start + count; i++) {
        QTextCharFormat mergedFormat = this->format(i);
        mergedFormat.merge(format);
        this->setFormat(i, 1, mergedFormat);
    }
}

void SceneDocumentBinder::resetScene()
{
    m_scene = nullptr;
    emit sceneChanged();

    this->initializeDocumentLater();
}

void SceneDocumentBinder::resetTextDocument()
{
    m_textDocument = nullptr;
    this->QSyntaxHighlighter::setDocument(nullptr);
    this->setDocumentLoadCount(0);
    this->evaluateAutoCompleteHintsAndCompletionPrefix();
    emit textDocumentChanged();
    this->setCursorPosition(-1);
}

void SceneDocumentBinder::resetScreenplayFormat()
{
    m_screenplayFormat = nullptr;
    emit screenplayFormatChanged();
}

void SceneDocumentBinder::resetScreenplayElement()
{
    if (m_screenplayElement != nullptr) {
        Screenplay *screenplay = m_screenplayElement->screenplay();
        disconnect(screenplay, &Screenplay::elementMoved, this,
                   &SceneDocumentBinder::polishAllSceneElements);
    }

    m_screenplayElement = nullptr;
    emit screenplayElementChanged();
}

void SceneDocumentBinder::initializeDocument()
{
    if (m_textDocument == nullptr || m_scene == nullptr || m_screenplayFormat == nullptr)
        return;

    m_initializingDocument = true;

    m_tabHistory.clear();

    QFont defaultFont = m_screenplayFormat->defaultFont();
    defaultFont.setPointSize(defaultFont.pointSize() + m_screenplayFormat->fontPointSizeDelta());

    QTextDocument *document = m_textDocument->textDocument();
    QSignalBlocker documentSignalBlocker(document);
    document->clear();
    document->setDefaultFont(defaultFont);
    document->setUseDesignMetrics(true);

    const int nrElements = m_scene->elementCount();

    QTextCursor cursor(document);
    QList<QTextBlock> blocks;

    // In the first pass, we simply insert text into the document.
    for (int i = 0; i < nrElements; i++) {
        SceneElement *element = m_scene->elementAt(i);
        if (i > 0)
            cursor.insertBlock();

        QTextBlock block = cursor.block();
        if (!block.isValid() && i == 0) {
            cursor.insertBlock();
            block = cursor.block();
        }

        SceneDocumentBlockUserData *userData = new SceneDocumentBlockUserData(block, element, this);
        block.setUserData(userData);
        cursor.insertText(element->text());
        blocks.append(block);
    }

    // In the second pass, we apply formatting to inserted text. We have to do this in the second
    // pass, because QTextDocument tends to pass character format at the last position of the
    // previous block, into the next block also. So for instance, if we have a fully bold paragraph
    // followed by a normal paragraph, QTextDocument will apply fully bold to both if we apply
    // text-formats while inserting text.
    for (QTextBlock &block : blocks) {
        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
        const SceneElement *element = userData->sceneElement();
        const SceneElementFormat *format = m_screenplayFormat->elementFormat(element->type());

        cursor = QTextCursor(block);

        userData->resetFormat();
        if (userData->shouldUpdateFromFormat(format)) {
            userData->blockFormat = format->createBlockFormat(element->alignment());
            userData->charFormat = format->createCharFormat();
        }
        cursor.setBlockFormat(userData->blockFormat);

        const QVector<QTextLayout::FormatRange> formatRanges =
                userData->sceneElement()->textFormats();
        if (formatRanges.isEmpty())
            continue;

        const int startPos = cursor.position();

        for (const QTextLayout::FormatRange &formatRange : formatRanges) {
            cursor.setPosition(startPos + formatRange.start);
            cursor.setPosition(startPos + formatRange.start + formatRange.length,
                               QTextCursor::KeepAnchor);
            cursor.mergeCharFormat(formatRange.format);
            cursor.clearSelection();
        }

        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
    }

    documentSignalBlocker.unblock();

    if (m_cursorPosition <= 0 && m_currentElement == nullptr && nrElements == 1)
        this->setCurrentElement(m_scene->elementAt(0));

    this->setDocumentLoadCount(m_documentLoadCount + 1);
    m_initializingDocument = false;
    this->QSyntaxHighlighter::rehighlight();
    this->polishAllSceneElements();

    emit documentInitialized();
}

void SceneDocumentBinder::initializeDocumentLater()
{
    m_initializeDocumentTimer.start(0, this);
}

void SceneDocumentBinder::setDocumentLoadCount(int val)
{
    if (m_documentLoadCount == val)
        return;

    m_documentLoadCount = val;
    emit documentLoadCountChanged();
}

void SceneDocumentBinder::setCurrentElement(SceneElement *val)
{
    if (m_currentElement == val)
        return;

    if (m_currentElement != nullptr) {
        disconnect(m_currentElement, &SceneElement::aboutToDelete, this,
                   &SceneDocumentBinder::resetCurrentElement);
        disconnect(m_currentElement, &SceneElement::typeChanged, this,
                   &SceneDocumentBinder::nextTabFormatChanged);
    }

    m_currentElement = val;

    if (m_currentElement != nullptr) {
        connect(m_currentElement, &SceneElement::aboutToDelete, this,
                &SceneDocumentBinder::resetCurrentElement);
        connect(m_currentElement, &SceneElement::typeChanged, this,
                &SceneDocumentBinder::nextTabFormatChanged);
    }

    emit currentElementChanged();

    m_tabHistory.clear();
    this->polishAllSceneElements();

    emit currentFontChanged();
}

void SceneDocumentBinder::resetCurrentElement()
{
    m_currentElement = nullptr;
    emit currentElementChanged();

    m_tabHistory.clear();
    this->evaluateAutoCompleteHintsAndCompletionPrefix();

    emit currentFontChanged();
}

void SceneDocumentBinder::activateCurrentElementDefaultLanguage()
{
    if (m_currentElement && m_screenplayFormat) {
        SceneElementFormat *format = m_screenplayFormat->elementFormat(m_currentElement->type());
        if (format != nullptr) {
#if 0
            Application::log("SceneDocumentBinder(" + this->objectName()
                             + ") activating default language for "
                             + QString::number(m_currentElement->type()));
#endif
            format->activateDefaultLanguage();
        }
    }
}

class ForceCursorPositionHack : public QObject
{
public:
    explicit ForceCursorPositionHack(const QTextBlock &block, int cp, SceneDocumentBinder *binder);
    ~ForceCursorPositionHack();

    void timerEvent(QTimerEvent *event);

private:
    QTextBlock m_block;
    int m_cursorBlockPosition = 0; // within m_block
    ExecLaterTimer m_timer;
    SceneDocumentBinder *m_binder = nullptr;
};

ForceCursorPositionHack::ForceCursorPositionHack(const QTextBlock &block, int cbp,
                                                 SceneDocumentBinder *binder)
    : QObject(const_cast<QTextDocument *>(block.document())),
      m_block(block),
      m_cursorBlockPosition(cbp),
      m_timer("ForceCursorPositionHack.m_timer"),
      m_binder(binder)
{
    m_timer.start(0, this);
}

ForceCursorPositionHack::~ForceCursorPositionHack() { }

void ForceCursorPositionHack::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timer.timerId()) {
        m_timer.stop();

        QScopedValueRollback<bool> rollback(m_binder->m_sceneElementTaskIsRunning, true);

        QTextCursor cursor(m_block);
        cursor.insertText(QStringLiteral("("));
        cursor.deletePreviousChar();

        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(m_block);
        if (userData && userData->sceneElement()->type() == SceneElement::Parenthetical) {
            const QString bo = QStringLiteral("(");
            const QString bc = QStringLiteral(")");

            if (m_block.text().isEmpty()) {
                cursor.insertText(QStringLiteral("()"));
                m_cursorBlockPosition = 1;
            } else {
                const QString blockText = m_block.text();
                if (!blockText.startsWith(bo)) {
                    cursor.insertText(bo);
                    m_cursorBlockPosition += 1;
                }
                if (!blockText.endsWith(bc)) {
                    cursor.movePosition(QTextCursor::EndOfBlock);
                    cursor.insertText(bc);
                }
            }
        }

        if (!m_binder->m_autoCompleteHints.isEmpty())
            m_binder->evaluateAutoCompleteHintsAndCompletionPrefix();

        emit m_binder->requestCursorPosition(m_block.position() + m_cursorBlockPosition);

        GarbageCollector::instance()->add(this);
    }
}

void SceneDocumentBinder::onSceneElementChanged(SceneElement *element,
                                                Scene::SceneElementChangeType type)
{
    if (m_initializingDocument)
        return;

    if (m_textDocument == nullptr || this->document() == nullptr || m_scene == nullptr
        || element->scene() != m_scene)
        return;

    if (m_forceSyncDocument)
        this->initializeDocumentLater();

    if (type != Scene::ElementTypeChange)
        return;

    if (m_currentElement != nullptr && element == m_currentElement) {
        SceneElementFormat *format = m_screenplayFormat->elementFormat(m_currentElement->type());
        if (format != nullptr)
            format->activateDefaultLanguage();
    }

    this->evaluateAutoCompleteHintsAndCompletionPrefix();

    auto updateBlock = [=](const QTextBlock &block) {
        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
        if (userData != nullptr && userData->sceneElement() == element) {
            // Text changes from scene element to block are not applied
            // Only element type changes can be applied.
            const SceneElementFormat *format = m_screenplayFormat->elementFormat(element->type());
            userData->blockFormat = format->createBlockFormat(element->alignment());
            userData->charFormat = format->createCharFormat();

            QTextCursor cursor(block);
            cursor.setBlockFormat(userData->blockFormat);

            if (m_cursorPosition >= block.position()
                && m_cursorPosition <= block.position() + block.length())
                new ForceCursorPositionHack(block, m_cursorPosition - block.position(), this);

            this->rehighlightBlockLater(block);

            return true;
        }
        return false;
    };

    const int elementNr = m_scene->indexOfElement(element);
    QTextBlock block;

    if (elementNr >= 0) {
        block = this->document()->findBlockByNumber(elementNr);
        if (updateBlock(block))
            return;
    }

    block = this->document()->firstBlock();
    while (block.isValid()) {
        if (updateBlock(block))
            return;

        block = block.next();
    }
}

void SceneDocumentBinder::onSpellCheckUpdated()
{
    if (m_scene == nullptr || this->document() == nullptr || m_initializingDocument)
        return;

    SpellCheckService *spellCheck = qobject_cast<SpellCheckService *>(this->sender());
    if (spellCheck == nullptr)
        return;

    SceneElement *element = qobject_cast<SceneElement *>(spellCheck->parent());
    if (element == nullptr)
        return;

    const int elementIndex = m_scene->indexOfElement(element);
    if (elementIndex < 0)
        return;

    const QTextBlock block = this->document()->findBlockByNumber(elementIndex);
    if (block.isValid())
        this->rehighlightBlockLater(block);
}

void SceneDocumentBinder::onContentsChange(int from, int charsRemoved, int charsAdded)
{
    if (m_initializingDocument || m_sceneIsBeingReset || m_sceneElementTaskIsRunning
        || m_cursorPosition < 0)
        return;

    if (m_textDocument == nullptr || m_scene == nullptr || this->document() == nullptr)
        return;

    m_tabHistory.clear();

    if (m_sceneElementTaskTimer.isActive())
        m_sceneElementTaskTimer.start(500, this);

    if (m_scene->elementCount() != this->document()->blockCount()) {
        /**
          If the number of paragraphs in the document is differnet from the number of
          paragraphs in our internal Scene data structure, then we better sync it once.
          This can happen when user pastes more than 1 paragraphs at once or if the user
          deletes more than 1 paragraphs at once.
          */
        this->syncSceneFromDocument();
        return;
    }

    QTextCursor cursor(this->document());
    cursor.setPosition(from);

    // Auto-capitalize first letter of each sentence.
    if (m_autoCapitalizeSentences && charsRemoved == 0) {
        QTextBlock block = cursor.block();
        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
        if (userData)
            userData->autoCapitalizeLater();
    }

    // Fixed an issue that caused formatting to not get applied on the next
    // character, when there is no selection or word under the cursor.
    if (charsAdded == 1 && charsRemoved == 0 && m_applyNextCharFormat) {
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 1);
        if (cursor.selectedText() != QStringLiteral(" ")) {
            m_applyNextCharFormat = false;
            cursor.setCharFormat(m_nextCharFormat);

            const QTextCharFormat ncf = m_nextCharFormat;
            QTextDocument *doc = this->document();
            QTimer::singleShot(0, this, [from, ncf, doc]() {
                QTextCursor cursor(doc);
                cursor.setPosition(from);
                cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 1);
                cursor.setCharFormat(ncf);
            });
        }
    }

    do {
        QTextBlock block = cursor.block();
        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
        if (userData == nullptr) {
            this->syncSceneFromDocument();
            return;
        }

        SceneElement *sceneElement = userData->sceneElement();
        if (sceneElement == nullptr) {
            qWarning("[%d] TextDocument has a block at %d that isnt backed by a SceneElement!!",
                     __LINE__, from);
            return;
        }

        sceneElement->setText(block.text());
        sceneElement->setTextFormats(block.textFormats());

        if (m_spellCheckEnabled && m_liveSpellCheckEnabled
            && ((charsAdded > 0 || charsRemoved > 0) && charsAdded != charsRemoved))
            userData->scheduleSpellCheckUpdate();

        if (!cursor.movePosition(QTextCursor::NextBlock))
            break;
    } while (!cursor.atEnd() && cursor.position() < from + charsAdded);

    if (m_cursorPosition >= 0) {
        cursor.setPosition(m_cursorPosition);
        m_textFormat->updateFromCharFormat(cursor.charFormat());
    }
}

void SceneDocumentBinder::syncSceneFromDocument(int nrBlocks)
{
    if (m_initializingDocument || m_sceneIsBeingReset)
        return;

    if (m_textDocument == nullptr || m_scene == nullptr)
        return;

    // Ofcourse we are refreshing the scene because the document changed.
    // But when we refresh the scene, the scene emits sceneRefreshed() signal
    // which will cause SceneDocumentBinder::onSceneRefreshed() to be called,
    // which is entirely unnecessary. We use this boolean to avoid that.
    QScopedValueRollback<bool> rollback(m_sceneIsBeingRefreshed, true);

    if (nrBlocks < 0)
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

    bool doPolishElements = false;

    m_scene->beginUndoCapture();

    QList<SceneElement *> elementList;
    elementList.reserve(nrBlocks);

    QTextBlock block = this->document()->begin();
    QTextBlock previousBlock;
    while (block.isValid()) {
        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
        if (userData == nullptr) {
            SceneElement *newElement = new SceneElement(m_scene);

            if (previousBlock.isValid()) {
                SceneDocumentBlockUserData *prevUserData =
                        SceneDocumentBlockUserData::get(previousBlock);
                SceneElement *prevElement = prevUserData->sceneElement();
                newElement->setType(prevElement->type());

                switch (prevElement->type()) {
                case SceneElement::Action:
                    newElement->setType(SceneElement::Action);
                    newElement->setAlignment(prevElement->alignment());
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
            } else {
                newElement->setType(SceneElement::Action);
                m_scene->insertElementAt(newElement, 0);
            }

            userData = new SceneDocumentBlockUserData(block, newElement, this);
            block.setUserData(userData);
            doPolishElements = true;
        }

        elementList.append(userData->sceneElement());
        userData->sceneElement()->setText(block.text());
        userData->sceneElement()->setTextFormats(block.textFormats());
        userData->autoCapitalizeLater();

        previousBlock = block;
        block = block.next();
    }

    m_scene->setElementsList(elementList);
    m_scene->endUndoCapture();

    if (doPolishElements)
        this->polishAllSceneElements();
}

void SceneDocumentBinder::evaluateAutoCompleteHintsAndCompletionPrefix()
{
    QStringList hints;
    QStringList priorityHints;
    QString completionPrefix;
    int completionStart = -1;
    int completionEnd = -1;

    if (m_currentElement == nullptr || m_cursorPosition < 0) {
        this->setAutoCompleteHints(hints, priorityHints);
        this->setCompletionPrefix(completionPrefix, completionStart, completionEnd);
        return;
    }

    QTextCursor cursor(m_textDocument->textDocument());
    cursor.setPosition(m_cursorPosition);

    const QTextBlock block = cursor.block();
    completionStart = block.position();
    completionEnd = m_cursorPosition;

    CompletionMode completionMode = NoCompletionMode;

    switch (m_currentElement->type()) {
    case SceneElement::Character: {
        const QString bracketOpen = QLatin1String(" (");
        const QString blockText = block.text();
        if (blockText != bracketOpen && blockText.contains(bracketOpen)) {
            const QTextCursor bracketCursor =
                    m_textDocument->textDocument()->find(bracketOpen, block.position());
            if (m_cursorPosition > bracketCursor.selectionStart()) {
                /*
                There are several common notations that can be used in brackets after a character's
                name in a screenplay. Here are a few examples:

                - (V.O.) - This stands for "voiceover" and indicates that the character's dialogue
                is being heard on the soundtrack, but they are not physically present in the scene.
                - (O.S.) - This stands for "off-screen" and indicates that the character is speaking
                from outside the frame or from a location that is not visible to the audience.
                - (O.C.) - This stands for "off-camera" and indicates that the character is speaking
                from a location that is not within the frame of the camera, but they are physically
                present in the scene.
                - (CONT'D) - This indicates that the character's dialogue continues from the
                previous page or shot.
                - (PHONE) - This indicates that the character is speaking on the phone.
                - (INTO PHONE) - This indicates that the character is speaking into a phone or other
                communication device.
                - (FILTERED) - This indicates that the character's voice is being filtered or
                altered in some way.
                - (SUBTITLED) - This indicates that the character's dialogue is being presented as
                subtitles on the screen.
                - (THROUGH TRANSLATOR) - This indicates that the character is speaking through a
                translator or interpreter.
                - (OVER RADIO) - This indicates that the character is speaking over a radio or other
                communication device.
                - (ON TV) - This indicates that the character is speaking on a television or other
                video device.
                - (ON COMPUTER) - This indicates that the character is speaking through a computer
                or other electronic device.
                - (ON SPEAKERPHONE) - This indicates that the character is speaking on a
                speakerphone or other device that allows multiple people to hear the conversation.
                - (OVER INTERCOM) - This indicates that the character is speaking over an intercom
                or other public address system.

                In Scrite, CONT'D is automatically generated, so we don't really have to list it.
                But we will list it anyway because users will flapg it as a bug.
                 */
                static QStringList commonBracketNotations(
                        { QLatin1String("V.O."), QLatin1String("O.S."), QLatin1String("O.C."),
                          QLatin1String("CONT'D"), QLatin1String("PHONE"),
                          QLatin1String("INTO PHONE"), QLatin1String("FILTERED"),
                          QLatin1String("SUBTITLED"), QLatin1String("THROUGH TRANSLATOR"),
                          QLatin1String("OVER RADIO"), QLatin1String("ON TV"),
                          QLatin1String("ON COMPUTER"), QLatin1String("ON SPEAKERPHONE"),
                          QLatin1String("OVER INTERCOM") });
                hints = commonBracketNotations;
                priorityHints = commonBracketNotations;
                completionStart = bracketCursor.position();

                cursor.setPosition(bracketCursor.position());
                cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
                completionPrefix = cursor.selectedText().simplified();
                completionMode = CharacterBracketNotationCompletionMode;
            } else {
                cursor.setPosition(block.position());
                cursor.setPosition(m_cursorPosition, QTextCursor::KeepAnchor);
                hints = m_characterNames;
                priorityHints = m_scene->characterNames();
                completionPrefix = cursor.selectedText().trimmed();
                completionStart = block.position();
                completionEnd = bracketCursor.selectionStart();
                completionMode = CharacterNameCompletionMode;
            }
        } else {
            hints = m_characterNames;
            priorityHints = m_scene->characterNames();
            completionPrefix = blockText;
            completionMode = CharacterNameCompletionMode;
        }
    } break;
    case SceneElement::Transition:
        hints = m_transitions;
        completionPrefix = block.text();
        completionMode = TransitionCompletionMode;
        break;
    case SceneElement::Shot:
        hints = m_shots;
        completionPrefix = block.text();
        completionMode = ShotCompletionMode;
        break;
    default:
        break;
    }

    this->setAutoCompleteHints(hints, priorityHints);
    this->setCompletionPrefix(completionPrefix, completionStart, completionEnd);
    this->setCompletionMode(completionMode);
}

void SceneDocumentBinder::setAutoCompleteHintsFor(SceneElement::Type val)
{
    if (m_autoCompleteHintsFor == val)
        return;

    m_autoCompleteHintsFor = val;
    emit autoCompleteHintsForChanged();
}

void SceneDocumentBinder::setAutoCompleteHints(const QStringList &hints,
                                               const QStringList &priorityHints)
{
    if (m_autoCompleteHints == hints && m_priorityAutoCompleteHints == priorityHints)
        return;

    m_autoCompleteHints = hints;
    m_priorityAutoCompleteHints = hints.isEmpty() ? QStringList() : priorityHints;
    emit autoCompleteHintsChanged();

    if (m_autoCompleteHints.isEmpty())
        this->setCompletionPrefix(QString());
}

void SceneDocumentBinder::setCompletionPrefix(const QString &prefix, int start, int end)
{
    if (m_completionPrefix == prefix)
        return;

    m_completionPrefix = prefix;
    m_completionPrefixStart = start;
    m_completionPrefixEnd = end;
    emit completionPrefixChanged();
}

void SceneDocumentBinder::setSpellingSuggestions(const QStringList &val)
{
    if (m_spellingSuggestions == val)
        return;

    m_spellingSuggestions = val;
    emit spellingSuggestionsChanged();
}

void SceneDocumentBinder::setWordUnderCursorIsMisspelled(bool val)
{
    if (m_wordUnderCursorIsMisspelled == val)
        return;

    m_wordUnderCursorIsMisspelled = val;
    emit wordUnderCursorIsMisspelledChanged();
}

void SceneDocumentBinder::onSceneAboutToReset()
{
    m_sceneIsBeingReset = true;
}

void SceneDocumentBinder::onSceneReset(int position)
{
    this->initializeDocument();

    if (position >= 0) {
        QTextCursor cursor(this->document());
        cursor.movePosition(QTextCursor::End);
        position = qBound(0, position, cursor.position());
        QTimer::singleShot(100, this, [=]() { emit requestCursorPosition(position); });

        m_sceneIsBeingReset = false;
    }
}

void SceneDocumentBinder::onSceneRefreshed()
{
    if (m_sceneIsBeingRefreshed)
        return;

    QScopedValueRollback<bool> rollback1(m_sceneIsBeingRefreshed, true);
    QScopedValueRollback<bool> rollback2(m_sceneIsBeingReset, true);

    const int cp = m_cursorPosition;
    this->setCursorPosition(-1);
    this->initializeDocument();
    if (cp >= 0)
        emit requestCursorPosition(cp);
}

void SceneDocumentBinder::rehighlightLater()
{
    if (!m_applyFormattingEvenInTransaction) {
        if (!m_screenplayFormat.isNull() && m_screenplayFormat->isInTransaction())
            return;
    }

    m_rehighlightTimer.start(0, this);
}

void SceneDocumentBinder::rehighlightBlockLater(const QTextBlock &block)
{
    m_rehighlightBlockQueue.removeOne(block);
    m_rehighlightBlockQueue << block;
    this->rehighlightLater();
}

void SceneDocumentBinder::applyBlockFormatLater(const QTextBlock &block)
{
    m_applyBlockFormatQueue.removeOne(block);
    m_applyBlockFormatQueue << block;
    m_applyBlockFormatTimer.start(0, this);
}

void SceneDocumentBinder::onTextFormatChanged(const QList<int> &properties)
{
    if (!m_acceptTextFormatChanges || !m_applyTextFormat
        || m_textFormat->isUpdatingFromCharFormat())
        return;

    const QTextCharFormat updatedFormat = m_textFormat->toCharFormat(properties);

    /**
     * I wish this function was simpler. I wish we could simply apply the curent
     * char format from m_textFormat to the selected text. But there are real-usage
     * complexities to deal with.
     *
     * For instance, its possible that the user selects a text fragment that already
     * has a few formatted sub-fragments. While applying a new format over this, it
     * should append the new format on top of the existing ones. Ofcourse we can use
     * mergeCharFormat() for this purpose, only when we are appending new format
     * properties over existing ones. But we cannot use that for removing already set
     * properties. For this reason, we will have to figure out all Fragments across
     * various blocks of text and then rework existing formats in those blocks
     * explicitly to match the new text format set by the user in the toolbar shown
     * within ScreenplayEditor.
     */

    struct Fragment
    {
        int start = -1;
        int end = -1;
        QTextBlock block;
    };
    QVector<Fragment> fragments;

    QTextCursor cursor(this->document());
    if (m_selectionStartPosition >= 0 && m_selectionEndPosition > 0
        && m_selectionStartPosition != m_selectionEndPosition) {
        cursor.setPosition(m_selectionStartPosition);
        while (1) {
            Fragment fragment;
            fragment.block = cursor.block();
            fragment.start = qMax(m_selectionStartPosition, fragment.block.position());
            fragment.end = qMin(m_selectionEndPosition, [](const QTextBlock &block) {
                QTextCursor c(block);
                c.movePosition(QTextCursor::EndOfBlock);
                return c.position();
            }(fragment.block));
            fragments.append(fragment);
            if (!cursor.movePosition(QTextCursor::NextBlock))
                break;
            if (cursor.atEnd() || cursor.position() > m_selectionEndPosition)
                break;
        }
        cursor.setPosition(m_selectionStartPosition);
        cursor.setPosition(m_selectionEndPosition, QTextCursor::KeepAnchor);
    } else if (m_cursorPosition >= 0) {
        cursor.setPosition(m_cursorPosition);
        cursor.select(QTextCursor::WordUnderCursor);

        if (!cursor.hasSelection()) {
            QTextCharFormat format = cursor.charFormat();
            for (int prop : properties)
                format.clearProperty(prop);
            format.merge(updatedFormat);
            m_nextCharFormat = format;
            m_applyNextCharFormat = true;
            return;
        }

        Fragment fragment;
        fragment.end = cursor.selectionEnd();
        fragment.start = cursor.selectionStart();
        fragment.block = cursor.block();
        fragments.append(fragment);
    } else
        return;

    for (const Fragment &fragment : fragments) {
        const QVector<QTextLayout::FormatRange> textFormats = fragment.block.textFormats();
        for (const QTextLayout::FormatRange &textFormat : textFormats) {
            int start = fragment.block.position() + textFormat.start;
            int end = fragment.block.position() + textFormat.start + textFormat.length;

            if ((start >= fragment.start && start <= fragment.end)
                || (end >= fragment.start && end <= fragment.end)
                || (start < fragment.start && end > fragment.end)) {

                start = qMax(start, fragment.start);
                end = qMin(end, fragment.end);

                cursor.setPosition(start);
                cursor.setPosition(end, QTextCursor::KeepAnchor);

                QTextCharFormat format = textFormat.format;
                for (int prop : properties)
                    format.clearProperty(prop);
                format.merge(updatedFormat);
                cursor.setCharFormat(format);
            }
        }
    }
}

void SceneDocumentBinder::polishAllSceneElements()
{
    QTextBlock block = this->document()->firstBlock();
    while (block.isValid()) {
        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
        if (userData)
            userData->polishTextLater();
        block = block.next();
    }
}

void SceneDocumentBinder::polishSceneElement(SceneElement *element)
{
    const int blockNr = m_scene->indexOfElement(element);
    if (blockNr < 0)
        return;

    QTextBlock block = this->document()->findBlockByNumber(blockNr);
    SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
    if (userData && userData->sceneElement() == element) {
        userData->polishTextLater();
        return;
    }

    block = this->document()->firstBlock();
    while (block.isValid()) {
        userData = SceneDocumentBlockUserData::get(block);
        if (userData && userData->sceneElement() == element) {
            userData->polishTextLater();
            return;
        }
        block = block.next();
    }
}

void SceneDocumentBinder::performAllSceneElementTasks()
{
    QTextBlock block = this->document()->firstBlock();
    while (block.isValid()) {
        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
        if (userData)
            userData->performPendingTasks();
        block = block.next();
    }
}
