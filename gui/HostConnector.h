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

#ifndef _HostConnector_h_
#define _HostConnector_h_

#include <qdialog.h>

class QLabel;
class QPushButton;
class QBoxLayout;
class QLineEdit;
class QRadioButton;

/**
 * This class creates and handles a simple dialog to establish a connection
 * with a new remote host.
 */
class HostConnector : public QDialog
{
	Q_OBJECT

public:
	HostConnector(QWidget* parent, const char* name);
	~HostConnector();

public slots:
	void okClicked();
	void cancelClicked();
	void sshClicked();
	void rshClicked();
	void otherClicked();

private:
	void createTextEntry(const QString& label, QLabel*& lbl, QLineEdit*& le);

	QBoxLayout* vLay;

	QLabel* hostLbl;
	QLineEdit* hostLE;
	QRadioButton* sshButton;
	QRadioButton* rshButton;
	QRadioButton* otherButton;
	QLabel* commandLbl;
	QLineEdit* commandLE;
	QPushButton* okButton;
	QPushButton* cancelButton;
};

#endif
