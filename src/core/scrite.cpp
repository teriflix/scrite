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

Application *Scrite::app()
{
    return Application::instance();
}

AppWindow *Scrite::window()
{
    return AppWindow::instance();
}

User *Scrite::user()
{
    return User::instance();
}

ScriteDocument *Scrite::document()
{
    return ScriteDocument::instance();
}


ScriteDocumentVault *Scrite::vault()
{
    return ScriteDocumentVault::instance();
}

ShortcutsModel *Scrite::shortcuts()
{
    return ShortcutsModel::instance();
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

QStringList Scrite::defaultTransitions()
{
    return QStringList(
            { QStringLiteral("CUT TO"), QStringLiteral("DISSOLVE TO"), QStringLiteral("FADE IN"),
              QStringLiteral("FADE OUT"), QStringLiteral("FADE TO"), QStringLiteral("FLASHBACK"),
              QStringLiteral("FLASH CUT TO"), QStringLiteral("FREEZE FRAME"),
              QStringLiteral("IRIS IN"), QStringLiteral("IRIS OUT"), QStringLiteral("JUMP CUT TO"),
              QStringLiteral("MATCH CUT TO"), QStringLiteral("MATCH DISSOLVE TO"),
              QStringLiteral("SMASH CUT TO"), QStringLiteral("STOCK SHOT"),
              QStringLiteral("TIME CUT"), QStringLiteral("WIPE TO") });
}

QStringList Scrite::defaultShots()
{
    return QStringList({ QStringLiteral("AIR"), QStringLiteral("CLOSE ON"),
                         QStringLiteral("CLOSER ON"), QStringLiteral("CLOSEUP"),
                         QStringLiteral("ESTABLISHING"), QStringLiteral("EXTREME CLOSEUP"),
                         QStringLiteral("INSERT"), QStringLiteral("POV"), QStringLiteral("SURFACE"),
                         QStringLiteral("THREE SHOT"), QStringLiteral("TWO SHOT"),
                         QStringLiteral("UNDERWATER"), QStringLiteral("WIDE"),
                         QStringLiteral("WIDE ON"), QStringLiteral("WIDER ANGLE") });
}
