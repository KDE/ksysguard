/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 - 2002 Chris Schlaeger <cs@kde.org>
    Copyright (c) 2006 John Tapsell <tapsell@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License version 2 or at your option version 3 as published by
    the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <math.h>  //For floor, ceil, log10 etc for calculating ranges

#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QPainterPath>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kapplication.h>

#include "SignalPlotter.h"

#ifdef SVG_SUPPORT
#include <plasma/svg.h>
#endif


#define VERTICAL_LINE_OFFSET 1


#ifdef SVG_SUPPORT
QHash<QString, Plasma::Svg *> KSignalPlotter::sSvgRenderer ;
#endif

KSignalPlotter::KSignalPlotter( SensorDataProvider* argDataProvider, QWidget *parent)
  : QWidget( parent)
{
  mPrecision = 0;
  mSamples = 0;
  mMinValue = mMaxValue = 0.0;
  mNiceMinValue = mNiceMaxValue = 0.0;
  mNiceRange = 0;
  mUseAutoRange = true;
  mScaleDownBy = 1;
  mShowThinFrame = true;
  mSmoothGraph = true;
  dataProvider = argDataProvider;

  // Anything smaller than this does not make sense.
  setMinimumSize( 16, 16 );
  QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  sizePolicy.setHeightForWidth(false);
  setSizePolicy( sizePolicy );

  mShowVerticalLines = true;
  mVerticalLinesColor = QColor(0xC3,0xC3,0xC3);
  mVerticalLinesDistance = 30;
  mVerticalLinesScroll = true;
  mVerticalLinesOffset = 0;
  mHorizontalScale = 1;

  mShowHorizontalLines = true;
  mHorizontalLinesColor = QColor(0xC3, 0xC3, 0xC3);
  mHorizontalLinesCount = 4;

  mShowAxis = true;

  mBackgroundColor = QColor(0,0,0);
  mAxisTextWidth = 0;
  mScrollOffset = 0;
  mFillOpacity = 20;
  mRescaleTime = 0;
}

KSignalPlotter::~KSignalPlotter()
{
}

KLocalizedString KSignalPlotter::unit() const {
  return mUnit;
}
void KSignalPlotter::setUnit(const KLocalizedString &unit) {
  mUnit= unit;
}

void KSignalPlotter::recalculateMaxMinValueForSample(const FancyPlotterSensor* sensor, int time )
{

  double sensorMaxValue = sensor->maxValue();
  double sensorMinValue = sensor->minValue();

  if(mMaxValue < sensorMaxValue)  {
	  mMaxValue = sensorMaxValue;
  }
  if(mMinValue > sensorMinValue) mMinValue = sensorMinValue;
  if(sensorMaxValue > 0.7*mMaxValue)
  	  mRescaleTime = time;
}

void KSignalPlotter::rescale() {
  mMaxValue = mMinValue = 0;
  for(int i = dataProvider->sensorCount()-1; i >= 0; i--) {
    recalculateMaxMinValueForSample(static_cast<FancyPlotterSensor*>(dataProvider->sensor(i)), i);
  }
}


void KSignalPlotter::updatePlot()
{

  int listSize = dataProvider->sensorCount();
  for (int var = 0; var < listSize; ++var) {
	  FancyPlotterSensor* sensor = static_cast<FancyPlotterSensor*>(dataProvider->sensor(var));
	  //there is a cost to removing old values so wait and remove a bunch at once
	  if ((uint)sensor->dataSize() > mSamples+256)
		  sensor->removeOldestValue(sensor->dataSize()-mSamples);
	  if(mUseAutoRange) {
		 recalculateMaxMinValueForSample(sensor, 0);
		 if(mRescaleTime++ > mSamples)
		  rescale();
	  }
  }

  // If the vertical lines are scrolling, increment the offset
  // so they move with the data.
  if ( mVerticalLinesScroll ) {
    mVerticalLinesOffset = ( mVerticalLinesOffset + mHorizontalScale)
                           % mVerticalLinesDistance;
  }
  drawAllBeamsToScrollableImage();
  update(mPlottingArea);
}

void KSignalPlotter::changeRange( double min, double max )
{
  mMinValue = min;
  mMaxValue = max;
  calculateNiceRange();
}



void KSignalPlotter::setScaleDownBy( double value )
{
  if(mScaleDownBy == value) return;
  mScaleDownBy = value;
  mBackgroundImage = QPixmap(); //we changed a paint setting, so reset the cache
  calculateNiceRange();
  update();
}
double KSignalPlotter::scaleDownBy() const { return mScaleDownBy; }

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
  mBackgroundImage = QPixmap(); //we changed a paint setting, so reset the cache
  update();
}

int KSignalPlotter::horizontalScale() const
{
  return mHorizontalScale;
}

void KSignalPlotter::setShowVerticalLines( bool value )
{
  if(mShowVerticalLines == value) return;
  mShowVerticalLines = value;
  mBackgroundImage = QPixmap(); //we changed a paint setting, so reset the cache
#ifdef USE_QIMAGE
  mScrollableImage = QImage();
#else
  mScrollableImage = QPixmap();
#endif
}

bool KSignalPlotter::showVerticalLines() const
{
  return mShowVerticalLines;
}

void KSignalPlotter::setVerticalLinesColor( const QColor &color )
{
  if(mVerticalLinesColor == color) return;
  if(!color.isValid()) {
	  kDebug(1215) << "Invalid color";
	  return;
  }

  mVerticalLinesColor = color;
  mBackgroundImage = QPixmap(); //we changed a paint setting, so reset the cache
#ifdef USE_QIMAGE
  mScrollableImage = QImage();
#else
  mScrollableImage = QPixmap();
#endif
}

QColor KSignalPlotter::verticalLinesColor() const
{
  return mVerticalLinesColor;
}

void KSignalPlotter::setVerticalLinesDistance( uint distance )
{
  if(distance == mVerticalLinesDistance) return;
  mVerticalLinesDistance = distance;
  mBackgroundImage = QPixmap(); //we changed a paint setting, so reset the cache
#ifdef USE_QIMAGE
  mScrollableImage = QImage();
#else
  mScrollableImage = QPixmap();
#endif

}

int KSignalPlotter::verticalLinesDistance() const
{
  return mVerticalLinesDistance;
}

void KSignalPlotter::setVerticalLinesScroll( bool value )
{
  if(value == mVerticalLinesScroll) return;
  mVerticalLinesScroll = value;
  mBackgroundImage = QPixmap(); //we changed a paint setting, so reset the cache
#ifdef USE_QIMAGE
  mScrollableImage = QImage();
#else
  mScrollableImage = QPixmap();
#endif

}

bool KSignalPlotter::verticalLinesScroll() const
{
  return mVerticalLinesScroll;
}

void KSignalPlotter::setShowHorizontalLines( bool value )
{
  if(value == mShowHorizontalLines) return;
  mShowHorizontalLines = value;
  mBackgroundImage = QPixmap(); //we changed a paint setting, so reset the cache
}

bool KSignalPlotter::showHorizontalLines() const
{
  return mShowHorizontalLines;
}
void KSignalPlotter::setAxisFontColor( const QColor &color )
{
  if(!color.isValid()) {
	  kDebug(1215) << "Invalid color";
	  return;
  }

  mFontColor = color;
}

QColor KSignalPlotter::axisFontColor() const
{
  return mFontColor;
}


void KSignalPlotter::setHorizontalLinesColor( const QColor &color )
{
  if(!color.isValid()) {
	  kDebug(1215) << "Invalid color";
	  return;
  }

  if(color == mHorizontalLinesColor) return;
  mHorizontalLinesColor = color;
  mBackgroundImage = QPixmap(); //we changed a paint setting, so reset the cache
}

QColor KSignalPlotter::horizontalLinesColor() const
{
  return mHorizontalLinesColor;
}

void KSignalPlotter::setShowAxis( bool value )
{
  if(value == mShowAxis) return;
  mShowAxis = value;
  mBackgroundImage = QPixmap(); //we changed a paint setting, so reset the cache
#ifdef USE_QIMAGE
  mScrollableImage = QImage();
#else
  mScrollableImage = QPixmap();
#endif
}

bool KSignalPlotter::showAxis() const
{
  return mShowAxis;
}

void KSignalPlotter::setAxisFont( const QFont &font )
{
  mFont = font;
  mBackgroundImage = QPixmap(); //we changed a paint setting, so reset the cache
#ifdef USE_QIMAGE
  mScrollableImage = QImage();
#else
  mScrollableImage = QPixmap();
#endif

}

QFont KSignalPlotter::axisFont() const
{
  return mFont;
}
QString KSignalPlotter::svgBackground() const {
  return mSvgFilename;
}
void KSignalPlotter::setSvgBackground( const QString &filename )
{
  if(mSvgFilename == filename) return;
  mSvgFilename = filename;
  mBackgroundImage = QPixmap();
}

void KSignalPlotter::setBackgroundColor( const QColor &color )
{
  if(color == mBackgroundColor) return;
  if(!color.isValid()) {
	  kDebug(1215) << "Invalid color";
	  return;
  }
  mBackgroundColor = color;
  mBackgroundImage = QPixmap(); //we changed a paint setting, so reset the cache
}

QColor KSignalPlotter::backgroundColor() const
{
  return mBackgroundColor;
}

void KSignalPlotter::setThinFrame( bool set)
{
  if(mShowThinFrame == set) return;
  mShowThinFrame = set;
  mBackgroundImage = QPixmap(); //we changed a paint setting, so reset the cache
}
bool KSignalPlotter::thinFrame() const
{
  return mShowThinFrame;
}
void KSignalPlotter::resizeEvent( QResizeEvent* )
{
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

void KSignalPlotter::paintEvent( QPaintEvent* event)
{
  uint w = width();
  uint h = height();
  /* Do not do repaints when the widget is not yet setup properly. */
  if ( w <= 2 )
    return;
  QPainter p(this);

  if(event && mPlottingArea.contains(event->rect()))
    drawWidget(&p, QRect(0,0,w, h), true);  // do not bother drawing axis text etc.
  else
    drawWidget(&p, QRect(0,0,w, h), false);
}

void KSignalPlotter::drawWidget(QPainter *p, QRect boundingBox, bool onlyDrawPlotter)
{
  if(boundingBox.height() <= 2 || boundingBox.width() <= 2 ) return;
  p->setFont( mFont );
  int fontheight = (p->fontMetrics().height() + p->fontMetrics().leading()/2.0);
  mHorizontalLinesCount = qBound(0,(int)(boundingBox.height() / fontheight)-2, 4);

  if(!onlyDrawPlotter) {
    if(mMinValue < mNiceMinValue || mMaxValue > mNiceMaxValue || (mNiceRange != 1 && mMaxValue < (mNiceRange*0.75 + mNiceMinValue)) || mNiceRange == 0) {
      calculateNiceRange();
    }
    QPen pen;
    pen.setWidth(1);
    pen.setCapStyle(Qt::RoundCap);
    p->setPen(pen);

    boundingBox.setTop(p->pen().width() / 2); //The y position of the top of the graph.  Basically this is one more than the height of the top bar

    //check if there's enough room to actually show a top bar. Must be enough room for a bar at the top, plus horizontal lines each of a size with room for a scale
    if( mShowAxis && boundingBox.width() > 60 && boundingBox.height() > ( fontheight + 1 ) ) {  //if there's room to draw the labels, then draw them!
      //We want to adjust the size of plotter bit inside so that the axis text aligns nicely at the top and bottom
      //but we don't want to sacrifice too much of the available room, so don't use it if it will take more than 20% of the available space
      qreal offset = (p->fontMetrics().height()+1)/2;
      drawAxisText(p, boundingBox.adjusted(0,offset,0,-offset));
      if(offset < boundingBox.height() * 0.1)
          boundingBox.adjust(0,offset, 0, -offset);
    }
    if( mShowAxis ) {
      if ( kapp->layoutDirection() == Qt::RightToLeft )
        boundingBox.setRight(boundingBox.right() - mAxisTextWidth - 10);
      else
        boundingBox.setLeft(mAxisTextWidth+10);
    }

    // Remember bounding box to pass to update, so that we only update the plotting area
    mPlottingArea = boundingBox;
  } else {
    boundingBox = mPlottingArea;
  }

  if(boundingBox.height() <= 2 || boundingBox.width() <= 2 ) return;

  if(mBackgroundImage.isNull() || mBackgroundImage.height() != boundingBox.height() || mBackgroundImage.width() != boundingBox.width()) { //recreate on resize etc

    mBackgroundImage = QPixmap(boundingBox.width(), boundingBox.height());
    Q_ASSERT(!mBackgroundImage.isNull());
    QPainter pCache(&mBackgroundImage);
    pCache.setRenderHint(QPainter::Antialiasing, false);
    pCache.setFont( mFont );
    //To paint to the cache, we need a new bounding box
    QRect cacheBoundingBox = QRect(0,0,boundingBox.width(), boundingBox.height());

    drawBackground(&pCache, cacheBoundingBox);
    if(mShowThinFrame) {
      drawThinFrame(&pCache, cacheBoundingBox);
      //We have a 'frame' in the bottom and right - so subtract them from the view
      cacheBoundingBox.adjust(0,0,-1,-1);
      pCache.setClipRect( cacheBoundingBox );
    }


    /* Draw scope-like grid vertical lines if it doesn't move.  If it does move, draw it in the dynamic part of the code*/
    if(!mVerticalLinesScroll && mShowVerticalLines && cacheBoundingBox.width() > 60)
      drawVerticalLines(&pCache, cacheBoundingBox);
    if ( mShowHorizontalLines )
      drawHorizontalLines(&pCache, cacheBoundingBox.adjusted(-3,0,0,0));
  }
  p->drawPixmap(boundingBox, mBackgroundImage);

  if(mShowThinFrame) {
    //We have a 'frame' in the bottom and right - so subtract them from the view
    boundingBox.adjust(0,0,-1,-1);
  }
  if(boundingBox.height() == 0 || boundingBox.width() == 0) return;
  p->setClipRect( boundingBox);

  bool redraw = false;
  //Align width of bounding box to the size of the horizontal scale and to the size of the vertical lines distance
  int alignedWidth;
  if(mVerticalLinesDistance) {
    if(mVerticalLinesDistance % mHorizontalScale == 0)
      alignedWidth = ((boundingBox.width() -1) / mVerticalLinesDistance + 1) * mVerticalLinesDistance;
    else // We should find the lowest common denomiator of mVerticalLinesDistance and mHorizontalScale, but this is close enough..
      alignedWidth = ((boundingBox.width() -1) / mVerticalLinesDistance + 1) * mVerticalLinesDistance * mHorizontalScale;
  }
  else
    alignedWidth = ((boundingBox.width() -1) / mHorizontalScale + 1) * mHorizontalScale;

  if(mScrollableImage.isNull() || mScrollableImage.height() != boundingBox.height() || mScrollableImage.width() != alignedWidth) {
#ifdef USE_QIMAGE
    mScrollableImage = QImage(alignedWidth, boundingBox.height(),QImage::Format_ARGB32_Premultiplied);
    mScrollableImage.fill(0);
#else
    mScrollableImage = QPixmap(alignedWidth, boundingBox.height());
    mScrollableImage.fill(QColormap::instance().pixel(QColor(Qt::transparent)));
#endif
    Q_ASSERT(!mScrollableImage.isNull());
    redraw = true;
  }

  if(redraw) {
      //Redraw the whole thing
      /* Draw scope-like grid vertical lines */
      mScrollOffset = 0;
      drawAllBeamsToScrollableImage();
      if(mVerticalLinesScroll && mShowVerticalLines) {
        QPainter pCache(&mScrollableImage);
        pCache.setRenderHint(QPainter::Antialiasing, true);
        int x = mScrollOffset - (mScrollOffset % mVerticalLinesDistance) + mVerticalLinesDistance;
        for(;x < mScrollableImage.width(); x+= mVerticalLinesDistance) {
          pCache.setPen( mVerticalLinesColor );
          pCache.drawLine( x + VERTICAL_LINE_OFFSET, 0, x + VERTICAL_LINE_OFFSET, mScrollableImage.height()-1);
        }
      }
  }


  //p->drawImage(boundingBox.left(), boundingBox.top(), mScrollableImage, 0, 0,0,0);

  //We draw the pixmap in two halves, wrapping around the window
  if(mScrollOffset != 0)
    p->drawImage(boundingBox.right() - mScrollOffset+1, boundingBox.top(), mScrollableImage, 0, 0, mScrollOffset, boundingBox.height());
  if(boundingBox.width() - mScrollOffset != 0) {
    int widthOfSecondHalf = boundingBox.width() - mScrollOffset+1;
#ifdef USE_QIMAGE
    p->drawImage(boundingBox.left(), boundingBox.top(), mScrollableImage, mScrollableImage.width() - widthOfSecondHalf, 0, widthOfSecondHalf, boundingBox.height());
#else
   p->drawPixmap(boundingBox.left(), boundingBox.top(), mScrollableImage, mScrollableImage.width() - widthOfSecondHalf, 0, widthOfSecondHalf, boundingBox.height());
#endif
  }
}
void KSignalPlotter::drawBackground(QPainter *p, const QRect &boundingBox)
{
  if (testAttribute(Qt::WA_PendingResizeEvent)) {
    return; // lets not do this more than necessary, shall we?
  }

  p->fillRect(boundingBox, mBackgroundColor);

  if(mSvgFilename.isEmpty())
    return; //nothing to draw, return

#ifdef SVG_SUPPORT
  Plasma::Svg *svgRenderer;
  if(!sSvgRenderer.contains(mSvgFilename)) {
    svgRenderer = new Plasma::Svg(this);
    svgRenderer->setImagePath(mSvgFilename);
    sSvgRenderer.insert(mSvgFilename, svgRenderer);
  } else {
    svgRenderer = sSvgRenderer[mSvgFilename];
  }

  svgRenderer->resize(boundingBox.width(), boundingBox.height());
  svgRenderer->paint(p, 0, 0);
#endif
}

void KSignalPlotter::drawThinFrame(QPainter *p, const QRect &boundingBox)
{
  /* Draw white line along the bottom and the right side of the
   * widget to create a 3D like look. */
  p->setPen( palette().color( QPalette::Light ) );
  p->drawLine( boundingBox.bottomLeft(), boundingBox.bottomRight());
  p->drawLine( boundingBox.bottomRight(), boundingBox.topRight());
}

void KSignalPlotter::calculateNiceRange()
{
  double newNiceRange = mMaxValue - mMinValue;
  /* If the range is too small we will force it to 1.0 since it
   * looks a lot nicer. */
  if ( newNiceRange < 0.000001 )
    newNiceRange = 1.0;

  double newNiceMinValue = mMinValue;
    if ( mMinValue != 0.0 ) {
      double dim = pow( 10, floor( log10( fabs( mMinValue ) ) ) ) / 2;
      if ( mMinValue < 0.0 )
        newNiceMinValue = dim * floor( mMinValue / dim );
      else
        newNiceMinValue = dim * ceil( mMinValue / dim );
      newNiceRange = mMaxValue - newNiceMinValue;
      if ( newNiceRange < 0.000001 )
        newNiceRange = 1.0;
    }
    // Massage the range so that the grid shows some nice values.
    double step = newNiceRange / (mScaleDownBy*(mHorizontalLinesCount+1));
    int logdim = (int)floor( log10( step ) );
    double dim = pow( (double)10.0, logdim ) / 2;
    int a = (int)ceil( step / dim );
    if(logdim >= 0)
        mPrecision = 0;
    else if( a % 2 == 0){
        mPrecision =-logdim;
    } else {
	mPrecision = 1-logdim;
    }
    newNiceRange = mScaleDownBy*dim * a * (mHorizontalLinesCount+1);
  if( mNiceMinValue == newNiceMinValue && mNiceRange == newNiceRange)
    return;  //nothing changed
  mNiceMaxValue = newNiceMinValue + newNiceRange;
  mNiceMinValue = newNiceMinValue;
  mNiceRange = newNiceRange;

#ifdef USE_QIMAGE
  mScrollableImage = QImage();
#else
  mScrollableImage = QPixmap();
#endif
  emit axisScaleChanged();
}


void KSignalPlotter::drawVerticalLines(QPainter *p, const QRect &boundingBox, int offset)
{
  p->setPen( mVerticalLinesColor );

  for ( int x = boundingBox.right() - ( (mVerticalLinesOffset + offset) % mVerticalLinesDistance); x >= boundingBox.left(); x -= mVerticalLinesDistance )
      p->drawLine( x, boundingBox.top(), x, boundingBox.bottom()  );
}

void KSignalPlotter::drawAllBeamsToScrollableImage()
{
  if(mScrollableImage.isNull())
    return;
  QRect cacheBoundingBox = QRect(mScrollOffset, 0, mHorizontalScale, mScrollableImage.height());

  QPainter pCache(&mScrollableImage);
  pCache.setRenderHint(QPainter::Antialiasing, true);
  //To paint to the cache, we need a new bounding box

  pCache.setCompositionMode(QPainter::CompositionMode_Clear);
  pCache.fillRect(cacheBoundingBox, Qt::transparent);

  drawAllBeams(&pCache, cacheBoundingBox, mHorizontalScale);


  if ( mVerticalLinesScroll && mShowVerticalLines ) {
    if( mScrollOffset % mVerticalLinesDistance == 0) {
      pCache.setPen( mVerticalLinesColor );
      pCache.setCompositionMode(QPainter::CompositionMode_DestinationOver);
      pCache.drawLine( mScrollOffset+VERTICAL_LINE_OFFSET, cacheBoundingBox.top(), mScrollOffset+VERTICAL_LINE_OFFSET, cacheBoundingBox.bottom()  );
    }
  }

  mScrollOffset += mHorizontalScale;
  if(mScrollOffset >= mScrollableImage.width()-1) {
    mScrollOffset = 0;
  }
}

void KSignalPlotter::drawAllBeams(QPainter *p, const QRect &boundingBox, int horizontalScale)
{

  QPen pen;
  QColor drawingColor;
  pen.setWidth(2);
  pen.setCapStyle(Qt::FlatCap);
  p->setCompositionMode(QPainter::CompositionMode_SourceOver);




  const double scaleFac = (boundingBox.height()-2) / mNiceRange;
  const int bottom = boundingBox.bottom();
  const float horizontalOverThree = horizontalScale/3.0;
  const float twoTimesHorizontalOverThree = 2*horizontalOverThree;

  float x0 = boundingBox.right();
  float x1 = x0 - horizontalScale;
  float y0 = 0;
  float y1 = 0;
  float y2 = 0;
  double lastSeenValue = 0;
  double lastValue = 0;



  int listSize = dataProvider->sensorCount();
  //we draw all the beam in one pass
  for (int i = 0; i < listSize; ++i)  {
		FancyPlotterSensor* sensor = static_cast<FancyPlotterSensor*>(dataProvider->sensor(i));
		if (sensor->dataSize() > 0) {
			lastSeenValue = sensor->lastSeenValue();
			lastValue = sensor->lastValue();
			QPainterPath path;

			y0 = bottom - (lastValue - mNiceMinValue) * scaleFac;
			y1 = bottom - (lastSeenValue - mNiceMinValue) * scaleFac;
			y2 = bottom - (sensor->prevSeenValue() - mNiceMinValue) * scaleFac;


			if (mSmoothGraph) {
				// Apply a weighted average just to smooth the graph out a bit
				y0 = (2* y0 + y1) / 3;
				y1 = (2* y1 + y2) / 3;
				// We don't bother to average out y2.  This will introduce slight inaccuracies in the gradients, but they aren't really noticeable.
			}
			path.moveTo(x1, y1);

			//draw a curve if the last drawn value is not the same as the current value, otherwise just draw a line
			if (lastValue != lastSeenValue)  {
				QPointF c1(x1 + horizontalOverThree, (4* y1 - y2) / 3.0);//Control point 1 - same gradient as prev_prev_datapoint to prev_datapoint
				QPointF c2(x1 + twoTimesHorizontalOverThree, (2* y0 + y1) / 3.0);//Control point 2 - same gradient as prev_datapoint to datapoint
				path.cubicTo(c1, c2, QPointF(x0, y0));
			} else {
				path.lineTo(x0,y0);
			}

			sensor->updateLastSeenValue(lastValue);

			drawingColor = sensor->color();
			if (mFillOpacity)
				drawingColor = sensor->lighterColor();
			pen.setColor(drawingColor);
			p->setPen(pen);
			p->drawPath(path);
			if (mFillOpacity) {
				path.lineTo(x0, bottom);
				path.lineTo(x1, bottom);
				path.lineTo(x1, y1);
				drawingColor = sensor->color();
				drawingColor.setAlpha(mFillOpacity);
				p->fillPath(path, drawingColor);
			}

		}



	}
}

void KSignalPlotter::setMaxAxisTextWidth(int axisTextWidth)
{
  mAxisTextWidth = axisTextWidth;
}

int KSignalPlotter::maxAxisTextWidth() const
{
  return mAxisTextWidth;
}

void KSignalPlotter::drawAxisText(QPainter *p, const QRect &boundingBox)
{
  if(mHorizontalLinesCount < 0) return;
  double stepsize = mNiceRange/(mScaleDownBy*(mHorizontalLinesCount+1));
  p->setPen(mFontColor);
  int axisTitleIndex=1;
  QString val;
  for ( int y = 0; y < mHorizontalLinesCount +2; y++, axisTitleIndex++) {
    int y_coord = boundingBox.top() + (y * (boundingBox.height()-1)) /(mHorizontalLinesCount+1);  //Make sure it's y*h first to avoid rounding bugs

    double value;
    if(y == mHorizontalLinesCount+1)
        value = mNiceMinValue; //sometimes using the formulas gives us a value very slightly off
    else
        value = mNiceMaxValue/mScaleDownBy - y * stepsize;

    QString number = KGlobal::locale()->formatNumber( value, mPrecision);
    val = mUnit.subs(number).toString();
    if ( kapp->layoutDirection() == Qt::RightToLeft )
      p->drawText( boundingBox.right()-mAxisTextWidth, y_coord - 1000, mAxisTextWidth, 2000, Qt::AlignRight | Qt::AlignVCenter, val);
    else
      p->drawText( boundingBox.left(), y_coord - 1000, mAxisTextWidth, 2000, Qt::AlignRight | Qt::AlignVCenter, val);
  }
}

void KSignalPlotter::drawHorizontalLines(QPainter *p, const QRect &boundingBox)
{
  if(mHorizontalLinesCount <= 0) return;
  p->setPen( QPen(mHorizontalLinesColor, 0, Qt::DashLine));
  for ( int y = 0; y <= mHorizontalLinesCount+1; y++ ) {
    //note that the y_coord starts from 0.  so we draw from pixel number 0 to h-1.  Thus the -1 in the y_coord
    int y_coord =  boundingBox.top() + (y * (boundingBox.height()-1)) / (mHorizontalLinesCount+1);  //Make sure it's y*h first to avoid rounding bugs
    p->drawLine( boundingBox.left(), y_coord, boundingBox.right() - 1, y_coord);
  }
}

bool KSignalPlotter::smoothGraph() const
{
  return mSmoothGraph;
}

void KSignalPlotter::setSmoothGraph(bool smooth)
{
  mSmoothGraph = smooth;
#ifdef USE_QIMAGE
  mScrollableImage = QImage();
#else
  mScrollableImage = QPixmap();
#endif
}

int KSignalPlotter::fillOpacity() const
{
  return mFillOpacity;
}
void KSignalPlotter::setFillOpacity(int fill)
{
  mFillOpacity = fill;
#ifdef USE_QIMAGE
  mScrollableImage = QImage();
#else
  mScrollableImage = QPixmap();
#endif

}

#include <SignalPlotter.moc>

