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
#include <QList>
#include <QShowEvent>
#include <QHideEvent>
#include <QSortFilterProxyModel>
#include <QHeaderView>
#include <QAction>
#include <QMenu>
#include <QSet>
#include <QComboBox>
#include <QItemDelegate>
#include <QPainter>
#include <QProcess>
#include <QLineEdit>


#include <signal.h> //For SIGTERM

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
#include "processcore/processes.h"


//Trolltech have a testing class for classes that inherit QAbstractItemModel.  If you want to run with this run-time testing enabled, put themodeltest.* files in this directory and uncomment the next line
//#define DO_MODELCHECK
#ifdef DO_MODELCHECK
#include "modeltest.h"
#endif

class ProgressBarItemDelegate : public QItemDelegate 
{
  public:
	ProgressBarItemDelegate(QObject *parent) : QItemDelegate(parent), startProgressColor(0x00, 0x71, 0xBC, 100), endProgressColor(0x83, 0xDD, 0xF5, 100), totalMemory(-1) {}
  protected:
	virtual void drawDisplay(QPainter *painter, const QStyleOptionViewItem &option,
		                                 const QRect &rect, const QString &text) const
	{
/*		if(throb > 0) {
			QPen old = painter->pen();
			painter->setPen(Qt::NoPen);
			QColor throbColor(255, 255-throb, 255-throb, 100);
//			QLinearGradient linearGrad( rect.x(), rect.y(), rect.x(), rect.y() + rect.height());
//			linearGrad.setColorAt(0, QColor(255,255,255,100));
//			linearGrad.setColorAt(1, QColor(255, 255-throb, 255-throb, 100));

			painter->fillRect( rect.x(), rect.y(), rect.width(), rect.height(), QBrush(throbColor));
			painter->setPen( old );

		}
*/
		if(percentage > 0 && percentage * rect.width() > 100 ) { //make sure the line will have a width of more than 1 pixel
			QPen old = painter->pen();
			painter->setPen(Qt::NoPen);
			QLinearGradient  linearGrad( QPointF(rect.x(),rect.y()), QPointF(rect.x() + rect.width(), rect.y()));
			linearGrad.setColorAt(0, startProgressColor);
			linearGrad.setColorAt(1, endProgressColor);
			painter->fillRect( rect.x(), rect.y(), rect.width() * percentage /100 , rect.height(), QBrush(linearGrad));
			painter->setPen( old );
		}

		QItemDelegate::drawDisplay( painter, option, rect, text);
	}
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		QModelIndex realIndex = (reinterpret_cast< const QAbstractProxyModel *> (index.model()))->mapToSource(index);
		KSysGuard::Process *process = reinterpret_cast< KSysGuard::Process * > (realIndex.internalPointer());
		if(index.column() == ProcessModel::HeadingCPUUsage) {
			percentage = process->userUsage + process->sysUsage;
		} else if(index.column() == ProcessModel::HeadingMemory) {
			long long memory = 0;
			if(process->vmURSS != -1) 
				memory = process->vmURSS;
			else 
				memory = process->vmRSS;
			if(totalMemory == -1)
				totalMemory = index.data(Qt::UserRole+3).toLongLong();

			percentage = (int)(memory*100/totalMemory);
		} else if(index.column() == ProcessModel::HeadingSharedMemory) {
			if(process->vmURSS != -1) {
				if(totalMemory == -1)
					totalMemory = index.data(Qt::UserRole+3).toLongLong();
				percentage = (int)((process->vmRSS - process->vmURSS)*100/totalMemory);
			}
		} else
			percentage = 0;
		if(percentage > 100) percentage = 100;
		if(process->timeKillWasSent.isNull())
			throb = 0;
		else {
			throb = process->timeKillWasSent.elapsed() % 200;
			if(throb > 100) throb = 200 - throb;
		}

		QItemDelegate::paint(painter, option, index);
	}
	mutable int percentage;
	mutable int throb;
	QColor startProgressColor;
	QColor endProgressColor;
	mutable long long totalMemory;

};

KSysGuardProcessList::KSysGuardProcessList(QWidget* parent)
	: QWidget(parent), mModel(this), mFilterModel(this), mUi(new Ui::ProcessWidget())
{
	mUpdateIntervalMSecs = 2000; //Set 2 seconds as the default update interval
	mUi->setupUi(this);
	mFilterModel.setSourceModel(&mModel);
	mUi->treeView->setModel(&mFilterModel);
#ifdef DO_MODELCHECK
	new ModelTest(&mModel, this);
#endif
	mUi->treeView->setItemDelegate(new ProgressBarItemDelegate(mUi->treeView));

	mColumnContextMenu = new QMenu(mUi->treeView->header());
	connect(mColumnContextMenu, SIGNAL(triggered(QAction*)), this, SLOT(showOrHideColumn(QAction *)));
	mUi->treeView->header()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(mUi->treeView->header(), SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showColumnContextMenu(const QPoint&)));

	mProcessContextMenu = new QMenu(mUi->treeView);
	mUi->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(mUi->treeView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showProcessContextMenu(const QPoint&)));
	
	mUi->treeView->header()->setClickable(true);
	mUi->treeView->header()->setSortIndicatorShown(true);
	mUi->treeView->header()->setCascadingSectionResizes(true);
	mUi->treeView->setSelectionMode( QAbstractItemView::ExtendedSelection );
	connect(mUi->btnKillProcess, SIGNAL(clicked()), this, SLOT(killSelectedProcesses()));
	connect(mUi->txtFilter, SIGNAL(textChanged(const QString &)), &mFilterModel, SLOT(setFilterRegExp(const QString &)));
	connect(mUi->txtFilter, SIGNAL(textChanged(const QString &)), this, SLOT(expandInit()));
	connect(mUi->cmbFilter, SIGNAL(currentIndexChanged(int)), this, SLOT(setStateInt(int)));
	connect(mUi->treeView, SIGNAL(expanded(const QModelIndex &)), this, SLOT(expandAllChildren(const QModelIndex &)));
	connect(mUi->treeView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex & , const QModelIndex & )), this, SLOT(currentRowChanged(const QModelIndex &)));
	setMinimumSize(sizeHint());

        enum State {AllProcesses=0,AllProcessesInTreeForm, SystemProcesses, UserProcesses, OwnProcesses};

	mUi->cmbFilter->setItemIcon(ProcessFilter::AllProcesses, KIcon("view-process-all"));
	mUi->cmbFilter->setItemIcon(ProcessFilter::AllProcessesInTreeForm, KIcon("view-process-all-tree"));
	mUi->cmbFilter->setItemIcon(ProcessFilter::SystemProcesses, KIcon("view-process-system"));
	mUi->cmbFilter->setItemIcon(ProcessFilter::UserProcesses, KIcon("view-process-users"));
	mUi->cmbFilter->setItemIcon(ProcessFilter::OwnProcesses, KIcon("view-process-own"));

	/*  Hide the vm size column by default since it's not very useful */
	mUi->treeView->header()->hideSection(ProcessModel::HeadingVmSize);
	mUi->treeView->header()->hideSection(ProcessModel::HeadingNiceness);
	mUi->treeView->header()->hideSection(ProcessModel::HeadingTty);
	mUi->treeView->header()->hideSection(ProcessModel::HeadingCommand);
	mFilterModel.setFilterKeyColumn(0);

	//Process names can have mixed case. Make the filter case insensitive.
	mFilterModel.setFilterCaseSensitivity(Qt::CaseInsensitive);

	//Logical column 0 will always be the tree bit with the process name.  We expand this automatically in code,
	//so don't let the user change it
	mUi->treeView->header()->setResizeMode(0, QHeaderView::ResizeToContents);
	mUi->treeView->header()->setStretchLastSection(true);

	//Sort by username by default
	mUi->treeView->sortByColumn(ProcessModel::HeadingUser, Qt::AscendingOrder);
	mFilterModel.sort(ProcessModel::HeadingUser, Qt::AscendingOrder);
	mFilterModel.setDynamicSortFilter(true);

	mUpdateTimer = new QTimer(this);
	mUpdateTimer->setSingleShot(true);
	connect(mUpdateTimer, SIGNAL(timeout()), this, SLOT(updateList()));
	mUpdateTimer->start(mUpdateIntervalMSecs);

	//If the view resorts continually, then it can be hard to keep track of processes.  By doing it only every few seconds it reduces the 'jumping around'
	QTimer *mTimer = new QTimer(this);
	connect(mTimer, SIGNAL(timeout()), &mFilterModel, SLOT(invalidate()));
	mTimer->start(6000); 

	expandInit(); //This will expand the init process
}

QTreeView *KSysGuardProcessList::treeView() const {
	return mUi->treeView;
}

QLineEdit *KSysGuardProcessList::filterLineEdit() const {
	return mUi->txtFilter;
}

ProcessFilter::State KSysGuardProcessList::state() const 
{
	return mFilterModel.filter();
}
void KSysGuardProcessList::setStateInt(int state) {
	setState((ProcessFilter::State) state);
}
void KSysGuardProcessList::setState(ProcessFilter::State state)
{  //index is the item the user selected in the combo box
	mFilterModel.setFilter(state);
	mModel.setSimpleMode( (state != ProcessFilter::AllProcessesInTreeForm) );
	mUi->cmbFilter->setCurrentIndex( (int)state);
}
void KSysGuardProcessList::currentRowChanged(const QModelIndex &current)
{
	mUi->btnKillProcess->setEnabled(current.isValid());
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
		reniceSelectedProcesses();
	} else if(result == kill) {
		killSelectedProcesses();
	}
}

void KSysGuardProcessList::showColumnContextMenu(const QPoint &point){
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
		mUi->treeView->resizeColumnToContents(index);
		mUi->treeView->resizeColumnToContents(mFilterModel.columnCount());
	}
		
}

void KSysGuardProcessList::expandAllChildren(const QModelIndex &parent) 
{
	//This is called when the user expands a node.  This then expands all of its 
	//children.  This will trigger this function again recursively.
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

void KSysGuardProcessList::hideEvent ( QHideEvent * event )  //virtual protected from QWidget
{
	//Stop updating the process list if we are hidden
	mUpdateTimer->stop();
	QWidget::hideEvent(event);
}

void KSysGuardProcessList::showEvent ( QShowEvent * event )  //virtual protected from QWidget
{
	//Start updating the process list again if we are shown again
	if(!mUpdateTimer->isActive()) 
		mUpdateTimer->start(mUpdateIntervalMSecs);
	QWidget::showEvent(event);
}

void KSysGuardProcessList::updateList()
{
	if(isVisible()) {
		mModel.update(mUpdateIntervalMSecs);
		expandInit();
		mUpdateTimer->start(mUpdateIntervalMSecs);
	}
}

int KSysGuardProcessList::updateIntervalMSecs() const 
{
	return mUpdateIntervalMSecs;
}	

void KSysGuardProcessList::setUpdateIntervalMSecs(int intervalMSecs) 
{
	mUpdateIntervalMSecs = intervalMSecs;
	mUpdateTimer->setInterval(mUpdateIntervalMSecs);
}

void KSysGuardProcessList::reniceProcesses(const QList<long long> &pids, int niceValue)
{
	QList< long long> unreniced_pids;
        for (int i = 0; i < pids.size(); ++i) {
		bool success = mModel.processController()->setNiceness(pids.at(i), niceValue);
		if(!success)
			unreniced_pids << pids.at(i);
	}
	if(unreniced_pids.isEmpty()) return; //All processes were reniced successfully
	if(!mModel.isLocalhost()) return; //We can't use kdesu to renice non-localhost processes
	
	QStringList arguments;
	arguments << "--" << "renice" << QString::number(niceValue);

        for (int i = 0; i < unreniced_pids.size(); ++i)
		arguments << QString::number(unreniced_pids.at(i));

	QProcess *reniceProcess = new QProcess(NULL);
	connect(reniceProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(reniceFailed()));
	connect(reniceProcess, SIGNAL(finished( int, QProcess::ExitStatus) ), this, SLOT(updateList()));
	reniceProcess->start("kdesu", arguments);
}

void KSysGuardProcessList::reniceSelectedProcesses()
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
	ReniceDlg reniceDlg(mUi->treeView, firstPriority, selectedAsStrings);
	if(reniceDlg.exec() == QDialog::Rejected) return;
	int newPriority = reniceDlg.newPriority;
	Q_ASSERT(newPriority <= 19 && newPriority >= -20); 

	Q_ASSERT(selectedPids.size() == selectedAsStrings.size());
	reniceProcesses(selectedPids, newPriority);
	updateList();
}

void KSysGuardProcessList::killProcesses(const QList< long long> &pids, int sig)
{
	QList< long long> unkilled_pids;
        for (int i = 0; i < pids.size(); ++i) {
		bool success = mModel.processController()->sendSignal(pids.at(i), sig);
		if(!success)
			unkilled_pids << pids.at(i);
	}
	if(unkilled_pids.isEmpty()) return;
	if(!mModel.isLocalhost()) return; //We can't use kdesu to kill non-localhost processes

	//We must use kdesu to kill the process
	QStringList arguments;
	arguments << "--" << "kill";
	if(sig != SIGTERM)
		arguments << ('-' + QString::number(sig));

        for (int i = 0; i < unkilled_pids.size(); ++i)
		arguments << QString::number(unkilled_pids.at(i));
	
	QProcess *killProcess = new QProcess(NULL);
	connect(killProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(killFailed()));
	connect(killProcess, SIGNAL(finished( int, QProcess::ExitStatus) ), this, SLOT(updateList()));
	killProcess->start("kdesu", arguments);

}
void KSysGuardProcessList::killSelectedProcesses()
{
	QModelIndexList selectedIndexes = mUi->treeView->selectionModel()->selectedRows();
	QStringList selectedAsStrings;
	QList< long long> selectedPids;
	QList<KSysGuard::Process *> selectedProcesses;
	for (int i = 0; i < selectedIndexes.size(); ++i) {
		KSysGuard::Process *process = reinterpret_cast<KSysGuard::Process *> (mFilterModel.mapToSource(selectedIndexes.at(i)).internalPointer());
		selectedProcesses << process;
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
								 KStandardGuiItem::cancel(),
								 "killconfirmation");
		if (res != KMessageBox::Continue)
		{
			return;
		}
	}
	foreach(KSysGuard::Process *process, selectedProcesses) {
		process->timeKillWasSent.start();
	}

	Q_ASSERT(selectedPids.size() == selectedAsStrings.size());
	killProcesses(selectedPids, SIGTERM);
	updateList();
}

void KSysGuardProcessList::reniceFailed()
{
	KMessageBox::sorry(this, i18n("You do not have the permission to renice the process and there "
                                      "was a problem trying to run as root"));
}
void KSysGuardProcessList::killFailed()
{
	KMessageBox::sorry(this, i18n("You do not have the permission to kill the process and there "
                                "was a problem trying to run as root"));
}

bool KSysGuardProcessList::showTotals() const {
	return mModel.showTotals();
}

void KSysGuardProcessList::setShowTotals(bool showTotals)  //slot
{
	mModel.setShowTotals(showTotals);
}
