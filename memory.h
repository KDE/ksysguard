/*
    KTop, a taskmanager and cpu load monitor
   
    Copyright (C) 1997 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu

    Copyright (C) 1998 Nicolas Leclercq
                       nicknet@planete.net
    
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

/*=============================================================================
  CLASSes
 =============================================================================*/
//-----------------------------------------------------------------------------
// class  : MemMon
//-----------------------------------------------------------------------------
class MemMon : public QWidget
{
    Q_OBJECT

public:     
     MemMon (QWidget *parent = 0, const char *name = 0, QWidget *child = 0);
    ~MemMon ();

protected:

    virtual void paintEvent(QPaintEvent *);
    virtual void timerEvent(QTimerEvent *);

    int      intervals,
             mem_size, 
             physsize,
            *mem_values;
    QWidget *my_child;
    QBrush   brush_0, 
             brush_1;
};
