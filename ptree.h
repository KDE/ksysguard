/*
    Most of the code is :
    Copyright (C) 1996 Keith Brown and KtSoft

    Copyright (C) 1998 Nicolas Leclercq
                  nicknet@planete.net
    kdeui=>ktstreelist modified for ktop. 

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef PROCTREE_H
#define PROCTREE_H

/*=============================================================================
  HEADERs
 =============================================================================*/
#include <qapp.h>       // used for QApplication::closingDown()
#include <qkeycode.h>   // used for keyboard interface
#include <qpainter.h>   // used to paint items
#include <qpixmap.h>	// used in items
#include <qstack.h>	// used to specify tree paths
#include <qstring.h>	// used in items
#include <qtablevw.h>	// base class for widget

#include "comm.h"     

/*=============================================================================
  #DEFINEs
 =============================================================================*/
#define OUTPUT_WIDTH 132

/*=============================================================================
  TYPEDEFs
 =============================================================================*/
typedef QStack<QString> KPath;
typedef struct {
  char  name[COMM_LEN+1];
  int   pid;
  int   ppid;
  int   uid;
  char  uname[256];
  char  arg[OUTPUT_WIDTH+1];
} ProcInfo;

class ProcTree;
class ProcTreeItem;
typedef bool (ProcTree::*KForEveryM) (ProcTreeItem*, void*);
typedef bool (*KForEvery)            (ProcTreeItem*, void*);

//-----------------------------------------------------------------------------
// class  : KtopIconList
//-----------------------------------------------------------------------------
class ProcTreeItem
{
public:
                 ProcTreeItem(const ProcInfo info, const QPixmap  *thePixmap = 0);
  virtual       ~ProcTreeItem();

  bool           drawExpandButton() const;
  void           setDrawExpandButton(bool doit);
  bool           drawText() const;
  void           setDrawText(bool doit);
  bool           drawTree() const;
  void           setDrawTree(bool doit);

  bool           isExpanded() const;
  void           setExpanded(bool is);

  int            getBranch() const;
  void           setBranch(int level);

  int            getIndent() const;
  void           setIndent(int value);

  bool           hasParent() const;
  ProcTreeItem  *getParent();
  void           setParent(ProcTreeItem *newParent);

  bool           hasChild() const;
  uint           childCount() const;
  int            childIndex(ProcTreeItem *child);
  ProcTreeItem  *childAt(int index);
  ProcTreeItem  *getChild();
  void           setChild(ProcTreeItem *newChild);
  void           insertChild(int index,ProcTreeItem *newItem);
  void           appendChild(ProcTreeItem *newChild);
  void           removeChild(ProcTreeItem *child);

  bool           hasSibling() const;
  ProcTreeItem  *getSibling();
  void           setSibling(ProcTreeItem *newSibling);

  const int      getParentId();
  const int      getProcId();
  const char*    getProcName();
  const int      getProcUid();
  const ProcInfo getProcInfo();

  const QPixmap *getPixmap() const;
  void           setPixmap(const QPixmap *pm);

  const char    *getText() const;
  void           setText(const char *t);
  void           setSortPidText(void);
  void           setSortNameText(void);
  void           setSortUidText(void);

  virtual int    height(const ProcTree *theOwner) const;
  virtual int    width(const ProcTree *theOwner) const;
  virtual QRect  textBoundingRect(const QFontMetrics& fm) const;
  virtual QRect  boundingRect(const QFontMetrics& fm) const;
  virtual void   paint(QPainter *p,const QColorGroup& cg,bool highlighted);

  bool           expandButtonClicked(const QPoint& coord) const;

protected:
  virtual int    itemHeight(const QFontMetrics& fm) const;
  virtual int    itemWidth(const QFontMetrics& fm) const;

private:
  int            branch;
  int            indent;
  int            numChildren;
  bool           doExpandButton;
  bool           expanded;
  bool           doTree;
  bool           doText;
  QRect          expandButton;
  ProcTreeItem  *child;
  ProcTreeItem  *parent;
  ProcTreeItem  *sibling;
  QPixmap        pixmap;
  QString        text;
  ProcInfo       pInfo;
};


//-----------------------------------------------------------------------------
// class  : ProcTree
//-----------------------------------------------------------------------------
struct KItemSearchInfo {
  int count;
  int index;
  ProcTreeItem *foundItem;
  KItemSearchInfo() : count(0), index(0), foundItem(0) {}
};


class ProcTree : public QTableView
{
  Q_OBJECT

public:
           ProcTree(QWidget *parent=0, const char *name=0, WFlags f= 0);
  virtual ~ProcTree();


  void addChildItem(const ProcInfo pi,const QPixmap *thePixmap
                   ,int index);
  void addChildItem(const ProcInfo pi,const QPixmap *thePixmap
                   ,const KPath *thePath); 
  void addChildItem(ProcTreeItem *newItem,int index);
  void addChildItem(ProcTreeItem *newItem,const KPath *thePath);                                                             

  void insertItem(const ProcInfo pi,const QPixmap *thePixmap,
                  int index=-1,bool prefix=TRUE);
  void insertItem(const ProcInfo pi,const QPixmap *thePixmap,
                  const KPath *thePath,bool prefix = TRUE);
  void insertItem(ProcTreeItem *newItem,int index = -1,
                  bool prefix = TRUE); 
  void insertItem(ProcTreeItem *newItem,const KPath *thePath,
                  bool prefix = TRUE);

  void changeItem(const ProcInfo pi,const QPixmap *newPixmap,
                  int index);
  void changeItem(const ProcInfo pi,const QPixmap *newPixmap,
                  const KPath *thePath);

  void clear();
  uint count();
  int  visibleCount();

  void collapseItem(int index);
  void expandItem(int index);
  int  expandLevel() const;
  void expandOrCollapseItem(int index);

  void forEveryItem(KForEvery func,void *user);
  void forEveryVisibleItem(KForEvery func,void *user);

  int           currentItem() const;
  ProcTreeItem *getCurrentItem();
  ProcTreeItem *itemAt(int index);
  ProcTreeItem *itemAt(const KPath *path);
  int           itemIndex(ProcTreeItem *item);
  int           itemVisibleIndex(ProcTreeItem *item);

  int    indentSpacing();

  KPath *itemPath(int index);

  void join(int index);
  void join(const KPath *path);

  void lowerItem(int index);
  void lowerItem(const KPath *path);
  void raiseItem(int index);
  void raiseItem(const KPath *path);

  void removeItem(int index);
  void removeItem(const KPath *thePath);

  bool autoScrollBar() const;
  bool autoBottomScrollBar() const;
  bool autoUpdate() const;
  void setAutoUpdate(bool enable);
  bool bottomScrollBar() const;
  void setBottomScrollBar(bool enable);
  bool scrollBar() const;
  void setScrollBar(bool enable);
  bool smoothScrolling() const;
  void setSmoothScrolling(bool enable);

  void setCurrentItem(int index);

  void setExpandButtonDrawing(bool enable);

  void setExpandLevel(int level);

  void setIndentSpacing(int spacing);

  bool showItemText() const;
  void setShowItemText(bool enable);

  bool treeDrawing() const;
  void setTreeDrawing(bool enable);

  void split(int index);
  void split(const KPath *path);

  ProcTreeItem *takeItem(int index);
  ProcTreeItem *takeItem(const KPath *path);

signals:
  void collapsed(int index);
  void expanded(int index);
  void highlighted(int index);
  void selected(int index);
  void clicked(QMouseEvent*);

protected:
  void 		addChildItem(ProcTreeItem *theParent,ProcTreeItem *theChild);
  virtual int 	cellHeight(int row);
  void 		changeItem(ProcTreeItem *toChange,int itemIndex,
                           const char *newText,const QPixmap *newPixmap);
  bool 		checkItemPath(const KPath *path) const;
  bool 		checkItemText(const char *text) const;
  void 		collapseSubTree(ProcTreeItem *subRoot);
  bool 		countItem(ProcTreeItem *item,void *total);
  void 		expandOrCollapse(ProcTreeItem *parentItem);
  void 		expandSubTree(ProcTreeItem *subRoot);
  bool 		findItemAt(ProcTreeItem *item,void *user);
  void 		fixChildBranches(ProcTreeItem *parentItem);
  virtual void focusInEvent(QFocusEvent *e);
  void 		forEveryItem(KForEveryM func, void *user);
  void 		forEveryVisibleItem(KForEveryM func,void *user);
  bool 		getItemIndex(ProcTreeItem *item,void *user);
  bool 		getMaxItemWidth(ProcTreeItem *item,void *user);
  void 		insertItem(ProcTreeItem *referenceItem,
                           ProcTreeItem *newItem,bool prefix);
  void 		join(ProcTreeItem *item);
  virtual void 	keyPressEvent(QKeyEvent *e);
  void 		lowerItem(ProcTreeItem *item);
  virtual void 	mouseDoubleClickEvent(QMouseEvent *e);
  virtual void 	mouseMoveEvent(QMouseEvent *e);
  virtual void 	mousePressEvent(QMouseEvent *e);
  virtual void 	mouseReleaseEvent(QMouseEvent *e);
  virtual void 	paintCell(QPainter *p, int row, int col);
  virtual void 	paintHighlight(QPainter *p,ProcTreeItem *item);
  virtual void 	paintItem(QPainter *p, ProcTreeItem *item,bool highlighted);
  void 		raiseItem(ProcTreeItem *item);
  ProcTreeItem *recursiveFind(ProcTreeItem *subRoot,KPath *path);
  bool 		setItemExpanded(ProcTreeItem *item, void *);
  bool 		setItemExpandButtonDrawing(ProcTreeItem *item, void *);
  bool 		setItemIndent(ProcTreeItem *item, void *);
  bool 		setItemShowText(ProcTreeItem *item, void *);
  bool 		setItemTreeDrawing(ProcTreeItem *item, void *);
  void          split(ProcTreeItem *item);
  void          takeItem(ProcTreeItem *item);
  virtual void	updateCellWidth();
  void          draw_rubberband();
  void          start_rubberband(const QPoint& where);
  void          end_rubberband();
  void          move_rubberband(const QPoint& where);

  ProcTreeItem  *treeRoot;
  bool          clearing;
  int           current;
  bool          drawExpandButton;
  bool          drawTree;
  int           expansion;
  bool          goingDown;
  int           indent;
  int           maxItemWidth;
  bool          showText;
  bool          rubberband_mode;            
  QPoint        rubber_startMouse;         
  int           rubber_height, 
                rubber_width,  
                rubber_startX, 
                rubber_startY;
  bool          updating,
                restoreStartupPage,
                mouseRightButDown;
  int           lastSelectionPid;
};

#endif // PTREE_H
