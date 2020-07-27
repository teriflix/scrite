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

#ifndef SYSTEMTEXTINPUTSOURCE_H
#define SYSTEMTEXTINPUTSOURCE_H

#include <QJsonArray>
#include <QJsonObject>
#include <QBasicTimer>
#include <QAbstractListModel>

class AbstractSystemTextInputSource;
class AbstractSystemTextInputManagerBackend;

class SystemTextInputManager : public QAbstractListModel
{
    Q_OBJECT

public:
    static SystemTextInputManager *instance();
    ~SystemTextInputManager();

    Q_INVOKABLE void reload();

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    int count() const { return m_inputSources.size(); }
    Q_SIGNAL void countChanged();

    AbstractSystemTextInputSource *sourceAt(int index) const;

    Q_PROPERTY(AbstractSystemTextInputSource *selectedInputSource READ selectedInputSource NOTIFY selectedInputSourceChanged)
    AbstractSystemTextInputSource *selectedInputSource() const { return m_selectedInputSource; }
    Q_SIGNAL void selectedInputSourceChanged();

    // QAbstractItemModel interface
    enum Role
    {
        TextInputSourceRole = Qt::UserRole,
        TextInputSourceIDRole,
        TextInputSourceDisplayNameRole,
        TextInputSourceSelectedRole
    };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int,QByteArray> roleNames() const;

private:
    SystemTextInputManager(QObject *parent=nullptr);

    void clear();
    void add(AbstractSystemTextInputSource *source);
    void remove(AbstractSystemTextInputSource *source);
    void setSelected(AbstractSystemTextInputSource *source);

private:
    friend class AbstractSystemTextInputSource;
    AbstractSystemTextInputManagerBackend *m_backend = nullptr;
    QList<AbstractSystemTextInputSource*> m_inputSources;
    AbstractSystemTextInputSource *m_selectedInputSource = nullptr;
};

class AbstractSystemTextInputSource : public QObject
{
    Q_OBJECT

public:
    AbstractSystemTextInputSource(SystemTextInputManager *parent=nullptr);
    ~AbstractSystemTextInputSource();

    Q_PROPERTY(QString id READ id CONSTANT)
    virtual QString id() const = 0;

    Q_PROPERTY(QString displayName READ displayName CONSTANT)
    virtual QString displayName() const = 0;

    Q_PROPERTY(bool selected READ isSelected NOTIFY selectedChanged)
    bool isSelected() const { return m_selected; }
    Q_SIGNAL void selectedChanged();

    Q_INVOKABLE virtual void select() = 0;

protected:
    void timerEvent(QTimerEvent *event);
    void setSelected(bool val);

private:
    bool m_selected = false;
    QBasicTimer m_addTimer;
    SystemTextInputManager *m_inputManager = nullptr;
};

class AbstractSystemTextInputManagerBackend : public QObject
{
public:
    AbstractSystemTextInputManagerBackend(SystemTextInputManager *parent=nullptr);
    ~AbstractSystemTextInputManagerBackend();

    SystemTextInputManager *inputManager() const { return m_inputManager; }

    virtual void reloadSources() = 0;
    virtual void determineSelectedInputSource() = 0;

private:
    SystemTextInputManager *m_inputManager = nullptr;
};

#endif // SYSTEMTEXTINPUTSOURCE_H
