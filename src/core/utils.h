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

#ifndef UTILS_H
#define UTILS_H

#include <QImage>
#include <QtMath>
#include <QFileInfo>
#include <QDateTime>
#include <QQuickItem>
#include <QVersionNumber>
#include <QAbstractItemModel>

namespace Utils {

struct KeyCombinations
{
    Q_GADGET
    QML_ANONYMOUS
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    // clang-format off
    Q_PROPERTY(bool controlModifier
               MEMBER controlModifier)
    // clang-format on
    bool controlModifier = false;

    // clang-format off
    Q_PROPERTY(bool shiftModifier
               MEMBER shiftModifier)
    // clang-format on
    bool shiftModifier = false;

    // clang-format off
    Q_PROPERTY(bool altModifier
               MEMBER altModifier)
    // clang-format on
    bool altModifier = false;

    // clang-format off
    Q_PROPERTY(bool metaModifier
               MEMBER metaModifier)
    // clang-format on
    bool metaModifier = false;

    // clang-format off
    Q_PROPERTY(QStringList modifiers
               READ modifiers)
    QStringList modifiers() const;

    // clang-format off
    Q_PROPERTY(QList<int> keyCodes
               MEMBER keyCodes)
    // clang-format on
    QList<int> keyCodes;

    // clang-format off
    Q_PROPERTY(QStringList keys
               MEMBER keys)
    // clang-format on
    QStringList keys;

    Q_INVOKABLE QString toShortcut() const;
    Q_INVOKABLE QKeySequence toKeySequence() const;

    KeyCombinations() { }
    KeyCombinations(bool _controlModifier, bool _shiftModifier, bool _altModifier,
                    bool _metaModifier, const QList<int> &_keyCodes, const QStringList &_keys)
        : controlModifier(_controlModifier),
          shiftModifier(_shiftModifier),
          altModifier(_altModifier),
          metaModifier(_metaModifier),
          keyCodes(_keyCodes),
          keys(_keys)
    {
    }
    KeyCombinations(const KeyCombinations &other) { *this = other; }
    KeyCombinations &operator=(const KeyCombinations &other)
    {
        this->controlModifier = other.controlModifier;
        this->shiftModifier = other.shiftModifier;
        this->altModifier = other.altModifier;
        this->metaModifier = other.metaModifier;
        this->keyCodes = other.keyCodes;
        this->keys = other.keys;
        return *this;
    }
    bool operator==(const KeyCombinations &other) const
    {
        return this->controlModifier == other.controlModifier
                && this->shiftModifier == other.shiftModifier
                && this->altModifier == other.altModifier
                && this->metaModifier == other.metaModifier && this->keyCodes == other.keyCodes
                && this->keys == other.keys;
    }
    bool operator!=(const KeyCombinations &other) const { return !(*this == other); }
};

struct FileInfo
{
    Q_GADGET
    QML_ANONYMOUS
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    QFileInfo info;

    // clang-format off
    Q_PROPERTY(bool valid
               READ valid)
    // clang-format on
    bool valid() const { return info != QFileInfo(); }

    // clang-format off
    Q_PROPERTY(bool exists
               READ exists)
    // clang-format on
    bool exists() const { return info.exists(); }

    // clang-format off
    Q_PROPERTY(bool readable
               READ readable)
    // clang-format on
    bool readable() const { return info.isReadable(); }

    // clang-format off
    Q_PROPERTY(bool writable
               READ writable)
    // clang-format on
    bool writable() const { return info.isWritable(); }

    // clang-format off
    Q_PROPERTY(QString baseName
               READ baseName)
    // clang-format on
    QString baseName() const { return info.baseName(); }

    // clang-format off
    Q_PROPERTY(QString absoluteFilePath
               READ absoluteFilePath)
    // clang-format on
    QString absoluteFilePath() const { return info.absoluteFilePath(); }

    // clang-format off
    Q_PROPERTY(QString absolutePath
               READ absolutePath)
    // clang-format on
    QString absolutePath() const { return info.absolutePath(); }

    // clang-format off
    Q_PROPERTY(QString suffix
               READ suffix)
    // clang-format on
    QString suffix() const { return info.suffix(); }

    // clang-format off
    Q_PROPERTY(QString fileName
               READ fileName)
    // clang-format on
    QString fileName() const { return info.fileName(); }

    FileInfo() { }
    FileInfo(const FileInfo &other) { *this = other; }
    FileInfo(const QFileInfo &_fileInfo) : info(_fileInfo) { }
    FileInfo &operator=(const FileInfo &other)
    {
        this->info = other.info;
        return *this;
    }
    bool operator==(const FileInfo &other) const { return info == other.info; }
    bool operator!=(const FileInfo &other) const { return info != other.info; }
};

class ObjectConfigFieldChoice
{
    Q_GADGET
    QML_ANONYMOUS
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    // clang-format off
    Q_PROPERTY(QString key
               MEMBER key)
    // clang-format on
    QString key;

    // clang-format off
    Q_PROPERTY(QString value
               MEMBER value)
    // clang-format on
    QString value;

    ObjectConfigFieldChoice() { }
    ObjectConfigFieldChoice(const QString &_key, const QString &_value) : key(_key), value(_value)
    {
    }
    ObjectConfigFieldChoice(const ObjectConfigFieldChoice &other) { *this = other; }
    ObjectConfigFieldChoice &operator=(const ObjectConfigFieldChoice &other)
    {
        this->key = other.key;
        this->value = other.value;
        return *this;
    }
    bool operator==(const ObjectConfigFieldChoice &other) const
    {
        return this->key == other.key && this->value == other.value;
    }
    bool operator!=(const ObjectConfigFieldChoice &other) const { return !(*this == other); }
};

struct ObjectConfigField
{
    Q_GADGET
    QML_ANONYMOUS
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    // clang-format off
    Q_PROPERTY(QString name
               MEMBER name)
    // clang-format on
    QString name;

    // clang-format off
    Q_PROPERTY(QString label
               MEMBER label)
    // clang-format on
    QString label;

    // clang-format off
    Q_PROPERTY(QString note
               MEMBER note)
    // clang-format on
    QString note;

    // clang-format off
    Q_PROPERTY(QString editor
               MEMBER editor)
    // clang-format on
    QString editor;

    // clang-format off
    Q_PROPERTY(QVariant min
               MEMBER min)
    // clang-format on
    QVariant min;

    // clang-format off
    Q_PROPERTY(QVariant max
               MEMBER max)
    // clang-format on
    QVariant max;

    // clang-format off
    Q_PROPERTY(QVariant ideal
               MEMBER ideal)
    // clang-format on
    QVariant ideal;

    // clang-format off
    Q_PROPERTY(QString group
               MEMBER group)
    // clang-format on
    QString group;

    // clang-format off
    Q_PROPERTY(QString feature
               MEMBER feature)
    // clang-format on
    QString feature;

    // clang-format off
    Q_PROPERTY(QList<Utils::ObjectConfigFieldChoice> choices
               MEMBER choices)
    // clang-format on
    QList<Utils::ObjectConfigFieldChoice> choices;

    ObjectConfigField() { }
    ObjectConfigField(const ObjectConfigField &other) { *this = other; }
    ObjectConfigField(const QString &_name, const QString &_label, const QString &_note,
                      const QString &_editor, const QVariant &_min, const QVariant &_max,
                      const QVariant &_ideal, const QString &_group, const QString &_feature,
                      const QList<Utils::ObjectConfigFieldChoice> &_choices)
        : name(_name),
          label(_label),
          note(_note),
          editor(_editor),
          min(_min),
          max(_max),
          ideal(_ideal),
          group(_group),
          feature(_feature),
          choices(_choices)
    {
    }
    ObjectConfigField &operator=(const ObjectConfigField &other)
    {
        this->name = other.name;
        this->label = other.label;
        this->note = other.note;
        this->editor = other.editor;
        this->min = other.min;
        this->max = other.max;
        this->ideal = other.ideal;
        this->group = other.group;
        this->feature = other.feature;
        this->choices = other.choices;
        return *this;
    }
    bool operator==(const ObjectConfigField &other) const
    {
        return this->name == other.name && this->label == other.label && this->note == other.note
                && this->editor == other.editor && this->min == other.min && this->max == other.max
                && this->ideal == other.ideal && this->group == other.group
                && this->feature == other.feature && this->choices == other.choices;
    }
    bool operator!=(const ObjectConfigField &other) const { return !(*this == other); }
};

struct ObjectConfigFieldGroup
{
    Q_GADGET
    QML_ANONYMOUS
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    // clang-format off
    Q_PROPERTY(QString name
               MEMBER name)
    // clang-format on
    QString name;

    // clang-format off
    Q_PROPERTY(QString description
               MEMBER description)
    // clang-format on
    QString description;

    // clang-format off
    Q_PROPERTY(QList<Utils::ObjectConfigField> fields
               MEMBER fields)
    // clang-format on
    QList<Utils::ObjectConfigField> fields;

    ObjectConfigFieldGroup() { }
    ObjectConfigFieldGroup(const ObjectConfigFieldGroup &other) { *this = other; }
    ObjectConfigFieldGroup(const QString &_name, const QString &_description,
                           const QList<Utils::ObjectConfigField> &_fields)
        : name(_name), description(_description), fields(_fields)
    {
    }
    ObjectConfigFieldGroup &operator=(const ObjectConfigFieldGroup &other)
    {
        this->name = other.name;
        this->description = other.description;
        this->fields = other.fields;
        return *this;
    }
    bool operator==(const ObjectConfigFieldGroup &other) const
    {
        return this->name == other.name && this->description == other.description
                && this->fields == other.fields;
    }
    bool operator!=(const ObjectConfigFieldGroup &other) const { return !(*this == other); }
};

struct ObjectConfig
{
    Q_GADGET
    QML_ANONYMOUS
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    // clang-format off
    Q_PROPERTY(bool valid
               READ isValid)
    // clang-format on
    bool isValid() const { return fields.size() > 0 && groups.size() >= 1; }

    // clang-format off
    Q_PROPERTY(QString title
               MEMBER title)
    // clang-format on
    QString title;

    // clang-format off
    Q_PROPERTY(QString description
               MEMBER description)
    // clang-format on
    QString description;

    // clang-format off
    Q_PROPERTY(QList<Utils::ObjectConfigField> fields
               MEMBER fields)
    // clang-format on
    QList<Utils::ObjectConfigField> fields;

    // clang-format off
    Q_PROPERTY(QList<Utils::ObjectConfigFieldGroup> groups
               MEMBER groups)
    // clang-format on
    QList<Utils::ObjectConfigFieldGroup> groups;

    ObjectConfig() { }
    ObjectConfig(const ObjectConfig &other) { *this = other; }
    ObjectConfig(const QString &_title, const QString &_description,
                 const QList<Utils::ObjectConfigField> &_fields,
                 const QList<Utils::ObjectConfigFieldGroup> &_groups)
        : title(_title), description(_description), fields(_fields), groups(_groups)
    {
    }
    ObjectConfig &operator=(const ObjectConfig &other)
    {
        this->title = other.title;
        this->description = other.description;
        this->fields = other.fields;
        this->groups = other.groups;
        return *this;
    }
    bool operator==(const ObjectConfig &other) const
    {
        return this->title == other.title && this->description == other.description
                && this->fields == other.fields && this->groups == other.groups;
    }
    bool operator!=(const ObjectConfig &other) const { return !(*this == other); }
};

// Must be called from main() before QML engine is constructed
void registerTypes();

}

/**
 *  Since the following types are in a namespace, whereever they are used with MOC macros
 *  they should be mentioned with their namespace. This means, for example:
 *
 *  The folowing will __not__ work.
 *   Q_PROPERTY(QList<ObjectConfigFieldChoice> choices MEMBER choices)
 *   QList<ObjectConfigFieldChoice> choices;
 *
 *  Why? Because, we cannot refer to ObjectConfigFieldChoice without using Utils namespace. So, it
 *  has to be:
 *
 *   Q_PROPERTY(QList<Utils::ObjectConfigFieldChoice> choices MEMBER choices)
 *   QList<Utils::ObjectConfigFieldChoice> choices;
 */
Q_DECLARE_METATYPE(Utils::FileInfo)
Q_DECLARE_METATYPE(Utils::ObjectConfigFieldChoice)
Q_DECLARE_METATYPE(QList<Utils::ObjectConfigFieldChoice>)
Q_DECLARE_METATYPE(Utils::ObjectConfigField)
Q_DECLARE_METATYPE(QList<Utils::ObjectConfigField>)
Q_DECLARE_METATYPE(Utils::ObjectConfigFieldGroup)
Q_DECLARE_METATYPE(QList<Utils::ObjectConfigFieldGroup>)
Q_DECLARE_METATYPE(Utils::ObjectConfig)

namespace Utils {

class Platform : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Platform)
    QML_SINGLETON

public:
    // Platform queries
    enum Type { LinuxDesktop, WindowsDesktop, MacOSDesktop };
    Q_ENUM(Type)

    // clang-format off
    Q_PROPERTY(Type type
               READ type
               CONSTANT )
    // clang-format on
    static Type type();

    // clang-format off
    Q_PROPERTY(QString typeString
               READ typeString
               CONSTANT )
    // clang-format on
    static QString typeString();

    // clang-format off
    Q_PROPERTY(bool isLinuxDesktop
               READ isLinuxDesktop
               CONSTANT )
    // clang-format on
    static bool isLinuxDesktop() { return type() == LinuxDesktop; }

    // clang-format off
    Q_PROPERTY(bool isWindowsDesktop
               READ isWindowsDesktop
               CONSTANT )
    // clang-format on
    static bool isWindowsDesktop() { return type() == WindowsDesktop; }

    // clang-format off
    Q_PROPERTY(bool isMacOSDesktop
               READ isMacOSDesktop
               CONSTANT )
    // clang-format on
    static bool isMacOSDesktop() { return type() == MacOSDesktop; }

    // clang-format off
    Q_PROPERTY(int osMajorVersion
               READ osMajorVersion
               CONSTANT )
    // clang-format on
    static int osMajorVersion();

    // clang-format off
    Q_PROPERTY(QList<int> osVersion
               READ osVersion
               CONSTANT )
    // clang-format on
    static QList<int> osVersion();

    // clang-format off
    Q_PROPERTY(QString osVersionString
               READ osVersionString
               CONSTANT )
    // clang-format on
    static QString osVersionString();

    // clang-format off
    Q_PROPERTY(QString qtVersionString
               READ qtVersionString
               CONSTANT )
    // clang-format on
    static QString qtVersionString();

    // clang-format off
    Q_PROPERTY(QString openSslVersionString
               READ openSslVersionString
               CONSTANT )
    // clang-format on
    static QString openSslVersionString();

    enum Architecture { x86, x64 };
    Q_ENUM(Architecture)

    // clang-format off
    Q_PROPERTY(Architecture architecture
               READ architecture
               CONSTANT )
    // clang-format on
    static Architecture architecture();

    // clang-format off
    Q_PROPERTY(QString architectureString
               READ architectureString
               CONSTANT )
    // clang-format on
    static QString architectureString();

    // clang-format off
    Q_PROPERTY(QString hostName
               READ hostName
               CONSTANT )
    // clang-format on
    static QString hostName();

    // clang-format off
    Q_PROPERTY(QString settingsPath
               READ settingsPath
               CONSTANT )
    // clang-format on
    static QString settingsPath(); // Folder in which settings.ini is stored

    // clang-format off
    Q_PROPERTY(QString settingsFile
               READ settingsFile
               CONSTANT )
    // clang-format on
    static QString settingsFile(); // Complete path to settings.ini file

    Q_INVOKABLE static QString configPath(const QString &relativeName);

    Q_INVOKABLE static QString modifierDescription(int modifier);
};

class Gui : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Gui)
    QML_SINGLETON

public:
    // clang-format off
    Q_PROPERTY(QImage emptyQImage
               READ emptyQImage
               CONSTANT )
    // clang-format on
    static QImage emptyQImage();

    Q_INVOKABLE static QString shortcut(int k1, int k2 = 0, int k3 = 0, int k4 = 0);
    Q_INVOKABLE static QKeySequence keySequence(int k1, int k2 = 0, int k3 = 0, int k4 = 0);
    Q_INVOKABLE static Utils::KeyCombinations keyCombinations(const QString &shortcut);

    // StandardKey is one of QKeySequence::StandardKey in C++, or StandardKey.xyz in QML
    Q_INVOKABLE static QString standardShortcut(int standardKey);
    Q_INVOKABLE static QKeySequence standardKeySequence(int standardKey);

    Q_INVOKABLE static QString nativeShortcut(const QString &shortcut);
    Q_INVOKABLE static QString portableShortcut(const QVariant &shortcut);
    Q_INVOKABLE static bool acceptsTextInput(QQuickItem *item);

    Q_INVOKABLE static void log(const QString &message);
};

class Url : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Url)
    QML_SINGLETON

public:
    // URL utils
    Q_INVOKABLE static QUrl toHttp(const QUrl &url);
    Q_INVOKABLE static QUrl fromPath(const QString &fileName);
    Q_INVOKABLE static QString toPath(const QUrl &url);
};

class Object : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Object)
    QML_SINGLETON

public:
    Q_INVOKABLE static bool isOfType(const QVariant &value, const QString &typeName);
    Q_INVOKABLE static QString typeOf(const QVariant &value);

    Q_INVOKABLE static bool changeProperty(QObject *object, const QString &name,
                                           const QVariant &value);
    Q_INVOKABLE static bool resetProperty(QObject *object, const QString &propName);
    Q_INVOKABLE static QVariant queryProperty(QObject *object, const QString &name);

    Q_INVOKABLE static bool save(QObject *object);
    Q_INVOKABLE static bool load(QObject *object);
    Q_INVOKABLE static Utils::ObjectConfig configuration(const QObject *object,
                                                         const QMetaObject *metaObject = nullptr);

    Q_INVOKABLE static int treeSize(QObject *object);

    Q_INVOKABLE static QObject *firstChildByType(QObject *in, const QString &typeName);
    Q_INVOKABLE static QObject *firstChildByName(QObject *in, const QString &objectName);

    Q_INVOKABLE static QObject *firstParentByType(QObject *of, const QString &typeName);
    Q_INVOKABLE static QObject *firstParentByName(QObject *of, const QString &objectName);

    Q_INVOKABLE static QObject *firstSiblingByType(QObject *of, const QString &typeName);
    Q_INVOKABLE static QObject *firstSiblingByName(QObject *of, const QString &objectName);

    Q_INVOKABLE static QObject *parentOf(QObject *object);
    Q_INVOKABLE static bool reparent(QObject *object, QObject *newParent);

    Q_INVOKABLE static QString enumKey(QObject *object, const QString &enumName, int value);
    Q_INVOKABLE static QString typeEnumKey(const QString &typeName, const QString &enumName,
                                           int value);
    Q_INVOKABLE static QAbstractListModel *enumModel(QObject *object, const QString &enumName);
    Q_INVOKABLE static QAbstractListModel *
    typeEnumModel(const QString &typeName, const QString &enumName, QObject *parent = nullptr);
};

class ObjectRegistry : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ObjectRegistry)
    QML_SINGLETON

public:
    // clang-format off
    Q_PROPERTY(QAbstractListModel *model
               READ model
               CONSTANT )
    // clang-format on
    static QAbstractListModel *model();

    static void remove(QObject *object);
    static QString add(QObject *object, const QString &name);

    Q_INVOKABLE static QObject *find(const QString &name);

    template<class T>
    static T *lookup(const QString &name)
    {
        QObject *object = ObjectRegistry::find(name);
        if (object)
            return qobject_cast<T *>(object);
        return nullptr;
    }
};

class ObjectRegister : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ObjectRegister)
    QML_ATTACHED(ObjectRegister)

public:
    virtual ~ObjectRegister();

    static ObjectRegister *qmlAttachedProperties(QObject *parent);

    // clang-format off
    Q_PROPERTY(QString name
               READ name
               WRITE setName
               NOTIFY nameChanged)
    // clang-format on
    void setName(const QString &val);
    QString name() const { return m_name; }
    Q_SIGNAL void nameChanged();

private:
    explicit ObjectRegister(QObject *parent = nullptr);

private:
    friend class ObjectRegistry;
    QString m_name;
};

class Color : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Color)
    QML_SINGLETON

public:
    Q_INVOKABLE static QString name(const QColor &color) { return color.name(); }
    Q_INVOKABLE static QColor pick(const QColor &initial);
    Q_INVOKABLE static QColor whitewash(const QColor &c, qreal factor);
    Q_INVOKABLE static QColor mix(const QColor &a, const QColor &b);
    Q_INVOKABLE static QColor tint(const QColor &input, const QVariant &alpha);
    Q_INVOKABLE static QColor stacked(const QColor &foreground, const QColor &background);
    Q_INVOKABLE static QColor translucent(const QColor &input, qreal alpha = 0.5);
    Q_INVOKABLE static bool isLight(const QColor &color);
    Q_INVOKABLE static bool isVeryLight(const QColor &color);
    Q_INVOKABLE static QColor textColorFor(const QColor &backgroundColor);
};

class SceneColors : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(SceneColors)
    QML_SINGLETON

public:
    Q_INVOKABLE static QList<QColor>
    paletteForVersion(const QVersionNumber &version = QVersionNumber());

    // clang-format off
    Q_PROPERTY(QVariantList palette
               READ paletteAsVariantList
               CONSTANT )
    // clang-format on
    static QList<QColor> palette();

    Q_INVOKABLE static QColor pick(int index);

private:
    static QVariantList paletteAsVariantList();
};

class File : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(File)
    QML_SINGLETON

public:
    Q_INVOKABLE static bool write(const QString &fileName, const QString &content);

    Q_INVOKABLE static void revealOnDesktop(const QString &path);

    Q_INVOKABLE static QString copyToFolder(const QString &fromFilePath, const QString &toFolder);
    Q_INVOKABLE static QString completeBaseName(const QString &path);
    Q_INVOKABLE static QString neighbouringFilePath(const QString &filePath,
                                                    const QString &nfileName);
    Q_INVOKABLE static QString path(const QString &name);
    Q_INVOKABLE static QString read(const QString &fileName);

    Q_INVOKABLE static Utils::FileInfo info(const QString &path);

    static QString sanitiseName(const QString &fileName, QSet<QChar> *removedChars = nullptr);
};

class MouseCursor : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(MouseCursor)
    QML_SINGLETON

public:
    Q_INVOKABLE static void setShape(Qt::CursorShape shape);
    Q_INVOKABLE static void unsetShape();

    Q_INVOKABLE static bool moveTo(const QPointF &globalPos);

    Q_INVOKABLE static bool isOverItem(QQuickItem *item);

    Q_INVOKABLE static QPointF position();
    Q_INVOKABLE static QPointF itemPosition(QQuickItem *item);
};

class GMath : public QObject // short for Geometry Math
{
    Q_OBJECT
    QML_NAMED_ELEMENT(GMath)
    QML_SINGLETON

public:
    Q_INVOKABLE static bool doRectanglesIntersect(const QRectF &r1, const QRectF &r2);
    Q_INVOKABLE static bool isRectangleInRectangle(const QRectF &bigRect, const QRectF &smallRect);

    Q_INVOKABLE static qreal distanceBetweenPoints(const QPointF &p1, const QPointF &p2);

    Q_INVOKABLE static QRectF adjustRectangle(const QRectF &rect, qreal left, qreal top,
                                              qreal right, qreal bottom);
    Q_INVOKABLE static QRectF boundingRect(const QString &text, const QFont &font);
    Q_INVOKABLE static QRectF intersectedRectangle(const QRectF &of, const QRectF &with);
    Q_INVOKABLE static QRectF largestBoundingRect(const QStringList &strings, const QFont &font);
    Q_INVOKABLE static QRectF querySubRectangle(const QRectF &in, const QRectF &around,
                                                const QSizeF &atBest);
    Q_INVOKABLE static QRectF uniteRectangles(const QRectF &r1, const QRectF &r2);

    Q_INVOKABLE static QSizeF scaledSize(const QSizeF &of, const QSizeF &into);

    Q_INVOKABLE static QPointF centerOf(const QRectF &rect);
    Q_INVOKABLE static QPointF
    translationRequiredToBringRectangleInRectangle(const QRectF &bigRect, const QRectF &smallRect);
};

class TMath : public QObject // short for Time Math
{
    Q_OBJECT
    QML_NAMED_ELEMENT(TMath)
    QML_SINGLETON

public:
    Q_INVOKABLE static void sleep(int ms);
    Q_INVOKABLE static QTime secondsToTime(int nrSeconds);
    Q_INVOKABLE static QString relativeTime(const QDateTime &dt);
    Q_INVOKABLE static QString timeLengthString(const QTime &time);
    Q_INVOKABLE static QString timeToString(const QTime &time);
    Q_INVOKABLE static QString dateToString(const QDate &date);
    Q_INVOKABLE static QString dateTimeToString(const QDateTime &dateTime);
};

class Clipboard : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Clipboard)
    QML_SINGLETON

public:
    Q_INVOKABLE static void set(const QString &text);
    Q_INVOKABLE static QString get();
};

class SystemEnvironment : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(SystemEnvironment)
    QML_SINGLETON

public:
    Q_INVOKABLE static void set(const QString &key, const QVariant &value);
    Q_INVOKABLE static void remove(const QString &key);

    Q_INVOKABLE static QVariant get(const QString &key, const QVariant &fallback = QVariant());
};

class SMath : public QObject // Short for String Math
{
    Q_OBJECT
    QML_NAMED_ELEMENT(SMath)
    QML_SINGLETON

public:
    Q_INVOKABLE static QString titleCased(const QString &val);

    Q_INVOKABLE static QString createUniqueId();

    Q_INVOKABLE static bool doListsIntersect(const QStringList &a, const QStringList &b,
                                             Qt::CaseSensitivity cs = Qt::CaseInsensitive);

    Q_INVOKABLE static bool isValidUrl(const QString &url);
    Q_INVOKABLE static bool isValidEmail(const QString &email);

    Q_INVOKABLE static QString formatAsBulletPoints(const QVariantList &items);

    static QString painterPathToString(const QPainterPath &val);

    static QPainterPath stringToPainterPath(const QString &val);

    static QJsonObject replaceCharacterName(const QString &from, const QString &to,
                                            const QJsonObject &delta,
                                            int *nrReplacements = nullptr);
    static QString replaceCharacterName(const QString &from, const QString &to, const QString &in,
                                        int *nrReplacements = nullptr);
};

class SystemClipboard : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(SystemClipboard)

public:
    explicit SystemClipboard(QObject *parent = nullptr);
    virtual ~SystemClipboard();

    // clang-format off
    Q_PROPERTY(bool valid
               READ isValid
               CONSTANT)
    // clang-format on
    bool isValid() const { return m_clipboard != nullptr; }

    // clang-format off
    Q_PROPERTY(QString text
               READ text
               WRITE setText
               NOTIFY textChanged)
    // clang-format on
    void setText(const QString &val);
    QString text() const;
    Q_SIGNAL void textChanged();

private:
    QClipboard *m_clipboard = nullptr;
};

}

#endif // UTILS_H
