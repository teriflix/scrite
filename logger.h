/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>

class QFile;

class Logger : public QObject
{
    Q_OBJECT

public:
    static Logger *instance();
    ~Logger();

    Q_PROPERTY(QString logFilePath READ logFilePath CONSTANT)
    QString logFilePath() const;

    Q_INVOKABLE void log(const QString &message);

    static void qtMessageHandler(QtMsgType, const QMessageLogContext &, const QString &);
    static void qtPropertyInfo(const QObject *object, const char *propertyName);
    static void qtInfo(const QObject *object, const QString &message);
    static QString objectInfo(const QObject *object, bool complete=false);

private:
    Logger();

private:
    QFile  *m_logFile = nullptr;
    QString m_logFilePath;
};

#endif // LOGGER_H
