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

#include "user.h"
#include "undoredo.h"
#include "fountain.h"
#include "hourglass.h"
#include "screenplay.h"
#include "application.h"
#include "scritedocument.h"
#include "garbagecollector.h"

#include <QMimeData>
#include <QSettings>
#include <QClipboard>
#include <QJsonDocument>
#include <QScopedValueRollback>

ScreenplayElement::ScreenplayElement(QObject *parent)
    : QObject(parent), m_scene(this, "scene"), m_screenplay(this, "screenplay")
{
    this->setScreenplay(qobject_cast<Screenplay *>(parent));

    connect(this, &ScreenplayElement::sceneChanged, this, &ScreenplayElement::elementChanged);
    connect(this, &ScreenplayElement::expandedChanged, this, &ScreenplayElement::elementChanged);
    connect(this, &ScreenplayElement::userSceneNumberChanged, this,
            &ScreenplayElement::elementChanged);
    connect(this, &ScreenplayElement::breakTitleChanged, this, &ScreenplayElement::elementChanged);
    connect(this, &ScreenplayElement::breakSubtitleChanged, this,
            &ScreenplayElement::elementChanged);
    connect(this, &ScreenplayElement::breakSummaryChanged, this,
            &ScreenplayElement::elementChanged);
    connect(this, &ScreenplayElement::elementChanged, [=]() { this->markAsModified(); });

    connect(this, &ScreenplayElement::sceneChanged, [=]() {
        if (m_elementType == BreakElementType)
            emit breakTitleChanged();
    });

    connect(this, &ScreenplayElement::sceneNumberChanged, this,
            &ScreenplayElement::resolvedSceneNumberChanged);
    connect(this, &ScreenplayElement::userSceneNumberChanged, this,
            &ScreenplayElement::resolvedSceneNumberChanged);
    connect(this, &ScreenplayElement::userSceneNumberChanged, this,
            &ScreenplayElement::evaluateSceneNumberRequest);
    connect(this, &ScreenplayElement::sceneChanged, this, &ScreenplayElement::wordCountChanged);
    connect(this, &ScreenplayElement::screenplayChanged, this,
            &ScreenplayElement::wordCountChanged);
}

ScreenplayElement::~ScreenplayElement()
{
    GarbageCollector::instance()->avoidChildrenOf(this);
    emit aboutToDelete(this);
}

void ScreenplayElement::setElementType(ScreenplayElement::ElementType val)
{
    if (m_elementType == val || m_elementTypeIsSet)
        return;

    m_elementType = val;
    emit elementTypeChanged();

    if (m_elementType == SceneElementType) {
        this->setNotes(nullptr);
        this->setAttachments(nullptr);
    } else {
        this->setNotes(new Notes(this));
        this->setAttachments(new Attachments(this));
    }
}

void ScreenplayElement::setBreakType(int val)
{
    if (m_breakType == val || m_elementType != BreakElementType)
        return;

    m_breakType = val;
    emit breakTypeChanged();
}

void ScreenplayElement::setBreakSubtitle(const QString &val)
{
    if (m_breakSubtitle == val)
        return;

    m_breakSubtitle = val;
    emit breakSubtitleChanged();
}

void ScreenplayElement::setBreakTitle(const QString &val)
{
    if (m_breakTitle == val || m_elementType != BreakElementType)
        return;

    m_breakTitle = val;
    emit breakTitleChanged();
}

void ScreenplayElement::setNotes(Notes *val)
{
    if (m_notes == val)
        return;

    if (m_notes != nullptr)
        m_notes->deleteLater();

    m_notes = val;
    emit notesChanged();
}

void ScreenplayElement::setAttachments(Attachments *val)
{
    if (m_attachments == val)
        return;

    if (m_attachments != nullptr)
        m_attachments->deleteLater();

    m_attachments = val;
    emit attachmentsChanged();
}

void ScreenplayElement::setScreenplay(Screenplay *val)
{
    if (m_screenplay != nullptr || m_screenplay == val)
        return;

    m_screenplay = val;

    if (m_screenplay != nullptr)
        connect(this, &ScreenplayElement::wordCountChanged, m_screenplay,
                &Screenplay::evaluateWordCountLater, Qt::UniqueConnection);

    emit screenplayChanged();
}

void ScreenplayElement::setSceneFromID(const QString &val)
{
    m_sceneID = val;
    if (m_elementType == BreakElementType)
        return;

    if (m_screenplay == nullptr)
        return;

    ScriteDocument *document = m_screenplay->scriteDocument();
    if (document == nullptr)
        return;

    Structure *structure = document->structure();
    if (structure == nullptr)
        return;

    StructureElement *element = structure->findElementBySceneID(val);
    if (element == nullptr)
        return;

    m_elementTypeIsSet = true;
    this->setScene(element->scene());
    if (m_scene != nullptr)
        m_sceneID.clear();
}

QString ScreenplayElement::sceneID() const
{
    if (m_elementType == BreakElementType) {
        if (m_sceneID.isEmpty()) {
            switch (m_breakType) {
            case Screenplay::Act:
                return "Act";
            case Screenplay::Episode:
                return "Episode";
            case Screenplay::Interval:
                return "Interval";
            default:
                break;
            }
            return "Break";
        }

        return m_sceneID;
    }

    return m_scene ? m_scene->id() : m_sceneID;
}

void ScreenplayElement::setUserSceneNumber(const QString &val)
{
    if (m_userSceneNumber == val)
        return;

    m_userSceneNumber = val.toUpper().trimmed();
    emit userSceneNumberChanged();
}

QString ScreenplayElement::resolvedSceneNumber() const
{
    const int sn = this->sceneNumber();
    return m_userSceneNumber.isEmpty() ? (sn < 0 ? QString() : QString::number(this->sceneNumber()))
                                       : m_userSceneNumber;
}

void ScreenplayElement::setScene(Scene *val)
{
    if (m_scene == val || m_scene != nullptr || val == nullptr)
        return;

    m_scene = val;
    m_sceneID = m_scene->id();
    connect(m_scene, &Scene::aboutToDelete, this, &ScreenplayElement::resetScene);
    connect(m_scene, &Scene::sceneAboutToReset, this, &ScreenplayElement::sceneAboutToReset);
    connect(m_scene, &Scene::sceneReset, this, &ScreenplayElement::sceneReset);
    connect(m_scene, &Scene::typeChanged, this, &ScreenplayElement::sceneTypeChanged);
    connect(m_scene, &Scene::groupsChanged, this, &ScreenplayElement::onSceneGroupsChanged);
    connect(m_scene, &Scene::wordCountChanged, this, &ScreenplayElement::wordCountChanged);

    if (m_screenplay)
        connect(m_scene->heading(), &SceneHeading::enabledChanged, this,
                &ScreenplayElement::evaluateSceneNumberRequest);

    emit sceneChanged();
}

void ScreenplayElement::setExpanded(bool val)
{
    if (m_expanded == val)
        return;

    m_expanded = val;
    emit expandedChanged();
}

void ScreenplayElement::setOmitted(bool val)
{
    if (m_omitted == val || m_elementType != SceneElementType)
        return;

    m_omitted = val;
    emit omittedChanged();
}

void ScreenplayElement::setUserData(const QJsonValue &val)
{
    if (m_userData == val)
        return;

    m_userData = val;
    emit userDataChanged();
}

void ScreenplayElement::setHeightHint(qreal val)
{
    if (qFuzzyCompare(m_heightHint, val))
        return;

    m_heightHint = val;
    emit heightHintChanged();
}

void ScreenplayElement::setSelected(bool val)
{
    if (m_selected == val)
        return;

    m_selected = val;
    emit selectedChanged();
}

void ScreenplayElement::setBreakSummary(const QString &val)
{
    if (m_breakSummary == val)
        return;

    m_breakSummary = val;
    emit breakSummaryChanged();
}

void ScreenplayElement::setPageBreakAfter(bool val)
{
    if (m_pageBreakAfter == val)
        return;

    m_pageBreakAfter = val;
    emit pageBreakAfterChanged();
}

void ScreenplayElement::setPageBreakBefore(bool val)
{
    if (m_pageBreakBefore == val)
        return;

    m_pageBreakBefore = val;
    emit pageBreakBeforeChanged();
}

int ScreenplayElement::wordCount() const
{
    return m_scene.isNull() ? 0 : m_scene->wordCount();
}

bool ScreenplayElement::canSerialize(const QMetaObject *mo, const QMetaProperty &prop) const
{
    if (mo != &ScreenplayElement::staticMetaObject)
        return false;

    static const int breakSummaryPropIndex =
            ScreenplayElement::staticMetaObject.indexOfProperty("breakSummary");
    if (prop.propertyIndex() == breakSummaryPropIndex)
        return (m_elementType == BreakElementType)
                && (m_breakType == Screenplay::Act || m_breakType == Screenplay::Episode);

    static const int notesPropIndex = ScreenplayElement::staticMetaObject.indexOfProperty("notes");
    if (prop.propertyIndex() == notesPropIndex)
        return m_notes != nullptr;

    static const int attachmentsPropIndex =
            ScreenplayElement::staticMetaObject.indexOfProperty("attachments");
    if (prop.propertyIndex() == attachmentsPropIndex)
        return m_attachments != nullptr;

    return true;
}

bool ScreenplayElement::event(QEvent *event)
{
    if (event->type() == QEvent::ParentChange) {
        this->setScreenplay(qobject_cast<Screenplay *>(this->parent()));
        if (m_scene == nullptr && !m_sceneID.isEmpty())
            this->setSceneFromID(m_sceneID);
    } else if (event->type() == QEvent::DynamicPropertyChange) {
        QDynamicPropertyChangeEvent *propEvent = static_cast<QDynamicPropertyChangeEvent *>(event);
        const QByteArray propName = propEvent->propertyName();
        if (propName == QByteArrayLiteral("#sceneNumber")) {
            m_customSceneNumber = property(propName).toInt();
            emit sceneNumberChanged();
        }
    }

    return QObject::event(event);
}

void ScreenplayElement::evaluateSceneNumber(SceneNumber &number, bool minorAlso)
{
    if (minorAlso && !m_userSceneNumber.isEmpty()) {
        bool hasLetter = false;
        for (int i = m_userSceneNumber.length() - 1; i >= 0; i--) {
            if (!m_userSceneNumber.at(i).isDigit()) {
                hasLetter = true;
                break;
            }
        }

        if (!hasLetter) {
            m_userSceneNumber.clear();
            emit userSceneNumberChanged();
        }
    }

    if (m_userSceneNumber.isEmpty()) {
        int sn = -1;
        if (m_scene != nullptr && m_scene->heading()->isEnabled())
            sn = number.nextMajor();

        if (m_sceneNumber != sn) {
            m_sceneNumber = sn;
            emit sceneNumberChanged();
        }
    } else {
        number.nextMinor();

        if (minorAlso) {
            const QString sn = number.toString();
            if (sn != m_userSceneNumber) {
                m_userSceneNumber = sn;
                emit userSceneNumberChanged();
            }
        }
    }
}

void ScreenplayElement::resetScene()
{
    if (m_screenplay != nullptr)
        m_screenplay->removeElement(this);
    else {
        if (m_sceneID.isEmpty())
            m_sceneID = m_scene->id();
        m_scene = nullptr;
        this->deleteLater();
    }
}

void ScreenplayElement::resetScreenplay()
{
    if (m_screenplay != nullptr) {
        disconnect(this, &ScreenplayElement::wordCountChanged, m_screenplay,
                   &Screenplay::evaluateWordCountLater);
        m_screenplay->evaluateWordCountLater();
    }
    m_screenplay = nullptr;
    emit screenplayChanged();

    this->deleteLater();
}

void ScreenplayElement::setActIndex(int val)
{
    if (m_actIndex == val)
        return;

    m_actIndex = val;
    emit actIndexChanged();
}

void ScreenplayElement::setEpisodeIndex(int val)
{
    if (m_episodeIndex == val)
        return;

    m_episodeIndex = val;
    emit episodeIndexChanged();
}

void ScreenplayElement::setElementIndex(int val)
{
    if (m_elementIndex == val)
        return;

    m_elementIndex = val;
    emit elementIndexChanged();
}

///////////////////////////////////////////////////////////////////////////////

Screenplay::Screenplay(QObject *parent)
    : QAbstractListModel(parent),
      m_scriteDocument(qobject_cast<ScriteDocument *>(parent)),
      m_activeScene(this, "activeScene"),
      m_sceneNumberEvaluationTimer("Screenplay.m_sceneNumberEvaluationTimer")
{
    connect(this, &Screenplay::titleChanged, this, &Screenplay::emptyChanged);
    connect(this, &Screenplay::emailChanged, this, &Screenplay::emptyChanged);
    connect(this, &Screenplay::authorChanged, this, &Screenplay::emptyChanged);
    connect(this, &Screenplay::loglineChanged, this, &Screenplay::emptyChanged);
    connect(this, &Screenplay::websiteChanged, this, &Screenplay::emptyChanged);
    connect(this, &Screenplay::basedOnChanged, this, &Screenplay::emptyChanged);
    connect(this, &Screenplay::contactChanged, this, &Screenplay::emptyChanged);
    connect(this, &Screenplay::versionChanged, this, &Screenplay::emptyChanged);
    connect(this, &Screenplay::addressChanged, this, &Screenplay::emptyChanged);
    connect(this, &Screenplay::subtitleChanged, this, &Screenplay::emptyChanged);
    connect(this, &Screenplay::elementsChanged, this, &Screenplay::emptyChanged);
    connect(this, &Screenplay::phoneNumberChanged, this, &Screenplay::emptyChanged);
    connect(this, &Screenplay::emptyChanged, this, &Screenplay::screenplayChanged);
    connect(this, &Screenplay::coverPagePhotoChanged, this, &Screenplay::screenplayChanged);
    connect(this, &Screenplay::elementsChanged, this, &Screenplay::evaluateSceneNumbersLater);
    connect(this, &Screenplay::elementsChanged, this, &Screenplay::evaluateParagraphCountsLater);
    connect(this, &Screenplay::elementsChanged, this,
            &Screenplay::evaluateIfHeightHintsAreAvailableLater);
    connect(this, &Screenplay::coverPagePhotoSizeChanged, this, &Screenplay::screenplayChanged);
    connect(this, &Screenplay::titlePageIsCenteredChanged, this, &Screenplay::screenplayChanged);
    connect(this, &Screenplay::screenplayChanged, [=]() {
        this->evaluateHasTitlePageAttributes();
        this->evaluateParagraphCountsLater();
        this->evaluateWordCountLater();
        this->markAsModified();
    });

    m_version = QStringLiteral("Initial Draft");

    QSettings *settings = Application::instance()->settings();
    auto fetchSettings = [=](const QString &field, QString &into) {
        const QString value =
                settings->value(QStringLiteral("TitlePage/") + field, QString()).toString();
        if (!value.isEmpty())
            into = value;
    };
    fetchSettings(QStringLiteral("author"), m_author);
    fetchSettings(QStringLiteral("contact"), m_contact);
    fetchSettings(QStringLiteral("address"), m_address);
    fetchSettings(QStringLiteral("email"), m_email);
    fetchSettings(QStringLiteral("phone"), m_phoneNumber);
    fetchSettings(QStringLiteral("website"), m_website);

    QTimer::singleShot(100, this, [=]() {
        connect(User::instance(), &User::loggedInChanged, this, &Screenplay::authorChanged);
        connect(User::instance(), &User::infoChanged, this, &Screenplay::authorChanged);
    });

    if (m_scriteDocument != nullptr) {
        DocumentFileSystem *dfs = m_scriteDocument->fileSystem();
        connect(dfs, &DocumentFileSystem::auction, this, &Screenplay::onDfsAuction);
    }

    QClipboard *clipboard = qApp->clipboard();
    connect(clipboard, &QClipboard::dataChanged, this, &Screenplay::canPasteChanged);
}

Screenplay::~Screenplay()
{
    GarbageCollector::instance()->avoidChildrenOf(this);
    emit aboutToDelete(this);
}

QString Screenplay::standardCoverPathPhotoPath()
{
    return QLatin1String("coverPage/photo.jpg");
}

void Screenplay::setTitle(const QString &val)
{
    if (m_title == val)
        return;

    m_title = val;
    emit titleChanged();
}

void Screenplay::setSubtitle(const QString &val)
{
    if (m_subtitle == val)
        return;

    m_subtitle = val;
    emit subtitleChanged();
}

void Screenplay::setLogline(const QString &val)
{
    if (m_logline == val)
        return;

    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "logline");
    QScopedPointer<PushObjectPropertyUndoCommand> cmd;
    if (!info->isLocked())
        cmd.reset(new PushObjectPropertyUndoCommand(this, info->property));

    m_logline = val;
    emit loglineChanged();
}

void Screenplay::setLoglineComments(const QString &val)
{
    if (m_loglineComments == val)
        return;

    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "loglineComments");
    QScopedPointer<PushObjectPropertyUndoCommand> cmd;
    if (!info->isLocked())
        cmd.reset(new PushObjectPropertyUndoCommand(this, info->property));

    m_loglineComments = val;
    emit loglineCommentsChanged();
}

void Screenplay::setBasedOn(const QString &val)
{
    if (m_basedOn == val)
        return;

    m_basedOn = val;
    emit basedOnChanged();
}

void Screenplay::setAuthor(const QString &val)
{
    if (m_author == val)
        return;

    m_author = val;
    emit authorChanged();
}

QString Screenplay::author() const
{
    if (m_author.isEmpty()) {
        if (User::instance()->isLoggedIn())
            return User::instance()->info().fullName;
        return QSysInfo::machineHostName();
    }

    return m_author;
}

void Screenplay::setContact(const QString &val)
{
    if (m_contact == val)
        return;

    m_contact = val;
    emit contactChanged();
}

void Screenplay::setAddress(QString val)
{
    if (m_address == val)
        return;

    m_address = val;
    emit addressChanged();
}

void Screenplay::setPhoneNumber(QString val)
{
    if (m_phoneNumber == val)
        return;

    m_phoneNumber = val;
    emit phoneNumberChanged();
}

void Screenplay::setEmail(QString val)
{
    if (m_email == val)
        return;

    m_email = val;
    emit emailChanged();
}

void Screenplay::setWebsite(QString val)
{
    if (m_website == val)
        return;

    m_website = val;
    emit websiteChanged();
}

void Screenplay::setVersion(const QString &val)
{
    if (m_version == val)
        return;

    m_version = val;
    emit versionChanged();
}

bool Screenplay::isEmpty() const
{
    const QSettings *settings = Application::instance()->settings();
    const QString titlePageGroup = QStringLiteral("TitlePage");
    auto configuredValue = [=](const QString &key) {
        return settings->value(titlePageGroup + QStringLiteral("/") + key).toString();
    };

    bool allEmpty = m_title.isEmpty() && m_subtitle.isEmpty() && m_logline.isEmpty()
            && m_basedOn.isEmpty()
            && (m_author == QSysInfo::machineHostName()
                || m_author == configuredValue(QStringLiteral("author")))
            && (m_contact.isEmpty() || m_contact == configuredValue(QStringLiteral("contact")))
            && (m_address.isEmpty() || m_address == configuredValue(QStringLiteral("address")))
            && (m_phoneNumber.isEmpty()
                || m_phoneNumber == configuredValue(QStringLiteral("phone")))
            && (m_email.isEmpty() || m_email == configuredValue(QStringLiteral("email")))
            && (m_website.isEmpty() || m_website == configuredValue(QStringLiteral("website")))
            && (m_version == QStringLiteral("Initial Draft"));

    if (allEmpty) {
        if (m_elements.size() == 1) {
            const ScreenplayElement *firstElement = m_elements.first();
            if (firstElement->elementType() == ScreenplayElement::SceneElementType) {
                const Scene *firstScene = firstElement->scene();
                return firstScene->isEmpty();
            }
        }

        allEmpty &= m_elements.isEmpty();
    }

    return allEmpty;
}

void Screenplay::setCoverPagePhoto(const QString &val)
{
    HourGlass hourGlass;

    DocumentFileSystem *dfs = m_scriteDocument->fileSystem();
    connect(dfs, &DocumentFileSystem::auction, this, &Screenplay::onDfsAuction);

    const QSize fullHdSize(1920, 1080);
    const QString val2 = dfs->addImage(val, standardCoverPathPhotoPath(), fullHdSize);

    m_coverPagePhoto.clear();
    emit coverPagePhotoChanged();

    /*
     * We need to give some time for the QML UI to unload the previously loaded
     * image, remove that from cache and then load this new image from the disk
     * again. The reason why we need to do this is because cover page photo has
     * a standard path and doesnt change even if the cover page photo itself is
     * changed.
     *
     * This also means that the Image {} QML elements used to show cover page
     * photo must have their cache property set to false.
     */
    QTimer::singleShot(500, this, [=]() {
        m_coverPagePhoto =
                val2.isEmpty() ? val2 : m_scriteDocument->fileSystem()->absolutePath(val2);
        emit coverPagePhotoChanged();
    });
}

void Screenplay::clearCoverPagePhoto()
{
    this->setCoverPagePhoto(QString());
}

void Screenplay::setCoverPagePhotoSize(Screenplay::CoverPagePhotoSize val)
{
    if (m_coverPagePhotoSize == val)
        return;

    m_coverPagePhotoSize = val;
    emit coverPagePhotoSizeChanged();
}

void Screenplay::setTitlePageIsCentered(bool val)
{
    if (m_titlePageIsCentered == val)
        return;

    m_titlePageIsCentered = val;
    emit titlePageIsCenteredChanged();
}

bool Screenplay::hasSelectedElements() const
{
    for (ScreenplayElement *element : m_elements)
        if (element->isSelected())
            return true;

    return false;
}

int Screenplay::selectedElementsCount() const
{
    int ret = 0;

    for (ScreenplayElement *element : m_elements)
        if (element->isSelected())
            ++ret;

    return ret;
}

void Screenplay::setSelectedElementsOmitStatus(OmitStatus val)
{
    for (ScreenplayElement *element : qAsConst(m_elements)) {
        if (element->isSelected()) {
            switch (val) {
            case Omitted:
                element->setOmitted(true);
                break;
            case NotOmitted:
                element->setOmitted(false);
                break;
            default:
                break;
            }
        }
    }
}

Screenplay::OmitStatus Screenplay::selectedElementsOmitStatus() const
{
    OmitStatus ret = NotOmitted;

    for (ScreenplayElement *element : m_elements) {
        if (element->isSelected()) {
            if (element->isOmitted())
                ret = ret == NotOmitted ? Omitted : ret;
            else if (ret == Omitted)
                ret = PartiallyOmitted;
        }
    }

    return ret;
}

QQmlListProperty<ScreenplayElement> Screenplay::elements()
{
    return QQmlListProperty<ScreenplayElement>(
            reinterpret_cast<QObject *>(this), static_cast<void *>(this),
            &Screenplay::staticAppendElement, &Screenplay::staticElementCount,
            &Screenplay::staticElementAt, &Screenplay::staticClearElements);
}

void Screenplay::addElement(ScreenplayElement *ptr)
{
    this->insertElementAt(ptr, -1);
}

void Screenplay::addScene(Scene *scene)
{
    if (scene == nullptr)
        return;

    ScreenplayElement *element = new ScreenplayElement(this);
    element->setScene(scene);
    this->addElement(element);
}

static void screenplayAppendElement(Screenplay *screenplay, ScreenplayElement *ptr)
{
    screenplay->addElement(ptr);
}
static void screenplayRemoveElement(Screenplay *screenplay, ScreenplayElement *ptr)
{
    screenplay->removeElement(ptr);
}
static void screenplayInsertElement(Screenplay *screenplay, ScreenplayElement *ptr, int index)
{
    screenplay->insertElementAt(ptr, index);
}
static ScreenplayElement *screenplayElementAt(Screenplay *screenplay, int index)
{
    return screenplay->elementAt(index);
}
static int screenplayIndexOfElement(Screenplay *screenplay, ScreenplayElement *ptr)
{
    return screenplay->indexOfElement(ptr);
}

void Screenplay::insertElementAt(ScreenplayElement *ptr, int index)
{
    if (ptr == nullptr || m_elements.indexOf(ptr) >= 0)
        return;

    index = (index < 0 || index >= m_elements.size()) ? m_elements.size() : index;

    QScopedPointer<PushObjectListCommand<Screenplay, ScreenplayElement>> cmd;
    ObjectPropertyInfo *info =
            m_scriteDocument == nullptr ? nullptr : ObjectPropertyInfo::get(this, "elements");
    if (info != nullptr && !info->isLocked()) {
        ObjectListPropertyMethods<Screenplay, ScreenplayElement> methods(
                &screenplayAppendElement, &screenplayRemoveElement, &screenplayInsertElement,
                &screenplayElementAt, screenplayIndexOfElement);
        cmd.reset(new PushObjectListCommand<Screenplay, ScreenplayElement>(
                ptr, this, info->property, ObjectList::InsertOperation, methods));
    }

    this->beginInsertRows(QModelIndex(), index, index);
    if (index == m_elements.size())
        m_elements.append(ptr);
    else
        m_elements.insert(index, ptr);

    // Keep the following connections in sync with the ones we make in
    // Screenplay::setPropertyFromObjectList()
    ptr->setParent(this);
    this->connectToScreenplayElementSignals(ptr);

    this->endInsertRows();

    emit elementInserted(ptr, index);
    emit elementCountChanged();
    emit elementsChanged();

    if (/*ptr->elementType() == ScreenplayElement::SceneElementType && */
        (this->scriteDocument() && !this->scriteDocument()->isLoading()))
        this->setCurrentElementIndex(index);

    if (ptr->elementType() == ScreenplayElement::BreakElementType)
        this->updateBreakTitlesLater();
}

void Screenplay::insertElementsAt(const QList<ScreenplayElement *> &elements, int index)
{
    const int startIndex = qMin(qMax(index, 0), m_elements.size());
    const int endIndex = startIndex + elements.size() - 1;

    this->beginInsertRows(QModelIndex(), startIndex, endIndex);

    int insertIndex = startIndex;
    for (ScreenplayElement *ptr : elements) {
        ptr->setParent(this);
        this->connectToScreenplayElementSignals(ptr);
        m_elements.insert(insertIndex, ptr);
        emit elementInserted(ptr, insertIndex);
        ++insertIndex;
    }

    this->endInsertRows();
    emit elementCountChanged();
    emit elementsChanged();
}

void Screenplay::removeElement(ScreenplayElement *ptr)
{
    HourGlass hourGlass;

    if (ptr == nullptr)
        return;

    const int row = m_elements.indexOf(ptr);
    if (row < 0)
        return;

    QScopedPointer<PushObjectListCommand<Screenplay, ScreenplayElement>> cmd;
    ObjectPropertyInfo *info =
            m_scriteDocument == nullptr ? nullptr : ObjectPropertyInfo::get(this, "elements");
    if (info != nullptr && !info->isLocked()) {
        ObjectListPropertyMethods<Screenplay, ScreenplayElement> methods(
                &screenplayAppendElement, &screenplayRemoveElement, &screenplayInsertElement,
                &screenplayElementAt, screenplayIndexOfElement);
        cmd.reset(new PushObjectListCommand<Screenplay, ScreenplayElement>(
                ptr, this, info->property, ObjectList::RemoveOperation, methods));
    }

    this->beginRemoveRows(QModelIndex(), row, row);
    m_elements.removeAt(row);

    Scene *scene = ptr->scene();
    if (scene != nullptr) {
        scene->setAct(QString());
        scene->setActIndex(-1);
        scene->setEpisode(QString());
        scene->setEpisodeIndex(-1);
        scene->setScreenplayElementIndexList(QList<int>());

        // If this scene still exists as another element in the screenplay, then
        // it is going to get the above properties set in evaluateSceneNumbers() shortly.
    }

    this->disconnectFromScreenplayElementSignals(ptr);

    this->endRemoveRows();

    emit elementRemoved(ptr, row);
    emit elementCountChanged();
    emit elementsChanged();

    this->validateCurrentElementIndex();

    if (ptr->parent() == this)
        GarbageCollector::instance()->add(ptr);
}

void Screenplay::removeElements(const QList<ScreenplayElement *> &givenElements)
{
    QList<ScreenplayElement *> elements;
    std::copy_if(givenElements.begin(), givenElements.end(), std::back_inserter(elements),
                 [=](ScreenplayElement *element) { return m_elements.contains(element); });
    if (elements.isEmpty())
        return;

    if (elements.size() == 1) {
        this->removeElement(elements.first());
        return;
    }

    struct Batch
    {
        int startIndex = 0;
        int endIndex = -1;
        QList<ScreenplayElement *> elements;
        bool isValid() const
        {
            return startIndex >= 0 && endIndex >= 0 && endIndex >= startIndex
                    && elements.size() == (endIndex - startIndex + 1);
        }
    };

    QList<Batch> batches = QList<Batch>() << Batch();
    int leastIndex = INT_MAX;

    std::sort(elements.begin(), elements.end(), [=](ScreenplayElement *e1, ScreenplayElement *e2) {
        return m_elements.indexOf(e1) < m_elements.indexOf(e2);
    });
    for (ScreenplayElement *element : qAsConst(elements)) {
        const int elementIndex = m_elements.indexOf(element);
        leastIndex = qMin(elementIndex, leastIndex);
        Batch &lastBatch = batches.last();
        if (!lastBatch.isValid()) {
            lastBatch.startIndex = elementIndex;
            lastBatch.endIndex = elementIndex;
            lastBatch.elements.append(element);
        } else if (elementIndex - lastBatch.endIndex == 1) {
            lastBatch.endIndex = elementIndex;
            lastBatch.elements.append(element);
        } else {
            Batch newBatch;
            newBatch.startIndex = elementIndex;
            newBatch.endIndex = elementIndex;
            newBatch.elements.append(element);
            batches.prepend(newBatch); // we need batches to be saved in reverse order only!
        }
    }

    for (const Batch &batch : qAsConst(batches)) {
        if (!batch.isValid())
            continue;
        this->beginRemoveRows(QModelIndex(), batch.startIndex, batch.endIndex);
        for (int row = batch.endIndex; row >= batch.startIndex; row--) {
            ScreenplayElement *ptr = m_elements.takeAt(row);

            Scene *scene = ptr->scene();
            if (scene != nullptr) {
                scene->setAct(QString());
                scene->setActIndex(-1);
                scene->setEpisode(QString());
                scene->setEpisodeIndex(-1);
                scene->setScreenplayElementIndexList(QList<int>());
            }

            this->disconnectFromScreenplayElementSignals(ptr);
            emit elementRemoved(ptr, row);

            GarbageCollector::instance()->add(ptr);
        }
        this->endRemoveRows();

        leastIndex = batch.startIndex;
    }

    emit elementCountChanged();
    emit elementsChanged();
    this->validateCurrentElementIndex();

    if (leastIndex >= 0)
        this->setCurrentElementIndex(qMax(0, leastIndex - 1));
}

void Screenplay::moveElement(ScreenplayElement *ptr, int toRow)
{
    if (ptr == nullptr)
        return;

    this->clearSelection();
    ptr->setSelected(true);
    this->moveSelectedElements(toRow);
}

// TODO implement undo-command to revert group move operation
// This could be a simple pre-and-post scene id list thing.
class ScreenplayElementsMoveCommand : public QUndoCommand
{
public:
    explicit ScreenplayElementsMoveCommand(Screenplay *screenplay);
    ~ScreenplayElementsMoveCommand();

    void setMovement(const QHash<ScreenplayElement *, QPair<int, int>> &movement)
    {
        if (m_movement.isEmpty())
            m_movement = movement;
    }
    QHash<ScreenplayElement *, QPair<int, int>> movement() const { return m_movement; }

    void undo();
    void redo();

private:
    QVariantList save() const;
    bool restore(const QVariantList &array, bool forward) const;

private:
    bool m_initialized = false;
    QVariantList m_after;
    QVariantList m_before;
    QPointer<Screenplay> m_screenplay;
    QMetaObject::Connection m_connection;
    QHash<ScreenplayElement *, QPair<int, int>> m_movement;
};

ScreenplayElementsMoveCommand::ScreenplayElementsMoveCommand(Screenplay *screenplay)
    : QUndoCommand(QStringLiteral("Element Selection Move")), m_screenplay(screenplay)
{
    m_before = this->save();

    m_connection = QObject::connect(screenplay, &Screenplay::aboutToDelete, screenplay,
                                    [=]() { this->setObsolete(true); });
}

ScreenplayElementsMoveCommand::~ScreenplayElementsMoveCommand()
{
    QObject::disconnect(m_connection);
}

void ScreenplayElementsMoveCommand::undo()
{
    if (m_screenplay.isNull() || !this->restore(m_before, false))
        this->setObsolete(true);
}

void ScreenplayElementsMoveCommand::redo()
{
    if (!m_initialized) {
        m_initialized = true;
        if (m_screenplay.isNull())
            this->setObsolete(true);
        else
            m_after = this->save();
        return;
    }

    if (m_screenplay.isNull() || !this->restore(m_after, true))
        this->setObsolete(true);
}

QVariantList ScreenplayElementsMoveCommand::save() const
{
    HourGlass hourGlass;

    QVariantList ret;
    if (m_screenplay.isNull())
        return ret;

    for (int i = 0; i < m_screenplay->elementCount(); i++) {
        ScreenplayElement *element = m_screenplay->elementAt(i);
        QVariant item;
        if (element->elementType() == ScreenplayElement::BreakElementType)
            item = QVariantList({ element->breakType(), false });
        else
            item = QVariantList({ element->sceneID(), element->isSelected() });
        ret << item;
    }

    return ret;
}

bool ScreenplayElementsMoveCommand::restore(const QVariantList &array, bool forward) const
{
    HourGlass hourGlass;

    QList<ScreenplayElement *> elements = m_screenplay->getElements();
    if (array.size() != elements.size())
        return false;

    auto findSceneElement = [&elements](const QString &id) {
        for (int i = 0; i < elements.size(); i++) {
            ScreenplayElement *element = elements.at(i);
            if (element->sceneID() == id) {
                elements.takeAt(i);
                return element;
            }
        }

        return (ScreenplayElement *)nullptr;
    };

    auto findBreakElement = [&elements](int type) {
        for (int i = 0; i < elements.size(); i++) {
            ScreenplayElement *element = elements.at(i);
            if (element->elementType() == ScreenplayElement::BreakElementType
                && element->breakType() == type) {
                elements.takeAt(i);
                return element;
            }
        }
        return (ScreenplayElement *)nullptr;
    };

    int currentIndex = -1;
    QList<ScreenplayElement *> newElements;
    for (const QVariant &item : array) {
        const QVariantList itemData = item.toList();
        const QVariant elementData = itemData.first();
        const bool elementIsSelected = itemData.last().toBool();

        ScreenplayElement *element = nullptr;

        if (elementData.userType() == QMetaType::QString)
            element = findSceneElement(elementData.toString());
        else
            element = findBreakElement(elementData.toInt());

        if (element == nullptr)
            return false;

        if (elementIsSelected)
            currentIndex =
                    currentIndex < 0 ? newElements.size() : qMin(currentIndex, newElements.size());

        newElements.append(element);
        element->setSelected(elementIsSelected);
    }

    if (newElements.size() != array.size() || !elements.isEmpty())
        return false;

    const bool ret = m_screenplay->setElements(newElements);
    if (ret && currentIndex >= 0) {
        auto it = m_movement.begin();
        auto end = m_movement.end();
        while (it != end) {
            const int from = forward ? it.value().first : it.value().second;
            const int to = forward ? it.value().second : it.value().first;
            emit m_screenplay->elementMoved(it.key(), from, to);
            ++it;
        }

        QTimer::singleShot(50, m_screenplay, [=]() {
            m_screenplay->setCurrentElementIndex(currentIndex);
            emit m_screenplay->requestEditorAt(currentIndex);
        });
    }

    return ret;
}

void Screenplay::moveSelectedElements(int toRow)
{
    HourGlass hourGlass;

    toRow = qBound(0, toRow, m_elements.size());

    /**
     * Why are we resetting the models while moving multiple elements, instead of removing and *
     * inserting them? Or better yet, moving them?
     *
     * The ScreenplayTextDocument class was built with the assumption that elements will be
     * added, removed or moved one at a time. So, if we removed all selected elements and
     * reinserted them elsewhere; the text document wont get updated properly.
     *
     * But when we reset the model, it will simply update the whole screenplay at once. This
     * could be a bit slow, but is still far better than moving one scene at a time.
     */
    ScreenplayElementsMoveCommand *cmd = nullptr;

    ScreenplayElement *toRowElement = this->elementAt(toRow);
    if (toRowElement && toRowElement->isSelected())
        return;

    emit aboutToMoveElements(toRow);

    QList<ScreenplayElement *> selectedElements;
    QHash<ScreenplayElement *, QPair<int, int>> movement;
    for (int i = m_elements.size() - 1; i >= 0; i--) {
        ScreenplayElement *element = m_elements.at(i);
        if (!element->isSelected())
            continue;

        if (cmd == nullptr) {
            cmd = new ScreenplayElementsMoveCommand(this);
            this->beginResetModel();
        }

        selectedElements.prepend(element);
        movement[element] = qMakePair(i, 0);
        m_elements.removeAt(i);
    }

    if (cmd == nullptr)
        return;

    toRow = toRowElement ? m_elements.indexOf(toRowElement) : -1;
    if (toRow < 0)
        toRow = m_elements.size();

    while (!selectedElements.isEmpty()) {
        ScreenplayElement *element = selectedElements.takeLast();
        m_elements.insert(toRow, element);
        movement[element].second = toRow + selectedElements.size();
    }

    this->endResetModel();

    emit elementsChanged();

    this->updateBreakTitlesLater();

    cmd->setMovement(movement);

    auto it = movement.begin();
    auto end = movement.end();
    while (it != end) {
        emit elementMoved(it.key(), it.value().first, it.value().second);
        ++it;
    }

    QTimer::singleShot(50, this, [=]() {
        this->setCurrentElementIndex(toRow);
        emit this->requestEditorAt(toRow);
    });

    if (UndoStack::active())
        UndoStack::active()->push(cmd);
    else
        delete cmd;
}

// TODO implement undo-command to revert group remove operation
// This could be a simple pre-and-post scene id list thing.

class ScreenplayRemoveElementsUndoCommand : public QUndoCommand
{
public:
    explicit ScreenplayRemoveElementsUndoCommand(Screenplay *screenplay);
    ~ScreenplayRemoveElementsUndoCommand() { }

    int id() const { return -1; }
    void undo();
    void redo();

private:
    QJsonArray save() const;

private:
    bool m_initialized = false;

    int m_afterCurrentIndex = -1;
    QJsonArray m_after;

    int m_beforeCurrentIndex = -1;
    QJsonArray m_before;

    QPointer<Screenplay> m_screenplay;
};

ScreenplayRemoveElementsUndoCommand::ScreenplayRemoveElementsUndoCommand(Screenplay *screenplay)
    : QUndoCommand(QStringLiteral("Remove Scenes From Screenplay")), m_screenplay(screenplay)
{
    m_before = this->save();
    m_afterCurrentIndex = m_screenplay.isNull() ? -1 : m_screenplay->currentElementIndex();
}

void ScreenplayRemoveElementsUndoCommand::undo()
{
    if (m_screenplay.isNull()) {
        this->setObsolete(true);
        return;
    }

    HourGlass hourGlass;

    QList<ScreenplayElement *> elements = m_screenplay->getElements();
    if (m_before.size() == elements.size()) {
        this->setObsolete(true);
        return;
    }

    for (int i = 0; i < m_before.size(); i++) {
        const QJsonValue item = m_before.at(i);
        const QJsonObject elementJson = item.toObject();
        const QString sceneID = elementJson.value(QStringLiteral("sceneID")).toString();

        ScreenplayElement *element = elements.isEmpty() ? nullptr : elements.first();
        if (element && element->sceneID() == sceneID) {
            elements.takeFirst();
            continue;
        }

        element = new ScreenplayElement(m_screenplay);
        QObjectSerializer::fromJson(elementJson, element);
        m_screenplay->insertElementAt(element, i);
    }

    m_screenplay->setCurrentElementIndex(m_beforeCurrentIndex);
    if (m_beforeCurrentIndex >= 0)
        emit m_screenplay->requestEditorAt(m_beforeCurrentIndex);

    Structure *structure = m_screenplay->scriteDocument()->structure();
    if (structure && structure->isForceBeatBoardLayout())
        structure->placeElementsInBeatBoardLayout(m_screenplay);
}

void ScreenplayRemoveElementsUndoCommand::redo()
{
    if (m_screenplay.isNull()) {
        this->setObsolete(true);
        return;
    }

    if (!m_initialized) {
        m_after = this->save();
        m_afterCurrentIndex = m_screenplay.isNull() ? -1 : m_screenplay->currentElementIndex();
        return;
    }

    const QList<ScreenplayElement *> elements = m_screenplay->getElements();
    if (m_after.size() == elements.size()) {
        this->setObsolete(true);
        return;
    }

    QJsonArray array = m_after;
    for (ScreenplayElement *element : elements) {
        const QJsonValue item = m_after.isEmpty() ? QJsonValue() : m_after.first();
        const QJsonObject elementJson = item.toObject();
        const QString sceneID = elementJson.value(QStringLiteral("sceneID")).toString();

        if (element->sceneID() == sceneID) {
            array.takeAt(0);
            continue;
        }

        m_screenplay->removeElement(element);
    }

    m_screenplay->setCurrentElementIndex(m_afterCurrentIndex);
    if (m_afterCurrentIndex >= 0)
        emit m_screenplay->requestEditorAt(m_afterCurrentIndex);

    Structure *structure = m_screenplay->scriteDocument()->structure();
    if (structure && structure->isForceBeatBoardLayout())
        structure->placeElementsInBeatBoardLayout(m_screenplay);
}

QJsonArray ScreenplayRemoveElementsUndoCommand::save() const
{
    HourGlass hourGlass;

    QJsonArray ret;
    if (m_screenplay.isNull())
        return ret;

    for (int i = 0; i < m_screenplay->elementCount(); i++) {
        const ScreenplayElement *element = m_screenplay->elementAt(i);
        const QJsonObject elementJson = QObjectSerializer::toJson(element);
        ret.append(elementJson);
    }

    return ret;
}

void Screenplay::removeSelectedElements()
{
    HourGlass hourGlass;

    int firstSelectedIndex = -1;
    QList<ScreenplayElement *> selectedElements;
    for (ScreenplayElement *element : qAsConst(m_elements)) {
        if (element->isSelected()) {
            selectedElements.append(element);
            if (firstSelectedIndex < 0)
                firstSelectedIndex = m_elements.indexOf(element);
        }
    }

    ObjectPropertyInfo *info =
            m_scriteDocument == nullptr ? nullptr : ObjectPropertyInfo::get(this, "elements");
    if (info)
        info->lock();

    ScreenplayRemoveElementsUndoCommand *cmd = new ScreenplayRemoveElementsUndoCommand(this);
    for (ScreenplayElement *element : qAsConst(selectedElements))
        this->removeElement(element);

    if (UndoStack::active())
        UndoStack::active()->push(cmd);
    else
        delete cmd;

    if (info)
        info->unlock();

    if (firstSelectedIndex >= 0)
        this->setCurrentElementIndex(qMax(0, firstSelectedIndex - 1));
}

void Screenplay::omitSelectedElements()
{
    for (ScreenplayElement *element : qAsConst(m_elements)) {
        if (element->isSelected())
            element->setOmitted(true);
    }
}

void Screenplay::includeSelectedElements()
{
    for (ScreenplayElement *element : qAsConst(m_elements)) {
        if (element->isSelected())
            element->setOmitted(false);
    }
}

void Screenplay::clearSelection()
{
    for (ScreenplayElement *element : qAsConst(m_elements))
        element->setSelected(false);
}

QList<int> Screenplay::selectedElementIndexes() const
{
    QList<int> ret;

    for (ScreenplayElement *element : qAsConst(m_elements)) {
        if (element->isSelected())
            ret << element->elementIndex();
    }

    return ret;
}

void Screenplay::setSelection(const QList<ScreenplayElement *> &elements)
{
    this->clearSelection();
    if (elements.isEmpty())
        return;

    for (ScreenplayElement *ptr : elements)
        ptr->setSelected(true);

    const int index = m_elements.indexOf(elements.first());
    if (index >= 0)
        this->setCurrentElementIndex(index);
}

ScreenplayElement *Screenplay::elementAt(int index) const
{
    return index < 0 || index >= m_elements.size() ? nullptr : m_elements.at(index);
}

ScreenplayElement *Screenplay::elementWithIndex(int index) const
{
    for (ScreenplayElement *ret : qAsConst(m_elements)) {
        if (ret->elementIndex() == index)
            return ret;
    }

    return nullptr;
}

int Screenplay::elementCount() const
{
    return m_elements.size();
}

class UndoClearScreenplayCommand : public QUndoCommand
{
public:
    explicit UndoClearScreenplayCommand(Screenplay *screenplay, const QStringList &sceneIds);
    ~UndoClearScreenplayCommand();

    // QUndoCommand interface
    void undo();
    void redo();

private:
    bool m_firstRedoDone = false;
    char m_padding[7];
    QStringList m_sceneIds;
    Screenplay *m_screenplay = nullptr;
    QMetaObject::Connection m_connection;
};

UndoClearScreenplayCommand::UndoClearScreenplayCommand(Screenplay *screenplay,
                                                       const QStringList &sceneIds)
    : QUndoCommand(), m_sceneIds(sceneIds), m_screenplay(screenplay)
{
    m_padding[0] = 0; // just to get rid of the unused private variable warning.

    m_connection = QObject::connect(m_screenplay, &Screenplay::destroyed,
                                    [this]() { this->setObsolete(true); });
}

UndoClearScreenplayCommand::~UndoClearScreenplayCommand()
{
    QObject::disconnect(m_connection);
}

void UndoClearScreenplayCommand::undo()
{
    ObjectPropertyInfo *info = ObjectPropertyInfo::get(m_screenplay, "elements");
    if (info)
        info->lock();

    ScriteDocument *document = m_screenplay->scriteDocument();
    Structure *structure = document->structure();
    for (const QString &sceneId : qAsConst(m_sceneIds)) {
        StructureElement *element = structure->findElementBySceneID(sceneId);
        if (element == nullptr)
            continue;

        Scene *scene = element->scene();
        ScreenplayElement *screenplayElement = new ScreenplayElement(m_screenplay);
        screenplayElement->setScene(scene);
        m_screenplay->addElement(screenplayElement);
    }

    if (info)
        info->unlock();
}

void UndoClearScreenplayCommand::redo()
{
    if (!m_firstRedoDone) {
        m_firstRedoDone = true;
        return;
    }

    ObjectPropertyInfo *info = ObjectPropertyInfo::get(m_screenplay, "elements");
    if (info)
        info->lock();

    while (m_screenplay->elementCount())
        m_screenplay->removeElement(m_screenplay->elementAt(0));

    if (info)
        info->unlock();
}

void Screenplay::clearElements()
{
    ObjectPropertyInfo *info = ObjectPropertyInfo::get(this, "elements");
    if (info)
        info->lock();

    this->beginResetModel();

    QStringList sceneIds;
    while (m_elements.size()) {
        sceneIds << m_elements.first()->sceneID();
        // this->removeElement(m_elements.first());

        ScreenplayElement *ptr = m_elements.takeLast();
        emit elementRemoved(ptr, m_elements.size());
        disconnect(ptr, nullptr, this, nullptr);

        Scene *scene = ptr->scene();
        if (scene != nullptr) {
            scene->setAct(QString());
            scene->setActIndex(-1);
            scene->setEpisode(QString());
            scene->setEpisodeIndex(-1);
            scene->setScreenplayElementIndexList(QList<int>());
        }

        GarbageCollector::instance()->add(ptr);
    }

    this->endResetModel();

    emit elementCountChanged();
    emit elementsChanged();
    this->evaluateSceneNumbersLater();
    this->validateCurrentElementIndex();

    if (UndoStack::active())
        UndoStack::active()->push(new UndoClearScreenplayCommand(this, sceneIds));

    if (info)
        info->unlock();
}

void Screenplay::gatherSelectedScenes(SceneGroup *into)
{
    into->clearScenes();

    for (ScreenplayElement *element : qAsConst(m_elements)) {
        if (element->isSelected())
            into->addScene(element->scene());
    }
}

class SplitElementUndoCommand : public QUndoCommand
{
public:
    explicit SplitElementUndoCommand(ScreenplayElement *ptr);
    ~SplitElementUndoCommand();

    void prepare();
    void commit(Scene *splitScene);

    // QUndoCommand interface
    void undo();
    void redo();

private:
    QByteArray captureScreenplayElements() const;
    void applyScreenplayElements(const QByteArray &bytes);

    QPair<StructureElement *, StructureElement *> findStructureElements() const;

private:
    QString m_splitSceneID;
    QString m_originalSceneID;
    QByteArray m_splitScenesData[2];
    QByteArray m_originalSceneData;
    QList<int> m_splitElementIndexes;
    Screenplay *m_screenplay = nullptr;
    ScreenplayElement *m_screenplayElement = nullptr;
};

SplitElementUndoCommand::SplitElementUndoCommand(ScreenplayElement *ptr)
    : QUndoCommand(), m_screenplayElement(ptr)
{
}

SplitElementUndoCommand::~SplitElementUndoCommand() { }

void SplitElementUndoCommand::prepare()
{
    if (m_screenplayElement == nullptr) {
        this->setObsolete(true);
        return;
    }

    m_screenplay = m_screenplayElement->screenplay();

    Scene *originalScene = m_screenplayElement->scene();
    m_originalSceneID = originalScene->id();
    m_originalSceneData = originalScene->toByteArray();
}

void SplitElementUndoCommand::commit(Scene *splitScene)
{
    if (m_screenplayElement == nullptr || splitScene == nullptr || m_screenplay == nullptr) {
        this->setObsolete(true);
        return;
    }

    m_splitSceneID = splitScene->id();
    m_splitScenesData[0] = m_screenplayElement->scene()->toByteArray();
    m_splitScenesData[1] = splitScene->toByteArray();
    UndoStack::active()->push(this);
}

void SplitElementUndoCommand::undo()
{
    QScopedValueRollback<bool> undoLock(UndoStack::ignoreUndoCommands, true);

    QPair<StructureElement *, StructureElement *> pair = this->findStructureElements();
    if (pair.first == nullptr || pair.second == nullptr) {
        this->setObsolete(true);
        return;
    }

    m_screenplay->scriteDocument()->setBusyMessage("Performing undo of split-scene operation...");

    Structure *structure = m_screenplay->scriteDocument()->structure();
    Scene *splitScene = pair.second->scene();
    Scene *originalScene = pair.first->scene();

    // Reset our screenplay first, one of the scenes that it refers to is about to be destroyed.
    for (int index : qAsConst(m_splitElementIndexes))
        m_screenplay->removeElement(m_screenplay->elementAt(index));

    // Destroy the split scene
    GarbageCollector::instance()->add(splitScene);
    structure->removeElement(pair.second);

    // Restore Original Scene to its original state
    originalScene->resetFromByteArray(m_originalSceneData);

    m_screenplay->scriteDocument()->clearBusyMessage();
}

void SplitElementUndoCommand::redo()
{
    if (m_splitElementIndexes.isEmpty()) {
        if (m_screenplayElement == nullptr) {
            this->setObsolete(true);
            return;
        }

        for (int i = m_screenplay->elementCount() - 1; i >= 0; i--) {
            ScreenplayElement *element = m_screenplay->elementAt(i);
            if (element->sceneID() == m_splitSceneID)
                m_splitElementIndexes.append(i);
        }

        m_screenplayElement = nullptr;

        return;
    }

    QScopedValueRollback<bool> undoLock(UndoStack::ignoreUndoCommands, true);

    QPair<StructureElement *, StructureElement *> pair = this->findStructureElements();
    if (pair.first == nullptr || pair.second != nullptr) {
        this->setObsolete(true);
        return;
    }

    m_screenplay->scriteDocument()->setBusyMessage("Performing redo of split-scene operation...");

    Structure *structure = m_screenplay->scriteDocument()->structure();

    // Create the split scene first
    StructureElement *splitStructureElement = new StructureElement(structure);
    splitStructureElement->setX(pair.first->x() + 300);
    splitStructureElement->setY(pair.first->y() + 80);
    Scene *splitScene = new Scene(splitStructureElement);
    splitScene->setId(m_splitSceneID);
    splitScene->resetFromByteArray(m_splitScenesData[1]);
    splitStructureElement->setScene(splitScene);
    structure->insertElement(splitStructureElement, structure->indexOfElement(pair.first) + 1);

    // Update original scene with its split data
    Scene *originalScene = pair.first->scene();
    originalScene->resetFromByteArray(m_splitScenesData[0]);

    // Reset our screenplay now
    for (int index : qAsConst(m_splitElementIndexes)) {
        ScreenplayElement *element = new ScreenplayElement(m_screenplay);
        element->setElementType(ScreenplayElement::SceneElementType);
        element->setScene(splitScene);
        m_screenplay->insertElementAt(element, index);
    }

    m_screenplay->scriteDocument()->clearBusyMessage();
}

QByteArray SplitElementUndoCommand::captureScreenplayElements() const
{
    QByteArray bytes;

    QDataStream ds(&bytes, QIODevice::WriteOnly);
    ds << m_screenplay->elementCount();
    for (int i = 0; i < m_screenplay->elementCount(); i++) {
        ScreenplayElement *element = m_screenplay->elementAt(i);
        ds << int(element->elementType());
        ds << element->sceneID();
    }

    ds << m_screenplay->currentElementIndex();
    return bytes;
}

void SplitElementUndoCommand::applyScreenplayElements(const QByteArray &bytes)
{
    if (bytes.isEmpty()) {
        this->setObsolete(true);
        return;
    }

    QDataStream ds(bytes);

    int nrElements = 0;
    ds >> nrElements;

    m_screenplay->clearElements();

    for (int i = 0; i < nrElements; i++) {
        ScreenplayElement *element = new ScreenplayElement(m_screenplay);

        int type = -1;
        ds >> type;
        element->setElementType(ScreenplayElement::ElementType(type));

        QString sceneID;
        ds >> sceneID;
        element->setSceneFromID(sceneID);

        m_screenplay->addElement(element);
    }

    int currentIndex = -1;
    ds >> currentIndex;
    m_screenplay->setCurrentElementIndex(currentIndex);
}

QPair<StructureElement *, StructureElement *> SplitElementUndoCommand::findStructureElements() const
{
    Structure *structure = m_screenplay->scriteDocument()->structure();

    StructureElement *splitSceneStructureElement = structure->findElementBySceneID(m_splitSceneID);
    if (splitSceneStructureElement == nullptr || splitSceneStructureElement->scene() == nullptr)
        splitSceneStructureElement = nullptr;

    StructureElement *originalSceneStructureElement =
            structure->findElementBySceneID(m_originalSceneID);
    if (originalSceneStructureElement == nullptr
        || originalSceneStructureElement->scene() == nullptr)
        originalSceneStructureElement = nullptr;

    return qMakePair(originalSceneStructureElement, splitSceneStructureElement);
}

ScreenplayElement *Screenplay::splitElement(ScreenplayElement *ptr, SceneElement *element,
                                            int textPosition)
{
    ScreenplayElement *ret = nullptr;
    QScopedPointer<SplitElementUndoCommand> undoCommand(new SplitElementUndoCommand(ptr));

    {
        QScopedValueRollback<bool> undoLock(UndoStack::ignoreUndoCommands, true);

        if (ptr == nullptr)
            return ret;

        const int index = this->indexOfElement(ptr);
        if (index < 0)
            return ret;

        if (ptr->elementType() == ScreenplayElement::BreakElementType)
            return ret;

        Scene *scene = ptr->scene();
        if (scene == nullptr)
            return ret;

        undoCommand->prepare();

        Structure *structure = this->scriteDocument()->structure();
        StructureElement *structureElement = structure->findElementBySceneID(scene->id());
        if (structureElement == nullptr)
            return ret;

        StructureElement *newStructureElement =
                structure->splitElement(structureElement, element, textPosition);
        if (newStructureElement == nullptr)
            return ret;

        for (int i = this->elementCount() - 1; i >= 0; i--) {
            ScreenplayElement *screenplayElement = this->elementAt(i);
            if (screenplayElement->scene() == scene) {
                ScreenplayElement *newScreenplayElement = new ScreenplayElement(this);
                newScreenplayElement->setScene(newStructureElement->scene());
                this->insertElementAt(newScreenplayElement, i + 1);

                if (screenplayElement == ptr)
                    ret = newScreenplayElement;
            }
        }

        this->setCurrentElementIndex(this->indexOfElement(ret));

        ptr->setHeightHint(0);
    }

    if (ret != nullptr && UndoStack::active() != nullptr) {
        undoCommand->commit(ret->scene());
        undoCommand.take();
    }

    return ret;
}

ScreenplayElement *Screenplay::mergeElementWithPrevious(ScreenplayElement *element)
{
    /* We dont capture undo for this action, because user can always split scene once again */
    QScopedValueRollback<bool> undoLock(UndoStack::ignoreUndoCommands, true);
    Screenplay *screenplay = this;

    if (element == nullptr || element->scene() == nullptr)
        return nullptr;

    Scene *currentScene = element->scene();
    int previousElementIndex = screenplay->indexOfElement(element) - 1;
    while (previousElementIndex >= 0) {
        ScreenplayElement *element = screenplay->elementAt(previousElementIndex);
        if (element == nullptr || element->scene() == nullptr) {
            --previousElementIndex;
            continue;
        }

        break;
    }

    if (previousElementIndex < 0)
        return nullptr;

    ScreenplayElement *previousSceneElement = screenplay->elementAt(previousElementIndex);
    Scene *previousScene = previousSceneElement->scene();
    currentScene->mergeInto(previousScene);

    previousSceneElement->setHeightHint(0);
    screenplay->setCurrentElementIndex(previousElementIndex);
    GarbageCollector::instance()->add(element);
    return previousSceneElement;
}

void Screenplay::removeSceneElements(Scene *scene)
{
    if (scene == nullptr)
        return;

    for (int i = m_elements.size() - 1; i >= 0; i--) {
        ScreenplayElement *ptr = m_elements.at(i);
        if (ptr->scene() == scene)
            this->removeElement(ptr);
    }
}

int Screenplay::firstIndexOfScene(Scene *scene) const
{
    const QList<int> indexes = this->sceneElementIndexes(scene, 1);
    return indexes.isEmpty() ? -1 : indexes.first();
}

int Screenplay::indexOfElement(ScreenplayElement *element) const
{
    return m_elements.indexOf(element);
}

QList<int> Screenplay::sceneElementIndexes(Scene *scene, int max) const
{
    QList<int> ret;
    if (scene == nullptr || max == 0)
        return ret;

    ret = scene->screenplayElementIndexList();
    if (max < 0 || max >= ret.size() || ret.isEmpty())
        return ret;

    if (max == 1)
        return QList<int>() << ret.first();

    while (ret.size() > max)
        ret.removeLast();

    return ret;
}

QList<ScreenplayElement *> Screenplay::sceneElements(Scene *scene, int max) const
{
    const QList<int> indexes = this->sceneElementIndexes(scene, max);
    QList<ScreenplayElement *> elements;

    if (indexes.isEmpty()) {
        for (ScreenplayElement *element : qAsConst(m_elements)) {
            if (element->scene() == scene)
                elements << element;
        }
    } else {
        for (int idx : indexes)
            elements << m_elements.at(idx);
    }

    return elements;
}

int Screenplay::firstSceneIndex() const
{
    int index = 0;
    while (index < m_elements.size()) {
        ScreenplayElement *element = m_elements.at(index);
        if (element->scene() != nullptr)
            return index;

        ++index;
    }

    return -1;
}

int Screenplay::lastSceneIndex() const
{
    int index = m_elements.size() - 1;
    while (index >= 0) {
        ScreenplayElement *element = m_elements.at(index);
        if (element->scene() != nullptr)
            return index;

        --index;
    }

    return -1;
}

QList<int> Screenplay::sceneElementsInBreak(ScreenplayElement *element) const
{
    QList<int> ret;

    int _index = this->indexOfElement(element);

    while (1) {
        ScreenplayElement *_element = this->elementAt(++_index);
        if (_element == nullptr)
            break;

        if (_element->elementType() == ScreenplayElement::BreakElementType) {
            if (_element->breakType() == element->breakType())
                break;
        } else
            ret << _index;
    }

    return ret;
}

int Screenplay::dialogueCount() const
{
    int ret = 0;
    for (const ScreenplayElement *element : qAsConst(m_elements)) {
        const Scene *scene = element->scene();
        if (scene == nullptr)
            continue;

        const int nrParas = scene->elementCount();
        for (int i = 0; i < nrParas; i++)
            ret += scene->elementAt(i)->type() == SceneElement::Character ? 1 : 0;
    }

    return ret;
}

QList<ScreenplayElement *>
Screenplay::getFilteredElements(std::function<bool(ScreenplayElement *)> filterFunc) const
{
    const QList<ScreenplayElement *> allElements = m_elements;
    QList<ScreenplayElement *> ret;
    for (ScreenplayElement *element : allElements)
        if (filterFunc(element))
            ret.append(element);
    return ret;
}

bool Screenplay::setElements(const QList<ScreenplayElement *> &list)
{
    // Works only if the elements in the list supplied as parameters
    // is just a reordered list of elements already in the screenplay.
    if (list == m_elements)
        return true;

    QList<ScreenplayElement *> copy = m_elements;
    for (ScreenplayElement *element : list) {
        const int index = copy.isEmpty() ? -1 : copy.indexOf(element);
        if (index < 0)
            return false;
        copy.takeAt(index);
    }

    if (!copy.isEmpty())
        return false;

    copy = m_elements;

    this->beginResetModel();
    m_elements = list;
    this->endResetModel();

    emit elementsChanged();

    return true;
}

void Screenplay::addBreakElement(Screenplay::BreakType type)
{
    this->insertBreakElement(type, -1);
}

void Screenplay::insertBreakElement(Screenplay::BreakType type, int index)
{
    ScreenplayElement *element = new ScreenplayElement(this);
    element->setElementType(ScreenplayElement::BreakElementType);
    element->setBreakType(type);
    this->insertElementAt(element, index);
}

void Screenplay::updateBreakTitles()
{
    if (this->property("#avoidUpdateBreakTitles").toBool())
        return;

    QStringList actNames;
    if (this->scriteDocument() != nullptr) {
        Structure *structure = this->scriteDocument()->structure();
        const QString category = structure->preferredGroupCategory();
        actNames = structure->categoryActNames().value(category).toStringList();
    }

    QList<ScreenplayElement *> episodes;
    QList<ScreenplayElement *> episodeActs;
    QList<ScreenplayElement *> episodeIntervals;

    int episodeOffset = 0;
    int actOffset = 0;

    for (ScreenplayElement *e : qAsConst(m_elements)) {
        if (e->elementType() != ScreenplayElement::BreakElementType) {
            if (episodeOffset == 0 && episodes.isEmpty() && e->scene()->heading()->isEnabled())
                ++episodeOffset;

            if (actOffset == 0 && episodeActs.isEmpty() && e->scene()->heading()->isEnabled())
                ++actOffset;

            continue;
        }

        switch (e->breakType()) {
        case Screenplay::Episode:
            episodeActs.clear();
            episodeIntervals.clear();
            episodes.append(e);
            actOffset = 0;
            e->setBreakTitle(QStringLiteral("EPISODE ")
                             + QString::number(episodes.size() + episodeOffset));
            break;
        case Screenplay::Act:
            episodeActs.append(e);
            e->setBreakTitle(episodeActs.size() + actOffset > actNames.size()
                                     ? QStringLiteral("ACT ")
                                             + QString::number(episodeActs.size() + actOffset)
                                     : actNames.at(episodeActs.size() + actOffset - 1));
            break;
        case Screenplay::Interval:
            episodeIntervals.append(e);
            e->setBreakTitle(QStringLiteral("INTERVAL ")
                             + QString::number(episodeIntervals.size()));
            break;
        }
    }

    this->evaluateSceneNumbers();
}

void Screenplay::setActCount(int val)
{
    if (m_actCount == val)
        return;

    m_actCount = val;
    emit actCountChanged();
}

void Screenplay::setSceneCount(int val)
{
    if (m_sceneCount == val)
        return;

    m_sceneCount = val;
    emit sceneCountChanged();
}

void Screenplay::setEpisodeCount(int val)
{
    if (m_episodeCount == val)
        return;

    m_episodeCount = val;
    emit episodeCountChanged();
}

void Screenplay::onDfsAuction(const QString &filePath, int *claims)
{
    if (filePath == standardCoverPathPhotoPath())
        *claims = *claims + 1;
}

void Screenplay::connectToScreenplayElementSignals(ScreenplayElement *ptr)
{
    if (ptr == nullptr)
        return;

    connect(ptr, &ScreenplayElement::elementChanged, this, &Screenplay::screenplayChanged,
            Qt::UniqueConnection);
    connect(ptr, &ScreenplayElement::aboutToDelete, this, &Screenplay::removeElement,
            Qt::UniqueConnection);
    connect(ptr, &ScreenplayElement::sceneReset, this, &Screenplay::onSceneReset,
            Qt::UniqueConnection);
    connect(ptr, &ScreenplayElement::evaluateSceneNumberRequest, this,
            &Screenplay::evaluateSceneNumbersLater, Qt::UniqueConnection);
    connect(ptr, &ScreenplayElement::sceneTypeChanged, this, &Screenplay::evaluateSceneNumbersLater,
            Qt::UniqueConnection);
    connect(ptr, &ScreenplayElement::sceneGroupsChanged, this,
            &Screenplay::elementSceneGroupsChanged, Qt::UniqueConnection);
    connect(ptr, &ScreenplayElement::elementTypeChanged, this, &Screenplay::updateBreakTitlesLater,
            Qt::UniqueConnection);
    connect(ptr, &ScreenplayElement::breakTypeChanged, this, &Screenplay::updateBreakTitlesLater,
            Qt::UniqueConnection);
    connect(ptr, &ScreenplayElement::breakTitleChanged, this, &Screenplay::breakTitleChanged,
            Qt::UniqueConnection);
    connect(ptr, &ScreenplayElement::selectedChanged, this, &Screenplay::hasSelectedElementsChanged,
            Qt::UniqueConnection);
    connect(ptr, &ScreenplayElement::selectedChanged, this,
            &Screenplay::selectedElementsCountChanged, Qt::UniqueConnection);
    connect(ptr, &ScreenplayElement::heightHintChanged, this,
            &Screenplay::evaluateIfHeightHintsAreAvailableLater, Qt::UniqueConnection);
    connect(ptr, &ScreenplayElement::omittedChanged, this,
            &Screenplay::onScreenplayElementOmittedChanged, Qt::UniqueConnection);
    // connect(ptr, &ScreenplayElement::selectedChanged, this,
    //         &Screenplay::onScreenplayElementOmittedChanged, Qt::UniqueConnection);
}

void Screenplay::disconnectFromScreenplayElementSignals(ScreenplayElement *ptr)
{
    if (ptr == nullptr)
        return;

    disconnect(ptr, &ScreenplayElement::elementChanged, this, &Screenplay::screenplayChanged);
    disconnect(ptr, &ScreenplayElement::aboutToDelete, this, &Screenplay::removeElement);
    disconnect(ptr, &ScreenplayElement::sceneReset, this, &Screenplay::onSceneReset);
    disconnect(ptr, &ScreenplayElement::evaluateSceneNumberRequest, this,
               &Screenplay::evaluateSceneNumbersLater);
    disconnect(ptr, &ScreenplayElement::sceneTypeChanged, this,
               &Screenplay::evaluateSceneNumbersLater);
    disconnect(ptr, &ScreenplayElement::sceneGroupsChanged, this,
               &Screenplay::elementSceneGroupsChanged);
    disconnect(ptr, &ScreenplayElement::elementTypeChanged, this,
               &Screenplay::updateBreakTitlesLater);
    disconnect(ptr, &ScreenplayElement::breakTypeChanged, this,
               &Screenplay::updateBreakTitlesLater);
    disconnect(ptr, &ScreenplayElement::breakTitleChanged, this, &Screenplay::breakTitleChanged);
    disconnect(ptr, &ScreenplayElement::selectedChanged, this,
               &Screenplay::hasSelectedElementsChanged);
    disconnect(ptr, &ScreenplayElement::selectedChanged, this,
               &Screenplay::selectedElementsCountChanged);
    disconnect(ptr, &ScreenplayElement::heightHintChanged, this,
               &Screenplay::evaluateIfHeightHintsAreAvailableLater);
    disconnect(ptr, &ScreenplayElement::omittedChanged, this,
               &Screenplay::onScreenplayElementOmittedChanged);
    // disconnect(ptr, &ScreenplayElement::selectedChanged, this,
    //            &Screenplay::onScreenplayElementOmittedChanged);
}

void Screenplay::setWordCount(int val)
{
    if (m_wordCount == val)
        return;

    m_wordCount = val;
    emit wordCountChanged();
}

void Screenplay::evaluateWordCount()
{
    int wordCount = 0;

    for (const ScreenplayElement *element : qAsConst(m_elements)) {
        if (element->elementType() == ScreenplayElement::SceneElementType) {
            const Scene *scene = element->scene();
            if (scene)
                wordCount += scene->wordCount();
        }
    }

    this->setWordCount(wordCount);
}

void Screenplay::evaluateWordCountLater()
{
    m_wordCountTimer.start(100, this);
}

bool Screenplay::getPasteDataFromClipboard(QJsonObject &clipboardJson) const
{
    clipboardJson = QJsonObject();

    ScriteDocument *sdoc = ScriteDocument::instance();
    if (sdoc->isReadOnly())
        return false;

    QClipboard *clipboard = qApp->clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    if (mimeData == nullptr)
        return false;

    const QString screenplayMimeType = QLatin1String("scrite/screenplay");
    if (!mimeData->hasFormat(screenplayMimeType))
        return false;

    const QByteArray clipboardText = mimeData->data(screenplayMimeType);
    if (clipboardText.isEmpty())
        return false;

    QJsonParseError parseError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(clipboardText, &parseError);
    if (parseError.error != QJsonParseError::NoError)
        return false;

    const QString appString =
            qApp->applicationName() + QLatin1String("-") + qApp->applicationVersion();

    const QJsonObject jsonObj = jsonDoc.object();
    if (jsonObj.value(QLatin1String("app")).toString() != appString)
        return false; // We dont want to support copy/paste between different versions of
                      // Scrite.

    if (jsonObj.value(QLatin1String("source")).toString() != QLatin1String("Screenplay"))
        return false;

    clipboardJson = jsonObj;

    return true;
}

void Screenplay::setHeightHintsAvailable(bool val)
{
    if (m_heightHintsAvailable == val)
        return;

    m_heightHintsAvailable = val;
    emit heightHintsAvailableChanged();
}

void Screenplay::evaluateIfHeightHintsAreAvailable()
{
    bool available = true;

    for (const ScreenplayElement *element : qAsConst(m_elements)) {
        if (element->elementType() != ScreenplayElement::SceneElementType)
            continue;

        available &= !qFuzzyIsNull(element->heightHint());
        if (!available)
            break;
    }

    this->setHeightHintsAvailable(available);
}

void Screenplay::evaluateIfHeightHintsAreAvailableLater()
{
    m_evalHeightHintsAvailableTimer.start(100, this);
}

void Screenplay::setCurrentElementIndex(int val)
{
    val = qBound(-1, val, m_elements.size() - 1);
    if (m_currentElementIndex == val)
        return;

    m_currentElementIndex = val;
    emit currentElementIndexChanged(m_currentElementIndex);

    if (m_currentElementIndex >= 0) {
        ScreenplayElement *element = m_elements.at(m_currentElementIndex);
        this->setActiveScene(element->scene());
        if (!element->isSelected())
            this->clearSelection();
    } else
        this->setActiveScene(nullptr);
}

int Screenplay::nextSceneElementIndex()
{
    int index = m_currentElementIndex + 1;
    while (index < m_elements.size() - 1) {
        ScreenplayElement *element = m_elements.at(index);
        if (element->elementType() == ScreenplayElement::BreakElementType || element->isOmitted()) {
            ++index;
            continue;
        }

        break;
    }

    if (index < m_elements.size())
        return index;

    return m_elements.size() - 1;
}

int Screenplay::previousSceneElementIndex()
{
    int index = m_currentElementIndex - 1;
    while (index >= 0) {
        ScreenplayElement *element = m_elements.at(index);
        if (element->elementType() == ScreenplayElement::BreakElementType || element->isOmitted()) {
            --index;
            continue;
        }

        break;
    }

    if (index >= 0)
        return index;

    return 0;
}

void Screenplay::setActiveScene(Scene *val)
{
    if (m_activeScene == val)
        return;

    // Ensure that the scene belongs to this screenplay.
    if (m_currentElementIndex >= 0) {
        ScreenplayElement *element = this->elementAt(m_currentElementIndex);
        if (element && element->scene() == val) {
            m_activeScene = val;
            emit activeSceneChanged();
            return;
        }
    }

    const int index = this->firstIndexOfScene(val);
    if (index < 0) {
        if (m_activeScene != nullptr) {
            m_activeScene = nullptr;
            emit activeSceneChanged();
        }
    } else {
        m_activeScene = val;
        emit activeSceneChanged();
    }

    this->setCurrentElementIndex(index);
}

QJsonArray Screenplay::search(const QString &text, int flags) const
{
    HourGlass hourGlass;

    QJsonArray ret;

    const int nrScenes = m_elements.size();
    for (int i = 0; i < nrScenes; i++) {
        Scene *scene = m_elements.at(i)->scene();
        if (scene == nullptr)
            continue;

        int sceneResultIndex = 0;

        const int nrElements = scene->elementCount();
        for (int j = 0; j < nrElements; j++) {
            SceneElement *element = scene->elementAt(j);

            const QJsonArray results = element->find(text, flags);
            if (!results.isEmpty()) {
                for (int r = 0; r < results.size(); r++) {
                    const QJsonObject result = results.at(r).toObject();

                    QJsonObject item;
                    item.insert(QStringLiteral("sceneIndex"), i);
                    item.insert(QStringLiteral("elementIndex"), j);
                    item.insert(QStringLiteral("sceneResultIndex"), sceneResultIndex++);

                    const QString _from = QStringLiteral("from");
                    const QString _to = QStringLiteral("to");
                    item.insert(_from, _from);
                    item.insert(_to, _to);
                    ret.append(item);
                }
            }
        }
    }

    return ret;
}

int Screenplay::replace(const QString &text, const QString &replacementText, int flags)
{
    HourGlass hourGlass;

    int counter = 0;

    const int nrScenes = m_elements.size();
    for (int i = 0; i < nrScenes; i++) {
        Scene *scene = m_elements.at(i)->scene();
        if (scene == nullptr)
            continue;

        bool begunUndoCapture = false;

        const int nrElements = scene->elementCount();
        for (int j = 0; j < nrElements; j++) {
            SceneElement *element = scene->elementAt(j);
            const QJsonArray results = element->find(text, flags);
            counter += results.size();

            if (results.isEmpty())
                continue;

            if (!begunUndoCapture) {
                scene->beginUndoCapture();
                begunUndoCapture = true;
            }

            QString elementText = element->text();
            for (int r = results.size() - 1; r >= 0; r--) {
                const QString _from = QStringLiteral("from");
                const QString _to = QStringLiteral("to");
                const QJsonObject result = results.at(r).toObject();
                const int from = result.value(_from).toInt();
                const int to = result.value(_to).toInt();
                elementText = elementText.replace(from, to - from + 1, replacementText);
            }

            element->setText(elementText);
        }

        if (begunUndoCapture)
            scene->endUndoCapture();
    }

    return counter;
}

void Screenplay::resetSceneNumbers()
{
    this->evaluateSceneNumbers(true);
}

bool Screenplay::polishText()
{
    bool ret = false;

    Scene *previousScene = nullptr;

    for (ScreenplayElement *element : qAsConst(m_elements)) {
        if (element->elementType() == ScreenplayElement::SceneElementType) {
            Scene *scene = element->scene();
            if (scene) {
                ret |= scene->polishText(previousScene);
                previousScene = scene;
            }
        } else if (element->elementType() == ScreenplayElement::BreakElementType
                   && element->breakType() == Screenplay::Episode)
            previousScene = nullptr;
    }

    return ret;
}

bool Screenplay::capitalizeSentences()
{
    bool ret = false;

    for (ScreenplayElement *element : qAsConst(m_elements)) {
        if (element->elementType() == ScreenplayElement::SceneElementType) {
            Scene *scene = element->scene();
            if (scene)
                ret |= scene->capitalizeSentences();
        }
    }

    return ret;
}

bool Screenplay::canPaste() const
{
    QJsonObject clipboardJson;
    if (this->getPasteDataFromClipboard(clipboardJson))
        return true;

    const QClipboard *clipboard = qApp->clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    if (mimeData && mimeData->hasText() && Screenplay::fountainPasteOptions() != 0) {
        const QString maybeFountainText = mimeData->text();
        Fountain::Body fBody = Fountain::Parser(maybeFountainText).body();
        return !fBody.isEmpty();
    }

    return false;
}

void Screenplay::copySelection()
{
    QJsonObject clipboardJson;
    clipboardJson.insert(QLatin1String("app"),
                         qApp->applicationName() + QLatin1String("-") + qApp->applicationVersion());
    clipboardJson.insert(QLatin1String("source"), QLatin1String("Screenplay"));

    QJsonArray data;
    QJsonObject scenes;

    Fountain::Body fBody;

    for (const ScreenplayElement *element : qAsConst(m_elements)) {
        if (!element->isSelected())
            continue;

        const QJsonObject elementJson = QObjectSerializer::toJson(element);
        data.append(elementJson);

        if (!scenes.contains(element->sceneID())) {
            const QJsonObject sceneJson = QObjectSerializer::toJson(element->scene());
            scenes.insert(element->sceneID(), sceneJson);
        }

        Fountain::populateBody(element, fBody);
    }
    clipboardJson.insert(QLatin1String("data"), data);
    clipboardJson.insert(QLatin1String("scenes"), scenes);

    const QByteArray clipboardText = QJsonDocument(clipboardJson).toJson();

    QMimeData *mimeData = new QMimeData;
    mimeData->setData(QLatin1String("scrite/screenplay"), clipboardText);
    mimeData->setText(Fountain::Writer(fBody).toString());

    QClipboard *clipboard = qApp->clipboard();
    clipboard->setMimeData(mimeData);
}

class ScreenplayPasteUndoCommand : public QUndoCommand
{
public:
    explicit ScreenplayPasteUndoCommand(Screenplay *screenplay, Structure *structure,
                                        const QJsonArray &elements, const QJsonObject &scenes,
                                        int pasteAfter);
    ~ScreenplayPasteUndoCommand();

    void redo();
    void undo();

private:
    Structure *m_structure = nullptr;
    Screenplay *m_screenplay = nullptr;
    int m_pasteAfter = -1;
    QJsonObject m_scenesData;
    QJsonArray m_screenplayElementsData;
    QList<Scene *> m_scenes;
    QList<ScreenplayElement *> m_screenplayElements;
};

ScreenplayPasteUndoCommand::ScreenplayPasteUndoCommand(Screenplay *screenplay, Structure *structure,
                                                       const QJsonArray &elements,
                                                       const QJsonObject &scenes, int pasteAfter)
    : m_structure(structure),
      m_screenplay(screenplay),
      m_pasteAfter(pasteAfter),
      m_scenesData(scenes),
      m_screenplayElementsData(elements)
{
}

ScreenplayPasteUndoCommand::~ScreenplayPasteUndoCommand() { }

void ScreenplayPasteUndoCommand::redo()
{
    for (int i = 0; i < m_screenplayElementsData.size(); i++) {
        const QJsonObject elementJson = m_screenplayElementsData.at(i).toObject();
        const QString sceneId = elementJson.value(QLatin1String("sceneID")).toString();
        if (!m_structure->findElementBySceneID(sceneId)) {
            const QJsonObject sceneJson = m_scenesData.value(sceneId).toObject();
            if (sceneJson.isEmpty())
                continue;

            QObjectFactory factory;
            factory.addClass<SceneElement>();

            StructureElement *structureElement = new StructureElement(m_structure);
            Scene *scene = new Scene(structureElement);
            if (!QObjectSerializer::fromJson(sceneJson, scene, &factory)) {
                delete scene;
                delete structureElement;
                continue;
            }

            structureElement->setScene(scene);
            m_structure->addElement(structureElement);
        }

        ScreenplayElement *screenplayElement = new ScreenplayElement(m_screenplay);
        if (!QObjectSerializer::fromJson(elementJson, screenplayElement)) {
            delete screenplayElement;
            continue;
        }

        m_screenplayElements.append(screenplayElement);
    }

    if (m_screenplayElements.isEmpty())
        return;

    m_screenplay->insertElementsAt(m_screenplayElements, m_pasteAfter + 1);
    m_screenplay->setSelection(m_screenplayElements);
}

void ScreenplayPasteUndoCommand::undo()
{
    m_screenplay->removeElements(m_screenplayElements);
    m_screenplayElements.clear();

    QList<StructureElement *> structureElements;
    for (Scene *scene : qAsConst(m_scenes))
        structureElements.append(scene->structureElement());
    m_structure->removeElements(structureElements);
    m_scenes.clear();
}

class ScreenplayPasteFromFountainUndoCommand : public QUndoCommand
{
public:
    explicit ScreenplayPasteFromFountainUndoCommand(Screenplay *screenplay, Structure *structure,
                                                    const Fountain::Body &body, int pasteAfter);
    ~ScreenplayPasteFromFountainUndoCommand();

    void redo();
    void undo();

private:
    Structure *m_structure = nullptr;
    Screenplay *m_screenplay = nullptr;
    int m_pasteAfter = -1;
    Fountain::Body m_body;
    QList<Scene *> m_scenes;
    QList<ScreenplayElement *> m_screenplayElements;
};

ScreenplayPasteFromFountainUndoCommand::ScreenplayPasteFromFountainUndoCommand(
        Screenplay *screenplay, Structure *structure, const Fountain::Body &body, int pasteAfter)
    : m_structure(structure), m_screenplay(screenplay), m_pasteAfter(pasteAfter), m_body(body)
{
}

ScreenplayPasteFromFountainUndoCommand::~ScreenplayPasteFromFountainUndoCommand() { }

void ScreenplayPasteFromFountainUndoCommand::redo()
{
    for (const Fountain::Element &fElement : qAsConst(m_body)) {
        if (fElement.type == Fountain::Element::SceneHeading || m_scenes.isEmpty()) {
            StructureElement *newStructureElement = new StructureElement(m_structure);
            Scene *newScene = new Scene(newStructureElement);
            newStructureElement->setScene(newScene);
            m_structure->addElement(newStructureElement);

            ScreenplayElement *newScreenplayElement = new ScreenplayElement(m_screenplay);
            newScreenplayElement->setScene(newScene);
            m_screenplayElements.append(newScreenplayElement);

            m_scenes.append(newScene);
        }

        Fountain::loadIntoScene(fElement, m_scenes.last(), m_screenplayElements.last());
    }

    if (m_screenplayElements.isEmpty())
        return;

    m_screenplay->insertElementsAt(m_screenplayElements, m_pasteAfter + 1);
    m_screenplay->setSelection(m_screenplayElements);
}

void ScreenplayPasteFromFountainUndoCommand::undo()
{
    // Same as ScreenplayPasteUndoCommand::undo()
    m_screenplay->removeElements(m_screenplayElements);
    m_screenplayElements.clear();

    QList<StructureElement *> structureElements;
    for (Scene *scene : qAsConst(m_scenes))
        structureElements.append(scene->structureElement());
    m_structure->removeElements(structureElements);
    m_scenes.clear();
}

void Screenplay::pasteAfter(int index)
{
    ScriteDocument *sdoc = ScriteDocument::instance();
    Structure *structure = sdoc->structure();

    QUndoCommand *cmd = nullptr;

    QJsonObject clipboardJson;
    if (this->getPasteDataFromClipboard(clipboardJson)) {
        // If scenes were copied from screenplay editor, then we will get
        // structured JSON data.
        const bool pasteByLinkingScenesWhenPossible =
                Application::instance()
                        ->settings()
                        ->value("Screenplay Editor/pasteByLinkingScenesWhenPossible", true)
                        .toBool();

        const QString scenesAttr = QLatin1String("scenes");
        const QString dataAttr = QLatin1String("data");
        const QString sceneIdAttr = QLatin1String("sceneID");

        if (!pasteByLinkingScenesWhenPossible) {
            QMap<QString, QString> sceneIdMap;

            // Change sceneID attributes for all elements
            QJsonArray elements = clipboardJson.value(dataAttr).toArray();
            QJsonArray::iterator it = elements.begin();
            QJsonArray::iterator end = elements.end();

            while (it != end) {
                QJsonObject elementObj = (*it).toObject();
                const QString oldId = elementObj.value(sceneIdAttr).toString();
                if (!oldId.isEmpty()) {
                    const QString newId = QUuid::createUuid().toString();

                    sceneIdMap.insert(oldId, newId);
                    elementObj.insert(sceneIdAttr, newId);
                    *it = elementObj;
                }

                ++it;
            }

            clipboardJson.insert(dataAttr, elements);

            // Ensure that changed sceneIDs are reflected in the scenes list as well
            QJsonObject newScenes;
            QJsonObject scenes = clipboardJson.value(scenesAttr).toObject();
            const QStringList sceneIds = scenes.keys();
            for (const QString &oldSceneId : sceneIds) {
                if (sceneIdMap.contains(oldSceneId)) {
                    const QString newSceneId = sceneIdMap.value(oldSceneId);
                    QJsonValue scene = scenes.take(oldSceneId);
                    QJsonObject sceneObj = scene.toObject();
                    sceneObj.insert(QLatin1String("id"), newSceneId);
                    newScenes.insert(newSceneId, sceneObj);
                } else {
                    newScenes.insert(oldSceneId, scenes.take(oldSceneId));
                }
            }

            clipboardJson.insert(scenesAttr, newScenes);
        }

        const QJsonObject scenes = clipboardJson.value(scenesAttr).toObject();
        const QJsonArray elements = clipboardJson.value(dataAttr).toArray();
        cmd = new ScreenplayPasteUndoCommand(this, structure, elements, scenes, index);
    } else {
        const int pasteOptions = Screenplay::fountainPasteOptions();

        if (pasteOptions != 0) {
            // If scenes were copied from fountain file, web-browser or other software,
            // then we will get fountain-text, which we will have to parse and then paste.
            const QClipboard *clipboard = qApp->clipboard();
            const QMimeData *mimeData = clipboard->mimeData();
            if (mimeData && mimeData->hasText()) {
                const QString maybeFountainText = mimeData->text();

                Fountain::Body fBody = Fountain::Parser(maybeFountainText, pasteOptions).body();
                if (!fBody.isEmpty())
                    cmd = new ScreenplayPasteFromFountainUndoCommand(this, structure, fBody, index);
            }
        }
    }

    if (cmd == nullptr)
        return;

    if (UndoStack::active()) {
        UndoStack::active()->push(cmd);
    } else {
        cmd->redo();
        delete cmd;
    }
}

void Screenplay::serializeToJson(QJsonObject &json) const
{
    json.insert("hasCoverPagePhoto", !m_coverPagePhoto.isEmpty());
    json.insert("#currentIndex", m_currentElementIndex);
}

void Screenplay::deserializeFromJson(const QJsonObject &json)
{
    const QString cpPhotoPath =
            m_scriteDocument->fileSystem()->absolutePath(standardCoverPathPhotoPath());
    if (QFile::exists(cpPhotoPath)) {
        m_coverPagePhoto = cpPhotoPath;
        emit coverPagePhotoChanged();
    }

    this->updateBreakTitlesLater();

#if 0
    if (!m_scriteDocument->isCreatedOnThisComputer()) {
        for (ScreenplayElement *element : qAsConst(m_elements))
            element->setHeightHint(0);

        this->setCurrentElementIndex(-1);
    }
#endif

    this->evaluateWordCountLater();
    this->evaluateIfHeightHintsAreAvailableLater();

    const int currentIndex = json.value("#currentIndex").toInt();
    QTimer::singleShot(1000, this, [=]() { this->setCurrentElementIndex(currentIndex); });
}

bool Screenplay::canSetPropertyFromObjectList(const QString &propName) const
{
    if (propName == QStringLiteral("elements"))
        return m_elements.isEmpty();

    return false;
}

void Screenplay::setPropertyFromObjectList(const QString &propName, const QList<QObject *> &objects)
{
    if (propName == QStringLiteral("elements")) {
        const QList<ScreenplayElement *> list = qobject_list_cast<ScreenplayElement *>(objects);
        if (!m_elements.isEmpty() || list.isEmpty())
            return;

        this->beginResetModel();

        for (ScreenplayElement *ptr : list) {
            // Keep the following connections in sync with the ones we make in
            // Screenplay::insertElementAt()
            ptr->setParent(this);
            this->connectToScreenplayElementSignals(ptr);
            m_elements.append(ptr);
            emit elementInserted(ptr, m_elements.size() - 1);
        }

        this->endResetModel();

        emit elementCountChanged();
        emit elementsChanged();

        this->setCurrentElementIndex(0);

        return;
    }
}

int Screenplay::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_elements.size();
}

QVariant Screenplay::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    ScreenplayElement *element = this->elementAt(index.row());
    switch (role) {
    case IdRole:
        return element->sceneID();
    case ScreenplayElementRole:
        return QVariant::fromValue<ScreenplayElement *>(element);
    case ScreenplayElementTypeRole:
        return element->elementType();
    case BreakTypeRole:
        return element->breakType();
    case SceneRole:
        return QVariant::fromValue<Scene *>(element->scene());
    default:
        break;
    }

    return QVariant();
}

QHash<int, QByteArray> Screenplay::roleNames() const
{
    static QHash<int, QByteArray> roles;
    if (roles.isEmpty()) {
        roles[IdRole] = "id";
        roles[ScreenplayElementRole] = "screenplayElement";
        roles[ScreenplayElementTypeRole] = "screenplayElementType";
        roles[BreakTypeRole] = "breakType";
        roles[SceneRole] = "scene";
    }

    return roles;
}

void Screenplay::write(QTextCursor &cursor, const WriteOptions &options) const
{
    if (options.includeTextNotes || options.includeFormNotes) {
        auto addSection = [&cursor, options](const QString &sectionName) {
            QTextBlockFormat sectionBlockFormat;
            sectionBlockFormat.setHeadingLevel(3);

            QTextCharFormat sectionCharFormat;
            sectionCharFormat.setFontWeight(QFont::Bold);
            sectionCharFormat.setFontPointSize(16);
            sectionBlockFormat.setTopMargin(10);

            cursor.insertBlock(sectionBlockFormat, sectionCharFormat);
            cursor.insertText(sectionName);
        };

        for (ScreenplayElement *element : m_elements) {
            switch (element->elementType()) {
            case ScreenplayElement::SceneElementType: {
                if (element->isOmitted())
                    continue;
                const Scene *scene = element->scene();
                QString heading = scene->structureElement()->title();
                if (scene->heading() && scene->heading()->isEnabled())
                    heading = element->resolvedSceneNumber() + QLatin1String(". ") + heading;
                addSection(heading);

                Scene::WriteOptions options;
                options.headingLevel = 4;
                options.includeHeading = false;
                scene->write(cursor, options);
            } break;
            case ScreenplayElement::BreakElementType: {
                addSection(element->breakTitle() + QLatin1String(" - ") + element->breakSubtitle());

                if ((options.actsOnNewPage && element->breakType() == Screenplay::Act)
                    || (options.episodesOnNewPage && element->breakType() == Screenplay::Episode)) {
                    QTextBlockFormat pageBreakFormat;
                    pageBreakFormat.setPageBreakPolicy(QTextBlockFormat::PageBreak_AlwaysBefore);
                    cursor.mergeBlockFormat(pageBreakFormat);
                }

                const QString breakSummary = element->breakSummary();
                if (!breakSummary.isEmpty()) {
                    QTextBlockFormat blockFormat;
                    QTextCharFormat charFormat;
                    cursor.insertBlock(blockFormat, charFormat);
                    cursor.insertText(QLatin1String("Summary: ") + breakSummary);
                }

                Notes *notes = element->notes();
                if (notes) {
                    const int cp1 = cursor.position();
                    Notes::WriteOptions options;
                    notes->write(cursor, options);
                    const int cp2 = cursor.position();

                    cursor.setPosition(cp1);
                    cursor.movePosition(QTextCursor::NextBlock);

                    while (cursor.position() < cp2) {
                        QTextBlockFormat blockFormat = cursor.blockFormat();
                        blockFormat.setIndent(blockFormat.indent() + 1);
                        cursor.setBlockFormat(blockFormat);
                        if (!cursor.movePosition(QTextCursor::NextBlock))
                            break;
                    }

                    cursor.setPosition(cp2);
                }
            } break;
            }
        }
    }
}

int Screenplay::fountainCopyOptions()
{
    int options = 0;

    const QString group = QStringLiteral("Screenplay Editor/");

    const QSettings *settings = Application::instance()->settings();
    if (settings->value(group + QStringLiteral("copyAsFountain")).toBool()) {
        if (settings->value(group + QStringLiteral("copyFountainUsingStrictSyntax")).toBool())
            options += Fountain::Writer::StrictSyntaxOption;
        if (settings->value(group + QStringLiteral("copyFountainWithEmphasis")).toBool())
            options += Fountain::Writer::EmphasisOption;
    }

    return options;
}

int Screenplay::fountainPasteOptions()
{
    int options = 0;

    const QString group = QStringLiteral("Screenplay Editor/");

    const QSettings *settings = Application::instance()->settings();
    if (settings->value(group + QStringLiteral("pasteAsFountain")).toBool()) {
        options += Fountain::Parser::IgnoreLeadingWhitespaceOption;
        options += Fountain::Parser::IgnoreTrailingWhiteSpaceOption;
        if (settings->value(group + QStringLiteral("pasteByMergingAdjacentElements")).toBool())
            options += Fountain::Parser::JoinAdjacentElementOption;
        if (settings->value(group + QStringLiteral("pasteAfterResolvingEmphasis")).toBool())
            options += Fountain::Parser::ResolveEmphasisOption;
    }

    return options;
}

bool Screenplay::event(QEvent *event)
{
    if (event->type() == QEvent::ParentChange)
        m_scriteDocument = qobject_cast<ScriteDocument *>(this->parent());

    return QObject::event(event);
}

void Screenplay::timerEvent(QTimerEvent *te)
{
    if (te->timerId() == m_sceneNumberEvaluationTimer.timerId()) {
        m_sceneNumberEvaluationTimer.stop();
        this->evaluateSceneNumbers();
    } else if (te->timerId() == m_updateBreakTitlesTimer.timerId()) {
        m_updateBreakTitlesTimer.stop();
        this->updateBreakTitles();
    } else if (te->timerId() == m_paragraphCountEvaluationTimer.timerId()) {
        m_paragraphCountEvaluationTimer.stop();
        this->evaluateParagraphCounts();
    } else if (te->timerId() == m_wordCountTimer.timerId()) {
        m_wordCountTimer.stop();
        this->evaluateWordCount();
    } else if (te->timerId() == m_evalHeightHintsAvailableTimer.timerId()) {
        m_evalHeightHintsAvailableTimer.stop();
        this->evaluateIfHeightHintsAreAvailable();
    } else if (te->timerId() == m_selectedElementsOmitStatusChangedTimer.timerId()) {
        m_selectedElementsOmitStatusChangedTimer.stop();
        emit selectedElementsOmitStatusChanged();
    }
}

void Screenplay::resetActiveScene()
{
    m_activeScene = nullptr;
    emit activeSceneChanged();
    this->setCurrentElementIndex(-1);
}

void Screenplay::onSceneReset(int elementIndex)
{
    ScreenplayElement *element = qobject_cast<ScreenplayElement *>(this->sender());
    if (element == nullptr)
        return;

    int sceneIndex = this->indexOfElement(element);
    if (sceneIndex < 0)
        return;

    emit sceneReset(sceneIndex, elementIndex);
}

void Screenplay::onScreenplayElementOmittedChanged()
{
    ScreenplayElement *element = qobject_cast<ScreenplayElement *>(this->sender());
    if (element == nullptr)
        return;

    int sceneIndex = this->indexOfElement(element);
    if (sceneIndex < 0)
        return;

    if (element->isOmitted())
        emit elementOmitted(element, sceneIndex);
    else
        emit elementIncluded(element, sceneIndex);

    m_selectedElementsOmitStatusChangedTimer.start(10, this);
}

void Screenplay::updateBreakTitlesLater()
{
    m_updateBreakTitlesTimer.start(0, this);
}

QList<ScreenplayBreakInfo> Screenplay::episodeInfoList() const
{
    QList<ScreenplayBreakInfo> ret;
    if (this->episodeCount() <= 0)
        return ret;

    int epIndex = 0;
    for (int i = 0; i < m_elements.size(); i++) {
        const ScreenplayElement *element = m_elements.at(i);
        if (element->elementType() != ScreenplayElement::BreakElementType)
            continue;

        if (element->breakType() != Screenplay::Episode)
            continue;

        ScreenplayBreakInfo info({ epIndex, epIndex + 1, element->breakTitle(),
                                   element->breakSubtitle(), QString() });
        ret.append(info);

        ++epIndex;
    }

    return ret;
}

void Screenplay::evaluateSceneNumbers(bool minorAlso)
{
    // Sometimes Screenplay is used by ScreenplayAdapter to house a single
    // scene. In such cases, we must not evaluate numbers.
    if (m_scriteDocument == nullptr)
        return;

    int actIndex = -1, totalActIndex = -1;
    int episodeIndex = -1;
    int elementIndex = -1;
    int nrScenes = 0;
    bool containsNonStandardScenes = false;

    ScreenplayElement *lastEpisodeElement = nullptr;
    ScreenplayElement *lastActElement = nullptr;
    QString lastEpisodeName;
    QString lastActName;

    QHash<Scene *, QList<int>> indexListMap;

    for (ScreenplayElement *element : qAsConst(m_elements))
        if (element->scene())
            indexListMap[element->scene()] = QList<int>();

    int index = -1;
    ScreenplayElement::SceneNumber sceneNumber;
    for (ScreenplayElement *element : qAsConst(m_elements)) {
        ++index;

        if (element->elementType() == ScreenplayElement::SceneElementType) {
            if (actIndex < 0 && element->scene()->heading()->isEnabled())
                ++actIndex;
            if (episodeIndex < 0 && element->scene()->heading()->isEnabled())
                ++episodeIndex;

            element->setElementIndex(++elementIndex);
            element->setActIndex(actIndex);
            element->setEpisodeIndex(episodeIndex);

            // This should never happen!
            if (element->scene() == nullptr)
                element->setScene(new Scene(element));

            Scene *scene = element->scene();
            scene->setAct(lastActElement         ? lastActName
                                  : actIndex < 0 ? QStringLiteral("No Act")
                                                 : QStringLiteral("ACT 1"));
            scene->setActIndex(actIndex);
            scene->setEpisode(lastEpisodeElement         ? lastEpisodeName
                                      : episodeIndex < 0 ? QStringLiteral("No Episode")
                                                         : QStringLiteral("EPISODE 1"));
            scene->setEpisodeIndex(episodeIndex);
            indexListMap[scene].append(index);

            if (scene->heading()->isEnabled()) {
                ++nrScenes;
                element->evaluateSceneNumber(sceneNumber, minorAlso);
            }
        } else {
            element->setElementIndex(-1);
            if (element->breakType() == Screenplay::Act) {
                ++actIndex;
                if (totalActIndex < 0)
                    ++totalActIndex;
                ++totalActIndex;

                lastActElement = element;
                lastActName = element->breakTitle();
                if (!element->breakSubtitle().isEmpty())
                    lastActName += ": " + element->breakSubtitle();
            } else if (element->breakType() == Screenplay::Episode) {
                ++episodeIndex;

                actIndex = 0;

                lastActElement = nullptr;
                lastEpisodeElement = element;
                lastEpisodeName = element->breakTitle();
                if (!element->breakSubtitle().isEmpty())
                    lastEpisodeName += ": " + element->breakSubtitle();
            }

            element->setActIndex(actIndex);
            element->setEpisodeIndex(episodeIndex);
        }

        if (!containsNonStandardScenes && element->scene()
            && element->scene()->type() != Scene::Standard)
            containsNonStandardScenes = true;
    }

    QHash<Scene *, QList<int>>::const_iterator it = indexListMap.constBegin();
    QHash<Scene *, QList<int>>::const_iterator end = indexListMap.constEnd();
    while (it != end) {
        it.key()->setScreenplayElementIndexList(it.value());
        ++it;
    }

    this->setSceneCount(nrScenes);
    this->setEpisodeCount(lastEpisodeElement ? episodeIndex + 1 : 0);
    this->setActCount(lastEpisodeElement ? totalActIndex + 1 : (lastActElement ? actIndex + 1 : 0));

    this->setHasNonStandardScenes(containsNonStandardScenes);
}

void Screenplay::evaluateSceneNumbersLater()
{
    m_sceneNumberEvaluationTimer.start(0, this);
}

void Screenplay::validateCurrentElementIndex()
{
    int val = m_currentElementIndex;
    if (m_elements.isEmpty())
        val = -1;
    else
        val = qBound(0, val, m_elements.size() - 1);

    if (val >= 0) {
        Scene *currentScene = m_elements.at(val)->scene();
        if (m_activeScene != currentScene)
            m_currentElementIndex = -2;
    }

    this->setCurrentElementIndex(val);
}

void Screenplay::evaluateParagraphCounts()
{
    int min = -1, max = -1, avg = 0, total = 0, count = 0;
    for (ScreenplayElement *element : qAsConst(m_elements)) {
        Scene *scene = element->scene();
        if (scene == nullptr)
            continue;

        if (min < 0)
            min = scene->elementCount();
        else
            min = qMin(min, scene->elementCount());

        if (max < 0)
            max = scene->elementCount();
        else
            max = qMax(max, scene->elementCount());

        total += scene->elementCount();
        ++count;
    }

    if (count > 0)
        avg = qRound(qreal(total) / qreal(count));
    else
        avg = 0;

    m_minimumParagraphCount = min;
    m_maximumParagraphCount = max;
    m_averageParagraphCount = avg;
    emit paragraphCountChanged();
}

void Screenplay::evaluateParagraphCountsLater()
{
    m_paragraphCountEvaluationTimer.start(0, this);
}

void Screenplay::setHasNonStandardScenes(bool val)
{
    if (m_hasNonStandardScenes == val)
        return;

    m_hasNonStandardScenes = val;
    emit hasNonStandardScenesChanged();
}

void Screenplay::setHasTitlePageAttributes(bool val)
{
    if (m_hasTitlePageAttributes == val)
        return;

    m_hasTitlePageAttributes = val;
    emit hasTitlePageAttributesChanged();
}

void Screenplay::evaluateHasTitlePageAttributes()
{
    this->setHasTitlePageAttributes(!m_title.isEmpty() || !m_author.isEmpty()
                                    || !m_version.isEmpty());
}

void Screenplay::staticAppendElement(QQmlListProperty<ScreenplayElement> *list,
                                     ScreenplayElement *ptr)
{
    reinterpret_cast<Screenplay *>(list->data)->addElement(ptr);
}

void Screenplay::staticClearElements(QQmlListProperty<ScreenplayElement> *list)
{
    reinterpret_cast<Screenplay *>(list->data)->clearElements();
}

ScreenplayElement *Screenplay::staticElementAt(QQmlListProperty<ScreenplayElement> *list, int index)
{
    return reinterpret_cast<Screenplay *>(list->data)->elementAt(index);
}

int Screenplay::staticElementCount(QQmlListProperty<ScreenplayElement> *list)
{
    return reinterpret_cast<Screenplay *>(list->data)->elementCount();
}

///////////////////////////////////////////////////////////////////////////////

ScreenplayTracks::ScreenplayTracks(QObject *parent)
    : QAbstractListModel(parent), m_screenplay(this, "screenplay")
{
    connect(this, &ScreenplayTracks::modelReset, this, &ScreenplayTracks::trackCountChanged);
    connect(this, &ScreenplayTracks::rowsInserted, this, &ScreenplayTracks::trackCountChanged);
    connect(this, &ScreenplayTracks::rowsRemoved, this, &ScreenplayTracks::trackCountChanged);
}

ScreenplayTracks::~ScreenplayTracks() { }

void ScreenplayTracks::setScreenplay(Screenplay *val)
{
    if (m_screenplay == val)
        return;

    if (!m_screenplay.isNull())
        m_screenplay->disconnect(this);

    m_screenplay = val;

    if (!m_screenplay.isNull()) {
        connect(m_screenplay, &Screenplay::elementsChanged, this, &ScreenplayTracks::refreshLater);
        connect(m_screenplay, &Screenplay::elementSceneGroupsChanged, this,
                &ScreenplayTracks::onElementSceneGroupsChanged);
    }

    this->refreshLater();

    emit screenplayChanged();
}

int ScreenplayTracks::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_data.size();
}

QVariant ScreenplayTracks::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_data.size() || role != ModelDataRole)
        return QVariant();

    return m_data.at(index.row());
}

QHash<int, QByteArray> ScreenplayTracks::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ModelDataRole] = "modelData";
    return roles;
}

void ScreenplayTracks::timerEvent(QTimerEvent *te)
{
    if (te->timerId() == m_refreshTimer.timerId()) {
        m_refreshTimer.stop();
        this->refresh();
    } else
        QAbstractListModel::timerEvent(te);
}

void ScreenplayTracks::refresh()
{
    if (m_screenplay.isNull()) {
        if (m_data.isEmpty())
            return;

        this->beginResetModel();
        m_data.clear();
        this->endResetModel();

        return;
    }

    const QString slash = QStringLiteral("/");

    QMap<QString, QMap<QString, QList<ScreenplayElement *>>> map;
    for (int i = 0; i < m_screenplay->elementCount(); i++) {
        ScreenplayElement *element = m_screenplay->elementAt(i);
        if (element->elementType() != ScreenplayElement::SceneElementType)
            continue;

        const QStringList sceneGroups = element->scene()->groups();
        if (sceneGroups.isEmpty())
            continue;

        for (const QString &sceneGroup : sceneGroups) {
            const QString categoryName = sceneGroup.section(slash, 0, 0);
            const QString groupName = sceneGroup.section(slash, 1);
            map[categoryName][groupName].append(element);
        }
    }

    const QString startIndexKey = QStringLiteral("startIndex");
    const QString endIndexKey = QStringLiteral("endIndex");
    const QString groupKey = QStringLiteral("group");

    this->beginResetModel();

    m_data.clear();

    QMap<QString, QMap<QString, QList<ScreenplayElement *>>>::iterator it = map.begin();
    QMap<QString, QMap<QString, QList<ScreenplayElement *>>>::iterator end = map.end();
    while (it != end) {
        const QString category = Application::instance()->camelCased(it.key());
        const QMap<QString, QList<ScreenplayElement *>> groupElementsMap = it.value();

        QVariantList categoryTrackItems;

        QMap<QString, QList<ScreenplayElement *>>::const_iterator it2 = groupElementsMap.begin();
        QMap<QString, QList<ScreenplayElement *>>::const_iterator end2 = groupElementsMap.end();
        while (it2 != end2) {
            const QString group = Application::instance()->camelCased(it2.key());
            QList<ScreenplayElement *> elements = it2.value();
            std::sort(elements.begin(), elements.end(),
                      [](ScreenplayElement *a, ScreenplayElement *b) {
                          return a->elementIndex() < b->elementIndex();
                      });

            QVariantMap groupTrackItem;

            for (ScreenplayElement *element : qAsConst(elements)) {
                const QVariantMap elementItem = { { startIndexKey, element->elementIndex() },
                                                  { endIndexKey, element->elementIndex() },
                                                  { groupKey, group } };

                if (groupTrackItem.isEmpty())
                    groupTrackItem = elementItem;
                else {
                    const int diff = element->elementIndex()
                            - groupTrackItem.value(endIndexKey, -10).toInt();
                    if (diff == 1)
                        groupTrackItem.insert(endIndexKey, element->elementIndex());
                    else {
                        categoryTrackItems.append(groupTrackItem);
                        groupTrackItem = elementItem;
                    }
                }
            }

            if (!groupTrackItem.isEmpty())
                categoryTrackItems.append(groupTrackItem);

            ++it2;
        }

        std::sort(categoryTrackItems.begin(), categoryTrackItems.end(),
                  [startIndexKey, endIndexKey](const QVariant &a, const QVariant &b) {
                      const QVariantMap trackA = a.toMap();
                      const QVariantMap trackB = b.toMap();
                      const int trackASize = trackA.value(endIndexKey).toInt()
                              - trackA.value(startIndexKey).toInt();
                      const int trackBSize = trackB.value(endIndexKey).toInt()
                              - trackB.value(startIndexKey).toInt();
                      return trackASize > trackBSize;
                  });

        QList<QVariantList> nonIntersectionTracks;
        nonIntersectionTracks << QVariantList();

        auto includeTrack = [&nonIntersectionTracks, startIndexKey, endIndexKey,
                             groupKey](const QVariantMap &trackB) {
            for (int i = 0; i < nonIntersectionTracks.size(); i++) {
                QVariantList &list = nonIntersectionTracks[i];
                bool intersectionFound = false;
                for (const QVariant &listItem : list) {
                    const QVariantMap &trackA = listItem.toMap();
                    const int startA = trackA.value(startIndexKey).toInt();
                    const int endA = trackA.value(endIndexKey).toInt();
                    const int startB = trackB.value(startIndexKey).toInt();
                    const int endB = trackB.value(endIndexKey).toInt();
                    if ((startA <= startB && startB <= endA) || (startA <= endB && endB <= endA)) {
                        intersectionFound = true;
                        break;
                    }
                }
                if (!intersectionFound) {
                    list << trackB;
                    return;
                }
            }

            QVariantList newTrack;
            newTrack << trackB;
            nonIntersectionTracks << newTrack;
        };

        for (const QVariant &item : qAsConst(categoryTrackItems))
            includeTrack(item.toMap());

        for (const QVariantList &tracks : qAsConst(nonIntersectionTracks)) {
            QVariantMap row;
            row.insert(QStringLiteral("category"), category);
            row.insert(QStringLiteral("tracks"), tracks);
            m_data.append(row);
        }

        ++it;
    }

    this->endResetModel();
}

void ScreenplayTracks::refreshLater()
{
    m_refreshTimer.start(0, this);
}
