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

#ifndef _ReniceDlg_h_
#define _ReniceDlg_h_

#include <kdialog.h>
#include <QLabel>
#include <QLayout>
#include <qlcdnumber.h>
#include <QPushButton>
#include <QSlider>
#include <QBoxLayout>

class KIntNumInput;

/**
 * This class creates and handles a simple dialog to change the scheduling
 * priority of a process.
 */
class ReniceDlg : public KDialog
{
	Q_OBJECT

public:
	ReniceDlg(QWidget* parent, int currentPPrio, const QStringList& pid);
	int newPriority;

public Q_SLOTS:
    void slotOk();

private:
	QBoxLayout* vLay;
	QBoxLayout* butLay;
	QBoxLayout* sldLay;

	QLabel* message;
	KIntNumInput* input;
};

#endif
