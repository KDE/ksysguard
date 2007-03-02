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

QHash<QString, QSvgRenderer *> KSignalPlotter::sSvgRenderer ;

KSignalPlotter::KSignalPlotter( QWidget *parent)
  : QWidget( parent)
{
  mPrecision = 0;
  mBezierCurveOffset = 0;
  mSamples = 0;
  mMinValue = mMaxValue = 0.0;
  mNiceMinValue = mNiceMaxValue = 0.0;
  mNiceRange = 0;
  mUseAutoRange = true;
  mScaleDownBy = 1;
  mShowThinFrame = true;

  // Anything smaller than this does not make sense.
  setMinimumSize( 16, 16 );
  QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  sizePolicy.setHeightForWidth(false);
  setSizePolicy( sizePolicy );

  mShowVerticalLines = true;
  mVerticalLinesColor = QColor("black");
  mVerticalLinesDistance = 30;
  mVerticalLinesScroll = true;
  mVerticalLinesOffset = 0;
  mHorizontalScale = 1;

  mShowHorizontalLines = true;
  mHorizontalLinesColor = QColor("black");
  mHorizontalLinesCount = 5;

  mShowLabels = true;
  mShowTopBar = false;
  mStackBeams = true;
  mFillBeams = true;

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
  mBeamColorsDark.append(color.dark(150));
  return true;
}

void KSignalPlotter::addSample( const QList<double>& sampleBuf )
{
  if(mSamples < 4) {
    //It might be possible, under some race conditions, for addSample to be called before mSamples is set
    //This is just to be safe
    kDebug(1215) << "Error - mSamples is only " << mSamples << endl;
    updateDataBuffers();
    kDebug(1215) << "mSamples is now " << mSamples << endl;
    if(mSamples < 4)
      return;
  }
  mBeamData.prepend(sampleBuf);
  Q_ASSERT(sampleBuf.count() == mBeamColors.count());
  if((unsigned int)mBeamData.size() > mSamples) {
    mBeamData.removeLast(); // we have too many.  Remove the last item
    if((unsigned int)mBeamData.size() > mSamples)
      mBeamData.removeLast(); // If we still have too many, then we have resized the widget.  Remove one more.  That way we will slowly resize to the new size
  }

  if(mBezierCurveOffset >= 2) mBezierCurveOffset = 0;
  else mBezierCurveOffset++;

  Q_ASSERT((uint)mBeamData.size() >= mBezierCurveOffset);
  
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
    kDebug(1215) << "neworder has " << newOrder.count() << " and beam colors is " << mBeamColors.count() << endl;
    return;
  }
  QLinkedList< QList<double> >::Iterator it;
  for(it = mBeamData.begin(); it != mBeamData.end(); ++it) {
    if(newOrder.count() != (*it).count()) {
      kDebug(1215) << "Serious problem in move sample.  beamdata[i] has " << (*it).count() << " and neworder has " << newOrder.count() << endl;
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
  QList< QColor> newBeamColorsDark;
  for(int i = 0; i < newOrder.count(); i++) {
    int newIndex = newOrder[i];
    newBeamColors.append(mBeamColors.at(newIndex));
    newBeamColorsDark.append(mBeamColorsDark.at(newIndex));
  }
  mBeamColors = newBeamColors;
  mBeamColorsDark = newBeamColorsDark;
}


void KSignalPlotter::changeRange( int beam, double min, double max )
{
  // Only the first beam affects range calculation.
  if ( beam > 1 )
    return;

  mMinValue = min;
  mMaxValue = max;
  calculateNiceRange();
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

void KSignalPlotter::setScaleDownBy( double value )
{ 
  if(mScaleDownBy == value) return;
  mScaleDownBy = value;
  mBackgroundImage = QImage(); //we changed a paint setting, so reset the cache
  calculateNiceRange();
}
double KSignalPlotter::scaleDownBy() const { return mScaleDownBy; }

void KSignalPlotter::setTitle( const QString &title )
{
  if(mTitle == title) return;
  mTitle = title;
  mBackgroundImage = QImage(); //we changed a paint setting, so reset the cache
}

QString KSignalPlotter::title() const
{
  return mTitle;
}

void KSignalPlotter::setUseAutoRange( bool value )
{
  mUseAutoRange = value;
  calculateNiceRange();
  //this change will be detected in paint and the image cache regenerated
}

bool KSignalPlotter::useAutoRange() const
{
  return mUseAutoRange;
}

void KSignalPlotter::setMinValue( double min )
{
  mMinValue = min;
  calculateNiceRange();
  //this change will be detected in paint and the image cache regenerated
}

double KSignalPlotter::minValue() const
{
  return mMinValue;
}

void KSignalPlotter::setMaxValue( double max )
{
  mMaxValue = max;
  calculateNiceRange();
  //this change will be detected in paint and the image cache regenerated
}

double KSignalPlotter::maxValue() const
{
  return mMaxValue;
}

void KSignalPlotter::setHorizontalScale( uint scale )
{
  if (scale == mHorizontalScale)
     return;

  mHorizontalScale = scale;
  updateDataBuffers();
  mBackgroundImage = QImage(); //we changed a paint setting, so reset the cache
}

int KSignalPlotter::horizontalScale() const
{
  return mHorizontalScale;
}

void KSignalPlotter::setShowVerticalLines( bool value )
{
  if(mShowVerticalLines == value) return;
  mShowVerticalLines = value;
  mBackgroundImage = QImage(); //we changed a paint setting, so reset the cache
}

bool KSignalPlotter::showVerticalLines() const
{
  return mShowVerticalLines;
}

void KSignalPlotter::setVerticalLinesColor( const QColor &color )
{
  if(mVerticalLinesColor == color) return;
  mVerticalLinesColor = color;
  mBackgroundImage = QImage(); //we changed a paint setting, so reset the cache
}

QColor KSignalPlotter::verticalLinesColor() const
{
  return mVerticalLinesColor;
}

void KSignalPlotter::setVerticalLinesDistance( uint distance )
{
  if(distance == mVerticalLinesDistance) return;
  mVerticalLinesDistance = distance;
  mBackgroundImage = QImage(); //we changed a paint setting, so reset the cache
}

int KSignalPlotter::verticalLinesDistance() const
{
  return mVerticalLinesDistance;
}

void KSignalPlotter::setVerticalLinesScroll( bool value )
{
  if(value == mVerticalLinesScroll) return;
  mVerticalLinesScroll = value;
  mBackgroundImage = QImage(); //we changed a paint setting, so reset the cache
}

bool KSignalPlotter::verticalLinesScroll() const
{
  return mVerticalLinesScroll;
}

void KSignalPlotter::setShowHorizontalLines( bool value )
{
  if(value == mShowHorizontalLines) return;
  mShowHorizontalLines = value;
  mBackgroundImage = QImage(); //we changed a paint setting, so reset the cache
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
  if(color == mHorizontalLinesColor) return;
  mHorizontalLinesColor = color;
  mBackgroundImage = QImage(); //we changed a paint setting, so reset the cache
}

QColor KSignalPlotter::horizontalLinesColor() const
{
  return mHorizontalLinesColor;
}

void KSignalPlotter::setHorizontalLinesCount( uint count )
{
  if(count == mHorizontalLinesCount) return;
  mHorizontalLinesCount = count;
  mBackgroundImage = QImage(); //we changed a paint setting, so reset the cache
  calculateNiceRange();
}

int KSignalPlotter::horizontalLinesCount() const
{
  return mHorizontalLinesCount;
}

void KSignalPlotter::setShowLabels( bool value )
{
  if(value == mShowLabels) return;
  mShowLabels = value;
  mBackgroundImage = QImage(); //we changed a paint setting, so reset the cache
}

bool KSignalPlotter::showLabels() const
{
  return mShowLabels;
}

void KSignalPlotter::setShowTopBar( bool value )
{
  if(mShowTopBar == value) return;
  mShowTopBar = value;
  mBackgroundImage = QImage(); //we changed a paint setting, so reset the cache
}

bool KSignalPlotter::showTopBar() const
{
  return mShowTopBar;
}

void KSignalPlotter::setFont( const QFont &font )
{
  mFont = font;
  mBackgroundImage = QImage(); //we changed a paint setting, so reset the cache
}

QFont KSignalPlotter::font() const
{
  return mFont;
}
QString KSignalPlotter::svgBackground() { 
  return mSvgFilename;
}
void KSignalPlotter::setSvgBackground( const QString &filename )
{
  if(mSvgFilename == filename) return;
  mSvgFilename = filename;
  //NOTE:  We don't free the old svg renderer.  This means that it will leak if we set it to use one svg, then reset it to use another svg.  
  //The svg rendererer object will be created on demand in drawBackground
}

void KSignalPlotter::setBackgroundColor( const QColor &color )
{
  if(color == mBackgroundColor) return;
  mBackgroundColor = color;
  mBackgroundImage = QImage(); //we changed a paint setting, so reset the cache
}

QColor KSignalPlotter::backgroundColor() const
{
  return mBackgroundColor;
}

void KSignalPlotter::setThinFrame( bool set) 
{
  if(mShowThinFrame == set) return;
  mShowThinFrame = set;
  mBackgroundImage = QImage(); //we changed a paint setting, so reset the cache
}
void KSignalPlotter::resizeEvent( QResizeEvent* )
{
  Q_ASSERT( width() > 2 );
//  mBackgroundImage = mBackgroundImage.scaled(width(), height()); //set to null.  If it's invalid, it will be rerendered.
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
QImage KSignalPlotter::getSnapshotImage(uint w, uint height)
{
  uint horizontalStep = (uint)((1.0*w/width())+0.5); //get the closest integer horizontal step
  uint newWidth = horizontalStep * width();
  QImage image = QImage(newWidth, height, QImage::Format_RGB32);
  QPainter p(&image);
  drawWidget(&p, newWidth, height, newWidth);
  return image;
}
void KSignalPlotter::paintEvent( QPaintEvent* )
{
  uint w = width();
  uint h = height();

  /* Do not do repaints when the widget is not yet setup properly. */
  if ( w <= 2 )
    return;
  QPainter p(this);
  drawWidget(&p, w, h, mHorizontalScale);
}
void KSignalPlotter::drawWidget(QPainter *p, uint w, uint height, int horizontalScale)
{
  uint h = height; //h will become the height of just the bit we draw the beams in
  p->setFont( mFont );

  uint fontheight = p->fontMetrics().height();
  if(mMinValue < mNiceMinValue || mMaxValue > mNiceMaxValue || mMaxValue < (mNiceRange*0.75 + mNiceMinValue))
    calculateNiceRange();
  QPen pen;
  pen.setWidth(1);
  pen.setCapStyle(Qt::RoundCap);
  p->setPen(pen);

  uint top = p->pen().width() / 2; //The y position of the top of the graph.  Basically this is one more than the height of the top bar
  h-= top;

  //check if there's enough room to actually show a top bar. Must be enough room for a bar at the top, plus horizontal lines each of a size with room for a scale
  bool showTopBar = mShowTopBar &&  h > (fontheight/*top bar size*/ +5/*smallest reasonable size for a graph*/ );
  if(showTopBar) {
    top += fontheight; //The top bar has the same height as fontheight. Thus the top of the graph is at fontheight
    h -= fontheight;
  }
  if(mBackgroundImage.isNull() || (uint)mBackgroundImage.height() != height || (uint)mBackgroundImage.width() != w) { //recreate on resize etc
    mBackgroundImage = QImage(w, height, QImage::Format_RGB32);
    QPainter pCache(&mBackgroundImage);
    pCache.setRenderHint(QPainter::Antialiasing, false);
    pCache.setFont( mFont );

    drawBackground(&pCache, w, height);

    if(mShowThinFrame) {
      drawThinFrame(&pCache, w, height);
      //We have a 'frame' in the bottom and right - so subtract them from the view
      h--;
      w--;
      pCache.setClipRect( 0, 0, w, height-1 );
    }
    
    if(showTopBar) { 
      int seperatorX = w / 2;
      drawTopBarFrame(&pCache, w, seperatorX, top);
    }

    /* Draw scope-like grid vertical lines if it doesn't move.  If it does move, draw it in the dynamic part of the code*/
    if(!mVerticalLinesScroll && mShowVerticalLines && w > 60)
      drawVerticalLines(&pCache, top, w, h);

    if ( mShowHorizontalLines ) 
      drawHorizontalLines(&pCache, top, w, h);
  
  } else {
    if(mShowThinFrame) {
      //We have a 'frame' in the bottom and right - so subtract them from the view
      h--;
      w--;
   }  
  }
  p->drawImage(0,0, mBackgroundImage);
  p->setRenderHint(QPainter::Antialiasing, true);

  if ( showTopBar ) {
    int seperatorX = w / 2;
    int topBarWidth = w - seperatorX -2;
    drawTopBarContents(p, seperatorX, topBarWidth, top -1);
  }

  p->setClipRect( 0, top, w, h);
  /* Draw scope-like grid vertical lines */
  if ( mVerticalLinesScroll && mShowVerticalLines && w > 60 )
    drawVerticalLines(p, top, w, h);

  drawBeams(p, top, w, h, horizontalScale);

  if( mShowLabels && w > 60 && h > ( fontheight + 1 ) )   //if there's room to draw the labels, then draw them!
    drawAxisText(p, top, h);

}
void KSignalPlotter::drawBackground(QPainter *p, int w, int h)
{
  p->fillRect(0,0,w, h, mBackgroundColor);

  if(mSvgFilename.isEmpty())
    return; //nothing to draw, return

  QSvgRenderer *svgRenderer;
  if(!sSvgRenderer.contains(mSvgFilename)) {
    KStandardDirs* kstd = KGlobal::dirs();
    QString file = kstd->findResource( "data", "ksysguard/" + mSvgFilename);

    svgRenderer =  new QSvgRenderer(file, this);
    sSvgRenderer.insert(mSvgFilename, svgRenderer);
  } else
    svgRenderer = sSvgRenderer[mSvgFilename];
  svgRenderer->render(p);
}

void KSignalPlotter::drawThinFrame(QPainter *p, int w, int h)
{
  /* Draw white line along the bottom and the right side of the
   * widget to create a 3D like look. */
  p->setPen( palette().color( QPalette::Light ) );
  p->drawLine( 0, h - 1, w - 1, h - 1 );
  p->drawLine( w - 1, 0, w - 1, h - 1 ); 
}

void KSignalPlotter::calculateNiceRange()
{
  mNiceRange = mMaxValue - mMinValue;
  /* If the range is too small we will force it to 1.0 since it
   * looks a lot nicer. */
  if ( mNiceRange < 0.000001 )
    mNiceRange = 1.0;

  mNiceMinValue = mMinValue;
//  if ( mUseAutoRange ) {
    if ( mMinValue != 0.0 ) {
      double dim = pow( 10, floor( log10( fabs( mMinValue ) ) ) ) / 2;
      if ( mMinValue < 0.0 )
        mNiceMinValue = dim * floor( mMinValue / dim );
      else
        mNiceMinValue = dim * ceil( mMinValue / dim );
      mNiceRange = mMaxValue - mNiceMinValue;
      if ( mNiceRange < 0.000001 )
        mNiceRange = 1.0;
    }
    // Massage the range so that the grid shows some nice values.
    double step = mNiceRange / (mScaleDownBy*(mHorizontalLinesCount+1));
    int logdim = (int)floor( log10( step ) );
    double dim = pow( 10, logdim ) / 2;
    int a = (int)ceil( step / dim );
    if(logdim >= 0)
        mPrecision = 0;
    else if( a % 2 == 0){
        mPrecision =-logdim;
    } else {
	mPrecision = 1-logdim;
    }
    mNiceRange = mScaleDownBy*dim * a * (mHorizontalLinesCount+1);
//  }
  mNiceMaxValue = mNiceMinValue + mNiceRange;
}


void KSignalPlotter::drawTopBarFrame(QPainter *p, int fullWidth, int seperatorX, int height)
{
      /* Draw horizontal bar with current sensor values at top of display. */

      //remember that it has a height of 'height'.  Thus the lowest pixel it can draw on is height-1 since we count from 0
      p->setPen( Qt::NoPen);
//      p->fillRect( 0, 0, fullWidth, height-1, QBrush(QColor(255,255,255,60)));
      p->setPen( mFontColor );
      p->drawText(0, 1, seperatorX, height, Qt::AlignCenter, mTitle );
      p->setPen( mHorizontalLinesColor );
      p->drawLine( seperatorX - 1, 1, seperatorX - 1, height-1 );
}

void KSignalPlotter::drawTopBarContents(QPainter *p, int x, int width, int height)
{
  //The height is the height of the contents, so this will be one pixel less than the height of the topbar
  double bias = -mNiceMinValue;
  double scaleFac = width / mNiceRange;
  QList<QColor>::Iterator col;
  col = mBeamColors.end();
  /**
   * The top bar shows the current values of all the beam data.
   * This iterates through each different beam and plots the newest data for each
   */
  if ( !mBeamData.isEmpty() ) {
    QList<double> newestData = mBeamData.first();
    for(int i = newestData.count()-1; i >= 0; --i) {
      double newest_datapoint = newestData.at(i);
      int start = x + (int)( bias * scaleFac );
      int end = x + (int)( ( bias += newest_datapoint ) * scaleFac );
      int start2 = qMin(start,end);
      end = qMax(start,end);
      start = start2;

      /* If the rect is wider than 2 pixels we draw only the last
       * pixels with the bright color. The rest is painted with
       * a 50% darker color. */

      p->setPen(Qt::NoPen);
      QLinearGradient  linearGrad( QPointF(start,1), QPointF(end, 1));
      linearGrad.setColorAt(0, mBeamColorsDark[i]);
      linearGrad.setColorAt(1, mBeamColors[i]);
      p->fillRect( start, 1, end - start, height-1, QBrush(linearGrad));
    }
  }
}
void KSignalPlotter::drawVerticalLines(QPainter *p, int top, int w, int h)
{
  p->setPen( mVerticalLinesColor );
  for ( int x = mVerticalLinesOffset; x < ( w - 2 ); x += mVerticalLinesDistance )
      p->drawLine( w - x, top, w - x, h + top -1 );
}

void KSignalPlotter::drawBeams(QPainter *p, int top, int w, int h, int horizontalScale)
{
  double scaleFac = (h-1) / mNiceRange;

  int xPos = 0;
  QLinkedList< QList<double> >::Iterator it = mBeamData.begin();

  p->setPen(Qt::NoPen);
  /* In autoRange mode we determine the range and plot the values in
   * one go. This is more efficiently than running through the
   * buffers twice but we do react on recently discarded samples as
   * well as new samples one plot too late. So the range is not
   * correct if the recently discarded samples are larger or smaller
   * than the current extreme values. But we can probably live with
   * this.
   *
   * These values aren't used directly anywhere.  Instead we call
   * calculateNiceRange()  which massages these values into a nicer 
   * values.  Rounding etc.  This means it's safe to change these values
   * without affecting any other drawings
   * */
  if ( mUseAutoRange )
    mMinValue = mMaxValue = 0.0;

  /* mBezierCurveOffset is how many points we have at the start.
   * All the bezier curves are in groups of 3, with the first of the next group being the last point
   * of the previous group->
   *
   * Example, when mBezierCurveOffset == 0, and we have data, then just plot a normal bezier curve 
   * (we will have at least 3 points in this case)
   * When mBezierCurveOffset == 1, then we want a bezier curve that uses the first data point and 
   * the second data point.  Then the next group starts from the second data point.
   * When mBezierCurveOffset == 2, then we want a bezier curve that uses the first, second and third data
   *
   */
  for (unsigned int i = 0; it != mBeamData.end() && i < mSamples; ++i) {
    QPen pen;
    pen.setWidth(1);
    pen.setCapStyle(Qt::FlatCap);

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
      xPos += horizontalScale*mBezierCurveOffset;
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
      xPos += horizontalScale*3;
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


    float x0 = w - xPos + 3.0*horizontalScale;
    float x1 = w - xPos + 2.0*horizontalScale;
    float x2 = w - xPos + 1.0*horizontalScale;
    float x3 = w - xPos;
    float y0 = h -1 + top;
    float y1 = y0;
    float y2 = y0;
    float y3 = y0;
    int offset = 0; //Our line is 2 pixels thick.  This means that when we draw the area, we need to offset 
    double max_y=0;
    double min_y=0;
    for (int j =  qMin(datapoints.size(), mBeamColors.size())-1; j >=0 ; --j) {
      if ( mUseAutoRange) {
        //If we use autorange, then we need to prepare the min and max values for _next_ time we paint
	//if we are stacking the beams, then we need to add the maximums together
	double current_maxvalue = qMax(datapoints[j], qMax(prev_datapoints[j], qMax(prev_prev_datapoints[j], prev_prev_prev_datapoints[j])));
	double current_minvalue = qMin(datapoints[j], qMin(prev_datapoints[j], qMin(prev_prev_datapoints[j], prev_prev_prev_datapoints[j])));
	mMaxValue = qMax(mMaxValue, current_maxvalue);
	mMinValue = qMin(mMinValue, current_maxvalue);
	if( mStackBeams ) {
	  max_y += current_maxvalue;
	  min_y += current_minvalue;
	}
      }

      /*
       * Draw polygon only if enough data points are available.
       */
      if ( j < prev_prev_prev_datapoints.count() &&
           j < prev_prev_datapoints.count() &&
           j < prev_datapoints.count() ) {

        QPolygon curve( 4 );


        /* The height of the whole widget is h+top->  The height of the area we are plotting in is just h.
	 * The y coordinate system starts from the top, so at the bottom the y coordinate is h+top
	 * So to draw a point at value y', we need to put this at  h+top-y'
	 */
	float delta_y0;
        delta_y0 = (datapoints[j] - mNiceMinValue)*scaleFac;

	float delta_y1;
	delta_y1 = (prev_datapoints[j] - mNiceMinValue)*scaleFac;
	
	float delta_y2;
	delta_y2 = (prev_prev_datapoints[j] - mNiceMinValue)*scaleFac;
	
	float delta_y3;
	delta_y3 = (prev_prev_prev_datapoints[j] - mNiceMinValue)*scaleFac;

	
        QPainterPath path;
	if(mStackBeams && offset) {
		//we don't want the lines to overdraw each other.  This isn't a great solution though :(
	  if(delta_y0 < 3) delta_y0=3;
	  if(delta_y1 < 3) delta_y1=3;
	  if(delta_y2 < 3) delta_y2=3;
	  if(delta_y3 < 3) delta_y3=3;
	}
	path.moveTo( x0,y0-delta_y0);
        path.cubicTo( x1,y1-delta_y1,x2,y2-delta_y2,x3,y3-delta_y3 );
        
	if(mFillBeams) {
	  QPainterPath path2(path);
          QLinearGradient myGradient(0,(h-1+top),0,(h-1+top)/5);
	  QColor c0(mBeamColorsDark[j]);
	  QColor c1(mBeamColors[j]);
	  c0.setAlpha(150);
	  c1.setAlpha(150);
	  myGradient.setColorAt(0, c0);
          myGradient.setColorAt(1, c1);

          path2.lineTo( x3,y3-offset);
	  if(mStackBeams)
	    path2.cubicTo( x2,y2-offset,x1,y1-offset,x0,y0-offset); //offet is set to 1 after the first beam is drawn, so we don't trample on top of the 2pt thick line
	  else
	    path2.lineTo(x0,y0-1);
          p->setBrush(myGradient);
          p->setPen(Qt::NoPen);
          p->drawPath( path2 );
	}
	p->setBrush(Qt::NoBrush);
	pen.setColor(mBeamColors[j]);
	p->setPen(pen);
        p->drawPath( path );

	if(mStackBeams) {
          //We can draw the beams stacked on top of each other.  This means that say beam 0 has the value 2 and beam
	  // 1 has the value 3, then we plot beam 0 at 2 and beam 1 at 2+3 = 5.
	  y0-=delta_y0;
	  y1-=delta_y1;
	  y2-=delta_y2;
	  y3-=delta_y3;
	  offset = 1;  //see the comment further up for int offset;
	}
      }
      if ( mUseAutoRange && mStackBeams) {
	mMaxValue = qMax(max_y, mMaxValue);
	mMinValue = qMin(min_y, mMinValue);
      }
    }
  }
}
void KSignalPlotter::drawAxisText(QPainter *p, int top, int h)
{
  /* Draw horizontal lines and values. Lines are always drawn.
   * Values are only draw when width is greater than 60 */
  QString val;
  /* top = 0 or  font.height    depending on whether there's a topbar or not
   * h = graphing area.height   - i.e. the actual space we have to draw inside
   *
   * Note we are drawing from 0,0 as the top left corner.  So we have to add on top to get to the top of where we are drawing
   * so top+h is the height of the widget
   */

  p->setPen( mFontColor );
  double stepsize = mNiceRange/(mScaleDownBy*(mHorizontalLinesCount+1));
  int step = (int)ceil((mHorizontalLinesCount+1) * (p->fontMetrics().height() + p->fontMetrics().leading()/2.0) / h);
  if(step ==0) step = 1;
  for ( int y = mHorizontalLinesCount+1; y >= 1; y-= step) {
    int y_coord =  top + (y * (h-1)) / (mHorizontalLinesCount+1);  //Make sure it's y*h first to avoid rounding bugs
    if(y_coord - p->fontMetrics().ascent() < top) continue;  //at most, only allow 4 pixels of the text to be covered up by the top bar.  Otherwise just don't bother to draw it
    double value;
    if((uint)y == mHorizontalLinesCount+1)
        value = mNiceMinValue; //sometimes using the formulas gives us a value very slightly off
    else
        value = mNiceMaxValue/mScaleDownBy - y * stepsize;

    QString number = KGlobal::locale()->formatNumber( value, mPrecision);
    val = QString( "%1 %2" ).arg( number, mUnit );
    p->drawText( 6, y_coord - 3, val );
  }
}

void KSignalPlotter::drawHorizontalLines(QPainter *p, int top, int w, int h)
{
  p->setPen( mHorizontalLinesColor );
  for ( uint y = 0; y <= mHorizontalLinesCount+1; y++ ) {
    //note that the y_coord starts from 0.  so we draw from pixel number 0 to h-1.  Thus the -1 in the y_coord
    int y_coord =  top + (y * (h-1)) / (mHorizontalLinesCount+1);  //Make sure it's y*h first to avoid rounding bugs
    p->drawLine( 0, y_coord, w - 2, y_coord);
  }
}

double KSignalPlotter::lastValue( int i) const
{
  if(mBeamData.isEmpty() || mBeamData.first().size() <= i) return 0;
  return mBeamData.first()[i];
}
QString KSignalPlotter::lastValueAsString( int i) const
{
  if(mBeamData.isEmpty()) return QString();
  double value = mBeamData.first()[i] / mScaleDownBy; //retrieve the newest value for this beam then scale it correct
  QString number = KGlobal::locale()->formatNumber( value, (value >= 100)?0:2);
  return QString( "%1 %2").arg(number, mUnit);
}
#include "SignalPlotter.moc"
