#ifndef _LogFile_h
#define _LogFile_h

#define MAXLINES 500

#include <qdom.h>
#include <qfile.h>
#include <qlistbox.h>
#include <qpopupmenu.h>
#include <qstring.h>
#include <qstringlist.h>

#include <SensorDisplay.h>
#include "LogFileSettings.h"

class LogFile : public SensorDisplay
{
	Q_OBJECT
public:
	LogFile(QWidget *parent = 0, const char *name = 0);
	~LogFile(void);

	bool addSensor(const QString& hostName, const QString& sensorName,
				   const QString& sensorDescr);
	void answerReceived(int id, const QString& answer);
	void resizeEvent(QResizeEvent*);

	bool createFromDOM(QDomElement& domEl);
	bool addToDOM(QDomDocument& doc, QDomElement& display, bool save = true);

	void updateMonitor(void);

	void settings(void);

	virtual void timerEvent(QTimerEvent*)
	{
		updateMonitor();
	}

	virtual bool hasSettingsDialog() const
	{
		return (TRUE);
	}

	virtual void sensorError(bool err);

public slots:
	void applySettings();

private:
	LogFileSettings* lfs;

	QFile* logFile;
	QLabel* errorLabel;
	QListBox* monitor;
	QString fileName;
	QStringList filterRules;

	unsigned long logFileID;
};

#endif // _LogFile_h
