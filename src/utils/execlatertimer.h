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

#ifndef EXECLATERTIMER_H
#define EXECLATERTIMER_H

#include <QTimer>
#include <QString>

class ExecLaterTimer : public QObject
{
    Q_OBJECT

public:
    static ExecLaterTimer *get(int timerId);

    explicit ExecLaterTimer(const QString &name = QStringLiteral("Scrite ExecLaterTimer"),
                            QObject *parent = nullptr);
    ~ExecLaterTimer();

    // clang-format off
    Q_PROPERTY(QString name
               READ name
               WRITE setName
               NOTIFY nameChanged)
    // clang-format on
    void setName(const QString &val);
    QString name() const { return m_name; }
    Q_SIGNAL void nameChanged();

    // clang-format off
    Q_PROPERTY(bool repeat
               READ isRepeat
               WRITE setRepeat
               NOTIFY repeatChanged)
    // clang-format on
    void setRepeat(bool val);
    bool isRepeat() const { return m_repeat; }
    Q_SIGNAL void repeatChanged();

    void start(int msec, QObject *object);
    void stop();
    int timerId() const { return m_timerId; }
    bool isActive() const { return m_timerId >= 0 && m_timer.isActive(); }

    static void discardCall(const char *name, QObject *receiver);
    static void call(const char *name, QObject *receiver, const std::function<void()> &func,
                     int timeout = 0);
    static void call(const char *name, const std::function<void()> &func, int timeout = 0);

private:
    void onTimeout();
    void onObjectDestroyed(QObject *ptr);

private:
    int m_timerId = -1;
    bool m_repeat = false;
    QTimer m_timer;
    QString m_name;
    bool m_destroyed = false;
    QObject *m_object = nullptr;
};

#endif // EXECLATERTIMER_H
