/*
    KSysGuard, the KDE System Guard

    Copyright (c) 2003 Stephan Uhlmann <su@su2.info>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef KSG_ACPI_H
#define KSG_ACPI_H

void initAcpi( struct SensorModul* );
void exitAcpi( void );

void initAcpiBattery( struct SensorModul* );
void printSysBatteryCharge(const char *cmd);
void printSysBatteryChargeInfo(const char *cmd);
void printSysBatteryChargeDesign(const char *cmd);
void printSysBatteryChargeDesignInfo(const char *cmd);
void printSysBatteryRate(const char *cmd);
void printSysBatteryRateInfo(const char *cmd);

void readTypeFile(const char *fileFormat, int number, char *buffer, int bufferSize);
static int getSysFileValue(const char *class, const char *group, int value, const char *file);
void initAcpiThermal( struct SensorModul * );
void printThermalZoneTemperature(const char *cmd);
void printSysThermalZoneTemperature(const char *cmd);
void printSysCompatibilityThermalZoneTemperature(const char *cmd);
void printSysThermalZoneTemperatureInfo(const char *cmd);
void printThermalZoneTemperatureInfo(const char *cmd);

void printCoolingDeviceStateInfo(const char *cmd);
void printCoolingDeviceState(const char *cmd);
void initAcpiFan( struct SensorModul * );
void printFanState(const char *cmd);
void printSysFanState(const char *cmd);
void printFanStateInfo(const char *cmd);

#endif
