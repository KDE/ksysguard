/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>

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
    ~BarGraph();

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
    virtual void paintEvent( QPaintEvent* );

  private:
    double minValue;
    double maxValue;
    double lowerLimit;
    double lowerLimitActive;
    double upperLimit;
    bool upperLimitActive;
    bool autoRange;
    QVector<double> samples;
    QStringList footers;
    uint bars;
    QColor normalColor;
    QColor alarmColor;
    QColor mBackgroundColor;
    int fontSize;
};

#endif
