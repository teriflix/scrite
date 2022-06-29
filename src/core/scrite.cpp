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

QObject *Scrite::appObject() const
{
    return this->app();
}

Application *Scrite::app() const
{
    return Application::instance();
}

QObject *Scrite::windowObject() const
{
    return this->window();
}

AppWindow *Scrite::window() const
{
    return AppWindow::instance();
}

QObject *Scrite::userObject() const
{
    return this->user();
}

User *Scrite::user() const
{
    return User::instance();
}

QObject *Scrite::documentObject() const
{
    return this->document();
}

ScriteDocument *Scrite::document() const
{
    return ScriteDocument::instance();
}

QObject *Scrite::vaultObject() const
{
    return ScriteDocumentVault::instance();
}

ScriteDocumentVault *Scrite::vault() const
{
    return ScriteDocumentVault::instance();
}

QObject *Scrite::shortcutsObject() const
{
    return this->shortcuts();
}

ShortcutsModel *Scrite::shortcuts() const
{
    return ShortcutsModel::instance();
}

QObject *Scrite::notificationsObject() const
{
    return this->notifications();
}

NotificationManager *Scrite::notifications() const
{
    return NotificationManager::instance();
}
