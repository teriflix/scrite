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

#include "systemtextinputmanager_windows.h"
#include <QtDebug>
#include <QGuiApplication>

SystemTextInputManagerBackend_Windows::SystemTextInputManagerBackend_Windows(
        SystemTextInputManager *parent)
    : AbstractSystemTextInputManagerBackend(parent)
{
    qApp->installNativeEventFilter(this);
    qApp->installEventFilter(this);
}

SystemTextInputManagerBackend_Windows::~SystemTextInputManagerBackend_Windows()
{
    // In anycase, this object will be removed from filter lists during destruction.
    // qApp->removeNativeEventFilter(this);
    // qApp->removeEventFilter(this);
}

QList<AbstractSystemTextInputSource *> SystemTextInputManagerBackend_Windows::reloadSources()
{
    QList<AbstractSystemTextInputSource *> ret;
    int nrKeyboards = GetKeyboardLayoutList(0, nullptr);
    if (nrKeyboards == 0)
        return ret;

    QVector<HKL> keyboards(nrKeyboards, nullptr);
    nrKeyboards = GetKeyboardLayoutList(nrKeyboards, keyboards.data());
    for (int i = 0; i < nrKeyboards; i++)
        ret << new SystemTextInputSource_Windows(keyboards.at(i), this->inputManager());

    return ret;
}

bool SystemTextInputManagerBackend_Windows::nativeEventFilter(const QByteArray &eventType,
                                                              void *message, long *result)
{
    Q_UNUSED(eventType)
    Q_UNUSED(result)

    const MSG *winMsg = static_cast<MSG *>(message);
    if (winMsg->message == WM_INPUTLANGCHANGE) {
        this->determineSelectedInputSource();
        return false;
    }

    return false;
}

bool SystemTextInputManagerBackend_Windows::eventFilter(QObject *object, QEvent *event)
{
    /**
      Windows doesn't seem to have a WM_ notification for when the set of input sources
      change. But what's clear is that the user will have to switch away from our application
      into Control Panel (or Settings on Windows 10) to add/remove input sources.

      So, everytime our application is activated; we reload all input sources just to make sure
      that our list of input-sources is current. This will result in unfortunate reloads for
      when the user has not added or removed input sources between the time our application
      lost focus and got it back, but its a price we have to pay for Windows not giving us any
      WM_ message to notify us.
      */
    if (event->type() == QEvent::ApplicationStateChange && object == qApp
        && qApp->applicationState() == Qt::ApplicationActive)
        this->inputManager()->reload();

    return false;
}

///////////////////////////////////////////////////////////////////////////////

SystemTextInputSource_Windows::SystemTextInputSource_Windows(HKL keyboardLayoutHandle,
                                                             SystemTextInputManager *parent)
    : AbstractSystemTextInputSource(parent), m_hkl(keyboardLayoutHandle)
{
    // HKL is a 4-byte number where last 2 bytes is language code and first 2 bytes is layout type
    LANGID languageId = LANGID(quint32(m_hkl) & 0x0000FFFF);
    int layoutType = (quint32(m_hkl) & 0xFFFF0000) >> 16;
    auto toHex = [](const int number) {
        QString ret = QString::number(number, 16).toUpper();
        if (ret.length() < 4)
            ret.prepend(QString(4 - ret.length(), QChar('0')));
        return ret;
    };

    LCID locale = MAKELCID(languageId, SORT_DEFAULT);
    wchar_t name[256];
    memset(name, 0, sizeof(name));
    GetLocaleInfoW(locale, LOCALE_SLANGUAGE, name, 256);
    m_id = toHex(languageId) + QStringLiteral("-") + toHex(layoutType);
    m_displayName = QString::fromWCharArray(name);

    memset(name, 0, sizeof(name));
    GetLocaleInfoW(locale, LOCALE_SNAME, name, 256);
    const QString languageCode = QString::fromWCharArray(name).left(2);

    static const QStringList languageCodes = QStringList()
            << QStringLiteral("en") << QStringLiteral("bn") << QStringLiteral("gu")
            << QStringLiteral("hi") << QStringLiteral("kn") << QStringLiteral("ml")
            << QStringLiteral("mr") << QStringLiteral("or") << QStringLiteral("pa")
            << QStringLiteral("sa") << QStringLiteral("ta") << QStringLiteral("te");
    m_language = languageCodes.indexOf(languageCode);

    this->setSelected(GetKeyboardLayout(0) == m_hkl);
}

SystemTextInputSource_Windows::~SystemTextInputSource_Windows() { }

void SystemTextInputSource_Windows::select()
{
    ActivateKeyboardLayout(m_hkl, 0);
    this->checkSelection();
}

void SystemTextInputSource_Windows::checkSelection()
{
    this->setSelected(GetKeyboardLayout(0) == m_hkl);
}
