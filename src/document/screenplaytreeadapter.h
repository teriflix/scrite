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

#ifndef SCREENPLAYTREEADAPTER_H
#define SCREENPLAYTREEADAPTER_H

#include "screenplay.h"

class ScreenplayTreeAdapter : public QAbstractItemModel, public QQmlParserStatus
{
    Q_OBJECT
    QML_ELEMENT
    Q_INTERFACES(QQmlParserStatus)

public:
    explicit ScreenplayTreeAdapter(QObject *parent = nullptr);
    virtual ~ScreenplayTreeAdapter();

    enum ItemType { UnknownType, EpisodeType, ActType, FormalTagType, OpenTagType, SceneType };
    Q_ENUM(ItemType)

    // clang-format off
    Q_PROPERTY(bool includeFormalTags
               READ isIncludeFormalTags
               WRITE setIncludeFormalTags
               NOTIFY includeFormalTagsChanged)
    // clang-format on
    void setIncludeFormalTags(bool val);
    bool isIncludeFormalTags() const { return m_includeFormalTags; }
    Q_SIGNAL void includeFormalTagsChanged();

    // clang-format off
    Q_PROPERTY(bool includeOpenTags
               READ isIncludeOpenTags
               WRITE setIncludeOpenTags
               NOTIFY includeOpenTagsChanged)
    // clang-format on
    void setIncludeOpenTags(bool val);
    bool isIncludeOpenTags() const { return m_includeOpenTags; }
    Q_SIGNAL void includeOpenTagsChanged();

    // clang-format off
    Q_PROPERTY(QStringList allowedOpenTags
               READ allowedOpenTags
               WRITE setAllowedOpenTags
               NOTIFY allowedOpenTagsChanged)
    // clang-format on
    void setAllowedOpenTags(const QStringList &val);
    QStringList allowedOpenTags() const { return m_allowedOpenTags; }
    Q_SIGNAL void allowedOpenTagsChanged();

    // clang-format off
    Q_PROPERTY(Screenplay* screenplay
               READ screenplay
               WRITE setScreenplay
               NOTIFY screenplayChanged)
    // clang-format on
    void setScreenplay(Screenplay *val);
    Screenplay *screenplay() const { return m_screenplay; }
    Q_SIGNAL void screenplayChanged();

    // QQmlParserStatus interface
    void classBegin();
    void componentComplete();

    // QAbstractItemModel interface
    enum { TextRole = Qt::DisplayRole, ScreenplayElementRole = Qt::UserRole };
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

private:
    void clear();
    void reload();

private:
    bool m_componentComplete = true;
    Screenplay *m_screenplay = nullptr;

    bool m_includeFormalTags = true;
    bool m_includeOpenTags = true;
    QStringList m_allowedOpenTags;

    struct Item
    {
    public:
        static Item *get(const QModelIndex &index);

        Item(Item *parent = nullptr);
        ~Item();

        ItemType type = ScreenplayTreeAdapter::UnknownType;
        QPointer<ScreenplayElement> element;
        QString text;
        QList<Item *> children;

    private:
        friend class ScreenplayTreeAdapter;
        Item *parent = nullptr;
    };
    friend struct Item;
    QList<Item *> m_items;
};

#endif // SCREENPLAYTREEADAPTER_H
