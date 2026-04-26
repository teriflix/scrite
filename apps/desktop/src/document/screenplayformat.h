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

#ifndef SCREENPLAYFORMAT_H
#define SCREENPLAYFORMAT_H

#include "scene.h"
#include "screenplay.h"
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

class ScriteDocument;
class SpellCheckService;
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
    Q_MOC_INCLUDE("screenplayformat.h")
    Q_PROPERTY(ScreenplayFormat *format
               READ format
               CONSTANT
               STORED false)
    // clang-format on
    ScreenplayFormat *format() const { return m_format; }

    // clang-format off
    Q_PROPERTY(SceneElement::Type elementType
               READ elementType
               CONSTANT)
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
    Q_MOC_INCLUDE("scritedocument.h")
    Q_PROPERTY(ScriteDocument *scriteDocument
               READ scriteDocument
               CONSTANT
               STORED false )
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
    Q_SLOT void setScreen(QScreen *val);
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
    Q_PROPERTY(int activeLanguageCode
               READ activeLanguageCode
               WRITE setActiveLanguageCode
               NOTIFY activeLanguageCodeChanged
               STORED false)
    // clang-format on
    void setActiveLanguageCode(int val);
    int activeLanguageCode() const { return m_activeLanguageCode; }
    Q_SIGNAL void activeLanguageCodeChanged();

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

    // clang-format off
    Q_PROPERTY(QColor hyperlinkTextColor
               READ hyperlinkTextColor
               WRITE setHyperlinkTextColor
               NOTIFY hyperlinkTextColorChanged)
    // clang-format on
    void setHyperlinkTextColor(const QColor &val)
    {
        if (m_hyperlinkTextColor == val)
            return;
        m_hyperlinkTextColor = val;
        emit hyperlinkTextColorChanged();
    }
    QColor hyperlinkTextColor() const { return m_hyperlinkTextColor; }
    Q_SIGNAL void hyperlinkTextColorChanged();
    QColor m_hyperlinkTextColor;

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
    int m_activeLanguageCode = QLocale::English;
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
                                                     qsizetype index);
    static qsizetype staticElementFormatCount(QQmlListProperty<SceneElementFormat> *list);
    QList<SceneElementFormat *> m_elementFormats;
};

#endif // SCREENPLAYFORMAT_H
