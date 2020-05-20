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

#ifndef SIMPLETIMER_H
#define SIMPLETIMER_H

#include <QTimer>
#include <QString>

class SimpleTimer : public QObject
{
    Q_OBJECT

public:
    static SimpleTimer *get(int timerId);

    SimpleTimer(const QString &name=QStringLiteral("Scrite Timer"), QObject *parent=nullptr);
    ~SimpleTimer();

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    void setName(const QString &val);
    QString name() const { return m_name; }
    Q_SIGNAL void nameChanged();

    void start(int msec, QObject *object);
    void stop() {
        m_timer.stop();
        m_timerId = -1;
    }
    int timerId() const { return m_timerId; }
    bool isActive() const { return m_timerId >= 0 && m_timer.isActive(); }

private:
    void onTimeout();
    void onObjectDestroyed(QObject *ptr);

private:
    int m_timerId = -1;
    QTimer m_timer;
    QString m_name;
    QObject *m_object = nullptr;
};

#endif // SIMPLETIMER_H

