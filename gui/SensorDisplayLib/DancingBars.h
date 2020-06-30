/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation; either version 2 of
 the License or (at your option) version 3 or any later version
 accepted by the membership of KDE e.V. (or its successor approved
 by the membership of KDE e.V.), which shall act as a proxy 
 defined in Section 14 of version 3 of the license.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KSG_DANCINGBARS_H
#define KSG_DANCINGBARS_H

#include <SensorDisplay.h>

#include <QBitArray>
#include <QVector>



class BarGraph;

class DancingBars : public KSGRD::SensorDisplay
{
  Q_OBJECT

  public:
    enum { MaximumBars = 32 };

    DancingBars( QWidget *parent, const QString &title, SharedSettings *workSheetSettings );
    ~DancingBars() override;

    void configureSettings() override;

    // Adds a sensor, up to MaximumBars (after that it fails)
    bool addSensor( const QString &hostName, const QString &name,
                    const QString &type, const QString &title ) override;
    bool removeSensor( uint pos ) override;

    void updateSamples( const QVector<double> &samples );

    void answerReceived( int id, const QList<QByteArray> &answerlist ) override;

    bool restoreSettings( QDomElement& ) override;
    bool saveSettings( QDomDocument&, QDomElement& ) override;

    bool hasSettingsDialog() const override;

  public Q_SLOTS:
    void applyStyle() override;

  private:
    /// Updates the tooltips after sensors have been added or removed.
    void updateToolTip();

    unsigned int mBars;

    BarGraph* mPlotter;

    /**
      The sample buffer and the flags are needed to store the incoming
      samples for each beam until all samples of the period have been
      received. The flags variable is used to ensure that all samples have
      been received.
     */
    QVector<double> mSampleBuffer;
    QBitArray mFlags;
};

#endif

