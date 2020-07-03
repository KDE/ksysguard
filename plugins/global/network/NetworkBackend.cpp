/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "NetworkBackend.h"

FileBackend::FileBackend(QObject* parent)
{
}

bool FileBackend::isSupported()
{
    return false;
}

void FileBackend::start()
{

}

void FileBackend::stop()
{

}
