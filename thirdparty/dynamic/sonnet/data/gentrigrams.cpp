/**
 * parsetrigrams.cpp
 *
 * Parse a corpus of data and generate trigrams
 *
 * SPDX-FileCopyrightText: 2013 Martin Sandsmark <martin.sandsmark@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "guesslanguage.h"

#include <QDebug>
#include <QFile>
#include <QHash>
#include <QString>

int main(int argc, char *argv[])
{
    if (argc < 3) {
        qWarning() << argv[0] << "corpus.txt outfile.trigram";
        return -1;
    }

    QFile file(QString::fromLocal8Bit(argv[1]));
    if (!file.open(QIODevice::ReadOnly | QFile::Text)) {
        qWarning() << "Unable to open corpus:" << argv[1];
        return -1;
    }
    QTextStream stream(&file);

    QFile outFile(QString::fromLocal8Bit(argv[2]));
    if (!outFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Unable to open output file" << argv[2];
        return -1;
    }

    QHash<QString, int> model;
    qDebug() << "Reading in" << file.size() << "bytes";
    QString trigram = stream.read(3);
    QString contents = stream.readAll();
    qDebug() << "finished reading!";
    qDebug() << "Building model...";
    for (int i = 0; i < contents.size(); i++) {
        if (!contents[i].isPrint()) {
            continue;
        }
        model[trigram]++;
        trigram[0] = trigram[1];
        trigram[1] = trigram[2];
        trigram[2] = contents[i];
    }
    qDebug() << "model built!";

    qDebug() << "Sorting...";
    QMultiMap<int, QString> orderedTrigrams;

    for (auto it = model.cbegin(); it != model.cend(); ++it) {
        const QString data = it.key();
        Q_ASSERT(data.size() >= 3);
        bool hasTwoSpaces = ((data.size() > 1 && data[0].isSpace() && data[1].isSpace()) //
                             || (data.size() > 2 && data[1].isSpace() && data[2].isSpace()));

        if (!hasTwoSpaces) {
            orderedTrigrams.insert(it.value(), data);
        }
    }

    qDebug() << "Sorted!";

    qDebug() << "Weeding out...";

    auto i = orderedTrigrams.begin();
    while (orderedTrigrams.size() > Sonnet::MAXGRAMS) {
        i = orderedTrigrams.erase(i);
    }
    qDebug() << "Weeded!";

    qDebug() << "Storing...";
    i = orderedTrigrams.end();
    int count = 0;
    QTextStream outStream(&outFile);

    while (i != orderedTrigrams.begin()) {
        --i;
        outStream << *i << "\t\t\t" << count++ << '\n';
    }
}
