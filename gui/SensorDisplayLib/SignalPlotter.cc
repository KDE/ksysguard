/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 - 2002 Chris Schlaeger <cs@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.

    KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
    Please do not commit any changes without consulting me first. Thanks!

*/

#include <math.h>
#include <string.h>

#include <qpainter.h>
#include <qpixmap.h>
#include <Q3PointArray>

#include <kdebug.h>
#include <ksgrd/StyleEngine.h>

#include "SignalPlotter.h"

static inline int min( int a, int b )
{
  return ( a < b ? a : b );
}

SignalPlotter::SignalPlotter( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  // Auto deletion does not work for pointer to arrays.
  mBeamData.setAutoDelete( false );

  setBackgroundMode( Qt::NoBackground );

  mSamples = 0;
  mMinValue = mMaxValue = 0.0;
  mUseAutoRange = true;

  mGraphStyle = GRAPH_POLYGON;

  // Anything smaller than this does not make sense.
  setMinimumSize( 16, 16 );
  setSizePolicy( QSizePolicy( QSizePolicy::Expanding,
                 QSizePolicy::Expanding, false ) );

  mShowVerticalLines = true;
  mVerticalLinesColor = KSGRD::Style->firstForegroundColor();
  mVerticalLinesDistance = 30;
  mVerticalLinesScroll = true;
  mVerticalLinesOffset = 0;
  mHorizontalScale = 1;

  mShowHorizontalLines = true;
  mHorizontalLinesColor = KSGRD::Style->secondForegroundColor();
  mHorizontalLinesCount = 5;

  mShowLabels = true;
  mShowTopBar = false;
  mFontSize = KSGRD::Style->fontSize();

  mBackgroundColor = KSGRD::Style->backgroundColor();
}

SignalPlotter::~SignalPlotter()
{
  for ( double* p = mBeamData.first(); p; p = mBeamData.next() )
    delete [] p;
}

bool SignalPlotter::addBeam( const QColor &color )
{
  double* d = new double[ mSamples ];
  memset( d, 0, sizeof(double) * mSamples );
  mBeamData.append( d );
  mBeamColor.append( color );

  return true;
}

void SignalPlotter::addSample( const QList<double>& sampleBuf )
{
  if ( mBeamData.count() != sampleBuf.count() )
    return;

  double* d;
  if ( mUseAutoRange ) {
    double sum = 0;
    for ( d = mBeamData.first(); d; d = mBeamData.next() ) {
      sum += d[ 0 ];
      if ( sum < mMinValue )
        mMinValue = sum;
      if ( sum > mMaxValue )
        mMaxValue = sum;
    }
  }

  /* If the vertical lines are scrolling, increment the offset
   * so they move with the data. The vOffset / hScale confusion
   * is because v refers to Vertical Lines, and h to the horizontal
   * distance between the vertical lines. */
  if ( mVerticalLinesScroll ) {
    mVerticalLinesOffset = ( mVerticalLinesOffset + mHorizontalScale)
                           % mVerticalLinesDistance;
  }

  // Shift data buffers one sample down and insert new samples.
  QList<double>::ConstIterator s;
  for ( d = mBeamData.first(), s = sampleBuf.begin(); d; d = mBeamData.next(), ++s ) {
    memmove( d, d + 1, ( mSamples - 1 ) * sizeof( double ) );
    d[ mSamples - 1 ] = *s;
  }

  update();
}

void SignalPlotter::changeRange( int beam, double min, double max )
{
  // Only the first beam affects range calculation.
  if ( beam > 1 )
    return;

  mMinValue = min;
  mMaxValue = max;
}

QList<QColor> &SignalPlotter::beamColors()
{
  return mBeamColor;
}

void SignalPlotter::removeBeam( uint pos )
{
  mBeamColor.remove( mBeamColor.at( pos ) );
  mBeamData.remove( pos );
}

void SignalPlotter::setTitle( const QString &title )
{
  mTitle = title;
}

QString SignalPlotter::title() const
{
  return mTitle;
}

void SignalPlotter::setUseAutoRange( bool value )
{
  mUseAutoRange = value;
}

bool SignalPlotter::useAutoRange() const
{
  return mUseAutoRange;
}

void SignalPlotter::setMinValue( double min )
{
  mMinValue = min;
}

double SignalPlotter::minValue() const
{
  return ( mUseAutoRange ? 0 : mMinValue );
}

void SignalPlotter::setMaxValue( double max )
{
  mMaxValue = max;
}

double SignalPlotter::maxValue() const
{
  return ( mUseAutoRange ? 0 : mMaxValue );
}

void SignalPlotter::setGraphStyle( uint style )
{
  mGraphStyle = style;
}

uint SignalPlotter::graphStyle() const
{
  return mGraphStyle;
}

void SignalPlotter::setHorizontalScale( uint scale )
{
  if (scale == mHorizontalScale)
     return;

  mHorizontalScale = scale;
  if (isVisible())
     updateDataBuffers();
}

int SignalPlotter::horizontalScale() const
{
  return mHorizontalScale;
}

void SignalPlotter::setShowVerticalLines( bool value )
{
  mShowVerticalLines = value;
}

bool SignalPlotter::showVerticalLines() const
{
  return mShowVerticalLines;
}

void SignalPlotter::setVerticalLinesColor( const QColor &color )
{
  mVerticalLinesColor = color;
}

QColor SignalPlotter::verticalLinesColor() const
{
  return mVerticalLinesColor;
}

void SignalPlotter::setVerticalLinesDistance( int distance )
{
  mVerticalLinesDistance = distance;
}

int SignalPlotter::verticalLinesDistance() const
{
  return mVerticalLinesDistance;
}

void SignalPlotter::setVerticalLinesScroll( bool value )
{
  mVerticalLinesScroll = value;
}

bool SignalPlotter::verticalLinesScroll() const
{
  return mVerticalLinesScroll;
}

void SignalPlotter::setShowHorizontalLines( bool value )
{
  mShowHorizontalLines = value;
}

bool SignalPlotter::showHorizontalLines() const
{
  return mShowHorizontalLines;
}

void SignalPlotter::setHorizontalLinesColor( const QColor &color )
{
  mHorizontalLinesColor = color;
}

QColor SignalPlotter::horizontalLinesColor() const
{
  return mHorizontalLinesColor;
}

void SignalPlotter::setHorizontalLinesCount( int count )
{
  mHorizontalLinesCount = count;
}

int SignalPlotter::horizontalLinesCount() const
{
  return mHorizontalLinesCount;
}

void SignalPlotter::setShowLabels( bool value )
{
  mShowLabels = value;
}

bool SignalPlotter::showLabels() const
{
  return mShowLabels;
}

void SignalPlotter::setShowTopBar( bool value )
{
  mShowTopBar = value;
}

bool SignalPlotter::showTopBar() const
{
  return mShowTopBar;
}

void SignalPlotter::setFontSize( int size )
{
  mFontSize = size;
}

int SignalPlotter::fontSize() const
{
  return mFontSize;
}

void SignalPlotter::setBackgroundColor( const QColor &color )
{
  mBackgroundColor = color;
}

QColor SignalPlotter::backgroundColor() const
{
  return mBackgroundColor;
}

void SignalPlotter::resizeEvent( QResizeEvent* )
{
  Q_ASSERT( width() > 2 );

  updateDataBuffers();
}

void SignalPlotter::updateDataBuffers()
{
  /* Since the data buffers for the beams are equal in size to the
   * width of the widget minus 2 we have to enlarge or shrink the
   * buffers accordingly when a resize occures. To have a nicer
   * display we try to keep as much data as possible. Data that is
   * lost due to shrinking the buffers cannot be recovered on
   * enlarging though. */

  /* Determine new number of samples first.
   *  +0.5 to ensure rounding up
   *  +2 for extra data points so there is
   *     1) no wasted space and
   *     2) no loss of precision when drawing the first data point. */
  uint newSampleNum = static_cast<uint>( ( ( width() - 2 ) /
                                         mHorizontalScale ) + 2.5 );

  // overlap between the old and the new buffers.
  int overlap = min( mSamples, newSampleNum );

  for ( uint i = 0; i < mBeamData.count(); ++i ) {
    double* nd = new double[ newSampleNum ];

    // initialize new part of the new buffer
    if ( newSampleNum > (uint)overlap )
      memset( nd, 0, sizeof( double ) * ( newSampleNum - overlap ) );

    // copy overlap from old buffer to new buffer
    memcpy( nd + ( newSampleNum - overlap ), mBeamData.at( i ) +
            ( mSamples - overlap ), overlap * sizeof( double ) );

    mBeamData.remove( i );
    mBeamData.insert( i, nd );
  }

  mSamples = newSampleNum;
}

void SignalPlotter::paintEvent( QPaintEvent* )
{
  uint w = width();
  uint h = height();

  /* Do not do repaints when the widget is not yet setup properly. */
  if ( w <= 2 )
    return;

  QPixmap pm( w, h );
  QPainter p;
  p.begin( &pm, this );

  pm.fill( mBackgroundColor );
  /* Draw white line along the bottom and the right side of the
   * widget to create a 3D like look. */
  p.setPen( QColor( colorGroup().light() ) );
  p.drawLine( 0, h - 1, w - 1, h - 1 );
  p.drawLine( w - 1, 0, w - 1, h - 1 );

  p.setClipRect( 1, 1, w - 2, h - 2 );
  double range = mMaxValue - mMinValue;

  /* If the range is too small we will force it to 1.0 since it
   * looks a lot nicer. */
  if ( range < 0.000001 )
    range = 1.0;

  double minValue = mMinValue;
  if ( mUseAutoRange ) {
    if ( mMinValue != 0.0 ) {
      double dim = pow( 10, floor( log10( fabs( mMinValue ) ) ) ) / 2;
      if ( mMinValue < 0.0 )
        minValue = dim * floor( mMinValue / dim );
      else
        minValue = dim * ceil( mMinValue / dim );
      range = mMaxValue - minValue;
      if ( range < 0.000001 )
        range = 1.0;
    }
    // Massage the range so that the grid shows some nice values.
    double step = range / mHorizontalLinesCount;
    double dim = pow( 10, floor( log10( step ) ) ) / 2;
    range = dim * ceil( step / dim ) * mHorizontalLinesCount;
  }
  double maxValue = minValue + range;

  int top = 0;
  if ( mShowTopBar && h > ( mFontSize + 2 + mHorizontalLinesCount * 10 ) ) {
    /* Draw horizontal bar with current sensor values at top of display. */
    p.setPen( mHorizontalLinesColor );
    int x0 = w / 2;
    p.setFont( QFont( p.font().family(), mFontSize ) );
    top = p.fontMetrics().height();
    h -= top;
    int h0 = top - 2;
    p.drawText(0, 0, x0, top - 2, Qt::AlignCenter, mTitle );

    p.drawLine( x0 - 1, 1, x0 - 1, h0 );
    p.drawLine( 0, top - 1, w - 2, top - 1 );

    double bias = -minValue;
    double scaleFac = ( w - x0 - 2 ) / range;
    QList<QColor>::Iterator col;
    col = mBeamColor.begin();
    for ( double* d = mBeamData.first(); d; d = mBeamData.next(), ++col ) {
      int start = x0 + (int)( bias * scaleFac );
      int end = x0 + (int)( ( bias += d[ w - 3 ] ) * scaleFac );
      /* If the rect is wider than 2 pixels we draw only the last
       * pixels with the bright color. The rest is painted with
       * a 50% darker color. */
      if ( end - start > 1 ) {
        p.setPen( (*col).dark( 150 ) );
        p.setBrush( (*col).dark( 150 ) );
        p.drawRect( start, 1, end - start, h0 );
        p.setPen( *col );
        p.drawLine( end, 1, end, h0 );
      } else if ( start - end > 1 ) {
        p.setPen( (*col).dark( 150 ) );
        p.setBrush( (*col).dark( 150 ) );
        p.drawRect( end, 1, start - end, h0 );
        p.setPen( *col );
        p.drawLine( end, 1, end, h0 );
      } else {
        p.setPen( *col );
        p.drawLine( start, 1, start, h0 );
      }
    }
  }

  /* Draw scope-like grid vertical lines */
  if ( mShowVerticalLines && w > 60 ) {
    p.setPen( mVerticalLinesColor );
    for ( uint x = mVerticalLinesOffset; x < ( w - 2 ); x += mVerticalLinesDistance )
      p.drawLine( w - x, top, w - x, h + top - 2 );
  }

  /* In autoRange mode we determine the range and plot the values in
   * one go. This is more efficiently than running through the
   * buffers twice but we do react on recently discarded samples as
   * well as new samples one plot too late. So the range is not
   * correct if the recently discarded samples are larger or smaller
   * than the current extreme values. But we can probably live with
   * this. */
  if ( mUseAutoRange )
    mMinValue = mMaxValue = 0.0;

  /* Plot stacked values */
  double scaleFac = ( h - 2 ) / range;
  if ( mGraphStyle == GRAPH_ORIGINAL ) {
    int xPos = 0;
    for ( int i = 0; i < mSamples; i++, xPos += mHorizontalScale ) {
      double bias = -minValue;
      QList<QColor>::Iterator col;
      col = mBeamColor.begin();
      double sum = 0.0;
      for ( double* d = mBeamData.first(); d; d = mBeamData.next(), ++col ) {
        if ( mUseAutoRange ) {
          sum += d[ i ];
          if ( sum < mMinValue )
            mMinValue = sum;
          if ( sum > mMaxValue )
            mMaxValue = sum;
        }
        int start = top + h - 2 - (int)( bias * scaleFac );
        int end = top + h - 2 - (int)( ( bias + d[ i ] ) * scaleFac );
        bias += d[ i ];
        /* If the line is longer than 2 pixels we draw only the last
         * 2 pixels with the bright color. The rest is painted with
         * a 50% darker color. */
        if ( end - start > 2 ) {
          p.fillRect( xPos, start, mHorizontalScale, end - start - 1, (*col).dark( 150 ) );
          p.fillRect( xPos, end - 1, mHorizontalScale, 2, *col );
        } else if ( start - end > 2 ) {
          p.fillRect( xPos, start, mHorizontalScale, end - start + 1, (*col).dark( 150 ) );
          p.fillRect( xPos, end + 1, mHorizontalScale, 2, *col );
        } else
          p.fillRect( xPos, start, mHorizontalScale, end - start, *col );

      }
    }
  } else if ( mGraphStyle == GRAPH_POLYGON ) {
    int *prevVals = new int[ mBeamData.count() ];
    int hack[ 4 ];
    int x1 = w - ( ( mSamples + 1 ) * mHorizontalScale );

    for ( int i = 0; i < mSamples; i++ ) {
      QList<QColor>::Iterator col;
      col = mBeamColor.begin();
      double sum = 0.0;
      int y = top + h - 2;
      int oldY = top + h;
      int oldPrevY = oldY;
      int height = 0;
      int j = 0;
      int jMax = mBeamData.count() - 1;
      x1 += mHorizontalScale;
      int x2 = x1 + mHorizontalScale;

      for ( double* d = mBeamData.first(); d; d = mBeamData.next(), ++col, j++ ) {
        if ( mUseAutoRange ) {
          sum += d[ i ];
          if ( sum < mMinValue )
            mMinValue = sum;
          if ( sum > mMaxValue )
            mMaxValue = sum;
        }
        height = (int)( ( d[ i ] - minValue ) * scaleFac );
        y -= height;

        /* If the line is longer than 2 pixels we draw only the last
         * 2 pixels with the bright color. The rest is painted with
         * a 50% darker color. */
        QPen lastPen = QPen( p.pen() );
        p.setPen( (*col).dark( 150 ) );
        p.setBrush( (*col).dark( 150 ) );
        Q3PointArray pa( 4 );
        int prevY = ( i == 0 ) ? y : prevVals[ j ];
        pa.putPoints( 0, 1, x1, prevY );
        pa.putPoints( 1, 1, x2, y );
        pa.putPoints( 2, 1, x2, oldY );
        pa.putPoints( 3, 1, x1, oldPrevY );
        p.drawPolygon( pa );
        p.setPen( lastPen );
        if ( jMax == 0 ) {
          // draw as normal, no deferred drawing req'd.
          p.setPen( *col );
          p.drawLine( x1, prevY, x2, y );
        } else if ( j == jMax ) {
          // draw previous values and current values
          p.drawLine( hack[ 0 ], hack[ 1 ], hack[ 2 ], hack[ 3 ] );
          p.setPen( *col );
          p.drawLine( x1, prevY, x2, y );
        } else if ( j == 0 ) {
          // save values only
          hack[ 0 ] = x1;
          hack[ 1 ] = prevY;
          hack[ 2 ] = x2;
          hack[ 3 ] = y;
          p.setPen( *col );
        } else {
          p.drawLine( hack[ 0 ], hack[ 1 ], hack[ 2 ], hack[ 3 ] );
          hack[ 0 ] = x1;
          hack[ 1 ] = prevY;
          hack[ 2 ] = x2;
          hack[ 3 ] = y;
          p.setPen( *col );
        }

        prevVals[ j ] = y;
        oldY = y;
        oldPrevY = prevY;
      }
    }

    delete[] prevVals;
  }

  /* Draw horizontal lines and values. Lines are drawn when the
   * height is greater than 10 times hCount + 1, values are shown
   * when width is greater than 60 */
  if ( mShowHorizontalLines && h > ( 10 * ( mHorizontalLinesCount + 1 ) ) ) {
    p.setPen( mHorizontalLinesColor );
    p.setFont( QFont( p.font().family(), mFontSize ) );
    QString val;
    for ( uint y = 1; y < mHorizontalLinesCount; y++ ) {
      p.drawLine( 0, top + y * ( h / mHorizontalLinesCount ), w - 2,
                  top + y * ( h / mHorizontalLinesCount ) );
      if ( mShowLabels && h > ( mFontSize + 1 ) * ( mHorizontalLinesCount + 1 )
           && w > 60 ) {
        val = QString( "%1" ).arg( maxValue - y * ( range / mHorizontalLinesCount ) );
        p.drawText( 6, top + y * ( h / mHorizontalLinesCount ) - 1, val );
      }
    }

    if ( mShowLabels && h > ( mFontSize + 1 ) * ( mHorizontalLinesCount + 1 )
         && w > 60 ) {
      val = QString( "%1" ).arg( minValue );
      p.drawText( 6, top + h - 2, val );
    }
  }

  p.end();
  bitBlt( this, 0, 0, &pm );
}

#include "SignalPlotter.moc"
