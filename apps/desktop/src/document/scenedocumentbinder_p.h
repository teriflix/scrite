/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef SCENEDOCUMENTBINDER_P_H
#define SCENEDOCUMENTBINDER_P_H

#include <QSet>
#include <QBrush>
#include <QPointer>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextBlockUserData>
#include <QMetaObject>

class SceneElement;
class SceneDocumentBinder;
class SpellCheckService;
class TextFragment;
class SceneElementFormat;

class SceneDocumentBlockUserData : public QTextBlockUserData
{
public:
    enum { Type = 1001 };
    const int type = Type;

    explicit SceneDocumentBlockUserData(const QTextBlock &block, SceneElement *element,
                                        SceneDocumentBinder *binder);
    ~SceneDocumentBlockUserData();

    QTextBlockFormat blockFormat;
    QTextCharFormat charFormat;

    bool isValid() const;

    SceneElement *sceneElement() const { return m_sceneElement; }

    void resetFormat();
    bool updateFromFormat(const SceneElementFormat *format);

    void initializeSpellCheck(SceneDocumentBinder *binder);
    bool shouldUpdateFromSpellCheck();
    void scheduleSpellCheckUpdate();
    QList<TextFragment> misspelledFragments() const;
    TextFragment findMisspelledFragment(int start, int end) const;

    void polishTextLater();
    void autoCapitalizeLater();

    static QBrush colorTransformBrush(const QBrush &brush);
    static SceneDocumentBlockUserData *get(const QTextBlock &block);
    static SceneDocumentBlockUserData *get(QTextBlockUserData *userData);

private:
    enum Tasks { PolishTextTask, AutoCapitalizeTask };
    void polishTextNow();
    void autoCapitalizeNow();
    void performPendingTasks();
    bool markCursorPosition();
    int markedCursorPosition(bool removeMarker = true);

private:
    friend class SceneDocumentBinder;
    QTextBlock m_textBlock;
    QSet<int> m_pendingTasks;
    QPointer<SpellCheckService> m_spellCheck;
    QPointer<SceneElement> m_sceneElement;
    QPointer<SceneDocumentBinder> m_binder;
    QString m_highlightedText;
    int m_formatMTime = -1;
    int m_spellCheckMTime = -1;
    QMetaObject::Connection m_spellCheckConnection;
};

#endif // SCENEDOCUMENTBINDER_P_H
