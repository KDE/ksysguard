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

#ifndef KSIGNALPLOTTER_H
#define KSIGNALPLOTTER_H

#include <QString>
#include <QWidget>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QLinkedList>
#include <QImage>

class QColor;
class QSvgRenderer;


class KSignalPlotter : public QWidget
{
  Q_OBJECT

  public:
    KSignalPlotter( QWidget *parent = 0);
    ~KSignalPlotter();

    bool addBeam( const QColor &color );
    void addSample( const QList<double> &samples );
    void reorderBeams( const QList<int>& newOrder );

    void removeBeam( uint pos );

    void changeRange( int beam, double min, double max );

    QList<QColor> &beamColors();

    void setTitle( const QString &title );
    QString title() const;

    void setTranslatedUnit( const QString &unit );
    QString translatedUnit() const;

    void setScaleDownBy( double value );
    double scaleDownBy() const;

    void setUseAutoRange( bool value );
    bool useAutoRange() const;

    void setMinValue( double min );
    double minValue() const;

    void setMaxValue( double max );
    double maxValue() const;

    void setHorizontalScale( uint scale );
    int horizontalScale() const;

    void setShowVerticalLines( bool value );
    bool showVerticalLines() const;

    void setVerticalLinesColor( const QColor &color );
    QColor verticalLinesColor() const;

    void setVerticalLinesDistance( int distance );
    int verticalLinesDistance() const;

    void setVerticalLinesScroll( bool value );
    bool verticalLinesScroll() const;

    void setShowHorizontalLines( bool value );
    bool showHorizontalLines() const;

    void setHorizontalLinesColor( const QColor &color );
    QColor horizontalLinesColor() const;

    void setFontColor( const QColor &color );
    QColor fontColor() const;

    void setFont( const QFont &font );
    QFont font() const;

    void setHorizontalLinesCount( int count );
    int horizontalLinesCount() const;

    void setShowLabels( bool value );
    bool showLabels() const;

    void setShowTopBar( bool value );
    bool showTopBar() const;

    void setBackgroundColor( const QColor &color );
    QColor backgroundColor() const;

    void setSvgBackground( const QString &filename );
    QString svgBackground() const;

    /** Return a translated string like:   "34 %" or "100 KB" for beam i
     */
    QString lastValue( int i) const;

  protected:
    void updateDataBuffers();

    virtual void resizeEvent( QResizeEvent* );
    virtual void paintEvent( QPaintEvent* );

  private:
    QSvgRenderer *mSvgRenderer;
    QString mSvgFilename;
    QImage mBackgroundImage;  //A cache of the svg

    double mMinValue;
    double mMaxValue;
    double mScaleDownBy;
    bool mUseAutoRange;

    uint mGraphStyle;

    bool mShowVerticalLines;
    QColor mVerticalLinesColor;
    uint mVerticalLinesDistance;
    bool mVerticalLinesScroll;
    uint mVerticalLinesOffset;
    uint mHorizontalScale;

    bool mShowHorizontalLines;
    QColor mHorizontalLinesColor;
    uint mHorizontalLinesCount;

    bool mShowLabels;
    bool mShowTopBar;
    uint mBezierCurveOffset;

    QColor mBackgroundColor;
    QColor mFontColor;

    QLinkedList < QList<double> > mBeamData; // Every item in the linked list contains a set of data points to plot.  The first item is the newest
    QList< QColor> mBeamColors;  //These colors match up against the QList<double> above

    int mSamples; //This is what mBeamData.size() should equal when full.  When we start off and have no data then mSamples will be higher.  If we resize the widget so it's smaller, then for a short while this will be smaller
    int mNewestIndex; //The index to the newest item added.  newestIndex+1   is the second newest, and so on

    QString mTitle;
    QString mUnit;

    QFont mFont;
};

#endif
