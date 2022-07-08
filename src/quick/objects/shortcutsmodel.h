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

#ifndef SHORTCUTSMODEL_H
#define SHORTCUTSMODEL_H

#include <QQmlEngine>
#include <QAbstractListModel>

class ShortcutsModelItem;
class ShortcutsModel : public QAbstractListModel
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ScriteShortcuts)
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    static ShortcutsModel *instance();
    ~ShortcutsModel();

    // QAbstractItemModel interface
    enum Roles { TitleRole = Qt::UserRole + 1, ShortcutRole, GroupRole, EnabledRole, VisibleRole };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

    Q_PROPERTY(QStringList groups READ groups WRITE setGroups NOTIFY groupsChanged)
    void setGroups(const QStringList &val);
    QStringList groups() const { return m_groups; }
    Q_SIGNAL void groupsChanged();

private:
    ShortcutsModel(QObject *parent = nullptr);

    void add(ShortcutsModelItem *item);
    void remove(ShortcutsModelItem *item);
    void update(ShortcutsModelItem *item, bool removeAndInsert = false);

    void sortItems(QList<ShortcutsModelItem *> &items);

private:
    friend class ShortcutsModelItem;
    QStringList m_groups;
    QList<ShortcutsModelItem *> m_items;
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

private:
    bool m_enabled = true;
    bool m_visible = true;
    QString m_group;
    QString m_title;
    QString m_shortcut;
    int m_priority = 0;
};

#endif // SHORTCUTSMODEL_H
