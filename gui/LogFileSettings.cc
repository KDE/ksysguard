/*
    KSysGuard, the KDE System Guard
   
	Copyright (c) 2001 Tobias Koenig <tokoe82@yahoo.de>
    
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
*/

#include <kfontdialog.h>
#include <klocale.h>

#include <qlabel.h>
#include <qlayout.h>

#include "LogFileSettings.moc"

LogFileSettings::LogFileSettings(QWidget *parent, const char *name)
	: QDialog(parent, name, TRUE)
{
	QVBoxLayout *tablayout = new QVBoxLayout(this, 2);
	CHECK_PTR(tablayout);

	tabWidget = new QTabWidget(this);
	CHECK_PTR(tabWidget);
	actionButtons = new KButtonBox(this);
	CHECK_PTR(actionButtons);

	actionButtons->addStretch(1);
	actionButtons->addButton(i18n("&Ok"), this, SLOT(accept()), FALSE);
	actionButtons->addButton(i18n("&Apply"), parent, SLOT(applySettings()), FALSE);
	actionButtons->addButton(i18n("&Cancel"), this, SLOT(reject()), FALSE);
	actionButtons->addStretch(1);

	tablayout->addWidget(tabWidget);
	tablayout->addWidget(actionButtons);

	// text sheet
	QWidget *textWidget = new QWidget(tabWidget);
	CHECK_PTR(textWidget);

	QVBoxLayout *layout1 = new QVBoxLayout(textWidget, 3);
	CHECK_PTR(layout1);
	QHBoxLayout *layout2 = new QHBoxLayout(textWidget, 3);
	CHECK_PTR(layout2);

	fgColorPicker = new ColorPicker(textWidget);
	CHECK_PTR(fgColorPicker);
	fgColorPicker->setText(i18n("Foreground Color"));
	bgColorPicker = new ColorPicker(textWidget);
	CHECK_PTR(bgColorPicker);
	bgColorPicker->setText(i18n("Background Color"));

	fontButton = new QPushButton(i18n("Font"), textWidget);
	CHECK_PTR(fontButton);
	fontButton->setFixedSize(100, 30);

	QLabel *fontLabel = new QLabel(i18n("Font"), textWidget);
	CHECK_PTR(fontLabel);

	layout1->addWidget(fgColorPicker);
	layout1->addWidget(bgColorPicker);
	layout1->addLayout(layout2);

	layout2->addWidget(fontLabel);
	layout2->addStretch(1);
	layout2->addWidget(fontButton);

	connect(fontButton, SIGNAL(clicked()), this, SLOT(slotFontSelection()));

	tabWidget->addTab(textWidget, i18n("Text"));

	// filter sheet
	QWidget *filterWidget = new QWidget(tabWidget);
	CHECK_PTR(filterWidget);

	QHBoxLayout *layout5 = new QHBoxLayout(filterWidget, 2);
	CHECK_PTR(layout5);
	QVBoxLayout *layout6 = new QVBoxLayout(filterWidget, 2);
	CHECK_PTR(layout6);
	
	layout5->addLayout(layout6);
	
	ruleButtons = new KButtonBox(filterWidget, KButtonBox::Vertical);
	CHECK_PTR(ruleButtons);
	ruleButtons->addButton(i18n("Add"), this, SLOT(slotAddRule()), FALSE);
	ruleButtons->addButton(i18n("Delete"), this, SLOT(slotDelRule()), FALSE);
	ruleButtons->addButton(i18n("Change"), this, SLOT(slotChangeRule()), FALSE);
	ruleButtons->layout();
	layout5->addWidget(ruleButtons);

	filterText = new QLineEdit(filterWidget);
	CHECK_PTR(filterText);
	layout6->addWidget(filterText);

	ruleList = new QListBox(filterWidget);
	CHECK_PTR(ruleList);
	layout6->addWidget(ruleList);

	connect(ruleList, SIGNAL(selected(int)), this, SLOT(slotRuleListSelected(int)));

	tabWidget->addTab(filterWidget, i18n("Filter"));
}

void LogFileSettings::slotFontSelection(void)
{
	if (KFontDialog::getFont(textFont) == KFontDialog::Accepted) {
		fontButton->setFont(textFont);
	}
}

void LogFileSettings::slotAddRule(void)
{
	if (!filterText->text().isEmpty()) {
		ruleList->insertItem(filterText->text(), -1);
		filterText->setText("");
	}
}

void LogFileSettings::slotDelRule(void)
{
	ruleList->removeItem(ruleList->currentItem());
	filterText->setText("");
}

void LogFileSettings::slotChangeRule(void)
{
	ruleList->changeItem(filterText->text(), ruleList->currentItem());
	filterText->setText("");
}

void LogFileSettings::slotRuleListSelected(int index)
{
	filterText->setText(ruleList->text(index));
}
