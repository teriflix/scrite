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

#ifndef USERGUIDESEARCHINDEX_H
#define USERGUIDESEARCHINDEX_H

#include <QQmlEngine>
#include <QSortFilterProxyModel>

class UserGuideSearchIndex : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit UserGuideSearchIndex(QObject *parent = nullptr);
    ~UserGuideSearchIndex();

    Q_PROPERTY(bool isBusy READ isBusy NOTIFY busyChanged)
    bool isBusy() const { return m_busy; }
    Q_SIGNAL void busyChanged();

    enum { LocationRole = Qt::UserRole, TitleRole, FullTitleRole, RichTextRole, PlainTextRole };

    // QAbstractItemModel interface
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

private:
    void loadSearchIndex();

    void checkSearchIndexForUpdates();
    void onSearchIndexChecked();

    void downloadSearchIndex(const QUrl &indexUrl);
    void onSearchIndexDownloaded();

private:
    struct Item
    {
        QUrl location;
        QString title;
        QString fullTitle;
        QString richText;
        QString plainText;

        int index = -1;
        int sortOrder = -1;
    };
    QList<Item> m_items;

    bool m_busy = false;
    QString m_filter;
};

class UserGuideSearchIndexFilter : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit UserGuideSearchIndexFilter(QObject *parent = nullptr);
    ~UserGuideSearchIndexFilter();

    // clang-format off
    Q_PROPERTY(QString filter
               READ filter
               WRITE setFilter
               NOTIFY filterChanged)
    // clang-format on
    void setFilter(const QString &val);
    QString filter() const { return m_filter; }
    Q_SIGNAL void filterChanged();

    Q_INVOKABLE static QString highlightFilter(const QString &text, const QString &filter,
                                               int maxChars = 120);

protected:
    // QSortFilterProxyModel interface
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;

private:
    QString m_filter, m_trimmedFilter;
};

#endif // USERGUIDESEARCHINDEX_H
