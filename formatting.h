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

#ifndef FORMATTING_H
#define FORMATTING_H

#include "scene.h"
#include "modifiable.h"

#include <QScreen>
#include <QBasicTimer>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QSyntaxHighlighter>
#include <QQuickTextDocument>

class ScreenplayFormat;
class ScriteDocument;

class SceneElementFormat : public QObject, public Modifiable
{
    Q_OBJECT

public:
    ~SceneElementFormat();

    Q_PROPERTY(ScreenplayFormat* format READ format CONSTANT STORED false)
    ScreenplayFormat* format() const { return m_format; }

    Q_PROPERTY(SceneElement::Type elementType READ elementType CONSTANT)
    SceneElement::Type elementType() const { return m_elementType; }
    Q_SIGNAL void elementTypeChanged();

    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged)
    void setFont(const QFont &val);
    QFont &fontRef() { return m_font; }
    QFont font() const { return m_font; }
    Q_SIGNAL void fontChanged();

    Q_INVOKABLE void setFontFamily(const QString &val);
    Q_INVOKABLE void setFontBold(bool val);
    Q_INVOKABLE void setFontItalics(bool val);
    Q_INVOKABLE void setFontUnderline(bool val);
    Q_INVOKABLE void setFontPointSize(int val);

    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    void setTextColor(const QColor &val);
    QColor textColor() const { return m_textColor; }
    Q_SIGNAL void textColorChanged();

    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    void setBackgroundColor(const QColor &val);
    QColor backgroundColor() const { return m_backgroundColor; }
    Q_SIGNAL void backgroundColorChanged();

    Q_PROPERTY(Qt::Alignment textAlignment READ textAlignment WRITE setTextAlignment NOTIFY textAlignmentChanged)
    void setTextAlignment(Qt::Alignment val);
    Qt::Alignment textAlignment() const { return m_textAlignment; }
    Q_SIGNAL void textAlignmentChanged();

    Q_PROPERTY(qreal blockWidth READ blockWidth WRITE setBlockWidth NOTIFY blockWidthChanged)
    void setBlockWidth(qreal val);
    qreal blockWidth() const { return m_blockWidth; }
    Q_SIGNAL void blockWidthChanged();

    Q_PROPERTY(Qt::Alignment blockAlignment READ blockAlignment WRITE setBlockAlignment NOTIFY blockAlignmentChanged)
    void setBlockAlignment(Qt::Alignment val);
    Qt::Alignment blockAlignment() const { return m_blockAlignment; }
    Q_SIGNAL void blockAlignmentChanged();

    Q_PROPERTY(qreal topMargin READ topMargin WRITE setTopMargin NOTIFY topMarginChanged)
    void setTopMargin(qreal val);
    qreal topMargin() const { return m_topMargin; }
    Q_SIGNAL void topMarginChanged();

    Q_PROPERTY(qreal bottomMargin READ bottomMargin WRITE setBottomMargin NOTIFY bottomMarginChanged)
    void setBottomMargin(qreal val);
    qreal bottomMargin() const { return m_bottomMargin; }
    Q_SIGNAL void bottomMarginChanged();

    Q_PROPERTY(qreal lineHeight READ lineHeight WRITE setLineHeight NOTIFY lineHeightChanged)
    void setLineHeight(qreal val);
    qreal lineHeight() const { return m_lineHeight; }
    Q_SIGNAL void lineHeightChanged();

    QTextBlockFormat createBlockFormat(const qreal *pageWidth=nullptr) const;
    QTextCharFormat createCharFormat(const qreal *pageWidth=nullptr) const;

    Q_SIGNAL void elementFormatChanged();

    enum Properties
    {
        FontFamily,
        FontSize,
        FontStyle,
        LineHeight,
        TextAndBackgroundColors,
        TextAlignment,
        BlockWidth,
        BlockAlignment,
        Margins
    };
    Q_ENUM(Properties)
    Q_INVOKABLE void applyToAll(Properties properties);

private:
    friend class ScreenplayFormat;
    SceneElementFormat(SceneElement::Type type=SceneElement::Action, ScreenplayFormat *parent=nullptr);

private:
    QFont m_font;
    qreal m_topMargin = 10;
    qreal m_blockWidth = 1.0;
    qreal m_lineHeight = 1.0;
    QColor m_textColor = QColor(Qt::black);
    qreal m_bottomMargin = 0;
    QColor m_backgroundColor = QColor(Qt::transparent);
    ScreenplayFormat *m_format = nullptr;
    Qt::Alignment m_textAlignment = Qt::AlignLeft;
    Qt::Alignment m_blockAlignment = Qt::AlignHCenter;
    SceneElement::Type m_elementType = SceneElement::Action;
};

class ScreenplayFormat : public QAbstractListModel, public Modifiable
{
    Q_OBJECT

public:
    ScreenplayFormat(QObject *parent=nullptr);
    ~ScreenplayFormat();

    Q_PROPERTY(ScriteDocument* scriteDocument READ scriteDocument CONSTANT STORED false)
    ScriteDocument* scriteDocument() const { return m_scriteDocument; }

    Q_PROPERTY(QScreen* screen READ screen WRITE setScreen NOTIFY screenChanged STORED false)
    void setScreen(QScreen* val);
    QScreen* screen() const { return m_screen; }
    Q_SIGNAL void screenChanged();

    Q_PROPERTY(qreal pageWidth READ pageWidth NOTIFY screenChanged)
    qreal pageWidth() const { return m_pageWidth; }

    Q_PROPERTY(QFont defaultFont READ defaultFont WRITE setDefaultFont NOTIFY defaultFontChanged)
    void setDefaultFont(const QFont &val);
    QFont defaultFont() const { return m_defaultFont; }
    QFont &defaultFontRef() { return m_defaultFont; }
    Q_SIGNAL void defaultFontChanged();

    Q_PROPERTY(int fontPointSizeDelta READ fontPointSizeDelta NOTIFY defaultFontChanged)
    int fontPointSizeDelta() const { return m_fontPointSizeDelta; }

    Q_INVOKABLE SceneElementFormat *elementFormat(SceneElement::Type type) const;
    Q_INVOKABLE SceneElementFormat *elementFormat(int type) const;
    Q_SIGNAL void formatChanged();

    Q_PROPERTY(QQmlListProperty<SceneElementFormat> elementFormats READ elementFormats)
    QQmlListProperty<SceneElementFormat> elementFormats();

    void applyToAll(const SceneElementFormat *from, SceneElementFormat::Properties properties);

    enum Role { SceneElementFomat=Qt::UserRole };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE void resetToDefaults();

private:
    char  m_padding[4];
    QFont m_defaultFont;
    qreal m_pageWidth = 750.0;
    int   m_fontPointSizeDelta = 0;
    QScreen* m_screen = nullptr;
    QStringList m_suggestionsAtCursor;
    ScriteDocument *m_scriteDocument = nullptr;

    static SceneElementFormat* staticElementFormatAt(QQmlListProperty<SceneElementFormat> *list, int index);
    static int staticElementFormatCount(QQmlListProperty<SceneElementFormat> *list);
    QList<SceneElementFormat*> m_elementFormats;
};

class SceneDocumentBinder : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    SceneDocumentBinder(QObject *parent=nullptr);
    ~SceneDocumentBinder();

    Q_PROPERTY(ScreenplayFormat* screenplayFormat READ screenplayFormat WRITE setScreenplayFormat NOTIFY screenplayFormatChanged)
    void setScreenplayFormat(ScreenplayFormat* val);
    ScreenplayFormat* screenplayFormat() const { return m_screenplayFormat; }
    Q_SIGNAL void screenplayFormatChanged();

    Q_PROPERTY(Scene* scene READ scene WRITE setScene NOTIFY sceneChanged)
    void setScene(Scene* val);
    Scene* scene() const { return m_scene; }
    Q_SIGNAL void sceneChanged();

    Q_PROPERTY(QQuickTextDocument* textDocument READ textDocument WRITE setTextDocument NOTIFY textDocumentChanged)
    void setTextDocument(QQuickTextDocument* val);
    QQuickTextDocument* textDocument() const { return m_textDocument; }
    Q_SIGNAL void textDocumentChanged();

    Q_PROPERTY(qreal textWidth READ textWidth WRITE setTextWidth NOTIFY textWidthChanged)
    void setTextWidth(qreal val);
    qreal textWidth() const { return m_textWidth; }
    Q_SIGNAL void textWidthChanged();

    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY cursorPositionChanged)
    void setCursorPosition(int val);
    int cursorPosition() const { return m_cursorPosition; }
    Q_SIGNAL void cursorPositionChanged();

    Q_SIGNAL void requestCursorPosition(int position);

    Q_PROPERTY(QStringList characterNames READ characterNames WRITE setCharacterNames NOTIFY characterNamesChanged)
    void setCharacterNames(const QStringList &val);
    QStringList characterNames() const { return m_characterNames; }
    Q_SIGNAL void characterNamesChanged();

    Q_PROPERTY(SceneElement* currentElement READ currentElement NOTIFY currentElementChanged)
    SceneElement* currentElement() const { return m_currentElement; }
    Q_SIGNAL void currentElementChanged();

    Q_PROPERTY(int currentElementCursorPosition READ currentElementCursorPosition NOTIFY cursorPositionChanged)
    int currentElementCursorPosition() const { return m_currentElementCursorPosition; }

    Q_PROPERTY(bool forceSyncDocument READ isForceSyncDocument WRITE setForceSyncDocument NOTIFY forceSyncDocumentChanged)
    void setForceSyncDocument(bool val);
    bool isForceSyncDocument() const { return m_forceSyncDocument; }
    Q_SIGNAL void forceSyncDocumentChanged();

    Q_INVOKABLE void tab();
    Q_INVOKABLE void backtab();
    Q_INVOKABLE bool canGoUp();
    Q_INVOKABLE bool canGoDown();

    Q_INVOKABLE int lastCursorPosition() const;

    Q_INVOKABLE int cursorPositionAtBlock(int blockNumber) const;

    Q_PROPERTY(QStringList autoCompleteHints READ autoCompleteHints NOTIFY autoCompleteHintsChanged)
    QStringList autoCompleteHints() const { return m_autoCompleteHints; }
    Q_SIGNAL void autoCompleteHintsChanged();

    Q_PROPERTY(QString completionPrefix READ completionPrefix NOTIFY completionPrefixChanged)
    QString completionPrefix() const { return m_completionPrefix; }
    Q_SIGNAL void completionPrefixChanged();

    Q_PROPERTY(QFont currentFont READ currentFont NOTIFY currentFontChanged)
    QFont currentFont() const;
    Q_SIGNAL void currentFontChanged();

    Q_PROPERTY(int documentLoadCount READ documentLoadCount NOTIFY documentLoadCountChanged)
    int documentLoadCount() const { return m_documentLoadCount; }
    Q_SIGNAL void documentLoadCountChanged();

    Q_SIGNAL void documentInitialized();

protected:
    // QSyntaxHighlighter interface
    void highlightBlock(const QString &text);

    // QObject interface
    void timerEvent(QTimerEvent *te);

private:
    void initializeDocument();
    void initializeDocumentLater();
    void setDocumentLoadCount(int val);
    void setCurrentElement(SceneElement* val);
    void onSceneElementChanged(SceneElement *element, Scene::SceneElementChangeType type);
    void onContentsChange(int from, int charsRemoved, int charsAdded);
    void syncSceneFromDocument(int nrBlocks=-1);
    bool eventFilter(QObject *object, QEvent *event);

    void evaluateAutoCompleteHints();
    void setAutoCompleteHints(const QStringList &val);
    void setCompletionPrefix(const QString &val);

    void onSceneAboutToReset();
    void onSceneReset(int position);

private:
    Scene* m_scene = nullptr;
    qreal m_textWidth = 0;
    int m_cursorPosition = -1;
    int m_documentLoadCount = 0;
    bool m_sceneIsBeingReset = false;
    bool m_forceSyncDocument = false;
    QString m_completionPrefix;
    bool m_initializingDocument = false;
    QStringList m_characterNames;
    SceneElement* m_currentElement = nullptr;
    QStringList m_autoCompleteHints;
    int m_currentElementCursorPosition = -1;
    QQuickTextDocument* m_textDocument = nullptr;
    ScreenplayFormat* m_screenplayFormat = nullptr;
    QBasicTimer m_initializeDocumentTimer;
    QList<SceneElement::Type> m_tabHistory;
};

#endif // FORMATTING_H
