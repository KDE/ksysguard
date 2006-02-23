#ifndef KSYSGUARD_PROCESS_H
#define KSYSGUARD_PROCESS_H

#include <QObject>
#include <QString>
#include <QList>
#include <QVariant>

class Process : public QObject {
  Q_OBJECT
  public:
	typedef enum { Daemon, Kernel, Init, Kdeapp, Shell, Tools, Wordprocessing, Term, Other, Invalid } ProcessType;
	Process() { uid = 0; pid = 0; parent_pid = 0; gid = -1; processType=Invalid; tracerpid = 0;}
	Process(long long _pid, long long _ppid)  {
		uid = 0; pid = _pid; parent_pid = _ppid; gid = -1; processType=Invalid; tracerpid = 0;}
	bool isValid() {return processType != Process::Invalid;}
	
	long long pid;    //The systems ID for this process
	long long parent_pid;  //The systems ID for the parent of this process.  0 for init.
	long long uid; //The user id that the process is running as
	long long gid; //The group id that the process is running as
	long long tracerpid; //If this is being debugged, this is the process that is debugging it
	ProcessType processType;
	QString name;  //The name (e.g. "ksysguard", "konversation", "init")
	QList<long long> children_pids;
	QList<QVariant> data;  //The column data, excluding the name, pid, ppid and uid
};

#endif
