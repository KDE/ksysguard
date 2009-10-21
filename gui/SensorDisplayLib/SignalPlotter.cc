/*
    This file is part of the KDE project

    Copyright (c) 2006 - 2009 John Tapsell <tapsell@kde.org>

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
#include <QtGui/QPaintEvent>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kapplication.h>

#include "SignalPlotter.h"
#include "SignalPlotter_p.h"

#ifdef SVG_SUPPORT
#include <plasma/svg.h>
#endif


#define VERTICAL_LINE_OFFSET 1


#ifdef SVG_SUPPORT
QHash<QString, Plasma::Svg *> KSignalPlotter::sSvgRenderer ;
#endif

KSignalPlotter::KSignalPlotter( QWidget *parent)
  : QWidget( parent), d(new KSignalPlotterPrivate(this))
{
    // Anything smaller than this does not make sense.
    setMinimumSize( 16, 16 );
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy.setHeightForWidth(false);
    setSizePolicy( sizePolicy );
}

KSignalPlotterPrivate::KSignalPlotterPrivate(KSignalPlotter * q_ptr) : q(q_ptr)
{
    mPrecision = 0;
    mMaxSamples = 4; //Never keep less than 4 samples. An arbitrary number
    mMinValue = mMaxValue = 0.0;
    mUserMinValue = mUserMaxValue = 0.0;
    mNiceMinValue = mNiceMaxValue = 0.0;
    mNiceRange = 0;
    mUseAutoRange = true;
    mScaleDownBy = 1;
    mShowThinFrame = true;
    mSmoothGraph = true;
    mShowVerticalLines = false;
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
    mStackBeams = false;
    mFillOpacity = 20;
    mRescaleTime = 0;
}

KSignalPlotter::~KSignalPlotter()
{
}

KLocalizedString KSignalPlotter::unit() const {
    return d->mUnit;
}
void KSignalPlotter::setUnit(const KLocalizedString &unit) {
    d->mUnit = unit;
}

void KSignalPlotter::addBeam( const QColor &color )
{
    QList< QList<double> >::Iterator it;
    //When we add a new beam, go back and set the data for this beam to 0 for all the other times. This is because it makes it easier for
    //moveSensors
    for(it = d->mBeamData.begin(); it != d->mBeamData.end(); ++it) {
        (*it).append(0);
    }
    d->mBeamColors.append(color);
    d->mBeamColorsDark.append(color.darker(150));
}

QColor KSignalPlotter::beamColor( int index ) const {
    return d->mBeamColors[ index ];
}

void KSignalPlotter::setBeamColor( int index, const QColor &color ) {
    if(!color.isValid()) {
        kDebug(1215) << "Invalid color";
        return;
    }

    d->mBeamColors[ index ] = color;
    d->mBeamColorsDark[ index ] = color.darker(150);
}

int KSignalPlotter::numBeams() const {
    return d->mBeamColors.count();
}

void KSignalPlotterPrivate::recalculateMaxMinValueForSample(const QList<double>&sampleBuf, int time )
{
    if(mStackBeams) {
        double value=0;
        for(int i = sampleBuf.count()-1; i>= 0; i--) {
            value += sampleBuf[i];
        }
        if(mMinValue > value) mMinValue = value;
        if(mMaxValue < value) mMaxValue = value;
        if(value > 0.7*mMaxValue)
            mRescaleTime = time;
    } else {
        double value;
        for(int i = sampleBuf.count()-1; i>= 0; i--) {
            value = sampleBuf[i];
            if(mMinValue > value) mMinValue = value;
            if(mMaxValue < value) mMaxValue = value;
            if(value > 0.7*mMaxValue)
                mRescaleTime = time;
        }
    }
}

void KSignalPlotterPrivate::rescale() {
    mMaxValue = mMinValue = 0;
    for(int i = mBeamData.count()-1; i >= 0; i--) {
        recalculateMaxMinValueForSample(mBeamData[i], i);
    }
    calculateNiceRange();
}

void KSignalPlotter::addSample( const QList<double>& sampleBuf )
{
    d->addSample(sampleBuf);
    update(d->mPlottingArea);
}
void KSignalPlotterPrivate::addSample( const QList<double>& sampleBuf )
{
    if(sampleBuf.count() != mBeamColors.count()) {
        kDebug(1215) << "Sample data discarded - contains wrong number of beams";
        return;
    }
    mBeamData.prepend(sampleBuf);
    if((unsigned int)mBeamData.size() > mMaxSamples) {
        mBeamData.removeLast(); // we have too many.  Remove the last item
        if((unsigned int)mBeamData.size() > mMaxSamples)
            mBeamData.removeLast(); // If we still have too many, then we have resized the widget.  Remove one more.  That way we will slowly resize to the new size
    }

    if(mUseAutoRange) {
        recalculateMaxMinValueForSample(sampleBuf, 0);

        if(mRescaleTime++ > mMaxSamples)
            rescale();
        else if(mMinValue < mNiceMinValue || mMaxValue > mNiceMaxValue || (mMaxValue > mUserMaxValue && mNiceRange != 1 && mMaxValue < (mNiceRange*0.75 + mNiceMinValue)) || mNiceRange == 0)
            calculateNiceRange();
    } else {
        if(mMinValue < mNiceMinValue || mMaxValue > mNiceMaxValue || (mMaxValue > mUserMaxValue && mNiceRange != 1 && mMaxValue < (mNiceRange*0.75 + mNiceMinValue)) || mNiceRange == 0)
            calculateNiceRange();
    }

    /* If the vertical lines are scrolling, increment the offset
     * so they move with the data. */
    if ( mVerticalLinesScroll ) {
        mVerticalLinesOffset = ( mVerticalLinesOffset + mHorizontalScale)
            % mVerticalLinesDistance;
    }
    drawBeamToScrollableImage(0);
}

void KSignalPlotter::reorderBeams( const QList<int>& newOrder )
{
    if(newOrder.count() != d->mBeamColors.count()) {
        return;
    }
    QList< QList<double> >::Iterator it;
    for(it = d->mBeamData.begin(); it != d->mBeamData.end(); ++it) {
        if(newOrder.count() != (*it).count()) {
            kWarning(1215) << "Serious problem in move sample.  beamdata[i] has " << (*it).count() << " and neworder has " << newOrder.count();
        } else {
            QList<double> newBeam;
            for(int i = 0; i < newOrder.count(); i++) {
                int newIndex = newOrder[i];
                newBeam.append((*it).at(newIndex));
            }
            (*it) = newBeam;
        }
    }
    QList< QColor > newBeamColors;
    QList< QColor > newBeamColorsDark;
    for(int i = 0; i < newOrder.count(); i++) {
        int newIndex = newOrder[i];
        newBeamColors.append(d->mBeamColors.at(newIndex));
        newBeamColorsDark.append(d->mBeamColorsDark.at(newIndex));
    }
    d->mBeamColors = newBeamColors;
    d->mBeamColorsDark = newBeamColorsDark;
}


void KSignalPlotter::changeRange( double min, double max )
{
    if( min == d->mUserMinValue && max == d->mUserMaxValue ) return;
    d->mUserMinValue = min;
    d->mUserMaxValue = max;
    d->calculateNiceRange();
}

void KSignalPlotter::removeBeam( int index )
{
    if(index >= d->mBeamColors.size()) return;
    if(index >= d->mBeamColorsDark.size()) return;
    d->mBeamColors.removeAt( index );
    d->mBeamColorsDark.removeAt(index);

    QList< QList<double> >::Iterator i;
    for(i = d->mBeamData.begin(); i != d->mBeamData.end(); ++i) {
        if( (*i).size() >= index)
            (*i).removeAt(index);
    }
    if(d->mUseAutoRange)
        d->rescale();
}

void KSignalPlotter::setScaleDownBy( double value )
{
    if(d->mScaleDownBy == value) return;
    d->mScaleDownBy = value;
    d->mBackgroundImage = QPixmap(); //we changed a paint setting, so reset the cache
    d->calculateNiceRange();
    update();
}
double KSignalPlotter::scaleDownBy() const
{
    return d->mScaleDownBy;
}

void KSignalPlotter::setUseAutoRange( bool value )
{
    d->mUseAutoRange = value;
    d->calculateNiceRange();
    //this change will be detected in paint and the image cache regenerated
}

bool KSignalPlotter::useAutoRange() const
{
    return d->mUseAutoRange;
}

void KSignalPlotter::setMinimumValue( double min )
{
    if(min == d->mUserMinValue) return;
    d->mUserMinValue = min;
    d->calculateNiceRange();
    //this change will be detected in paint and the image cache regenerated
}

double KSignalPlotter::minimumValue() const
{
    return d->mUserMinValue;
}

void KSignalPlotter::setMaximumValue( double max )
{
    if(max == d->mUserMaxValue) return;
    d->mUserMaxValue = max;
    d->calculateNiceRange();
    //this change will be detected in paint and the image cache regenerated
}

double KSignalPlotter::maximumValue() const
{
    return d->mUserMaxValue;
}

double KSignalPlotter::currentMaximumRangeValue() const
{
    return d->mNiceMaxValue;
}

double KSignalPlotter::currentMinimumRangeValue() const
{
    return d->mNiceMinValue;
}

void KSignalPlotter::setHorizontalScale( uint scale )
{
    if (scale == d->mHorizontalScale)
        return;

    d->mHorizontalScale = scale;
    d->updateDataBuffers();
    d->mBackgroundImage = QPixmap(); //we changed a paint setting, so reset the cache
    update();
}

int KSignalPlotter::horizontalScale() const
{
    return d->mHorizontalScale;
}

void KSignalPlotter::setShowVerticalLines( bool value )
{
    if(d->mShowVerticalLines == value) return;
    d->mShowVerticalLines = value;
    d->mBackgroundImage = QPixmap(); //we changed a paint setting, so reset the cache
#ifdef USE_QIMAGE
    d->mScrollableImage = QImage();
#else
    d->mScrollableImage = QPixmap();
#endif
}

bool KSignalPlotter::showVerticalLines() const
{
    return d->mShowVerticalLines;
}

void KSignalPlotter::setVerticalLinesColor( const QColor &color )
{
    if(d->mVerticalLinesColor == color) return;
    if(!color.isValid()) {
        kWarning(1215) << "Invalid color";
        return;
    }

    d->mVerticalLinesColor = color;
    d->mBackgroundImage = QPixmap(); //we changed a paint setting, so reset the cache
#ifdef USE_QIMAGE
    d->mScrollableImage = QImage();
#else
    d->mScrollableImage = QPixmap();
#endif
}

QColor KSignalPlotter::verticalLinesColor() const
{
    return d->mVerticalLinesColor;
}

void KSignalPlotter::setVerticalLinesDistance( uint distance )
{
    if(distance == d->mVerticalLinesDistance) return;
    d->mVerticalLinesDistance = distance;
    d->mBackgroundImage = QPixmap(); //we changed a paint setting, so reset the cache
#ifdef USE_QIMAGE
    d->mScrollableImage = QImage();
#else
    d->mScrollableImage = QPixmap();
#endif

}

uint KSignalPlotter::verticalLinesDistance() const
{
    return d->mVerticalLinesDistance;
}

void KSignalPlotter::setVerticalLinesScroll( bool value )
{
    if(value == d->mVerticalLinesScroll) return;
    d->mVerticalLinesScroll = value;
    d->mBackgroundImage = QPixmap(); //we changed a paint setting, so reset the cache
#ifdef USE_QIMAGE
    d->mScrollableImage = QImage();
#else
    d->mScrollableImage = QPixmap();
#endif

}

bool KSignalPlotter::verticalLinesScroll() const
{
    return d->mVerticalLinesScroll;
}

void KSignalPlotter::setShowHorizontalLines( bool value )
{
    if(value == d->mShowHorizontalLines) return;
    d->mShowHorizontalLines = value;
    d->mBackgroundImage = QPixmap(); //we changed a paint setting, so reset the cache
}

bool KSignalPlotter::showHorizontalLines() const
{
    return d->mShowHorizontalLines;
}
void KSignalPlotter::setAxisFontColor( const QColor &color )
{
    if(!color.isValid()) {
        kDebug(1215) << "Invalid color";
        return;
    }

    d->mFontColor = color;
}

QColor KSignalPlotter::axisFontColor() const
{
    return d->mFontColor;
}


void KSignalPlotter::setHorizontalLinesColor( const QColor &color )
{
    if(!color.isValid()) {
        kDebug(1215) << "Invalid color";
        return;
    }

    if(color == d->mHorizontalLinesColor) return;
    d->mHorizontalLinesColor = color;
    d->mBackgroundImage = QPixmap(); //we changed a paint setting, so reset the cache
}

QColor KSignalPlotter::horizontalLinesColor() const
{
    return d->mHorizontalLinesColor;
}

void KSignalPlotter::setShowAxis( bool value )
{
    if(value == d->mShowAxis) return;
    d->mShowAxis = value;
    d->mBackgroundImage = QPixmap(); //we changed a paint setting, so reset the cache
#ifdef USE_QIMAGE
    d->mScrollableImage = QImage();
#else
    d->mScrollableImage = QPixmap();
#endif
}

bool KSignalPlotter::showAxis() const
{
    return d->mShowAxis;
}

void KSignalPlotter::setAxisFont( const QFont &font )
{
    d->mFont = font;
    d->mBackgroundImage = QPixmap(); //we changed a paint setting, so reset the cache
#ifdef USE_QIMAGE
    d->mScrollableImage = QImage();
#else
    d->mScrollableImage = QPixmap();
#endif

}

QFont KSignalPlotter::axisFont() const
{
    return d->mFont;
}
QString KSignalPlotter::svgBackground() const {
    return d->mSvgFilename;
}
void KSignalPlotter::setSvgBackground( const QString &filename )
{
    if(d->mSvgFilename == filename) return;
    d->mSvgFilename = filename;
    d->mBackgroundImage = QPixmap();
}

void KSignalPlotter::setBackgroundColor( const QColor &color )
{
    if(color == d->mBackgroundColor) return;
    if(!color.isValid()) {
        kDebug(1215) << "Invalid color";
        return;
    }
    d->mBackgroundColor = color;
    d->mBackgroundImage = QPixmap(); //we changed a paint setting, so reset the cache
}

QColor KSignalPlotter::backgroundColor() const
{
    return d->mBackgroundColor;
}

void KSignalPlotter::setThinFrame( bool set)
{
    if(d->mShowThinFrame == set) return;
    d->mShowThinFrame = set;
    d->mBackgroundImage = QPixmap(); //we changed a paint setting, so reset the cache
}
bool KSignalPlotter::thinFrame() const
{
    return d->mShowThinFrame;
}

void KSignalPlotter::setMaxAxisTextWidth(int axisTextWidth)
{
    d->mAxisTextWidth = axisTextWidth;
}

int KSignalPlotter::maxAxisTextWidth() const
{
    return d->mAxisTextWidth;
}

void KSignalPlotter::resizeEvent( QResizeEvent* )
{
    d->updateDataBuffers();
}

void KSignalPlotterPrivate::updateDataBuffers()
{

    /*  This is called when the widget has resized
     *
     *  Determine new number of samples first.
     *  +0.5 to ensure rounding up
     *  +4 for extra data points so there is
     *     1) no wasted space and
     *     2) no loss of precision when drawing the first data point.
     */
    mMaxSamples = uint(q->width() / mHorizontalScale + 4);
}

void KSignalPlotter::paintEvent( QPaintEvent* event)
{
    if (testAttribute(Qt::WA_PendingResizeEvent))
        return; // lets not do this more than necessary, shall we?

    uint w = width();
    uint h = height();
    /* Do not do repaints when the widget is not yet setup properly. */
    if ( w <= 2 )
        return;
    QPainter p(this);

    if(event && d->mPlottingArea.contains(event->rect()))
        d->drawWidget(&p, QRect(0,0,w, h), true);  // do not bother drawing axis text etc.
    else
        d->drawWidget(&p, QRect(0,0,w, h), false);
}

void KSignalPlotterPrivate::drawWidget(QPainter *p, QRect boundingBox, bool onlyDrawPlotter)
{
    if(boundingBox.height() <= 2 || boundingBox.width() <= 2 ) return;
    p->setFont( mFont );
    int fontheight = p->fontMetrics().height();
    mHorizontalLinesCount = qBound(0, (int)(boundingBox.height() / fontheight)-2, 4);

    if(!onlyDrawPlotter) {
        QPen pen;
        pen.setWidth(1);
        pen.setCapStyle(Qt::RoundCap);
        p->setPen(pen);

        boundingBox.setTop(p->pen().width() / 2); //The y position of the top of the graph.  Basically this is one more than the height of the top bar

        //check if there's enough room to actually show a top bar. Must be enough room for a bar at the top, plus horizontal lines each of a size with room for a scale
        if( mShowAxis && boundingBox.width() > 60 && boundingBox.height() > p->fontMetrics().height() ) {  //if there's room to draw the labels, then draw them!
            //We want to adjust the size of plotter bit inside so that the axis text aligns nicely at the top and bottom
            //but we don't want to sacrifice too much of the available room, so don't use it if it will take more than 20% of the available space
            qreal offset = (p->fontMetrics().height()+1)/2;
            drawAxisText(p, boundingBox);
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

    if( redraw || mScrollableImage.isNull() || mScrollableImage.height() != boundingBox.height() || mScrollableImage.width() != alignedWidth) {
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
        if(mBeamData.size() > 2) {
            for(int i = mBeamData.size()-2; i >= 0; i--)
                drawBeamToScrollableImage(i);
        }
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
void KSignalPlotterPrivate::drawBackground(QPainter *p, const QRect &boundingBox)
{
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

void KSignalPlotterPrivate::drawThinFrame(QPainter *p, const QRect &boundingBox)
{
    /* Draw white line along the bottom and the right side of the
     * widget to create a 3D like look. */
    p->setPen( q->palette().color( QPalette::Light ) );
    p->drawLine( boundingBox.bottomLeft(), boundingBox.bottomRight());
    p->drawLine( boundingBox.bottomRight(), boundingBox.topRight());
}

void KSignalPlotterPrivate::calculateNiceRange()
{
    double max = mUserMaxValue;
    double min = mUserMinValue;
    if( mUseAutoRange ) {
        if(mMaxValue * 0.99 > max)  //Allow max value to go very slightly over the given max, for rounding reasons
            max = mMaxValue;
        min = qMin(min, mMinValue);
    }
    double newNiceRange = max - min;
    /* If the range is too small we will force it to 1.0 since it
     * looks a lot nicer. */
    if ( newNiceRange < 0.000001 )
        newNiceRange = 1.0;

    double newNiceMinValue = min;
    if ( mMinValue != 0.0 ) {
        double dim = pow( 10, floor( log10( fabs( min ) ) ) ) / 2;
        if ( min < 0.0 )
        newNiceMinValue = dim * floor( min / dim );
      else
        newNiceMinValue = dim * ceil( min / dim );
      newNiceRange = max - newNiceMinValue;
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
        mPrecision = -logdim;
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
    emit q->axisScaleChanged();
}


void KSignalPlotterPrivate::drawVerticalLines(QPainter *p, const QRect &boundingBox, int offset)
{
    p->setPen( mVerticalLinesColor );

    for ( int x = boundingBox.right() - ( (mVerticalLinesOffset + offset) % mVerticalLinesDistance); x >= boundingBox.left(); x -= mVerticalLinesDistance )
        p->drawLine( x, boundingBox.top(), x, boundingBox.bottom()  );
}

void KSignalPlotterPrivate::drawBeamToScrollableImage(int index)
{
    if(mScrollableImage.isNull())
        return;
    QRect cacheBoundingBox = QRect(mScrollOffset, 0, mHorizontalScale, mScrollableImage.height());

    QPainter pCache(&mScrollableImage);
    pCache.setRenderHint(QPainter::Antialiasing, true);
    //To paint to the cache, we need a new bounding box

    pCache.setCompositionMode(QPainter::CompositionMode_Clear);
    pCache.fillRect(cacheBoundingBox, Qt::transparent);

    drawBeam(&pCache, cacheBoundingBox, mHorizontalScale, index);

    pCache.setCompositionMode(QPainter::CompositionMode_SourceOver);
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

void KSignalPlotterPrivate::drawBeam(QPainter *p, const QRect &boundingBox, int horizontalScale, int index)
{
    if(mNiceRange == 0) return;
    QPen pen;
    pen.setWidth(2);
    pen.setCapStyle(Qt::FlatCap);

    double scaleFac = (boundingBox.height()-2) / mNiceRange;
    if(mBeamData.size() - 1 <= index )
        return;  // Something went wrong?

    QList<double> datapoints = mBeamData[index];
    QList<double> prev_datapoints = mBeamData[index+1];
    QList<double> prev_prev_datapoints;
    if(index +2 < mBeamData.size())
        prev_prev_datapoints = mBeamData[index+2]; //used for bezier curve gradient calculation
    else
        prev_prev_datapoints = prev_datapoints;

    float x0 = boundingBox.right();
    float x1 = boundingBox.right() - horizontalScale;

    float y0 = 0;
    float y1 = 0;
    float y2 = 0;
    for (int j =  qMin(datapoints.size(), mBeamColors.size())-1; j >=0 ; --j) {
        if(!mStackBeams)
            y0 = y1 = y2 = 0;
        y0 += boundingBox.bottom() - (datapoints[j] - mNiceMinValue)*scaleFac;
        y1 += boundingBox.bottom() - (prev_datapoints[j] - mNiceMinValue)*scaleFac;
        y2 += boundingBox.bottom() - (prev_prev_datapoints[j] - mNiceMinValue)*scaleFac;
        if(mSmoothGraph) {
            // Apply a weighted average just to smooth the graph out a bit
            y0 = (2*y0 + y1)/3;
            y1 = (2*y1 + y2)/3;
            // We don't bother to average out y2.  This will introduce slight inaccuracies in the gradients, but they aren't really noticeable.
        }
        QColor beamColor = mBeamColors[j];
        if(mFillOpacity)
            beamColor = beamColor.lighter();
        pen.setColor(beamColor);


        QPainterPath path;
        path.moveTo( x1, y1);
        QPointF c1( x1 + horizontalScale/3.0, (4* y1 - y2)/3.0 );//Control point 1 - same gradient as prev_prev_datapoint to prev_datapoint
        QPointF c2( x1 + 2*horizontalScale/3.0, (2* y0 + y1)/3.0);//Control point 2 - same gradient as prev_datapoint to datapoint
        path.cubicTo(  c1, c2, QPointF(x0, y0));
        p->setCompositionMode(QPainter::CompositionMode_SourceOver);
        p->setPen(pen);
        p->drawPath(path);
        if(mFillOpacity) {
            path.lineTo(x0,boundingBox.bottom());
            path.lineTo(x1,boundingBox.bottom());
            path.lineTo(x1,y1);
            QColor fillColor = mBeamColors[j];
            fillColor.setAlpha(mFillOpacity);
            p->fillPath(path, fillColor);
        }
    }
}
void KSignalPlotterPrivate::drawAxisText(QPainter *p, const QRect &boundingBox)
{
    if(mHorizontalLinesCount < 0) return;
    double stepsize = mNiceRange/(mScaleDownBy*(mHorizontalLinesCount+1));
    p->setPen(mFontColor);
    int axisTitleIndex=1;
    QString val;
    int numItems = mHorizontalLinesCount +2;
    int fontHeight = p->fontMetrics().height();
    if(numItems == 2 && boundingBox.height() < fontHeight*2 )
        numItems = 1;
    for ( int y = 0; y < numItems; y++, axisTitleIndex++) {
        int y_coord = boundingBox.top() + (y * (boundingBox.height()-fontHeight)) /(mHorizontalLinesCount+1);  //Make sure it's y*h first to avoid rounding bugs
        double value;
        if(y == mHorizontalLinesCount+1)
            value = mNiceMinValue; //sometimes using the formulas gives us a value very slightly off
        else
            value = mNiceMaxValue/mScaleDownBy - y * stepsize;

        val = scaledValueAsString(value, mPrecision);

        if ( kapp->layoutDirection() == Qt::RightToLeft )
            p->drawText( boundingBox.right()-mAxisTextWidth, y_coord, mAxisTextWidth, fontHeight+1, Qt::AlignRight | Qt::AlignTop, val);
        else
            p->drawText( boundingBox.left(), y_coord, mAxisTextWidth, fontHeight+1, Qt::AlignRight | Qt::AlignTop, val);
    }
}

void KSignalPlotterPrivate::drawHorizontalLines(QPainter *p, const QRect &boundingBox)
{
    if(mHorizontalLinesCount <= 0) return;
    p->setPen( QPen(mHorizontalLinesColor, 0, Qt::DashLine));
    for ( int y = 0; y <= mHorizontalLinesCount+1; y++ ) {
        //note that the y_coord starts from 0.  so we draw from pixel number 0 to h-1.  Thus the -1 in the y_coord
        int y_coord =  boundingBox.top() + (y * (boundingBox.height()-1)) / (mHorizontalLinesCount+1);  //Make sure it's y*h first to avoid rounding bugs
        p->drawLine( boundingBox.left(), y_coord, boundingBox.right() - 1, y_coord);
    }
}

double KSignalPlotter::lastValue( int i) const
{
    if(d->mBeamData.isEmpty() || d->mBeamData.first().size() <= i) return 0;
    return d->mBeamData.first().at(i);
}
QString KSignalPlotter::lastValueAsString( int i, int precision) const
{
    if(d->mBeamData.isEmpty() || d->mBeamData.first().size() <= i) return QString();
    return valueAsString(d->mBeamData.first().at(i), precision); //retrieve the newest value for this beam
}
QString KSignalPlotter::valueAsString( double value, int precision) const
{
    value = value / d->mScaleDownBy; // scale the value.  E.g. from Bytes to KiB
    return d->scaledValueAsString(value, precision);
}
QString KSignalPlotterPrivate::scaledValueAsString( double value, int precision) const
{
    double absvalue = qAbs(value);
    if(precision == -1)
        precision = (absvalue >= 99.5)?0:((absvalue>=0.995)?1:2);

    if( absvalue < 1E6 ) {
        if(precision == 0)
            return mUnit.subs((long)value).toString();
        else
            return mUnit.subs(value, 0, 'f', precision).toString();
    }
    else
        return mUnit.subs(value, 0, 'g', precision).toString();
}

bool KSignalPlotter::smoothGraph() const
{
    return d->mSmoothGraph;
}

void KSignalPlotter::setSmoothGraph(bool smooth)
{
    d->mSmoothGraph = smooth;
#ifdef USE_QIMAGE
    d->mScrollableImage = QImage();
#else
    d->mScrollableImage = QPixmap();
#endif
}

bool KSignalPlotter::stackGraph() const
{
    return d->mStackBeams;
}
void KSignalPlotter::setStackGraph(bool stack)
{
    d->mStackBeams = stack;
#ifdef USE_QIMAGE
    d->mScrollableImage = QImage();
#else
    d->mScrollableImage = QPixmap();
#endif

}

int KSignalPlotter::fillOpacity() const
{
    return d->mFillOpacity;
}
void KSignalPlotter::setFillOpacity(int fill)
{
    d->mFillOpacity = fill;
#ifdef USE_QIMAGE
    d->mScrollableImage = QImage();
#else
    d->mScrollableImage = QPixmap();
#endif

}


#include <SignalPlotter.moc>

