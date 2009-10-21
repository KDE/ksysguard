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

#define USE_QIMAGE

// SVG support causes it to crash at the moment :(
//#define SVG_SUPPORT
#ifdef SVG_SUPPORT
namespace Plasma
{
    class SVG;
}
#endif

class KSignalPlotter;

struct KSignalPlotterPrivate {

    KSignalPlotterPrivate( KSignalPlotter * q_ptr );

    void drawWidget(QPainter *p, QRect boundingBox, bool onlyDrawPlotter);
    void drawBackground(QPainter *p, const QRect & boundingBox);
    void drawThinFrame(QPainter *p, const QRect &boundingBox);
    void calculateNiceRange();
    void drawVerticalLines(QPainter *p, const QRect &boundingBox, int correction=0);
    void drawBeamToScrollableImage(int index);
    void drawBeam(QPainter *p, const QRect &boundingBox, int horizontalScale, int index);
    void drawAxisText(QPainter *p, const QRect &boundingBox);
    void drawHorizontalLines(QPainter *p, const QRect &boundingBox);

    void recalculateMaxMinValueForSample(const QList<double>&sampleBuf, int time );
    void rescale();
    void updateDataBuffers();

    /** Return the given value as a string, with the given precision */
    QString scaledValueAsString( double value, int precision) const;
    void addSample( const QList<double>& sampleBuf );
    /** We make the SVG renderer static so that an SVG renderer is shared among all of the images.  This is because a SVG renderer takes up a lot of memory, so we want to
     *  share them as much as we can */
#ifdef SVG_SUPPORT
    static QHash<QString, Plasma::SVG *> sSvgRenderer;
#endif
    QString mSvgFilename;

    QPixmap mBackgroundImage;	///A cache of the background of the widget. Contains the SVG or just white background with lines
#ifdef USE_QIMAGE
    QImage mScrollableImage;	///The scrollable image for the widget.  Contains the SVG lines
#else
    QPixmap mScrollableImage;	///The scrollable image for the widget.  Contains the SVG lines
#endif
    int mScrollOffset;		///The scrollable image is, well, scrolled in a wrap-around window.  mScrollOffset determines where the left hand side of the mScrollableImage should be drawn relative to the right hand side of view.  0 <= mScrollOffset < mScrollableImage.width()
    double mMinValue;		///The minimum value (unscaled) currently being displayed
    double mMaxValue;		///The maximum value (unscaled) currently being displayed

    double mUserMinValue;		///The minimum value (unscaled) set by changeRange().  This is the _maximum_ value that the range will start from.
    double mUserMaxValue;		///The maximum value (unscaled) set by changeRange().  This is the _minimum_ value that the range will reach to.
    unsigned int mRescaleTime;		///The number of data points passed since a value that is within 70% of the current maximum was found.  This is for scaling the graph

    double mNiceMinValue;	///The minimum value rounded down to a 'nice' value
    double mNiceMaxValue;	///The maximum value rounded up to a 'nice' value.  The idea is to round the value, say, 93 to 100.
    double mNiceRange;		/// mNiceMaxValue - mNiceMinValue
    int mPrecision;		///The number of decimal place required to unambiguously label the axis

    double mScaleDownBy;	/// @see setScaleDownBy
    bool mUseAutoRange;		/// @see setUseAutoRange

    /**  Whether to show a white line on the left and bottom of the widget, for a 3D effect */
    bool mShowThinFrame;

    bool mShowVerticalLines;
    QColor mVerticalLinesColor;
    uint mVerticalLinesDistance;
    bool mVerticalLinesScroll;
    uint mVerticalLinesOffset;
    uint mHorizontalScale;
    int mHorizontalLinesCount;

    bool mShowHorizontalLines;
    QColor mHorizontalLinesColor;

    bool mStackBeams;	/// Set to add the beam values onto each other
    int mFillOpacity;	/// Fill the area underneath the beams

    bool mShowAxis;

    QColor mBackgroundColor;
    QColor mFontColor;

    QList < QList<double> > mBeamData; // Every item in the linked list contains a set of data points to plot.  The first item is the newest
    QList< QColor> mBeamColors;  //These colors match up against the QList<double>  in mBeamData
    QList< QColor> mBeamColorsDark;  //These colors match up against the QList<double> in mBeamData, and are darker than mBeamColors.  Done for gradient effects

    unsigned int mMaxSamples; //This is what mBeamData.size() should equal when full.  When we start off and have no data then mSamples will be higher.  If we resize the widget so it's smaller, then for a short while this will be smaller
    int mNewestIndex; //The index to the newest item added.  newestIndex+1   is the second newest, and so on

    KLocalizedString mUnit;

    QFont mFont;
    int mAxisTextWidth;
    QRect mPlottingArea; /// The area in which the beams are drawn.  Saved to make update() more efficient

    bool mSmoothGraph; /// Whether to smooth the graph by averaging using the formula (value*2 + last_value)/3.
    KSignalPlotter *q;
};
