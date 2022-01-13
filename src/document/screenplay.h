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
    Q_INVOKABLE ScreenplayElement(QObject *parent = nullptr);
    ~ScreenplayElement();
    Q_SIGNAL void aboutToDelete(ScreenplayElement *element);

    Q_PROPERTY(int elementIndex READ elementIndex NOTIFY elementIndexChanged)
    int elementIndex() const { return m_elementIndex; }
    Q_SIGNAL void elementIndexChanged();

    Q_PROPERTY(int actIndex READ actIndex NOTIFY actIndexChanged)
    int actIndex() const { return m_actIndex; }
    Q_SIGNAL void actIndexChanged();

    Q_PROPERTY(int episodeIndex READ episodeIndex NOTIFY episodeIndexChanged)
    int episodeIndex() const { return m_episodeIndex; }
    Q_SIGNAL void episodeIndexChanged();

    enum ElementType { SceneElementType, BreakElementType };
    Q_ENUM(ElementType)
    Q_PROPERTY(ElementType elementType READ elementType WRITE setElementType NOTIFY elementTypeChanged)
    void setElementType(ElementType val);
    ElementType elementType() const { return m_elementType; }
    Q_SIGNAL void elementTypeChanged();

    Q_PROPERTY(int breakType READ breakType WRITE setBreakType NOTIFY breakTypeChanged)
    void setBreakType(int val);
    int breakType() const { return m_breakType; }
    Q_SIGNAL void breakTypeChanged();

    Q_PROPERTY(QString breakTitle READ breakTitle NOTIFY breakTitleChanged)
    QString breakTitle() const { return m_breakTitle.isEmpty() ? this->sceneID() : m_breakTitle; }
    Q_SIGNAL void breakTitleChanged();

    Q_PROPERTY(QString breakSubtitle READ breakSubtitle WRITE setBreakSubtitle NOTIFY breakSubtitleChanged)
    void setBreakSubtitle(const QString &val);
    QString breakSubtitle() const { return m_breakSubtitle; }
    Q_SIGNAL void breakSubtitleChanged();

    Q_PROPERTY(Screenplay* screenplay READ screenplay WRITE setScreenplay NOTIFY screenplayChanged STORED false RESET resetScreenplay)
    void setScreenplay(Screenplay *val);
    Screenplay *screenplay() const { return m_screenplay; }
    Q_SIGNAL void screenplayChanged();

    Q_PROPERTY(QString sceneID READ sceneID WRITE setSceneFromID NOTIFY sceneChanged)
    void setSceneFromID(const QString &val);
    QString sceneID() const;

    Q_PROPERTY(int sceneNumber READ sceneNumber NOTIFY sceneNumberChanged)
    int sceneNumber() const
    {
        return m_customSceneNumber < 0 ? m_sceneNumber : m_customSceneNumber;
    }
    Q_SIGNAL void sceneNumberChanged();

    Q_PROPERTY(QString userSceneNumber READ userSceneNumber WRITE setUserSceneNumber NOTIFY userSceneNumberChanged)
    void setUserSceneNumber(const QString &val);
    QString userSceneNumber() const { return m_userSceneNumber; }
    Q_SIGNAL void userSceneNumberChanged();

    Q_PROPERTY(bool hasUserSceneNumber READ hasUserSceneNumber NOTIFY userSceneNumberChanged)
    bool hasUserSceneNumber() const { return !m_userSceneNumber.isEmpty(); }

    Q_PROPERTY(QString resolvedSceneNumber READ resolvedSceneNumber NOTIFY resolvedSceneNumberChanged)
    QString resolvedSceneNumber() const;
    Q_SIGNAL void resolvedSceneNumberChanged();

    Q_PROPERTY(Scene* scene READ scene NOTIFY sceneChanged STORED false RESET resetScene)
    void setScene(Scene *val);
    Scene *scene() const { return m_scene; }
    Q_SIGNAL void sceneChanged();

    Q_PROPERTY(bool expanded READ isExpanded WRITE setExpanded NOTIFY expandedChanged)
    void setExpanded(bool val);
    bool isExpanded() const { return m_expanded; }
    Q_SIGNAL void expandedChanged();

    Q_PROPERTY(QJsonValue userData READ userData WRITE setUserData NOTIFY userDataChanged STORED false)
    void setUserData(const QJsonValue &val);
    QJsonValue userData() const { return m_userData; }
    Q_SIGNAL void userDataChanged();

    Q_PROPERTY(QJsonValue editorHints READ editorHints WRITE setEditorHints NOTIFY editorHintsChanged)
    void setEditorHints(const QJsonValue &val);
    QJsonValue editorHints() const { return m_editorHints; }
    Q_SIGNAL void editorHintsChanged();

    Q_PROPERTY(bool selected READ isSelected WRITE setSelected NOTIFY selectedChanged STORED false)
    void setSelected(bool val);
    bool isSelected() const { return m_selected; }
    Q_SIGNAL void selectedChanged();

    Q_PROPERTY(QString breakSummary READ breakSummary WRITE setBreakSummary NOTIFY breakSummaryChanged)
    void setBreakSummary(const QString &val);
    QString breakSummary() const { return m_breakSummary; }
    Q_SIGNAL void breakSummaryChanged();

    Q_PROPERTY(Notes* notes READ notes NOTIFY notesChanged)
    Notes *notes() const { return m_notes ? m_notes : (m_scene ? m_scene->notes() : nullptr); }
    Q_SIGNAL void notesChanged();

    Q_PROPERTY(Attachments* attachments READ attachments NOTIFY attachmentsChanged)
    Attachments *attachments() const
    {
        return m_attachments ? m_attachments : (m_scene ? m_scene->attachments() : nullptr);
    }
    Q_SIGNAL void attachmentsChanged();

    Q_INVOKABLE void toggleSelection() { this->setSelected(!m_selected); }

    Q_SIGNAL void elementChanged();

    Q_SIGNAL void sceneAboutToReset();
    Q_SIGNAL void sceneReset(int elementIndex);
    Q_SIGNAL void evaluateSceneNumberRequest();
    Q_SIGNAL void sceneTypeChanged();
    Q_SIGNAL void sceneGroupsChanged(ScreenplayElement *ptr);

    // QObjectSerializer::Interface interface
    bool canSerialize(const QMetaObject *, const QMetaProperty &) const;

protected:
    bool event(QEvent *event);
    void evaluateSceneNumber(int &number);
    void resetScene();
    void resetScreenplay();
    void setActIndex(int val);
    void setEpisodeIndex(int val);
    void setElementIndex(int val);
    void setBreakTitle(const QString &val);

private:
    void onSceneGroupsChanged() { emit sceneGroupsChanged(this); }
    void setNotes(Notes *val);
    void setAttachments(Attachments *val);

private:
    friend class Screenplay;
    friend class AbstractImporter;
    friend class AbstractScreenplaySubsetReport;

    bool m_expanded = true;
    int m_breakType = -1;
    bool m_selected = false;
    int m_actIndex = -1;
    int m_elementIndex = -1;
    int m_episodeIndex = -1;
    int m_sceneNumber = -1;
    QString m_sceneID;
    Notes *m_notes = nullptr;
    Attachments *m_attachments = nullptr;
    QString m_breakTitle;
    QJsonValue m_userData;
    QString m_breakSummary;
    QString m_breakSubtitle;
    int m_customSceneNumber = -1;
    bool m_elementTypeIsSet = false;
    QJsonValue m_editorHints;
    QString m_userSceneNumber;
    ElementType m_elementType = SceneElementType;
    QObjectProperty<Scene> m_scene;
    QObjectProperty<Screenplay> m_screenplay;
};

class Screenplay : public QAbstractListModel, public Modifiable, public QObjectSerializer::Interface
{
    Q_OBJECT
    Q_INTERFACES(QObjectSerializer::Interface)
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    Screenplay(QObject *parent = nullptr);
    ~Screenplay();
    Q_SIGNAL void aboutToDelete(Screenplay *ptr);

    Q_PROPERTY(ScriteDocument* scriteDocument READ scriteDocument CONSTANT STORED false)
    ScriteDocument *scriteDocument() const { return m_scriteDocument; }

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    void setTitle(const QString &val);
    QString title() const { return m_title; }
    Q_SIGNAL void titleChanged();

    Q_PROPERTY(QString subtitle READ subtitle WRITE setSubtitle NOTIFY subtitleChanged)
    void setSubtitle(const QString &val);
    QString subtitle() const { return m_subtitle; }
    Q_SIGNAL void subtitleChanged();

    Q_PROPERTY(QString logline READ logline WRITE setLogline NOTIFY loglineChanged)
    void setLogline(const QString &val);
    QString logline() const { return m_logline; }
    Q_SIGNAL void loglineChanged();

    Q_PROPERTY(QString basedOn READ basedOn WRITE setBasedOn NOTIFY basedOnChanged)
    void setBasedOn(const QString &val);
    QString basedOn() const { return m_basedOn; }
    Q_SIGNAL void basedOnChanged();

    Q_PROPERTY(QString author READ author WRITE setAuthor NOTIFY authorChanged)
    void setAuthor(const QString &val);
    QString author() const { return m_author; }
    Q_SIGNAL void authorChanged();

    Q_PROPERTY(QString contact READ contact WRITE setContact NOTIFY contactChanged)
    void setContact(const QString &val);
    QString contact() const { return m_contact; }
    Q_SIGNAL void contactChanged();

    Q_PROPERTY(QString address READ address WRITE setAddress NOTIFY addressChanged)
    void setAddress(QString val);
    QString address() const { return m_address; }
    Q_SIGNAL void addressChanged();

    Q_PROPERTY(QString phoneNumber READ phoneNumber WRITE setPhoneNumber NOTIFY phoneNumberChanged)
    void setPhoneNumber(QString val);
    QString phoneNumber() const { return m_phoneNumber; }
    Q_SIGNAL void phoneNumberChanged();

    Q_PROPERTY(QString email READ email WRITE setEmail NOTIFY emailChanged)
    void setEmail(QString val);
    QString email() const { return m_email; }
    Q_SIGNAL void emailChanged();

    Q_PROPERTY(QString website READ website WRITE setWebsite NOTIFY websiteChanged)
    void setWebsite(QString val);
    QString website() const { return m_website; }
    Q_SIGNAL void websiteChanged();

    Q_PROPERTY(QString version READ version WRITE setVersion NOTIFY versionChanged)
    void setVersion(const QString &val);
    QString version() const { return m_version; }
    Q_SIGNAL void versionChanged();

    Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY emptyChanged)
    bool isEmpty() const;
    Q_SIGNAL void emptyChanged();

    Q_PROPERTY(QString coverPagePhoto READ coverPagePhoto NOTIFY coverPagePhotoChanged STORED false)
    Q_INVOKABLE void setCoverPagePhoto(const QString &val);
    Q_INVOKABLE void clearCoverPagePhoto();
    QString coverPagePhoto() const { return m_coverPagePhoto; }
    Q_SIGNAL void coverPagePhotoChanged();

    enum CoverPagePhotoSize { SmallCoverPhoto, MediumCoverPhoto, LargeCoverPhoto };
    Q_ENUM(CoverPagePhotoSize)
    Q_PROPERTY(CoverPagePhotoSize coverPagePhotoSize READ coverPagePhotoSize WRITE setCoverPagePhotoSize NOTIFY coverPagePhotoSizeChanged)
    void setCoverPagePhotoSize(CoverPagePhotoSize val);
    CoverPagePhotoSize coverPagePhotoSize() const { return m_coverPagePhotoSize; }
    Q_SIGNAL void coverPagePhotoSizeChanged();

    Q_PROPERTY(bool titlePageIsCentered READ isTitlePageIsCentered WRITE setTitlePageIsCentered NOTIFY titlePageIsCenteredChanged)
    void setTitlePageIsCentered(bool val);
    bool isTitlePageIsCentered() const { return m_titlePageIsCentered; }
    Q_SIGNAL void titlePageIsCenteredChanged();

    Q_PROPERTY(bool hasTitlePageAttributes READ hasTitlePageAttributes NOTIFY hasTitlePageAttributesChanged)
    bool hasTitlePageAttributes() const { return m_hasTitlePageAttributes; }
    Q_SIGNAL void hasTitlePageAttributesChanged();

    Q_PROPERTY(bool hasNonStandardScenes READ hasNonStandardScenes NOTIFY hasNonStandardScenesChanged)
    bool hasNonStandardScenes() const { return m_hasNonStandardScenes; }
    Q_SIGNAL void hasNonStandardScenesChanged();

    Q_PROPERTY(bool hasSelectedElements READ hasSelectedElements NOTIFY hasSelectedElementsChanged)
    bool hasSelectedElements() const;
    Q_SIGNAL void hasSelectedElementsChanged();

    Q_PROPERTY(QQmlListProperty<ScreenplayElement> elements READ elements NOTIFY elementsChanged)
    QQmlListProperty<ScreenplayElement> elements();
    Q_INVOKABLE void addElement(ScreenplayElement *ptr);
    Q_INVOKABLE void addScene(Scene *scene);
    Q_INVOKABLE void insertElementAt(ScreenplayElement *ptr, int index);
    Q_INVOKABLE void removeElement(ScreenplayElement *ptr);
    Q_INVOKABLE void moveElement(ScreenplayElement *ptr, int toRow);
    Q_INVOKABLE void moveSelectedElements(int toRow);
    Q_INVOKABLE void removeSelectedElements();
    Q_INVOKABLE void clearSelection();
    Q_INVOKABLE ScreenplayElement *elementAt(int index) const;
    Q_INVOKABLE ScreenplayElement *elementWithIndex(int index) const;
    Q_PROPERTY(int elementCount READ elementCount NOTIFY elementCountChanged)
    int elementCount() const;
    Q_INVOKABLE void clearElements();
    Q_SIGNAL void elementCountChanged();
    Q_SIGNAL void elementsChanged();
    Q_SIGNAL void elementInserted(ScreenplayElement *element, int index);
    Q_SIGNAL void elementRemoved(ScreenplayElement *element, int index);
    Q_SIGNAL void elementMoved(ScreenplayElement *element, int from, int to);
    Q_SIGNAL void aboutToMoveElements(int at);

    Q_SIGNAL void elementSceneGroupsChanged(ScreenplayElement *ptr);

    Q_INVOKABLE void gatherSelectedScenes(SceneGroup *into);

    Q_INVOKABLE ScreenplayElement *splitElement(ScreenplayElement *ptr, SceneElement *element,
                                                int textPosition);
    Q_INVOKABLE ScreenplayElement *mergeElementWithPrevious(ScreenplayElement *ptr);

    Q_INVOKABLE void removeSceneElements(Scene *scene);
    Q_INVOKABLE int firstIndexOfScene(Scene *scene) const;
    Q_INVOKABLE int indexOfElement(ScreenplayElement *element) const;
    Q_INVOKABLE QList<int> sceneElementIndexes(Scene *scene, int max = -1) const;
    QList<ScreenplayElement *> sceneElements(Scene *scene, int max = -1) const;
    Q_INVOKABLE int firstSceneIndex() const;
    Q_INVOKABLE int lastSceneIndex() const;
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

    Q_PROPERTY(int sceneCount READ sceneCount NOTIFY sceneCountChanged)
    int sceneCount() const { return m_sceneCount; }
    Q_SIGNAL void sceneCountChanged();

    Q_PROPERTY(int actCount READ actCount NOTIFY actCountChanged)
    int actCount() const { return m_actCount; }
    Q_SIGNAL void actCountChanged();

    Q_PROPERTY(int episodeCount READ episodeCount NOTIFY episodeCountChanged)
    int episodeCount() const { return m_episodeCount; }
    Q_SIGNAL void episodeCountChanged();

    Q_SIGNAL void screenplayChanged();

    Q_PROPERTY(int currentElementIndex READ currentElementIndex WRITE setCurrentElementIndex NOTIFY currentElementIndexChanged)
    void setCurrentElementIndex(int val);
    int currentElementIndex() const { return m_currentElementIndex; }
    Q_SIGNAL void currentElementIndexChanged(int val);

    Q_SIGNAL void requestEditorAt(int index);

    Q_INVOKABLE int previousSceneElementIndex();
    Q_INVOKABLE int nextSceneElementIndex();

    Q_PROPERTY(Scene* activeScene READ activeScene WRITE setActiveScene NOTIFY activeSceneChanged STORED false RESET resetActiveScene)
    void setActiveScene(Scene *val);
    Scene *activeScene() const { return m_activeScene; }
    Q_SIGNAL void activeSceneChanged();

    Q_SIGNAL void sceneReset(int sceneIndex, int sceneElementIndex);

    Q_INVOKABLE QJsonArray search(const QString &text, int flags = 0) const;
    Q_INVOKABLE int replace(const QString &text, const QString &replacementText, int flags = 0);

    Q_PROPERTY(int minimumParagraphCount READ minimumParagraphCount NOTIFY paragraphCountChanged)
    int minimumParagraphCount() const { return m_minimumParagraphCount; }

    Q_PROPERTY(int maximumParagraphCount READ maximumParagraphCount NOTIFY paragraphCountChanged)
    int maximumParagraphCount() const { return m_maximumParagraphCount; }

    Q_PROPERTY(int averageParagraphCount READ averageParagraphCount NOTIFY paragraphCountChanged)
    int averageParagraphCount() const { return m_averageParagraphCount; }

    Q_SIGNAL void paragraphCountChanged();

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
        BreakTypeRole,
        SceneRole
    };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

protected:
    bool event(QEvent *event);
    void timerEvent(QTimerEvent *te);
    void resetActiveScene();
    void onSceneReset(int elementIndex);
    void evaluateSceneNumbers();
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
    bool m_titlePageIsCentered = true;
    int m_minimumParagraphCount = 0;
    int m_maximumParagraphCount = 0;
    int m_averageParagraphCount = 0;
    bool m_hasTitlePageAttributes = false;
    ScriteDocument *m_scriteDocument = nullptr;
    CoverPagePhotoSize m_coverPagePhotoSize = LargeCoverPhoto;
    friend class ScreenplayTextDocument;

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

    ExecLaterTimer m_updateBreakTitlesTimer;
    ExecLaterTimer m_sceneNumberEvaluationTimer;
    ExecLaterTimer m_paragraphCountEvaluationTimer;
};

/**
 * Looks up scenes in a screenplay and determines tracks that it can overlay on top of scenes
 * based on the groups to which various scene belong
 */
class ScreenplayTracks : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    ScreenplayTracks(QObject *parent = nullptr);
    ~ScreenplayTracks();

    Q_PROPERTY(Screenplay* screenplay READ screenplay WRITE setScreenplay NOTIFY screenplayChanged)
    void setScreenplay(Screenplay *val);
    Screenplay *screenplay() const { return m_screenplay; }
    Q_SIGNAL void screenplayChanged();

    Q_PROPERTY(int trackCount READ trackCount NOTIFY trackCountChanged)
    int trackCount() const { return m_data.size(); }
    Q_SIGNAL void trackCountChanged();

    // QAbstractItemModel interface
    enum { ModelDataRole = Qt::UserRole };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

protected:
    void timerEvent(QTimerEvent *te);

private:
    void refresh();
    void refreshLater();
    void onElementSceneGroupsChanged(ScreenplayElement *) { this->refreshLater(); }

private:
    QObjectProperty<Screenplay> m_screenplay;
    QList<QVariantMap> m_data;
    ExecLaterTimer m_refreshTimer;
};

#endif // SCREENPLAY_H
