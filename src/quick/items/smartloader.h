/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef SMARTLOADER_H
#define SMARTLOADER_H

#include <QJsonValue>
#include <QQuickItem>

/**
 Special loader item for use with SmartVerticalListView.qml
 and SmartHorizontalListView.qml only!!
 */

class SmartLoader : public QQuickItem
{
    Q_OBJECT

public:
    SmartLoader(QQuickItem *parent=nullptr);
    ~SmartLoader();

    enum SizePolicy
    {
        PreferItemValue,
        PreferLoaderValue,
        PreferHigherValue
    };
    Q_ENUM(SizePolicy)

    Q_PROPERTY(SizePolicy widthPolicy READ widthPolicy WRITE setWidthPolicy NOTIFY widthPolicyChanged)
    void setWidthPolicy(SizePolicy val);
    SizePolicy widthPolicy() const { return m_widthPolicy; }
    Q_SIGNAL void widthPolicyChanged();

    Q_PROPERTY(SizePolicy heightPolicy READ heightPolicy WRITE setHeightPolicy NOTIFY heightPolicyChanged)
    void setHeightPolicy(SizePolicy val);
    SizePolicy heightPolicy() const { return m_heightPolicy; }
    Q_SIGNAL void heightPolicyChanged();

    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)
    void setActive(bool val);
    bool isActive() const { return m_active; }
    Q_SIGNAL void activeChanged();

    Q_PROPERTY(QQmlComponent* sourceComponent READ sourceComponent WRITE setSourceComponent NOTIFY sourceComponentChanged)
    void setSourceComponent(QQmlComponent* val);
    QQmlComponent* sourceComponent() const { return m_sourceComponent; }
    Q_SIGNAL void sourceComponentChanged();

    Q_PROPERTY(QQuickItem* item READ item NOTIFY itemChanged)
    QQuickItem* item() const { return m_item; }
    Q_SIGNAL void itemChanged();

    Q_SIGNAL void widthUpdated();
    Q_SIGNAL void heightUpdated();

private:
    void setItem(QQuickItem* val);
    void loadItem();
    void unloadItem();
    void sizeItem();
    void updateWidth();
    void updateHeight();

private:
    QQuickItem* m_item = nullptr;
    bool m_active = false;
    QQmlComponent* m_sourceComponent = nullptr;
    SizePolicy m_widthPolicy = PreferLoaderValue;
    SizePolicy m_heightPolicy = PreferItemValue;
};

#endif // SMARTLOADER_H

