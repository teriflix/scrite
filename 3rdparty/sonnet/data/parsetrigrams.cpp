/**
 * parsetrigrams.cpp
 *
 * Parse a set of trigram files into a QMap, and serialize to stdout.
 * Note: we allow this data to be read into QHash. We use QMap here
 * to get deterministic output from run to run.
 *
 * SPDX-FileCopyrightText: 2006 Jacob Rideout <kde@jacobrideout.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QMap>
#include <QRegularExpression>
#include <QString>
#include <QTextStream>

int main(int argc, char **argv)
{
    if (argc < 2) {
        return 1;
    }

    QFile sout;
    sout.open(stdout, QIODevice::WriteOnly);
    QDataStream out(&sout);

    QString path = QLatin1String(argv[1]);
    QDir td(path);

    /*
     * We use QMap (instead of QHash) here to get deterministic output
     * from run to run.
     */
    QMap<QString, QMap<QString, int>> models;

    const QRegularExpression rx(QStringLiteral("(?:.{3})\\s+(.*)"));
    const QStringList files = td.entryList(QDir::Files);
    for (const QString &fname : files) {
        QFile fin(td.filePath(fname));
        fin.open(QFile::ReadOnly | QFile::Text);
        QTextStream stream(&fin);

        while (!stream.atEnd()) {
            QString line = stream.readLine();
            const QRegularExpressionMatch match = rx.match(line);
            if (match.hasMatch()) {
                models[fname][line.left(3)] = match.capturedView(1).toInt();
            }
        }
    }

    out << models;
}
