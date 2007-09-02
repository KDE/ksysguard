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

struct KSysGuardProcessListPrivate {
    
	KSysGuardProcessListPrivate(KSysGuardProcessList* q) : mModel(q), mFilterModel(q), mUi(new Ui::ProcessWidget()) {}
    	/** The column context menu when you right click on a column.*/
	QMenu *mColumnContextMenu;
	
	/** The context menu when you right click on a process */
	QMenu *mProcessContextMenu;
	
	/** The process model.  This contains all the data on all the processes running on the system */
	ProcessModel mModel;
	
	/** The process filter.  The mModel is connected to this, and this filter model connects to the view.  This lets us
	 *  sort the view and filter (by using the combo box or the search line)
	 */
	ProcessFilter mFilterModel;
	
	/** The graphical user interface for this process list widget, auto-generated by Qt Designer */
	Ui::ProcessWidget *mUi;

	/** The time to wait, in milliseconds, between updating the process list */
	int mUpdateIntervalMSecs;
	
	/** A timer to call updateList() every mUpdateIntervalMSecs */
	QTimer *mUpdateTimer;

	/** A timer to rapidly pulse a process being killed */
	QTimer *mPulseTimer;
};

KSysGuardProcessList::KSysGuardProcessList(QWidget* parent)
	: QWidget(parent), d(new KSysGuardProcessListPrivate(this))
{
	d->mUpdateIntervalMSecs = 2000; //Set 2 seconds as the default update interval
	d->mUi->setupUi(this);
	d->mFilterModel.setSourceModel(&d->mModel);
	d->mUi->treeView->setModel(&d->mFilterModel);
#ifdef DO_MODELCHECK
	new ModelTest(&d->mModel, this);
#endif
	d->mUi->treeView->setItemDelegate(new ProgressBarItemDelegate(d->mUi->treeView));

	d->mColumnContextMenu = new QMenu(d->mUi->treeView->header());
	connect(d->mColumnContextMenu, SIGNAL(triggered(QAction*)), this, SLOT(showOrHideColumn(QAction *)));
	d->mUi->treeView->header()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(d->mUi->treeView->header(), SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showColumnContextMenu(const QPoint&)));

	d->mProcessContextMenu = new QMenu(d->mUi->treeView);
	d->mUi->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(d->mUi->treeView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showProcessContextMenu(const QPoint&)));

	d->mUi->treeView->header()->setClickable(true);
	d->mUi->treeView->header()->setSortIndicatorShown(true);
	d->mUi->treeView->header()->setCascadingSectionResizes(true);
	d->mUi->treeView->setSelectionMode( QAbstractItemView::ExtendedSelection );
	connect(d->mUi->btnKillProcess, SIGNAL(clicked()), this, SLOT(killSelectedProcesses()));
	connect(d->mUi->txtFilter, SIGNAL(textChanged(const QString &)), &d->mFilterModel, SLOT(setFilterRegExp(const QString &)));
	connect(d->mUi->txtFilter, SIGNAL(textChanged(const QString &)), this, SLOT(expandInit()));
	connect(d->mUi->cmbFilter, SIGNAL(currentIndexChanged(int)), this, SLOT(setStateInt(int)));
	connect(d->mUi->treeView, SIGNAL(expanded(const QModelIndex &)), this, SLOT(expandAllChildren(const QModelIndex &)));
	connect(d->mUi->treeView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex & , const QModelIndex & )), this, SLOT(currentRowChanged(const QModelIndex &)));
	setMinimumSize(sizeHint());

        enum State {AllProcesses=0,AllProcessesInTreeForm, SystemProcesses, UserProcesses, OwnProcesses};

	d->mUi->cmbFilter->setItemIcon(ProcessFilter::AllProcesses, KIcon("view-process-all"));
	d->mUi->cmbFilter->setItemIcon(ProcessFilter::AllProcessesInTreeForm, KIcon("view-process-all-tree"));
	d->mUi->cmbFilter->setItemIcon(ProcessFilter::SystemProcesses, KIcon("view-process-system"));
	d->mUi->cmbFilter->setItemIcon(ProcessFilter::UserProcesses, KIcon("view-process-users"));
	d->mUi->cmbFilter->setItemIcon(ProcessFilter::OwnProcesses, KIcon("view-process-own"));

	/*  Hide the vm size column by default since it's not very useful */
	d->mUi->treeView->header()->hideSection(ProcessModel::HeadingVmSize);
	d->mUi->treeView->header()->hideSection(ProcessModel::HeadingNiceness);
	d->mUi->treeView->header()->hideSection(ProcessModel::HeadingTty);
	d->mUi->treeView->header()->hideSection(ProcessModel::HeadingCommand);
	d->mFilterModel.setFilterKeyColumn(0);

	//Process names can have mixed case. Make the filter case insensitive.
	d->mFilterModel.setFilterCaseSensitivity(Qt::CaseInsensitive);

	//Logical column 0 will always be the tree bit with the process name.  We expand this automatically in code,
	//so don't let the user change it
	d->mUi->treeView->header()->setResizeMode(0, QHeaderView::ResizeToContents);
	d->mUi->treeView->header()->setStretchLastSection(true);

	//Sort by username by default
	d->mUi->treeView->sortByColumn(ProcessModel::HeadingUser, Qt::AscendingOrder);
	d->mFilterModel.sort(ProcessModel::HeadingUser, Qt::AscendingOrder);
	
	// Dynamic sort filter seems to require repainting the whole screen, slowing everything down drastically.
	// When this bug is fixed we can re-enable this.
	//d->mFilterModel.setDynamicSortFilter(true);

	d->mUpdateTimer = new QTimer(this);
	d->mUpdateTimer->setSingleShot(true);
	connect(d->mUpdateTimer, SIGNAL(timeout()), this, SLOT(updateList()));
	d->mUpdateTimer->start(d->mUpdateIntervalMSecs);

	//If the view resorts continually, then it can be hard to keep track of processes.  By doing it only every few seconds it reduces the 'jumping around'
	QTimer *mTimer = new QTimer(this);
	connect(mTimer, SIGNAL(timeout()), &d->mFilterModel, SLOT(invalidate()));
	mTimer->start(10000); 

	expandInit(); //This will expand the init process
}

KSysGuardProcessList::~KSysGuardProcessList()
{
    delete d;
}

QTreeView *KSysGuardProcessList::treeView() const {
	return d->mUi->treeView;
}

QLineEdit *KSysGuardProcessList::filterLineEdit() const {
	return d->mUi->txtFilter;
}

ProcessFilter::State KSysGuardProcessList::state() const 
{
	return d->mFilterModel.filter();
}
void KSysGuardProcessList::setStateInt(int state) {
	setState((ProcessFilter::State) state);
}
void KSysGuardProcessList::setState(ProcessFilter::State state)
{  //index is the item the user selected in the combo box
	d->mFilterModel.setFilter(state);
	d->mModel.setSimpleMode( (state != ProcessFilter::AllProcessesInTreeForm) );
	d->mUi->cmbFilter->setCurrentIndex( (int)state);
}
void KSysGuardProcessList::currentRowChanged(const QModelIndex &current)
{
	d->mUi->btnKillProcess->setEnabled(current.isValid());
}
void KSysGuardProcessList::showProcessContextMenu(const QPoint &point){
	d->mProcessContextMenu->clear();

	QAction *renice = new QAction(d->mProcessContextMenu);
	renice->setText(i18n("Renice Process..."));
	d->mProcessContextMenu->addAction(renice);

	QAction *kill = new QAction(d->mProcessContextMenu);
	kill->setText(i18n("Kill Process"));
	kill->setIcon(KIcon("stop"));
	d->mProcessContextMenu->addAction(kill);

	QAction *result = d->mProcessContextMenu->exec(d->mUi->treeView->mapToGlobal(point));	
	if(result == renice) {
		reniceSelectedProcesses();
	} else if(result == kill) {
		killSelectedProcesses();
	}
}

void KSysGuardProcessList::showColumnContextMenu(const QPoint &point){
	d->mColumnContextMenu->clear();
	
	{
		int index = d->mUi->treeView->header()->logicalIndexAt(point);
		if(index >= 0) {
			//selected a column.  Give the option to hide it
			QAction *action = new QAction(d->mColumnContextMenu);
			action->setData(-index-1); //We set data to be negative (and minus 1) to hide a column, and positive to show a column
			action->setText(i18n("Hide column '%1'", d->mFilterModel.headerData(index, Qt::Horizontal, Qt::DisplayRole).toString()));
			d->mColumnContextMenu->addAction(action);
			if(d->mUi->treeView->header()->sectionsHidden()) {
				d->mColumnContextMenu->addSeparator();
			}
		}
	}

	if(d->mUi->treeView->header()->sectionsHidden()) {
		int num_headings = d->mFilterModel.columnCount();
		for(int i = 0; i < num_headings; ++i) {
			if(d->mUi->treeView->header()->isSectionHidden(i)) {
				QAction *action = new QAction(d->mColumnContextMenu);
				action->setText(i18n("Show column '%1'", d->mFilterModel.headerData(i, Qt::Horizontal, Qt::DisplayRole).toString()));
				action->setData(i); //We set data to be negative (and minus 1) to hide a column, and positive to show a column
				d->mColumnContextMenu->addAction(action);
			}
		}
	}
	d->mColumnContextMenu->exec(d->mUi->treeView->header()->mapToGlobal(point));	
}

void KSysGuardProcessList::showOrHideColumn(QAction *action)
{
	int index = action->data().toInt();
	//We set data to be negative to hide a column, and positive to show a column
	if(index < 0)
		d->mUi->treeView->hideColumn(-1-index);
	else {
		d->mUi->treeView->showColumn(index);
		d->mUi->treeView->resizeColumnToContents(index);
		d->mUi->treeView->resizeColumnToContents(d->mFilterModel.columnCount());
	}
		
}

void KSysGuardProcessList::expandAllChildren(const QModelIndex &parent) 
{
	//This is called when the user expands a node.  This then expands all of its 
	//children.  This will trigger this function again recursively.
	QModelIndex sourceParent = d->mFilterModel.mapToSource(parent);
	for(int i = 0; i < d->mModel.rowCount(sourceParent); i++) 
		d->mUi->treeView->expand(d->mFilterModel.mapFromSource(d->mModel.index(i,0, sourceParent)));
}

void KSysGuardProcessList::expandInit()
{
	//When we expand the items, make sure we don't call our expand all children function
	disconnect(d->mUi->treeView, SIGNAL(expanded(const QModelIndex &)), this, SLOT(expandAllChildren(const QModelIndex &)));
	d->mUi->treeView->expand(d->mFilterModel.mapFromSource(d->mModel.index(0,0, QModelIndex())));
	connect(d->mUi->treeView, SIGNAL(expanded(const QModelIndex &)), this, SLOT(expandAllChildren(const QModelIndex &)));
}

void KSysGuardProcessList::hideEvent ( QHideEvent * event )  //virtual protected from QWidget
{
	//Stop updating the process list if we are hidden
	d->mUpdateTimer->stop();
	QWidget::hideEvent(event);
}

void KSysGuardProcessList::showEvent ( QShowEvent * event )  //virtual protected from QWidget
{
	//Start updating the process list again if we are shown again
	if(!d->mUpdateTimer->isActive()) 
		d->mUpdateTimer->start(d->mUpdateIntervalMSecs);
	QWidget::showEvent(event);
}

void KSysGuardProcessList::updateList()
{
	if(isVisible()) {
		d->mModel.update(d->mUpdateIntervalMSecs);
		d->mUpdateTimer->start(d->mUpdateIntervalMSecs);
	}
}

int KSysGuardProcessList::updateIntervalMSecs() const 
{
	return d->mUpdateIntervalMSecs;
}	

void KSysGuardProcessList::setUpdateIntervalMSecs(int intervalMSecs) 
{
	d->mUpdateIntervalMSecs = intervalMSecs;
	d->mUpdateTimer->setInterval(d->mUpdateIntervalMSecs);
}

void KSysGuardProcessList::reniceProcesses(const QList<long long> &pids, int niceValue)
{
	QList< long long> unreniced_pids;
        for (int i = 0; i < pids.size(); ++i) {
		bool success = d->mModel.processController()->setNiceness(pids.at(i), niceValue);
		if(!success)
			unreniced_pids << pids.at(i);
	}
	if(unreniced_pids.isEmpty()) return; //All processes were reniced successfully
	if(!d->mModel.isLocalhost()) return; //We can't use kdesu to renice non-localhost processes
	
	QStringList arguments;
	arguments << "--" << "renice" << QString::number(niceValue);

        for (int i = 0; i < unreniced_pids.size(); ++i)
		arguments << QString::number(unreniced_pids.at(i));

	QProcess *reniceProcess = new QProcess(NULL);
	connect(reniceProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(reniceFailed()));
	connect(reniceProcess, SIGNAL(finished( int, QProcess::ExitStatus) ), this, SLOT(updateList()));
	reniceProcess->start("kdesu", arguments);
}

QList<KSysGuard::Process *> KSysGuardProcessList::selectedProcesses() const
{
	QList<KSysGuard::Process *> processes;
	QModelIndexList selectedIndexes = d->mUi->treeView->selectionModel()->selectedRows();
	for(int i = 0; i < selectedIndexes.size(); ++i) {
		KSysGuard::Process *process = reinterpret_cast<KSysGuard::Process *> (d->mFilterModel.mapToSource(selectedIndexes.at(i)).internalPointer());
		processes << process;
	}
	return processes;

}

void KSysGuardProcessList::reniceSelectedProcesses()
{
	QList<KSysGuard::Process *> processes = selectedProcesses();
	QStringList selectedAsStrings;
	QList< long long> selectedPids;
	
	if (processes.isEmpty())
	{
		KMessageBox::sorry(this, i18n("You need to select a process first."));
		return;
	}

	int firstPriority = 0;
	foreach(KSysGuard::Process *process, processes) {
		selectedPids << process->pid;
		selectedAsStrings << d->mModel.getStringForProcess(process);
	}
	firstPriority = processes.first()->niceLevel;

	ReniceDlg reniceDlg(d->mUi->treeView, firstPriority, selectedAsStrings);
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
		bool success = d->mModel.processController()->sendSignal(pids.at(i), sig);
		if(!success)
			unkilled_pids << pids.at(i);
	}
	if(unkilled_pids.isEmpty()) return;
	if(!d->mModel.isLocalhost()) return; //We can't use kdesu to kill non-localhost processes

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
	QModelIndexList selectedIndexes = d->mUi->treeView->selectionModel()->selectedRows();
	QStringList selectedAsStrings;
	QList< long long> selectedPids;

	QList<KSysGuard::Process *> processes = selectedProcesses();
	foreach(KSysGuard::Process *process, processes) {
		selectedPids << process->pid;
		selectedAsStrings << d->mModel.getStringForProcess(process);
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
	foreach(KSysGuard::Process *process, processes) {
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
	return d->mModel.showTotals();
}

void KSysGuardProcessList::setShowTotals(bool showTotals)  //slot
{
	d->mModel.setShowTotals(showTotals);
}
