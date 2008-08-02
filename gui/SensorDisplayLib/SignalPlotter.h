/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>

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

#ifndef KSIGNALPLOTTER_H
#define KSIGNALPLOTTER_H


#include <QWidget>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QLinkedList>
#include <QImage>

#define USE_QIMAGE


#define SVG_SUPPORT
#ifdef SVG_SUPPORT
namespace Plasma
{
    class Svg;
}
#endif
class QColor;

/** \brief The KSignalPlotter widget draws a real time graph of data that updates continually
 * 
 *  Features include:
 *  *) Points are joined by a bezier curve.
 *  *) Lines are anti-aliased
 *  *) Background can be set as a specified SVG
 *  *) The lines can be reordered
 *  *) Uses as little memory and CPU as possible
 *  *) Graph can be smoothed using the formula (value * 2 + last_value)/3.
 *
 *  Example usage:
 *  \code
 *    KSignalPlotter *s = KSignalPlotter(parent);
 *    s->addBeam(Qt::Blue);
 *    s->addBeam(Qt::Green);
 *    QList data;
 *    data << 4.0 << 5.0;
 *    s->addSample(data); 
 *  \endcode
 *
 *  Note that the number of horizontal lines is calculated automatically based on the axis font size, even if the axis labels are not shown.
 *  
 *  Smoothing looks very nice visually and is enabled by default.  It can be disabled with setSmoothGraph().
 *
 *  \image KSignalPlotter.png  Example KSignalPlotter with two beams
 */
class KSignalPlotter : public QWidget
{
  Q_OBJECT
  Q_PROPERTY( double minValue READ minValue WRITE setMinValue )
  Q_PROPERTY( double maxValue READ maxValue WRITE setMaxValue )
  Q_PROPERTY( bool useAutoRange READ useAutoRange WRITE setUseAutoRange )
  Q_PROPERTY( QString unit READ unit WRITE setUnit )
  Q_PROPERTY( bool thinFrame READ thinFrame WRITE setThinFrame )
  Q_PROPERTY( double scaleDownBy READ scaleDownBy WRITE setScaleDownBy )
  Q_PROPERTY( uint horizontalScale READ horizontalScale WRITE setHorizontalScale )
  Q_PROPERTY( bool showHorizontalLines READ showHorizontalLines WRITE setShowHorizontalLines )
  Q_PROPERTY( bool showVerticalLines READ showVerticalLines WRITE setShowVerticalLines )
  Q_PROPERTY( bool verticalLinesScroll READ verticalLinesScroll WRITE setVerticalLinesScroll )
  Q_PROPERTY( QColor verticalLinesColor READ verticalLinesColor WRITE setVerticalLinesColor )
  Q_PROPERTY( QColor horizontalLinesColor READ horizontalLinesColor WRITE setHorizontalLinesColor )
  Q_PROPERTY( bool verticalLinesDistance READ verticalLinesDistance WRITE setVerticalLinesDistance )
  Q_PROPERTY( QColor axisFontColor READ axisFontColor WRITE setAxisFontColor )
  Q_PROPERTY( QFont axisFont READ axisFont WRITE setAxisFont )
  Q_PROPERTY( bool showAxis READ showAxis WRITE setShowAxis )
  Q_PROPERTY( QColor backgroundColor READ backgroundColor WRITE setBackgroundColor )
  Q_PROPERTY( QString svgBackground READ svgBackground WRITE setSvgBackground )
  Q_PROPERTY( int maxAxisTextWidth READ maxAxisTextWidth WRITE setMaxAxisTextWidth )
  Q_PROPERTY( bool smoothGraph READ smoothGraph WRITE setSmoothGraph )

  public:
    KSignalPlotter( QWidget *parent = 0);
    ~KSignalPlotter();

    /** Add a new line to the graph plotter, with the specified color.
     *  Note that the order you add the beams in must be the same order that
     *  the beam data is given in. (Unless you reorder the beams)
     */
    void addBeam( const QColor &color );

    /** Add data to the graph, and advance the graph by one time period.
     *  The data must be given as a list in the same order that the beams were
     *  added (or consequently reordered)
     */
    void addSample( const QList<double> &samples );

    /** Reorder the beams into the order given.  For example:
     * \code
     *   KSignalPlotter *s = KSignalPlotter(parent);
     *   s->addBeam(Qt::Blue);
     *   s->addBeam(Qt::Green);   
     *   QList neworder;
     *   neworder << 1 << 0;
     *   reorderBeams( newOrder);
     *   //Now the order is Green then Blue
     * \endcode
     */
    void reorderBeams( const QList<int>& newOrder );

    /** Removes the beam at the specified index.  This causes the graph to be redrawn with the
     *  specified beam completely removed.
     */
    void removeBeam( uint pos );

    /** Get the beam (the graph lines) colors, in the order
     *  that the beams were added (or later reordered)
     */
    QColor beamColor( int index );

    /** Set the beam (the graph lines) colors, in the order
     *  that the beams were added (or later reordered)
     */
    void setBeamColor( int index, QColor color );

    /** returns the number of beams */
    int numBeams();

    /** Set the units.  Drawn on the vertical axis of the graph.
     *  Must be already translated into the local language. 
     */
    void setUnit( const QString &unit );

    /** Return the units used on the vertical axis of the graph.
     */ 
    QString unit() const;

    /** Scale all the values down by the given amount.  This is useful
     *  when the data is given in, say, kilobytes, but you set the 
     *  units as megabytes.  Thus you would have to call this with @p value
     *  set to 1024.  This affects all the data already entered.
     *  Typically this is followed by calling setUnit to set
     *  the display axis units.
     */
    void setScaleDownBy( double value );

    /** Amount scaled down by.  @see setScaleDownBy */
    double scaleDownBy() const;

    /** Set the minimum and maximum values on the vertical axis
     *  automatically from the data available.
     */
    void setUseAutoRange( bool value );

    /** Whether the vertical axis range is set automatically.
     */
    bool useAutoRange() const;

    /** Change the minimum and maximum values drawn on the graph.
     *  Note that these values are sanitised.  For example, if you
     *  set the minimum as 3, and the maximum as 97, then the graph
     *  would be drawn between 0 and 100.  The algorithm to determine
     *  this "nice range" attempts to minimize the number of non-zero
     *  digits.
     *
     *  Use setAutoRange instead to determine the range automatically 
     *  from the data.
     */ 
    void changeRange( double min, double max );
    /** Set the min value of the vertical axis.  @see changeRange */
    void setMinValue( double min );
    /** Get the min value of the vertical axis.  @see changeRange */
    double minValue() const;
    /** Set the max value of the vertical axis.  @see changeRange */
    void setMaxValue( double max );
    /** Get the max value of the vertical axis.  @see changeRange */
    double maxValue() const;

    /** Set the number of pixels horizontally between data points */
    void setHorizontalScale( uint scale );
    /** The number of pixels horizontally between data points*/
    int horizontalScale() const;

    /** Whether to draw the vertical grid lines */
    void setShowVerticalLines( bool value );
    /** Whether to draw the vertical grid lines */
    bool showVerticalLines() const;

    /** The color of the vertical grid lines */
    void setVerticalLinesColor( const QColor &color );
    /** The color of the vertical grid lines */
    QColor verticalLinesColor() const;

    /** The horizontal distance between the vertical grid lines */
    void setVerticalLinesDistance( uint distance );
    /** The horizontal distance between the vertical grid lines */
    int verticalLinesDistance() const;

    /** Whether the vertical lines move with the data */
    void setVerticalLinesScroll( bool value );
    /** Whether the vertical lines move with the data */
    bool verticalLinesScroll() const;

    /** Whether to draw the horizontal grid lines */
    void setShowHorizontalLines( bool value );
    /** Whether to draw the horizontal grid lines */
    bool showHorizontalLines() const;

    /** The color of the horizontal grid lines */
    void setHorizontalLinesColor( const QColor &color );
    /** The color of the horizontal grid lines */
    QColor horizontalLinesColor() const;

    /** The color of the font used for the axis */
    void setAxisFontColor( const QColor &color );
    /** The color of the font used for the axis */
    QColor axisFontColor() const;

    /** The font used for the axis */
    void setAxisFont( const QFont &font );
    /** The font used for the axis */
    QFont axisFont() const;

    /** Whether to show the vertical axis labels */
    void setShowAxis( bool show );
    /** Whether to show the vertical axis labels */
    bool showAxis() const;

    /** The color to set the background.  This is painted even if there
     *  is an SVG, to allow for translucent/transparent SVGs.
     */
    void setBackgroundColor( const QColor &color );
    
    /** The color to set the background.  This is painted even if there
     *  is an SVG, to allow for translucent/transparent SVGs.
     */
    QColor backgroundColor() const;

    /** The filename of the svg background.  Set to empty to disable
     *  again. */
    void setSvgBackground( const QString &filename );

    /** The filename of the svg background.  Set to empty to disable
     *  again. */
    QString svgBackground() const;

    /** Return the last value that we have for beam i.
     *  Returns 0 if not known */
    double lastValue( int i) const;

    /** Return a translated string like:   "34 %" or "100 KB" for beam i */
    QString lastValueAsString( int i) const;
    
    /** Return a translated string like:   "34 %" or "100 KB" for the given value in unscaled units */
    QString valueAsString( double value) const;
    
    /**  Whether to show a white line on the left and bottom of the widget, for a 3D effect */
    void setThinFrame( bool set );
    
    /**  Whether to show a white line on the left and bottom of the widget, for a 3D effect */
    bool thinFrame() const;

    /** Set the distance between the left of the widget and the left of the plotting region. */
    void setMaxAxisTextWidth(int maxAxisTextWidth);

    /** Get the distance between the left of the widget and the left of the plotting region. */
    int maxAxisTextWidth() const;

    /** Whether to smooth the graph by averaging the points using the formula:  (value*2 + last_value)/3 */
    bool smoothGraph() const;

    /** Set whether to smooth the graph by averaging the points using the formula:  (value*2 + last_value)/3 */
    void setSmoothGraph(bool smooth);
  
  Q_SIGNALS:
    /** When the axis has changed because we are in autorange mode, then this signal is emitted */
    void axisScaleChanged();

  protected:
    virtual void resizeEvent( QResizeEvent* );
    virtual void paintEvent( QPaintEvent* );

    void drawWidget(QPainter *p, QRect boundingBox, bool onlyDrawPlotter);
    void drawBackground(QPainter *p, const QRect & boundingBox);
    void drawThinFrame(QPainter *p, const QRect &boundingBox);
    void calculateNiceRange();
    void drawVerticalLines(QPainter *p, const QRect &boundingBox, int correction=0);
    void drawBeamToScrollableImage(int index);
    void drawBeam(QPainter *p, const QRect &boundingBox, int horizontalScale, int index);
    void drawAxisText(QPainter *p, const QRect &boundingBox);
    void drawHorizontalLines(QPainter *p, const QRect &boundingBox);

  private:
    void updateDataBuffers();
    /** We make the svg renderer static so that an svg renderer is shared among all of the images.  This is because a svg renderer takes up a lot of memory, so we want to 
     *  share them as much as we can */
#ifdef SVG_SUPPORT
    static QHash<QString, Plasma::Svg *> sSvgRenderer;
#endif
    QString mSvgFilename; 

    QPixmap mBackgroundImage;	///A cache of the background of the widget. Contains the svg or just white background with lines
#ifdef USE_QIMAGE
    QImage mScrollableImage;	///The scrollable image for the widget.  Contains the svg lines
#else
    QPixmap mScrollableImage;	///The scrollable image for the widget.  Contains the svg lines
#endif
    int mScrollOffset;		///The scrollable image is, well, scrolled in a wrap-around window.  mScrollOffset determines where the left hand side of the mScrollableImage should be drawn relative to the right hand side of view.  0 <= mScrollOffset < mScrollableImage.width()
    double mMinValue;		///The minimum value (unscaled) currently being displayed
    double mMaxValue;		///The maximum value (unscaled) currently being displayed

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
    bool mFillBeams;	/// Fill the area underneath the beams

    bool mShowAxis;

    QColor mBackgroundColor;
    QColor mFontColor;

    QList < QList<double> > mBeamData; // Every item in the linked list contains a set of data points to plot.  The first item is the newest
    QList< QColor> mBeamColors;  //These colors match up against the QList<double>  in mBeamData
    QList< QColor> mBeamColorsDark;  //These colors match up against the QList<double> in mBeamData, and are darker than mBeamColors.  Done for gradient effects

    unsigned int mSamples; //This is what mBeamData.size() should equal when full.  When we start off and have no data then mSamples will be higher.  If we resize the widget so it's smaller, then for a short while this will be smaller
    int mNewestIndex; //The index to the newest item added.  newestIndex+1   is the second newest, and so on

    QString mUnit;

    QFont mFont;
    int mAxisTextWidth;
    QRect mPlottingArea; /// The area in which the beams are drawn.  Saved to make update() more efficent

    bool mSmoothGraph; /// Whether to smooth the graph by averaging using the formula (value*2 + last_value)/3.
};

#endif
