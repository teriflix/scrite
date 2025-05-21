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
#include "transliteration.h"
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

class SceneElementFormat : public QObject, public Modifiable
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    ~SceneElementFormat();

    Q_PROPERTY(ScreenplayFormat *format READ format CONSTANT STORED false)
    ScreenplayFormat *format() const { return m_format; }

    Q_PROPERTY(SceneElement::Type elementType READ elementType CONSTANT)
    SceneElement::Type elementType() const { return m_elementType; }
    Q_SIGNAL void elementTypeChanged();

    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged)
    void setFont(const QFont &val);
    QFont &fontRef() { return m_font; }
    QFont font() const { return m_font; }
    Q_SIGNAL void fontChanged();

    Q_PROPERTY(QFont font2 READ font2 NOTIFY font2Changed)
    QFont font2() const;
    Q_SIGNAL void font2Changed();

    Q_INVOKABLE void setFontFamily(const QString &val);
    Q_INVOKABLE void setFontBold(bool val);
    Q_INVOKABLE void setFontItalics(bool val);
    Q_INVOKABLE void setFontUnderline(bool val);
    Q_INVOKABLE void setFontPointSize(int val);
    Q_INVOKABLE void setFontCapitalization(QFont::Capitalization caps);

    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    void setTextColor(const QColor &val);
    QColor textColor() const { return m_textColor; }
    Q_SIGNAL void textColorChanged();

    Q_PROPERTY(Qt::Alignment textAlignment READ textAlignment WRITE setTextAlignment NOTIFY
                       textAlignmentChanged)
    void setTextAlignment(Qt::Alignment val);
    Qt::Alignment textAlignment() const { return m_textAlignment; }
    Q_SIGNAL void textAlignmentChanged();

    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY
                       backgroundColorChanged)
    void setBackgroundColor(const QColor &val);
    QColor backgroundColor() const { return m_backgroundColor; }
    Q_SIGNAL void backgroundColorChanged();

    Q_PROPERTY(qreal textIndent READ textIndent WRITE setTextIndent NOTIFY textIndentChanged)
    void setTextIndent(qreal val);
    qreal textIndent() const { return m_textIndent; }
    Q_SIGNAL void textIndentChanged();

    Q_PROPERTY(qreal lineHeight READ lineHeight WRITE setLineHeight NOTIFY lineHeightChanged)
    void setLineHeight(qreal val);
    qreal lineHeight() const { return m_lineHeight; }
    Q_SIGNAL void lineHeightChanged();

    Q_PROPERTY(qreal lineSpacingBefore READ lineSpacingBefore WRITE setLineSpacingBefore NOTIFY
                       lineSpacingBeforeChanged STORED false)
    void setLineSpacingBefore(qreal val);
    qreal lineSpacingBefore() const { return m_lineSpacingBefore; }
    Q_SIGNAL void lineSpacingBeforeChanged();

    Q_PROPERTY(qreal leftMargin READ leftMargin WRITE setLeftMargin NOTIFY leftMarginChanged
                       STORED false)
    void setLeftMargin(qreal val);
    qreal leftMargin() const { return m_leftMargin; }
    Q_SIGNAL void leftMarginChanged();

    Q_PROPERTY(qreal rightMargin READ rightMargin WRITE setRightMargin NOTIFY rightMarginChanged
                       STORED false)
    void setRightMargin(qreal val);
    qreal rightMargin() const { return m_rightMargin; }
    Q_SIGNAL void rightMarginChanged();

    // Must be manually kept in sync with TransliterationEngine::Language
    enum DefaultLanguage {
        Default,
        English,
        Bengali,
        Gujarati,
        Hindi,
        Kannada,
        Malayalam,
        Marathi,
        Oriya,
        Punjabi,
        Sanskrit,
        Tamil,
        Telugu
    };
    Q_ENUM(DefaultLanguage)
    Q_PROPERTY(DefaultLanguage defaultLanguage READ defaultLanguage WRITE setDefaultLanguage NOTIFY
                       defaultLanguageChanged)
    void setDefaultLanguage(DefaultLanguage val);
    DefaultLanguage defaultLanguage() const { return m_defaultLanguage; }
    Q_SIGNAL void defaultLanguageChanged();

    Q_PROPERTY(int defaultLanguageInt READ defaultLanguageInt WRITE setDefaultLanguageInt NOTIFY
                       defaultLanguageChanged)
    int defaultLanguageInt() const { return int(m_defaultLanguage); }
    void setDefaultLanguageInt(int val) { this->setDefaultLanguage(DefaultLanguage(val)); }

    Q_INVOKABLE void activateDefaultLanguage();

    QTextBlockFormat createBlockFormat(Qt::Alignment overrideAlignment,
                                       const qreal *pageWidth = nullptr) const;
    QTextCharFormat createCharFormat(const qreal *pageWidth = nullptr) const;

    Q_SIGNAL void elementFormatChanged();

    enum Properties {
        FontFamily,
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

    Q_PROPERTY(bool inTransaction READ isInTransaction NOTIFY inTransactionChanged)
    bool isInTransaction() const { return m_inTransaction; }
    Q_SIGNAL void inTransactionChanged();

    void resetToFactoryDefaults();

private:
    friend class ScreenplayFormat;
    SceneElementFormat(SceneElement::Type type = SceneElement::Action,
                       ScreenplayFormat *parent = nullptr);
    void countTransactionChange() { ++m_nrChangesDuringTransation; }

private:
    QFont m_font;
    qreal m_textIndent = 0.0;
    qreal m_lineHeight = 1.0;
    qreal m_leftMargin = 0;
    qreal m_rightMargin = 0;
    qreal m_lineSpacingBefore = 0;
    QColor m_textColor = QColor(Qt::black);
    QColor m_backgroundColor = QColor(Qt::transparent);
    bool m_inTransaction = false;
    int m_nrChangesDuringTransation = 0;
    ScreenplayFormat *m_format = nullptr;
    Qt::Alignment m_textAlignment = Qt::AlignLeft;
    SceneElement::Type m_elementType = SceneElement::Action;
    DefaultLanguage m_defaultLanguage = Default;

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

    Q_PROPERTY(ScreenplayFormat *format READ format CONSTANT)
    ScreenplayFormat *format() const { return m_format; }

    enum PaperSize { A4, Letter };
    Q_ENUM(PaperSize)
    Q_PROPERTY(PaperSize paperSize READ paperSize WRITE setPaperSize NOTIFY paperSizeChanged
                       STORED false)
    void setPaperSize(PaperSize val);
    PaperSize paperSize() const { return m_paperSize; }
    Q_SIGNAL void paperSizeChanged();

    Q_PROPERTY(QMarginsF margins READ margins NOTIFY rectsChanged STORED false)
    QMarginsF margins() const { return m_margins; }
    Q_SIGNAL void marginsChanged();

    Q_PROPERTY(qreal leftMargin READ leftMargin NOTIFY rectsChanged STORED false)
    qreal leftMargin() const { return m_margins.left(); }

    Q_PROPERTY(qreal topMargin READ topMargin NOTIFY rectsChanged STORED false)
    qreal topMargin() const { return m_margins.top(); }

    Q_PROPERTY(qreal rightMargin READ rightMargin NOTIFY rectsChanged STORED false)
    qreal rightMargin() const { return m_margins.right(); }

    Q_PROPERTY(qreal bottomMargin READ bottomMargin NOTIFY rectsChanged STORED false)
    qreal bottomMargin() const { return m_margins.bottom(); }

    Q_PROPERTY(QRectF paperRect READ paperRect NOTIFY rectsChanged STORED false)
    QRectF paperRect() const { return m_paperRect; }
    Q_SIGNAL void paperRectChanged();

    Q_PROPERTY(qreal paperWidth READ paperWidth NOTIFY rectsChanged STORED false)
    qreal paperWidth() const { return m_paperRect.width(); }

    Q_PROPERTY(qreal pageWidth READ pageWidth NOTIFY rectsChanged STORED false)
    qreal pageWidth() const { return m_paperRect.width(); }

    Q_PROPERTY(QRectF paintRect READ paintRect NOTIFY rectsChanged STORED false)
    QRectF paintRect() const { return m_paintRect; }
    Q_SIGNAL void paintRectChanged();

    Q_PROPERTY(QRectF headerRect READ headerRect NOTIFY rectsChanged STORED false)
    QRectF headerRect() const { return m_headerRect; }
    Q_SIGNAL void headerRectChanged();

    Q_PROPERTY(QRectF footerRect READ footerRect NOTIFY rectsChanged STORED false)
    QRectF footerRect() const { return m_footerRect; }
    Q_SIGNAL void footerRectChanged();

    Q_PROPERTY(QRectF contentRect READ contentRect NOTIFY rectsChanged STORED false)
    QRectF contentRect() const { return m_paintRect; }

    Q_PROPERTY(qreal contentWidth READ contentWidth NOTIFY rectsChanged STORED false)
    qreal contentWidth() const { return m_paintRect.width(); }

    Q_PROPERTY(qreal defaultResolution READ defaultResolution NOTIFY defaultResolutionChanged)
    qreal defaultResolution() const { return m_defaultResolution; }
    Q_SIGNAL void defaultResolutionChanged();

    Q_PROPERTY(qreal customResolution READ customResolution WRITE setCustomResolution NOTIFY
                       customResolutionChanged)
    void setCustomResolution(qreal val);
    qreal customResolution() const { return m_customResolution; }
    Q_SIGNAL void customResolutionChanged();

    Q_PROPERTY(qreal resolution READ resolution NOTIFY resolutionChanged)
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
    char m_padding[4];
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

    Q_PROPERTY(ScriteDocument *scriteDocument READ scriteDocument CONSTANT STORED false)
    ScriteDocument *scriteDocument() const { return m_scriteDocument; }

    Q_PROPERTY(QScreen *screen READ screen WRITE setScreen NOTIFY screenChanged RESET resetScreen
                       STORED false)
    Q_INVOKABLE void setScreen(QScreen *val);
    QScreen *screen() const { return m_screen; }
    Q_SIGNAL void screenChanged();

    Q_INVOKABLE void setSreeenFromWindow(QObject *windowObject);

    qreal screenDevicePixelRatio() const { return m_screen ? m_screen->devicePixelRatio() : 1.0; }

    Q_PROPERTY(qreal devicePixelRatio READ devicePixelRatio NOTIFY fontZoomLevelIndexChanged)
    qreal devicePixelRatio() const;

    Q_PROPERTY(ScreenplayPageLayout *pageLayout READ pageLayout CONSTANT STORED false)
    ScreenplayPageLayout *pageLayout() const { return m_pageLayout; }

    Q_PROPERTY(TransliterationEngine::Language defaultLanguage READ defaultLanguage WRITE
                       setDefaultLanguage NOTIFY defaultLanguageChanged STORED false)
    void setDefaultLanguage(TransliterationEngine::Language val);
    TransliterationEngine::Language defaultLanguage() const { return m_defaultLanguage; }
    Q_SIGNAL void defaultLanguageChanged();

    Q_PROPERTY(int defaultLanguageInt READ defaultLanguageInt WRITE setDefaultLanguageInt NOTIFY
                       defaultLanguageChanged STORED false)
    int defaultLanguageInt() const { return int(m_defaultLanguage); }
    void setDefaultLanguageInt(int val)
    {
        this->setDefaultLanguage(TransliterationEngine::Language(val));
    }

    Q_PROPERTY(QFont defaultFont READ defaultFont WRITE setDefaultFont NOTIFY defaultFontChanged)
    void setDefaultFont(const QFont &val);
    QFont defaultFont() const { return m_defaultFont; }
    QFont &defaultFontRef() { return m_defaultFont; }
    Q_SIGNAL void defaultFontChanged();

    Q_PROPERTY(QFont defaultFont2 READ defaultFont2 NOTIFY fontPointSizeDeltaChanged)
    QFont defaultFont2() const;

    QFontMetrics defaultFontMetrics() const { return m_defaultFontMetrics; }
    QFontMetrics defaultFont2Metrics() const { return m_defaultFont2Metrics; }

    Q_PROPERTY(int fontPointSizeDelta READ fontPointSizeDelta NOTIFY fontPointSizeDeltaChanged)
    int fontPointSizeDelta() const { return m_fontPointSizeDelta; }
    Q_SIGNAL void fontPointSizeDeltaChanged();

    Q_PROPERTY(int fontZoomLevelIndex READ fontZoomLevelIndex WRITE setFontZoomLevelIndex NOTIFY
                       fontZoomLevelIndexChanged STORED false)
    void setFontZoomLevelIndex(int val);
    int fontZoomLevelIndex() const { return m_fontZoomLevelIndex; }
    Q_SIGNAL void fontZoomLevelIndexChanged();

    Q_PROPERTY(QVariantList fontZoomLevels READ fontZoomLevels NOTIFY fontZoomLevelsChanged)
    QVariantList fontZoomLevels() const { return m_fontZoomLevels; }
    Q_SIGNAL void fontZoomLevelsChanged();

    Q_INVOKABLE SceneElementFormat *elementFormat(SceneElement::Type type) const;
    Q_INVOKABLE SceneElementFormat *elementFormat(int type) const;
    Q_SIGNAL void formatChanged();

    Q_PROPERTY(QQmlListProperty<SceneElementFormat> elementFormats READ elementFormats)
    QQmlListProperty<SceneElementFormat> elementFormats();

    void applyToAll(const SceneElementFormat *from, SceneElementFormat::Properties properties);

    enum Role { SceneElementFomat = Qt::UserRole };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

    Q_PROPERTY(int secondsPerPage READ secondsPerPage WRITE setSecondsPerPage NOTIFY
                       secondsPerPageChanged)
    void setSecondsPerPage(int val);
    int secondsPerPage() const { return m_secondsPerPage; }
    Q_SIGNAL void secondsPerPageChanged();

    Q_INVOKABLE void resetToFactoryDefaults();

    Q_INVOKABLE bool saveAsUserDefaults();
    Q_INVOKABLE void resetToUserDefaults();

    Q_INVOKABLE void beginTransaction();
    Q_INVOKABLE bool hasChangesToCommit() { return m_nrChangesDuringTransation > 0; }
    Q_INVOKABLE void commitTransaction();

    Q_PROPERTY(bool inTransaction READ isInTransaction NOTIFY inTransactionChanged)
    bool isInTransaction() const { return m_inTransaction; }
    Q_SIGNAL void inTransactionChanged();

    void useUserSpecifiedFonts();

    // Interface interface
    void deserializeFromJson(const QJsonObject &);

private:
    void resetScreen();
    void evaluateFontPointSizeDelta();
    void evaluateFontZoomLevels();
    void countTransactionChange() { ++m_nrChangesDuringTransation; }

private:
    char m_padding[4];
    QFont m_defaultFont;
    qreal m_pageWidth = 750.0;
    int m_secondsPerPage = 60;
    int m_fontPointSizeDelta = 0;
    int m_fontZoomLevelIndex = -1;
    bool m_inTransaction = false;
    int m_nrChangesDuringTransation = 0;
    QList<int> m_fontPointSizes;
    QVariantList m_fontZoomLevels;
    QObjectProperty<QScreen> m_screen;
    ScriteDocument *m_scriteDocument = nullptr;
    QFontMetrics m_defaultFontMetrics;
    QFontMetrics m_defaultFont2Metrics;
    QStringList m_suggestionsAtCursor;
    ScreenplayPageLayout *m_pageLayout = new ScreenplayPageLayout(this);
    TransliterationEngine::Language m_defaultLanguage = TransliterationEngine::English;

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

    Q_PROPERTY(bool bold READ isBold WRITE setBold NOTIFY boldChanged)
    void setBold(bool val);
    bool isBold() const { return m_bold; }
    Q_SIGNAL void boldChanged();

    Q_INVOKABLE void toggleBold() { this->setBold(!m_bold); }

    Q_PROPERTY(bool italics READ isItalics WRITE setItalics NOTIFY italicsChanged)
    void setItalics(bool val);
    bool isItalics() const { return m_italics; }
    Q_SIGNAL void italicsChanged();

    Q_INVOKABLE void toggleItalics() { this->setItalics(!m_italics); }

    Q_PROPERTY(bool underline READ isUnderline WRITE setUnderline NOTIFY underlineChanged)
    void setUnderline(bool val);
    bool isUnderline() const { return m_underline; }
    Q_SIGNAL void underlineChanged();

    Q_INVOKABLE void toggleUnderline() { this->setUnderline(!m_underline); }

    Q_PROPERTY(bool strikeout READ isStrikeout WRITE setStrikeout NOTIFY strikeoutChanged)
    void setStrikeout(bool val);
    bool isStrikeout() const { return m_strikeout; }
    Q_SIGNAL void strikeoutChanged();

    Q_INVOKABLE void toggleStrikeout() { this->setStrikeout(!m_strikeout); }

    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    void setTextColor(const QColor &val);
    QColor textColor() const { return m_textColor; }
    Q_SIGNAL void textColorChanged();

    Q_PROPERTY(bool hasTextColor READ hasTextColor NOTIFY textColorChanged)
    bool hasTextColor() const { return m_textColor.alpha() > 0; }

    Q_INVOKABLE void resetTextColor() { this->setTextColor(Qt::transparent); }

    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY
                       backgroundColorChanged)
    void setBackgroundColor(const QColor &val);
    QColor backgroundColor() const { return m_backgroundColor; }
    Q_SIGNAL void backgroundColorChanged();

    Q_PROPERTY(bool hasBackgroundColor READ hasBackgroundColor NOTIFY backgroundColorChanged)
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

    Q_PROPERTY(ScreenplayFormat *screenplayFormat READ screenplayFormat WRITE setScreenplayFormat
                       NOTIFY screenplayFormatChanged RESET resetScreenplayFormat)
    void setScreenplayFormat(ScreenplayFormat *val);
    ScreenplayFormat *screenplayFormat() const { return m_screenplayFormat; }
    Q_SIGNAL void screenplayFormatChanged();

    Q_PROPERTY(Scene *scene READ scene WRITE setScene NOTIFY sceneChanged RESET resetScene)
    void setScene(Scene *val);
    Scene *scene() const { return m_scene; }
    Q_SIGNAL void sceneChanged();

    Q_PROPERTY(ScreenplayElement* screenplayElement READ screenplayElement WRITE setScreenplayElement NOTIFY screenplayElementChanged RESET resetScreenplayElement)
    void setScreenplayElement(ScreenplayElement *val);
    ScreenplayElement *screenplayElement() const { return m_screenplayElement; }
    Q_SIGNAL void screenplayElementChanged();

    Q_PROPERTY(QQuickTextDocument *textDocument READ textDocument WRITE setTextDocument NOTIFY
                       textDocumentChanged RESET resetTextDocument)
    void setTextDocument(QQuickTextDocument *val);
    QQuickTextDocument *textDocument() const { return m_textDocument; }
    Q_SIGNAL void textDocumentChanged();

    Q_PROPERTY(bool spellCheckEnabled READ isSpellCheckEnabled WRITE setSpellCheckEnabled NOTIFY
                       spellCheckEnabledChanged)
    void setSpellCheckEnabled(bool val);
    bool isSpellCheckEnabled() const { return m_spellCheckEnabled; }
    Q_SIGNAL void spellCheckEnabledChanged();

    Q_PROPERTY(bool liveSpellCheckEnabled READ isLiveSpellCheckEnabled WRITE
                       setLiveSpellCheckEnabled NOTIFY liveSpellCheckEnabledChanged)
    void setLiveSpellCheckEnabled(bool val);
    bool isLiveSpellCheckEnabled() const { return m_liveSpellCheckEnabled; }
    Q_SIGNAL void liveSpellCheckEnabledChanged();

    Q_PROPERTY(bool autoCapitalizeSentences READ isAutoCapitalizeSentences WRITE setAutoCapitalizeSentences NOTIFY autoCapitalizeSentencesChanged)
    void setAutoCapitalizeSentences(bool val);
    bool isAutoCapitalizeSentences() const { return m_autoCapitalizeSentences; }
    Q_SIGNAL void autoCapitalizeSentencesChanged();

    // Adds : at end of shots & transitions, CONT'D for characters where applicable.
    Q_PROPERTY(bool autoPolishParagraphs READ autoPolishParagraphs WRITE setAutoPolishParagraphs NOTIFY autoPolishParagraphsChanged)
    void setAutoPolishParagraphs(bool val);
    bool autoPolishParagraphs() const { return m_autoPolishParagraphs; }
    Q_SIGNAL void autoPolishParagraphsChanged();

    Q_PROPERTY(qreal textWidth READ textWidth WRITE setTextWidth NOTIFY textWidthChanged)
    void setTextWidth(qreal val);
    qreal textWidth() const { return m_textWidth; }
    Q_SIGNAL void textWidthChanged();

    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY
                       cursorPositionChanged)
    void setCursorPosition(int val);
    int cursorPosition() const { return m_cursorPosition; }
    Q_SIGNAL void cursorPositionChanged();

    Q_PROPERTY(int selectionStartPosition READ selectionStartPosition WRITE
                       setSelectionStartPosition NOTIFY selectionStartPositionChanged)
    void setSelectionStartPosition(int val);
    int selectionStartPosition() const { return m_selectionStartPosition; }
    Q_SIGNAL void selectionStartPositionChanged();

    Q_PROPERTY(int selectionEndPosition READ selectionEndPosition WRITE setSelectionEndPosition
                       NOTIFY selectionEndPositionChanged)
    void setSelectionEndPosition(int val);
    int selectionEndPosition() const { return m_selectionEndPosition; }
    Q_SIGNAL void selectionEndPositionChanged();

    enum TextCasing { LowerCase, UpperCase };
    Q_ENUM(TextCasing)

    Q_INVOKABLE bool changeTextCase(SceneDocumentBinder::TextCasing casing);

    Q_PROPERTY(bool applyTextFormat READ isApplyTextFormat WRITE setApplyTextFormat NOTIFY
                       applyTextFormatChanged) void setApplyTextFormat(bool val);
    bool isApplyTextFormat() const { return m_applyTextFormat; }
    Q_SIGNAL void applyTextFormatChanged();

    Q_PROPERTY(TextFormat *textFormat READ textFormat CONSTANT)
    TextFormat *textFormat() const { return m_textFormat; }

    Q_SIGNAL void requestCursorPosition(int position);

    Q_PROPERTY(QStringList characterNames READ characterNames WRITE setCharacterNames NOTIFY
                       characterNamesChanged)
    void setCharacterNames(const QStringList &val);
    QStringList characterNames() const { return m_characterNames; }
    Q_SIGNAL void characterNamesChanged();

    Q_PROPERTY(
            QStringList transitions READ transitions WRITE setTransitions NOTIFY transitionsChanged)
    void setTransitions(const QStringList &val);
    QStringList transitions() const { return m_transitions; }
    Q_SIGNAL void transitionsChanged();

    Q_PROPERTY(QStringList shots READ shots WRITE setShots NOTIFY shotsChanged)
    void setShots(const QStringList &val);
    QStringList shots() const { return m_shots; }
    Q_SIGNAL void shotsChanged();

    Q_PROPERTY(SceneElement *currentElement READ currentElement NOTIFY currentElementChanged RESET
                       resetCurrentElement)
    SceneElement *currentElement() const { return m_currentElement; }
    Q_SIGNAL void currentElementChanged();

    Q_PROPERTY(int currentElementCursorPosition READ currentElementCursorPosition NOTIFY
                       cursorPositionChanged)
    int currentElementCursorPosition() const { return m_currentElementCursorPosition; }

    Q_PROPERTY(QList<SceneElement*> selectedElements READ selectedElements NOTIFY selectedElementsChanged)
    QList<SceneElement *> selectedElements() const;
    Q_SIGNAL void selectedElementsChanged();

    Q_INVOKABLE SceneElement *sceneElementAt(int cursorPosition) const;
    Q_INVOKABLE QRectF sceneElementBoundingRect(SceneElement *sceneElement) const;

    Q_PROPERTY(bool forceSyncDocument READ isForceSyncDocument WRITE setForceSyncDocument NOTIFY
                       forceSyncDocumentChanged)
    void setForceSyncDocument(bool val);
    bool isForceSyncDocument() const { return m_forceSyncDocument; }
    Q_SIGNAL void forceSyncDocumentChanged();

    Q_PROPERTY(bool applyLanguageFonts READ isApplyLanguageFonts WRITE setApplyLanguageFonts NOTIFY
                       applyLanguageFontsChanged)
    void setApplyLanguageFonts(bool val);
    bool isApplyLanguageFonts() const { return m_applyLanguageFonts; }
    Q_SIGNAL void applyLanguageFontsChanged();

    Q_PROPERTY(QString nextTabFormatAsString READ nextTabFormatAsString NOTIFY nextTabFormatChanged)
    QString nextTabFormatAsString() const;

    Q_PROPERTY(int nextTabFormat READ nextTabFormat NOTIFY nextTabFormatChanged)
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

    Q_PROPERTY(QStringList spellingSuggestions READ spellingSuggestions NOTIFY
                       spellingSuggestionsChanged)
    QStringList spellingSuggestions() const { return m_spellingSuggestions; }
    Q_SIGNAL void spellingSuggestionsChanged();

    Q_PROPERTY(bool wordUnderCursorIsMisspelled READ isWordUnderCursorIsMisspelled NOTIFY
                       wordUnderCursorIsMisspelledChanged)
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

    Q_PROPERTY(QStringList autoCompleteHints READ autoCompleteHints NOTIFY autoCompleteHintsChanged)
    QStringList autoCompleteHints() const { return m_autoCompleteHints; }
    Q_SIGNAL void autoCompleteHintsChanged();

    Q_PROPERTY(QStringList priorityAutoCompleteHints READ priorityAutoCompleteHints NOTIFY autoCompleteHintsChanged)
    QStringList priorityAutoCompleteHints() const { return m_priorityAutoCompleteHints; }

    Q_PROPERTY(SceneElement::Type autoCompleteHintsFor READ autoCompleteHintsFor NOTIFY
                       autoCompleteHintsForChanged)
    SceneElement::Type autoCompleteHintsFor() const { return m_autoCompleteHintsFor; }
    Q_SIGNAL void autoCompleteHintsForChanged();

    Q_PROPERTY(QString completionPrefix READ completionPrefix NOTIFY completionPrefixChanged)
    QString completionPrefix() const { return m_completionPrefix; }
    Q_SIGNAL void completionPrefixChanged();

    Q_PROPERTY(int completionPrefixStart READ completionPrefixStart NOTIFY completionPrefixChanged)
    int completionPrefixStart() const { return m_completionPrefixStart; }

    Q_PROPERTY(int completionPrefixEnd READ completionPrefixEnd NOTIFY completionPrefixChanged)
    int completionPrefixEnd() const { return m_completionPrefixEnd; }

    Q_PROPERTY(bool hasCompletionPrefixBoundary READ hasCompletionPrefixBoundary NOTIFY completionPrefixChanged)
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
    Q_PROPERTY(CompletionMode completionMode READ completionMode NOTIFY completionModeChanged)
    CompletionMode completionMode() const { return m_completionMode; }
    Q_SIGNAL void completionModeChanged();

    Q_PROPERTY(QFont currentFont READ currentFont NOTIFY currentFontChanged)
    QFont currentFont() const;
    Q_SIGNAL void currentFontChanged();

    Q_PROPERTY(int documentLoadCount READ documentLoadCount NOTIFY documentLoadCountChanged)
    int documentLoadCount() const { return m_documentLoadCount; }
    Q_SIGNAL void documentLoadCountChanged();

    Q_SIGNAL void documentInitialized();
    Q_SIGNAL void spellingMistakesDetected();

    Q_INVOKABLE void copy(int fromPosition, int toPosition);
    Q_INVOKABLE int paste(int fromPosition = -1);

    Q_PROPERTY(bool applyFormattingEvenInTransaction READ isApplyFormattingEvenInTransaction WRITE
                       setApplyFormattingEvenInTransaction NOTIFY
                               applyFormattingEvenInTransactionChanged)
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
    qreal m_textWidth = 0;
    int m_cursorPosition = -1;
    int m_selectionEndPosition = -1;
    int m_selectionStartPosition = -1;
    bool m_applyTextFormat = false;
    bool m_acceptTextFormatChanges = true;
    bool m_pastingContent = false;
    int m_documentLoadCount = 0;
    TextFormat *m_textFormat = new TextFormat(this);
    bool m_sceneIsBeingReset = false;
    bool m_sceneIsBeingRefreshed = false;
    bool m_sceneElementTaskIsRunning = false;
    bool m_forceSyncDocument = false;
    bool m_spellCheckEnabled = true;
    bool m_applyLanguageFonts = false;
    QString m_completionPrefix;
    int m_completionPrefixEnd = -1;
    int m_completionPrefixStart = -1;
    CompletionMode m_completionMode = NoCompletionMode;
    bool m_initializingDocument = false;
    QStringList m_shots;
    QStringList m_transitions;
    QStringList m_characterNames;
    bool m_liveSpellCheckEnabled = true;
    bool m_autoCapitalizeSentences = true;
    bool m_autoPolishParagraphs = true;
    QObjectProperty<Scene> m_scene;
    bool m_applyNextCharFormat = false;
    QTextCharFormat m_nextCharFormat;
    ExecLaterTimer m_rehighlightTimer;
    QStringList m_autoCompleteHints;
    QStringList m_priorityAutoCompleteHints;
    QStringList m_spellingSuggestions;
    int m_currentElementCursorPosition = -1;
    bool m_wordUnderCursorIsMisspelled = false;
    ExecLaterTimer m_initializeDocumentTimer;
    ExecLaterTimer m_sceneElementTaskTimer;
    ExecLaterTimer m_applyBlockFormatTimer;
    QList<SceneElement::Type> m_tabHistory;
    bool m_applyFormattingEvenInTransaction = false;
    QList<QTextBlock> m_applyBlockFormatQueue;
    QList<QTextBlock> m_rehighlightBlockQueue;
    QObjectProperty<SceneElement> m_currentElement;
    QObjectProperty<QQuickTextDocument> m_textDocument;
    QObjectProperty<ScreenplayFormat> m_screenplayFormat;
    QObjectProperty<ScreenplayElement> m_screenplayElement;
    SceneElement::Type m_autoCompleteHintsFor = SceneElement::Action;
};

#endif // FORMATTING_H
