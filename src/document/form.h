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

#ifndef FORM_H
#define FORM_H

#include <QUrl>
#include <QQmlEngine>
#include <QQmlListProperty>

#include "errorreport.h"
#include "qobjectserializer.h"
#include "qobjectlistmodel.h"

class Form;

class FormQuestion : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    explicit FormQuestion(QObject *parent = nullptr);
    ~FormQuestion();
    Q_SIGNAL void aboutToDelete(FormQuestion *ptr);

    Q_PROPERTY(QString id READ id NOTIFY idChanged)
    QString id() const { return m_id; }
    Q_SIGNAL void idChanged();

    Q_PROPERTY(QString number READ number NOTIFY numberChanged)
    QString number() const { return m_number; }
    Q_SIGNAL void numberChanged();

    Q_PROPERTY(QString questionText READ questionText NOTIFY questionTextChanged)
    QString questionText() const { return m_questionText; }
    Q_SIGNAL void questionTextChanged();

    Q_PROPERTY(QString answerHint READ answerHint NOTIFY answerHintChanged)
    QString answerHint() const { return m_answerHint; }
    Q_SIGNAL void answerHintChanged();

    Q_PROPERTY(int indentation READ indentation NOTIFY indentationChanged)
    int indentation() const { return m_indentation; }
    Q_SIGNAL void indentationChanged();

    enum Type {
        None,
        ShortParagraph,
        LongParagraph,
        RadioButons,
        CheckBoxes,
        Date,
        Time,
        LinearScale
    };
    Q_ENUM(Type)
    Q_PROPERTY(Type type READ type NOTIFY typeChanged)
    Type type() const { return m_type; }
    Q_SIGNAL void typeChanged();

    Q_PROPERTY(QJsonObject metaData READ metaData NOTIFY metaDataChanged)
    QJsonObject metaData() const { return m_metaData; }
    Q_SIGNAL void metaDataChanged();

private:
    void setId(const QString &val);
    void setNumber(const QString &val);
    void setIndentation(int val);
    void setQuestionText(const QString &val);
    void setAnswerHint(const QString &val);
    void setType(Type val);
    void setMetaData(const QJsonObject &val);

private:
    friend class Form;
    QString m_id;
    QString m_number;
    int m_indentation = 0;
    QString m_questionText;
    QString m_answerHint;
    Type m_type = LongParagraph;
    QJsonObject m_metaData;
};

class Form : public QObject, public QObjectSerializer::Interface
{
    Q_OBJECT
    Q_INTERFACES(QObjectSerializer::Interface)
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    explicit Form(QObject *parent = nullptr);
    ~Form();
    Q_SIGNAL void aboutToDelete(Form *ptr);

    enum Type {
        PropForm,
        SceneForm,
        StoryForm,
        GeneralForm,
        LocationForm,
        CharacterForm,
        RelationshipForm
    };
    Q_ENUM(Type)
    Q_PROPERTY(Type type READ type NOTIFY typeChanged)
    Type type() const { return m_type; }
    Q_SIGNAL void typeChanged();

    Q_PROPERTY(QString id READ id NOTIFY idChanged)
    QString id() const { return m_id; }
    Q_SIGNAL void idChanged();

    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    QString title() const { return m_title; }
    Q_SIGNAL void titleChanged();

    Q_PROPERTY(QString subtitle READ subtitle NOTIFY subtitleChanged)
    QString subtitle() const { return m_subtitle; }
    Q_SIGNAL void subtitleChanged();

    Q_PROPERTY(QString createdBy READ createdBy NOTIFY createdByChanged)
    QString createdBy() const;
    Q_SIGNAL void createdByChanged();

    Q_PROPERTY(QString version READ version NOTIFY versionChanged)
    QString version() const;
    Q_SIGNAL void versionChanged();

    Q_PROPERTY(QUrl moreInfoUrl READ moreInfoUrl NOTIFY moreInfoUrlChanged)
    QUrl moreInfoUrl() const;
    Q_SIGNAL void moreInfoUrlChanged();

    Q_PROPERTY(QAbstractListModel* questionsModel READ questionsModel CONSTANT STORED false)
    QObjectListModel<FormQuestion *> *questionsModel() const;

    Q_INVOKABLE FormQuestion *questionAt(int index) const;

    Q_PROPERTY(int questionCount READ questionCount NOTIFY questionCountChanged)
    int questionCount() const { return m_questions.size(); }
    Q_SIGNAL void questionCountChanged();

    QJsonObject formDataTemplate() const;
    void validateFormData(QJsonObject &val);

    int ref() { return ++m_refCount; }
    int deref() { return --m_refCount; }

    // QObjectSerializer::Interface interface
    void serializeToJson(QJsonObject &) const;
    void deserializeFromJson(const QJsonObject &);

private:
    void setType(Type val);
    void setId(const QString &val);
    void setTitle(const QString &val);
    void setSubtitle(const QString &val);
    void setCreatedBy(const QString &val);
    void setVersion(const QString &val);
    void setMoreInfoUrl(const QUrl &val);

    void setTypeFromString(const QString &val);
    QString typeAsString() const;

private:
    QString m_id;
    QString m_title;
    QString m_subtitle;
    QUrl m_moreInfoUrl;
    int m_refCount = 0;
    QString m_version;
    QString m_createdBy;
    Type m_type = GeneralForm;
    QJsonObject m_formDataTemplate;

    QObjectListModel<FormQuestion *> m_questions;
};

class Forms : public QObjectListModel<Form *>, public QObjectSerializer::Interface
{
    Q_OBJECT
    Q_INTERFACES(QObjectSerializer::Interface)
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    static Forms *global();

    explicit Forms(QObject *parent = nullptr);
    ~Forms();

    Q_PROPERTY(int formCount READ formCount NOTIFY formCountChanged)
    int formCount() const { return this->objectCount(); }
    Q_SIGNAL void formCountChanged();

    Q_INVOKABLE Form *findForm(const QString &id) const;
    Q_INVOKABLE Form *formAt(int index) const { return this->at(index); }

    Q_INVOKABLE QList<Form *> forms(Form::Type type) const;

    Q_INVOKABLE Form *addForm(const QJsonObject &val);
    Q_INVOKABLE Form *addFormFromFile(const QString &path);
    Q_INVOKABLE QList<Form *> addFormsInFolder(const QString &dirPath);

    // QObjectSerializer::Interface interface
    void serializeToJson(QJsonObject &) const;
    void deserializeFromJson(const QJsonObject &);

protected:
    void itemInsertEvent(Form *ptr);
    void itemRemoveEvent(Form *ptr);

private:
    Forms(bool fetchGlobal, QObject *parent = nullptr);
    void downloadForms();

private:
    ErrorReport *m_errorReport = new ErrorReport(this);
};

#endif // FORM_H
