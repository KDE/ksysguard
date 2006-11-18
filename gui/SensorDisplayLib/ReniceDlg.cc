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
#include <knuminput.h>

#include "ReniceDlg.moc"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

ReniceDlg::ReniceDlg(QWidget* parent, int currentPPrio,
					 const QStringList& processes)
	: KDialog( parent )
{
	setObjectName( "Renice Dialog" );
	setModal( true );
	setCaption( i18n("Renice Process") );
	setButtons( Ok | Cancel );
	showButtonSeparator( true );

	connect( this, SIGNAL( okClicked() ), SLOT( slotOk() ) );


	QWidget *page = new QWidget( this );
	setMainWidget(page);

	vLay = new QVBoxLayout(page);
	vLay->setObjectName("ReniceLayout");
	vLay->setSpacing(-1);
	vLay->setMargin(20);

	QString msg;
	msg = i18n("<qt>You are about to change the scheduling priority for:<br>"
			   "<i>%1</i><br>"
			   "Be aware that only the Superuser (root) "
			   "can decrease the nice level of a process. The lower "
			   "the number is the higher the priority. "
			   "Please enter the desired nice level:", processes.join("\n"));
	message = new QLabel(msg, page);
	message->setWordWrap(true);
	message->setMinimumSize(message->sizeHint());
	vLay->addWidget(message);

	sldLay = new QHBoxLayout();
	vLay->addLayout(sldLay);

	input = new KIntNumInput(currentPPrio, page, 10);
	input->setRange(-20, 19);
	vLay->addWidget(input);
	newPriority = 40;
}

void ReniceDlg::slotOk()
{
  newPriority = input->value();
}

