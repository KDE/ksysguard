/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "SysFsSensor.h"

#include <QFile>

SysFsSensor::SysFsSensor(const QString& id, const QString& path, SensorObject* parent)
    : SensorProperty(id, parent)
{
    m_path = path;

    m_convertFunction = [](const QByteArray &input) {
        return std::atoll(input);
    };
}

void SysFsSensor::setConvertFunction(const std::function<QVariant (const QByteArray &)>& function)
{
    m_convertFunction = function;
}

void SysFsSensor::update()
{
    if (!isSubscribed()) {
        return;
    }

    QFile file(m_path);
    if (!file.exists()) {
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    auto value = file.readAll();
    setValue(m_convertFunction(value));
}
