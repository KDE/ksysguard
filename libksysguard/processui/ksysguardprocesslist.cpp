/*
    KSysGuard, the KDE System Guard

	Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>
	Copyright (c) 2006 - 2007 John Tapsell <john.tapsell@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms version 2 of of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <QTimer>

#include <QDomElement>
#include <QVBoxLayout>
#include <QList>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QSortFilterProxyModel>
#include <QHeaderView>
#include <QAction>
#include <QMenu>
#include <QTime>
#include <QSet>


#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdialog.h>
#include <kicon.h>

#include "ksysguardprocesslist.moc"
#include "ksysguardprocesslist.h"
#include "ReniceDlg.h"
#include "ui_ProcessWidgetUI.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLayout>
#include <QItemDelegate>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QProgressBar>

#include <kapplication.h>
//#define DO_MODELCHECK
#ifdef DO_MODELCHECK
#include "modeltest.h"
#endif

class ProgressBarItemDelegate : public QItemDelegate 
{
  public:
	ProgressBarItemDelegate(QObject *parent) : QItemDelegate(parent) {}
  protected:
	virtual void drawDisplay(QPainter *painter, const QStyleOptionViewItem &option,
		                                 const QRect &rect, const QString &text) const
	{
		if(percentage > 0) {
			QPen old = painter->pen();
			painter->setPen(Qt::NoPen);
			QLinearGradient  linearGrad( QPointF(rect.x(),rect.y()), QPointF(rect.x() + rect.width(), rect.y()));
			linearGrad.setColorAt(0, QColor(0x00, 0x71, 0xBC, 100));
			linearGrad.setColorAt(1, QColor(0x83, 0xDD, 0xF5, 100));
			painter->fillRect( rect.x(), rect.y(), rect.width() * percentage /100 , rect.height(), QBrush(linearGrad));
			painter->setPen( old );
		}

		QItemDelegate::drawDisplay( painter, option, rect, text);
	}
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		percentage = index.data(Qt::UserRole+2).toInt(); //we have set that UserRole+2  returns a number between 1 and 100 for the percentage to draw. 0 for none.
		QItemDelegate::paint(painter, option, index);
	}
	mutable int percentage;

};

KSysGuardProcessList::KSysGuardProcessList(QWidget* parent)
	: QWidget(parent), mModel(this), mFilterModel(this), mUi(new Ui::ProcessWidget())
{
	mKillProcess = 0;
	mSimple = true;
	mInitialSortCol = 1;
        mInitialSortInc = false;
	mReadyForPs = false;
	mMemFree = -1;
	mMemUsed = -1;
	mMemTotal = -1;
	mModel.setIsLocalhost(true);
	mUi->setupUi(this);
	mFilterModel.setSourceModel(&mModel);
	mUi->treeView->setModel(&mFilterModel);
#ifdef DO_MODELCHECK
	new Modeltest(&mFilterModel, this);
#endif
	mUi->treeView->setItemDelegate(new ProgressBarItemDelegate(mUi->treeView));

	mColumnContextMenu = new QMenu(mUi->treeView->header());
	connect(mColumnContextMenu, SIGNAL(triggered(QAction*)), this, SLOT(showOrHideColumn(QAction *)));
	mUi->treeView->header()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(mUi->treeView->header(), SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));

	mProcessContextMenu = new QMenu(mUi->treeView);
	mUi->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(mUi->treeView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showProcessContextMenu(const QPoint&)));
	
	mUi->treeView->header()->setClickable(true);
	mUi->treeView->header()->setSortIndicatorShown(true);
	mUi->treeView->header()->setCascadingSectionResizes(true);
	mUi->treeView->setSelectionMode( QAbstractItemView::ExtendedSelection );
	connect(mUi->btnKillProcess, SIGNAL(clicked()), this, SLOT(killProcess()));
	connect(mUi->txtFilter, SIGNAL(textChanged(const QString &)), &mFilterModel, SLOT(setFilterRegExp(const QString &)));
	connect(mUi->txtFilter, SIGNAL(textChanged(const QString &)), this, SLOT(expandInit()));
	connect(mUi->cmbFilter, SIGNAL(currentIndexChanged(int)), &mFilterModel, SLOT(setFilter(int)));
	connect(mUi->cmbFilter, SIGNAL(currentIndexChanged(int)), this, SLOT(setSimpleMode(int)));
	connect(mUi->treeView, SIGNAL(expanded(const QModelIndex &)), this, SLOT(expandAllChildren(const QModelIndex &)));
	connect(mUi->treeView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex & , const QModelIndex & )), this, SLOT(currentRowChanged(const QModelIndex &)));
	connect(mUi->chkShowTotals, SIGNAL(stateChanged(int)), &mModel, SLOT(setShowTotals(int)));
	mUi->chkShowTotals->setVisible(!mSimple);
	setMinimumSize(sizeHint());


	mUpdateTimer = new QTimer(this);
	mUpdateTimer->setSingleShot(false);
	connect(mUpdateTimer, SIGNAL(timeout()), this, SLOT(updateList()));
	mUpdateTimer->start(2000);

	//If the view resorts continually, then it can be hard to keep track of processes.  By doing it only every 3 seconds it reduces the 'jumping around'
	QTimer *mTimer = new QTimer(this);
	connect(mTimer, SIGNAL(timeout()), &mFilterModel, SLOT(resort()));
	mTimer->start(3000); 

	expandInit(); //This will expand the init process
}
void KSysGuardProcessList::setSimpleMode(int index)
{  //index is the item the user selected in the combo box
	bool simple = (index != PROCESS_FILTER_ALL_TREE);
	if(simple == mSimple) return; //Optimization - don't bother changing anything if the simple mode hasn't been toggled
	mSimple = simple;
	mModel.setSimpleMode(mSimple);
	
	mUi->chkShowTotals->setVisible(!mSimple);
}
void KSysGuardProcessList::currentRowChanged(const QModelIndex &current)
{
	mUi->btnKillProcess->setEnabled(current.isValid() && mKillSupported);
}
void KSysGuardProcessList::showProcessContextMenu(const QPoint &point){
	mProcessContextMenu->clear();

	QAction *renice = new QAction(mProcessContextMenu);
	renice->setText(i18n("Renice process"));
	mProcessContextMenu->addAction(renice);

	QAction *kill = new QAction(mProcessContextMenu);
	kill->setText(i18n("Kill process"));
	kill->setIcon(KIcon("stop"));
	mProcessContextMenu->addAction(kill);

	QAction *result = mProcessContextMenu->exec(mUi->treeView->mapToGlobal(point));	
	if(result == renice) {
		reniceProcess();
	} else if(result == kill) {
		killProcess();
	}
}

void KSysGuardProcessList::showContextMenu(const QPoint &point){
	mColumnContextMenu->clear();
	
	{
		int index = mUi->treeView->header()->logicalIndexAt(point);
		if(index >= 0) {
			//selected a column.  Give the option to hide it
			QAction *action = new QAction(mColumnContextMenu);
			action->setData(-index-1); //We set data to be negative (and minus 1) to hide a column, and positive to show a column
			action->setText(i18n("Hide column '%1'", mFilterModel.headerData(index, Qt::Horizontal, Qt::DisplayRole).toString()));
			mColumnContextMenu->addAction(action);
			if(mUi->treeView->header()->sectionsHidden()) {
				mColumnContextMenu->addSeparator();
			}
		}
	}

	if(mUi->treeView->header()->sectionsHidden()) {
		int num_headings = mFilterModel.columnCount();
		for(int i = 0; i < num_headings; ++i) {
			if(mUi->treeView->header()->isSectionHidden(i)) {
				QAction *action = new QAction(mColumnContextMenu);
				action->setText(i18n("Show column '%1'", mFilterModel.headerData(i, Qt::Horizontal, Qt::DisplayRole).toString()));
				action->setData(i); //We set data to be negative (and minus 1) to hide a column, and positive to show a column
				mColumnContextMenu->addAction(action);
			}
		}
	}
	mColumnContextMenu->exec(mUi->treeView->header()->mapToGlobal(point));	
}

void KSysGuardProcessList::showOrHideColumn(QAction *action)
{
	int index = action->data().toInt();
	//We set data to be negative to hide a column, and positive to show a column
	if(index < 0)
		mUi->treeView->hideColumn(-1-index);
	else {
		mUi->treeView->showColumn(index);
		mUi->treeView->resizeColumnToContents(mFilterModel.columnCount());
	}
		
}
void KSysGuardProcessList::expandAllChildren(const QModelIndex &parent) 
{
	//This is called when the user expands a node.  This then expands all of its children.  This will trigger this function again
	//recursively.
	QModelIndex sourceParent = mFilterModel.mapToSource(parent);
	for(int i = 0; i < mModel.rowCount(sourceParent); i++) 
		mUi->treeView->expand(mFilterModel.mapFromSource(mModel.index(i,0, sourceParent)));
}

void KSysGuardProcessList::expandInit()
{
	//When we expand the items, make sure we don't call our expand all children function
	disconnect(mUi->treeView, SIGNAL(expanded(const QModelIndex &)), this, SLOT(expandAllChildren(const QModelIndex &)));
	mUi->treeView->expand(mFilterModel.mapFromSource(mModel.index(0,0, QModelIndex())));
	connect(mUi->treeView, SIGNAL(expanded(const QModelIndex &)), this, SLOT(expandAllChildren(const QModelIndex &)));
}

void KSysGuardProcessList::resizeEvent(QResizeEvent* ev)
{
	QWidget::resizeEvent(ev);
}

void
KSysGuardProcessList::updateList()
{
	mModel.update();
	expandInit(); //This will expand the init process
}

void KSysGuardProcessList::killProcess(int pid, int sig)
{
//	sendRequest(sensors().at(0)->hostName(),
//				QString("kill %1 %2" ).arg(pid).arg(sig), Kill_Command);
}

void KSysGuardProcessList::reniceProcess()
{
	QModelIndexList selectedIndexes = mUi->treeView->selectionModel()->selectedRows();
	QStringList selectedAsStrings;
	QList< long long> selectedPids;
	int firstPriority = 0;
	for (int i = 0; i < selectedIndexes.size(); ++i) {
		KSysGuard::Process *process = reinterpret_cast<KSysGuard::Process *> (mFilterModel.mapToSource(selectedIndexes.at(i)).internalPointer());
		if(i==0) firstPriority = process->niceLevel;
		selectedPids << process->pid;
		selectedAsStrings << mModel.getStringForProcess(process);
	}
	
	if (selectedAsStrings.isEmpty())
	{
		KMessageBox::sorry(this, i18n("You need to select a process first."));
		return;
	}
	else
	{
		ReniceDlg reniceDlg(mUi->treeView, firstPriority, selectedAsStrings);
		if(reniceDlg.exec() == QDialog::Rejected) return;
		int newPriority = reniceDlg.newPriority;
		Q_ASSERT(newPriority <= 19 && newPriority >= -20); 

		Q_ASSERT(selectedPids.size() == selectedAsStrings.size());
//	        for (int i = 0; i < selectedPids.size(); ++i) 
//	            sendRequest(sensors().at(0)->hostName(), QString("setpriority %1 %2" ).arg(selectedPids.at( i ))
//                       .arg(newPriority), Renice_Command);
//		sendRequest(sensors().at(0)->hostName(), "ps", Ps_Command);  //update the display afterwards
        }
}
void KSysGuardProcessList::killProcess()
{
/*	QModelIndexList selectedIndexes = mUi->treeView->selectionModel()->selectedRows();
	QStringList selectedAsStrings;
	QList< long long> selectedPids;
	for (int i = 0; i < selectedIndexes.size(); ++i) {
		KSysGuard::Process *process = reinterpret_cast<KSysGuard::Process *> (mFilterModel.mapToSource(selectedIndexes.at(i)).internalPointer());
		selectedPids << process->pid;
		selectedAsStrings << mModel.getStringForProcess(process);
	}
	
	if (selectedAsStrings.isEmpty())
	{
		KMessageBox::sorry(this, i18n("You need to select a process first."));
		return;
	}
	else
	{
		QString  msg = i18np("Do you want to kill the selected process?",
				"Do you want to kill the %1 selected processes?",
				selectedAsStrings.count());

		int res = KMessageBox::warningContinueCancelList(this, msg, selectedAsStrings,
				                                 i18n("Kill Process"),
								 KGuiItem(i18n("Kill")),
								 "killconfirmation");
		if (res != KMessageBox::Continue)
		{
			return;
		}
	}

	Q_ASSERT(selectedPids.size() == selectedAsStrings.size());
        for (int i = 0; i < selectedPids.size(); ++i) {
            sendRequest(sensors().at(0)->hostName(), QString("kill %1 %2" ).arg(selectedPids.at( i ))
                       .arg(MENU_ID_SIGKILL), Kill_Command);
        }
	sendRequest(sensors().at(0)->hostName(), "ps", Ps_Command);  //update the display afterwards*/
}

void
KSysGuardProcessList::reniceProcess(int pid, int niceValue)
{
/*	sendRequest(sensors().at(0)->hostName(),
				QString("setpriority %1 %2" ).arg(pid).arg(niceValue), Renice_Command);
	sendRequest(sensors().at(0)->hostName(), "ps", Ps_Command);  //update the display afterwards
*/
}
#if 0
void
KSysGuardProcessList::answerReceived(int id, const QList<QByteArray>& answer)
{
	/* We received something, so the sensor is probably ok. */
	sensorError(id, false);
	switch (id)
	{
	case Ps_Info_Command:
	{

		kDebug() << "PS_INFO_COMMAND: Received ps info at " << QTime::currentTime().toString("hh:mm:ss.zzz") << endl;
		/* We have received the answer to a ps? command that contains
		 * the information about the table headers. */
		if (answer.count() != 2)
		{
			kDebug (1215) << "KSysGuardProcessList::answerReceived(Ps_Info_Command)"
				  "wrong number of lines [" <<  answer << "]" << endl;
			sensorError(id, true);
			return;
		}
		QList<QByteArray> header = answer[0].split('\t');
		if(answer[1].count() != header.count() *2 -1) {
			kDebug(1215) << "KSysGuardProcessList::answerReceived.  Invalid data from a client - column type and data don't match in number.  Discarding" << endl;
			sensorError(id, true);
			return;
		}
		//answer.at(1) is the column type is in the form "d\tf\tS\t" etc
		if(!mModel.setHeader(header, answer[1])) {
			sensorError(id,true);
			return;
		}
		int vmSizeColumn = mModel.vmSizeColumn();
		if(vmSizeColumn >=0)
			mUi->treeView->header()->hideSection(vmSizeColumn);
		mFilterModel.setFilterKeyColumn(0);
		//Process names can have mixed case. Make the filter case insensitive.
		mFilterModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
		//Logical column 0 will always be the tree bit with the process name.  We expand this automatically in code,
		//so don't let the user change it
		mUi->treeView->header()->setResizeMode(0, QHeaderView::ResizeToContents);
		//set the last section to stretch to take up all the room.  We also set this again in xres_info to be sure
		mUi->treeView->header()->setStretchLastSection(true);
		//When we loaded the settings, we set mInitialSort* to the column and direction to sort in/
		//Since we have just added the columns no, we should set the sort now.
		
		mUi->treeView->sortByColumn(mInitialSortCol, (mInitialSortInc)?Qt::AscendingOrder:Qt::DescendingOrder);
		mFilterModel.sort(mInitialSortCol,(mInitialSortInc)?Qt::AscendingOrder:Qt::DescendingOrder);
		mFilterModel.setDynamicSortFilter(true);
		kDebug() << "We have added the columns and now setting the sort by col " << mInitialSortCol << endl;

		kDebug() << "PS_INFO_COMMAND: We are now ready for ps.  " << QTime::currentTime().toString("hh:mm:ss.zzz") << endl;
		mReadyForPs = true;
//		sendRequest(sensors().at(0)->hostName(), "ps", Ps_Command);

		break;
	}
	case Ps_Command:
	{
		kDebug() << "We received ps data." << QTime::currentTime().toString("hh:mm:ss.zzz") << endl;
		/* We have received the answer to a ps command that contains a
		 * list of processes with various additional information. */
		if(!mReadyForPs) {
			sendRequest(sensors().at(0)->hostName(), "ps", Ps_Command); //send another request - we will be ready next time, honest!
			kDebug(1215) << "Process data arrived before we were ready for it. hmm" << endl;
			break;
		}
		QList< QList<QByteArray> > data;
		QListIterator<QByteArray> i(answer);
		while(i.hasNext()) {
			QByteArray row = i.next();
			if(!row.trimmed().isEmpty())
				data << row.split('\t');
		}
		if(data.isEmpty()) {
			kDebug(1215) << "No data in the answer from 'ps'" << endl;
			break;
		}
		//We are now ready to send this data to the model.  
		if(!mModel.setData(data)) {
			sensorError(id, true);
			return;
		}
		expandInit(); //This will expand the init process

//		kDebug() << "We finished with ps data." << QTime::currentTime().toString("hh:mm:ss.zzz") << endl;
		break;
	}
	case Kill_Command:
	{
		// we should get a reply like  {"0\t3230"}  - i.e. just one string containing the error code and the pid
		if(answer.count() != 1) {
			kDebug(1215) << "Invalid answer from a kill request" << endl;
			break;
		}

		QList<QByteArray> answer2 = answer[0].split('\t');
		if(answer2.count() != 2) {
			kDebug(1215) << "Invalid answer from a kill request" << endl;
			break;
		}
		switch (answer2[0].toInt())
		{
		case 0:	// successful kill operation
			break;
		case 1:	// unknown error
//				i18n("Error while attempting to kill process %1.",
//				 QString::fromUtf8(answer2[1])));
			break;
		case 2: {
			if(!sensors().at(0)->isLocalhost()) {
//					i18n("You do not have the permission to kill process %1 on host %2 and due to security "
//					     "considerations, KDE cannot execute root commands remotely",
//					     QString::fromUtf8(answer2[1]), sensors().at(0)->hostName()));
				break;
			} 
			//Run as root with kdesu to get around insufficent privillages
			QStringList arguments;
			bool ok;
			int pid = answer2[1].toInt(&ok);
			//I want to be extra careful here.  Make sure the pid really is a number. 
			if(!ok) {
				//something has gone seriously wrong.
//				KSGRD::SensorMgr->notify(i18n("There was an internal safety check problem trying to kill the process."));
				break;
			}
			arguments << "kill" << QString::number(pid);
			if(mKillProcess == 0) {
				mKillProcess = new QProcess(this);
				connect(mKillProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(killFailed()));
			}
			mKillProcess->start("kdesu", arguments);
			
			break;
		}
		case 3:
//				i18n("Process %1 has already disappeared.",
//				 QString::fromUtf8(answer2[1])));
			break;
		case 4:
//			i18n("Invalid Signal.");
			break;
		}
		QTimer::singleShot(0, this, SLOT(updateList())); //use a single shot incase we have multiple kill_command results
		break;	
	}
	case Kill_Supported_Command:
		kDebug() << "We received kill supported data." << QTime::currentTime().toString("hh:mm:ss.zzz") << endl;
		if(!answer.isEmpty())
			mKillSupported = (answer[0].toInt() == 1);
		break;
	case Renice_Command:
	{
		// we should get a reply like  {"0\t3230"}  - i.e. just one string containing the error code and the pid
		if(answer.count() != 1) {
			kDebug(1215) << "Invalid answer from a renice request" << endl;
			break;
		}

		QList<QByteArray> answer2 = answer[0].split('\t');

		// result of renice operation
		if(answer2.count() != 3 && answer2.count() != 1) 
			break;
		
		if(answer2.count() == 1) { //Unfortunetly kde3 ksysguardd does not return the pid of the process,  This means we need to handle the two cases separately
			switch (answer2[0].toInt())
			{
			case 0:	// successful renice operation
				break;
			case 1:	// unknown error
//				i18n("Error while attempting to renice process.");
				break;
			case 2:
//				KSGRD::SensorMgr->notify(
//					i18n("Insufficient permissions to renice process"));
				break;
			case 3:
//				KSGRD::SensorMgr->notify(i18n("Process has disappeared."));
				break;
			case 4:
//				KSGRD::SensorMgr->notify(i18n("Internal communication problem."));
				break;
			}
			break;
		} 
		// In kde4, ksysguardd returns the pid of the process.  If renice-ing failed, and localhost, then
		// attempt a renice with ksudo
		switch (answer2[0].toInt())
		{
		case 0:	// successful renice operation
			break;
		case 1:	// unknown error
//			KSGRD::SensorMgr->notify(
//				i18n("Error while attempting to renice process %1.",
//				 answer2[1].constData()));
			break;
		case 2: {
			if(!sensors().at(0)->isLocalhost()) {
//				KSGRD::SensorMgr->notify(
//					i18n("You do not have the permission to renice process %1 on host %2 and due to security "
//					     "considerations, KDE cannot execute root commands remotely",
//					     answer2[1].constData(), sensors().at(0)->hostName()));
				break;
			} 
			//Run as root with kdesu to get around insufficent privillages
			QStringList arguments;
			bool ok,ok2;
			int pid = answer2[1].toInt(&ok);
			int prio = answer2[2].toInt(&ok2);
			//I want to be extra careful here.  Make sure the pid really is a number. 
			if(!ok || !ok2) {
				//something has gone seriously wrong.
//				KSGRD::SensorMgr->notify(i18n("There was an internal safety check problem trying to renice the process."));
				break;
			}
			arguments << "renice" << QString::number(prio) << QString::number(pid);
			if(mReniceProcess == 0) {
				mReniceProcess = new QProcess(this);
				connect(mReniceProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(reniceFailed()));
			}
			mReniceProcess->start("kdesu", arguments);

			break;  
		} 
		case 3:
//			KSGRD::SensorMgr->notify(
//				i18n("Process %1 has disappeared.",
//				 answer2[1].constData()));
			break;
		case 4:  
//			KSGRD::SensorMgr->notify(i18n("Internal communication problem."));
			break;
		}
		break;
	}
	case XRes_Info_Command:
	{
		//Note: It makes things easier to just send and accept the xres info command even when in simple mode
		kDebug() << "XRES INFO" << endl;
		if(mXResHeadingStart != -1) break; //Already has xres info
		if (answer.count() != 2)
		{
//			kDebug (1215) << "KSysGuardProcessList::answerReceived(XRes_Info_Command)"
//				  "wrong number of lines: " <<  answer.count() <<  endl;
//			sensorError(id, true);
			return;
		}
		QList<QByteArray> header = answer.at(0).split('\t');
		//answer[1] is the column type.  it looks like "f\tf\td"
		if(answer[1].size() != header.count()*2-1) {
//			kDebug(1215) << "KSysGuardProcessList::answerReceived.  Invalid data from a client - column type and data don't match in number. " << answer[1].size() << "," << header.count() << "  Discarding" << endl;
//			sensorError(id, true);
			return;
		}
		mXResHeadingStart = mUi->treeView->header()->count();
		if(!mModel.setXResHeader(header)) {
//			sensorError(id,true);
			mXResHeadingStart = -1;
			return;
		}
		mXResHeadingEnd = mUi->treeView->header()->count() -1;
	        for(int i = mXResHeadingStart; i <= mXResHeadingEnd; i++) {
	                if(mSimple)
				mUi->treeView->header()->hideSection(i);
			else
				mUi->treeView->header()->showSection(i);
		}
		mUi->treeView->header()->setStretchLastSection(true);
		break;
	}
	case MemFree_Command:
	case MemUsed_Command:
		if(answer.count() != 1) {
			kDebug(1215) << "Invalid answer from a mem free request" << endl;
			break;
		}
		// We should get a reply like "235384" meaning the amount, kb, of free memory
		if(id == MemFree_Command)
			mMemFree = answer[0].toLong();
		else
			mMemUsed = answer[0].toLong();

		if(mMemUsed != -1 && mMemFree != -1) {
			mMemTotal = mMemUsed + mMemFree;
			mModel.setTotalMemory(mMemTotal);
		}
		break;
	}
}
#endif
void KSysGuardProcessList::reniceFailed()
{
//  KSGRD::SensorMgr->notify(i18n("You do not have the permission to renice the process and there "
//                                "was a problem trying to run as root"));
}
void KSysGuardProcessList::killFailed()
{
//  KSGRD::SensorMgr->notify(i18n("You do not have the permission to kill the process and there "
//                                "was a problem trying to run as root"));
}


