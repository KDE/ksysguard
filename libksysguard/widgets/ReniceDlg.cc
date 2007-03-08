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
#include <QListWidget>
#include <QSpinBox>
#include "ui_ReniceDlgUi.h"

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

	QWidget *widget = new QWidget(this);
	setMainWidget(widget);

	reniceDlgUi = new Ui_ReniceDlgUi();
	reniceDlgUi->setupUi(widget);
	reniceDlgUi->listWidget->insertItems(0, processes);
	reniceDlgUi->spinBoxPriority->setValue(currentPPrio);
       	newPriority = 40;
}

void ReniceDlg::slotOk()
{
  newPriority = reniceDlgUi->spinBoxPriority->value();
}

