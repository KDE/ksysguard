/*
    KSysGuard, the KDE System Guard
   
	Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>
    
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

	KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!

	$Id$
*/

#ifndef _WorkSheetSetup_h_
#define _WorkSheetSetup_h_

#include "kdialogbase.h"

class QLineEdit;
class KIntNumInput;

class WorkSheetSetup : public KDialogBase
{
	Q_OBJECT

public:
	WorkSheetSetup(const QString& defSheetName);
	~WorkSheetSetup()
	{
		delete mainWidget;
	}

	QString getSheetName() const;

	int getRows() const;

	int getColumns() const;

private:
	QLineEdit* sheetNameLE;
	KIntNumInput* rowNI;
	KIntNumInput* colNI;
	QWidget* mainWidget;
} ;

#endif
