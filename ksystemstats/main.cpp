/*
    Copyright (c) 2019 David Edmundson <davidedmundson@kde.org>
    Copyright (c) 2020 David Redondo <kde@david-redondo.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>

#include "ksysguarddaemon.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    app.setQuitLockEnabled(false) ;
    app.setOrganizationDomain(QStringLiteral("kde.org"));

    QCommandLineParser parser;
    parser.addOption(QCommandLineOption(QStringLiteral("replace"), QStringLiteral("Replace the running instance")));
    parser.process(app);

    KSysGuardDaemon d;
    d.init(parser.isSet(QStringLiteral("replace")) ? KSysGuardDaemon::ReplaceIfRunning::Replace : KSysGuardDaemon::ReplaceIfRunning::DoNotReplace);
    app.exec();
}
