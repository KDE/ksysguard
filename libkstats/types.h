/*
    Copyright (c) 2019 David Edmundson <davidedmundson@kde.org>

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
#pragma once

#include <QDateTime>
#include <QDebug>
#include <QObject>
#include <QVariant>

#include <QDBusArgument>

namespace KSysGuard
{
namespace utils
{

/**
 * This enum type is used to specify metric prefixes.
 */
enum MetricPrefix {
    MetricPrefixAutoAdjust = -1,
    MetricPrefixUnity = 0,
    MetricPrefixKilo,
    MetricPrefixMega,
    MetricPrefixGiga,
    MetricPrefixTera,
    MetricPrefixPeta,
    MetricPrefixLast = MetricPrefixPeta
};

/**
 * This enum types is used to specify units.
 */
enum Unit {
    UnitInvalid = -1,
    UnitNone = 0,

    // Byte size units.
    UnitByte = 100,
    UnitKiloByte = MetricPrefixKilo + UnitByte,
    UnitMegaByte = MetricPrefixMega + UnitByte,
    UnitGigaByte = MetricPrefixGiga + UnitByte,
    UnitTeraByte = MetricPrefixTera + UnitByte,
    UnitPetaByte = MetricPrefixPeta + UnitByte,

    // Data rate units.
    UnitByteRate = 200,
    UnitKiloByteRate = MetricPrefixKilo + UnitByteRate,
    UnitMegaByteRate = MetricPrefixMega + UnitByteRate,
    UnitGigaByteRate = MetricPrefixGiga + UnitByteRate,
    UnitTeraByteRate = MetricPrefixTera + UnitByteRate,
    UnitPetaByteRate = MetricPrefixPeta + UnitByteRate,

    // Frequency.
    UnitHertz = 300,
    UnitKiloHertz = MetricPrefixKilo + UnitHertz,
    UnitMegaHertz = MetricPrefixMega + UnitHertz,
    UnitGigaHertz = MetricPrefixGiga + UnitHertz,
    UnitTeraHertz = MetricPrefixTera + UnitHertz,
    UnitPetaHertz = MetricPrefixPeta + UnitHertz,

    // Time units.
    UnitBootTimestamp = 400,
    UnitSecond,
    UnitTime,

    // Misc units.
    UnitCelsius = 500,
    UnitDecibelMilliWatts,
    UnitPercent,
    UnitRate,
    UnitRpm,
    UnitVolt,
    UnitWatt,
};
}
}

//Data that is static for the lifespan of the sensor
class SensorInfo
{
public:
    SensorInfo() = default;
    QString name; //translated?
    QString shortName;
    QString description; // translated
    QVariant::Type variantType = QVariant::Invalid;
    KSysGuard::utils::Unit unit = KSysGuard::utils::UnitInvalid; //Both a format hint and implies data type (i.e double/string)
    qreal min = 0;
    qreal max = 0;
};
Q_DECLARE_METATYPE(SensorInfo);
// this stuff could come from .desktop files (for the DBus case) or hardcoded (eg. for example nvidia-smi case) or come from current "ksysgrd monitors"

class Q_DECL_EXPORT SensorData
{
public:
    SensorData() = default;
    SensorData(const QString &_sensorProperty, const QVariant &_payload)
        : sensorProperty(_sensorProperty)
        , payload(_payload)
    {
    }
    QString sensorProperty;
    QVariant payload;
};
Q_DECLARE_METATYPE(SensorData);

typedef QHash<QString, SensorInfo> SensorInfoMap;
typedef QVector<SensorData> SensorDataList;

Q_DECLARE_METATYPE(SensorDataList);

inline QDBusArgument &operator<<(QDBusArgument &argument, const SensorInfo &s)
{
    argument.beginStructure();
    argument << s.name;
    argument << s.shortName;
    argument << s.description;
    argument << s.variantType;
    argument << s.unit;
    argument << s.min;
    argument << s.max;
    argument.endStructure();
    return argument;
}

inline const QDBusArgument &operator>>(const QDBusArgument &argument, SensorInfo &s)
{
    argument.beginStructure();
    argument >> s.name;
    argument >> s.shortName;
    argument >> s.description;
    uint32_t t;
    argument >> t;
    s.variantType = static_cast<QVariant::Type>(t);
    argument >> t;
    s.unit = static_cast<KSysGuard::utils::Unit>(t);
    argument >> s.min;
    argument >> s.max;
    argument.endStructure();
    return argument;
}

inline QDBusArgument &operator<<(QDBusArgument &argument, const SensorData &s)
{
    argument.beginStructure();
    argument << s.sensorProperty;
    argument << QDBusVariant(s.payload);
    argument.endStructure();
    return argument;
}

inline const QDBusArgument &operator>>(const QDBusArgument &argument, SensorData &s)
{
    argument.beginStructure();
    argument >> s.sensorProperty;
    argument >> s.payload;
    argument.endStructure();
    return argument;
}
