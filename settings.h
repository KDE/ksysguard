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
  CONSTs
 =============================================================================*/
const int SET_MAX_SORT=4;

/*=============================================================================
  CLASS(es)
 =============================================================================*/
//-----------------------------------------------------------------------------
// class  : AppSettings : public QDialog
//-----------------------------------------------------------------------------
class AppSettings : public QDialog
{
    Q_OBJECT
public:
     AppSettings(QWidget *parent = 0, const char *name = 0);
    ~AppSettings();

    void setStartUpPage(int);
    int  getStartUpPage();
    
protected:
    QButtonGroup *startuppage;
    QPushButton  *ok, *cancel;
    QRadioButton *rb_pList, *rb_pTree, *rb_Perf;
    virtual void resizeEvent(QResizeEvent *);
    
public slots:
    void doValidate();
};


