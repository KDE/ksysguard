/*  This file is part of the KDE project
    Copyright (C) 2007 John Tapsell <tapsell@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#include <QtTest>
#include <QtCore>

#include <klocale.h>
#include <qtest_kde.h>

#include "solidstats/processes.h"
#include "solidstats/process.h"

#include "processtest.h"

void testProcess::testProcesses() {
	QHash<long, Solid::Process *> processes = Solid::Processes::getInstance()->getProcesses();
	QSet<long> pids;
	foreach( Solid::Process *process, processes) {
		if(process->pid == 0) continue;
		QVERIFY(process->pid > 0);
		QVERIFY(!process->name.isEmpty());

		//test all the pids are unique
		QVERIFY(!pids.contains(process->pid));
		pids.insert(process->pid);
	}

	QHash<long, Solid::Process *> processes2 = Solid::Processes::getInstance()->getProcesses();
	foreach( Solid::Process *process, processes) {
		if(process->pid == 0) continue;
		QVERIFY(process->pid > 0);
		QVERIFY(!process->name.isEmpty());

		//test all the pids are unique
		if(!pids.contains(process->pid)) {
			kDebug() << process->pid << " not found. " << process->name << endl;
		}
		pids.remove(process->pid);
	}

	QVERIFY(processes2.size() == processes.size());
	QCOMPARE(processes, processes2); //Make sure calling it twice gives the same results.  The difference in time is so small that it really shouldn't have changed
}


unsigned long testProcess::countNumChildren(Solid::Process *p) {
	unsigned long total = p->children.size();
	for(int i =0; i < p->children.size(); i++) {
		total += countNumChildren(p->children[i]);
	}
	return total;
}

void testProcess::testProcessesTreeStructure() {
	QHash<long, Solid::Process *> processes = Solid::Processes::getInstance()->getProcesses();
	foreach( Solid::Process *process, processes) {
		QCOMPARE(countNumChildren(process), process->numChildren);

                for(int i =0; i < process->children.size(); i++) {
			QVERIFY(process->children[i]->parent);
			QCOMPARE(process->children[i]->parent, process);
		}
	}

}

void testProcess::testProcessesModification() {
	//We will modify the tree, then re-call getProcesses and make sure that it fixed everything we modified
	QHash<long, Solid::Process *> processes = Solid::Processes::getInstance()->getProcesses();


	if(!processes.contains(1) || processes[1]->numChildren < 3)
		return;

	QVERIFY(processes[1]);
	QVERIFY(processes[1]->children[0]);
	QVERIFY(processes[1]->children[1]);
	kDebug() << processes[1]->numChildren << endl;
	processes[1]->children[0]->parent = processes[1]->children[1];
	processes[1]->children[1]->children.append(processes[1]->children[0]);
	processes[1]->children[1]->numChildren++;
	processes[1]->numChildren--;
	processes[1]->children.removeAt(0);

	QHash<long, Solid::Process *> processes2 = Solid::Processes::getInstance()->getProcesses();

	QCOMPARE(processes, processes2);

}

void testProcess::testTime() {
	//See how long it takes to get proccess information	
	QTime t;
	t.start();
	QHash<long, Solid::Process *> processes = Solid::Processes::getInstance()->getProcesses();
	kDebug() << "Time elapsed: "<< t.elapsed() <<" ms" <<  endl;
	QVERIFY(t.elapsed() < 300); //It should take less than about 100ms.  Anything longer than 300ms even on a slow system really needs to be optimised
        t.start();
	processes = Solid::Processes::getInstance()->getProcesses();
	kDebug() << "Time elapsed second time: "<< t.elapsed() <<" ms" <<  endl;
	QVERIFY(t.elapsed() < 300); //It should take less than about 100ms.  Anything longer than 300ms even on a slow system really needs to be optimised

}

QTEST_KDEMAIN(testProcess, NoGUI)

#include "processtest.moc"

