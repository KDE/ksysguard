/*
    KSysGuard, the KDE System Guard
   
	Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>
    
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

	KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
	Please do not commit any changes without consulting me first. Thanks!

	$Id$
*/

#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include <kcolordialog.h>
#include <klocale.h>

#include "ColorPicker.moc"

ColorPicker::ColorPicker(QWidget* parent, const char* name)
	: QWidget(parent, name)
{
	l = new QHBoxLayout(this);
	label = new QLabel(this, "label");
	l->addWidget(label);
	l->addSpacing(8);

	box = new QFrame(this, "box");
	box->setFixedSize(16, 16);
	box->setFrameShape(QFrame::WinPanel);
	box->setFrameShadow(QFrame::Sunken);
	l->addWidget(box);
	l->addSpacing(8);

	button = new QPushButton(this, "button");
	button->setText(i18n("Change Color"));
	l->addWidget(button);

	label->setBuddy(button);

	connect(button, SIGNAL(clicked()), this, SLOT(colorDialog()));
}

void
ColorPicker::colorDialog()
{
	QColor col = getColor();
	if (KColorDialog::getColor(col, this->parentWidget()) == KColorDialog::Accepted)
		setColor(col);
}

QString 
ColorPicker::getText() const
{
	return (label->text());
}

void
ColorPicker::setText(const QString& t)
{
	label->setText(t);
}

QColor
ColorPicker::getColor() const
{
	return (box->palette().color(QPalette::Normal,
								 QColorGroup::Background));
}

void
ColorPicker::setColor(const QColor& c)
{
	QPalette cp = box->palette();
	cp.setColor(QPalette::Normal, QColorGroup::Background, c);
	box->setPalette(cp);
}
