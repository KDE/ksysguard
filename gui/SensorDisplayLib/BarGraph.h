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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
    Please do not commit any changes without consulting me first. Thanks!

*/

#ifndef KSG_BARGRAPH_H
#define KSG_BARGRAPH_H

#include <qmemarray.h>
#include <qptrvector.h>
#include <qwidget.h>

class BarGraph : public QWidget
{
  Q_OBJECT

  friend class DancingBars;

  public:
    BarGraph( QWidget *parent, const char *name = 0 );
    ~BarGraph();

    bool addBar( const QString &footer );
    bool removeBar( uint idx );

    void updateSamples( const QMemArray<double> &newSamples );

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
    QMemArray<double> samples;
    QStringList footers;
    uint bars;
    QColor normalColor;
    QColor alarmColor;
    QColor backgroundColor;
    int fontSize;
};

#endif
