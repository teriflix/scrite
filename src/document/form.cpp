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

#include "form.h"
#include "application.h"
#include "restapicall.h"
#include "networkaccessmanager.h"

#include <QDir>
#include <QUuid>
#include <QStack>
#include <QApplication>
#include <QJsonDocument>

FormQuestion::FormQuestion(QObject *parent) : QObject(parent) { }

FormQuestion::~FormQuestion()
{
    emit aboutToDelete(this);
}

void FormQuestion::setId(const QString &val)
{
    if (m_id == val || !m_id.isEmpty())
        return;

    m_id = val;
    emit idChanged();
}

void FormQuestion::setNumber(const QString &val)
{
    if (m_number == val || !m_number.isEmpty())
        return;

    m_number = val;
    emit numberChanged();
}

void FormQuestion::setIndentation(int val)
{
    if (m_indentation == val)
        return;

    m_indentation = val;
    emit indentationChanged();
}

void FormQuestion::setQuestionText(const QString &val)
{
    if (m_questionText == val || !m_questionText.isEmpty())
        return;

    m_questionText = val;
    emit questionTextChanged();
}

void FormQuestion::setAnswerHint(const QString &val)
{
    if (m_answerHint == val || !m_answerHint.isEmpty())
        return;

    m_answerHint = val;
    emit answerHintChanged();
}

void FormQuestion::setType(Type val)
{
    if (m_type == val)
        return;

    m_type = val;
    emit typeChanged();
}

void FormQuestion::setMetaData(const QJsonObject &val)
{
    if (m_metaData == val)
        return;

    m_metaData = val;
    emit metaDataChanged();
}

///////////////////////////////////////////////////////////////////////////////

Form::Form(QObject *parent) : QObject(parent) { }

Form::~Form()
{
    emit aboutToDelete(this);
}

QString Form::createdBy() const
{
    return m_createdBy;
}

QString Form::version() const
{
    return m_version;
}

QUrl Form::moreInfoUrl() const
{
    return m_moreInfoUrl;
}

QObjectListModel<FormQuestion *> *Form::questionsModel() const
{
    return const_cast<QObjectListModel<FormQuestion *> *>(&m_questions);
}

FormQuestion *Form::questionAt(int index) const
{
    return m_questions.at(index);
}

QJsonObject Form::formDataTemplate() const
{
    const QList<FormQuestion *> list = m_questions.list();
    if (!m_formDataTemplate.isEmpty() || list.isEmpty())
        return m_formDataTemplate;

    QJsonObject ret;
    for (FormQuestion *q : list)
        ret.insert(q->id(), QString());

    (const_cast<Form *>(this))->m_formDataTemplate = ret;
    return m_formDataTemplate;
}

void Form::validateFormData(QJsonObject &val)
{
    QJsonObject validated = this->formDataTemplate();
    QJsonObject::iterator it = validated.begin();
    QJsonObject::iterator end = validated.end();
    while (it != end) {
        it.value() = val.value(it.key());
        ++it;
    }

    val = validated;
}

void Form::serializeToJson(QJsonObject &json) const
{
    // Since all properties in Form are read-only, we cannot expect QObjectSerializer
    // to serialize this class for us. That's the reason why we are explicitly
    // serializing the Form here.
    json.insert(QStringLiteral("id"), m_id);
    json.insert(QStringLiteral("type"), this->typeAsString());
    json.insert(QStringLiteral("title"), m_title);
    json.insert(QStringLiteral("subtitle"), m_subtitle);
    json.insert(QStringLiteral("createdBy"), m_createdBy);
    json.insert(QStringLiteral("version"), m_version);
    json.insert(QStringLiteral("moreInfoUrl"), m_moreInfoUrl.toString());

    const QMetaEnum formQuestionTypeEnum = FormQuestion::staticMetaObject.enumerator(
            FormQuestion::staticMetaObject.indexOfEnumerator("Type"));
    const QList<FormQuestion *> list = m_questions.list();

    QJsonArray qArray;
    for (FormQuestion *q : list) {
        const QString typeAsString = QLatin1String(formQuestionTypeEnum.valueToKey(q->type()));

        QJsonObject qjs;
        qjs.insert(QStringLiteral("id"), q->id());
        qjs.insert(QStringLiteral("question"), q->questionText());
        qjs.insert(QStringLiteral("answerHint"), q->answerHint());
        qjs.insert(QStringLiteral("type"), typeAsString);
        qjs.insert(QStringLiteral("metaData"), q->metaData());
        qjs.insert(QStringLiteral("indentation"), q->indentation());
        qArray.append(qjs);
    }

    json.insert(QStringLiteral("questions"), qArray);
}

void Form::deserializeFromJson(const QJsonObject &json)
{
    // Since all properties in Form are read-only, we cannot expect QObjectSerializer
    // to serialize this class for us. That's the reason why we are explicitly
    // serializing the Form here.
    const QString idAttr = QStringLiteral("id");

    if (json.contains(idAttr))
        this->setId(json.value(idAttr).toString());
    else
        this->setId(QUuid::createUuid().toString());

    this->setTypeFromString(json.value(QStringLiteral("type")).toString());
    this->setTitle(json.value(QStringLiteral("title")).toString());
    this->setSubtitle(json.value(QStringLiteral("subtitle")).toString());
    this->setCreatedBy(json.value(QStringLiteral("createdBy")).toString());
    this->setVersion(json.value(QStringLiteral("version")).toString());
    this->setMoreInfoUrl(QUrl(json.value(QStringLiteral("moreInfoUrl")).toString()));

    const QMetaEnum formQuestionTypeEnum = FormQuestion::staticMetaObject.enumerator(
            FormQuestion::staticMetaObject.indexOfEnumerator("Type"));
    QList<FormQuestion *> list;

    const QJsonArray qArray = json.value(QStringLiteral("questions")).toArray();

    QStack<int> qnumberStack;
    qnumberStack.push(0);
    auto qnumber = [](const QStack<int> &stack) {
        QStringList ret;
        for (int val : stack)
            ret << QString::number(val);
        return ret.join(QStringLiteral("."));
    };

    for (const QJsonValue &qjsi : qArray) {
        const QJsonObject qjs = qjsi.toObject();
        FormQuestion *question = new FormQuestion(this);
        connect(question, &FormQuestion::aboutToDelete, &m_questions,
                &QObjectListModel<FormQuestion *>::objectDestroyed);

        if (qjs.contains(idAttr))
            question->setId(qjs.value(idAttr).toString());
        else
            question->setId(QUuid::createUuid().toString());

        const QString ques = qjs.value(QStringLiteral("question")).toString();
        const QString ansHint = qjs.value(QStringLiteral("answerHint")).toString();
        const QString qtypeAsString = qjs.value(QStringLiteral("type")).toString();
        const QJsonObject metaData = qjs.value(QStringLiteral("metaData")).toObject();
        const QJsonValue qindentValue = qjs.value(QStringLiteral("indentation"));
        const int qindent = qMax(qindentValue.isString() ? qindentValue.toString().toInt()
                                                         : qindentValue.toInt(),
                                 0);

        if (qnumberStack.size() == qindent + 1)
            qnumberStack.top() += 1;
        if (qnumberStack.size() < qindent + 1) {
            while (qnumberStack.size() < qindent + 1)
                qnumberStack.push(1);
        } else if (qnumberStack.size() > qindent + 1) {
            while (qnumberStack.size() > qindent + 1)
                qnumberStack.pop();
            qnumberStack.top() += 1;
        }

        question->setQuestionText(ques);
        question->setAnswerHint(ansHint);
        question->setNumber(qnumber(qnumberStack));
        if (!qtypeAsString.isEmpty()) {
            bool ok = false;
            const int itype = formQuestionTypeEnum.keyToValue(qPrintable(qtypeAsString), &ok);
            if (ok)
                question->setType(FormQuestion::Type(itype));
        }
        question->setMetaData(metaData);
        question->setIndentation(qindent);

        list.append(question);
    }

    m_questions.assign(list);
}

void Form::setType(Type val)
{
    if (m_type == val)
        return;

    m_type = val;
    emit typeChanged();
}

void Form::setId(const QString &val)
{
    if (m_id == val || !m_id.isEmpty())
        return;

    m_id = val;
    emit idChanged();
}

void Form::setTitle(const QString &val)
{
    if (m_title == val || !m_title.isEmpty())
        return;

    m_title = val;
    emit titleChanged();
}

void Form::setSubtitle(const QString &val)
{
    if (m_subtitle == val || !m_subtitle.isEmpty())
        return;

    m_subtitle = val;
    emit subtitleChanged();
}

void Form::setCreatedBy(const QString &val)
{
    if (m_createdBy == val || !m_createdBy.isEmpty())
        return;

    m_createdBy = val;
    emit createdByChanged();
}

void Form::setVersion(const QString &val)
{
    if (m_version == val || !m_version.isEmpty())
        return;

    m_version = val;
    emit versionChanged();
}

void Form::setMoreInfoUrl(const QUrl &val)
{
    if (m_moreInfoUrl == val || !m_moreInfoUrl.isEmpty())
        return;

    m_moreInfoUrl = val;
    emit moreInfoUrlChanged();
}

void Form::setTypeFromString(const QString &val)
{
    static const int enumIndex = Form::staticMetaObject.indexOfEnumerator("Type");
    static const QMetaEnum enumerator = Form::staticMetaObject.enumerator(enumIndex);

    bool ok = true;
    const int itype = enumerator.keyToValue(qPrintable(val), &ok);
    if (!ok)
        this->setType(GeneralForm);
    else
        this->setType(Type(itype));
}

QString Form::typeAsString() const
{
    static const int enumIndex = Form::staticMetaObject.indexOfEnumerator("Type");
    static const QMetaEnum enumerator = Form::staticMetaObject.enumerator(enumIndex);
    return QString::fromLatin1(enumerator.valueToKey(m_type));
}

///////////////////////////////////////////////////////////////////////////////

Forms *Forms::global()
{
    static Forms *forms = new Forms(true, qApp);
    return forms;
}

Forms::Forms(QObject *parent) : QObjectListModel<Form *>(parent)
{
    connect(this, &QObjectListModel<Form *>::objectCountChanged, this, &Forms::formCountChanged);
}

Forms::Forms(bool, QObject *parent) : QObjectListModel<Form *>(parent)
{
    this->downloadForms();
}

Forms::~Forms() { }

Form *Forms::findForm(const QString &id) const
{
    if (id.isEmpty())
        return nullptr;

    const QList<Form *> forms = this->list();
    for (Form *form : forms) {
        if (form->id() == id)
            return form;
    }

    return nullptr;
}

QList<Form *> Forms::forms(Form::Type type) const
{
    const QList<Form *> forms = this->list();
    QList<Form *> ret;
    for (Form *form : forms) {
        if (form->type() == type)
            ret << form;
    }

    return ret;
}

Form *Forms::addForm(const QJsonObject &val)
{
    m_errorReport->clear();

    if (val.isEmpty()) {
        m_errorReport->setErrorMessage(QStringLiteral("Cannot load form from empty JSON data."));
        return nullptr;
    }

    Form *form = new Form(this);
    if (QObjectSerializer::fromJson(val, form)) {
        this->append(form);
        return form;
    }

    m_errorReport->setErrorMessage(QStringLiteral("Couldnt load form from JSON data."));
    delete form;
    return nullptr;
}

Form *Forms::addFormFromFile(const QString &path)
{
    m_errorReport->clear();

    if (!QFile::exists(path)) {
        m_errorReport->setErrorMessage(QStringLiteral("File '%1' does not exist.").arg(path));
        return nullptr;
    }

    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        m_errorReport->setErrorMessage(QStringLiteral("Couldn't load '%1' for reading.").arg(path));
        return nullptr;
    }

    QJsonParseError error;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        m_errorReport->setErrorMessage(error.errorString());
        return nullptr;
    }

    const QJsonObject jsonObj = jsonDoc.object();
    return this->addForm(jsonObj);
}

QList<Form *> Forms::addFormsInFolder(const QString &dirPath)
{
    m_errorReport->clear();

    QList<Form *> ret;
    const QDir dir(dirPath);
    const QFileInfoList fiList =
            dir.entryInfoList({ QStringLiteral("*.sform") }, QDir::Files, QDir::Name);

    if (fiList.isEmpty())
        return ret;

    ErrorReport *actualErrorReport = m_errorReport;
    QStringList errors;

    for (const QFileInfo &fi : fiList) {
        ErrorReport tempErrorReport;
        m_errorReport = &tempErrorReport;

        Form *form = this->addFormFromFile(fi.absoluteFilePath());

        if (form)
            ret << form;
        else {
            const QString error =
                    fi.fileName() + QStringLiteral(": ") + tempErrorReport.errorMessage();
            errors << error;
        }
    }

    m_errorReport = actualErrorReport;

    if (!errors.isEmpty())
        m_errorReport->setErrorMessage(errors.join(QStringLiteral(", ")));

    return ret;
}

void Forms::serializeToJson(QJsonObject &json) const
{
    const QList<Form *> forms = this->list();

    QJsonArray data;

    for (Form *form : forms) {
        const QJsonObject formObj = QObjectSerializer::toJson(form);
        data.append(formObj);
    }

    json.insert(QStringLiteral("#data"), data);
}

void Forms::deserializeFromJson(const QJsonObject &json)
{
    const QJsonArray data = json.value(QStringLiteral("#data")).toArray();
    QList<Form *> forms;

    for (const QJsonValue &item : data) {
        const QJsonObject formObj = item.toObject();
        Form *form = new Form(this);
        if (QObjectSerializer::fromJson(formObj, form)) {
            forms << form;
            continue;
        }

        delete form;
    }

    this->assign(forms);
}

void Forms::itemInsertEvent(Form *ptr)
{
    if (ptr)
        connect(ptr, &Form::aboutToDelete, this, &Forms::objectDestroyed);
}

void Forms::itemRemoveEvent(Form *ptr)
{
    if (ptr)
        disconnect(ptr, &Form::aboutToDelete, this, &Forms::objectDestroyed);
}

void Forms::downloadForms()
{
    if (this->findChild<RestApiCall *>() != nullptr)
        return;

    ScriptalayFormsRestApiCall *call = new ScriptalayFormsRestApiCall(this);
    connect(call, &RestApiCall::finished, this, [=]() {
        if (call->hasError()) {
            m_errorReport->setErrorMessage(call->errorText(), call->errorData());
            return;
        }

        if (!call->hasResponse())
            return;

        QList<Form *> forms;

        const QJsonArray records = call->records();
        if (records.isEmpty())
            return;

        ErrorReport *actualErrorReport = m_errorReport;
        QStringList errors;

        for (const QJsonValue &record : records) {
            ErrorReport tempErrorReport;
            m_errorReport = &tempErrorReport;

            const QJsonObject formJson = record.toObject();

            Form *form = new Form(this);
            if (QObjectSerializer::fromJson(formJson, form))
                forms.append(form);
            else {
                errors << tempErrorReport.errorMessage();
                delete form;
            }
        }

        this->assign(forms);

        m_errorReport = actualErrorReport;

        if (!errors.isEmpty())
            m_errorReport->setErrorMessage(errors.join(QStringLiteral(", ")));
    });
    call->call();
}
