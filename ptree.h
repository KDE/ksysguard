/* ptree.h 
-------------------------------------------------------------------
$Revision$
$Date$
  
ProcTree class declaration

Copyright (C) 1998 Nicolas Leclercq
               nicknet@planete.net

Most of the code is :
Copyright (C) 1996 Keith Brown and KtSoft
kdeui=>ktstreelist modified for kpstree.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABLILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details. You should have received a
copy of the GNU General Public License along with this program; if
not, write to the Free Software Foundation, Inc, 675 Mass Ave, 
Cambridge, MA 02139, USA.

-------------------------------------------------------------------
*/

#ifndef PTREE__H
#define PTREE__H

#include <qapp.h>       // used for QApplication::closingDown()
#include <qkeycode.h>   // used for keyboard interface
#include <qpainter.h>   // used to paint items
#include <qpixmap.h>	// used in items
#include <qstack.h>	// used to specify tree paths
#include <qstring.h>	// used in items
#include <qtablevw.h>	// base class for widget

#include "comm.h"     

#define OUTPUT_WIDTH 132

// use stack of strings to represent path information
typedef QStack<QString> KPath;

// use a struct to store each proc info
typedef struct {
  char  name[COMM_LEN+1];
  int   pid;
  int   ppid;
  int   uid;
  char  uname[256];
  char  arg[OUTPUT_WIDTH+1];
} ProcInfo;

class ProcTree;		// forward declaration

/** Items for the ProcTree widget */
class ProcTreeItem
{
public:
  /**
    Item constructor. While text defaults to a null pointer, and the item
    can be constructed this way, the text has to be non-null when the
    item is added to the tree, or it will not be inserted.
    !!! ext can not be null when added to the list!

    */
  ProcTreeItem(const ProcInfo  info          ,
	       const QPixmap  *thePixmap = 0);

  virtual ~ProcTreeItem();

  void appendChild(ProcTreeItem *newChild);

  virtual QRect boundingRect(const QFontMetrics& fm) const;

  /**
	Returns a pointer to the child item at the given index in this item's
	sub tree, or 0 if not found.
	*/	
  ProcTreeItem *childAt(int index);

  /**
	Returns the number of child items in this item's sub tree.
	*/
  uint childCount() const;
  /**
	Returns the index in this items sub tree of the given item or -1 if
	not found.
	*/
  int childIndex(ProcTreeItem *child);

  bool drawExpandButton() const;

  bool drawText() const;

  bool drawTree() const;

  bool expandButtonClicked(const QPoint& coord) const;

  int getBranch() const;

  /**
	Returns a pointer to the first child item in this item's sub tree, or
	0 if none.
	*/
  ProcTreeItem *getChild();

  int getIndent() const;

  /**
	Returns a pointer to the parent of this item, or 0 if none.
	*/
  ProcTreeItem *getParent();

  /**
	Returns parent ID (ppid) of this item.
	*/
  const int getParentId();


  const int      getProcId();
  const char*    getProcName();
  const int      getProcUid();
  const ProcInfo getProcInfo();

  /**
	Returns a pointer to this item's pixmap. If there is no pixmap
	associated with this item, it will return a pointer to a valid, null
	QPixmap.
	*/
  const QPixmap *getPixmap() const;

  /**
	Returns a pointer to the next item in the same branch below this one,
	or 0 if none.
	*/
  ProcTreeItem *getSibling();

  /**
	Returns a pointer to this item's text.
	*/
  const char *getText() const;

  /**
	Indicates whether this item has any children.
	*/
  bool hasChild() const;

  /**
	Indicates whether this item has a parent.
	*/
  bool hasParent() const;

  /**
	Indicates whether this item has an item in the same branch below it.
	*/
  bool hasSibling() const;

  virtual int height(const ProcTree *theOwner) const;

  void insertChild(int index,
				   ProcTreeItem *newItem);

  bool isExpanded() const;

  virtual void paint(QPainter *p, 
					 const QColorGroup& cg, 
					 bool highlighted);

  void removeChild(ProcTreeItem *child);

  void setBranch(int level);

  void setChild(ProcTreeItem *newChild);

  void setDrawExpandButton(bool doit);

  void setDrawText(bool doit);

  void setDrawTree(bool doit);

  void setExpanded(bool is);

  void setIndent(int value);

  void setParent(ProcTreeItem *newParent);

  /**
	Sets the item pixmap to the given pixmap.
	*/
  void setPixmap(const QPixmap *pm);

  void setSibling(ProcTreeItem *newSibling);

  /**
	Sets the item text to the given item.
	*/
  void setText(const char *t);
  void setSortPidText(void);
  void setSortNameText(void);
  void setSortUidText(void);

  virtual QRect textBoundingRect(const QFontMetrics& fm) const;

  virtual int width(const ProcTree *theOwner) const;

protected:
  virtual int itemHeight(const QFontMetrics& fm) const;
  virtual int itemWidth(const QFontMetrics& fm) const;

private:
  int           branch;
  int           indent;
  int           numChildren;
  bool          doExpandButton;
  bool          expanded;
  bool          doTree;
  bool          doText;
  QRect         expandButton;
  ProcTreeItem *child;
  ProcTreeItem *parent;
  ProcTreeItem *sibling;
  QPixmap       pixmap;
  QString       text;
  ProcInfo      pInfo;
};

// easier declarations of function prototypes for forEvery type functions
typedef bool (ProcTree::*KForEveryM)
  (ProcTreeItem *, void *);
typedef bool (*KForEvery)
  (ProcTreeItem *, void *);

struct KItemSearchInfo {
  int count;
  int index;
  ProcTreeItem *foundItem;
  KItemSearchInfo() : count(0), index(0), foundItem(0) {}
};

/** 
  A collapsible treelist widget.
  
  1. Introduction
  2. Features
  3. Installation
  4. Public interface

  1. Introduction
  ================================================================================

  ProcTree is a class inherited from QTableView in the Qt user interface
  library. It provides a way to display hierarchical data in a single-inheritance
  tree, similar to tree controls in Microsoft Windows and other GUI's. It is most
  suitable for directory trees or outlines, but I'm sure other uses will come to
  mind. Frankly, it was designed mostly with the above two functions in mind, but
  I have tried to make it as flexible as I know how to make it easy to adapt to
  other uses. 

  In case of problems, I encourage you to read all of the other documentation
  files in this package before contacting me  as you may find the answer to your
  question in one of them. Also read the source code if you have time. I have
  tried to comment it adequately and make the source understandable.

  2. Features
  ================================================================================

  * Displays both text and optional pixmap supplied by the programmer. A support
  class, ProcTreeItem, can be inherited and modified to draw items as needed
  by the programmer.

  * The list items can be returned by index or logical path and the tree
  navigated by parent, child or sibling references contained in them. Also,
  item information such as text, pixmap, branch level can be obtained.
  
  * Items can be inserted, changed and removed either by index in the visible
  structure, or by logical paths through the tree hierarchy. 

  * The logical path through the tree for any item can be obtained with the index
  of the item.

  * Tree structure display and expanding/collapsing of sub-trees is handled with
  no intervention from the programmer.
  
  * entire tree can be expanded or collapsed to a specified sub-level (handy for
  outline views)
  
  * Configuration as follows:

  enable/disable item text display (if you only want to display pixmaps)
  
  enable/disable drawing of expand/collapse button
  
  enable/disable drawing of tree structure
  
  * Keyboard support as follows:

  up/down arrows move the highlight appropriately and scroll the list an item at
  a time, if necessary
  
  pgup/pgdn move the highlight a 'page' up or down as applicable and scroll the
  view
  
  +/- keys expand/collapse the highlighted item if it appropriate
  
  enter key selects the highlighted item
  
  * Mouse support as follows:

  left click on item highlights it
  
  left click on an item "hot button" expands or collapses its sub-tree, as
  applicable
  
  double click on item selects it
  
  normal scrolling functions in conjunction with scrollbars if present

  2nd scrolling with the middle mouse button: pressing MMB inserts a
  rubberband, showing which part of the whole tree is currently visible.
  moving the mouse will scroll the visible part
  
  * Signals/Slots

  signal void highlighted(int) - emitted when an item in the tree is
  highlighted; sends the index of the item
  
  signal void selected(int) - emitted when an item in the tree is
  selected; sends the index of the item
  
  signal void expanded(int) - emitted when an item in the tree is expanded;
  sends the index of the item
  
  signal void collpased(int) - emitted when an item in the tree is collapsed;
  sends the index of the item
  */
class ProcTree : public QTableView
{
  Q_OBJECT
public:
  /**
	Widget contructor. Passes all parameters on to base QTableView, and
	does not use them directly. Does internal initialization, sets the
	current item to -1, and sets default values for scroll bars (both
	auto) and grid snap (snap to grid vertically).
	*/
  ProcTree(QWidget *parent   = 0, 
	   const char *name  = 0,
	   WFlags f          = 0);

  virtual ~ProcTree();

  /**
	Adds a new item to the tree with the given text and pixmap as a child
	of the item currently at the given index. If the current item already
	has children, the new item is appended below them.
	*/
  void addChildItem(const ProcInfo pi,
		    const QPixmap *thePixmap,
		    int index);

  /**
	Same as above except parent item is specified by path.
	*/
  void addChildItem(const ProcInfo pi,
		    const QPixmap *thePixmap,
		    const KPath *thePath); 

  /**
	Adds the given item as a child of the item currently at the given
	index. If the current item already has children, the new item is
	appended below them.
	*/
  void addChildItem(ProcTreeItem *newItem,
		    int index);

  /**
	Same as above except parent item is specified by path.
	*/
  void addChildItem(ProcTreeItem *newItem,
		    const KPath *thePath);                                                             
  /**
	Returns a bool value indicating whether the list will display a
	horizontal scrollbar if one of the displayed items is wider than can
	be displayed at the current width of the view.
	*/
  bool autoBottomScrollBar() const;

  /**
	Returns a bool value indicating whether the list will display a
	vertical scrollbar if the number of displayed items is more than can
	be displayed at the current height of the view.
	*/
  bool autoScrollBar() const;

  /**
	Returns a bool value indicating whether the list will update
	immediately on changing the state of the widget in some way.
	*/
  bool autoUpdate() const;

  /**
	Returns a bool value indicating whether the list has currently has a
	horizontal scroll bar.
	*/
  bool bottomScrollBar() const;

  /**
	Changes the text and/or pixmap of the given item at the specified
	index to the given values and updates the display if auto update
	enabled. If changing only the text or pixmap, set the other parameter
	to 0.
	*/
  void changeItem(const ProcInfo pi,
		  const QPixmap *newPixmap, 
		  int index);

  /**
	Same as above function, except item to change is specified by a path
	through the tree.
	*/
  void changeItem(const ProcInfo pi,
		  const QPixmap *newPixmap,
		  const KPath *thePath);

  /**
	Removes all items from the tree.

	*/
  void clear();

  /**
	Returns the total number of items in the tree, whether visible
	(expanded sub-trees) or not (collapsed).
	*/
  uint count();

  /**
	Returns the index of the current (highlighted) item. If no current
	item, returns -1.
	*/
  int currentItem() const;

  /**
	Collapses the sub-tree at the specified index. 
	*/
  void collapseItem(int index);

  /**
	Expands the sub-tree at the specified index. 
	*/
  void expandItem(int index);

  /**
	Returns the depth to which all parent items are automatically
	expanded.
	*/
  int expandLevel() const;

  /**
	Same as above functions combined into one. If sub-tree is expanded,
	collapses it, if it is collapsed, it expands it.
	*/
  void expandOrCollapseItem(int index);

  /**
	Iterates every item in the tree, visible or not, and applies the
	function func with a pointer to each item and user data
	supplied as parameters. KForEveryFunc is defined as:
          
	typedef void (*KForEveryFunc)(ProcTreeItem *, void *); 
          
	That is, a function that returns void and takes a pointer to a
	ProcTreeItem and pointer to void as parameters. 
	*/
  void forEveryItem(KForEvery func, 
					void *user);

  /**
	Same as above, but only iterates visible items, in order.			    
	*/
  void forEveryVisibleItem(KForEvery func, 
						   void *user);

  /**
	Returns a pointer to the current item if there is one, or 0.
	*/
  ProcTreeItem *getCurrentItem();

  /**
	Returns the number of pixels an item is indented for each level.
	*/
  int indentSpacing();

  /**
	Inserts an item into the tree with the given text and pixmap either
	before or after the item currently at the given index, depending on
	the value of prefix. The new item is added to the same branch as the
	reference item. If index is -1, the item is simply appended to the
	tree at the root level. The item text must not be null.
	*/
  void insertItem(const ProcInfo pi,
		  const QPixmap *thePixmap,
		  int index = -1,
		  bool prefix = TRUE);

  /**
	Same as above, but uses a path through the tree to reference the
	insert position.
	*/
  void insertItem(const ProcInfo pi,
		  const QPixmap *thePixmap,
		  const KPath *thePath,
		  bool prefix = TRUE);

  /**
	Inserts the given item into the tree either before or after the item
	currently at the given index, depending on the value of prefix. The
	new item is add to the same branch as the reference item. If index is
	-1, the item is simply appended to the tree at the root level. The
	item text must not be null.
	*/
  void insertItem(ProcTreeItem *newItem, 
		  int index = -1,
		  bool prefix = TRUE); 

  /**
	Same as above, but uses a path through the tree to reference the
	insert position.
	*/
  void insertItem(ProcTreeItem *newItem,
		  const KPath *thePath,
		  bool prefix = TRUE);

  /**
	Returns a pointer to the item at index.
	*/
  ProcTreeItem *itemAt(int index);

  /**
	Returns a pointer ot the item at the end of thePath.
	*/
  ProcTreeItem *itemAt(const KPath *path);

  /**
	Returns the index of the given item in the visible tree or -1 if not
	found.			 
	*/
  int itemIndex(ProcTreeItem *item);
  int itemVisibleIndex(ProcTreeItem *item);

  /**
	Returns a pointer to the logical path to the item at the specified
	index. The return KPath object must be deleted by the caller.
	Any strings still contained in the stack will be automatically
	deleted, but any popped from the path must also be deleted by the
	caller.
	*/
  KPath *itemPath(int index);

  /**
	Outdents the item at the given index one level so that it becomes a
	sibling of its parent.
	*/
  void join(int index);

  /**
   */
  void join(const KPath *path);

  /**
	Moves the item at the specified index down one slot in its current
	branch.
	*/
  void lowerItem(int index);

  /**
	Same as above but uses a path to specify the item.                              
	*/
  void lowerItem(const KPath *path);

  /**
	Moves the item at the specified index up one slot in its current
	branch. 
	*/
  void raiseItem(int index);

  /**
	Same as above but uses a path to specify the item.                  

	*/
  void raiseItem(const KPath *path);

  /**
	Removes the item at the specified index. 
	*/
  void removeItem(int index);

  /**
	Same as above except uses path through the tree to find the item.
	*/
  void removeItem(const KPath *thePath);

  /**
	Returns bool value indicating whether the list currently displays a
	vertical scroll bar.
	*/
  bool scrollBar() const;

  /**
	If enable is TRUE (default), enables auto update, else disables it.
	*/
  void setAutoUpdate(bool enable);

  /**
	If enable is TRUE, displays a horizontal scroll bar, else hides it.
	*/
  void setBottomScrollBar(bool enable);

  /**
	Makes the item at index current and highlights it.
	*/
  void setCurrentItem(int index);

  void setExpandButtonDrawing(bool enable);

  void setExpandLevel(int level);

  void setIndentSpacing(int spacing);

  /**
	If enable is TRUE, displays a vertical scroll bar, else hides it.                                        
	*/
  void setScrollBar(bool enable);

  /**
	If enable is TRUE (default), item text will be displayed, otherwise 
	it will not, and no highlight will be shown in the default widget.
	*/
  void setShowItemText(bool enable);

  /**
	If enable is TRUE, enables smooth scrolling, else disables 
	it (default).
	*/
  void setSmoothScrolling(bool enable);

  /**
	If enable is TRUE (default), lines depicting the structure of the
	tree will be drawn, otherwise they will not.
	*/
  void setTreeDrawing(bool enable);

  /**
	Indicates whether item text is displayed.
	*/
  bool showItemText() const;

  /**
	Returns a bool value indicating whether smooth scrolling is enabled.
	*/
  bool smoothScrolling() const;

  /**
	Indents the item at the specified index, creating a new branch.
	*/
  void split(int index);

  /**
	Same as above but uses a path to specify the item.                    
	*/
  void split(const KPath *path);

  /**
	Removes the item at the given index from the tree, but does not
	delete it, returning a pointer to the removed item.
	*/
  ProcTreeItem *takeItem(int index);

  /**
	Same as above but uses a path to specify the item to take.
	*/
  ProcTreeItem *takeItem(const KPath *path);

  /**
	Indicates whether the tree structure is drawn.
	*/
  bool treeDrawing() const;

  /**
	Returns the number of items that are visible (their parents are
	expanded).                   
	*/
  int visibleCount();

  signals:
  void collapsed(int index);
  void expanded(int index);
  void highlighted(int index);
  void selected(int index);
  void clicked(QMouseEvent*);

protected:
  void addChildItem(ProcTreeItem *theParent,
		   ProcTreeItem *theChild);
  virtual int cellHeight(int row);
  void changeItem(ProcTreeItem *toChange,
		  int itemIndex,
		  const char *newText,
		  const QPixmap *newPixmap);
  bool checkItemPath(const KPath *path) const;
  bool checkItemText(const char *text) const;
  void collapseSubTree(ProcTreeItem *subRoot);
  bool countItem(ProcTreeItem *item,void *total);
  void expandOrCollapse(ProcTreeItem *parentItem);
  void expandSubTree(ProcTreeItem *subRoot);
  bool findItemAt(ProcTreeItem *item,void *user);
  void fixChildBranches(ProcTreeItem *parentItem);
  virtual void focusInEvent(QFocusEvent *e);
  void forEveryItem(KForEveryM func, void *user);
  void forEveryVisibleItem(KForEveryM func,void *user);
  bool getItemIndex(ProcTreeItem *item,void *user);
  bool getMaxItemWidth(ProcTreeItem *item,void *user);
  void insertItem(ProcTreeItem *referenceItem,
		  ProcTreeItem *newItem,
		  bool prefix);
  void join(ProcTreeItem *item);
  virtual void keyPressEvent(QKeyEvent *e);
  void lowerItem(ProcTreeItem *item);
  virtual void mouseDoubleClickEvent(QMouseEvent *e);
  virtual void mouseMoveEvent(QMouseEvent *e);
  virtual void mousePressEvent(QMouseEvent *e);
  virtual void mouseReleaseEvent(QMouseEvent *e);
  virtual void paintCell(QPainter *p, int row, int col);
  virtual void paintHighlight(QPainter *p,ProcTreeItem *item);
  virtual void paintItem(QPainter *p, ProcTreeItem *item, 
			 bool highlighted);
  void raiseItem(ProcTreeItem *item);
  ProcTreeItem *recursiveFind(ProcTreeItem *subRoot,KPath *path);
  bool setItemExpanded(ProcTreeItem *item, void *);
  bool setItemExpandButtonDrawing(ProcTreeItem *item, void *);
  bool setItemIndent(ProcTreeItem *item, void *);
  bool setItemShowText(ProcTreeItem *item, void *);
  bool setItemTreeDrawing(ProcTreeItem *item, void *);
  void split(ProcTreeItem *item);
  void takeItem(ProcTreeItem *item);
  virtual void updateCellWidth();
  ProcTreeItem *treeRoot;
  bool clearing;
  int current;
  bool drawExpandButton;
  bool drawTree;
  int expansion;
  bool goingDown;
  int indent;
  int maxItemWidth;
  bool showText;

  // Rainer Bawidamann: move window in "rubberband" mode
  bool rubberband_mode;             // true if in "rubberband_mode"
  QPoint rubber_startMouse;         // where the user pressed the MMB
  int rubber_height, rubber_width,  // the size if the rubberband rect
	rubber_startX, rubber_startY; // the x/yOffset() when the MMB was pressed
  void draw_rubberband();
  void start_rubberband(const QPoint& where);
  void end_rubberband();
  void move_rubberband(const QPoint& where);
};

#endif // PTREE_H
