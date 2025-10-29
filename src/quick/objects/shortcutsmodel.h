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

// TODO: get rid of this entirely

#ifndef SHORTCUTSMODEL_H
#define SHORTCUTSMODEL_H

#include <QShortcut>
#include <QQmlEngine>
#include <QSortFilterProxyModel>

class ExecLaterTimer;
class ShortcutsModelItem;
class ShortcutsModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    ShortcutsModel(QObject *parent = nullptr);
    ~ShortcutsModel();

    Q_PROPERTY(QStringList groups READ groups WRITE setGroups NOTIFY groupsChanged)
    void setGroups(const QStringList &val);
    QStringList groups() const { return m_groups; }
    Q_SIGNAL void groupsChanged();

    Q_PROPERTY(QString titleFilter READ titleFilter WRITE setTitleFilter NOTIFY titleFilterChanged)
    void setTitleFilter(const QString &val);
    QString titleFilter() const { return m_titleFilter; }
    Q_SIGNAL void titleFilterChanged();

    // QAbstractItemModel interface
    enum Roles {
        TitleRole = Qt::DisplayRole,
        ShortcutRole = Qt::UserRole + 1,
        GroupRole,
        EnabledRole,
        VisibleRole,
        CanActivateRole
    };
    QHash<int, QByteArray> roleNames() const;
    QVariant data(const QModelIndex &index, int role) const;

    Q_INVOKABLE void activateShortcutAt(int row);

    Q_INVOKABLE void printWholeThing();

protected:
    // QSortFilterProxyModel interface
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

private:
    QStringList m_groups;
    QString m_titleFilter;
};

class ShortcutsModelItem : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use as attached property.")
    QML_ATTACHED(ShortcutsModelItem)

public:
    explicit ShortcutsModelItem(QObject *parent = nullptr);
    ~ShortcutsModelItem();

    static ShortcutsModelItem *qmlAttachedProperties(QObject *object);

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    void setTitle(const QString &val);
    QString title() const { return m_title; }
    Q_SIGNAL void titleChanged();

    Q_PROPERTY(QString shortcut READ shortcut WRITE setShortcut NOTIFY shortcutChanged)
    void setShortcut(const QString &val);
    QString shortcut() const { return m_shortcut; }
    Q_SIGNAL void shortcutChanged();

    Q_PROPERTY(QString group READ group WRITE setGroup NOTIFY groupChanged)
    void setGroup(const QString &val);
    QString group() const { return m_group; }
    Q_SIGNAL void groupChanged();

    Q_PROPERTY(int priority READ priority WRITE setPriority NOTIFY priorityChanged)
    void setPriority(int val);
    int priority() const { return m_priority; }
    Q_SIGNAL void priorityChanged();

    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    void setEnabled(bool val);
    bool isEnabled() const { return m_enabled; }
    Q_SIGNAL void enabledChanged();

    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged)
    void setVisible(bool val);
    bool isVisible() const { return m_visible; }
    Q_SIGNAL void visibleChanged();

    Q_PROPERTY(bool canActivate READ canActivate WRITE setCanActivate NOTIFY canActivateChanged)
    void setCanActivate(bool val);
    bool canActivate() const { return m_canActivate; }
    Q_SIGNAL void canActivateChanged();

    Q_INVOKABLE void activate();

signals:
    void changed(ShortcutsModelItem *ptr);
    void activated();

private:
    int m_priority = 0;
    bool m_enabled = true;
    bool m_visible = true;
    bool m_canActivate = false;
    bool m_handleShortcut = false;
    QString m_group;
    QString m_title;
    QString m_shortcut;
};

class ShortcutsModelRecord : public ShortcutsModelItem
{
    Q_OBJECT
    QML_ELEMENT

public:
    ShortcutsModelRecord(QObject *parent = nullptr) : ShortcutsModelItem(parent) { }
    ~ShortcutsModelRecord() { }
};

#endif // SHORTCUTSMODEL_H
