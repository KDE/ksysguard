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
#include <QCheckBox>
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
#include "processes.h"

#define UPDATE_INTERVAL 2000

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
	mSimple = true;
	mMemFree = -1;
	mMemUsed = -1;
	mMemTotal = -1;
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
	connect(mUi->chkShowTotals, SIGNAL(toggled(bool)), &mModel, SLOT(setShowTotals(bool)));
	mUi->chkShowTotals->setVisible(!mSimple);
	setMinimumSize(sizeHint());

	/*  Hide the vm size column by default since it's not very useful */
	mUi->treeView->header()->hideSection(ProcessModel::HeadingVmSize);
	mUi->treeView->header()->hideSection(ProcessModel::HeadingNiceness);
	mUi->treeView->header()->hideSection(ProcessModel::HeadingTty);
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
	mUpdateTimer->start(UPDATE_INTERVAL);

	//If the view resorts continually, then it can be hard to keep track of processes.  By doing it only every 3 seconds it reduces the 'jumping around'
	QTimer *mTimer = new QTimer(this);
	connect(mTimer, SIGNAL(timeout()), &mFilterModel, SLOT(resort()));
	mTimer->start(3000); 

	expandInit(); //This will expand the init process
}

QLineEdit *KSysGuardProcessList::filterLineEdit() const {
	return mUi->txtFilter;
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
	mUpdateTimer->stop();
	QWidget::hideEvent(event);
}
void KSysGuardProcessList::showEvent ( QShowEvent * event )  //virtual protected from QWidget
{
	if(!mUpdateTimer->isActive()) 
		mUpdateTimer->start(UPDATE_INTERVAL);
	QWidget::showEvent(event);
}

void
KSysGuardProcessList::updateList()
{
	if(isVisible()) {
		mModel.update(UPDATE_INTERVAL);
		expandInit(); //This will expand the init process
		mUpdateTimer->start(UPDATE_INTERVAL);
	}
}

void KSysGuardProcessList::reniceProcess(int pid, int niceValue)
{
	bool success = mModel.processController()->setNiceness(pid, niceValue);
	if(success) return;
	if(!mModel.isLocalhost()) return; //We can't renice non-localhost processes

	QStringList arguments;
	arguments << "renice" << QString::number(niceValue) << QString::number(pid);

	QProcess *reniceProcess = new QProcess(this);
	connect(reniceProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(reniceFailed()));
	reniceProcess->start("kdesu", arguments);
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
	ReniceDlg reniceDlg(mUi->treeView, firstPriority, selectedAsStrings);
	if(reniceDlg.exec() == QDialog::Rejected) return;
	int newPriority = reniceDlg.newPriority;
	Q_ASSERT(newPriority <= 19 && newPriority >= -20); 

	Q_ASSERT(selectedPids.size() == selectedAsStrings.size());
	for (int i = 0; i < selectedPids.size(); ++i)
		reniceProcess(selectedPids.at(i), newPriority);
	updateList();
}

void KSysGuardProcessList::killProcess(int pid, int sig)
{
	bool success = mModel.processController()->sendSignal(pid, sig);
	if(success) return;
	if(!mModel.isLocalhost()) return; //We can't kill non-localhost processes

	//We must use kdesu to kill the process
	QStringList arguments;
	if(sig != SIGTERM)
		arguments << "kill" << ('-' + QString::number(sig)) << QString::number(pid);
	else
		arguments << "kill" << QString::number(pid);

	QProcess *killProcess = new QProcess(this);
	connect(killProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(killFailed()));
	killProcess->start("kdesu", arguments);

}
void KSysGuardProcessList::killProcess()
{
	QModelIndexList selectedIndexes = mUi->treeView->selectionModel()->selectedRows();
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
								 KStandardGuiItem::cancel(),
								 "killconfirmation");
		if (res != KMessageBox::Continue)
		{
			return;
		}
	}

	Q_ASSERT(selectedPids.size() == selectedAsStrings.size());
        for (int i = 0; i < selectedPids.size(); ++i) {
		killProcess(selectedPids.at(i), SIGTERM);
        }
	updateList();
}

#if 0
void
KSysGuardProcessList::answerReceived(int id, const QList<QByteArray>& answer)
{
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
	KMessageBox::sorry(this, i18n("You do not have the permission to renice the process and there "
                                "was a problem trying to run as root"));
}
void KSysGuardProcessList::killFailed()
{
	KMessageBox::sorry(this, i18n("You do not have the permission to kill the process and there "
                                "was a problem trying to run as root"));
}


