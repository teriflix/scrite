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

#ifndef SYSTEMTEXTINPUTSOURCE_H
#define SYSTEMTEXTINPUTSOURCE_H

#include <QJsonArray>
#include <QJsonObject>
#include <QBasicTimer>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>

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

    // language = AbstractSystemTextInputSource::Language
    Q_INVOKABLE QList<AbstractSystemTextInputSource *> sourcesForLanguage(int language) const;
    Q_INVOKABLE QJsonArray sourcesForLanguageJson(int language) const;

    Q_INVOKABLE AbstractSystemTextInputSource *findSourceById(const QString &id) const;

    Q_PROPERTY(AbstractSystemTextInputSource *selectedInputSource READ selectedInputSource NOTIFY selectedInputSourceChanged)
    AbstractSystemTextInputSource *selectedInputSource() const { return m_selectedInputSource; }
    Q_SIGNAL void selectedInputSourceChanged();

    Q_PROPERTY(AbstractSystemTextInputSource *defaultInputSource READ defaultInputSource NOTIFY defaultInputSourceChanged)
    AbstractSystemTextInputSource *defaultInputSource() const { return m_defaultInputSource; }
    Q_SIGNAL void defaultInputSourceChanged();

    AbstractSystemTextInputSource *fallbackInputSource(int fallbackLanguage = 0) const;

    // QAbstractItemModel interface
    enum Role {
        TextInputSourceRole = Qt::UserRole,
        TextInputSourceIDRole,
        TextInputSourceDisplayNameRole,
        TextInputSourceSelectedRole,
        TextInputSourceLanguageRole,
        TextInputSourceLanguageAsStringRole
    };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

protected:
    bool eventFilter(QObject *object, QEvent *event);

private:
    SystemTextInputManager(QObject *parent = nullptr);

    void clear();
    void add(AbstractSystemTextInputSource *source);
    void remove(AbstractSystemTextInputSource *source);
    void setSelected(AbstractSystemTextInputSource *source);
    void setDefault(AbstractSystemTextInputSource *dSource);

private:
    friend class AbstractSystemTextInputSource;
    AbstractSystemTextInputManagerBackend *m_backend = nullptr;
    QList<AbstractSystemTextInputSource *> m_inputSources;
    AbstractSystemTextInputSource *m_defaultInputSource = nullptr;
    AbstractSystemTextInputSource *m_selectedInputSource = nullptr;
    QString m_revertToInputSourceUponActivation;
};

class AbstractSystemTextInputSource : public QObject
{
    Q_OBJECT

public:
    explicit AbstractSystemTextInputSource(SystemTextInputManager *parent = nullptr);
    ~AbstractSystemTextInputSource();
    Q_SIGNAL void aboutToDelete(AbstractSystemTextInputSource *source);

    Q_PROPERTY(QString id READ id CONSTANT)
    virtual QString id() const = 0;

    Q_PROPERTY(QString displayName READ displayName CONSTANT)
    virtual QString displayName() const = 0;

    Q_PROPERTY(QString title READ title CONSTANT)
    QString title() const
    {
        return this->displayName() + QStringLiteral(" - [") + this->id() + QStringLiteral("] ");
    }

    Q_PROPERTY(bool selected READ isSelected NOTIFY selectedChanged)
    bool isSelected() const { return m_selected; }
    Q_SIGNAL void selectedChanged();

    // here language is -1 for unknown, or one of the constants from
    // TransliterationEngine::Language
    Q_PROPERTY(int language READ language CONSTANT)
    virtual int language() const = 0;

    Q_INVOKABLE virtual void select() = 0;
    Q_INVOKABLE virtual void checkSelection() = 0;

protected:
    void setSelected(bool val);

private:
    friend class SystemTextInputManager;
    bool m_selected = false;
    QBasicTimer m_addTimer;
    SystemTextInputManager *m_inputManager = nullptr;
};

class AbstractSystemTextInputManagerBackend : public QObject
{
public:
    explicit AbstractSystemTextInputManagerBackend(SystemTextInputManager *parent = nullptr);
    ~AbstractSystemTextInputManagerBackend();

    SystemTextInputManager *inputManager() const { return m_inputManager; }

    virtual QList<AbstractSystemTextInputSource *> reloadSources() = 0;
    void determineSelectedInputSource();

private:
    SystemTextInputManager *m_inputManager = nullptr;
};

#endif // SYSTEMTEXTINPUTSOURCE_H
