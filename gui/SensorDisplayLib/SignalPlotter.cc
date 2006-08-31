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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <math.h>
#include <string.h>

#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QPainterPath>
#include <QtGui/QPolygon>

#include <kdebug.h>
#include <ksgrd/StyleEngine.h>

#include "SignalPlotter.h"

static inline int min( int a, int b )
{
  return ( a < b ? a : b );
}

SignalPlotter::SignalPlotter( QWidget *parent)
  : QWidget( parent)
{
  mBezierCurveOffset = 0;
  mSamples = 0;
  mMinValue = mMaxValue = 0.0;
  mUseAutoRange = true;

  mGraphStyle = GRAPH_POLYGON;

  // Anything smaller than this does not make sense.
  setMinimumSize( 16, 16 );
  QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  sizePolicy.setHeightForWidth(false);
  setSizePolicy( sizePolicy );

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
}

bool SignalPlotter::addBeam( const QColor &color )
{
  mBeamColors.append(color);
  return true;
}

void SignalPlotter::addSample( const QList<double>& sampleBuf )
{
  if(mSamples < 4) {
    //It might be possible, under some race conditions, for addSample to be called before mSamples is set
    //This is just to be safe
    kDebug() << "Error - mSamples is only " << mSamples << endl;
    updateDataBuffers();
    kDebug() << "mSamples is now " << mSamples << endl;
    if(mSamples < 4)
      return;
  }
  mBeamData.prepend(sampleBuf);
  if(mBeamData.size() > mSamples) {
    mBeamData.removeLast(); // we have too many.  Remove the last item
    if(mBeamData.size() > mSamples)
      mBeamData.removeLast(); // If we still have too many, then we have resized the widget.  Remove one more.  That way we will slowly resize to the new size
  }

  if(mBezierCurveOffset >= 2) mBezierCurveOffset = 0;
  else mBezierCurveOffset++;

  Q_ASSERT((uint)mBeamData.size() >= mBezierCurveOffset);
  
  //FIXME: IS THIS NEEDED STILL?
  if ( mUseAutoRange ) { /* When all the data points are stacked on top of each other, the range will be up to the sum of them all */
    double sum = 0;
    for(int i =0; i < sampleBuf.size(); i++)
      sum += sampleBuf[i];
    if ( sum < mMinValue )
      mMinValue = sum;
    if ( sum > mMaxValue )
      mMaxValue = sum;
  }

  /* If the vertical lines are scrolling, increment the offset
   * so they move with the data. */
  if ( mVerticalLinesScroll ) {
    mVerticalLinesOffset = ( mVerticalLinesOffset + mHorizontalScale)
                           % mVerticalLinesDistance;
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
  return mBeamColors;
}

void SignalPlotter::removeBeam( uint pos )
{
  if(pos >= (uint)mBeamColors.size()) return;
  mBeamColors.removeAt( pos );

  QLinkedList< QList<double> >::Iterator i;
  for(i = mBeamData.begin(); i != mBeamData.end(); ++i) {
    if( (uint)(*i).size() >= pos)
      (*i).removeAt(pos);
  }
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

  /*  This is called when the widget has resized
   *
   *  Determine new number of samples first.
   *  +0.5 to ensure rounding up
   *  +4 for extra data points so there is
   *     1) no wasted space and
   *     2) no loss of precision when drawing the first data point. */
  mSamples = static_cast<uint>( ( ( width() - 2 ) /
                                mHorizontalScale ) + 4.5 );
}

void SignalPlotter::paintEvent( QPaintEvent* )
{
  uint w = width();
  uint h = height();

  /* Do not do repaints when the widget is not yet setup properly. */
  if ( w <= 2 )
    return;
  QPainter p(this);

  p.setRenderHint(QPainter::Antialiasing, true);

  p.fillRect(0,0,w, h, mBackgroundColor);
  /* Draw white line along the bottom and the right side of the
   * widget to create a 3D like look. */
  p.setPen( palette().color( QPalette::Light ) );
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
    col = mBeamColors.begin();

    /* The top bar shows the current values of all the beam data.  This iterates through each different beam and plots the newest data for each*/
    QList<double> newestData = mBeamData.first();
    QList<double>::Iterator i;
    for(i = newestData.begin(); i != newestData.end(); ++i, ++col) {
      double newest_datapoint = *i;
      int start = x0 + (int)( bias * scaleFac );
      int end = x0 + (int)( ( bias += newest_datapoint ) * scaleFac );
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

    QLinkedList< QList<double> >::Iterator it = mBeamData.begin();
    for(int i = 0; it != mBeamData.end() && i < mSamples; ++it, ++i) {
      xPos += mHorizontalScale;

      double bias = -minValue;
      QList<QColor>::Iterator col;
      col = mBeamColors.begin();
      double sum = 0.0;
      const QList<double> &datapoints = *it;
      QList<double>::ConstIterator datapoint = datapoints.begin(); //The list of data points using all the colors for a particular point in time
      for (; datapoint != datapoints.end() && col != mBeamColors.end(); ++datapoint, ++col ) {
        if ( mUseAutoRange ) {
          sum += *datapoint;
          if ( sum < mMinValue )
            mMinValue = sum;
          if ( sum > mMaxValue )
            mMaxValue = sum;
        }
        int start = top + h - 2 - (int)( bias * scaleFac );
        int end = top + h - 2 - (int)( ( bias + *datapoint ) * scaleFac );
        bias += *datapoint;
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
    int xPos = 0;
    QLinkedList< QList<double> >::Iterator it = mBeamData.begin();

    /* mBezierCurveOffset is how many points we have at the start.
     * All the bezier curves are in groups of 3, with the first of the next group being the last point
     * of the previous group.
     *
     * Example, when mBezierCurveOffset == 0, and we have data, then just plot a normal bezier curve 
     * (we will have at least 3 points in this case)
     * When mBezierCurveOffset == 1, then we want a bezier curve that uses the first data point and 
     * the second data point.  Then the next group starts from the second data point.
     * When mBezierCurveOffset == 2, then we want a bezier curve that uses the first, second and third data
     *
     */

    for(int i = 0; it != mBeamData.end() && i < mSamples; ++i) {
      // double bias = -minValue;
      double sum = 0.0;
      QPen pen;
      pen.setWidth(2);
      pen.setCapStyle(Qt::RoundCap);

      // We will plot 1 bezier curve for every 3 points, with the 4th point being the end of one bezier curve and the start of the second.
      // This does means the bezier curves will not join nicely, but it should be better than nothing.

      QList<double> datapoints = *it;
      QList<double> prev_datapoints = datapoints;
      QList<double> prev_prev_datapoints = datapoints;
      QList<double> prev_prev_prev_datapoints = datapoints;

      if(i == 0 && mBezierCurveOffset>0) {
	//We are plotting an incomplete bezier curve - we don't have all the data we want.  Try to cope
        xPos += mHorizontalScale*mBezierCurveOffset;
	if(mBezierCurveOffset == 1) {
	  prev_datapoints = *it;
          ++it; //Now we are on the first element of the next group, if it exists
	  if(it != mBeamData.end()) {
            prev_prev_prev_datapoints = prev_prev_datapoints = *it;
	  } else {
            prev_prev_prev_datapoints = prev_prev_datapoints = prev_datapoints;
	  }
	} else {
	  //mBezierCurveOffset must be 2 now
          prev_datapoints = *it;
	  Q_ASSERT(it != mBeamData.end());
	  ++it;
	  prev_prev_datapoints = *it;
	  Q_ASSERT(it != mBeamData.end());
	  ++it; //Now we are on the first element of the next group, if it exists
	  if(it != mBeamData.end()) {
            prev_prev_prev_datapoints = *it;
	  } else {
            prev_prev_prev_datapoints = prev_prev_datapoints;
	  }
	}
      } else {
	//We have a group of 3 points at least.  That's 1 start point and 2 control points.
        xPos += mHorizontalScale*3;
	it++;
	if(it != mBeamData.end()) {
          prev_datapoints = *it;
          it++;
	  if(it != mBeamData.end()) {
            prev_prev_datapoints = *it;
	    it++;  //We are now on the next set of data points
            if(it != mBeamData.end()) {
              prev_prev_prev_datapoints = *it; //We have this datapoint, so use it for our finish point
            } else {
              prev_prev_prev_datapoints = prev_prev_datapoints;  //we don't have the next set, so use our last control point as our finish point
	    }
	  } else {
            prev_prev_prev_datapoints = prev_prev_datapoints = prev_datapoints;
	  }
	} else {
            prev_prev_prev_datapoints = prev_prev_datapoints = prev_datapoints = datapoints;
	}
      }

      for(int j = 0; j < datapoints.size() && j < mBeamColors.size(); ++j) {
        if ( mUseAutoRange ) {
          sum += datapoints[j];
          if ( sum < mMinValue )
            mMinValue = sum;
          if ( sum > mMaxValue )
            mMaxValue = sum;
        }
//        int start = top + h - 2 - (int)( bias * scaleFac );
//        int end = top + h - 2 - (int)( ( bias + *datapoint ) * scaleFac );
//        bias += *datapoint;

	pen.setColor(mBeamColors[j]);
	p.setPen(pen);
	QPolygon curve(3);
	curve.putPoints(0,4, w - xPos + 3*mHorizontalScale, h - (int)((datapoints[j] - minValue)*scaleFac),
			     w - xPos + 2*mHorizontalScale, h - (int)((prev_datapoints[j] - minValue)*scaleFac),
			     w - xPos + mHorizontalScale, h - (int)((prev_prev_datapoints[j] - minValue)*scaleFac),
			     w - xPos, h - (int)((prev_prev_prev_datapoints[j] - minValue)*scaleFac));

        QPainterPath path;
        path.moveTo( curve.at( 0 ) );
        path.cubicTo( curve.at( 1 ), curve.at( 2 ), curve.at( 3 ) );
        p.strokePath( path, p.pen() );
//	p.drawLine( w - xPos, h - (int)((prev_prev_datapoints[i] - minValue)*scaleFac),
//		    w - xPos - mHorizontalScale + 1, h - (int)((prev_datapoints[i] - minValue)*scaleFac));

//	p.drawLine( w - xPos, h - (int)((prev_datapoints[i] - minValue)*scaleFac), w - xPos - mHorizontalScale + 1, h- (int)((datapoints[i] - minValue)*scaleFac));
        /* If the line is longer than 2 pixels we draw only the last
         * 2 pixels with the bright color. The rest is painted with
         * a 50% darker color. */
/*        if ( end - start > 2 ) {
//          p.fillRect( xPos, start, mHorizontalScale, end - start - 1, (*col).dark( 150 ) );
	  p.drawLine( xPos+1, end -1, xPos + mHorizontalScale-1, end -1);
//          p.fillRect( xPos, end - 1, mHorizontalScale, 2, *col );
        } else if ( start - end > 2 ) {
//          p.fillRect( xPos, start, mHorizontalScale, end - start + 1, (*col).dark( 150 ) );
	  pen.setColor(*col);
	  p.setPen(pen);
	  p.drawLine( xPos+1, end + 1, xPos + mHorizontalScale-1,  end+1);
//          p.fillRect( xPos, end + 1, mHorizontalScale, 2, *col );
        } else
          p.fillRect( xPos, start, mHorizontalScale, end - start, *col );
*/
      }
    }




#if 0
    int *prevVals = new int[ mBeamData.count() ];
    int hack[ 4 ];
    int x1 = w - ( ( mSamples + 1 ) * mHorizontalScale );

    for ( int i = 0; i < mSamples; i++ ) {
      QList<QColor>::Iterator col;
      col = mBeamColors.begin();
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
        QPolygon pa( 4 );
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
#endif
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
}

#include "SignalPlotter.moc"
