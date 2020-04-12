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

#ifndef NOTE_H
#define NOTE_H

#include <QColor>
#include <QObject>

class Scene;
class Structure;
class Character;

class Note : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE Note(QObject *parent=nullptr);
    ~Note();
    Q_SIGNAL void aboutToDelete(Note *ptr);

    Q_PROPERTY(Structure* structure READ structure CONSTANT STORED false)
    Structure *structure() const { return m_structure; }

    Q_PROPERTY(Character* character READ character CONSTANT STORED false)
    Character *character() const { return m_character; }

    Q_PROPERTY(QString heading READ heading WRITE setHeading NOTIFY headingChanged)
    void setHeading(const QString &val);
    QString heading() const { return m_heading; }
    Q_SIGNAL void headingChanged();

    Q_PROPERTY(QString content READ content WRITE setContent NOTIFY contentChanged)
    void setContent(const QString &val);
    QString content() const { return m_content; }
    Q_SIGNAL void contentChanged();

    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    void setColor(const QColor &val);
    QColor color() const { return m_color; }
    Q_SIGNAL void colorChanged();

    Q_SIGNAL void noteChanged();

protected:
    bool event(QEvent *event);

private:
    QColor m_color = QColor(Qt::white);
    QString m_heading;
    QString m_content;
    Structure *m_structure = nullptr;
    Character *m_character = nullptr;
};

#endif // NOTE_H
