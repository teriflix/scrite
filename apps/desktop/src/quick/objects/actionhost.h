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

#ifndef ACTIONHOST_H
#define ACTIONHOST_H

#include <QTimer>
#include <QObject>
#include <QHash>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QAbstractListModel>

#include "actionmanager.h"

class AbstractActionHost;
class AbstractActionGroup;

/**
 * Base class of the context that a host gives to the actions triggered from it. Every place that
 * hosts menus or toolbars has its own small class derived from this, with typed properties. See
 * actioncontexts.h.
 */
class AbstractActionContext : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Base class")

public:
    ~AbstractActionContext() override;

    // A short name for the kind of place this is the context of, such as "Notes". Every context
    // class gives its own, and it is part of the contract of the host uris that use the class.
    // clang-format off
    Q_PROPERTY(QString kind
               READ kind
               CONSTANT)
    // clang-format on
    QString kind() const { return m_kind; }

protected:
    explicit AbstractActionContext(const QString &kind, QObject *parent = nullptr);

private:
    QString m_kind;
};

/**
 * Everything that MenuGroup and ToolBarGroup have in common. A group is a contribution to the hosts
 * whose uri is its hostUri, and shows up in every one of them.
 */
class AbstractActionGroup : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Base class")

public:
    enum Kind { MenuKind, ToolBarKind };

    ~AbstractActionGroup() override;

    Kind kind() const { return m_kind; }

    // I am this: my name among the groups with my hostUri, so that other groups can merge into me.
    // Optional, set once, and unique among the groups of my kind with my hostUri.
    // clang-format off
    Q_PROPERTY(QString uri
               READ uri
               WRITE setUri
               NOTIFY uriChanged)
    // clang-format on
    void setUri(const QString &val);
    QString uri() const { return m_uri; }
    Q_SIGNAL void uriChanged();

    // I go into this: the uri of the hosts that show me, all of them. Set once.
    // clang-format off
    Q_PROPERTY(QString hostUri
               READ hostUri
               WRITE setHostUri
               NOTIFY hostUriChanged)
    // clang-format on
    void setHostUri(const QString &val);
    QString hostUri() const { return m_hostUri; }
    Q_SIGNAL void hostUriChanged();

    // I blend into this: the uri of the group with my hostUri that I am merged into. Empty, or a
    // target that doesn't exist, leaves me at the top level of my hostUri.
    // clang-format off
    Q_PROPERTY(QString mergeIntoUri
               READ mergeIntoUri
               WRITE setMergeIntoUri
               NOTIFY mergeIntoUriChanged)
    // clang-format on
    void setMergeIntoUri(const QString &val);
    QString mergeIntoUri() const { return m_mergeIntoUri; }
    Q_SIGNAL void mergeIntoUriChanged();

    // clang-format off
    Q_PROPERTY(QString title
               READ title
               WRITE setTitle
               NOTIFY titleChanged)
    // clang-format on
    void setTitle(const QString &val);
    QString title() const;
    Q_SIGNAL void titleChanged();

    // clang-format off
    Q_PROPERTY(QString iconSource
               READ iconSource
               WRITE setIconSource
               NOTIFY iconSourceChanged)
    // clang-format on
    void setIconSource(const QString &val);
    QString iconSource() const;
    Q_SIGNAL void iconSourceChanged();

    // clang-format off
    Q_PROPERTY(ActionManager* actionManager
               READ actionManager
               WRITE setActionManager
               NOTIFY actionManagerChanged)
    // clang-format on
    void setActionManager(ActionManager *val);
    ActionManager *actionManager() const { return m_actionManager; }
    Q_SIGNAL void actionManagerChanged();

    // A mixed list, which is why it is not typed. Each entry is one of:
    //   Action         a menu item, or a toolbar button
    //   ActionManager  a submenu, or a menu button
    //   MenuGroup      a submenu, or a menu button, with whatever is merged into it
    //   null           a separator (menus only)
    // The renderers cast each entry, and skip the ones that are none of these.
    // clang-format off
    Q_PROPERTY(QVariantList entries
               READ entries
               WRITE setEntries
               NOTIFY entriesChanged)
    // clang-format on
    void setEntries(const QVariantList &val);
    QVariantList entries() const { return m_entries; }
    Q_SIGNAL void entriesChanged();

    // For entries that look different in each host, because of its context. The host makes one
    // object from it for each host with my hostUri, sets its context property (which the root must
    // have, as a required property), and destroys it with the host. What the root can be depends on
    // the kind of group:
    //   MenuGroup      Action              one menu item
    //                  MenuItem            one menu item, as it is
    //                  Menu                one submenu, as it is
    //                  MenuItemsDelegate   several of those, and Instantiators of them
    //   ToolBarGroup   Item                a button, or a layout of several controls
    // Anything else is not shown. Menus ignore an Item, and toolbars ignore the rest.
    // clang-format off
    Q_PROPERTY(QQmlComponent* delegate
               READ delegate
               WRITE setDelegate
               NOTIFY delegateChanged)
    // clang-format on
    void setDelegate(QQmlComponent *val);
    QQmlComponent *delegate() const { return m_delegate; }
    Q_SIGNAL void delegateChanged();

    // clang-format off
    Q_PROPERTY(int order
               READ order
               WRITE setOrder
               NOTIFY orderChanged)
    // clang-format on
    void setOrder(int val);
    int order() const { return m_order; }
    Q_SIGNAL void orderChanged();

    // clang-format off
    Q_PROPERTY(bool visible
               READ isVisible
               WRITE setVisible
               NOTIFY visibleChanged)
    // clang-format on
    void setVisible(bool val);
    bool isVisible() const { return m_visible; }
    Q_SIGNAL void visibleChanged();

    // The visible groups that are merged into this one, sorted by order.
    // clang-format off
    Q_PROPERTY(QList<AbstractActionGroup*> merged
               READ merged
               NOTIFY mergedChanged)
    // clang-format on
    QList<AbstractActionGroup *> merged() const { return m_merged; }
    Q_SIGNAL void mergedChanged();

    // The group that this one is merged into. Null for groups at the top level of their hostUri.
    AbstractActionGroup *mergeParent() const { return m_mergeParent; }

protected:
    explicit AbstractActionGroup(Kind kind, QObject *parent = nullptr);

private:
    friend class ActionHostRegistry;
    void scheduleResolve();
    void onActionManagerDestroyed();
    void onDelegateDestroyed();

private:
    int m_order = 0;
    bool m_visible = true;
    Kind m_kind = MenuKind;
    QString m_uri;
    QString m_hostUri;
    QString m_title;
    QString m_iconSource;
    QString m_mergeIntoUri;
    QVariantList m_entries;
    QQmlComponent *m_delegate = nullptr;
    ActionManager *m_actionManager = nullptr;
    AbstractActionGroup *m_mergeParent = nullptr;
    QList<AbstractActionGroup *> m_merged;
};

/**
 * Something that a menu gets from Scrite or from an extension. Entries are Actions
 * (menu items), ActionManagers and MenuGroups (submenus) and nulls (separators).
 */
class MenuGroup : public AbstractActionGroup
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit MenuGroup(QObject *parent = nullptr);
    ~MenuGroup() override;

    // When true, the items go straight into the parent menu, set apart by separators, instead of
    // into a submenu.
    // clang-format off
    Q_PROPERTY(bool inline
               READ isInline
               WRITE setInline
               NOTIFY inlineChanged)
    // clang-format on
    void setInline(bool val);
    bool isInline() const { return m_inline; }
    Q_SIGNAL void inlineChanged();

private:
    bool m_inline = false;
};

/**
 * The root of a delegate of a MenuGroup that makes several items for each menu, instead of one. Its
 * default property takes menu items, menus and actions, and Instantiators of them. Menus show what
 * the Instantiators make as they make it. The host sets the context, which the items can use.
 */
class MenuItemsDelegate : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    // clang-format off
    Q_CLASSINFO("DefaultProperty", "objects")
    // clang-format on

public:
    explicit MenuItemsDelegate(QObject *parent = nullptr);
    ~MenuItemsDelegate() override;

    // clang-format off
    Q_PROPERTY(QQmlListProperty<QObject> objects
               READ objects
               CONSTANT)
    // clang-format on
    QQmlListProperty<QObject> objects();

    // clang-format off
    Q_PROPERTY(AbstractActionContext* context
               READ context
               WRITE setContext
               NOTIFY contextChanged)
    // clang-format on
    void setContext(AbstractActionContext *val);
    AbstractActionContext *context() const { return m_context; }
    Q_SIGNAL void contextChanged();

private:
    void onContextDestroyed();

private:
    AbstractActionContext *m_context = nullptr;
    QList<QObject *> m_objects;
};

/**
 * Something that a toolbar gets from Scrite or from an extension. Entries are Actions
 * (buttons), ActionManagers and MenuGroups (menu buttons).
 */
class ToolBarGroup : public AbstractActionGroup
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ToolBarGroup(QObject *parent = nullptr);
    ~ToolBarGroup() override;

    // Groups with lower priority collapse first. Ignored for groups that are merged into others,
    // because they collapse along with the group they are merged into.
    // clang-format off
    Q_PROPERTY(int priority
               READ priority
               WRITE setPriority
               NOTIFY priorityChanged)
    // clang-format on
    void setPriority(int val);
    int priority() const { return m_priority; }
    Q_SIGNAL void priorityChanged();

    // clang-format off
    Q_PROPERTY(bool collapsible
               READ isCollapsible
               WRITE setCollapsible
               NOTIFY collapsibleChanged)
    // clang-format on
    void setCollapsible(bool val);
    bool isCollapsible() const { return m_collapsible; }
    Q_SIGNAL void collapsibleChanged();

    // clang-format off
    Q_PROPERTY(bool separator
               READ hasSeparator
               WRITE setSeparator
               NOTIFY separatorChanged)
    // clang-format on
    void setSeparator(bool val);
    bool hasSeparator() const { return m_separator; }
    Q_SIGNAL void separatorChanged();

    // Something to pull the attention of the user to the group, and to what is merged into it: an
    // Item that the toolbar sizes to the group, and puts behind its buttons. It is made once for
    // each toolbar that shows the group. Groups without one have no backdrop. Its root must have
    // a required bool property named expanded, which the toolbar keeps at whether the group is
    // expanded, as opposed to folded into one button, so that the backdrop can look different.
    // clang-format off
    Q_PROPERTY(QQmlComponent* backdrop
               READ backdrop
               WRITE setBackdrop
               NOTIFY backdropChanged)
    // clang-format on
    void setBackdrop(QQmlComponent *val);
    QQmlComponent *backdrop() const { return m_backdrop; }
    Q_SIGNAL void backdropChanged();

private:
    void onBackdropDestroyed();

    int m_priority = 0;
    bool m_collapsible = false;
    bool m_separator = true;
    QQmlComponent *m_backdrop = nullptr;
};

/**
 * A place that shows contributions. Its uri names a kind of place, and many hosts can have the same
 * uri, as with the context menu of every scene in the screenplay editor. Groups name it with their
 * hostUri. The model has one row for each visible group with that hostUri that is at the top level,
 * sorted by order.
 */
class AbstractActionHost : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Base class")

public:
    ~AbstractActionHost() override;

    AbstractActionGroup::Kind kind() const { return m_kind; }

    // clang-format off
    Q_PROPERTY(QString uri
               READ uri
               WRITE setUri
               NOTIFY uriChanged)
    // clang-format on
    void setUri(const QString &val);
    QString uri() const { return m_uri; }
    Q_SIGNAL void uriChanged();

    // clang-format off
    Q_PROPERTY(AbstractActionContext* context
               READ context
               WRITE setContext
               NOTIFY contextChanged)
    // clang-format on
    void setContext(AbstractActionContext *val);
    AbstractActionContext *context() const { return m_context; }
    Q_SIGNAL void contextChanged();

    // clang-format off
    Q_PROPERTY(int count
               READ count
               NOTIFY countChanged)
    // clang-format on
    int count() const { return m_groups.size(); }
    Q_SIGNAL void countChanged();

    Q_INVOKABLE AbstractActionGroup *groupAt(int row) const { return m_groups.value(row); }

    // QAbstractItemModel interface
    enum Roles { GroupRole = Qt::UserRole + 1, LevelRole };
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

protected:
    explicit AbstractActionHost(AbstractActionGroup::Kind kind, QObject *parent = nullptr);

    QList<AbstractActionGroup *> groups() const { return m_groups; }

    virtual void groupsReloaded() { }
    virtual QVariant levelOf(AbstractActionGroup *group) const;

private:
    friend class ActionHostRegistry;
    void reload();
    void onContextDestroyed();

private:
    AbstractActionGroup::Kind m_kind = AbstractActionGroup::MenuKind;
    QString m_uri;
    AbstractActionContext *m_context = nullptr;
    QList<AbstractActionGroup *> m_groups;
};

class MenuHost : public AbstractActionHost
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit MenuHost(QObject *parent = nullptr);
    ~MenuHost() override;
};

/**
 * A host of toolbar groups, which also decides how much of each group fits in the width that the
 * toolbar has. The fit state is kept here, in the host, and not in the groups, because the same
 * group can be in several toolbars that are of different widths.
 */
class ToolBarHost : public AbstractActionHost
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ToolBarHost(QObject *parent = nullptr);
    ~ToolBarHost() override;

    enum Level { Expanded, Menu, Hidden };
    Q_ENUM(Level)

    // Zero means that nothing is fitted: every group is expanded.
    // clang-format off
    Q_PROPERTY(qreal availableWidth
               READ availableWidth
               WRITE setAvailableWidth
               NOTIFY availableWidthChanged)
    // clang-format on
    void setAvailableWidth(qreal val);
    qreal availableWidth() const { return m_availableWidth; }
    Q_SIGNAL void availableWidthChanged();

    // clang-format off
    Q_PROPERTY(qreal roomyWidth
               READ roomyWidth
               WRITE setRoomyWidth
               NOTIFY roomyWidthChanged)
    // clang-format on
    void setRoomyWidth(qreal val);
    qreal roomyWidth() const { return m_roomyWidth; }
    Q_SIGNAL void roomyWidthChanged();

    // True when every group is expanded and there is at least roomyWidth to spare.
    // clang-format off
    Q_PROPERTY(bool roomy
               READ isRoomy
               NOTIFY roomyChanged)
    // clang-format on
    bool isRoomy() const { return m_roomy; }
    Q_SIGNAL void roomyChanged();

    // Delegates tell the host how wide a group is when it is expanded, and when it is a menu button.
    Q_INVOKABLE void setGroupWidths(int row, qreal expanded, qreal menu);

protected:
    void groupsReloaded() override;
    QVariant levelOf(AbstractActionGroup *group) const override;

private:
    void scheduleFit();
    void fit();

private:
    struct FitInfo
    {
        Level level = Expanded;
        qreal expandedWidth = 0;
        qreal menuWidth = 0;
    };

    bool m_roomy = false;
    qreal m_roomyWidth = 0;
    qreal m_availableWidth = 0;
    QTimer m_fitTimer;
    QHash<const AbstractActionGroup *, FitInfo> m_fitInfo;
};

#endif // ACTIONHOST_H
