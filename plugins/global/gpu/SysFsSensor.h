/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include <QObject>

#include "SensorProperty.h"

class SysFsSensor : public SensorProperty
{
    Q_OBJECT

public:
    SysFsSensor(const QString &id, const QString &path, SensorObject *parent);

    void setConvertFunction(const std::function<QVariant(const QByteArray&)> &function);
    void update();

private:
    QString m_path;
    std::function<QVariant(const QByteArray&)> m_convertFunction;
};
