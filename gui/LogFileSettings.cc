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

#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>

#include "LogFileSettings.moc"

#include <stdio.h>

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
	actionButtons->addStretch(1);
	actionButtons->addButton(i18n("&Cancel"), this, SLOT(reject()), FALSE);
	actionButtons->addStretch(1);

	tablayout->addWidget(tabWidget);
	tablayout->addWidget(actionButtons);

	// text sheet
	QWidget *textWidget = new QWidget(tabWidget);
	CHECK_PTR(textWidget);

	QVBoxLayout *textLayout = new QVBoxLayout(textWidget, 3);
	CHECK_PTR(textLayout);

	QGroupBox *titleFrame = new QGroupBox(textWidget, "titleFrame");
	CHECK_PTR(titleFrame);
	titleFrame->setColumnLayout(0, Qt::Vertical );
	titleFrame->layout()->setMargin(10);
	titleFrame->setTitle(i18n("Title"));

	QVBoxLayout *vbox1 = new QVBoxLayout(titleFrame->layout());
	CHECK_PTR(vbox1);

	titleText = new QLineEdit(titleFrame, "titleText");
	CHECK_PTR(titleText);
	vbox1->addWidget(titleText);

	QGroupBox *colorFrame = new QGroupBox(textWidget, "colorFrame");
	CHECK_PTR(colorFrame);
	colorFrame->setColumnLayout(0, Qt::Vertical );
	colorFrame->layout()->setMargin(10);
	colorFrame->setTitle(i18n("Colors"));

	QVBoxLayout *vbox2 = new QVBoxLayout(colorFrame->layout());
	CHECK_PTR(vbox2);

	fgColorPicker = new ColorPicker(colorFrame);
	CHECK_PTR(fgColorPicker);
	fgColorPicker->setText(i18n("Foreground Color"));
	bgColorPicker = new ColorPicker(colorFrame);
	CHECK_PTR(bgColorPicker);
	bgColorPicker->setText(i18n("Background Color"));

	vbox2->addWidget(fgColorPicker);
	vbox2->addStretch(1);
	vbox2->addWidget(bgColorPicker);

	QGroupBox *fontFrame = new QGroupBox(textWidget, "fontFrame");
	CHECK_PTR(fontFrame);
	fontFrame->setColumnLayout(0, Qt::Vertical );
	fontFrame->layout()->setMargin(10);
	fontFrame->setTitle(i18n("Font"));

	QHBoxLayout *hbox1 = new QHBoxLayout(fontFrame->layout());
	CHECK_PTR(hbox1);

	fontButton = new QPushButton(i18n("Font"), fontFrame);
	CHECK_PTR(fontButton);
	fontButton->setFixedSize(100, 25);

	hbox1->addStretch(1);
	hbox1->addWidget(fontButton);

	textLayout->addWidget(titleFrame);
	textLayout->addWidget(colorFrame);
	textLayout->addWidget(fontFrame);

	connect(fontButton, SIGNAL(clicked()), this, SLOT(slotFontSelection()));

	tabWidget->addTab(textWidget, i18n("Text"));

	// filter sheet
	QWidget *filterWidget = new QWidget(tabWidget);
	CHECK_PTR(filterWidget);

	QVBoxLayout *vbox3 = new QVBoxLayout(filterWidget, 1);
	CHECK_PTR(vbox3);
	vbox3->setMargin(10);

	QGroupBox *filterFrame = new QGroupBox(filterWidget, "filterWidget");
	CHECK_PTR(filterFrame);

	QHBoxLayout *layout5 = new QHBoxLayout(filterFrame);
	CHECK_PTR(layout5);
	QVBoxLayout *layout6 = new QVBoxLayout(filterFrame);
	CHECK_PTR(layout6);
	
	layout5->addLayout(layout6);
	
	ruleButtons = new KButtonBox(filterFrame, KButtonBox::Vertical);
	CHECK_PTR(ruleButtons);
	ruleButtons->addButton(i18n("Add"), this, SLOT(slotAddRule()), FALSE);
	ruleButtons->addButton(i18n("Delete"), this, SLOT(slotDelRule()), FALSE);
	ruleButtons->addButton(i18n("Change"), this, SLOT(slotChangeRule()), FALSE);
	ruleButtons->layout();
	layout5->addWidget(ruleButtons);

	filterText = new QLineEdit(filterFrame);
	CHECK_PTR(filterText);
	layout6->addWidget(filterText);

	ruleList = new QListBox(filterFrame);
	CHECK_PTR(ruleList);
	layout6->addWidget(ruleList);

	vbox3->addWidget(filterFrame);

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
