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

#ifndef KSG_BARGRAPH_H
#define KSG_BARGRAPH_H

#include <QWidget>
#include <QPaintEvent>

class BarGraph : public QWidget
{
  Q_OBJECT

  friend class DancingBars;

  public:
    explicit BarGraph( QWidget *parent );
    ~BarGraph() override;

    bool addBar( const QString &footer );
    bool removeBar( uint idx );

    void updateSamples( const QVector<double> &newSamples );

    double getMin() const
    {
      return minValue;
    }

    double getMax() const
    {
      return maxValue;
    }

    void getLimits( double &l, bool &la, double &u, bool &ua ) const
    {
      l = lowerLimit;
      la = lowerLimitActive;
      u = upperLimit;
      ua = upperLimitActive;
    }

    void setLimits( double l, bool la, double u, bool ua )
    {
      lowerLimit = l;
      lowerLimitActive = la;
      upperLimit = u;
      upperLimitActive = ua;
    }

    void changeRange( double min, double max );

  protected:
    void paintEvent( QPaintEvent* ) override;

  private:
    double minValue;
    double maxValue;
    double lowerLimit;
    bool lowerLimitActive;
    double upperLimit;
    bool upperLimitActive;
    QVector<double> samples;
    QStringList footers;
    uint bars;
    QColor normalColor;
    QColor alarmColor;
    QColor mBackgroundColor;
    int fontSize;
};

#endif
