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

#ifndef SCREENPLAY_H
#define SCREENPLAY_H

#include "scene.h"
#include "modifiable.h"
#include "execlatertimer.h"
#include "qobjectproperty.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QQmlListProperty>

class Screenplay;
class ScriteDocument;
class AbstractImporter;
class ScreenplayTextDocument;
class AbstractScreenplaySubsetReport;

class ScreenplayElement : public QObject, public Modifiable, public QObjectSerializer::Interface
{
    Q_OBJECT
    Q_INTERFACES(QObjectSerializer::Interface)
    QML_ELEMENT

public:
    Q_INVOKABLE explicit ScreenplayElement(QObject *parent = nullptr);
    ~ScreenplayElement();
    Q_SIGNAL void aboutToDelete(ScreenplayElement *element);

    // clang-format off
    Q_PROPERTY(int serialNumber
               READ serialNumber
               CONSTANT )
    // clang-format on
    int serialNumber() const { return m_serialNumber; }

    // clang-format off
    Q_PROPERTY(int elementIndex
               READ elementIndex
               NOTIFY elementIndexChanged)
    // clang-format on
    int elementIndex() const { return m_elementIndex; }
    Q_SIGNAL void elementIndexChanged();

    // clang-format off
    Q_PROPERTY(int actIndex
               READ actIndex
               NOTIFY actIndexChanged)
    // clang-format on
    int actIndex() const { return m_actIndex; }
    Q_SIGNAL void actIndexChanged();

    // clang-format off
    Q_PROPERTY(int episodeIndex
               READ episodeIndex
               NOTIFY episodeIndexChanged)
    // clang-format on
    int episodeIndex() const { return m_episodeIndex; }
    Q_SIGNAL void episodeIndexChanged();

    enum ElementType { SceneElementType, BreakElementType };
    Q_ENUM(ElementType)
    // clang-format off
    Q_PROPERTY(ElementType elementType
               READ elementType
               WRITE setElementType
               NOTIFY elementTypeChanged)
    // clang-format on
    void setElementType(ElementType val);
    ElementType elementType() const { return m_elementType; }
    Q_SIGNAL void elementTypeChanged();

    // clang-format off
    Q_PROPERTY(int breakType
               READ breakType
               WRITE setBreakType
               NOTIFY breakTypeChanged)
    // clang-format on
    void setBreakType(int val);
    int breakType() const { return m_breakType; }
    Q_SIGNAL void breakTypeChanged();

    // clang-format off
    Q_PROPERTY(QString breakTitle
               READ breakTitle
               NOTIFY breakTitleChanged)
    // clang-format on
    QString breakTitle() const { return m_breakTitle.isEmpty() ? this->sceneID() : m_breakTitle; }
    Q_SIGNAL void breakTitleChanged();

    // clang-format off
    Q_PROPERTY(QString breakSubtitle
               READ breakSubtitle
               WRITE setBreakSubtitle
               NOTIFY breakSubtitleChanged)
    // clang-format on
    void setBreakSubtitle(const QString &val);
    QString breakSubtitle() const { return m_breakSubtitle; }
    Q_SIGNAL void breakSubtitleChanged();

    // clang-format off
    Q_PROPERTY(Screenplay *screenplay
               READ screenplay
               WRITE setScreenplay
               NOTIFY screenplayChanged
               STORED false
               RESET resetScreenplay)
    // clang-format on
    void setScreenplay(Screenplay *val);
    Screenplay *screenplay() const { return m_screenplay; }
    Q_SIGNAL void screenplayChanged();

    // clang-format off
    Q_PROPERTY(QString sceneID
               READ sceneID
               WRITE setSceneFromID
               NOTIFY sceneChanged)
    // clang-format on
    void setSceneFromID(const QString &val);
    QString sceneID() const;

    // clang-format off
    Q_PROPERTY(int sceneNumber
               READ sceneNumber
               NOTIFY sceneNumberChanged)
    // clang-format on
    int sceneNumber() const
    {
        return m_customSceneNumber < 0 ? m_sceneNumber : m_customSceneNumber;
    }
    Q_SIGNAL void sceneNumberChanged();

    // clang-format off
    Q_PROPERTY(QString userSceneNumber
               READ userSceneNumber
               WRITE setUserSceneNumber
               NOTIFY userSceneNumberChanged)
    // clang-format on
    void setUserSceneNumber(const QString &val);
    QString userSceneNumber() const { return m_userSceneNumber; }
    Q_SIGNAL void userSceneNumberChanged();

    // clang-format off
    Q_PROPERTY(bool hasUserSceneNumber
               READ hasUserSceneNumber
               NOTIFY userSceneNumberChanged)
    // clang-format on
    bool hasUserSceneNumber() const { return !m_userSceneNumber.isEmpty(); }

    // clang-format off
    Q_PROPERTY(QString resolvedSceneNumber
               READ resolvedSceneNumber
               NOTIFY resolvedSceneNumberChanged)
    // clang-format on
    QString resolvedSceneNumber() const;
    Q_SIGNAL void resolvedSceneNumberChanged();

    // clang-format off
    Q_PROPERTY(Scene *scene
               READ scene
               NOTIFY sceneChanged
               STORED false
               RESET resetScene)
    // clang-format on
    void setScene(Scene *val);
    Scene *scene() const { return m_scene; }
    Q_SIGNAL void sceneChanged();

    // clang-format off
    Q_PROPERTY(bool expanded
               READ isExpanded
               WRITE setExpanded
               NOTIFY expandedChanged)
    // clang-format on
    void setExpanded(bool val);
    bool isExpanded() const { return m_expanded; }
    Q_SIGNAL void expandedChanged();

    // clang-format off
    Q_PROPERTY(bool omitted
               READ isOmitted
               WRITE setOmitted
               NOTIFY omittedChanged)
    // clang-format on
    void setOmitted(bool val);
    bool isOmitted() const { return m_omitted; }
    Q_SIGNAL void omittedChanged();

    // clang-format off
    Q_PROPERTY(QJsonValue userData
               READ userData
               WRITE setUserData
               NOTIFY userDataChanged
               STORED false)
    // clang-format on
    void setUserData(const QJsonValue &val);
    QJsonValue userData() const { return m_userData; }
    Q_SIGNAL void userDataChanged();

    // clang-format off
    Q_PROPERTY(qreal heightHint
               READ heightHint
               WRITE setHeightHint
               NOTIFY heightHintChanged)
    // clang-format on
    void setHeightHint(qreal val);
    qreal heightHint() const { return m_heightHint; }
    Q_SIGNAL void heightHintChanged();

    // clang-format off
    Q_PROPERTY(bool selected
               READ isSelected
               WRITE setSelected
               NOTIFY selectedChanged
               STORED false)
    // clang-format on
    void setSelected(bool val);
    bool isSelected() const { return m_selected; }
    Q_SIGNAL void selectedChanged();

    // clang-format off
    Q_PROPERTY(QString breakSummary
               READ breakSummary
               WRITE setBreakSummary
               NOTIFY breakSummaryChanged)
    // clang-format on
    void setBreakSummary(const QString &val);
    QString breakSummary() const { return m_breakSummary; }
    Q_SIGNAL void breakSummaryChanged();

    // clang-format off
    Q_PROPERTY(bool pageBreakAfter
               READ isPageBreakAfter
               WRITE setPageBreakAfter
               NOTIFY pageBreakAfterChanged)
    // clang-format on
    void setPageBreakAfter(bool val);
    bool isPageBreakAfter() const { return m_pageBreakAfter; }
    Q_SIGNAL void pageBreakAfterChanged();

    // clang-format off
    Q_PROPERTY(bool pageBreakBefore
               READ isPageBreakBefore
               WRITE setPageBreakBefore
               NOTIFY pageBreakBeforeChanged)
    // clang-format on
    void setPageBreakBefore(bool val);
    bool isPageBreakBefore() const { return m_pageBreakBefore; }
    Q_SIGNAL void pageBreakBeforeChanged();

    // clang-format off
    Q_PROPERTY(Notes *notes
               READ notes
               NOTIFY notesChanged)
    // clang-format on
    Notes *notes() const { return m_notes ? m_notes : (m_scene ? m_scene->notes() : nullptr); }
    Q_SIGNAL void notesChanged();

    // clang-format off
    Q_PROPERTY(Attachments *attachments
               READ attachments
               NOTIFY attachmentsChanged)
    // clang-format on
    Attachments *attachments() const
    {
        return m_attachments ? m_attachments : (m_scene ? m_scene->attachments() : nullptr);
    }
    Q_SIGNAL void attachmentsChanged();

    Q_INVOKABLE void toggleSelection() { this->setSelected(!m_selected); }

    // clang-format off
    Q_PROPERTY(int wordCount
               READ wordCount
               NOTIFY wordCountChanged)
    // clang-format on
    int wordCount() const;
    Q_SIGNAL void wordCountChanged();

    Q_SIGNAL void elementChanged();

    Q_SIGNAL void sceneAboutToReset();
    Q_SIGNAL void sceneReset(int elementIndex);
    Q_SIGNAL void sceneContentChanged();
    Q_SIGNAL void sceneHeadingChanged();
    Q_SIGNAL void sceneElementChanged(SceneElement *sceneElement);
    Q_SIGNAL void evaluateSceneNumberRequest();
    Q_SIGNAL void sceneTypeChanged();
    Q_SIGNAL void sceneTagsChanged(ScreenplayElement *ptr);
    Q_SIGNAL void sceneGroupsChanged(ScreenplayElement *ptr);

    QString delegateKind() const;

    // QObjectSerializer::Interface interface
    bool canSerialize(const QMetaObject *, const QMetaProperty &) const;

protected:
    struct SceneNumber
    {
        int major = 0; // 0 means no major, 1 means 1
        int minor = -1; // -1 means no minor, 0 means A

        int nextMajor()
        {
            minor = -1;
            return ++major;
        }

        int nextMinor()
        {
            major = qMax(major, 1);
            minor = qMax(minor, 0) + 1;
            return minor;
        }

        QString toString() const
        {
            QString ret;
            int _minor = minor;
            while (--_minor >= 0) {
                ret = QChar('A' + _minor % 26) + ret;
                _minor /= 26;
            }
            ret = QString::number(qMax(major, 1)) + ret;
            return ret;
        }
    };
    bool event(QEvent *event);
    void evaluateSceneNumber(SceneNumber &number, bool minorAlso = false);
    void resetScene();
    void resetScreenplay();
    void setActIndex(int val);
    void setEpisodeIndex(int val);
    void setElementIndex(int val);
    void setBreakTitle(const QString &val);

private:
    void onSceneTagsChanged() { emit sceneTagsChanged(this); }
    void onSceneGroupsChanged() { emit sceneGroupsChanged(this); }
    void setNotes(Notes *val);
    void setAttachments(Attachments *val);

private:
    friend class Screenplay;
    friend class AbstractImporter;
    friend class AbstractScreenplaySubsetReport;

    bool m_omitted = false;
    bool m_expanded = true;
    int m_breakType = -1;
    bool m_selected = false;
    int m_actIndex = -1;
    int m_serialNumber = -1;
    int m_elementIndex = -1;
    int m_episodeIndex = -1;
    int m_sceneNumber = -1;
    QString m_sceneID;
    Notes *m_notes = nullptr;
    Attachments *m_attachments = nullptr;
    QString m_breakTitle;
    QJsonValue m_userData;
    qreal m_heightHint = 0;
    bool m_pageBreakAfter = false;
    bool m_pageBreakBefore = false;
    QString m_breakSummary;
    QString m_breakSubtitle;
    int m_customSceneNumber = -1;
    bool m_elementTypeIsSet = false;
    QString m_userSceneNumber;
    ElementType m_elementType = SceneElementType;
    QObjectProperty<Scene> m_scene;
    QObjectProperty<Screenplay> m_screenplay;
};

struct ScreenplayBreakInfo
{
    Q_GADGET
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    // clang-format off
    Q_PROPERTY(int index
               MEMBER index)
    // clang-format on
    int index = -1;

    // clang-format off
    Q_PROPERTY(int number
               MEMBER number)
    // clang-format on
    int number = -1;

    // clang-format off
    Q_PROPERTY(QString title
               MEMBER title)
    // clang-format on
    QString title;

    // clang-format off
    Q_PROPERTY(QString subtitle
               MEMBER subtitle)
    // clang-format on
    QString subtitle;

    // clang-format off
    Q_PROPERTY(QString path
               MEMBER path)
    // clang-format on
    QString path;
};
Q_DECLARE_METATYPE(ScreenplayBreakInfo)
Q_DECLARE_METATYPE(QList<ScreenplayBreakInfo>)

class Screenplay : public QAbstractListModel, public Modifiable, public QObjectSerializer::Interface
{
    Q_OBJECT
    Q_INTERFACES(QObjectSerializer::Interface)
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    explicit Screenplay(QObject *parent = nullptr);
    ~Screenplay();
    Q_SIGNAL void aboutToDelete(Screenplay *ptr);

    static QString standardCoverPathPhotoPath();

    // clang-format off
    Q_PROPERTY(ScriteDocument *scriteDocument
               READ scriteDocument
               CONSTANT STORED
               false )
    // clang-format on
    ScriteDocument *scriteDocument() const { return m_scriteDocument; }

    // clang-format off
    Q_PROPERTY(QString title
               READ title
               WRITE setTitle
               NOTIFY titleChanged)
    // clang-format on
    void setTitle(const QString &val);
    QString title() const { return m_title; }
    Q_SIGNAL void titleChanged();

    // clang-format off
    Q_PROPERTY(QString subtitle
               READ subtitle
               WRITE setSubtitle
               NOTIFY subtitleChanged)
    // clang-format on
    void setSubtitle(const QString &val);
    QString subtitle() const { return m_subtitle; }
    Q_SIGNAL void subtitleChanged();

    // clang-format off
    Q_PROPERTY(QString logline
               READ logline
               WRITE setLogline
               NOTIFY loglineChanged)
    // clang-format on
    void setLogline(const QString &val);
    QString logline() const { return m_logline; }
    Q_SIGNAL void loglineChanged();

    // clang-format off
    Q_PROPERTY(QString loglineComments
               READ loglineComments
               WRITE setLoglineComments
               NOTIFY loglineCommentsChanged)
    // clang-format on
    void setLoglineComments(const QString &val);
    QString loglineComments() const { return m_loglineComments; }
    Q_SIGNAL void loglineCommentsChanged();

    // clang-format off
    Q_PROPERTY(QString basedOn
               READ basedOn
               WRITE setBasedOn
               NOTIFY basedOnChanged)
    // clang-format on
    void setBasedOn(const QString &val);
    QString basedOn() const { return m_basedOn; }
    Q_SIGNAL void basedOnChanged();

    // clang-format off
    Q_PROPERTY(QString authorValue
               READ authorValue
               WRITE setAuthor
               NOTIFY authorChanged)
    Q_PROPERTY(QString author
               READ author
               WRITE setAuthor
               NOTIFY authorChanged)
    // clang-format on
    void setAuthor(const QString &val);
    QString author() const;
    QString authorValue() const { return m_author; }
    Q_SIGNAL void authorChanged();

    // clang-format off
    Q_PROPERTY(QString contact
               READ contact
               WRITE setContact
               NOTIFY contactChanged)
    // clang-format on
    void setContact(const QString &val);
    QString contact() const { return m_contact; }
    Q_SIGNAL void contactChanged();

    // clang-format off
    Q_PROPERTY(QString address
               READ address
               WRITE setAddress
               NOTIFY addressChanged)
    // clang-format on
    void setAddress(QString val);
    QString address() const { return m_address; }
    Q_SIGNAL void addressChanged();

    // clang-format off
    Q_PROPERTY(QString phoneNumber
               READ phoneNumber
               WRITE setPhoneNumber
               NOTIFY phoneNumberChanged)
    // clang-format on
    void setPhoneNumber(QString val);
    QString phoneNumber() const { return m_phoneNumber; }
    Q_SIGNAL void phoneNumberChanged();

    // clang-format off
    Q_PROPERTY(QString email
               READ email
               WRITE setEmail
               NOTIFY emailChanged)
    // clang-format on
    void setEmail(QString val);
    QString email() const { return m_email; }
    Q_SIGNAL void emailChanged();

    // clang-format off
    Q_PROPERTY(QString website
               READ website
               WRITE setWebsite
               NOTIFY websiteChanged)
    // clang-format on
    void setWebsite(QString val);
    QString website() const { return m_website; }
    Q_SIGNAL void websiteChanged();

    // clang-format off
    Q_PROPERTY(QString version
               READ version
               WRITE setVersion
               NOTIFY versionChanged)
    // clang-format on
    void setVersion(const QString &val);
    QString version() const { return m_version; }
    Q_SIGNAL void versionChanged();

    // clang-format off
    Q_PROPERTY(bool isEmpty
               READ isEmpty
               NOTIFY emptyChanged)
    // clang-format on
    bool isEmpty() const;
    Q_SIGNAL void emptyChanged();

    // clang-format off
    Q_PROPERTY(bool heightHintsAvailable
               READ isHeightHintsAvailable
               NOTIFY heightHintsAvailableChanged)
    // clang-format on
    bool isHeightHintsAvailable() const { return m_heightHintsAvailable; }
    Q_SIGNAL void heightHintsAvailableChanged();

    // clang-format off
    Q_PROPERTY(QString coverPagePhoto
               READ coverPagePhoto
               NOTIFY coverPagePhotoChanged
               STORED false)
    // clang-format on
    Q_INVOKABLE void setCoverPagePhoto(const QString &val);
    Q_INVOKABLE void clearCoverPagePhoto();
    QString coverPagePhoto() const { return m_coverPagePhoto; }
    Q_SIGNAL void coverPagePhotoChanged();

    enum CoverPagePhotoSize { SmallCoverPhoto, MediumCoverPhoto, LargeCoverPhoto };
    Q_ENUM(CoverPagePhotoSize)
    // clang-format off
    Q_PROPERTY(CoverPagePhotoSize coverPagePhotoSize
               READ coverPagePhotoSize
               WRITE setCoverPagePhotoSize
               NOTIFY coverPagePhotoSizeChanged)
    // clang-format on
    void setCoverPagePhotoSize(CoverPagePhotoSize val);
    CoverPagePhotoSize coverPagePhotoSize() const { return m_coverPagePhotoSize; }
    Q_SIGNAL void coverPagePhotoSizeChanged();

    // clang-format off
    Q_PROPERTY(bool titlePageIsCentered
               READ isTitlePageIsCentered
               WRITE setTitlePageIsCentered
               NOTIFY titlePageIsCenteredChanged)
    // clang-format on
    void setTitlePageIsCentered(bool val);
    bool isTitlePageIsCentered() const { return m_titlePageIsCentered; }
    Q_SIGNAL void titlePageIsCenteredChanged();

    // clang-format off
    Q_PROPERTY(bool hasTitlePageAttributes
               READ hasTitlePageAttributes
               NOTIFY hasTitlePageAttributesChanged)
    // clang-format on
    bool hasTitlePageAttributes() const { return m_hasTitlePageAttributes; }
    Q_SIGNAL void hasTitlePageAttributesChanged();

    // clang-format off
    Q_PROPERTY(bool hasNonStandardScenes
               READ hasNonStandardScenes
               NOTIFY hasNonStandardScenesChanged)
    // clang-format on
    bool hasNonStandardScenes() const { return m_hasNonStandardScenes; }
    Q_SIGNAL void hasNonStandardScenesChanged();

    // clang-format off
    Q_PROPERTY(bool hasSelectedElements
               READ hasSelectedElements
               NOTIFY selectionChanged)
    // clang-format on
    bool hasSelectedElements() const;

    // clang-format off
    Q_PROPERTY(int selectedElementsCount
               READ selectedElementsCount
               NOTIFY selectionChanged)
    // clang-format on
    int selectedElementsCount() const;
    Q_SIGNAL void selectionChanged();

    enum OmitStatus { Omitted, NotOmitted, PartiallyOmitted };
    Q_ENUM(OmitStatus)

    // clang-format off
    Q_PROPERTY(OmitStatus selectedElementsOmitStatus
               READ selectedElementsOmitStatus
               WRITE setSelectedElementsOmitStatus
               NOTIFY selectedElementsOmitStatusChanged)
    // clang-format on
    void setSelectedElementsOmitStatus(OmitStatus val);
    OmitStatus selectedElementsOmitStatus() const;
    Q_SIGNAL void selectedElementsOmitStatusChanged();

    // clang-format off
    Q_PROPERTY(QQmlListProperty<ScreenplayElement> elements
               READ elements
               NOTIFY elementsChanged)
    // clang-format on
    QQmlListProperty<ScreenplayElement> elements();
    Q_INVOKABLE void addElement(ScreenplayElement *ptr);
    Q_INVOKABLE void addScene(Scene *scene);
    Q_INVOKABLE void insertElementAt(ScreenplayElement *ptr, int index);
    void insertElementsAt(const QList<ScreenplayElement *> &elements, int index);
    Q_INVOKABLE void removeElement(ScreenplayElement *ptr);
    void removeElements(const QList<ScreenplayElement *> &elements);
    Q_INVOKABLE void moveElement(ScreenplayElement *ptr, int toRow);
    Q_INVOKABLE void moveSelectedElements(int toRow);
    Q_INVOKABLE void removeSelectedElements();
    Q_INVOKABLE void omitSelectedElements();
    Q_INVOKABLE void includeSelectedElements();
    Q_INVOKABLE void clearSelection();
    Q_INVOKABLE QList<int> selectedElementIndexes() const;
    void setSelection(const QList<ScreenplayElement *> &elements);
    Q_INVOKABLE ScreenplayElement *elementAt(int index) const;
    Q_INVOKABLE ScreenplayElement *elementWithIndex(int index) const;
    // clang-format off
    Q_PROPERTY(int elementCount
               READ elementCount
               NOTIFY elementCountChanged)
    // clang-format on
    int elementCount() const;
    Q_INVOKABLE void clearElements();
    Q_SIGNAL void elementCountChanged();
    Q_SIGNAL void elementsChanged();
    Q_SIGNAL void elementInserted(ScreenplayElement *element, int index);
    Q_SIGNAL void elementRemoved(ScreenplayElement *element, int index);
    Q_SIGNAL void elementMoved(ScreenplayElement *element, int from, int to);
    Q_SIGNAL void elementOmitted(ScreenplayElement *element, int index);
    Q_SIGNAL void elementIncluded(ScreenplayElement *element, int index);
    Q_SIGNAL void elementSceneContentChanged(ScreenplayElement *element, Scene *scene);
    Q_SIGNAL void elementSceneHeadingChanged(ScreenplayElement *element,
                                             SceneHeading *sceneHeading);
    Q_SIGNAL void elementSceneElementChanged(ScreenplayElement *element,
                                             SceneElement *sceneElement);
    Q_SIGNAL void aboutToMoveElements(int at);

    Q_SIGNAL void elementTagsChanged(ScreenplayElement *ptr);
    Q_SIGNAL void elementSceneGroupsChanged(ScreenplayElement *ptr);

    Q_INVOKABLE void gatherSelectedScenes(SceneGroup *into);

    Q_INVOKABLE ScreenplayElement *splitElement(ScreenplayElement *ptr, SceneElement *element,
                                                int textPosition);
    Q_INVOKABLE ScreenplayElement *mergeElementWithPrevious(ScreenplayElement *ptr);

    Q_INVOKABLE void removeSceneElements(Scene *scene);
    Q_INVOKABLE int firstIndexOfScene(Scene *scene) const;
    Q_INVOKABLE int indexOfElement(ScreenplayElement *element) const;
    Q_INVOKABLE int indexOfSerialNumber(int serialNumber) const;
    Q_INVOKABLE QList<int> sceneElementIndexes(Scene *scene, int max = -1) const;
    QList<ScreenplayElement *> sceneElements(Scene *scene, int max = -1) const;
    Q_INVOKABLE int firstSceneElementIndex() const;
    Q_INVOKABLE int lastSceneElementIndex() const;
    Q_INVOKABLE QList<int> sceneElementsInBreak(ScreenplayElement *element) const;

    int dialogueCount() const;
    QList<ScreenplayElement *> getElements() const { return m_elements; }
    QList<ScreenplayElement *>
    getFilteredElements(std::function<bool(ScreenplayElement *item)> filterFunc) const;
    bool setElements(const QList<ScreenplayElement *> &list);

    enum BreakType { Act, Episode, Chapter = Episode, Interval };
    Q_ENUM(BreakType)
    Q_INVOKABLE void addBreakElement(Screenplay::BreakType type);
    Q_INVOKABLE void addBreakElementI(int type) { this->addBreakElement(BreakType(type)); }
    Q_INVOKABLE void insertBreakElement(Screenplay::BreakType type, int index);
    Q_INVOKABLE void insertBreakElementI(int type, int index)
    {
        this->insertBreakElement(BreakType(type), index);
    }
    Q_INVOKABLE void updateBreakTitles();
    Q_SIGNAL void breakTitleChanged();
    void updateBreakTitlesLater();

    // clang-format off
    Q_PROPERTY(int sceneCount
               READ sceneCount
               NOTIFY sceneCountChanged)
    // clang-format on
    int sceneCount() const { return m_sceneCount; }
    Q_SIGNAL void sceneCountChanged();

    // clang-format off
    Q_PROPERTY(int actCount
               READ actCount
               NOTIFY actCountChanged)
    // clang-format on
    int actCount() const { return m_actCount; }
    Q_SIGNAL void actCountChanged();

    // clang-format off
    Q_PROPERTY(int episodeCount
               READ episodeCount
               NOTIFY episodeCountChanged)
    // clang-format on
    int episodeCount() const { return m_episodeCount; }
    Q_SIGNAL void episodeCountChanged();

    // clang-format off
    Q_PROPERTY(QList<ScreenplayBreakInfo> episodeInfoList
               READ episodeInfoList
               NOTIFY episodeCountChanged)
    // clang-format on
    QList<ScreenplayBreakInfo> episodeInfoList() const;

    Q_SIGNAL void screenplayChanged();

    // clang-format off
    Q_PROPERTY(int currentElementIndex
               READ currentElementIndex
               WRITE setCurrentElementIndex
               NOTIFY currentElementIndexChanged
               STORED false)
    // clang-format on
    void setCurrentElementIndex(int val);
    int currentElementIndex() const { return m_currentElementIndex; }
    Q_SIGNAL void currentElementIndexChanged(int val);

    Q_SIGNAL void requestEditorAt(int index);

    Q_INVOKABLE int previousSceneElementIndex() const;
    Q_INVOKABLE int nextSceneElementIndex() const;

    // clang-format off
    Q_PROPERTY(Scene *activeScene
               READ activeScene
               WRITE setActiveScene
               NOTIFY activeSceneChanged
               STORED false
               RESET resetActiveScene)
    // clang-format on
    void setActiveScene(Scene *val);
    Scene *activeScene() const { return m_activeScene; }
    Q_SIGNAL void activeSceneChanged();

    Q_SIGNAL void sceneReset(int sceneIndex, int sceneElementIndex);

    Q_INVOKABLE QJsonArray search(const QString &text, int flags = 0) const;
    Q_INVOKABLE int replace(const QString &text, const QString &replacementText, int flags = 0);

    // clang-format off
    Q_PROPERTY(int minimumParagraphCount
               READ minimumParagraphCount
               NOTIFY paragraphCountChanged)
    // clang-format on
    int minimumParagraphCount() const { return m_minimumParagraphCount; }

    // clang-format off
    Q_PROPERTY(int maximumParagraphCount
               READ maximumParagraphCount
               NOTIFY paragraphCountChanged)
    // clang-format on
    int maximumParagraphCount() const { return m_maximumParagraphCount; }

    // clang-format off
    Q_PROPERTY(int averageParagraphCount
               READ averageParagraphCount
               NOTIFY paragraphCountChanged)
    // clang-format on
    int averageParagraphCount() const { return m_averageParagraphCount; }

    Q_SIGNAL void paragraphCountChanged();

    Q_INVOKABLE void resetSceneNumbers();

    Q_INVOKABLE bool polishText();
    Q_INVOKABLE bool capitalizeSentences();

    // clang-format off
    Q_PROPERTY(int wordCount
               READ wordCount
               NOTIFY wordCountChanged)
    // clang-format on
    int wordCount() const { return m_wordCount; }
    Q_SIGNAL void wordCountChanged();

    // clang-format off
    Q_PROPERTY(bool canPaste
               READ canPaste
               NOTIFY canPasteChanged)
    // clang-format on
    bool canPaste() const;
    Q_SIGNAL void canPasteChanged();

    Q_INVOKABLE void copySelection();
    Q_INVOKABLE void pasteAfter(int index);

    // QObjectSerializer::Interface interface
    void serializeToJson(QJsonObject &) const;
    void deserializeFromJson(const QJsonObject &);
    bool canSetPropertyFromObjectList(const QString &propName) const;
    void setPropertyFromObjectList(const QString &propName, const QList<QObject *> &objects);

    // QAbstractItemModel interface
    enum Roles {
        IdRole = Qt::UserRole,
        ScreenplayElementRole,
        ScreenplayElementTypeRole,
        DelegateKindRole,
        BreakTypeRole,
        SceneRole
    };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

    // Text Document Export Support
    struct WriteOptions
    {
        WriteOptions() { }
        bool includeTextNotes = true;
        bool includeFormNotes = true;
        bool actsOnNewPage = false;
        bool episodesOnNewPage = false;
    };
    void write(QTextCursor &cursor, const WriteOptions &options = WriteOptions()) const;

    static int fountainCopyOptions();
    static int fountainPasteOptions();

protected:
    bool event(QEvent *event);
    void timerEvent(QTimerEvent *te);
    void resetActiveScene();
    void onScreenplayElementChanged();
    void onSceneReset(int elementIndex);
    void onSceneContentChanged();
    void onSceneHeadingChanged();
    void onSceneElementChanged(SceneElement *sceneElement);
    void onScreenplayElementOmittedChanged();
    void evaluateSceneNumbers(bool minorAlso = false);
    void evaluateSceneNumbersLater();
    void validateCurrentElementIndex();
    void evaluateParagraphCounts();
    void evaluateParagraphCountsLater();
    void setHasNonStandardScenes(bool val);
    void setHasTitlePageAttributes(bool val);
    void evaluateHasTitlePageAttributes();
    QList<ScreenplayElement *> takeSelectedElements();
    void setActCount(int val);
    void setSceneCount(int val);
    void setEpisodeCount(int val);
    void onDfsAuction(const QString &filePath, int *claims);
    void connectToScreenplayElementSignals(ScreenplayElement *ptr);
    void disconnectFromScreenplayElementSignals(ScreenplayElement *ptr);
    void setWordCount(int val);
    void evaluateWordCount();
    void evaluateWordCountLater();
    bool getPasteDataFromClipboard(QJsonObject &clipboardJson) const;
    void setHeightHintsAvailable(bool val);
    void evaluateIfHeightHintsAreAvailable();
    void evaluateIfHeightHintsAreAvailableLater();

private:
    QString m_title;
    QString m_email;
    QString m_author;
    QString m_basedOn;
    QString m_logline;
    QString m_contact;
    QString m_version;
    QString m_website;
    QString m_address;
    QString m_subtitle;
    QString m_phoneNumber;
    QString m_coverPagePhoto;
    QString m_loglineComments;
    bool m_titlePageIsCentered = true;
    int m_minimumParagraphCount = 0;
    int m_maximumParagraphCount = 0;
    int m_averageParagraphCount = 0;
    bool m_hasTitlePageAttributes = false;
    bool m_heightHintsAvailable = false;
    ScriteDocument *m_scriteDocument = nullptr;
    CoverPagePhotoSize m_coverPagePhotoSize = LargeCoverPhoto;
    friend class ScreenplayTextDocument;
    friend class ScreenplayElement;

    static void staticAppendElement(QQmlListProperty<ScreenplayElement> *list,
                                    ScreenplayElement *ptr);
    static void staticClearElements(QQmlListProperty<ScreenplayElement> *list);
    static ScreenplayElement *staticElementAt(QQmlListProperty<ScreenplayElement> *list, int index);
    static int staticElementCount(QQmlListProperty<ScreenplayElement> *list);
    QList<ScreenplayElement *>
            m_elements; // We dont use ObjectListPropertyModel<ScreenplayElement*> for this because
                        // the Screenplay class is already a list model of screenplay elements.
    int m_currentElementIndex = -1;
    QObjectProperty<Scene> m_activeScene;
    bool m_hasNonStandardScenes = false;
    int m_episodeCount = 0;
    int m_actCount = 0;
    int m_sceneCount = 0;
    int m_wordCount = 0;

    ExecLaterTimer m_wordCountTimer;
    ExecLaterTimer m_updateBreakTitlesTimer;
    ExecLaterTimer m_sceneNumberEvaluationTimer;
    ExecLaterTimer m_paragraphCountEvaluationTimer;
    ExecLaterTimer m_evalHeightHintsAvailableTimer;
    ExecLaterTimer m_selectedElementsOmitStatusChangedTimer;
};

struct ScreenplayTrackItem
{
    Q_GADGET

public:
    Q_PROPERTY(bool valid READ isValid)
    bool isValid() const { return startIndex >= 0 && endIndex >= startIndex && !name.isEmpty(); }

    // clang-format off
    Q_PROPERTY(int startIndex
               MEMBER startIndex)
    // clang-format on
    int startIndex = -1; // elementIndex in ScreenplayElement class

    // clang-format off
    Q_PROPERTY(int endIndex
               MEMBER endIndex)
    // clang-format on
    int endIndex = -1; // elementIndex in ScreenplayElement class

    // clang-format off
    Q_PROPERTY(QString name
               MEMBER name)
    // clang-format on
    QString name; // Eg. Opening Image, Catalyst, B Story etc..

    // clang-format off
    Q_PROPERTY(QColor color
               MEMBER color)
    // clang-format on
    QColor color;

    ScreenplayTrackItem() { }
    ScreenplayTrackItem(int _startIndex, int _endIndex, const QString &_name,
                        const QColor &_color = Qt::transparent)
        : startIndex(_startIndex), endIndex(_endIndex), name(_name), color(_color)
    {
    }
    ScreenplayTrackItem(const ScreenplayTrackItem &other) { *this = other; }
    bool operator!=(const ScreenplayTrackItem &other) const { return !(*this == other); }
    ScreenplayTrackItem &operator=(const ScreenplayTrackItem &other)
    {
        this->startIndex = other.startIndex;
        this->endIndex = other.endIndex;
        this->name = other.name;
        this->color = other.color;
        return *this;
    }
    bool operator==(const ScreenplayTrackItem &other) const
    {
        return this->startIndex == other.startIndex && this->endIndex == other.endIndex
                && this->name == other.name && this->color == other.color;
    }
};
Q_DECLARE_METATYPE(ScreenplayTrackItem)
Q_DECLARE_METATYPE(QList<ScreenplayTrackItem>)

struct ScreenplayTrack
{
    Q_GADGET

public:
    Q_PROPERTY(bool valid READ isValid)
    bool isValid() const { return !name.isEmpty() && !items.isEmpty(); }

    // clang-format off
    Q_PROPERTY(QString name
               MEMBER name)
    // clang-format on
    QString name; // Eg. Save The Cat, Heroes Journey etc, empty in case of keywords/open-tags

    // clang-format off
    Q_PROPERTY(QColor color
               MEMBER color)
    // clang-format on
    QColor color;

    // clang-format off
    Q_PROPERTY(QList<ScreenplayTrackItem> items
               MEMBER items)
    // clang-format on
    QList<ScreenplayTrackItem> items;

    ScreenplayTrack() { }
    ScreenplayTrack(const QString &_name, const QColor &_color,
                    const QList<ScreenplayTrackItem> &_items)
        : name(_name), color(_color), items(_items)
    {
    }
    ScreenplayTrack(const ScreenplayTrack &other) { *this = other; }
    bool operator!=(const ScreenplayTrack &other) const { return !(*this == other); }
    ScreenplayTrack &operator=(const ScreenplayTrack &other)
    {
        this->name = other.name;
        this->color = other.color;
        this->items = other.items;
        return *this;
    }
    bool operator==(const ScreenplayTrack &other) const
    {
        return this->name == other.name && this->color == other.color && this->items == other.items;
    }
};
Q_DECLARE_METATYPE(ScreenplayTrack)
Q_DECLARE_METATYPE(QList<ScreenplayTrack>)

/**
 * Looks up scenes in a screenplay and determines tracks that it can overlay on top of scenes
 * based on the groups to which various scene belong
 */
class ScreenplayTracks : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ScreenplayTracks(QObject *parent = nullptr);
    ~ScreenplayTracks();

    // clang-format off
    Q_PROPERTY(Screenplay *screenplay
               READ screenplay
               WRITE setScreenplay
               NOTIFY screenplayChanged)
    // clang-format on
    void setScreenplay(Screenplay *val);
    Screenplay *screenplay() const { return m_screenplay; }
    Q_SIGNAL void screenplayChanged();

    // clang-format off
    Q_PROPERTY(Structure* structure
               READ structure
               WRITE setStructure
               NOTIFY structureChanged)
    // clang-format on
    void setStructure(Structure *val);
    Structure *structure() const { return m_structure; }
    Q_SIGNAL void structureChanged();

    // clang-format off
    Q_PROPERTY(bool includeStructureTags
               READ isIncludeStructureTags
               WRITE setIncludeStructureTags
               NOTIFY includeStructureTagsChanged)
    // clang-format on
    void setIncludeStructureTags(bool val);
    bool isIncludeStructureTags() const { return m_includeStructureTags; }
    Q_SIGNAL void includeStructureTagsChanged();

    // clang-format off
    Q_PROPERTY(bool includeOpenTags
               READ isIncludeOpenTags
               WRITE setIncludeOpenTags
               NOTIFY includeOpenTagsChanged)
    // clang-format on
    void setIncludeOpenTags(bool val);
    bool isIncludeOpenTags() const { return m_includeOpenTags; }
    Q_SIGNAL void includeOpenTagsChanged();

    // clang-format off
    Q_PROPERTY(QStringList allowedOpenTags
               READ allowedOpenTags
               WRITE setAllowedOpenTags
               NOTIFY allowedOpenTagsChanged)
    // clang-format on
    void setAllowedOpenTags(const QStringList &val);
    QStringList allowedOpenTags() const { return m_allowedOpenTags; }
    Q_SIGNAL void allowedOpenTagsChanged();

    // clang-format off
    Q_PROPERTY(bool includeStacks
               READ isIncludeStacks
               WRITE setIncludeStacks
               NOTIFY includeStacksChanged)
    // clang-format on
    void setIncludeStacks(bool val);
    bool isIncludeStacks() const { return m_includeStacks; }
    Q_SIGNAL void includeStacksChanged();

    // clang-format off
    Q_PROPERTY(QString stackTrackName
               READ stackTrackName
               WRITE setStackTrackName
               NOTIFY stackTrackNameChanged)
    // clang-format on
    void setStackTrackName(const QString &val);
    QString stackTrackName() const { return m_stackTrackName; }
    Q_SIGNAL void stackTrackNameChanged();

    // clang-format off
    Q_PROPERTY(int trackCount
               READ trackCount
               NOTIFY trackCountChanged)
    // clang-format on
    int trackCount() const { return m_tracks.size(); }
    Q_SIGNAL void trackCountChanged();

    // clang-format off
    Q_PROPERTY(QList<QColor> colors
               READ colors
               WRITE setColors
               NOTIFY colorsChanged)
    // clang-format on
    void setColors(const QList<QColor> &val);
    QList<QColor> colors() const { return m_colors; }
    Q_SIGNAL void colorsChanged();

    static QVector<QColor> defaultColors();

    Q_INVOKABLE ScreenplayTrack trackAt(int index) const;

    Q_INVOKABLE void reload() { this->refreshLater(); }

    // QAbstractItemModel interface
    enum { TrackRole = Qt::UserRole };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

protected:
    void timerEvent(QTimerEvent *te);

private:
    void refresh();
    void refreshLater();

private:
    bool m_includeStacks = true;
    bool m_includeOpenTags = true;
    bool m_includeStructureTags = true;
    QStringList m_allowedOpenTags;
    QString m_stackTrackName = QStringLiteral("Sequences");

    QList<QColor> m_colors;
    ExecLaterTimer m_refreshTimer;
    QList<ScreenplayTrack> m_tracks;
    QObjectProperty<Structure> m_structure;
    QObjectProperty<Screenplay> m_screenplay;
};

#endif // SCREENPLAY_H
