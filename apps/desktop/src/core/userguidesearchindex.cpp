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

    connect(this, &UserGuideSearchIndex::modelReset, this, &UserGuideSearchIndex::modified);
    connect(this, &UserGuideSearchIndex::rowsRemoved, this, &UserGuideSearchIndex::modified);
    connect(this, &UserGuideSearchIndex::rowsInserted, this, &UserGuideSearchIndex::modified);
}

UserGuideSearchIndex::~UserGuideSearchIndex()
{
    Utils::ObjectRegistry::remove(this);
}

QUrl UserGuideSearchIndex::find(const QString &searchTerm) const
{
    const QString term = searchTerm.trimmed();
    if (term.isEmpty() || m_items.isEmpty())
        return QUrl();

    const Item *titleMatch = nullptr;
    const Item *textMatch = nullptr;

    for (const Item &item : m_items) {
        if (item.title.compare(term, Qt::CaseInsensitive) == 0)
            return item.location;

        if (!titleMatch && item.title.contains(term, Qt::CaseInsensitive))
            titleMatch = &item;

        if (!textMatch && item.plainText.contains(term, Qt::CaseInsensitive))
            textMatch = &item;
    }

    if (titleMatch)
        return titleMatch->location;

    if (textMatch)
        return textMatch->location;

    return QUrl();
}

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

bool UserGuideSearchIndex::addGuide(const QString &uri, const QString &title, const QUrl &baseUrl,
                                    const QJsonArray &docs)
{
    if (uri.isEmpty() || m_guides.contains(uri))
        return false;

    this->beginResetModel();

    m_guides.append(uri);
    this->appendRecords(uri, title, baseUrl.toString(), docs, QStringList());
    this->sortItems();

    this->endResetModel();

    return true;
}

void UserGuideSearchIndex::removeGuide(const QString &uri)
{
    if (uri.isEmpty() || !m_guides.contains(uri))
        return;

    this->beginResetModel();

    m_guides.removeAll(uri);
    m_items.removeIf([uri](const Item &item) { return item.guide == uri; });

    this->endResetModel();
}

void UserGuideSearchIndex::loadSearchIndex()
{
    this->beginResetModel();

    m_items.removeIf([](const Item &item) { return item.guide.isEmpty(); });

    const QString baseUrl = LocalStorage::load(userGuideBaseUrl).toString();
    const QVariant searchIndexVariant = LocalStorage::load(userGuideSearchIndex);
    const QJsonArray searchIndex = searchIndexVariant.value<QJsonArray>();
    const QStringList sortOrder = LocalStorage::load(userGuideSortOrder).toStringList();

    this->appendRecords(QString(), QStringLiteral("User Guide"), baseUrl, searchIndex, sortOrder);
    this->sortItems();

    this->endResetModel();
}

void UserGuideSearchIndex::appendRecords(const QString &guide, const QString &guideTitle,
                                         const QString &baseUrl, const QJsonArray &docs,
                                         const QStringList &sortOrder)
{
    QString chapterName;

    const QString fullTitleSeparator = " » ";

    int index = 0;

    for (const QJsonValue &searchItem : docs) {
        const QJsonObject record = searchItem.toObject();

        const QString recordLocation = record.value("location").toString();

        Item item;
        item.guide = guide;
        item.location = QUrl(baseUrl + recordLocation);
        item.title = record.value("title").toString();
        item.richText = record.value("text").toString();

        if (!item.location.hasFragment())
            chapterName = item.title;

        if (item.title == chapterName)
            item.fullTitle = guideTitle + fullTitleSeparator + item.title;
        else
            item.fullTitle = chapterName + fullTitleSeparator + item.title;

        QTextDocument doc;
        doc.setHtml(item.richText);
        item.plainText = doc.toPlainText();

        item.index = index++;
        if (recordLocation.isEmpty() || recordLocation.startsWith("#"))
            item.sortOrder = 0;
        else {
            item.sortOrder = sortOrder.size() + 2;
            for (int i = 0; i < sortOrder.size(); i++) {
                if (recordLocation.startsWith(sortOrder.at(i))) {
                    item.sortOrder = i + 1;
                    break;
                }
            }
        }

        m_items.append(item);
    }
}

void UserGuideSearchIndex::sortItems()
{
    std::sort(m_items.begin(), m_items.end(), [this](const Item &a, const Item &b) {
        if (a.guide != b.guide)
            return m_guides.indexOf(a.guide) < m_guides.indexOf(b.guide);
        if (a.sortOrder != b.sortOrder)
            return a.sortOrder < b.sortOrder;
        return a.index < b.index;
    });
}

void UserGuideSearchIndex::checkSearchIndexForUpdates()
{
    Utils::ObjectRegistry::add(this, QStringLiteral("Scrite.App.UserGuideSearchIndex"));

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
    reply->deleteLater();

    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        m_busy = false;
        emit busyChanged();
        return;
    }

    const QJsonObject json = doc.object();

    LocalStorage::store(userGuideSearchIndex, json.value("docs").toArray());

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

UserGuideSearchIndex *UserGuideSearchIndexFilter::userGuide() const
{
    return qobject_cast<UserGuideSearchIndex *>(this->sourceModel());
}

void UserGuideSearchIndexFilter::setFilter(const QString &val)
{
    if (m_filter == val)
        return;

    m_filter = val;
    m_trimmedFilter = val.trimmed();
    emit filterChanged();

    this->invalidate();
}

QUrl UserGuideSearchIndexFilter::find(const QString &searchTerm) const
{
    UserGuideSearchIndex *src = qobject_cast<UserGuideSearchIndex *>(this->sourceModel());
    if (src == nullptr)
        return QUrl();

    return src->find(searchTerm);
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
            if (plainText.isEmpty())
                return false;

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
        return source_left.row() < source_right.row();

    const QString titleA = source_left.data(UserGuideSearchIndex::FullTitleRole).toString();
    const QString titleB = source_right.data(UserGuideSearchIndex::FullTitleRole).toString();

    const bool titleAPass = titleA.contains(m_trimmedFilter, Qt::CaseInsensitive);
    const bool titleBPass = titleB.contains(m_trimmedFilter, Qt::CaseInsensitive);

    return (titleAPass && !titleBPass);
}
