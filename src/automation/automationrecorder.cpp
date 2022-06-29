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

#ifdef SCRITE_ENABLE_AUTOMATION

#include "automationrecorder.h"
#include "application.h"

#include <QDir>
#include <QDateTime>
#include <QStandardPaths>
#include <QWindow>

AutomationRecorder::AutomationRecorder(QObject *parent) : QObject(parent)
{
    qApp->installEventFilter(this);
}

AutomationRecorder::~AutomationRecorder() { }

void AutomationRecorder::startRecording()
{
    if (m_recording)
        return;

    m_recordedStatements.clear();
    m_lastRecordedStatementTimestamp = 0;
    this->parent()->installEventFilter(this);
    this->setRecording(true);
}

void AutomationRecorder::stopRecording()
{
    if (!m_recording)
        return;

    this->parent()->removeEventFilter(this);
    this->setRecording(false);

    static const QDir desktopDir =
            QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    static const QString fileName = QStringLiteral("automation.qml");

    const QFileInfo fi(desktopDir.absoluteFilePath(fileName));
    QString absoluteFilePath;

    int counter = 1;
    while (1) {
        absoluteFilePath = desktopDir.absoluteFilePath(fi.baseName() + QStringLiteral("-")
                                                       + QString::number(counter++)
                                                       + QStringLiteral(".") + fi.suffix());
        if (!QFile::exists(absoluteFilePath))
            break;
    }

    QFile file(absoluteFilePath);
    file.open(QFile::WriteOnly);

    QTextStream ts(&file);
    ts << "import Scrite 1.0\n\n";
    ts << "Automation {\n";
    ts << "    id: automation\n\n";
    ts << "    ScriptStep { onRunScript: splashLoader.active = false }\n\n";
    ts << "    PauseStep { duration: 500 }\n\n";

    for (const QVariant &statement, qAsConst(m_recordedStatements)) {
        if (statement.userType() == QMetaType::QString) {
            ts << "    EventStep {\n";
            ts << "        window: qmlWindow\n";
            ts << "        onAutomate: " << statement.toString() << "\n";
            ts << "    }\n";
        } else
            ts << "    PauseStep { duration: " << statement.value<qint64>() << " }\n";
    }

    QWindow *window = qobject_cast<QWindow *>(this->parent());
    if (window && !m_recordedStatements.isEmpty()) {
        const QPoint pos = window->mapFromGlobal(QCursor::pos());
        ts << "    EventStep {\n";
        ts << "        window: qmlWindow\n";
        ts << "        onAutomate: mouseMove(" << pos.x() << ", " << pos.y() << ", 0, 0);\n";
        ts << "    }\n";
    }

    ts << "\n    PauseStep { duration: 500 }\n";
    ts << "    WindowCapture {\n";
    ts << "        path: automation.pathOf(automation.fromUrl(automationScript))\n";
    ts << "        fileName: automation.fileNameOf(automation.fromUrl(automationScript), "
          "\"jpg\")\n";
    ts << "        format: WindowCapture.JPGFormat\n";
    ts << "        forceCounterInFileName: false\n";
    ts << "        window: qmlWindow\n";
    ts << "        replaceExistingFile: true\n";
    ts << "        maxImageSize: Qt.size(1920, 1080)\n";
    ts << "    }\n";
    ts << "}\n\n";

    m_recordedStatements.clear();
    m_lastRecordedStatementTimestamp = QDateTime::currentMSecsSinceEpoch();
    ;
}

void AutomationRecorder::timerEvent(QTimerEvent *te)
{
    if (te->timerId() == m_timer.timerId()) {
        m_timer.stop();
        this->toggleRecording();
    }
}

bool AutomationRecorder::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_F7) {
            this->toggleRecordingLater();
            return false;
        }
    }

    if (object == this->parent()) {
        const qint64 timestamp = QDateTime::currentMSecsSinceEpoch();
        if (timestamp - m_lastRecordedStatementTimestamp < 50)
            return false;

        static const QString comma = QStringLiteral(", ");
        static const QString closingBracket = QStringLiteral(")");
        auto addSleepStatement = [=]() {
            if (m_lastRecordedStatementTimestamp > 0) {
                qint64 duration = timestamp - m_lastRecordedStatementTimestamp;
                duration = qMax(qint64(duration / 100) * 100, qint64(100));
                m_recordedStatements << duration;
            }
        };

        switch (event->type()) {
        case QEvent::MouseButtonPress: {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            m_pressPos = me->pos();
            m_lastRecordedStatementTimestamp = timestamp;
        } break;
        case QEvent::MouseButtonRelease: {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            addSleepStatement();
            if (me->pos() != m_pressPos) {
                m_recordedStatements << QStringLiteral("mousePress(") + QString::number(me->x())
                                + comma + QString::number(me->y()) + comma
                                + QString::number(me->button()) + comma
                                + QString::number(me->modifiers()) + closingBracket;
                m_recordedStatements << QStringLiteral("mouseMove(") + QString::number(me->x())
                                + comma + QString::number(me->y()) + comma
                                + QString::number(me->button()) + comma
                                + QString::number(me->modifiers()) + closingBracket;
                m_recordedStatements << QStringLiteral("mouseRelease(") + QString::number(me->x())
                                + comma + QString::number(me->y()) + comma
                                + QString::number(me->button()) + comma
                                + QString::number(me->modifiers()) + closingBracket;
            } else
                m_recordedStatements << QStringLiteral("mouseClick(") + QString::number(me->x())
                                + comma + QString::number(me->y()) + comma
                                + QString::number(me->button()) + comma
                                + QString::number(me->modifiers()) + closingBracket;
            m_pressPos = me->pos();
            m_lastRecordedStatementTimestamp = timestamp;
        } break;
#if 0
        case QEvent::MouseMove: {
            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            addSleepStatement();
            m_recordedStatements << QStringLiteral("mouseMove(") +
                                    QString::number(me->x()) + comma +
                                    QString::number(me->y()) + comma +
                                    QString::number(me->button()) + comma +
                                    QString::number(me->modifiers()) + closingBracket;
            m_lastRecordedStatementTimestamp = timestamp;
            } break;
#endif
        case QEvent::MouseButtonDblClick: {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            addSleepStatement();
            m_recordedStatements << QStringLiteral("mouseDoubleClick(") + QString::number(me->x())
                            + comma + QString::number(me->y()) + comma
                            + QString::number(me->button()) + comma
                            + QString::number(me->modifiers()) + closingBracket;
            m_lastRecordedStatementTimestamp = timestamp;
        } break;
        case QEvent::Wheel: {
            QWheelEvent *we = static_cast<QWheelEvent *>(event);
            addSleepStatement();
            m_recordedStatements << QStringLiteral("mouseWheel(") + QString::number(we->x()) + comma
                            + QString::number(we->y()) + comma + QString::number(we->delta())
                            + comma + QString::number(we->orientation()) + comma
                            + QString::number(we->modifiers()) + closingBracket;
            m_lastRecordedStatementTimestamp = timestamp;
        } break;
        case QEvent::KeyRelease: {
            QKeyEvent *ke = static_cast<QKeyEvent *>(event);
            addSleepStatement();
            m_recordedStatements << QStringLiteral("keyClick(") + QString::number(ke->key()) + comma
                            + QString::number(ke->modifiers()) + closingBracket;
        } break;
        default:
            break;
        }
    }

    return false;
}

void AutomationRecorder::setRecording(bool val)
{
    if (m_recording == val)
        return;

    m_recording = val;
    if (this->parent()) {
        if (m_recording)
            this->parent()->setProperty("windowTitle", "Recording");
        else
            this->parent()->setProperty("windowTitle", "Done");
    }
    emit recordingChanged();
}

void AutomationRecorder::toggleRecording()
{
    if (m_recording)
        this->stopRecording();
    else
        this->startRecording();
}

void AutomationRecorder::toggleRecordingLater()
{
    m_timer.start(50, this);
}

#endif // #ifdef SCRITE_ENABLE_AUTOMATION
