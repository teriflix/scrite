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

#include "placeholdervisibility.h"
#include "utils.h"

#include <QQuickItem>

PlaceholderVisibility::PlaceholderVisibility(QObject *parent) : QObject(parent)
{
    m_textItem = qobject_cast<QQuickItem *>(parent);
}

PlaceholderVisibility::~PlaceholderVisibility() { }

PlaceholderVisibility *PlaceholderVisibility::qmlAttachedProperties(QObject *parent)
{
    return new PlaceholderVisibility(parent);
}

void PlaceholderVisibility::setVisible(bool val)
{
    if (m_visible == val)
        return;

    m_visible = val;
    this->implimentVisibility();
    emit visibleChanged();
}

void PlaceholderVisibility::implimentVisibility()
{
    if (m_placeholderItem == nullptr) {
        QObject *placeholderTextObject = Utils::Object::firstChildByType(
                m_textItem, QStringLiteral("QQuickPlaceholderText"));
        m_placeholderItem = qobject_cast<QQuickItem *>(placeholderTextObject);
    }

    if (m_placeholderItem != nullptr)
        m_placeholderItem->setVisible(m_visible);
}
