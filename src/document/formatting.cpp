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

#include "formatting.h"
#include "application.h"
#include "timeprofiler.h"
#include "scritedocument.h"
#include "qobjectserializer.h"
#include "qobjectserializer.h"

#include <QPointer>
#include <QMarginsF>
#include <QSettings>
#include <QMetaEnum>
#include <QPdfWriter>
#include <QTextCursor>
#include <QPageLayout>
#include <QFontDatabase>
#include <QTextBlockUserData>
#include <QClipboard>
#include <QMimeData>
#include <QJsonDocument>
#include <QTextBoundaryFinder>

struct ParagraphMetrics
{
    // The following metrics are picked up from FinalDraft 12
    const qreal leftMargin = 1.5; // inches
    const qreal rightMargin = 7.69; // inches
    const qreal topMargin = 0.70; // inches
    const qreal bottomMargin = 0.70; // inches
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

void SceneElementFormat::setLineHeight(qreal val)
{
    if (qFuzzyCompare(m_lineHeight, val))
        return;

    m_lineHeight = val;
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
}

void SceneElementFormat::activateDefaultLanguage()
{
    if (m_defaultLanguage == Default) {
        TransliterationEngine::instance()->setLanguage(m_format->defaultLanguage());
        return;
    }

    TransliterationEngine::Language language =
            TransliterationEngine::Language(int(m_defaultLanguage) - 1);
    TransliterationEngine::instance()->setLanguage(language);
}

QTextBlockFormat SceneElementFormat::createBlockFormat(const qreal *givenContentWidth) const
{
    if (m_lastCreatedBlockFormatPageWidth > 0 && givenContentWidth
        && *givenContentWidth == m_lastCreatedBlockFormatPageWidth)
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
    format.setAlignment(m_textAlignment);

    if (!qFuzzyIsNull(m_backgroundColor.alphaF()))
        format.setBackground(QBrush(m_backgroundColor));

    if (givenContentWidth) {
        m_lastCreatedBlockFormatPageWidth = *givenContentWidth;
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
    format.setFontItalic(font.italic());
    format.setFontWeight(font.weight());
    // format.setFontKerning(font.kerning());
    format.setFontStretch(font.stretch());
    format.setFontOverline(font.overline());
    format.setFontPointSize(font.pointSize());
    format.setFontStrikeOut(font.strikeOut());
    // format.setFontStyleHint(font.styleHint());
    // format.setFontStyleName(font.styleName());
    format.setFontUnderline(font.underline());
    // format.setFontFixedPitch(font.fixedPitch());
    format.setFontWordSpacing(font.wordSpacing());
    format.setFontLetterSpacing(font.letterSpacing());
    // format.setFontStyleStrategy(font.styleStrategy());
    format.setFontCapitalization(font.capitalization());
    // format.setFontHintingPreference(font.hintingPreference());
    format.setFontLetterSpacingType(font.letterSpacingType());

    format.setForeground(QBrush(m_textColor));

    if (givenPageWidth) {
        m_lastCreatedCharFormatPageWidth = *givenPageWidth;
        m_lastCreatedCharFormat = format;
    }

    return format;
}

void SceneElementFormat::applyToAll(SceneElementFormat::Properties properties)
{
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

void SceneElementFormat::resetToDefaults()
{
    this->setFont(m_format->defaultFont());
    this->setLineHeight(0.85);
    this->setLeftMargin(0);
    this->setRightMargin(0);
    this->setLineSpacingBefore(0);
    this->setTextColor(Qt::black);
    this->setBackgroundColor(Qt::transparent);
    this->setTextAlignment(Qt::AlignLeft);
    this->setDefaultLanguage(Default);

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
    m_evaluateRectsTimer.start(100, this);
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

    this->resetToDefaults();
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
    this->setScreen(Application::instance()->windowScreen(windowObject));
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
        case SceneElementFormat::TextAndBackgroundColors:
            format->setTextColor(from->textColor());
            format->setBackgroundColor(from->backgroundColor());
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

void ScreenplayFormat::resetToDefaults()
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
        m_elementFormats.at(i)->resetToDefaults();

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
    QFont font2 = m_defaultFont;
    font2.setPointSize(int(font2.pointSize() * this->screenDevicePixelRatio()));

    QFontInfo defaultFontInfo(font2);
    font2.setPointSize(defaultFontInfo.pointSize());

    QFontMetricsF fm2(font2);
    const qreal zoomOneACW = fm2.averageCharWidth();
    const int maxPointSize = int(2.0 * qreal(defaultFontInfo.pointSize()));
    const int minPointSize = 8;

    QVariantList zoomLevels = QVariantList() << QVariant(1.0);
    QList<int> selectedPointSizes = QList<int>() << defaultFontInfo.pointSize();
    const QList<int> stdSizes = QFontDatabase().pointSizes(m_defaultFont.family());

    const int start = stdSizes.indexOf(defaultFontInfo.pointSize());
    for (int i = start + 1; i < stdSizes.size(); i++) {
        const int fontSize = stdSizes.at(i);
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
        const int fontSize = stdSizes.at(i);
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

TextFormat::TextFormat(QObject *parent) : QObject(parent)
{
    connect(this, &TextFormat::boldChanged, this, &TextFormat::formatChanged);
    connect(this, &TextFormat::italicChanged, this, &TextFormat::formatChanged);
    connect(this, &TextFormat::underlineChanged, this, &TextFormat::formatChanged);
    connect(this, &TextFormat::textColorChanged, this, &TextFormat::formatChanged);
    connect(this, &TextFormat::backgroundColorChanged, this, &TextFormat::formatChanged);
}

TextFormat::~TextFormat() { }

void TextFormat::setBold(bool val)
{
    if (m_bold == val)
        return;

    m_bold = val;
    emit boldChanged();
}

void TextFormat::setItalic(bool val)
{
    if (m_italic == val)
        return;

    m_italic = val;
    emit italicChanged();
}

void TextFormat::setUnderline(bool val)
{
    if (m_underline == val)
        return;

    m_underline = val;
    emit underlineChanged();
}

void TextFormat::setTextColor(const QColor &val)
{
    if (m_textColor == val)
        return;

    m_textColor = val;
    emit textColorChanged();
}

void TextFormat::setBackgroundColor(const QColor &val)
{
    if (m_backgroundColor == val)
        return;

    m_backgroundColor = val;
    emit backgroundColorChanged();
}

void TextFormat::reset()
{
    this->resetTextColor();
    this->resetBackgroundColor();
    this->setBold(false);
    this->setItalic(false);
    this->setUnderline(false);
}

void TextFormat::updateFromFormat(const QTextCharFormat &format)
{
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
        this->setItalic(format.fontItalic());
    else
        this->setItalic(false);

    if (format.hasProperty(QTextFormat::FontUnderline))
        this->setUnderline(format.fontUnderline());
    else
        this->setUnderline(false);

    emit formatChanged();
}

QTextCharFormat TextFormat::toFormat() const
{
    QTextCharFormat format;

    if (m_textColor.alpha() > 0)
        format.setForeground(m_textColor);

    if (m_backgroundColor.alpha() > 0)
        format.setBackground(m_backgroundColor);

    if (m_bold)
        format.setFontWeight(QFont::Bold);

    if (m_italic)
        format.setFontItalic(true);

    if (m_underline)
        format.setFontUnderline(true);

    return format;
}

///////////////////////////////////////////////////////////////////////////////

class SceneDocumentBlockUserData : public QTextBlockUserData
{
public:
    SceneDocumentBlockUserData(SceneElement *element, SceneDocumentBinder *binder);
    ~SceneDocumentBlockUserData();

    QTextBlockFormat blockFormat;
    QTextCharFormat charFormat;

    SceneElement *sceneElement() const { return m_sceneElement; }

    void resetFormat() { m_formatMTime = -1; }
    bool shouldUpdateFromFormat(const SceneElementFormat *format)
    {
        return format->isModified(&m_formatMTime);
    }

    void initializeSpellCheck(SceneDocumentBinder *binder);
    bool shouldUpdateFromSpellCheck()
    {
        return !m_spellCheck.isNull() && m_spellCheck->isModified(&m_spellCheckMTime);
    }
    void scheduleSpellCheckUpdate()
    {
        if (!m_spellCheck.isNull())
            m_spellCheck->scheduleUpdate();
    }
    QList<TextFragment> misspelledFragments() const
    {
        if (!m_spellCheck.isNull())
            return m_spellCheck->misspelledFragments();
        return QList<TextFragment>();
    }

    TextFragment findMisspelledFragment(int start, int end) const
    {
        const QList<TextFragment> fragments = this->misspelledFragments();
        for (const TextFragment &fragment : fragments) {
            if ((fragment.start() >= start && fragment.start() < end)
                || (fragment.end() > start && fragment.end() <= end))
                return fragment;
        }
        return TextFragment();
    }

    static SceneDocumentBlockUserData *get(const QTextBlock &block);
    static SceneDocumentBlockUserData *get(QTextBlockUserData *userData);

private:
    QPointer<SpellCheckService> m_spellCheck;
    QPointer<SceneElement> m_sceneElement;
    QString m_highlightedText;
    int m_formatMTime = -1;
    int m_spellCheckMTime = -1;
    QMetaObject::Connection m_spellCheckConnection;
};

SceneDocumentBlockUserData::SceneDocumentBlockUserData(SceneElement *element,
                                                       SceneDocumentBinder *binder)
    : m_sceneElement(element)
{
    if (binder->isSpellCheckEnabled()) {
        m_spellCheck = element->spellCheck();
        m_spellCheckConnection =
                QObject::connect(m_spellCheck, SIGNAL(misspelledFragmentsChanged()), binder,
                                 SLOT(onSpellCheckUpdated()), Qt::UniqueConnection);
        m_spellCheck->scheduleUpdate();
    }
}

SceneDocumentBlockUserData::~SceneDocumentBlockUserData()
{
    if (m_spellCheckConnection)
        QObject::disconnect(m_spellCheckConnection);
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

SceneDocumentBlockUserData *SceneDocumentBlockUserData::get(const QTextBlock &block)
{
    return get(block.userData());
}

SceneDocumentBlockUserData *SceneDocumentBlockUserData::get(QTextBlockUserData *userData)
{
    if (userData == nullptr)
        return nullptr;

    SceneDocumentBlockUserData *userData2 =
            reinterpret_cast<SceneDocumentBlockUserData *>(userData);
    return userData2;
}

class SpellCheckCursor : public QTextCursor
{
public:
    SpellCheckCursor(QTextDocument *document, int position) : QTextCursor(document)
    {
        this->setPosition(position);
        this->select(QTextCursor::WordUnderCursor);

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
        this->insertText(word);
    }

    void resetCharFormat() { this->replace(this->word()); }

private:
    SceneDocumentBlockUserData *m_blockData = nullptr;
    TextFragment m_misspelledFragment;
};

SceneDocumentBinder::SceneDocumentBinder(QObject *parent)
    : QSyntaxHighlighter(parent),
      m_scene(this, "scene"),
      m_rehighlightTimer("SceneDocumentBinder.m_rehighlightTimer"),
      m_initializeDocumentTimer("SceneDocumentBinder.m_initializeDocumentTimer"),
      m_currentElement(this, "currentElement"),
      m_textDocument(this, "textDocument"),
      m_screenplayFormat(this, "screenplayFormat")
{
    connect(this, &SceneDocumentBinder::currentElementChanged, this,
            &SceneDocumentBinder::nextTabFormatChanged);
}

SceneDocumentBinder::~SceneDocumentBinder() { }

void SceneDocumentBinder::setScreenplayFormat(ScreenplayFormat *val)
{
    if (m_screenplayFormat == val)
        return;

    if (m_screenplayFormat != nullptr) {
        disconnect(m_screenplayFormat, &ScreenplayFormat::formatChanged, this,
                   &SceneDocumentBinder::rehighlightLater);
    }

    m_screenplayFormat = val;
    if (m_screenplayFormat != nullptr) {
        connect(m_screenplayFormat, &ScreenplayFormat::formatChanged, this,
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
    }

    m_scene = val;

    if (m_scene != nullptr) {
        connect(m_scene, &Scene::sceneElementChanged, this,
                &SceneDocumentBinder::onSceneElementChanged);
        connect(m_scene, &Scene::sceneAboutToReset, this,
                &SceneDocumentBinder::onSceneAboutToReset);
        connect(m_scene, &Scene::sceneReset, this, &SceneDocumentBinder::onSceneReset);
    }

    emit sceneChanged();

    this->initializeDocumentLater();
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

    this->evaluateAutoCompleteHints();

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
    if (m_initializingDocument)
        return;

    if (m_cursorPosition == val)
        return;

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
        m_textFormat->reset();
        emit cursorPositionChanged();
        return;
    }

    if (this->document()->isEmpty() || m_cursorPosition > this->document()->characterCount()) {
        m_textFormat->reset();
        emit cursorPositionChanged();
        return;
    }

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
        if (!m_autoCompleteHints.isEmpty())
            this->setCompletionPrefix(block.text());

        this->setWordUnderCursorIsMisspelled(cursor.isMisspelled());
        this->setSpellingSuggestions(cursor.suggestions());

        const QTextCharFormat format = cursor.charFormat();
        m_textFormat->updateFromFormat(format);
    }

    m_currentElementCursorPosition = m_cursorPosition - block.position();
    emit cursorPositionChanged();
}

void SceneDocumentBinder::setCharacterNames(const QStringList &val)
{
    if (m_characterNames == val)
        return;

    m_characterNames = val;
    emit characterNamesChanged();
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
#ifdef Q_OS_MAC
    return current + QStringLiteral(" → ") + next;
#else
    return current + QStringLiteral(" -> ") + next;
#endif
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

    QJsonArray content;

    QTextCursor cursor(this->document());
    cursor.setPosition(fromPosition);

    QStringList lines;

    QTextBlock block = cursor.block();
    while (block.isValid() && toPosition > block.position()) {
        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
        if (userData == nullptr) {
            block = block.next();
            continue;
        }

        const int bstart = block.position();
        const int bend = block.position() + block.length() - 1;
        cursor.setPosition(qMax(fromPosition, bstart));
        cursor.setPosition(qMin(toPosition, bend), QTextCursor::KeepAnchor);

        SceneElement *element = userData->sceneElement();

        QJsonObject para;
        para.insert(QStringLiteral("type"), element->type());
        para.insert(QStringLiteral("text"), cursor.selectedText());
        content.append(para);

        lines += cursor.selectedText();

        block = block.next();
    }

    const QByteArray contentJson = QJsonDocument(content).toJson();

    QClipboard *clipboard = Application::instance()->clipboard();
    QMimeData *mimeData = new QMimeData;
    mimeData->setData(QStringLiteral("scrite/screenplay"), contentJson);
    mimeData->setText(lines.join("\n"));
    clipboard->setMimeData(mimeData);
}

int SceneDocumentBinder::paste(int fromPosition)
{
    if (this->document() == nullptr)
        return -1;

    const QClipboard *clipboard = Application::instance()->clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    const QByteArray contentJson = mimeData->data(QStringLiteral("scrite/screenplay"));
    if (contentJson.isEmpty())
        return -1;

    const QJsonArray content = QJsonDocument::fromJson(contentJson).array();
    if (content.isEmpty())
        return -1;

    fromPosition = fromPosition >= 0 ? fromPosition : m_cursorPosition;

    QTextCursor cursor(this->document());
    cursor.setPosition(fromPosition >= 0 ? fromPosition : m_cursorPosition);

    const bool pasteFormatting = content.size() > 1;

    for (int i = 0; i < content.size(); i++) {
        const QJsonObject item = content.at(i).toObject();
        const int type = item.value(QStringLiteral("type")).toInt();
        if (type < SceneElement::Min || type > SceneElement::Max || type == SceneElement::Heading)
            continue;

        if (i > 0)
            cursor.insertBlock();

        const QString text = item.value(QStringLiteral("text")).toString();
        cursor.insertText(text);

        if (pasteFormatting) {
            SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(cursor.block());
            if (userData) {
                userData->sceneElement()->setType(SceneElement::Type(type));
                userData->resetFormat();
            }
        }
    }

    const int ret = cursor.position();
    this->refresh();

    return ret;
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
    if (m_initializingDocument)
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
        userData->blockFormat = format->createBlockFormat();
        userData->charFormat = format->createCharFormat();

        QTextCursor cursor(block);
        cursor.setBlockFormat(userData->blockFormat);
    }

    this->setFormat(0, block.length(), userData->charFormat);

    // Per-language fonts.
    if (m_applyLanguageFonts) {
        const QList<TransliterationEngine::Boundary> boundaries =
                TransliterationEngine::instance()->evaluateBoundaries(text);

        for (const TransliterationEngine::Boundary &boundary : boundaries) {
            if (boundary.isEmpty() || boundary.language == TransliterationEngine::English)
                continue;

            QTextCharFormat format = this->format(boundary.start);
            format.setFontFamily(boundary.font.family());
            this->setFormat(boundary.start, boundary.end - boundary.start + 1, format);
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

            QTextCharFormat spellingErrorFormat = this->format(fragment.start());
#if 0
            spellingErrorFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
            spellingErrorFormat.setUnderlineColor(Qt::red);
            spellingErrorFormat.setFontUnderline(true);
#else
            spellingErrorFormat.setBackground(QColor(255, 0, 0, 32));
#endif
            this->setFormat(fragment.start(), fragment.length(), spellingErrorFormat);
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
        if (m_rehighlightBlockQueue.size() > nrTresholdBlocks || m_rehighlightBlockQueue.isEmpty())
            this->QSyntaxHighlighter::rehighlight();
        else {
            for (const QTextBlock &block : qAsConst(m_rehighlightBlockQueue))
                this->rehighlightBlock(block);
        }

        m_rehighlightBlockQueue.clear();
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
    this->evaluateAutoCompleteHints();
    emit textDocumentChanged();
    this->setCursorPosition(-1);
}

void SceneDocumentBinder::resetScreenplayFormat()
{
    m_screenplayFormat = nullptr;
    emit screenplayFormatChanged();
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
    document->blockSignals(true);
    document->clear();
    document->setDefaultFont(defaultFont);
    document->setUseDesignMetrics(true);

    const int nrElements = m_scene->elementCount();

    QTextCursor cursor(document);
    for (int i = 0; i < nrElements; i++) {
        SceneElement *element = m_scene->elementAt(i);
        if (i > 0)
            cursor.insertBlock();

        QTextBlock block = cursor.block();
        if (!block.isValid() && i == 0) {
            cursor.insertBlock();
            block = cursor.block();
        }

        SceneDocumentBlockUserData *userData = new SceneDocumentBlockUserData(element, this);
        block.setUserData(userData);
        cursor.insertText(element->text());
    }
    document->blockSignals(false);

    if (m_cursorPosition <= 0 && m_currentElement == nullptr && nrElements == 1)
        this->setCurrentElement(m_scene->elementAt(0));

    this->setDocumentLoadCount(m_documentLoadCount + 1);
    m_initializingDocument = false;
    this->QSyntaxHighlighter::rehighlight();

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

        SceneElementFormat *format = m_screenplayFormat->elementFormat(m_currentElement->type());
        if (format != nullptr)
            format->activateDefaultLanguage();
    }

    emit currentElementChanged();

    m_tabHistory.clear();
    this->evaluateAutoCompleteHints();

    emit currentFontChanged();
}

void SceneDocumentBinder::resetCurrentElement()
{
    m_currentElement = nullptr;
    emit currentElementChanged();

    m_tabHistory.clear();
    this->evaluateAutoCompleteHints();

    emit currentFontChanged();
}

class ForceCursorPositionHack : public QObject
{
public:
    ForceCursorPositionHack(const QTextBlock &block, int cp, SceneDocumentBinder *binder);
    ~ForceCursorPositionHack();

    void timerEvent(QTimerEvent *event);

private:
    QTextBlock m_block;
    int m_cursorPosition = 0; // within m_block
    ExecLaterTimer m_timer;
    SceneDocumentBinder *m_binder = nullptr;
};

ForceCursorPositionHack::ForceCursorPositionHack(const QTextBlock &block, int cp,
                                                 SceneDocumentBinder *binder)
    : QObject(const_cast<QTextDocument *>(block.document())),
      m_block(block),
      m_cursorPosition(cp),
      m_timer("ForceCursorPositionHack.m_timer"),
      m_binder(binder)
{
    QTextCursor cursor(m_block);
    cursor.insertText(QStringLiteral("("));
    m_timer.start(0, this);
}

ForceCursorPositionHack::~ForceCursorPositionHack() { }

void ForceCursorPositionHack::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timer.timerId()) {
        m_timer.stop();
        QTextCursor cursor(m_block);
        cursor.deleteChar();

        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(m_block);
        if (userData && userData->sceneElement()->type() == SceneElement::Parenthetical) {
            const QString bo = QStringLiteral("(");
            const QString bc = QStringLiteral(")");

            if (m_block.text().isEmpty()) {
                cursor.insertText(QStringLiteral("()"));
                m_cursorPosition = 1;
            } else {
                const QString blockText = m_block.text();
                if (!blockText.startsWith(bo)) {
                    cursor.insertText(bo);
                    m_cursorPosition += 1;
                }
                if (!blockText.endsWith(bc)) {
                    cursor.movePosition(QTextCursor::EndOfBlock);
                    cursor.insertText(bc);
                }
            }
        }

        emit m_binder->requestCursorPosition(m_block.position() + m_cursorPosition);

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

    this->evaluateAutoCompleteHints();

    auto updateBlock = [=](const QTextBlock &block) {
        SceneDocumentBlockUserData *userData = SceneDocumentBlockUserData::get(block);
        if (userData != nullptr && userData->sceneElement() == element) {
            // Text changes from scene element to block are not applied
            // Only element type changes can be applied.
            new ForceCursorPositionHack(block, m_cursorPosition - block.position(), this);
            userData->resetFormat();
            this->rehighlightBlock(block);
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
    if (m_initializingDocument || m_sceneIsBeingReset)
        return;

    if (m_textDocument == nullptr || m_scene == nullptr || this->document() == nullptr)
        return;

    m_tabHistory.clear();

    QTextCursor cursor(this->document());
    cursor.setPosition(from);

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

    /**
      If the number of paragraphs in the document is differnet from the number of
      paragraphs in our internal Scene data structure, then we better sync it once.
      This can happen when user pastes more than 1 paragraphs at once or if the user
      deletes more than 1 paragraphs at once.
      */
    if (sceneElement->scene() == nullptr
        || this->document()->blockCount() != sceneElement->scene()->elementCount()) {
        this->syncSceneFromDocument();
        return;
    }

    sceneElement->setText(block.text());

    if (m_spellCheckEnabled && m_liveSpellCheckEnabled && (charsAdded > 0 || charsRemoved > 0))
        userData->scheduleSpellCheckUpdate();
}

void SceneDocumentBinder::syncSceneFromDocument(int nrBlocks)
{
    if (m_initializingDocument || m_sceneIsBeingReset)
        return;

    if (m_textDocument == nullptr || m_scene == nullptr)
        return;

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
                switch (prevElement->type()) {
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
            } else {
                newElement->setType(SceneElement::Action);
                m_scene->insertElementAt(newElement, 0);
            }

            userData = new SceneDocumentBlockUserData(newElement, this);
            block.setUserData(userData);
        }

        elementList.append(userData->sceneElement());
        userData->sceneElement()->setText(block.text());

        previousBlock = block;
        block = block.next();
    }

    m_scene->setElementsList(elementList);
    m_scene->endUndoCapture();
}

void SceneDocumentBinder::evaluateAutoCompleteHints()
{
    QStringList hints;

    if (m_currentElement == nullptr) {
        this->setAutoCompleteHints(hints);
        return;
    }

    static QStringList transitions = QStringList()
            << QStringLiteral("CUT TO") << QStringLiteral("DISSOLVE TO")
            << QStringLiteral("FADE IN") << QStringLiteral("FADE OUT") << QStringLiteral("FADE TO")
            << QStringLiteral("FLASH CUT TO") << QStringLiteral("FREEZE FRAME")
            << QStringLiteral("IRIS IN") << QStringLiteral("IRIS OUT")
            << QStringLiteral("JUMP CUT TO") << QStringLiteral("MATCH CUT TO")
            << QStringLiteral("MATCH DISSOLVE TO") << QStringLiteral("SMASH CUT TO")
            << QStringLiteral("STOCK SHOT") << QStringLiteral("TIME CUT")
            << QStringLiteral("WIPE TO");

    static QStringList shots = QStringList()
            << QStringLiteral("AIR") << QStringLiteral("CLOSE ON") << QStringLiteral("CLOSER ON")
            << QStringLiteral("CLOSEUP") << QStringLiteral("ESTABLISHING")
            << QStringLiteral("EXTREME CLOSEUP") << QStringLiteral("INSERT")
            << QStringLiteral("POV") << QStringLiteral("SURFACE") << QStringLiteral("THREE SHOT")
            << QStringLiteral("TWO SHOT") << QStringLiteral("UNDERWATER") << QStringLiteral("WIDE")
            << QStringLiteral("WIDE ON") << QStringLiteral("WIDER ANGLE");

    switch (m_currentElement->type()) {
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

    this->setAutoCompleteHintsFor(m_currentElement->type());
    this->setAutoCompleteHints(hints);
}

void SceneDocumentBinder::setAutoCompleteHintsFor(SceneElement::Type val)
{
    if (m_autoCompleteHintsFor == val)
        return;

    m_autoCompleteHintsFor = val;
    emit autoCompleteHintsForChanged();
}

void SceneDocumentBinder::setAutoCompleteHints(const QStringList &val)
{
    if (m_autoCompleteHints == val)
        return;

    m_autoCompleteHints = val;
    emit autoCompleteHintsChanged();
}

void SceneDocumentBinder::setCompletionPrefix(const QString &val)
{
    if (m_completionPrefix == val)
        return;

    m_completionPrefix = val;
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
        emit requestCursorPosition(position);
    }

    m_sceneIsBeingReset = false;
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
    m_rehighlightBlockQueue << block;
    this->rehighlightLater();
}

void SceneDocumentBinder::onTextFormatChanged()
{
    // TODO: here is where we can take changes made to text format
    // and apply it to the text document at the current cursor.
}
