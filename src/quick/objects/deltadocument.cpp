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

#include "application.h"
#include "deltadocument.h"
#include "execlatertimer.h"

#include <QFutureWatcher>
#include <QtConcurrentRun>

#include <QTextList>
#include <QTextBlock>
#include <QEventLoop>
#include <QTextCursor>
#include <QWebChannel>
#include <QTextDocument>
#include <QWebEnginePage>
#include <QTextBlockFormat>

DeltaDocument::DeltaDocument(QObject *parent) : QObject(parent) { }

DeltaDocument::~DeltaDocument() { }

void DeltaDocument::setContent(const QJsonValue &val)
{
    if (m_content == val)
        return;

    m_content = val;
    emit contentChanged();

    this->transformLater();
}

void DeltaDocument::setHtml(const QString &val)
{
    if (m_html == val)
        return;

    m_html = val;
    emit htmlChanged();
}

void DeltaDocument::setPlainText(const QString &val)
{
    if (m_plainText == val)
        return;

    m_plainText = val;
    emit plainTextChanged();
}

void DeltaDocument::transformNow()
{
    const QJsonObject contentObject = m_content.toObject();

#if 0
    QFutureWatcher<QVariantList> *futureWatcher = new QFutureWatcher<QVariantList>(this);
    connect(futureWatcher, &QFutureWatcher<QVariantList>::finished, this, [=]() {
        const QVariantList result = futureWatcher->result();
        if (result.size() == 3 && result.first().toInt() == m_modificationCounter) {
            this->setPlainText(result.at(1).toString());
            this->setHtml(result.at(2).toString());
        }
        futureWatcher->deleteLater();
    });
    QFuture<QVariantList> future = QtConcurrent::run(&QuillDeltaTransform::transformProc, contentObject,
                                                     ++m_modificationCounter);
    futureWatcher->setFuture(future);
#else
    DeltaDocument::asyncResolve(contentObject, ++m_modificationCounter, this,
                                [=](const ResolveResult &result) {
                                    if (result.callId == m_modificationCounter) {
                                        this->setPlainText(result.plainText);
                                        this->setHtml(result.htmlText);
                                    }
                                });
#endif
}

void DeltaDocument::transformLater()
{
    if (m_content.isString()) {
        ++m_modificationCounter;

        const QString text = m_content.toString();
        const QStringList paras = text.split(QStringLiteral("\n"), Qt::KeepEmptyParts);
        this->setPlainText(text);

        QString html;
        for (const QString &para : paras)
            html += QStringLiteral("<p>") + para + QStringLiteral("</p>");
        this->setHtml(html);
        return;
    }

    const QJsonObject contentObject = m_content.toObject();
    if (contentObject.isEmpty()) {
        ++m_modificationCounter;
        this->setPlainText(QString());
        this->setHtml(QString());
        return;
    }

    ExecLaterTimer::call(
            "QuillDeltaTransform.transform", this, [=]() { this->transformNow(); }, 50);
}

class TransformAttributes : public QObject
{
    Q_OBJECT

public:
    explicit TransformAttributes(QObject *parent = nullptr) : QObject(parent) { }
    ~TransformAttributes() { }

    Q_INVOKABLE QJsonObject getContent() const { return content; }

    Q_INVOKABLE void setTransformedTexts(const QString &_plainText, const QString &_html)
    {
        this->plainText = _plainText;
        this->html = _html;
    }

    Q_INVOKABLE void quit() { emit quitRequest(); }
    Q_SIGNAL void quitRequest();

    QJsonObject content;
    QString plainText;
    QString html;
};

DeltaDocument::ResolveResult DeltaDocument::blockingResolve(const QJsonObject &content, int callId)
{
    TransformAttributes txAttrs;
    txAttrs.content = content;

    QWebChannel webChannel;
    webChannel.registerObject(QStringLiteral("transform"), &txAttrs);

    QEventLoop eventLoop;
    connect(&txAttrs, &TransformAttributes::quitRequest, &eventLoop, &QEventLoop::quit);

    QWebEnginePage webPage;
    webPage.setWebChannel(&webChannel);
    webPage.load(QUrl(QStringLiteral("qrc:/richtexttransform.html")));

    eventLoop.exec();

    return ResolveResult(callId, txAttrs.plainText, txAttrs.html);
}

void DeltaDocument::asyncResolve(const QJsonObject &content, int callId, QObject *receiver,
                                 std::function<void(const ResolveResult &)> function)
{
    TransformAttributes *txAttrs = new TransformAttributes(receiver);
    txAttrs->content = content;

    QWebChannel *webChannel = new QWebChannel(txAttrs);
    webChannel->registerObject(QStringLiteral("transform"), txAttrs);

    QWebEnginePage *webPage = new QWebEnginePage(txAttrs);
    webPage->setWebChannel(webChannel);
    webPage->load(QUrl(QStringLiteral("qrc:/richtexttransform.html")));

    connect(txAttrs, &TransformAttributes::quitRequest, receiver, [=]() {
        const ResolveResult result(callId, txAttrs->plainText, txAttrs->html);
        function(result);
        txAttrs->deleteLater();
    });
}

void DeltaDocument::blockingResolveAndInsertHtml(const QJsonObject &content, QTextCursor &cursor)
{
    const DeltaDocument::ResolveResult result = DeltaDocument::blockingResolve(content);

    if (!result.htmlText.isEmpty()) {
        QTextFrame *rootFrame = cursor.currentFrame();
        const int frameIndent = cursor.blockFormat().indent();

        QTextFrameFormat htmlFrameFormat;
        htmlFrameFormat.setLeftMargin(cursor.document()->indentWidth() * frameIndent);
        cursor.insertFrame(htmlFrameFormat);
        cursor.insertHtml(result.htmlText);
        cursor = rootFrame->lastCursorPosition();
    }
}

#include "deltadocument.moc"
