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

#include "textdocumentitem.h"

#include <QtMath>
#include <QImage>
#include <QTimer>
#include <QPainter>
#include <QQuickWindow>
#include <QQuickPaintedItem>
#include <QAbstractTextDocumentLayout>

class TextDocumentViewportItem : public QQuickPaintedItem
{
public:
    explicit TextDocumentViewportItem(TextDocumentItem *parent);
    ~TextDocumentViewportItem();

    void setViewportImage(const QImage &image)
    {
        m_image = image;
        this->update();
    }
    QImage viewportImage() const { return m_image; }

    // QQuickPaintedItem interface
    void paint(QPainter *painter);

private:
    QImage m_image;
};

TextDocumentItem::TextDocumentItem(QQuickItem *parent) : QQuickItem(parent)
{
    this->setFlag(ItemHasContents, false);

    m_documentChangeHandler = new QTimer(this);
    m_documentChangeHandler->setInterval(0);
    m_documentChangeHandler->setSingleShot(true);
    connect(m_documentChangeHandler, &QTimer::timeout, this, &TextDocumentItem::onDocumentChanged);

    m_viewportUpdateHandler = new QTimer(this);
    m_viewportUpdateHandler->setInterval(0);
    m_viewportUpdateHandler->setSingleShot(true);
    connect(m_viewportUpdateHandler, &QTimer::timeout, this, &TextDocumentItem::updateViewport);

    m_viewportItem = new TextDocumentViewportItem(this);
    m_viewportItem->setVisible(false);
}

TextDocumentItem::~TextDocumentItem() { }

void TextDocumentItem::setDocument(QTextDocument *val)
{
    if (m_document == val)
        return;

    if (m_document) {
        m_document->disconnect(m_documentChangeHandler);
        if (m_document->parent() == this)
            m_document->deleteLater();
    }

    m_document = val;
    emit documentChanged();

    if (m_document)
        connect(m_document, SIGNAL(contentsChanged()), m_documentChangeHandler, SLOT(start()));

    m_documentChangeHandler->start();
}

void TextDocumentItem::setDocumentScale(qreal val)
{
    if (qFuzzyCompare(m_documentScale, val))
        return;

    m_documentScale = qBound(0.1, val, 20.0);
    emit documentScaleChanged();

    m_documentChangeHandler->start();
}

void TextDocumentItem::setFlickable(QQuickItem *val)
{
    if (m_flickable == val)
        return;

    if (m_flickable)
        m_flickable->disconnect(this);

    m_flickable = val;
    emit flickableChanged();

    if (m_flickable) {
        connect(m_flickable, SIGNAL(contentYChanged()), m_viewportUpdateHandler, SLOT(start()));
        connect(m_flickable, SIGNAL(widthChanged()), m_viewportUpdateHandler, SLOT(start()));
        connect(m_flickable, SIGNAL(heightChanged()), m_viewportUpdateHandler, SLOT(start()));
    }

    m_viewportUpdateHandler->start();
}

void TextDocumentItem::setVerticalPadding(qreal val)
{
    if (qFuzzyCompare(m_verticalPadding, val))
        return;

    m_verticalPadding = val;
    emit verticalPaddingChanged();

    m_viewportUpdateHandler->start();
}

void TextDocumentItem::setInvertColors(bool val)
{
    if (m_invertColors == val)
        return;

    m_invertColors = val;
    emit invertColorsChanged();

    m_viewportUpdateHandler->start();
}

void TextDocumentItem::updateViewport()
{
    if (m_document == nullptr) {
        m_viewportItem->setVisible(false);
        return;
    }

    const qreal maxViewportDim = 4000.0 / m_documentScale;

    const qreal contentY =
            (m_flickable == nullptr ? 0 : m_flickable->property("contentY").toDouble())
            - m_verticalPadding;

    const qreal x = 0;
    const qreal y = contentY / m_documentScale;
    const qreal width = qMin(m_document->textWidth(), maxViewportDim);
    const qreal height = (qMin(m_flickable == nullptr ? this->height()
                                                      : (m_flickable->height() + m_verticalPadding),
                               maxViewportDim))
            / m_documentScale;
    const QRectF rect(x, y, width, height);
    if (rect.isEmpty()) {
        m_viewportItem->setVisible(false);
        return;
    }

    const qreal dpr = this->window() ? this->window()->devicePixelRatio() : 1.0;
    const QSizeF viewportSize(width * m_documentScale, height * m_documentScale);

    QImage image((viewportSize * dpr).toSize(), QImage::Format_ARGB32);
    image.setDevicePixelRatio(dpr);
    image.fill(Qt::transparent);

    QPainter paint;
    paint.begin(&image);
    paint.scale(m_documentScale, m_documentScale);
    paint.translate(-x, -y);
    paint.setRenderHint(QPainter::Antialiasing);
    paint.setRenderHint(QPainter::TextAntialiasing);

    QAbstractTextDocumentLayout *layout = m_document->documentLayout();

    QAbstractTextDocumentLayout::PaintContext ctx;
    ctx.clip = rect;
    layout->draw(&paint, ctx);

    paint.end();

    m_viewportItem->setViewportImage(image);

    m_viewportItem->setX(qMax((this->width() - viewportSize.width()) / 2, 0.0));
    m_viewportItem->setY(contentY);
    m_viewportItem->setWidth(viewportSize.width());
    m_viewportItem->setHeight(viewportSize.height());
    m_viewportItem->setVisible(true);
}

void TextDocumentItem::onDocumentChanged()
{
    if (m_document == nullptr) {
        this->setWidth(0);
        this->setHeight(0);
        m_viewportItem->setVisible(false);
        return;
    }

    if (qFuzzyIsNull(m_document->textWidth()))
        m_document->setTextWidth(this->width());
    this->setHeight(qCeil(m_document->size().height() * m_documentScale));
    this->updateViewport();
}

TextDocumentViewportItem::TextDocumentViewportItem(TextDocumentItem *parent)
    : QQuickPaintedItem(parent)
{
}

TextDocumentViewportItem::~TextDocumentViewportItem() { }

void TextDocumentViewportItem::paint(QPainter *painter)
{
    painter->drawImage(0, 0, m_image);
}
