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
	QListBox *ruleList;
	QPushButton *fontButton;
	QTabWidget *tabWidget;
};

#endif // _LogFileSettings_h
