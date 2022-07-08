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

#ifndef TIME_PROFILER_H
#define TIME_PROFILER_H

#define ENABLE_TIME_PROFILING

#include <QString>
#include <QElapsedTimer>

class TimeProfiler;
class ProfilerItem;

class TimeProfile
{
public:
    static TimeProfile get(const QString &context);

    enum PrintSortOrder { NoSort, SortByTime, SortByCounter, SortByAverageTime };

    enum PrintFormat { CSVFormat, NormalFormat };
    static void print(PrintSortOrder sortOrder = NoSort, PrintFormat format = NormalFormat);
    static void save(const QString &fileName, PrintSortOrder sortOrder = NoSort,
                     PrintFormat format = CSVFormat);

    TimeProfile() : m_time(0), m_avgTime(0), m_counter(0) { }
    TimeProfile(const TimeProfile &other)
    {
        m_context = other.m_context;
        m_time = other.m_time;
        m_counter = other.m_counter;
        m_avgTime = other.m_avgTime;
    }

    ~TimeProfile() { }

    bool isValid() const { return m_counter > 0; }
    QString context() const { return m_context; }

    qint64 timeInNanoseconds() const { return m_time; }
    qint64 timeInMicroseconds() const { return m_time / 1000; }
    qint64 timeInMilliseconds() const { return m_time / 1000000; }
    qint64 timeInSeconds() const { return m_time / 1000000000; }

    qint64 averageTimeInNanoseconds() const { return m_avgTime; }
    qint64 averageTimeInMicroseconds() const { return m_avgTime / 1000; }
    qint64 averageTimeInMilliseconds() const { return m_avgTime / 1000000; }
    qint64 averageTimeInSeconds() const { return m_avgTime / 1000000000; }

    int counter() const { return m_counter; }

    TimeProfile &operator=(const TimeProfile &other)
    {
        m_context = other.m_context;
        m_time = other.m_time;
        m_counter = other.m_counter;
        m_avgTime = other.m_avgTime;
        return *this;
    }

    QString toString(int indent = 0,
                     TimeProfile::PrintFormat format = TimeProfile::NormalFormat) const;
    void printSelf(int indent = 0,
                   TimeProfile::PrintFormat format = TimeProfile::NormalFormat) const;

private:
    friend class TimeProfiler;
    friend class ProfilerItem;

    static TimeProfile put(const TimeProfile &profile);

    TimeProfile operator+(const TimeProfile &other) const
    {
        if (this->m_context == other.m_context) {
            return TimeProfile(this->m_context, this->m_time + other.m_time,
                               this->m_counter + other.m_counter);
        }
        return *this;
    }

    TimeProfile &operator+=(const TimeProfile &other)
    {
        if (this->m_context == other.m_context) {
            this->m_time += other.m_time;
            this->m_counter += other.m_counter;
            this->computeAverageTime();
        }
        return *this;
    }

    TimeProfile operator+(qint64 time) const
    {
        return TimeProfile(this->m_context, this->m_time + time, this->m_counter + 1);
    }

    TimeProfile &operator+=(qint64 time)
    {
        this->m_time += time;
        this->m_counter++;
        this->computeAverageTime();
        return *this;
    }

    void computeAverageTime()
    {
        if (m_time == 0)
            m_avgTime = 0;
        else if (m_counter > 0)
            m_avgTime = qint64(qreal(m_time) / qreal(m_counter));
    }

    TimeProfile(const QString &context, qint64 time, int counter = 1)
        : m_context(context), m_time(time), m_counter(counter)
    {
        this->computeAverageTime();
    }

    QString m_context;
    qint64 m_time = 0;
    qint64 m_avgTime = 0;
    int m_counter = 1;
};

#ifdef ENABLE_TIME_PROFILING

#include <QQmlEngine>

class TimeProfiler
{
public:
    TimeProfiler(const QString &context, bool print = false);
    ~TimeProfiler();

    QString context() const { return m_context; }
    TimeProfile profile(bool aggregate = false) const;

    void capture();

private:
    bool m_captured = false;
    QString m_context;
    QElapsedTimer m_timer;
    bool m_printDuringCapture = false;
};

class ProfilerItem : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Profiler)
    QML_ATTACHED(ProfilerItem)
    QML_UNCREATABLE("Use as attached property.")

public:
    explicit ProfilerItem(QObject *parent = nullptr);
    ~ProfilerItem();

    static ProfilerItem *qmlAttachedProperties(QObject *object);

    Q_PROPERTY(QString context READ context WRITE setContext NOTIFY contextChanged)
    void setContext(const QString &val);
    QString context() const { return m_context; }
    Q_SIGNAL void contextChanged();

    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)
    void setActive(bool val);
    bool isActive() const { return m_active; }
    Q_SIGNAL void activeChanged();

private:
    bool m_active = false;
    QString m_context;
    QElapsedTimer *m_timer = nullptr;
};

#define PROFILE_THIS_FUNCTION TimeProfiler profiler##__LINE__(Q_FUNC_INFO, false)
#define PROFILE_THIS_FUNCTION2 TimeProfiler profiler##__LINE__(Q_FUNC_INFO, true)

#else // #ifdef ENABLE_TIME_PROFILING

class TimeProfiler
{
public:
    TimeProfiler(const QString &, bool = false) { }
    ~TimeProfiler() { }

    QString context() const { return QString(); }
    TimeProfile profile(bool = false) const { return TimeProfile(); }
};

#define PROFILE_THIS_FUNCTION
#define PROFILE_THIS_FUNCTION2

#endif // #ifdef ENABLE_TIME_PROFILING

#endif // TIME_PROFILER_H
