/*
    KTop, a taskmanager and cpu load monitor
   
    Copyright (C) 1997 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu

    Copyright (C) 1998 Nicolas Leclercq
                       nicknet@planete.net
    
	Copyright (c) 1999 Chris Schlaeger
	                   cs@axys.de
    
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

#include <qlabel.h>
#include <qlcdnum.h>
#include <qslider.h>
#include <qpushbutton.h>

#include <kapp.h>

#include "ReniceDlg.moc"

ReniceDlg::ReniceDlg(QWidget* parent, const char* name, int currentPPrio)
	: QDialog(parent, name, TRUE)
{
	QLabel* label0 = new QLabel(i18n("Please enter desired priority:"), this);
	label0->setGeometry(10, 10, 210, 15);

	QSlider* priority = new QSlider(-20, 20, 1, 0, QSlider::Horizontal,
									this, "prio" );
	priority->setGeometry(10, 35, 210, 25);
	priority->setTickmarks((QSlider::TickSetting) 2);
	priority->setFocusPolicy(QWidget::TabFocus);
	priority->setFixedHeight(priority->sizeHint().height());

	QLCDNumber* lcd0= new QLCDNumber(3, this, "lcd");
	lcd0->setGeometry(80, 65, 70, 30);
	QObject::connect(priority, SIGNAL(valueChanged(int)), lcd0,
					 SLOT(display(int)));
	QObject::connect(priority, SIGNAL(valueChanged(int)),
					 SLOT(setPriorityValue(int)));

	QPushButton* ok = new QPushButton(i18n("OK"), this);
	ok->setGeometry(10, 110, 100, 30);
	connect(ok, SIGNAL(clicked()), SLOT(ok()));

	QPushButton* cancel = new QPushButton(i18n("Cancel"), this);
	cancel->setGeometry(120, 110, 100, 30);
	connect(cancel, SIGNAL(clicked()), SLOT(cancel()));

	value = currentPPrio;
	priority->setValue(value);
	lcd0->display(value);
}
