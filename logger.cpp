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

#include "logger.h"

#include <QDir>
#include <QFile>
#include <QDate>
#include <QTime>
#include <QStandardPaths>
#include <QTextStream>
#include <QtDebug>
#include <QMetaProperty>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

Logger *Logger::instance()
{
    static Logger theInstance;
    return &theInstance;
}

Logger::Logger()
    : m_logFile(nullptr)
{
    const QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appData);
    QDir appDataDir(appData);
    if( !appDataDir.cd("logs") )
    {
        appDataDir.mkdir("logs");
        appDataDir.cd("logs");
    }

    m_logFilePath = appDataDir.absoluteFilePath("%1.txt")
                              .arg(QDateTime::currentMSecsSinceEpoch());

    m_logFile = new QFile(m_logFilePath);
    m_logFile->open(QFile::Append);
}

Logger::~Logger()
{
    delete m_logFile;
}

QString Logger::logFilePath() const
{
    return m_logFilePath;
}

void Logger::log(const QString &message)
{
    QTextStream ts(m_logFile);
    ts << "[" << QDateTime::currentDateTime().toString(Qt::ISODateWithMs) << "] " << message << "\n";
}

void Logger::qtMessageHandler(QtMsgType type, const QMessageLogContext & context, const QString &message)
{
#ifdef QT_NO_DEBUG
    Q_UNUSED(type)
    Q_UNUSED(context)
    Q_UNUSED(message)
#else
    QString logMessage;

    QTextStream ts(&logMessage, QIODevice::WriteOnly);
    switch(type)
    {
    case QtDebugMsg: ts << "Debug: "; break;
    case QtWarningMsg: ts << "Warning: "; break;
    case QtCriticalMsg: ts << "Critical: "; break;
    case QtFatalMsg: ts << "Fatal: "; break;
    case QtInfoMsg: ts << "Info: "; break;
    }

#ifndef QT_NO_DEBUG
    ts << "[" << context.category << " / " << context.file << " / " << context.function << " / " << context.line << "] (" << context.version << ") - ";
#else
    Q_UNUSED(context);
#endif

    ts << message;
    ts.flush();

    Logger::instance()->log(logMessage);

#ifndef QT_NO_DEBUG
    fprintf(stderr, "%s\n", qPrintable(logMessage));
#endif

#endif
}

void Logger::qtPropertyInfo(const QObject *object, const char *propertyName)
{
    if( object == nullptr || propertyName == nullptr )
        return;

    const int propIndex = object->metaObject()->indexOfProperty(propertyName);
    if( propIndex < 0 )
        return;

    QString objInfo = object->objectName();
    if( objInfo.isEmpty() )
        objInfo = QString::fromLatin1(object->metaObject()->className());
    objInfo += QString(" (0x%1)").arg((unsigned long)((void*)object),0,16);

    QString valueInfo;

    QMetaProperty prop = object->metaObject()->property(propIndex);
    QMetaType propType(prop.userType());
    if( propType.flags() & QMetaType::PointerToQObject )
    {
        const QObject *value = prop.read(object).value<QObject*>();
        valueInfo = Logger::objectInfo(value);
    }
    else if( propType.flags() & QMetaType::IsEnumeration || prop.isEnumType() )
    {
        const int enumValue = prop.read(object).toInt();
        valueInfo = QString::fromLatin1(prop.enumerator().valueToKey(enumValue));
    }
    else
    {
        switch(prop.userType())
        {
        case QMetaType::QJsonArray:
            valueInfo = QString::fromLatin1( QJsonDocument(prop.read(object).toJsonArray()).toJson() );
            break;
        case QMetaType::QJsonObject:
            valueInfo = QString::fromLatin1( QJsonDocument(prop.read(object).toJsonObject()).toJson() );
            break;
        case QMetaType::QJsonDocument:
            valueInfo = QString::fromLatin1( prop.read(object).toJsonDocument().toJson() );
            break;
        case QMetaType::QStringList:
            valueInfo = "[" + prop.read(object).toStringList().join(",") + "]";
            break;
        default:
            valueInfo = prop.read(object).toString();
        }
    }

    qInfo("%s: %s=%s", qPrintable(objInfo), propertyName, qPrintable(valueInfo));
}

void Logger::qtInfo(const QObject *object, const QString &message)
{
    if( object == nullptr )
        return;

    QString objInfo = object->objectName();
    if( objInfo.isEmpty() )
        objInfo = QString::fromLatin1(object->metaObject()->className());
    objInfo += QString(" (0x%1)").arg((unsigned long)((void*)object),0,16);

    qInfo("%s: %s", qPrintable(objInfo), qPrintable(message));
}

QString Logger::objectInfo(const QObject *object, bool complete)
{
    QString objectInfo;
    if( object != nullptr )
    {
        objectInfo = object->objectName();
        if(objectInfo.isEmpty())
            objectInfo = QString::fromLatin1(object->metaObject()->className());
        objectInfo += QString(" (0x%1)").arg((unsigned long)((void*)object),0,16);

        if( complete && object->metaObject()->indexOfProperty("string") >= 0 )
            objectInfo += QString(" [%1]").arg(object->property("string").toString());
    }
    else
        objectInfo = "NULL";

    return objectInfo;
}
