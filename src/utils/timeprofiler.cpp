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

#include "timeprofiler.h"

#include <QDir>
#include <QMap>
#include <QFile>
#include <QStack>
#include <QtDebug>
#include <QReadWriteLock>
#include <QThreadStorage>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QThread>

static void dump_time_profile_data()
{
    TimeProfile::print(TimeProfile::SortByAverageTime);

    const QString fileName = QString("%1_performance").arg(qApp->applicationName());
    const QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    const QString csvFileName = QDir(desktopPath).absoluteFilePath(QString("%1.csv").arg(fileName));
    const QString normalFileName =
            QDir(desktopPath).absoluteFilePath(QString("%1.txt").arg(fileName));

    TimeProfile::save(csvFileName, TimeProfile::SortByAverageTime, TimeProfile::CSVFormat);
    TimeProfile::save(normalFileName, TimeProfile::SortByAverageTime, TimeProfile::NormalFormat);
}

typedef QMap<QString, TimeProfile> TimeProfileMapType;
Q_GLOBAL_STATIC(QReadWriteLock, TimeProfileMapLock)
Q_GLOBAL_STATIC(TimeProfileMapType, TimeProfileMap)

TimeProfile TimeProfile::get(const QString &context)
{
    QReadLocker locker(::TimeProfileMapLock());
    return ::TimeProfileMap()->value(context);
}

static int timeSorter(const TimeProfile &a, const TimeProfile &b)
{
    return a.timeInNanoseconds() > b.timeInNanoseconds();
}

static int averageTimeSorter(const TimeProfile &a, const TimeProfile &b)
{
    return a.averageTimeInNanoseconds() > b.averageTimeInNanoseconds();
}

static int counterSorter(const TimeProfile &a, const TimeProfile &b)
{
    return a.counter() > b.counter();
}

static QList<TimeProfile> sortedProfiles(TimeProfile::PrintSortOrder sortOrder)
{
    QList<TimeProfile> profiles;

    {
        QReadLocker locker(::TimeProfileMapLock());
        profiles = ::TimeProfileMap()->values();
    }

    switch (sortOrder) {
    case TimeProfile::SortByTime:
        std::sort(profiles.begin(), profiles.end(), timeSorter);
        break;
    case TimeProfile::SortByAverageTime:
        std::sort(profiles.begin(), profiles.end(), averageTimeSorter);
        break;
    case TimeProfile::SortByCounter:
        std::sort(profiles.begin(), profiles.end(), counterSorter);
        break;
    case TimeProfile::NoSort:
        break;
    }

    return profiles;
}

void TimeProfile::print(TimeProfile::PrintSortOrder sortOrder, TimeProfile::PrintFormat format)
{
    const QList<TimeProfile> profiles = ::sortedProfiles(sortOrder);
    for (const TimeProfile &p : profiles)
        p.printSelf(0, format);
}

void TimeProfile::save(const QString &fileName, TimeProfile::PrintSortOrder sortOrder,
                       TimeProfile::PrintFormat format)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly))
        return;

    QTextStream ts(&file);

    ts << QString("%1;%2;%3;%4;%5;%6;%7;%8;%9;%10;%11")
                    .arg("Indent")
                    .arg("Context")
                    .arg("#Calls")
                    .arg("Time(s)")
                    .arg("Time(ms)")
                    .arg("Time(us)")
                    .arg("Time(ns)")
                    .arg("Avg.(s)")
                    .arg("Avg.(ms)")
                    .arg("Avg.(us)")
                    .arg("Avg.(ns)")
       << "\n";

    const QList<TimeProfile> profiles = ::sortedProfiles(sortOrder);
    for (const TimeProfile &p : profiles)
        ts << p.toString(0, format) << "\n";

    ts.flush();
    file.close();
}

QString TimeProfile::toString(int indent, TimeProfile::PrintFormat format) const
{
    switch (format) {
    case NormalFormat:
        return QString("[%7] %1%2\n Time: %3 s, %4 ms, %5 us %6 ns\n Avg: %8 s, %9 ms, %10 us %11 "
                       "ns")
                .arg(QString(indent, QChar(' ')))
                .arg(m_context)
                .arg(this->timeInSeconds())
                .arg(this->timeInMilliseconds())
                .arg(this->timeInMicroseconds())
                .arg(this->timeInNanoseconds())
                .arg(m_counter)
                .arg(this->averageTimeInSeconds())
                .arg(this->averageTimeInMilliseconds())
                .arg(this->averageTimeInMicroseconds())
                .arg(this->averageTimeInNanoseconds());
    case CSVFormat:
        return QString("%1;%2;%3;%4;%5;%6;%7;%8;%9;%10;%11")
                .arg(indent)
                .arg(m_context)
                .arg(m_counter)
                .arg(this->timeInSeconds())
                .arg(this->timeInMilliseconds())
                .arg(this->timeInMicroseconds())
                .arg(this->timeInNanoseconds())
                .arg(this->averageTimeInSeconds())
                .arg(this->averageTimeInMilliseconds())
                .arg(this->averageTimeInMicroseconds())
                .arg(this->averageTimeInNanoseconds());
    }

    return QString();
}

void TimeProfile::printSelf(int indent, PrintFormat format) const
{
    const QString str = this->toString(indent, format);
    fprintf(stderr, "%s\n", qPrintable(str));
}

TimeProfile TimeProfile::put(const TimeProfile &profile)
{
    if (!profile.isValid())
        return profile;

    QWriteLocker locker(::TimeProfileMapLock());

    TimeProfile existing = ::TimeProfileMap()->value(profile.context());
    if (existing.isValid())
        ::TimeProfileMap()->insert(profile.context(), existing + profile);
    else
        ::TimeProfileMap()->insert(profile.context(), profile);

    return profile;
}

#ifdef ENABLE_TIME_PROFILING

Q_GLOBAL_STATIC(QThreadStorage<QStack<TimeProfiler *>>, TimeProfilerStack)

static void addPostRoutine()
{
    static bool postRoutineAdded = false;
    if (!postRoutineAdded && qApp) {
        qAddPostRoutine(dump_time_profile_data);
        postRoutineAdded = true;
    }
}

inline QString evaluateContextPrefix()
{
    return qApp->thread() == QThread::currentThread() ? QStringLiteral(" [MainThread]")
                                                      : QStringLiteral(" [BackgroundThread]");
}

TimeProfiler::TimeProfiler(const QString &context, bool print)
    : m_context(context + evaluateContextPrefix()), m_printDuringCapture(print)
{
    addPostRoutine();
    ::TimeProfilerStack()->localData().push(this);
    m_timer.start();
}

TimeProfiler::~TimeProfiler()
{
    this->capture();
}

TimeProfile TimeProfiler::profile(bool aggregate) const
{
    TimeProfile p(m_context, m_timer.nsecsElapsed());
    if (aggregate)
        p += TimeProfile::get(m_context);
    return p;
}

void TimeProfiler::capture()
{
    if (m_captured)
        return;

    TimeProfile p = this->profile();
    TimeProfile::put(p);

    int indent = 0;
    Q_ASSERT(::TimeProfilerStack()->localData().pop() == this);
    indent = ::TimeProfilerStack()->localData().size();

    if (m_printDuringCapture)
        p.printSelf(indent);

    m_captured = true;
}

///////////////////////////////////////////////////////////////////////////////

ProfilerItem::ProfilerItem(QObject *parent) : QObject(parent)
{
    addPostRoutine();
    m_context = QString::fromLatin1(parent->metaObject()->className());
}

ProfilerItem::~ProfilerItem()
{
    if (m_active)
        this->setActive(false);

    if (m_timer)
        delete m_timer;
    m_timer = nullptr;
}

ProfilerItem *ProfilerItem::qmlAttachedProperties(QObject *object)
{
    return new ProfilerItem(object);
}

void ProfilerItem::setContext(const QString &val)
{
    if (m_context == val)
        return;

    m_context = val;
    emit contextChanged();
}

void ProfilerItem::setActive(bool val)
{
    if (m_active == val)
        return;

    m_active = val;

    if (m_active) {
        if (m_timer)
            delete m_timer;
        m_timer = new QElapsedTimer;
        m_timer->start();
    } else {
        if (m_timer) {
            const qint64 nsecs = m_timer->nsecsElapsed();
            delete m_timer;
            m_timer = nullptr;

            const QString ctx = m_context + evaluateContextPrefix();
            TimeProfile::put(TimeProfile(ctx, nsecs));
        }
    }

    emit activeChanged();
}

#endif // ENABLE_TIME_PROFILING
