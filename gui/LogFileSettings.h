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

#ifndef _LogFileSettings_h
#define _LogFileSettings_h

#include <kbuttonbox.h>

#include <qcolor.h>
#include <qdialog.h>
#include <qfont.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qtabwidget.h>

#include "ColorPicker.h"

class LogFileSettings : public QDialog
{
	Q_OBJECT
public:
	LogFileSettings(QWidget *parent = 0, const char *name = 0);

	void setFilterRules(const QStringList& rules) { ruleList->insertStringList(rules); }

	void setForegroundColor(const QColor& color) { fgColorPicker->setColor(color); }
	void setBackgroundColor(const QColor& color) { bgColorPicker->setColor(color); }
	void setFont(const QFont& font) { textFont = font; fontButton->setFont(font); }
	void setTitle(const QString& title) { titleText->setText(title); }

	QStringList getFilterRules(void) {
		QStringList rules;
		
		for (uint i = 0; i < ruleList->count(); i++) {
			rules.append(ruleList->text(i));
		}
		
		return rules;
	}

	QColor getForegroundColor(void) { return fgColorPicker->getColor(); }
	QColor getBackgroundColor(void) { return bgColorPicker->getColor(); }
	QFont getFont(void) { return textFont; }
	QString getTitle(void) { return titleText->text(); }

public slots:
	void slotFontSelection(void);
	void slotAddRule(void);
	void slotDelRule(void);
	void slotChangeRule(void);
	void slotRuleListSelected(int);

private:
	KButtonBox *actionButtons;
	KButtonBox *ruleButtons;

	ColorPicker *fgColorPicker;
	ColorPicker *bgColorPicker;

	QFont textFont;
	QLineEdit *filterText;
	QLineEdit *titleText;
	QListBox *ruleList;
	QPushButton *fontButton;
	QTabWidget *tabWidget;
};

#endif // _LogFileSettings_h
