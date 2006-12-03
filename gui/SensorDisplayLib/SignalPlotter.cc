/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 - 2002 Chris Schlaeger <cs@kde.org>
    Copyright (c) 2006 John Tapsell <tapsell@kde.org>

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
#include <QtSvg/QSvgRenderer>

#include <kstandarddirs.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#include "SignalPlotter.h"

static inline int min( int a, int b )
{
  return ( a < b ? a : b );
}

KSignalPlotter::KSignalPlotter( QWidget *parent)
  : QWidget( parent)
{
  mSvgRenderer = 0;
  mBezierCurveOffset = 0;
  mSamples = 0;
  mMinValue = mMaxValue = 0.0;
  mUseAutoRange = true;
  mScaleDownBy = 1;

  // Anything smaller than this does not make sense.
  setMinimumSize( 16, 16 );
  QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  sizePolicy.setHeightForWidth(false);
  setSizePolicy( sizePolicy );

  mShowVerticalLines = true;
  mVerticalLinesColor = QColor("green");
  mVerticalLinesDistance = 30;
  mVerticalLinesScroll = true;
  mVerticalLinesOffset = 0;
  mHorizontalScale = 1;

  mShowHorizontalLines = true;
  mHorizontalLinesColor = QColor("green");
  mHorizontalLinesCount = 5;

  mShowLabels = true;
  mShowTopBar = false;

  mBackgroundColor = QColor(0,0,0);
}

KSignalPlotter::~KSignalPlotter()
{
}

QString KSignalPlotter::translatedUnit() const {
  return mUnit;
}
void KSignalPlotter::setTranslatedUnit(const QString &unit) {
  mUnit= unit;
}


bool KSignalPlotter::addBeam( const QColor &color )
{
  QLinkedList< QList<double> >::Iterator it;
  //When we add a new beam, go back and set the data for this beam to 0 for all the other times. This is because it makes it easier for
  //moveSensors
  for(it = mBeamData.begin(); it != mBeamData.end(); ++it) {
    (*it).append(0);
  }
  mBeamColors.append(color);
  return true;
}

void KSignalPlotter::addSample( const QList<double>& sampleBuf )
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
  Q_ASSERT(sampleBuf.count() == mBeamColors.count());
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

void KSignalPlotter::reorderBeams( const QList<int>& newOrder )
{
  if(newOrder.count() != mBeamColors.count()) {
    kDebug() << "neworder has " << newOrder.count() << " and beam colors is " << mBeamColors.count() << endl;
    return;
  }
  QLinkedList< QList<double> >::Iterator it;
  for(it = mBeamData.begin(); it != mBeamData.end(); ++it) {
    if(newOrder.count() != (*it).count()) {
      kDebug() << "Serious problem in move sample.  beamdata[i] has " << (*it).count() << " and neworder has " << newOrder.count() << endl;
    } else {
     QList<double> newBeam;
     for(int i = 0; i < newOrder.count(); i++) {
        int newIndex = newOrder[i];
        newBeam.append((*it).at(newIndex));
      }
      (*it) = newBeam;
    }
  }
  QList< QColor> newBeamColors;
  for(int i = 0; i < newOrder.count(); i++) {
    int newIndex = newOrder[i];
    newBeamColors.append(mBeamColors.at(newIndex));
  }
  mBeamColors = newBeamColors;


}


void KSignalPlotter::changeRange( int beam, double min, double max )
{
  // Only the first beam affects range calculation.
  if ( beam > 1 )
    return;

  mMinValue = min;
  mMaxValue = max;
}

QList<QColor> &KSignalPlotter::beamColors()
{
  return mBeamColors;
}

void KSignalPlotter::removeBeam( uint pos )
{
  if(pos >= (uint)mBeamColors.size()) return;
  mBeamColors.removeAt( pos );

  QLinkedList< QList<double> >::Iterator i;
  for(i = mBeamData.begin(); i != mBeamData.end(); ++i) {
    if( (uint)(*i).size() >= pos)
      (*i).removeAt(pos);
  }
}

void KSignalPlotter::setScaleDownBy( double value ) { mScaleDownBy = value; }
double KSignalPlotter::scaleDownBy() const { return mScaleDownBy; }

void KSignalPlotter::setTitle( const QString &title )
{
  mTitle = title;
}

QString KSignalPlotter::title() const
{
  return mTitle;
}

void KSignalPlotter::setUseAutoRange( bool value )
{
  mUseAutoRange = value;
}

bool KSignalPlotter::useAutoRange() const
{
  return mUseAutoRange;
}

void KSignalPlotter::setMinValue( double min )
{
  mMinValue = min;
}

double KSignalPlotter::minValue() const
{
  return ( mUseAutoRange ? 0 : mMinValue );
}

void KSignalPlotter::setMaxValue( double max )
{
  mMaxValue = max;
}

double KSignalPlotter::maxValue() const
{
  return ( mUseAutoRange ? 0 : mMaxValue );
}

void KSignalPlotter::setHorizontalScale( uint scale )
{
  if (scale == mHorizontalScale)
     return;

  mHorizontalScale = scale;
  updateDataBuffers();
}

int KSignalPlotter::horizontalScale() const
{
  return mHorizontalScale;
}

void KSignalPlotter::setShowVerticalLines( bool value )
{
  mShowVerticalLines = value;
}

bool KSignalPlotter::showVerticalLines() const
{
  return mShowVerticalLines;
}

void KSignalPlotter::setVerticalLinesColor( const QColor &color )
{
  mVerticalLinesColor = color;
}

QColor KSignalPlotter::verticalLinesColor() const
{
  return mVerticalLinesColor;
}

void KSignalPlotter::setVerticalLinesDistance( int distance )
{
  mVerticalLinesDistance = distance;
}

int KSignalPlotter::verticalLinesDistance() const
{
  return mVerticalLinesDistance;
}

void KSignalPlotter::setVerticalLinesScroll( bool value )
{
  mVerticalLinesScroll = value;
}

bool KSignalPlotter::verticalLinesScroll() const
{
  return mVerticalLinesScroll;
}

void KSignalPlotter::setShowHorizontalLines( bool value )
{
  mShowHorizontalLines = value;
}

bool KSignalPlotter::showHorizontalLines() const
{
  return mShowHorizontalLines;
}
void KSignalPlotter::setFontColor( const QColor &color )
{
  mFontColor = color;
}

QColor KSignalPlotter::fontColor() const
{
  return mFontColor;
}


void KSignalPlotter::setHorizontalLinesColor( const QColor &color )
{
  mHorizontalLinesColor = color;
}

QColor KSignalPlotter::horizontalLinesColor() const
{
  return mHorizontalLinesColor;
}

void KSignalPlotter::setHorizontalLinesCount( int count )
{
  mHorizontalLinesCount = count;
}

int KSignalPlotter::horizontalLinesCount() const
{
  return mHorizontalLinesCount;
}

void KSignalPlotter::setShowLabels( bool value )
{
  mShowLabels = value;
}

bool KSignalPlotter::showLabels() const
{
  return mShowLabels;
}

void KSignalPlotter::setShowTopBar( bool value )
{
  mShowTopBar = value;
}

bool KSignalPlotter::showTopBar() const
{
  return mShowTopBar;
}

void KSignalPlotter::setFont( const QFont &font )
{
  mFont = font;
}

QFont KSignalPlotter::font() const
{
  return mFont;
}
QString KSignalPlotter::svgBackground() const {
  return mSvgFilename;
}
void KSignalPlotter::setSvgBackground( const QString &filename )
{
  mSvgFilename = filename;

  KStandardDirs* kstd = KGlobal::dirs();
  QString file = kstd->findResource( "data", "ksysguard/" + filename);

  if(!mSvgRenderer) {
    if(file.isEmpty()) return; //nothing changed
    mSvgRenderer = new QSvgRenderer(file, this);
  } else {
    if(file.isEmpty()) {
      delete mSvgRenderer;
      mSvgRenderer = 0;
    } else {
      mSvgRenderer->load(file);
    }
  }
  mBackgroundImage = QImage(); //we changed the svg, so reset the cache
}

void KSignalPlotter::setBackgroundColor( const QColor &color )
{
  mBackgroundColor = color;
}

QColor KSignalPlotter::backgroundColor() const
{
  return mBackgroundColor;
}

void KSignalPlotter::resizeEvent( QResizeEvent* )
{
  Q_ASSERT( width() > 2 );
  mBackgroundImage = QImage(); //set to null.  If it's invalid, it will be rerendered.
  updateDataBuffers();
}

void KSignalPlotter::updateDataBuffers()
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

void KSignalPlotter::paintEvent( QPaintEvent* )
{
  uint w = width();
  uint h = height();

  /* Do not do repaints when the widget is not yet setup properly. */
  if ( w <= 2 )
    return;
  QPainter p(this);
  p.setFont( mFont );

  if(mBackgroundImage.isNull()) {
    kDebug() << "setting up image" << endl;
    mBackgroundImage = QImage(w, h, QImage::Format_RGB32);
    QPainter pCache(&mBackgroundImage);
    pCache.setRenderHint(QPainter::Antialiasing, false);
    
    pCache.fillRect(0,0,w, h, mBackgroundColor);
    if(mSvgRenderer)
      mSvgRenderer->render(&pCache);

    /* Draw white line along the bottom and the right side of the
     * widget to create a 3D like look. */
    p.setPen( palette().color( QPalette::Light ) );
    p.drawLine( 0, h - 1, w - 1, h - 1 );
    p.drawLine( w - 1, 0, w - 1, h - 1 );
    p.setClipRect( 1, 1, w - 2, h - 2 );

  }
  p.drawImage(0,0, mBackgroundImage);
  p.setRenderHint(QPainter::Antialiasing, true);


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
    double step = range / (mHorizontalLinesCount+1);
    double dim = pow( 10, floor( log10( step ) ) ) / 2;
    range = dim * ceil( step / dim ) * (mHorizontalLinesCount+1);
  }
  double maxValue = minValue + range;

  uint top = 0; //The y position of the top of the graph.  Basically this is the height of the top bar
  if ( mShowTopBar ) {
    int x0 = w / 2;
    top = p.fontMetrics().height();
    //Before we continue, check if there's enough room.  So enough room for a bar at the top, plus horizontal lines each of a size with room for a scale
    if( h <= (top/*top bar size*/ + 2/*padding*/ +5/*smallest reasonable size for a graph*/ ) ) {
      top = 0;
    } else {
      h -= top;
      /* Draw horizontal bar with current sensor values at top of display. */
      int h0 = top - 2;
      p.setPen( mFontColor );
      p.drawText(0, 0, x0, top - 2, Qt::AlignCenter, mTitle );
      p.setPen( mHorizontalLinesColor );
      p.drawLine( x0 - 1, 1, x0 - 1, h0 );
      p.drawLine( 0, top - 1, w - 2, top - 1 );

      double bias = -minValue;
      double scaleFac = ( w - x0 - 2 ) / range;
      QList<QColor>::Iterator col;
      col = mBeamColors.begin();

      /**
       * The top bar shows the current values of all the beam data.
       * This iterates through each different beam and plots the newest data for each
       */
      if ( !mBeamData.isEmpty() ) {
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
  for (int i = 0; it != mBeamData.end() && i < mSamples; ++i) {
    double sum = 0.0;
    QPen pen;
    pen.setWidth(2);
    pen.setCapStyle(Qt::RoundCap);

    /**
     * We will plot 1 bezier curve for every 3 points, with the 4th point being the end
     * of one bezier curve and the start of the second.
     * This does means the bezier curves will not join nicely,
     * but it should be better than nothing.
     */

    QList<double> datapoints = *it;
    QList<double> prev_datapoints = datapoints;
    QList<double> prev_prev_datapoints = datapoints;
    QList<double> prev_prev_prev_datapoints = datapoints;

    if (i == 0 && mBezierCurveOffset>0) {
      /**
       * We are plotting an incomplete bezier curve - we don't have all the data we want.
       * Try to cope
       */
      xPos += mHorizontalScale*mBezierCurveOffset;
      if (mBezierCurveOffset == 1) {
        prev_datapoints = *it;
        ++it; //Now we are on the first element of the next group, if it exists
        if (it != mBeamData.end()) {
          prev_prev_prev_datapoints = prev_prev_datapoints = *it;
        } else {
          prev_prev_prev_datapoints = prev_prev_datapoints = prev_datapoints;
        }
      } else {
        // mBezierCurveOffset must be 2 now
        prev_datapoints = *it;
        Q_ASSERT(it != mBeamData.end());
        ++it;
        prev_prev_datapoints = *it;
        Q_ASSERT(it != mBeamData.end());
        ++it; //Now we are on the first element of the next group, if it exists
        if (it != mBeamData.end()) {
          prev_prev_prev_datapoints = *it;
        } else {
          prev_prev_prev_datapoints = prev_prev_datapoints;
        }
      }
    } else {
      /**
       * We have a group of 3 points at least.  That's 1 start point and 2 control points.
       */
      xPos += mHorizontalScale*3;
      it++;
      if (it != mBeamData.end()) {
        prev_datapoints = *it;
        it++;
        if (it != mBeamData.end()) {
          prev_prev_datapoints = *it;
          it++;  //We are now on the next set of data points
          if (it != mBeamData.end()) {
            // We have this datapoint, so use it for our finish point
            prev_prev_prev_datapoints = *it;
          } else {
            // We don't have the next set, so use our last control point as our finish point
            prev_prev_prev_datapoints = prev_prev_datapoints;
          }
        } else {
          prev_prev_prev_datapoints = prev_prev_datapoints = prev_datapoints;
        }
      } else {
          prev_prev_prev_datapoints = prev_prev_datapoints = prev_datapoints = datapoints;
      }
    }

    for (int j = 0; j < datapoints.size() && j < mBeamColors.size(); ++j) {
      if ( mUseAutoRange ) {
        sum += datapoints[j];
        if ( sum < mMinValue )
          mMinValue = sum;
        if ( sum > mMaxValue )
          mMaxValue = sum;
      }

      pen.setColor(mBeamColors[j]);
      p.setPen(pen);

      /*
       * Draw polygon only if enough data points are available.
       */
      if ( j < prev_prev_prev_datapoints.count() &&
           j < prev_prev_datapoints.count() &&
           j < prev_datapoints.count() ) {

        QPolygon curve( 4 );
        /* The height of the whole widget is h+top.  The height of the area we are plotting in is just h.
	 * The y coordinate system starts from the top, so at the bottom the y coordinate is h+top
	 * So to draw a point at value y', we need to put this at  h+top-y'
	 */
	int y;
        y = h + top - (int)((datapoints[j] - minValue)*scaleFac);
	if(y < (int)top) y = top;
        curve.setPoint( 0, w - xPos + 3*mHorizontalScale, y );

	y = h + top - (int)((prev_datapoints[j] - minValue)*scaleFac);
	if(y < (int)top) y = top;
        curve.setPoint( 1, w - xPos + 2*mHorizontalScale, y);

	y=   h + top - (int)((prev_prev_datapoints[j] - minValue)*scaleFac);
	if(y < (int)top) y = top;
        curve.setPoint( 2, w - xPos + mHorizontalScale, y );

	y =  h + top - (int)((prev_prev_prev_datapoints[j] - minValue)*scaleFac);
	if(y < (int)top) y = top;
        curve.setPoint( 3, w - xPos, y );

        QPainterPath path;
        path.moveTo( curve.at( 0 ) );
        path.cubicTo( curve.at( 1 ), curve.at( 2 ), curve.at( 3 ) );
        p.strokePath( path, p.pen() );
      }
    }
  }

  /* Draw horizontal lines and values. Lines are always drawn.
   * Values are only draw when width is greater than 60 */
  int fontheight = p.fontMetrics().height();
  if ( mShowHorizontalLines ) { 
    QString val;
    /* top = 0 or  font.height    depending on whether there's a topbar or not
     * h = graphing area.height   - i.e. the actual space we have to draw inside
     *
     * Note we are drawing from 0,0 as the top left corner.  So we have to add on top to get to the top of where we are drawing
     * so top+h is the height of the widget
     */

    for ( uint y = 1; y <= mHorizontalLinesCount; y++ ) {
      int y_coord =  top + (y * h) / (mHorizontalLinesCount+1);  //Make sure it's y*h first to avoid rounding bugs

      p.setPen( mHorizontalLinesColor );
      p.drawLine( 0, y_coord, w - 2, y_coord);
      if ( mShowLabels && h > ( fontheight + 1 ) * ( mHorizontalLinesCount + 1 )
           && w > 60 ) {
	double value = (maxValue - (y * range) / (mHorizontalLinesCount+1) )/mScaleDownBy;

        QString number = KGlobal::locale()->formatNumber( value, (value >= 100)?0:2);
        val = QString( "%1 %2" ).arg( number, mUnit );
        p.setPen( mFontColor );
        p.drawText( 6, y_coord - 2, val );
      }
    }

    //Draw the bottom most (minimum) number as well
    if ( mShowLabels && h > ( fontheight + 1 ) * ( mHorizontalLinesCount + 1 )
         && w > 60 ) {
      int value = (int)(minValue / mScaleDownBy);
      QString number = KGlobal::locale()->formatNumber( value, (value >= 100)?0:1);
      val = QString( "%1 %2" ).arg( number, mUnit);
      p.setPen( mFontColor );
      p.drawText( 6, top + h - 2, val );
    }
  }
}

#include "SignalPlotter.moc"
