/*
    KTop, the KDE Task Manager and System Monitor
   
	Copyright (c) 1999 Chris Schlaeger <cs@kde.org>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	KTop is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!

	$Id$
*/

#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>

#include <klocale.h>

#include "HostConnector.h"
#include "SensorManager.h"
#include "HostConnector.moc"

HostConnector::HostConnector(QWidget* parent, const char* name) :
	QDialog(parent, name, TRUE)
{
	vLay = new QVBoxLayout(this, 20, -1, "ReniceLayout");
	CHECK_PTR(vLay);

	/*
	 * Create an "OK" and a "Cancel" button in a horizontal layout.
	 */
	QBoxLayout* subLay;

	// Host entry line
	createTextEntry(i18n("Host:"), hostLbl, hostLE);

	// shell selection radio buttons
	subLay = new QHBoxLayout();
	CHECK_PTR(subLay);
	vLay->addLayout(subLay);
	subLay->addStretch(1);

	sshButton = new QRadioButton(i18n("ssh"), this);
	CHECK_PTR(sshButton);
	subLay->addWidget(sshButton);
	subLay->addStretch(1);
	
	rshButton = new QRadioButton(i18n("rsh"), this);
	CHECK_PTR(rshButton);
	subLay->addWidget(rshButton);
	subLay->addStretch(1);
	
	otherButton = new QRadioButton(i18n("other"), this);
	CHECK_PTR(otherButton);
	subLay->addWidget(otherButton);
	subLay->addStretch(1);
	
	// Command entry line
	createTextEntry(i18n("Command:"), commandLbl, commandLE);

	// OK/Cancel button row
	subLay = new QHBoxLayout();
	CHECK_PTR(subLay);
	vLay->addLayout(subLay);
	subLay->addStretch(1);

	connect(sshButton, SIGNAL(clicked()), SLOT(sshClicked()));
	connect(rshButton, SIGNAL(clicked()), SLOT(rshClicked()));
	connect(otherButton, SIGNAL(clicked()), SLOT(otherClicked()));

	okButton = new QPushButton(i18n("&OK"), this);
	CHECK_PTR(okButton);
	okButton->setMaximumSize(100, 30);
	okButton->setMinimumSize(100, 30);
	connect(okButton, SIGNAL(clicked()), SLOT(okClicked()));
	subLay->addWidget(okButton);
	subLay->addStretch(1);

	sshClicked();

	cancelButton = new QPushButton(i18n("&Cancel"), this);
	CHECK_PTR(cancelButton);
	cancelButton->setMaximumSize(100, 30);
	cancelButton->setMinimumSize(100, 30);
	connect(cancelButton, SIGNAL(clicked()), SLOT(cancelClicked()));
	subLay->addWidget(cancelButton);
	subLay->addStretch(1);

	vLay->activate();
}

HostConnector::~HostConnector()
{
	delete hostLbl;
	delete hostLE;
	delete sshButton;
	delete rshButton;
	delete otherButton;
	delete commandLbl;
	delete commandLE;
	delete okButton;
	delete cancelButton;
	
	delete vLay;
}

void
HostConnector::createTextEntry(const QString& label, QLabel*& lbl,
							   QLineEdit*& le)
{
	QBoxLayout* subLay;

	subLay = new QHBoxLayout();
	CHECK_PTR(subLay);
	vLay->addLayout(subLay);

	lbl = new QLabel(label, this);
	CHECK_PTR(lbl);
	subLay->addWidget(lbl);

	le = new QLineEdit(this);
	CHECK_PTR(le);
	subLay->addWidget(le);
}

void 
HostConnector::okClicked()
{
	QString shell;
	QString command;

	if (sshButton->isChecked())
	{
		shell = "ssh";
		command = "";
	}
	else if (rshButton->isChecked())
	{
		shell = "rsh";
		command = "";
	}
	else
	{
		shell = "";
		command = commandLE->text();
	}
	SensorMgr->engage(hostLE->text(), shell, command);

	done(1);
}

void
HostConnector::cancelClicked()
{
	done(0);
}

void
HostConnector::sshClicked()
{
	rshButton->setChecked(false);
	otherButton->setChecked(false);
	commandLE->setEnabled(false);
}

void
HostConnector::rshClicked()
{
	sshButton->setChecked(false);
	otherButton->setChecked(false);
	commandLE->setEnabled(false);
}

void
HostConnector::otherClicked()
{
	sshButton->setChecked(false);
	rshButton->setChecked(false);
	commandLE->setEnabled(true);
}
