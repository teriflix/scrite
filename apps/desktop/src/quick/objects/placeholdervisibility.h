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

#ifndef PLACEHOLDERVISIBILITY_H
#define PLACEHOLDERVISIBILITY_H

#include <QObject>
#include <QQmlEngine>

class QQuickItem;

class PlaceholderVisibility : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_ATTACHED(PlaceholderVisibility)

public:
    explicit PlaceholderVisibility(QObject *parent = nullptr);
    ~PlaceholderVisibility();

    static PlaceholderVisibility *qmlAttachedProperties(QObject *parent);

    // clang-format off
    Q_PROPERTY(bool visible
               READ isVisible
               WRITE setVisible
               NOTIFY visibleChanged)
    // clang-format on
    void setVisible(bool val);
    bool isVisible() const { return m_visible; }
    Q_SIGNAL void visibleChanged();

private:
    void implimentVisibility();

private:
    bool m_visible = true;
    QQuickItem *m_textItem = nullptr;
    QQuickItem *m_placeholderItem = nullptr;
};

#endif
