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

#include "scrite.h"

#include "user.h"
#include "appwindow.h"
#include "application.h"
#include "scritedocument.h"
#include "shortcutsmodel.h"
#include "notificationmanager.h"
#include "scritedocumentvault.h"

Scrite::Scrite(QObject *parent) : QObject(parent)
{
    qDebug() << "Warning: Scrite namespace being created.";
}

Scrite::~Scrite()
{
    qDebug() << "Warning: Scrite namespace being destroyed.";
}

QObject *Scrite::appObject()
{
    return Scrite::app();
}

Application *Scrite::app()
{
    return Application::instance();
}

QObject *Scrite::windowObject()
{
    return Scrite::window();
}

AppWindow *Scrite::window()
{
    return AppWindow::instance();
}

QObject *Scrite::userObject()
{
    return Scrite::user();
}

User *Scrite::user()
{
    return User::instance();
}

QObject *Scrite::documentObject()
{
    return Scrite::document();
}

ScriteDocument *Scrite::document()
{
    return ScriteDocument::instance();
}

QObject *Scrite::vaultObject()
{
    return ScriteDocumentVault::instance();
}

ScriteDocumentVault *Scrite::vault()
{
    return ScriteDocumentVault::instance();
}

QObject *Scrite::shortcutsObject()
{
    return Scrite::shortcuts();
}

ShortcutsModel *Scrite::shortcuts()
{
    return ShortcutsModel::instance();
}

QObject *Scrite::notificationsObject()
{
    return Scrite::notifications();
}

NotificationManager *Scrite::notifications()
{
    return NotificationManager::instance();
}

QString Scrite::m_fileNameToOpen;
void Scrite::setFileNameToOpen(const QString &val)
{
    if (m_fileNameToOpen.isEmpty())
        m_fileNameToOpen = val;
}
