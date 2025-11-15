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

#ifndef QTEXTDOCUMENTPAGEDPRINTER_H
#define QTEXTDOCUMENTPAGEDPRINTER_H

#include <QColor>
#include <QEvent>
#include <QObject>
#include <QTextDocument>
#include <QPagedPaintDevice>

#include "errorreport.h"
#include "progressreport.h"

class QTextDocumentPagedPrinter;

class HeaderFooter : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    enum Type { Header, Footer };
    Q_ENUM(Type)

    explicit HeaderFooter(Type type, QObject *parent = nullptr);
    ~HeaderFooter();

    // clang-format off
    Q_PROPERTY(Type type
               READ type
               CONSTANT )
    // clang-format on
    Type type() const { return m_type; }

    enum Field {
        Nothing,

        Title, // query QTextDocument::property("#title") for this.
        Author, // query QTextDocument::property("#author") for this.
        Contact, // query QTextDocument::property("#contact") for this.
        Version, // query QTextDocument::property("#version") for this.
        Subtitle, // query QTextDocument::property("#subtitle") for this.
        Phone, // query QTextDocument::property("#phone") for this.
        Email, // query QTextDocument::property("#email") for this.
        Website, // query QTextDocument::property("#website") for this.

        AppName,
        AppVersion,

        Date,
        Time,
        DateTime,

        Comment,
        Watermark,

        PageNumber,
        PageNumberOfCount
    };
    Q_ENUM(Field)

    // clang-format off
    Q_PROPERTY(Field left
               READ left
               WRITE setLeft
               NOTIFY leftChanged)
    // clang-format on
    void setLeft(Field val);
    Field left() const { return m_left; }
    Q_SIGNAL void leftChanged();

    // clang-format off
    Q_PROPERTY(Field center
               READ center
               WRITE setCenter
               NOTIFY centerChanged)
    // clang-format on
    void setCenter(Field val);
    Field center() const { return m_center; }
    Q_SIGNAL void centerChanged();

    // clang-format off
    Q_PROPERTY(Field right
               READ right
               WRITE setRight
               NOTIFY rightChanged)
    // clang-format on
    void setRight(Field val);
    Field right() const { return m_right; }
    Q_SIGNAL void rightChanged();

    // clang-format off
    Q_PROPERTY(QFont font
               READ font
               WRITE setFont
               NOTIFY fontChanged)
    // clang-format on
    void setFont(const QFont &val);
    QFont font() const { return m_font; }
    Q_SIGNAL void fontChanged();

    // clang-format off
    Q_PROPERTY(qreal opacity
               READ opacity
               WRITE setOpacity
               NOTIFY opacityChanged)
    // clang-format on
    void setOpacity(qreal val);
    qreal opacity() const { return m_opacity; }
    Q_SIGNAL void opacityChanged();

    // clang-format off
    Q_PROPERTY(bool visibleFromPageOne
               READ isVisibleFromPageOne
               WRITE setVisibleFromPageOne
               NOTIFY visibleFromPageOneChanged)
    // clang-format on
    void setVisibleFromPageOne(bool val);
    bool isVisibleFromPageOne() const { return m_visibleFromPageOne; }
    Q_SIGNAL void visibleFromPageOneChanged();

    // clang-format off
    Q_PROPERTY(QRectF rect
               READ rect
               WRITE setRect
               NOTIFY rectChanged)
    // clang-format on
    void setRect(const QRectF &val);
    QRectF rect() const { return m_rect; }
    Q_SIGNAL void rectChanged();

    void prepare(const QMap<Field, QString> &fieldValues, const QRectF &rect, QPaintDevice *pd);
    void paint(QPainter *paint, const QRectF &, int pageNr, int pageCount);
    void finish();

private:
    friend class QTextDocumentPagedPrinter;
    Type m_type = Header;
    QFont m_font;
    Field m_left = Nothing;
    Field m_center = Nothing;
    Field m_right = Nothing;
    bool m_visibleFromPageOne = false;

    struct ColumnContent
    {
        QRectF columnRect;
        QString content;
        int flags;
    };
    QVector<ColumnContent> m_columns;
    qreal m_opacity = 0.5;
    QRectF m_rect;
};

class Watermark : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    explicit Watermark(QObject *parent = nullptr);
    ~Watermark();

    // clang-format off
    Q_PROPERTY(bool enabled
               READ isEnabled
               WRITE setEnabled
               NOTIFY enabledChanged)
    // clang-format on
    void setEnabled(bool val);
    bool isEnabled() const { return m_enabled; }
    Q_SIGNAL void enabledChanged();

    // clang-format off
    Q_PROPERTY(QString text
               READ text
               WRITE setText
               NOTIFY textChanged)
    // clang-format on
    void setText(const QString &val);
    QString text() const { return m_text; }
    Q_SIGNAL void textChanged();

    // clang-format off
    Q_PROPERTY(QFont font
               READ font
               WRITE setFont
               NOTIFY fontChanged)
    // clang-format on
    void setFont(const QFont &val);
    QFont font() const { return m_font; }
    Q_SIGNAL void fontChanged();

    // clang-format off
    Q_PROPERTY(QColor color
               READ color
               WRITE setColor
               NOTIFY colorChanged)
    // clang-format on
    void setColor(const QColor &val);
    QColor color() const { return m_color; }
    Q_SIGNAL void colorChanged();

    // clang-format off
    Q_PROPERTY(qreal opacity
               READ opacity
               WRITE setOpacity
               NOTIFY opacityChanged)
    // clang-format on
    void setOpacity(qreal val);
    qreal opacity() const { return m_opacity; }
    Q_SIGNAL void opacityChanged();

    // clang-format off
    Q_PROPERTY(qreal rotation
               READ rotation
               WRITE setRotation
               NOTIFY rotationChanged)
    // clang-format on
    void setRotation(qreal val);
    qreal rotation() const { return m_rotation; }
    Q_SIGNAL void rotationChanged();

    // clang-format off
    Q_PROPERTY(Qt::Alignment alignment
               READ alignment
               WRITE setAlignment
               NOTIFY alignmentChanged)
    // clang-format on
    void setAlignment(Qt::Alignment val);
    Qt::Alignment alignment() const { return m_alignment; }
    Q_SIGNAL void alignmentChanged();

    // clang-format off
    Q_PROPERTY(bool visibleFromPageOne
               READ isVisibleFromPageOne
               WRITE setVisibleFromPageOne
               NOTIFY visibleFromPageOneChanged)
    // clang-format on
    void setVisibleFromPageOne(bool val);
    bool isVisibleFromPageOne() const { return m_visibleFromPageOne; }
    Q_SIGNAL void visibleFromPageOneChanged();

    // clang-format off
    Q_PROPERTY(QRectF rect
               READ rect
               WRITE setRect
               NOTIFY rectChanged)
    // clang-format on
    void setRect(const QRectF &val);
    QRectF rect() const { return m_rect; }
    Q_SIGNAL void rectChanged();

    void paint(QPainter *paint, const QRectF &pageRect, int pageNr, int pageCount);

private:
    QFont m_font = QFont("Courier Prime", 120, QFont::Bold);
    QString m_text = QLatin1String("Scrite");
    QColor m_color = QColor(Qt::lightGray);
    qreal m_opacity = 0.375;
    qreal m_rotation = -45;
    Qt::Alignment m_alignment = Qt::AlignCenter;
    bool m_enabled = false;
    bool m_visibleFromPageOne = false;
    QRectF m_rect;
};

class QTextDocumentPageSideBarInterface
{
public:
    enum Side { LeftSide, RightSide };
    virtual void paint(QPainter *paint, Side side, const QRectF &rect, const QRectF &docRect) = 0;
};

class QTextDocumentPagedPrinter : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    explicit QTextDocumentPagedPrinter(QObject *parent = nullptr);
    ~QTextDocumentPagedPrinter();

    // clang-format off
    Q_PROPERTY(HeaderFooter *header
               READ header
               CONSTANT )
    // clang-format on
    HeaderFooter *header() const { return m_header; }

    // clang-format off
    Q_PROPERTY(HeaderFooter *footer
               READ footer
               CONSTANT )
    // clang-format on
    HeaderFooter *footer() const { return m_footer; }

    // clang-format off
    Q_PROPERTY(Watermark *watermark
               READ watermark
               CONSTANT )
    // clang-format on
    Watermark *watermark() const { return m_watermark; }

    void setSideBar(QTextDocumentPageSideBarInterface *val) { m_sideBar = val; }
    QTextDocumentPageSideBarInterface *sideBar() const { return m_sideBar; }

    Q_INVOKABLE bool print(QTextDocument *document, QPagedPaintDevice *device);

    static void loadSettings(HeaderFooter *header, HeaderFooter *footer, Watermark *watermark);

private:
    void printPageContents(int pageNr, int pageCount, QPainter *painter, const QTextDocument *doc,
                           const QRectF &body, QRectF &docPageRect);
    void printHeaderFooterWatermark(int pageNr, int pageCount, QPainter *painter,
                                    const QTextDocument *doc, const QRectF &body,
                                    const QRectF &docPageRect);

private:
    HeaderFooter *m_header = new HeaderFooter(HeaderFooter::Header, this);
    HeaderFooter *m_footer = new HeaderFooter(HeaderFooter::Footer, this);
    Watermark *m_watermark = new Watermark(this);
    ErrorReport *m_errorReport = new ErrorReport(this);
    ProgressReport *m_progressReport = new ProgressReport(this);
    QPagedPaintDevice *m_printer = nullptr;
    QTextDocument *m_textDocument = nullptr;
    QTextDocumentPageSideBarInterface *m_sideBar = nullptr;
    QRectF m_headerRect;
    QRectF m_footerRect;
};

#endif // QTEXTDOCUMENTPAGEDPRINTER_H
