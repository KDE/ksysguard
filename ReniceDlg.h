/*
    KTop, a taskmanager and cpu load monitor
   
	Copyright (c) 1999 Chris Schlaeger
	                   cs@kde.org
    
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

// $Id$

#ifndef _ReniceDlg_h_
#define _ReniceDlg_h_

/*
 * kapp.h includes a dirty X.h file that contains marcos that collide with
 * qslider.h. The following defines work around the problem.
 */
#ifdef Above
#undef Above
#endif
#ifdef Below
#undef Below
#endif

#include <qlabel.h>
#include <qlcdnum.h>
#include <qslider.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qlayout.h>

/**
 * This class creates and handles a simple dialog to change the scheduling
 * priority of a process.
 */
class ReniceDlg : public QDialog
{
	Q_OBJECT

public:
	ReniceDlg(QWidget* parent, const char* name, int currentPPrio, int pid);
	~ReniceDlg()
	{
		delete message;

		delete slider;
		delete lcd;

		delete okButton;
		delete cancelButton;
		
		delete vLay;
	}

public slots:
	void ok()
	{
		done(value);
	}

	void cancel()
	{
		done(40);
	}

private:
	int value;

	QBoxLayout* vLay;
	QBoxLayout* butLay;
	QBoxLayout* sldLay;

	QLabel* message;

	QSlider* slider;
	QLCDNumber* lcd;

	QPushButton* okButton;
	QPushButton* cancelButton;

private slots:
	void setPriorityValue(int priority)
	{
		value = priority;
	}
};

#endif
