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


#include <QWidget>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QLinkedList>
#include <QImage>

class QColor;
namespace Plasma
{
    class Svg;
} // namespace Plasma

/** Draw a real time graph of data that updates continually
 * 
 *  Features include:
 *  *) Points are joined by bezier curve.  Note that this technically means that the line may not actually go through all the specified points if they are too far apart.
 *  *) Lines are anti-aliased
 *  *) Background can be set as a specified SVG
 *  *) The lines can be reordered
 *  *) Uses as little memory and CPU as possible
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
 */
class KSignalPlotter : public QWidget
{
  Q_OBJECT

  public:
    KSignalPlotter( QWidget *parent = 0);
    ~KSignalPlotter();

    /** Add a new line to the graph plotter, with the specified color.
     *  Note that the order you add the beams must be the same order that
     *  the same data is given in. (Unless you reorder the beams)
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

    /** Removes the beam at the specified index.
     */
    void removeBeam( uint pos );

    /** Return the list of beam (the graph lines) colors, in the order
     *  that the beams 
     *  were added (or later reordered)
     */
    QList<QColor> &beamColors();

    /** Set the title of the graph.  Drawn in the top left. */
    void setTitle( const QString &title );
    /** Get the title of the graph.  Drawn in the top left. */
    QString title() const;

    /** Set the units.  Drawn on the vertical axis of the graph.
     *  Must be already translated into the local language. 
     */
    void setTranslatedUnit( const QString &unit );
    /** Return the units used on the horizontal axis of the graph.
     */ 
    QString translatedUnit() const;

    /** Scale all the values down by the given amount.  This is useful
     *  when the data is given in, say, kilobytes, but you set the 
     *  units as megabytes.  Thus you would have to call this with @p value
     *  set to 1024.  This affects all the data already entered.
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
    void setFontColor( const QColor &color );
    /** The color of the font used for the axis */
    QColor fontColor() const;

    /** The font used for the axis */
    void setFont( const QFont &font );
    /** The font used for the axis */
    QFont font() const;

    /** The number of horizontal lines to draw.  Doesn't include the top
     *  most and bottom most lines. */
    void setHorizontalLinesCount( uint count );
    /** The number of horizontal lines to draw.  Doesn't include the top
     *  most and bottom most lines. */
    int horizontalLinesCount() const;

    /** Whether to show the vertical axis labels */
    void setShowLabels( bool value );
    /** Whether to show the vertical axis labels */
    bool showLabels() const;

    /** Whether to show the title etc at the top.  Even if set, it
     *  won't be shown if there isn't room */
    void setShowTopBar( bool value );
    /** Whether to show the title etc at the top.  Even if set, it
     *  won't be shown if there isn't room */
    bool showTopBar() const;

    /** The color to set the background.  This might not be seen
     *  if an svg is also set.*/
    void setBackgroundColor( const QColor &color );
    /** The color to set the background.  This might not be seen
     *  if an svg is also set.*/
    QColor backgroundColor() const;

    /** The filename of the svg background.  Set to empty to disable
     *  again. */
    void setSvgBackground( const QString &filename );
    /** The filename of the svg background.  Set to empty to disable
     *  again. */
    QString svgBackground();

    /** Return the last value that we have for beam i.
     *  Returns 0 if not known
     */
    double lastValue( int i) const;
    /** Return a translated string like:   "34 %" or "100 KB" for beam i
     */
    QString lastValueAsString( int i) const;
    
    /**  Whether to show a white line on the left and bottom of the widget, for a 3D effect
     */
    void setThinFrame( bool set);

    /** Whether to stack the beams on top of each other.  The first beam
     *  added will be at the bottom.  The next beam will be drawn on top,
     *  and so on. */
    void setStackBeams( bool stack) { mStackBeams = stack; mFillBeams = stack; }
    /** Whether to stack the beams.  @see setStackBeams */
    bool stackBeams() const { return mStackBeams;}
    /** Render the graph to the specified width and height, and return it
     *  as an image.  This is useful, for example, if you draw a small version
     *  of the graph, but then want to show a large version in a tooltip etc */
    QImage getSnapshotImage(uint width, uint height);

  protected:
    void updateDataBuffers();

    virtual void resizeEvent( QResizeEvent* );
    virtual void paintEvent( QPaintEvent* );

    void drawWidget(QPainter *p, uint w, uint height, int horizontalScale);
    void drawBackground(QPainter *p, int w, int h);
    void drawThinFrame(QPainter *p, int w, int h);
    void calculateNiceRange();
    void drawTopBarFrame(QPainter *p, int fullWidth, int seperatorX, int height);
    void drawTopBarContents(QPainter *p, int x, int width, int height);
    void drawVerticalLines(QPainter *p, int top, int w, int h);
    /** Used from paint().  Draws all the beams on the paint device, given the top, width and height and the range of values to show
     */
    void drawBeams(QPainter *p, int top, int w, int h, int horizontalScale);
    void drawAxisText(QPainter *p, int top, int h);
    void drawHorizontalLines(QPainter *p, int top, int w, int h);

  private:
    /** We make the svg renderer static so that an svg renderer is shared among all of the images.  This is because a svg renderer takes up a lot of memory, so we want to 
     *  share them as much as we can */
    static QHash<QString, Plasma::Svg *> sSvgRenderer; 
    QString mSvgFilename; 

    QImage mBackgroundImage;  //A cache of the svg

    double mMinValue;
    double mMaxValue;

    double mNiceMinValue;
    double mNiceMaxValue;
    double mNiceRange;

    double mScaleDownBy;
    bool mUseAutoRange;

    /**  Whether to show a white line on the left and bottom of the widget, for a 3D effect */
    bool mShowThinFrame;

    /** Whether to stack the beams on top of each other */
    bool mStackBeams;
    /** Whether to fill the area underneath the beams */
    bool mFillBeams;

    uint mGraphStyle;

    bool mShowVerticalLines;
    int mPrecision;
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
    QList< QColor> mBeamColors;  //These colors match up against the QList<double>  in mBeamData
    QList< QColor> mBeamColorsDark;  //These colors match up against the QList<double> in mBeamData, and are darker than mBeamColors.  Done for gradient effects

    unsigned int mSamples; //This is what mBeamData.size() should equal when full.  When we start off and have no data then mSamples will be higher.  If we resize the widget so it's smaller, then for a short while this will be smaller
    int mNewestIndex; //The index to the newest item added.  newestIndex+1   is the second newest, and so on

    QString mTitle;
    QString mUnit;

    QFont mFont;
};

#endif
