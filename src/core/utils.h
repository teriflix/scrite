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
#include <QFileInfo>
#include <QQuickItem>
#include <QVersionNumber>
#include <QAbstractItemModel>
#include <QDateTime>

namespace Utils {
struct FileInfo
{
    Q_GADGET

public:
    QFileInfo info;

    Q_PROPERTY(bool valid READ valid)
    bool valid() const { return info != QFileInfo(); }

    Q_PROPERTY(bool exists READ exists)
    bool exists() const { return info.exists(); }

    Q_PROPERTY(bool readable READ readable)
    bool readable() const { return info.isReadable(); }

    Q_PROPERTY(bool writable READ writable )
    bool writable() const { return info.isWritable(); }

    Q_PROPERTY(QString baseName READ baseName)
    QString baseName() const { return info.baseName(); }

    Q_PROPERTY(QString absoluteFilePath READ absoluteFilePath)
    QString absoluteFilePath() const { return info.absoluteFilePath(); }

    Q_PROPERTY(QString absolutePath READ absolutePath)
    QString absolutePath() const { return info.absolutePath(); }

    Q_PROPERTY(QString suffix READ suffix)
    QString suffix() const { return info.suffix(); }

    Q_PROPERTY(QString fileName READ fileName)
    QString fileName() const { return info.fileName(); }

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

public:
    Q_PROPERTY(QString key MEMBER key)
    QString key;

    Q_PROPERTY(QString value MEMBER value)
    QString value;

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

public:
    Q_PROPERTY(QString name MEMBER name)
    QString name;

    Q_PROPERTY(QString label MEMBER label)
    QString label;

    Q_PROPERTY(QString note MEMBER note)
    QString note;

    Q_PROPERTY(QString editor MEMBER editor)
    QString editor;

    Q_PROPERTY(QVariant min MEMBER min)
    QVariant min;

    Q_PROPERTY(QVariant max MEMBER max)
    QVariant max;

    Q_PROPERTY(QVariant ideal MEMBER ideal)
    QVariant ideal;

    Q_PROPERTY(QString group MEMBER group)
    QString group;

    Q_PROPERTY(QString feature MEMBER feature)
    QString feature;

    Q_PROPERTY(QList<ObjectConfigFieldChoice> choices MEMBER choices)
    QList<ObjectConfigFieldChoice> choices;

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

public:
    Q_PROPERTY(QString name MEMBER name)
    QString name;

    Q_PROPERTY(QString description MEMBER description)
    QString description;

    Q_PROPERTY(QList<ObjectConfigField> fields MEMBER fields)
    QList<ObjectConfigField> fields;

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

public:
    Q_PROPERTY(bool valid READ isValid)
    bool isValid() const { return fields.size() > 0 && groups.size() >= 1; }

    Q_PROPERTY(QString title MEMBER title)
    QString title;

    Q_PROPERTY(QString description MEMBER description)
    QString description;

    Q_PROPERTY(QList<ObjectConfigField> fields MEMBER fields)
    QList<ObjectConfigField> fields;

    Q_PROPERTY(QList<ObjectConfigFieldGroup> groups MEMBER groups)
    QList<ObjectConfigFieldGroup> groups;

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

Q_DECLARE_METATYPE(Utils::FileInfo)
Q_DECLARE_METATYPE(Utils::ObjectConfigFieldChoice)
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

    Q_PROPERTY(Type type READ type CONSTANT)
    static Type type();

    Q_PROPERTY(QString typeString READ typeString CONSTANT)
    static QString typeString();

    Q_PROPERTY(bool isLinuxDesktop READ isLinuxDesktop CONSTANT)
    static bool isLinuxDesktop() { return type() == LinuxDesktop; }

    Q_PROPERTY(bool isWindowsDesktop READ isWindowsDesktop CONSTANT)
    static bool isWindowsDesktop() { return type() == WindowsDesktop; }

    Q_PROPERTY(bool isMacOSDesktop READ isMacOSDesktop CONSTANT)
    static bool isMacOSDesktop() { return type() == MacOSDesktop; }

    Q_PROPERTY(int osMajorVersion READ osMajorVersion CONSTANT)
    static int osMajorVersion();

    Q_PROPERTY(QList<int> osVersion READ osVersion CONSTANT)
    static QList<int> osVersion();

    Q_PROPERTY(QString osVersionString READ osVersionString CONSTANT)
    static QString osVersionString();

    Q_PROPERTY(QString qtVersionString READ qtVersionString CONSTANT)
    static QString qtVersionString();

    Q_PROPERTY(QString openSslVersionString READ openSslVersionString CONSTANT)
    static QString openSslVersionString();

    enum Architecture { x86, x64 };
    Q_ENUM(Architecture)

    Q_PROPERTY(Architecture architecture READ architecture CONSTANT)
    static Architecture architecture();

    Q_PROPERTY(QString architectureString READ architectureString CONSTANT)
    static QString architectureString();

    Q_PROPERTY(QString hostName READ hostName CONSTANT)
    static QString hostName();
};

class Gui : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Gui)
    QML_SINGLETON

public:
    Q_PROPERTY(QImage emptyQImage READ emptyQImage CONSTANT)
    static QImage emptyQImage();

    Q_INVOKABLE static QString nativeShortcut(const QString &shortcut);
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
    Q_INVOKABLE static ObjectConfig configuration(const QObject *object,
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
    Q_PROPERTY(QAbstractListModel* model READ model CONSTANT)
    static QAbstractListModel *model();

    Q_INVOKABLE static void remove(QObject *object);
    Q_INVOKABLE static QString add(QObject *object, const QString &name);
    Q_INVOKABLE static QObject *find(const QString &name);
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

    Q_PROPERTY(QVariantList palette READ paletteAsVariantList CONSTANT)
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

    Q_INVOKABLE static FileInfo info(const QString &path);

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

    static QString painterPathToString(const QPainterPath &val);

    static QPainterPath stringToPainterPath(const QString &val);

    static QJsonObject replaceCharacterName(const QString &from, const QString &to,
                                            const QJsonObject &delta,
                                            int *nrReplacements = nullptr);
    static QString replaceCharacterName(const QString &from, const QString &to, const QString &in,
                                        int *nrReplacements = nullptr);
};

}

#endif // UTILS_H
