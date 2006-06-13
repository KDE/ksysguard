/*
    KSysGuard, the KDE System Guard

	Copyright (c) 1999 Chris Schlaeger <cs@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.


*/

#include <klocale.h>

#include "ReniceDlg.moc"
//Added by qt3to4:
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

ReniceDlg::ReniceDlg(QWidget* parent, const char* name, int currentPPrio,
					 int pid)
	: KDialog( parent )
{
  setObjectName( name );
  setModal( true );
  setCaption( i18n("Renice Process") );
  setButtons( Ok | Cancel );
  enableButtonSeparator( true );

  connect( this, SIGNAL( okClicked() ), SLOT( slotOk() ) );
  connect( this, SIGNAL( cancelClicked() ), SLOT( slotCancel() ) );

	value = currentPPrio;

  QWidget *page = new QWidget( this );
  setMainWidget(page);

	vLay = new QVBoxLayout(page);
	vLay->setObjectName("ReniceLayout");
	vLay->setSpacing(-1);
	vLay->setMargin(20);

	QString msg;
	msg = i18n("You are about to change the scheduling priority of\n"
			   "process %1. Be aware that only the Superuser (root)\n"
			   "can decrease the nice level of a process. The lower\n"
			   "the number is the higher the priority.\n\n"
			   "Please enter the desired nice level:", pid);
	message = new QLabel(msg, page);
	message->setMinimumSize(message->sizeHint());
	vLay->addWidget(message);

	/*
	 * Create a slider with an LCD display to the right using a horizontal
	 * layout. The slider and the LCD are kept in sync through signals
	 */
	sldLay = new QHBoxLayout();
	vLay->addLayout(sldLay);

	slider = new QSlider(Qt::Horizontal, page ); 
	slider->setMinimum(-20);
	slider->setMaximum(19);
	slider->setPageStep(1);
	slider->setObjectName("prio");
	slider->setMaximumSize(210, 25);
	slider->setMinimumSize(210, 25);
	slider->setTickPosition(QSlider::TicksBelow);
	slider->setFocusPolicy(Qt::TabFocus);
	slider->setFixedHeight(slider->sizeHint().height());
	slider->setValue(value);
	sldLay->addWidget(slider);
	sldLay->addSpacing(10);

	lcd = new QLCDNumber(3, page);
	lcd->setMaximumSize(55, 23);
	lcd->setMinimumSize(55, 23);
	lcd->display(value);
	QObject::connect(slider, SIGNAL(valueChanged(int)), lcd,
					 SLOT(display(int)));
	QObject::connect(slider, SIGNAL(valueChanged(int)),
					 SLOT(setPriorityValue(int)));
	sldLay->addWidget(lcd);
	vLay->activate();
}

void ReniceDlg::slotOk()
{
  done(value);
}

void ReniceDlg::slotCancel()
{
  done(40);
}
