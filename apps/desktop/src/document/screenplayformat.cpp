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

#include "utils.h"
#include "fountain.h"
#include "screenplayformat.h"
#include "scenedocumentbinder_p.h"
#include "application.h"
#include "languageengine.h"
#include "scritedocument.h"
#include "garbagecollector.h"
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
#include <QJsonObject>
#include <QStyleHints>
#include <QFontDatabase>
#include <QJsonDocument>
#include <QTextBlockUserData>
#include <QTextBoundaryFinder>
#include <QScopedValueRollback>
#include <QTextDocumentFragment>
#include <QAbstractTextDocumentLayout>

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
    : QObject(parent), m_elementType(type), m_format(parent)
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

QFont SceneElementFormat::font() const
{
    Language language = LanguageEngine::instance()->supportedLanguages()->findLanguage(
            m_defaultLanguageCode < 0 ? m_format->defaultLanguageCode() : m_defaultLanguageCode);

    QFont font = language.font();
    if (m_fontBold != Auto)
        font.setBold(m_fontBold == Set);
    if (m_fontItalics != Auto)
        font.setItalic(m_fontItalics == Set);
    if (m_fontUnderline != Auto)
        font.setUnderline(m_fontUnderline == Set);
    if (m_fontPointSize > 0)
        font.setPointSize(m_fontPointSize);
    font.setCapitalization(m_fontCapitalization);

    return font;
}

QFont SceneElementFormat::font2() const
{
    QFont font = this->font();
    font.setPointSize(font.pointSize() + m_format->fontPointSizeDelta());
    return font;
}

void SceneElementFormat::setFontBold(SceneElementFormat::Tristate val)
{
    if (m_fontBold == val)
        return;

    m_fontBold = val;
    emit fontBoldChanged();
    emit fontChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setFontItalics(SceneElementFormat::Tristate val)
{
    if (m_fontItalics == val)
        return;

    m_fontItalics = val;
    emit fontItalicsChanged();
    emit fontChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setFontUnderline(SceneElementFormat::Tristate val)
{
    if (m_fontUnderline == val)
        return;

    m_fontUnderline = val;
    emit fontUnderlineChanged();
    emit fontChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setFontPointSize(int val)
{
    if (m_fontPointSize == val)
        return;

    m_fontPointSize = val;
    emit fontPointSizeChanged();
    emit fontChanged();
    emit elementFormatChanged();
}

void SceneElementFormat::setFontCapitalization(QFont::Capitalization val)
{
    if (m_fontCapitalization == val)
        return;

    m_fontCapitalization = val;
    emit fontCapitalizationChanged();
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

void SceneElementFormat::setDefaultLanguageCode(int val)
{
    if (m_defaultLanguageCode == val)
        return;

    m_defaultLanguageCode = val;
    emit defaultLanguageCodeChanged();
    emit fontChanged();
}

void SceneElementFormat::activateDefaultLanguage()
{
    Language language = LanguageEngine::instance()->supportedLanguages()->findLanguage(
            m_defaultLanguageCode < 0 ? m_format->activeLanguageCode() : m_defaultLanguageCode);
    if (language.activate()) {
        LanguageEngine::instance()->supportedLanguages()->setActiveLanguageCode(language.code);
    }
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
    const qreal topMargin = fm.lineSpacing() * m_lineSpacingBefore * m_lineHeight;

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
    format.setFontFamilies({ font.family() });
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
        for (int i = FontSize; i <= TextIndent; i++)
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
    this->setFontBold(Auto);
    this->setFontItalics(Auto);
    this->setFontUnderline(Auto);
    this->setFontCapitalization(QFont::MixedCase);
    this->setFontPointSize(-1);
    this->setLineHeight(0.85);
    this->setLeftMargin(0);
    this->setRightMargin(0);
    this->setLineSpacingBefore(0);
    this->setTextIndent(0);
    this->setTextColor(Qt::black);
    this->setBackgroundColor(Qt::transparent);
    this->setTextAlignment(Qt::AlignLeft);
    this->setDefaultLanguageCode(-1);
}

void SceneElementFormat::serializeToJson(QJsonObject &json) const
{
    Language language =
            LanguageEngine::instance()->supportedLanguages()->findLanguage(m_defaultLanguageCode);

    json.insert(
            "defaultLanguage",
            QJsonObject({ { "code", language.isValid() ? language.code : -1 },
                          { "name",
                            language.isValid() ? language.name() : QStringLiteral("Default") } }));

    const QMetaEnum elementTypeMeta = QMetaEnum::fromType<SceneElement::Type>();
    json.insert("#kind", QString::fromLatin1(elementTypeMeta.valueToKey(m_elementType)));
}

void SceneElementFormat::deserializeFromJson(const QJsonObject &json)
{
    const QJsonObject language = json.value("defaultLanguage").toObject();
    if (language.isEmpty())
        this->setDefaultLanguageCode(-1);
    else
        this->setDefaultLanguageCode(language.value("code").toInt());
}

///////////////////////////////////////////////////////////////////////////////

Q_DECL_IMPORT int qt_defaultDpi();

ScreenplayPageLayout::ScreenplayPageLayout(ScreenplayFormat *parent)
    : QObject(parent), m_format(parent)
{
    m_resolution = qt_defaultDpi();

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
    this->setDefaultResolution(qt_defaultDpi());
    this->loadCustomResolutionFromSettings();
    this->setResolution(qFuzzyIsNull(m_customResolution) ? m_defaultResolution
                                                         : m_customResolution);

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
      m_scriteDocument(qobject_cast<ScriteDocument *>(parent)),
      m_screen(this, "screen")
{
    for (int i = SceneElement::Min; i <= SceneElement::Max; i++) {
        SceneElementFormat *elementFormat = new SceneElementFormat(SceneElement::Type(i), this);
        connect(elementFormat, &SceneElementFormat::elementFormatChanged, this,
                &ScreenplayFormat::formatChanged);
        m_elementFormats.append(elementFormat);
    }

    connect(this, &ScreenplayFormat::formatChanged, [this]() { this->markAsModified(); });

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

void ScreenplayFormat::setDefaultLanguageCode(int val)
{
    if (m_defaultLanguageCode == val)
        return;

    m_defaultLanguageCode = val;
    emit defaultLanguageCodeChanged();
}

void ScreenplayFormat::setActiveLanguageCode(int val)
{
    if (m_activeLanguageCode == val)
        return;

    m_activeLanguageCode = val;
    emit activeLanguageCodeChanged();
}

QFont ScreenplayFormat::defaultFont() const
{
    Language language = LanguageEngine::instance()->supportedLanguages()->findLanguage(
            m_defaultLanguageCode < 0
                    ? LanguageEngine::instance()->supportedLanguages()->defaultLanguageCode()
                    : m_defaultLanguageCode);
    return language.font();
}

QFont ScreenplayFormat::defaultFont2() const
{
    QFont font = this->defaultFont();
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

    for (SceneElementFormat *format : std::as_const(m_elementFormats)) {
        if (from == format)
            continue;

        switch (properties) {
        case SceneElementFormat::FontSize:
            format->setFontPointSize(from->font().pointSize());
            break;
        case SceneElementFormat::FontStyle:
            format->setFontBold(from->fontBold());
            format->setFontItalics(from->fontItalics());
            format->setFontUnderline(from->fontItalics());
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

    if (m_screen != nullptr) {
        const int index = m_fontZoomLevels.indexOf(QVariant(1.0));
        this->setFontZoomLevelIndex(index);
    }

    this->setDefaultLanguageCode(
            LanguageEngine::instance()->supportedLanguages()->defaultLanguageCode());

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

    const QString formatFile = Utils::Platform::configPath(QStringLiteral("formatting.json"));

    QFile file(formatFile);
    if (!file.open(QFile::WriteOnly))
        return false;

    file.write(jsonStr);
    return true;
}

void ScreenplayFormat::resetToUserDefaults()
{
    const QString formatFile = Utils::Platform::configPath(QStringLiteral("formatting.json"));

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

void ScreenplayFormat::serializeToJson(QJsonObject &json) const
{
    Language language =
            LanguageEngine::instance()->supportedLanguages()->findLanguage(m_defaultLanguageCode);

    json.insert(
            "defaultLanguage",
            QJsonObject({ { "code", language.isValid() ? language.code : -1 },
                          { "name",
                            language.isValid() ? language.name() : QStringLiteral("Default") } }));
}

void ScreenplayFormat::deserializeFromJson(const QJsonObject &json)
{
    const QJsonObject language = json.value("defaultLanguage").toObject();
    if (language.isEmpty())
        this->setDefaultLanguageCode(-1);
    else
        this->setDefaultLanguageCode(language.value("code").toInt());
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

    const QFont defaultFont = this->defaultFont();
    const int val = fontPointSize - defaultFont.pointSize();
    if (m_fontPointSizeDelta == val)
        return;

    m_fontPointSizeDelta = val;
    emit fontPointSizeDeltaChanged();
    emit formatChanged();
}

void ScreenplayFormat::evaluateFontZoomLevels()
{
    const QFont defaultFont = this->defaultFont();
    const QList<int> defaultFontPointSizes = QFontDatabase::pointSizes(defaultFont.family());

    QFont font2 = defaultFont;
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
ScreenplayFormat::staticElementFormatAt(QQmlListProperty<SceneElementFormat> *list, qsizetype index)
{
    index = index % (SceneElement::Max + 1);
    return reinterpret_cast<ScreenplayFormat *>(list->data)->m_elementFormats.at(index);
}

qsizetype ScreenplayFormat::staticElementFormatCount(QQmlListProperty<SceneElementFormat> *list)
{
    return reinterpret_cast<ScreenplayFormat *>(list->data)->m_elementFormats.size();
}
