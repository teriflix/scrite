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

#ifndef SPELL_CHECK_SERVICE_H
#define SPELL_CHECK_SERVICE_H

#include <QObject>
#include <QJsonArray>
#include <QQmlParserStatus>

#include "modifiable.h"
#include "simpletimer.h"

struct TextFragment
{
    TextFragment() {}
    TextFragment(int s, int l) : m_start(s), m_length(l) { }

    int start() const { return m_start; }
    int length() const { return m_length; }
    int end() const { return m_start+m_length-1; }
    bool isValid() { return m_length > 0 && m_start >= 0; }
    bool operator == (const TextFragment &other) const {
        return m_start == other.m_start && m_length == other.m_length;
    }
    TextFragment & operator = (const TextFragment &other) {
        m_start = other.m_start;
        m_length = other.m_length;
        return *this;
    }

private:
    int m_start = -1;
    int m_length = 0;
};
Q_DECLARE_METATYPE(TextFragment)

class SpellCheckServiceResult;
class SpellCheckService : public QObject, public Modifiable, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

public:
    SpellCheckService(QObject *parent=nullptr);
    ~SpellCheckService();

    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    void setText(const QString &val);
    QString text() const { return m_text; }
    Q_SIGNAL void textChanged();

    enum Method { Automatic, OnDemand };
    Q_ENUM(Method)
    Q_PROPERTY(Method method READ method WRITE setMethod NOTIFY methodChanged)
    void setMethod(Method val);
    Method method() const { return m_method; }
    Q_SIGNAL void methodChanged();

    Q_PROPERTY(QJsonArray misspelledFragments READ misspelledFragmentsJson NOTIFY misspelledFragmentsChanged)
    QJsonArray misspelledFragmentsJson() const { return m_misspelledFragmentsJson; } // for QML access
    QList<TextFragment> misspelledFragments() const { return m_misspelledFragments; } // for C++ access
    Q_SIGNAL void misspelledFragmentsChanged();

    Q_PROPERTY(bool asynchronous READ isAsynchronous WRITE setAsynchronous NOTIFY asynchronousChanged)
    void setAsynchronous(bool val);
    bool isAsynchronous() const { return m_asynchronous; }
    Q_SIGNAL void asynchronousChanged();

    Q_PROPERTY(bool threaded READ isThreaded WRITE setThreaded NOTIFY threadedChanged)
    void setThreaded(bool val);
    bool isThreaded() const { return m_threaded; }
    Q_SIGNAL void threadedChanged();

    Q_INVOKABLE void scheduleUpdate();
    Q_INVOKABLE void update();

    // QQmlParserStatus interface
    void classBegin();
    void componentComplete();

signals:
    void started();
    void finished();

private:
    void setMisspelledFragments(const QList<TextFragment> &val);
    void doUpdate();
    void timerEvent(QTimerEvent *event);
    Q_SLOT void spellCheckThreadComplete();
    void acceptResult(const SpellCheckServiceResult& result);

private:
    QString m_text;
    Method m_method = OnDemand;
    bool m_threaded = true;
    bool m_asynchronous = true;
    bool m_requiresSpellCheck = false;
    SimpleTimer m_updateTimer;
    Modifiable m_textModifiable;
    ModificationTracker m_textTracker;
    QJsonArray m_misspelledFragmentsJson;
    QList<TextFragment> m_misspelledFragments;
};

#endif // SPELL_CHECK_SERVICE_H
