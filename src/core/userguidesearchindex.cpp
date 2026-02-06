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

#include "userguidesearchindex.h"
#include "restapicall.h"
#include "localstorage.h"
#include "networkaccessmanager.h"
#include "utils.h"

#include <QTimer>
#include <QTextDocument>
#include <QNetworkReply>
#include <QJsonDocument>

static const char *userGuideBaseUrl = "userGuideBaseUrl";
static const char *userGuideSortOrder = "userGuideSortOrder";
static const char *userGuideSearchIndex = "userGuideSearchIndex";

UserGuideSearchIndex::UserGuideSearchIndex(QObject *parent) : QAbstractListModel(parent)
{
    QTimer::singleShot(0, this, &UserGuideSearchIndex::checkSearchIndexForUpdates);
}

UserGuideSearchIndex::~UserGuideSearchIndex() { }

int UserGuideSearchIndex::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_items.size();
}

QVariant UserGuideSearchIndex::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const Item &item = m_items[index.row()];
    switch (role) {
    case LocationRole:
        return item.location;
    case TitleRole:
        return item.title;
    case FullTitleRole:
        return item.fullTitle;
    case RichTextRole:
        return item.richText;
    case PlainTextRole:
        return item.plainText;
    }

    return QVariant();
}

QHash<int, QByteArray> UserGuideSearchIndex::roleNames() const
{
    return { { LocationRole, QByteArrayLiteral("location") },
             { TitleRole, QByteArrayLiteral("title") },
             { FullTitleRole, QByteArrayLiteral("fullTitle") },
             { RichTextRole, QByteArrayLiteral("richText") },
             { PlainTextRole, QByteArrayLiteral("plainText") } };
}

void UserGuideSearchIndex::loadSearchIndex()
{
    this->beginResetModel();

    m_items.clear();

    const QString baseUrl = LocalStorage::load(userGuideBaseUrl).toString();
    const QVariant searchIndexVariant = LocalStorage::load(userGuideSearchIndex);
    const QJsonArray searchIndex = searchIndexVariant.value<QJsonArray>();

    QString chapterName;

    const QString userGuide = QStringLiteral("User Guide");
    const QString fullTitleSeparator = " » ";
    const QStringList sortOrder = LocalStorage::load(userGuideSortOrder).toStringList();

    int index = 0;

    for (const QJsonValue &searchItem : searchIndex) {
        const QJsonObject record = searchItem.toObject();

        const QString recordLocation = record.value("location").toString();

        Item item;
        item.location = QUrl(baseUrl + recordLocation);
        item.title = record.value("title").toString();
        item.richText = record.value("text").toString();

        if (!item.location.hasFragment())
            chapterName = item.title;

        if (item.title == chapterName)
            item.fullTitle = userGuide + fullTitleSeparator + item.title;
        else
            item.fullTitle = chapterName + fullTitleSeparator + item.title;

        QTextDocument doc;
        doc.setHtml(item.richText);
        item.plainText = doc.toPlainText();

        item.index = index++;
        if (recordLocation.isEmpty() || recordLocation.startsWith("#"))
            item.sortOrder = 0;
        else {
            for (int i = 0; i < sortOrder.size(); i++) {
                if (recordLocation.startsWith(sortOrder.at(i))) {
                    item.sortOrder = i + 1;
                    break;
                }
            }
        }

        m_items.append(item);
    }

    std::sort(m_items.begin(), m_items.end(), [](const Item &a, const Item &b) {
        if (a.sortOrder != b.sortOrder)
            return a.sortOrder < b.sortOrder;
        return a.index < b.index;
    });

    this->endResetModel();
}

void UserGuideSearchIndex::checkSearchIndexForUpdates()
{
    AppUserGuideSearchIndexRestApiCall *call =
            this->findChild<AppUserGuideSearchIndexRestApiCall *>(QString(),
                                                                  Qt::FindDirectChildrenOnly);
    if (call && call->isBusy())
        return;

    m_busy = true;
    emit busyChanged();

    call = new AppUserGuideSearchIndexRestApiCall(this);
    connect(call, &AppUserGuideSearchIndexRestApiCall::finished, this,
            &UserGuideSearchIndex::onSearchIndexChecked);
    call->call();
}

void UserGuideSearchIndex::onSearchIndexChecked()
{
    AppUserGuideSearchIndexRestApiCall *call =
            qobject_cast<AppUserGuideSearchIndexRestApiCall *>(this->sender());
    if (call == nullptr)
        return;

    LocalStorage::store(userGuideBaseUrl, call->userGuideBaseUrl());
    LocalStorage::store(userGuideSortOrder, call->userGuideSortOrder());

    if (call->isUpdateRequired())
        this->downloadSearchIndex(call->userGuideIndexUrl());
    else {
        this->loadSearchIndex();

        m_busy = false;
        emit busyChanged();
    }

    call->deleteLater();
}

void UserGuideSearchIndex::downloadSearchIndex(const QUrl &indexUrl)
{
    QNetworkReply *reply = NetworkAccessManager::instance()->get(QNetworkRequest(indexUrl));
    connect(reply, &QNetworkReply::finished, this, &UserGuideSearchIndex::onSearchIndexDownloaded);
}

void UserGuideSearchIndex::onSearchIndexDownloaded()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(this->sender());
    if (reply == nullptr)
        return;

    const QByteArray data = reply->readAll();

    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        return;
    }

    const QJsonObject json = doc.object();
    const QJsonArray array = json.value("docs").toArray();

    LocalStorage::store(userGuideSearchIndex, json.value("docs").toArray());

    reply->deleteLater();

    this->loadSearchIndex();

    m_busy = false;
    emit busyChanged();
}

///////////////////////////////////////////////////////////////////////////////

UserGuideSearchIndexFilter::UserGuideSearchIndexFilter(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    this->setSourceModel(new UserGuideSearchIndex(this));
    this->setDynamicSortFilter(true);
    this->setSortRole(UserGuideSearchIndex::FullTitleRole);
    this->sort(0, Qt::AscendingOrder);
}

UserGuideSearchIndexFilter::~UserGuideSearchIndexFilter() { }

void UserGuideSearchIndexFilter::setFilter(const QString &val)
{
    if (m_filter == val)
        return;

    m_filter = val;
    m_trimmedFilter = val.trimmed();
    emit filterChanged();

    this->invalidate();
}

QString UserGuideSearchIndexFilter::highlightFilter(const QString &text, const QString &filter,
                                                    int maxChars)
{
    const QString tfilter = filter.trimmed();
    if (tfilter.isEmpty())
        return text;

    const int index = text.indexOf(tfilter, 0, Qt::CaseInsensitive);
    if (index == -1)
        return text;

    const int startIndex = qMax(0, index - maxChars / 2);
    const int endIndex = qMin(text.length(), index + tfilter.length() + maxChars / 2);

    QString highlightedText = text.mid(startIndex, endIndex - startIndex);
    highlightedText.replace(tfilter, QStringLiteral("<b>%1</b>").arg(tfilter), Qt::CaseInsensitive);

    if (startIndex > 0)
        highlightedText.prepend("...");

    if (endIndex < text.length())
        highlightedText.append("...");

    return highlightedText;
}

bool UserGuideSearchIndexFilter::filterAcceptsRow(int source_row,
                                                  const QModelIndex &source_parent) const
{
    if (this->sourceModel() != nullptr) {
        const QModelIndex source_index = this->sourceModel()->index(source_row, 0, source_parent);
        if (m_trimmedFilter.isEmpty()) {
            const QUrl location = source_index.data(UserGuideSearchIndex::LocationRole).toUrl();
            return !location.hasFragment();
        } else {
            const QString title = source_index.data(UserGuideSearchIndex::FullTitleRole).toString();
            const QString plainText =
                    source_index.data(UserGuideSearchIndex::PlainTextRole).toString();
            return title.contains(m_trimmedFilter, Qt::CaseInsensitive)
                    || plainText.contains(m_trimmedFilter, Qt::CaseInsensitive);
        }
    }

    return true;
}

bool UserGuideSearchIndexFilter::lessThan(const QModelIndex &source_left,
                                          const QModelIndex &source_right) const
{
    if (m_trimmedFilter.isEmpty())
        return QSortFilterProxyModel::lessThan(source_left, source_right);

    const QString titleA = source_left.data(UserGuideSearchIndex::FullTitleRole).toString();
    const QString titleB = source_right.data(UserGuideSearchIndex::FullTitleRole).toString();

    const bool titleAPass = titleA.contains(m_trimmedFilter, Qt::CaseInsensitive);
    const bool titleBPass = titleB.contains(m_trimmedFilter, Qt::CaseInsensitive);

    return (titleAPass && !titleBPass);
}
