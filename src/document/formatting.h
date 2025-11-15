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

#ifndef FORMATTING_H
#define FORMATTING_H

#include "scene.h"
#include "modifiable.h"
#include "execlatertimer.h"
#include "qobjectproperty.h"

#include <QScreen>
#include <QPageLayout>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QPagedPaintDevice>
#include <QSyntaxHighlighter>
#include <QQuickTextDocument>

class SpellCheckService;
class ScriteDocument;
class ScreenplayFormat;

class SceneElementFormat : public QObject, public Modifiable, public QObjectSerializer::Interface
{
    Q_OBJECT
    QML_ELEMENT
    Q_INTERFACES(QObjectSerializer::Interface)
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    ~SceneElementFormat();

    // clang-format off
    Q_PROPERTY(ScreenplayFormat *format
               READ format
               CONSTANT STORED
               false )
    // clang-format on
    ScreenplayFormat *format() const { return m_format; }

    // clang-format off
    Q_PROPERTY(SceneElement::Type elementType
               READ elementType
               CONSTANT )
    // clang-format on
    SceneElement::Type elementType() const { return m_elementType; }
    Q_SIGNAL void elementTypeChanged();

    enum Tristate { Auto = -1, Unset = 0, Set = 1 };
    Q_ENUM(Tristate)

    // clang-format off
    Q_PROPERTY(QFont font
               READ font
               NOTIFY fontChanged)
    // clang-format on
    QFont font() const;
    Q_SIGNAL void fontChanged();

    // clang-format off
    Q_PROPERTY(QFont font2
               READ font2
               NOTIFY font2Changed)
    // clang-format on
    QFont font2() const;
    Q_SIGNAL void font2Changed();

    // clang-format off
    Q_PROPERTY(Tristate fontBold
               READ fontBold
               WRITE setFontBold
               NOTIFY fontBoldChanged)
    // clang-format on
    void setFontBold(Tristate val);
    Tristate fontBold() const { return m_fontBold; }
    Q_SIGNAL void fontBoldChanged();

    // clang-format off
    Q_PROPERTY(Tristate fontItalics
               READ fontItalics
               WRITE setFontItalics
               NOTIFY fontItalicsChanged)
    // clang-format on
    void setFontItalics(Tristate val);
    Tristate fontItalics() const { return m_fontItalics; }
    Q_SIGNAL void fontItalicsChanged();

    // clang-format off
    Q_PROPERTY(Tristate fontUnderline
               READ fontUnderline
               WRITE setFontUnderline
               NOTIFY fontUnderlineChanged)
    // clang-format on
    void setFontUnderline(Tristate val);
    Tristate fontUnderline() const { return m_fontUnderline; }
    Q_SIGNAL void fontUnderlineChanged();

    // clang-format off
    Q_PROPERTY(int fontPointSize
               READ fontPointSize
               WRITE setFontPointSize
               NOTIFY fontPointSizeChanged)
    // clang-format on
    void setFontPointSize(int val);
    int fontPointSize() const { return m_fontPointSize; }
    Q_SIGNAL void fontPointSizeChanged();

    // clang-format off
    Q_PROPERTY(QFont::Capitalization fontCapitalization
               READ fontCapitalization
               WRITE setFontCapitalization
               NOTIFY fontCapitalizationChanged)
    // clang-format on
    void setFontCapitalization(QFont::Capitalization val);
    QFont::Capitalization fontCapitalization() const { return m_fontCapitalization; }
    Q_SIGNAL void fontCapitalizationChanged();

    // clang-format off
    Q_PROPERTY(QColor textColor
               READ textColor
               WRITE setTextColor
               NOTIFY textColorChanged)
    // clang-format on
    void setTextColor(const QColor &val);
    QColor textColor() const { return m_textColor; }
    Q_SIGNAL void textColorChanged();

    // clang-format off
    Q_PROPERTY(Qt::Alignment textAlignment
               READ textAlignment
               WRITE setTextAlignment
               NOTIFY textAlignmentChanged)
    // clang-format on
    void setTextAlignment(Qt::Alignment val);
    Qt::Alignment textAlignment() const { return m_textAlignment; }
    Q_SIGNAL void textAlignmentChanged();

    // clang-format off
    Q_PROPERTY(QColor backgroundColor
               READ backgroundColor
               WRITE setBackgroundColor
               NOTIFY backgroundColorChanged)
    // clang-format on
    void setBackgroundColor(const QColor &val);
    QColor backgroundColor() const { return m_backgroundColor; }
    Q_SIGNAL void backgroundColorChanged();

    // clang-format off
    Q_PROPERTY(qreal textIndent
               READ textIndent
               WRITE setTextIndent
               NOTIFY textIndentChanged)
    // clang-format on
    void setTextIndent(qreal val);
    qreal textIndent() const { return m_textIndent; }
    Q_SIGNAL void textIndentChanged();

    // clang-format off
    Q_PROPERTY(qreal lineHeight
               READ lineHeight
               WRITE setLineHeight
               NOTIFY lineHeightChanged)
    // clang-format on
    void setLineHeight(qreal val);
    qreal lineHeight() const { return m_lineHeight; }
    Q_SIGNAL void lineHeightChanged();

    // clang-format off
    Q_PROPERTY(qreal lineSpacingBefore
               READ lineSpacingBefore
               WRITE setLineSpacingBefore
               NOTIFY lineSpacingBeforeChanged
               STORED false)
    // clang-format on
    void setLineSpacingBefore(qreal val);
    qreal lineSpacingBefore() const { return m_lineSpacingBefore; }
    Q_SIGNAL void lineSpacingBeforeChanged();

    // clang-format off
    Q_PROPERTY(qreal leftMargin
               READ leftMargin
               WRITE setLeftMargin
               NOTIFY leftMarginChanged
               STORED false)
    // clang-format on
    void setLeftMargin(qreal val);
    qreal leftMargin() const { return m_leftMargin; }
    Q_SIGNAL void leftMarginChanged();

    // clang-format off
    Q_PROPERTY(qreal rightMargin
               READ rightMargin
               WRITE setRightMargin
               NOTIFY rightMarginChanged
               STORED false)
    // clang-format on
    void setRightMargin(qreal val);
    qreal rightMargin() const { return m_rightMargin; }
    Q_SIGNAL void rightMarginChanged();

    // clang-format off
    Q_PROPERTY(int defaultLanguageCode
               READ defaultLanguageCode
               WRITE setDefaultLanguageCode
               NOTIFY defaultLanguageCodeChanged
               STORED false)
    // clang-format on
    void setDefaultLanguageCode(int val);
    int defaultLanguageCode() const { return m_defaultLanguageCode; }
    Q_SIGNAL void defaultLanguageCodeChanged();

    Q_INVOKABLE void activateDefaultLanguage();

    QTextBlockFormat createBlockFormat(Qt::Alignment overrideAlignment,
                                       const qreal *pageWidth = nullptr) const;
    QTextCharFormat createCharFormat(const qreal *pageWidth = nullptr) const;

    Q_SIGNAL void elementFormatChanged();

    enum Properties {
        FontSize,
        FontStyle,
        LineHeight,
        LineSpacingBefore,
        TextAlignment,
        TextAndBackgroundColors,
        TextIndent,
        AllProperties
    };
    Q_ENUM(Properties)
    Q_INVOKABLE void applyToAll(SceneElementFormat::Properties properties);

    Q_INVOKABLE void beginTransaction();
    Q_INVOKABLE bool hasChangesToCommit() { return m_nrChangesDuringTransation > 0; }
    Q_INVOKABLE void commitTransaction();

    // clang-format off
    Q_PROPERTY(bool inTransaction
               READ isInTransaction
               NOTIFY inTransactionChanged)
    // clang-format on
    bool isInTransaction() const { return m_inTransaction; }
    Q_SIGNAL void inTransactionChanged();

    void resetToFactoryDefaults();

    // Interface interface
    void serializeToJson(QJsonObject &) const;
    void deserializeFromJson(const QJsonObject &);

private:
    friend class ScreenplayFormat;
    SceneElementFormat(SceneElement::Type type = SceneElement::Action,
                       ScreenplayFormat *parent = nullptr);
    void countTransactionChange() { ++m_nrChangesDuringTransation; }

private:
    int m_fontPointSize = -1;
    int m_defaultLanguageCode = -1; // means default
    int m_nrChangesDuringTransation = 0;

    bool m_inTransaction = false;

    qreal m_textIndent = 0.0;
    qreal m_lineHeight = 1.0;
    qreal m_leftMargin = 0;
    qreal m_rightMargin = 0;
    qreal m_lineSpacingBefore = 0;

    QColor m_textColor = QColor(Qt::black);
    QColor m_backgroundColor = QColor(Qt::transparent);

    Tristate m_fontBold = Auto;
    Tristate m_fontItalics = Auto;
    Tristate m_fontUnderline = Auto;

    Qt::Alignment m_textAlignment = Qt::AlignLeft;
    SceneElement::Type m_elementType = SceneElement::Action;
    QFont::Capitalization m_fontCapitalization = QFont::MixedCase;

    ScreenplayFormat *m_format = nullptr;

    mutable Qt::Alignment m_lastCreatedBlockAlignment;
    mutable qreal m_lastCreatedBlockFormatPageWidth = 0;
    mutable QTextBlockFormat m_lastCreatedBlockFormat;
    mutable qreal m_lastCreatedCharFormatPageWidth = 0;
    mutable QTextCharFormat m_lastCreatedCharFormat;
};

class ScreenplayPageLayout : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    explicit ScreenplayPageLayout(ScreenplayFormat *parent = nullptr);
    ~ScreenplayPageLayout();

    // clang-format off
    Q_PROPERTY(ScreenplayFormat *format
               READ format
               CONSTANT )
    // clang-format on
    ScreenplayFormat *format() const { return m_format; }

    enum PaperSize { A4, Letter };
    Q_ENUM(PaperSize)
    // clang-format off
    Q_PROPERTY(PaperSize paperSize
               READ paperSize
               WRITE setPaperSize
               NOTIFY paperSizeChanged
               STORED false)
    // clang-format on
    void setPaperSize(PaperSize val);
    PaperSize paperSize() const { return m_paperSize; }
    Q_SIGNAL void paperSizeChanged();

    // clang-format off
    Q_PROPERTY(QMarginsF margins
               READ margins
               NOTIFY rectsChanged
               STORED false)
    // clang-format on
    QMarginsF margins() const { return m_margins; }
    Q_SIGNAL void marginsChanged();

    // clang-format off
    Q_PROPERTY(qreal leftMargin
               READ leftMargin
               NOTIFY rectsChanged
               STORED false)
    // clang-format on
    qreal leftMargin() const { return m_margins.left(); }

    // clang-format off
    Q_PROPERTY(qreal topMargin
               READ topMargin
               NOTIFY rectsChanged
               STORED false)
    // clang-format on
    qreal topMargin() const { return m_margins.top(); }

    // clang-format off
    Q_PROPERTY(qreal rightMargin
               READ rightMargin
               NOTIFY rectsChanged
               STORED false)
    // clang-format on
    qreal rightMargin() const { return m_margins.right(); }

    // clang-format off
    Q_PROPERTY(qreal bottomMargin
               READ bottomMargin
               NOTIFY rectsChanged
               STORED false)
    // clang-format on
    qreal bottomMargin() const { return m_margins.bottom(); }

    // clang-format off
    Q_PROPERTY(QRectF paperRect
               READ paperRect
               NOTIFY rectsChanged
               STORED false)
    // clang-format on
    QRectF paperRect() const { return m_paperRect; }
    Q_SIGNAL void paperRectChanged();

    // clang-format off
    Q_PROPERTY(qreal paperWidth
               READ paperWidth
               NOTIFY rectsChanged
               STORED false)
    // clang-format on
    qreal paperWidth() const { return m_paperRect.width(); }

    // clang-format off
    Q_PROPERTY(qreal pageWidth
               READ pageWidth
               NOTIFY rectsChanged
               STORED false)
    // clang-format on
    qreal pageWidth() const { return m_paperRect.width(); }

    // clang-format off
    Q_PROPERTY(QRectF paintRect
               READ paintRect
               NOTIFY rectsChanged
               STORED false)
    // clang-format on
    QRectF paintRect() const { return m_paintRect; }
    Q_SIGNAL void paintRectChanged();

    // clang-format off
    Q_PROPERTY(QRectF headerRect
               READ headerRect
               NOTIFY rectsChanged
               STORED false)
    // clang-format on
    QRectF headerRect() const { return m_headerRect; }
    Q_SIGNAL void headerRectChanged();

    // clang-format off
    Q_PROPERTY(QRectF footerRect
               READ footerRect
               NOTIFY rectsChanged
               STORED false)
    // clang-format on
    QRectF footerRect() const { return m_footerRect; }
    Q_SIGNAL void footerRectChanged();

    // clang-format off
    Q_PROPERTY(QRectF contentRect
               READ contentRect
               NOTIFY rectsChanged
               STORED false)
    // clang-format on
    QRectF contentRect() const { return m_paintRect; }

    // clang-format off
    Q_PROPERTY(qreal contentWidth
               READ contentWidth
               NOTIFY rectsChanged
               STORED false)
    // clang-format on
    qreal contentWidth() const { return m_paintRect.width(); }

    // clang-format off
    Q_PROPERTY(qreal defaultResolution
               READ defaultResolution
               NOTIFY defaultResolutionChanged)
    // clang-format on
    qreal defaultResolution() const { return m_defaultResolution; }
    Q_SIGNAL void defaultResolutionChanged();

    // clang-format off
    Q_PROPERTY(qreal customResolution
               READ customResolution
               WRITE setCustomResolution
               NOTIFY customResolutionChanged)
    // clang-format on
    void setCustomResolution(qreal val);
    qreal customResolution() const { return m_customResolution; }
    Q_SIGNAL void customResolutionChanged();

    // clang-format off
    Q_PROPERTY(qreal resolution
               READ resolution
               NOTIFY resolutionChanged)
    // clang-format on
    qreal resolution() const { return m_resolution; }
    Q_SIGNAL void resolutionChanged();

    void configure(QTextDocument *document) const;
    void configure(QPagedPaintDevice *printer) const;

    Q_INVOKABLE void evaluateRectsNow();

signals:
    void rectsChanged();

private:
    void evaluateRects();
    void evaluateRectsLater();
    void timerEvent(QTimerEvent *event);

    void setResolution(qreal val);
    void setDefaultResolution(qreal val);
    void loadCustomResolutionFromSettings();

private:
    qreal m_resolution = 0;
    QRectF m_paintRect;
    QRectF m_paperRect;
    QMarginsF m_margins;
    QRectF m_headerRect;
    QRectF m_footerRect;
    PaperSize m_paperSize = Letter;
    QPageLayout m_pageLayout;
    qreal m_customResolution = 0;
    qreal m_defaultResolution = 0;
    ScreenplayFormat *m_format = nullptr;
    ExecLaterTimer m_evaluateRectsTimer;
};

class ScreenplayFormat : public QAbstractListModel,
                         public Modifiable,
                         public QObjectSerializer::Interface
{
    Q_OBJECT
    QML_ELEMENT
    Q_INTERFACES(QObjectSerializer::Interface)
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    explicit ScreenplayFormat(QObject *parent = nullptr);
    ~ScreenplayFormat();

    // clang-format off
    Q_PROPERTY(ScriteDocument *scriteDocument
               READ scriteDocument
               CONSTANT STORED
               false )
    // clang-format on
    ScriteDocument *scriteDocument() const { return m_scriteDocument; }

    // clang-format off
    Q_PROPERTY(QScreen *screen
               READ screen
               WRITE setScreen
               NOTIFY screenChanged
               RESET resetScreen
               STORED false)
    // clang-format on
    Q_INVOKABLE void setScreen(QScreen *val);
    QScreen *screen() const { return m_screen; }
    Q_SIGNAL void screenChanged();

    Q_INVOKABLE void setSreeenFromWindow(QObject *windowObject);

    qreal screenDevicePixelRatio() const { return m_screen ? m_screen->devicePixelRatio() : 1.0; }

    // clang-format off
    Q_PROPERTY(qreal devicePixelRatio
               READ devicePixelRatio
               NOTIFY fontZoomLevelIndexChanged)
    // clang-format on
    qreal devicePixelRatio() const;

    // clang-format off
    Q_PROPERTY(ScreenplayPageLayout *pageLayout
               READ pageLayout
               CONSTANT STORED
               false )
    // clang-format on
    ScreenplayPageLayout *pageLayout() const { return m_pageLayout; }

    // clang-format off
    Q_PROPERTY(int defaultLanguageCode
               READ defaultLanguageCode
               WRITE setDefaultLanguageCode
               NOTIFY defaultLanguageCodeChanged
               STORED false)
    // clang-format on
    void setDefaultLanguageCode(int val);
    int defaultLanguageCode() const { return m_defaultLanguageCode; }
    Q_SIGNAL void defaultLanguageCodeChanged();

    // clang-format off
    Q_PROPERTY(QFont defaultFont
               READ defaultFont
               NOTIFY defaultFontChanged)
    // clang-format on
    QFont defaultFont() const;
    Q_SIGNAL void defaultFontChanged();

    // clang-format off
    Q_PROPERTY(QFont defaultFont2
               READ defaultFont2
               NOTIFY fontPointSizeDeltaChanged)
    // clang-format on
    QFont defaultFont2() const;

    QFontMetrics defaultFontMetrics() const { return QFontMetrics(this->defaultFont()); }
    QFontMetrics defaultFont2Metrics() const { return QFontMetrics(this->defaultFont2()); }

    // clang-format off
    Q_PROPERTY(int fontPointSizeDelta
               READ fontPointSizeDelta
               NOTIFY fontPointSizeDeltaChanged)
    // clang-format on
    int fontPointSizeDelta() const { return m_fontPointSizeDelta; }
    Q_SIGNAL void fontPointSizeDeltaChanged();

    // clang-format off
    Q_PROPERTY(int fontZoomLevelIndex
               READ fontZoomLevelIndex
               WRITE setFontZoomLevelIndex
               NOTIFY fontZoomLevelIndexChanged
               STORED false)
    // clang-format on
    void setFontZoomLevelIndex(int val);
    int fontZoomLevelIndex() const { return m_fontZoomLevelIndex; }
    Q_SIGNAL void fontZoomLevelIndexChanged();

    // clang-format off
    Q_PROPERTY(QVariantList fontZoomLevels
               READ fontZoomLevels
               NOTIFY fontZoomLevelsChanged)
    // clang-format on
    QVariantList fontZoomLevels() const { return m_fontZoomLevels; }
    Q_SIGNAL void fontZoomLevelsChanged();

    Q_INVOKABLE SceneElementFormat *elementFormat(SceneElement::Type type) const;
    Q_INVOKABLE SceneElementFormat *elementFormat(int type) const;
    Q_SIGNAL void formatChanged();

    // clang-format off
    Q_PROPERTY(QQmlListProperty<SceneElementFormat> elementFormats
               READ elementFormats)
    // clang-format on
    QQmlListProperty<SceneElementFormat> elementFormats();

    void applyToAll(const SceneElementFormat *from, SceneElementFormat::Properties properties);

    enum Role { SceneElementFomat = Qt::UserRole };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

    // clang-format off
    Q_PROPERTY(int secondsPerPage
               READ secondsPerPage
               WRITE setSecondsPerPage
               NOTIFY secondsPerPageChanged)
    // clang-format on
    void setSecondsPerPage(int val);
    int secondsPerPage() const { return m_secondsPerPage; }
    Q_SIGNAL void secondsPerPageChanged();

    Q_INVOKABLE void resetToFactoryDefaults();

    Q_INVOKABLE bool saveAsUserDefaults();
    Q_INVOKABLE void resetToUserDefaults();

    Q_INVOKABLE void beginTransaction();
    Q_INVOKABLE bool hasChangesToCommit() { return m_nrChangesDuringTransation > 0; }
    Q_INVOKABLE void commitTransaction();

    // clang-format off
    Q_PROPERTY(bool inTransaction
               READ isInTransaction
               NOTIFY inTransactionChanged)
    // clang-format on
    bool isInTransaction() const { return m_inTransaction; }
    Q_SIGNAL void inTransactionChanged();

    // Interface interface
    void serializeToJson(QJsonObject &) const;
    void deserializeFromJson(const QJsonObject &);

private:
    void resetScreen();
    void evaluateFontPointSizeDelta();
    void evaluateFontZoomLevels();
    void countTransactionChange() { ++m_nrChangesDuringTransation; }

private:
    int m_secondsPerPage = 60;
    int m_fontPointSizeDelta = 0;
    int m_fontZoomLevelIndex = -1;
    int m_defaultLanguageCode = QLocale::English;
    int m_nrChangesDuringTransation = 0;

    bool m_inTransaction = false;

    qreal m_pageWidth = 750.0;

    QList<int> m_fontPointSizes;
    QStringList m_suggestionsAtCursor;
    QVariantList m_fontZoomLevels;
    ScriteDocument *m_scriteDocument = nullptr;
    QObjectProperty<QScreen> m_screen;
    ScreenplayPageLayout *m_pageLayout = new ScreenplayPageLayout(this);

    static SceneElementFormat *staticElementFormatAt(QQmlListProperty<SceneElementFormat> *list,
                                                     int index);
    static int staticElementFormatCount(QQmlListProperty<SceneElementFormat> *list);
    QList<SceneElementFormat *> m_elementFormats;
};

class TextFormat : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    explicit TextFormat(QObject *parent = nullptr);
    ~TextFormat();

    // clang-format off
    Q_PROPERTY(bool bold
               READ isBold
               WRITE setBold
               NOTIFY boldChanged)
    // clang-format on
    void setBold(bool val);
    bool isBold() const { return m_bold; }
    Q_SIGNAL void boldChanged();

    Q_INVOKABLE void toggleBold() { this->setBold(!m_bold); }

    // clang-format off
    Q_PROPERTY(bool italics
               READ isItalics
               WRITE setItalics
               NOTIFY italicsChanged)
    // clang-format on
    void setItalics(bool val);
    bool isItalics() const { return m_italics; }
    Q_SIGNAL void italicsChanged();

    Q_INVOKABLE void toggleItalics() { this->setItalics(!m_italics); }

    // clang-format off
    Q_PROPERTY(bool underline
               READ isUnderline
               WRITE setUnderline
               NOTIFY underlineChanged)
    // clang-format on
    void setUnderline(bool val);
    bool isUnderline() const { return m_underline; }
    Q_SIGNAL void underlineChanged();

    Q_INVOKABLE void toggleUnderline() { this->setUnderline(!m_underline); }

    // clang-format off
    Q_PROPERTY(bool strikeout
               READ isStrikeout
               WRITE setStrikeout
               NOTIFY strikeoutChanged)
    // clang-format on
    void setStrikeout(bool val);
    bool isStrikeout() const { return m_strikeout; }
    Q_SIGNAL void strikeoutChanged();

    Q_INVOKABLE void toggleStrikeout() { this->setStrikeout(!m_strikeout); }

    // clang-format off
    Q_PROPERTY(QColor textColor
               READ textColor
               WRITE setTextColor
               NOTIFY textColorChanged)
    // clang-format on
    void setTextColor(const QColor &val);
    QColor textColor() const { return m_textColor; }
    Q_SIGNAL void textColorChanged();

    // clang-format off
    Q_PROPERTY(bool hasTextColor
               READ hasTextColor
               NOTIFY textColorChanged)
    // clang-format on
    bool hasTextColor() const { return m_textColor.alpha() > 0; }

    Q_INVOKABLE void resetTextColor() { this->setTextColor(Qt::transparent); }

    // clang-format off
    Q_PROPERTY(QColor backgroundColor
               READ backgroundColor
               WRITE setBackgroundColor
               NOTIFY backgroundColorChanged)
    // clang-format on
    void setBackgroundColor(const QColor &val);
    QColor backgroundColor() const { return m_backgroundColor; }
    Q_SIGNAL void backgroundColorChanged();

    // clang-format off
    Q_PROPERTY(bool hasBackgroundColor
               READ hasBackgroundColor
               NOTIFY backgroundColorChanged)
    // clang-format on
    bool hasBackgroundColor() const { return m_backgroundColor.alpha() > 0; }

    Q_INVOKABLE void resetBackgroundColor() { this->setBackgroundColor(Qt::transparent); }

    Q_INVOKABLE void reset();

    void updateFromCharFormat(const QTextCharFormat &format);
    bool isUpdatingFromCharFormat() const { return m_updatingFromFormat; }
    QTextCharFormat toCharFormat(const QList<int> &properties = allProperties()) const;

    static QList<int> allProperties();

signals:
    void formatChanged(const QList<int> &properties = allProperties());

private:
    bool m_bold = false;
    bool m_italics = false;
    bool m_strikeout = false;
    bool m_underline = false;
    bool m_updatingFromFormat = false;
    QColor m_textColor = Qt::transparent;
    QColor m_backgroundColor = Qt::transparent;
};

class SceneDocumentBlockUserData;
class SceneDocumentBinder : public QSyntaxHighlighter, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    QML_ELEMENT

public:
    explicit SceneDocumentBinder(QObject *parent = nullptr);
    ~SceneDocumentBinder();

    // clang-format off
    Q_PROPERTY(ScreenplayFormat *screenplayFormat
               READ screenplayFormat
               WRITE setScreenplayFormat
               NOTIFY screenplayFormatChanged
               RESET resetScreenplayFormat)
    // clang-format on
    void setScreenplayFormat(ScreenplayFormat *val);
    ScreenplayFormat *screenplayFormat() const { return m_screenplayFormat; }
    Q_SIGNAL void screenplayFormatChanged();

    // clang-format off
    Q_PROPERTY(Scene *scene
               READ scene
               WRITE setScene
               NOTIFY sceneChanged
               RESET resetScene)
    // clang-format on
    void setScene(Scene *val);
    Scene *scene() const { return m_scene; }
    Q_SIGNAL void sceneChanged();

    // clang-format off
    Q_PROPERTY(ScreenplayElement *screenplayElement
               READ screenplayElement
               WRITE setScreenplayElement
               NOTIFY screenplayElementChanged
               RESET resetScreenplayElement)
    // clang-format on
    void setScreenplayElement(ScreenplayElement *val);
    ScreenplayElement *screenplayElement() const { return m_screenplayElement; }
    Q_SIGNAL void screenplayElementChanged();

    // clang-format off
    Q_PROPERTY(QQuickTextDocument *textDocument
               READ textDocument
               WRITE setTextDocument
               NOTIFY textDocumentChanged
               RESET resetTextDocument)
    // clang-format on
    void setTextDocument(QQuickTextDocument *val);
    QQuickTextDocument *textDocument() const { return m_textDocument; }
    Q_SIGNAL void textDocumentChanged();

    // clang-format off
    Q_PROPERTY(bool spellCheckEnabled
               READ isSpellCheckEnabled
               WRITE setSpellCheckEnabled
               NOTIFY spellCheckEnabledChanged)
    // clang-format on
    void setSpellCheckEnabled(bool val);
    bool isSpellCheckEnabled() const { return m_spellCheckEnabled; }
    Q_SIGNAL void spellCheckEnabledChanged();

    // clang-format off
    Q_PROPERTY(bool liveSpellCheckEnabled
               READ isLiveSpellCheckEnabled
               WRITE setLiveSpellCheckEnabled
               NOTIFY liveSpellCheckEnabledChanged)
    // clang-format on
    void setLiveSpellCheckEnabled(bool val);
    bool isLiveSpellCheckEnabled() const { return m_liveSpellCheckEnabled; }
    Q_SIGNAL void liveSpellCheckEnabledChanged();

    // clang-format off
    Q_PROPERTY(bool autoCapitalizeSentences
               READ isAutoCapitalizeSentences
               WRITE setAutoCapitalizeSentences
               NOTIFY autoCapitalizeSentencesChanged)
    // clang-format on
    void setAutoCapitalizeSentences(bool val);
    bool isAutoCapitalizeSentences() const { return m_autoCapitalizeSentences; }
    Q_SIGNAL void autoCapitalizeSentencesChanged();

    // Adds : at end of shots & transitions, CONT'D for characters where applicable.
    // clang-format off
    Q_PROPERTY(bool autoPolishParagraphs
               READ isAutoPolishParagraphs
               WRITE setAutoPolishParagraphs
               NOTIFY autoPolishParagraphsChanged)
    // clang-format on
    void setAutoPolishParagraphs(bool val);
    bool isAutoPolishParagraphs() const { return m_autoPolishParagraphs; }
    Q_SIGNAL void autoPolishParagraphsChanged();

    // clang-format off
    Q_PROPERTY(qreal textWidth
               READ textWidth
               WRITE setTextWidth
               NOTIFY textWidthChanged)
    // clang-format on
    void setTextWidth(qreal val);
    qreal textWidth() const { return m_textWidth; }
    Q_SIGNAL void textWidthChanged();

    // clang-format off
    Q_PROPERTY(int cursorPosition
               READ cursorPosition
               WRITE setCursorPosition
               NOTIFY cursorPositionChanged)
    // clang-format on
    void setCursorPosition(int val);
    int cursorPosition() const { return m_cursorPosition; }
    Q_SIGNAL void cursorPositionChanged();

    // clang-format off
    Q_PROPERTY(int selectionStartPosition
               READ selectionStartPosition
               WRITE setSelectionStartPosition
               NOTIFY selectionStartPositionChanged)
    // clang-format on
    void setSelectionStartPosition(int val);
    int selectionStartPosition() const { return m_selectionStartPosition; }
    Q_SIGNAL void selectionStartPositionChanged();

    // clang-format off
    Q_PROPERTY(int selectionEndPosition
               READ selectionEndPosition
               WRITE setSelectionEndPosition
               NOTIFY selectionEndPositionChanged)
    // clang-format on
    void setSelectionEndPosition(int val);
    int selectionEndPosition() const { return m_selectionEndPosition; }
    Q_SIGNAL void selectionEndPositionChanged();

    enum TextCasing { LowerCase, UpperCase };
    Q_ENUM(TextCasing)

    Q_INVOKABLE bool changeTextCase(SceneDocumentBinder::TextCasing casing);

    // clang-format off
    Q_PROPERTY(bool applyTextFormat
               READ isApplyTextFormat
               WRITE setApplyTextFormat
               NOTIFY applyTextFormatChanged)
    // clang-format on
    void setApplyTextFormat(bool val);
    bool isApplyTextFormat() const { return m_applyTextFormat; }
    Q_SIGNAL void applyTextFormatChanged();

    // clang-format off
    Q_PROPERTY(TextFormat *textFormat
               READ textFormat
               CONSTANT )
    // clang-format on
    TextFormat *textFormat() const { return m_textFormat; }

    Q_SIGNAL void requestCursorPosition(int position);

    // clang-format off
    Q_PROPERTY(QStringList characterNames
               READ characterNames
               WRITE setCharacterNames
               NOTIFY characterNamesChanged)
    // clang-format on
    void setCharacterNames(const QStringList &val);
    QStringList characterNames() const { return m_characterNames; }
    Q_SIGNAL void characterNamesChanged();

    // clang-format off
    Q_PROPERTY(QStringList transitions
               READ transitions
               WRITE setTransitions
               NOTIFY transitionsChanged)
    // clang-format on
    void setTransitions(const QStringList &val);
    QStringList transitions() const { return m_transitions; }
    Q_SIGNAL void transitionsChanged();

    // clang-format off
    Q_PROPERTY(QStringList shots
               READ shots
               WRITE setShots
               NOTIFY shotsChanged)
    // clang-format on
    void setShots(const QStringList &val);
    QStringList shots() const { return m_shots; }
    Q_SIGNAL void shotsChanged();

    // clang-format off
    Q_PROPERTY(SceneElement *currentElement
               READ currentElement
               NOTIFY currentElementChanged
               RESET resetCurrentElement)
    // clang-format on
    SceneElement *currentElement() const { return m_currentElement; }
    Q_SIGNAL void currentElementChanged();

    // clang-format off
    Q_PROPERTY(int currentElementCursorPosition
               READ currentElementCursorPosition
               NOTIFY cursorPositionChanged)
    // clang-format on
    int currentElementCursorPosition() const { return m_currentElementCursorPosition; }

    // clang-format off
    Q_PROPERTY(QList<SceneElement *>
               selectedElements READ
               selectedElements NOTIFY
               selectedElementsChanged )
    // clang-format on
    QList<SceneElement *> selectedElements() const;
    Q_SIGNAL void selectedElementsChanged();

    Q_INVOKABLE SceneElement *sceneElementAt(int cursorPosition) const;
    Q_INVOKABLE QRectF sceneElementBoundingRect(SceneElement *sceneElement) const;

    // clang-format off
    Q_PROPERTY(bool forceSyncDocument
               READ isForceSyncDocument
               WRITE setForceSyncDocument
               NOTIFY forceSyncDocumentChanged)
    // clang-format on
    void setForceSyncDocument(bool val);
    bool isForceSyncDocument() const { return m_forceSyncDocument; }
    Q_SIGNAL void forceSyncDocumentChanged();

    // clang-format off
    Q_PROPERTY(bool applyLanguageFonts
               READ isApplyLanguageFonts
               WRITE setApplyLanguageFonts
               NOTIFY applyLanguageFontsChanged)
    // clang-format on
    void setApplyLanguageFonts(bool val);
    bool isApplyLanguageFonts() const { return m_applyLanguageFonts; }
    Q_SIGNAL void applyLanguageFontsChanged();

    // clang-format off
    Q_PROPERTY(QString nextTabFormatAsString
               READ nextTabFormatAsString
               NOTIFY nextTabFormatChanged)
    // clang-format on
    QString nextTabFormatAsString() const;

    // clang-format off
    Q_PROPERTY(int nextTabFormat
               READ nextTabFormat
               NOTIFY nextTabFormatChanged)
    // clang-format on
    int nextTabFormat() const;
    Q_SIGNAL void nextTabFormatChanged();

    Q_INVOKABLE void tab();
    Q_INVOKABLE void backtab();
    Q_INVOKABLE bool canGoUp();
    Q_INVOKABLE bool canGoDown();
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void reload();

    Q_INVOKABLE int lastCursorPosition() const;
    Q_INVOKABLE int cursorPositionAtBlock(int blockNumber) const;
    Q_INVOKABLE int currentBlockPosition() const;

    // clang-format off
    Q_PROPERTY(QStringList spellingSuggestions
               READ spellingSuggestions
               NOTIFY spellingSuggestionsChanged)
    // clang-format on
    QStringList spellingSuggestions() const { return m_spellingSuggestions; }
    Q_SIGNAL void spellingSuggestionsChanged();

    // clang-format off
    Q_PROPERTY(bool wordUnderCursorIsMisspelled
               READ isWordUnderCursorIsMisspelled
               NOTIFY wordUnderCursorIsMisspelledChanged)
    // clang-format on
    bool isWordUnderCursorIsMisspelled() const { return m_wordUnderCursorIsMisspelled; }
    Q_SIGNAL void wordUnderCursorIsMisspelledChanged();

    Q_INVOKABLE QStringList spellingSuggestionsForWordAt(int position) const;

    Q_INVOKABLE void replaceWordAt(int position, const QString &with);
    Q_INVOKABLE void replaceWordUnderCursor(const QString &with)
    {
        this->replaceWordAt(m_cursorPosition, with);
    }

    Q_INVOKABLE void addWordAtPositionToDictionary(int position);
    Q_INVOKABLE void addWordUnderCursorToDictionary()
    {
        this->addWordAtPositionToDictionary(m_cursorPosition);
    }

    Q_INVOKABLE void addWordAtPositionToIgnoreList(int position);
    Q_INVOKABLE void addWordUnderCursorToIgnoreList()
    {
        this->addWordAtPositionToIgnoreList(m_cursorPosition);
    }

    // clang-format off
    Q_PROPERTY(QStringList autoCompleteHints
               READ autoCompleteHints
               NOTIFY autoCompleteHintsChanged)
    // clang-format on
    QStringList autoCompleteHints() const { return m_autoCompleteHints; }
    Q_SIGNAL void autoCompleteHintsChanged();

    // clang-format off
    Q_PROPERTY(QStringList priorityAutoCompleteHints
               READ priorityAutoCompleteHints
               NOTIFY autoCompleteHintsChanged)
    // clang-format on
    QStringList priorityAutoCompleteHints() const { return m_priorityAutoCompleteHints; }

    // clang-format off
    Q_PROPERTY(SceneElement::Type autoCompleteHintsFor
               READ autoCompleteHintsFor
               NOTIFY autoCompleteHintsForChanged)
    // clang-format on
    SceneElement::Type autoCompleteHintsFor() const { return m_autoCompleteHintsFor; }
    Q_SIGNAL void autoCompleteHintsForChanged();

    // clang-format off
    Q_PROPERTY(QString completionPrefix
               READ completionPrefix
               NOTIFY completionPrefixChanged)
    // clang-format on
    QString completionPrefix() const { return m_completionPrefix; }
    Q_SIGNAL void completionPrefixChanged();

    // clang-format off
    Q_PROPERTY(int completionPrefixStart
               READ completionPrefixStart
               NOTIFY completionPrefixChanged)
    // clang-format on
    int completionPrefixStart() const { return m_completionPrefixStart; }

    // clang-format off
    Q_PROPERTY(int completionPrefixEnd
               READ completionPrefixEnd
               NOTIFY completionPrefixChanged)
    // clang-format on
    int completionPrefixEnd() const { return m_completionPrefixEnd; }

    // clang-format off
    Q_PROPERTY(bool hasCompletionPrefixBoundary
               READ hasCompletionPrefixBoundary
               NOTIFY completionPrefixChanged)
    // clang-format on
    bool hasCompletionPrefixBoundary() const
    {
        return m_completionPrefixStart >= 0 && m_completionPrefixEnd >= 0;
    }

    enum CompletionMode {
        NoCompletionMode,
        CharacterNameCompletionMode,
        CharacterBracketNotationCompletionMode,
        ShotCompletionMode,
        TransitionCompletionMode
    };
    Q_ENUM(CompletionMode)
    // clang-format off
    Q_PROPERTY(CompletionMode completionMode
               READ completionMode
               NOTIFY completionModeChanged)
    // clang-format on
    CompletionMode completionMode() const { return m_completionMode; }
    Q_SIGNAL void completionModeChanged();

    // clang-format off
    Q_PROPERTY(QFont currentFont
               READ currentFont
               NOTIFY currentFontChanged)
    // clang-format on
    QFont currentFont() const;
    Q_SIGNAL void currentFontChanged();

    // clang-format off
    Q_PROPERTY(int documentLoadCount
               READ documentLoadCount
               NOTIFY documentLoadCountChanged)
    // clang-format on
    int documentLoadCount() const { return m_documentLoadCount; }
    Q_SIGNAL void documentLoadCountChanged();

    Q_SIGNAL void documentInitialized();
    Q_SIGNAL void spellingMistakesDetected();

    Q_INVOKABLE void copy(int fromPosition, int toPosition);
    Q_INVOKABLE int paste(int fromPosition = -1);

    // clang-format off
    Q_PROPERTY(bool applyFormattingEvenInTransaction
               READ isApplyFormattingEvenInTransaction
               WRITE setApplyFormattingEvenInTransaction
               NOTIFY applyFormattingEvenInTransactionChanged)
    // clang-format on
    void setApplyFormattingEvenInTransaction(bool val);
    bool isApplyFormattingEvenInTransaction() const { return m_applyFormattingEvenInTransaction; }
    Q_SIGNAL void applyFormattingEvenInTransactionChanged();

    // QQmlParserStatus interface
    void classBegin();
    void componentComplete();

protected:
    // QSyntaxHighlighter interface
    void highlightBlock(const QString &text);

    // QObject interface
    void timerEvent(QTimerEvent *te);
    bool eventFilter(QObject *watched, QEvent *event);

    // Helpers
    void mergeFormat(int start, int count, const QTextCharFormat &format);

private:
    void resetScene();
    void resetTextDocument();
    void resetScreenplayFormat();
    void resetScreenplayElement();

    void initializeDocument();
    void initializeDocumentLater();
    void setDocumentLoadCount(int val);
    void setCurrentElement(SceneElement *val);
    void resetCurrentElement();
    void activateCurrentElementDefaultLanguage();
    void onSceneElementChanged(SceneElement *element, Scene::SceneElementChangeType type);
    Q_SLOT void onSpellCheckUpdated();
    void onContentsChange(int from, int charsRemoved, int charsAdded);
    void syncSceneFromDocument(int nrBlocks = -1);

    void evaluateAutoCompleteHintsAndCompletionPrefix();
    void setAutoCompleteHintsFor(SceneElement::Type val);
    void setAutoCompleteHints(const QStringList &hints,
                              const QStringList &priorityHints = QStringList());
    void setCompletionPrefix(const QString &prefix, int start = -1, int end = -1);
    void setCompletionMode(CompletionMode val);
    void setSpellingSuggestions(const QStringList &val);
    void setWordUnderCursorIsMisspelled(bool val);

    void onSceneAboutToReset();
    void onSceneReset(int position);
    void onSceneRefreshed();

    void rehighlightLater();
    void rehighlightBlockLater(const QTextBlock &block);

    void applyBlockFormatLater(const QTextBlock &block);

    void onTextFormatChanged(const QList<int> &properties);

    void polishAllSceneElements();
    void polishSceneElement(SceneElement *element);

    void performAllSceneElementTasks();

private:
    friend class SpellCheckService;
    friend class ForceCursorPositionHack;
    friend class SceneDocumentBlockUserData;

    int m_completionPrefixEnd = -1;
    int m_completionPrefixStart = -1;
    int m_currentElementCursorPosition = -1;
    int m_cursorPosition = -1;
    int m_documentLoadCount = 0;
    int m_selectionEndPosition = -1;
    int m_selectionStartPosition = -1;

    bool m_acceptTextFormatChanges = true;
    bool m_applyFormattingEvenInTransaction = false;
    bool m_applyLanguageFonts = false;
    bool m_applyNextCharFormat = false;
    bool m_applyTextFormat = false;
    bool m_autoCapitalizeSentences = true;
    bool m_autoPolishParagraphs = true;
    bool m_forceSyncDocument = false;
    bool m_initializingDocument = false;
    bool m_liveSpellCheckEnabled = true;
    bool m_pastingContent = false;
    bool m_sceneElementTaskIsRunning = false;
    bool m_sceneIsBeingRefreshed = false;
    bool m_sceneIsBeingReset = false;
    bool m_spellCheckEnabled = true;
    bool m_wordUnderCursorIsMisspelled = false;

    qreal m_textWidth = 0;

    QString m_completionPrefix;

    QStringList m_autoCompleteHints;
    QStringList m_characterNames;
    QStringList m_priorityAutoCompleteHints;
    QStringList m_shots;
    QStringList m_spellingSuggestions;
    QStringList m_transitions;

    CompletionMode m_completionMode = NoCompletionMode;

    SceneElement::Type m_autoCompleteHintsFor = SceneElement::Action;

    ExecLaterTimer m_applyBlockFormatTimer;
    ExecLaterTimer m_initializeDocumentTimer;
    ExecLaterTimer m_rehighlightTimer;
    ExecLaterTimer m_sceneElementTaskTimer;

    QTextCharFormat m_nextCharFormat;

    QList<QTextBlock> m_applyBlockFormatQueue;
    QList<QTextBlock> m_rehighlightBlockQueue;
    QList<SceneElement::Type> m_tabHistory;

    QObjectProperty<QQuickTextDocument> m_textDocument;
    QObjectProperty<Scene> m_scene;
    QObjectProperty<SceneElement> m_currentElement;
    QObjectProperty<ScreenplayElement> m_screenplayElement;
    QObjectProperty<ScreenplayFormat> m_screenplayFormat;

    TextFormat *m_textFormat = new TextFormat(this);
};

#endif // FORMATTING_H
