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

#ifndef USERGUIDESEARCHINDEX_H
#define USERGUIDESEARCHINDEX_H

#include <QQmlEngine>
#include <QJsonArray>
#include <QSortFilterProxyModel>

class UserGuideSearchIndex : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit UserGuideSearchIndex(QObject *parent = nullptr);
    ~UserGuideSearchIndex() override;

    Q_PROPERTY(bool isBusy READ isBusy NOTIFY busyChanged)
    bool isBusy() const { return m_busy; }
    Q_SIGNAL void busyChanged();

    Q_INVOKABLE QUrl find(const QString &searchTerm) const;

    // Extensions own the download and upkeep of their guides. They hand over the records (in the
    // same format as the "docs" array of a MkDocs search index) using addGuide(). A uri that is
    // empty or already added is rejected. To refresh a guide, remove it and add it again.
    Q_INVOKABLE bool addGuide(const QString &uri, const QString &title, const QUrl &baseUrl,
                              const QJsonArray &docs);
    Q_INVOKABLE void removeGuide(const QString &uri);

    enum { LocationRole = Qt::UserRole, TitleRole, FullTitleRole, RichTextRole, PlainTextRole };

    // QAbstractItemModel interface
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

signals:
    void modified();

private:
    void loadSearchIndex();
    void appendRecords(const QString &guide, const QString &guideTitle, const QString &baseUrl,
                       const QJsonArray &docs, const QStringList &sortOrder);
    void sortItems();

    void checkSearchIndexForUpdates();
    void onSearchIndexChecked();

    void downloadSearchIndex(const QUrl &indexUrl);
    void onSearchIndexDownloaded();

private:
    struct Item
    {
        QString guide; // uri of the guide this item belongs to, empty for the primary guide
        QUrl location;
        QString title;
        QString fullTitle;
        QString richText;
        QString plainText;

        int index = -1;
        int sortOrder = -1;
    };
    QList<Item> m_items;
    QStringList m_guides; // uris of added guides, in the order they were added

    bool m_busy = false;
    QString m_filter;
};

class UserGuideSearchIndexFilter : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit UserGuideSearchIndexFilter(QObject *parent = nullptr);
    ~UserGuideSearchIndexFilter() override;

    // clang-format off
    Q_PROPERTY(UserGuideSearchIndex* userGuide
               READ userGuide
               CONSTANT)
    // clang-format on
    UserGuideSearchIndex *userGuide() const;

    // clang-format off
    Q_PROPERTY(QString filter
               READ filter
               WRITE setFilter
               NOTIFY filterChanged)
    // clang-format on
    void setFilter(const QString &val);
    QString filter() const { return m_filter; }
    Q_SIGNAL void filterChanged();

    Q_INVOKABLE QUrl find(const QString &searchTerm) const;

    Q_INVOKABLE static QString highlightFilter(const QString &text, const QString &filter,
                                               int maxChars = 120);

protected:
    // QSortFilterProxyModel interface
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

private:
    QString m_filter, m_trimmedFilter;
};

#endif // USERGUIDESEARCHINDEX_H
