/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "NetworkDevice.h"

#include <KLocalizedString>

NetworkDevice::NetworkDevice(const QString &id, const QString &name)
    : SensorObject(id, name, nullptr)
{
    m_networkSensor = new SensorProperty(QStringLiteral("network"), i18nc("@title", "Network Name"), this);
    m_networkSensor->setShortName(i18nc("@title Short of Network Name", "Name"));

    m_signalSensor = new SensorProperty(QStringLiteral("signal"), i18nc("@title", "Signal Strength"), this);
    m_signalSensor->setShortName(i18nc("@title Short of Signal Strength", "Signal"));
    m_signalSensor->setUnit(KSysGuard::utils::UnitPercent);
    m_signalSensor->setMin(0);
    m_signalSensor->setMax(100);

    m_ipv4Sensor = new SensorProperty(QStringLiteral("ipv4address"), i18nc("@title", "IPv4 Address"), this);
    m_ipv4Sensor->setShortName(i18nc("@title Short of IPv4 Address", "IPv4"));

    m_ipv6Sensor = new SensorProperty(QStringLiteral("ipv6address"), i18nc("@title", "IPv6 Address"), this);
    m_ipv6Sensor->setShortName(i18nc("@title Short of IPv6 Address", "IPv6"));

    m_downloadSensor = new SensorProperty(QStringLiteral("download"), i18nc("@title", "Download Rate"), this);
    m_downloadSensor->setShortName(i18nc("@title Short for Download Rate", "Download"));
    m_downloadSensor->setUnit(KSysGuard::utils::UnitByteRate);

    m_uploadSensor = new SensorProperty(QStringLiteral("upload"), i18nc("@title", "Upload Rate"), this);
    m_uploadSensor->setShortName(i18nc("@title Short for Upload Rate", "Upload"));
    m_uploadSensor->setUnit(KSysGuard::utils::UnitByteRate);

    m_totalDownloadSensor = new SensorProperty(QStringLiteral("totalDownload"), i18nc("@title", "Total Downloaded"), this);
    m_totalDownloadSensor->setShortName(i18nc("@title Short for Total Downloaded", "Downloaded"));
    m_totalDownloadSensor->setUnit(KSysGuard::utils::UnitByte);

    m_totalUploadSensor = new SensorProperty(QStringLiteral("totalUpload"), i18nc("@title", "Total Uploaded"), this);
    m_totalUploadSensor->setShortName(i18nc("@title Short for Total Uploaded", "Uploaded"));
    m_totalUploadSensor->setUnit(KSysGuard::utils::UnitByte);
}
