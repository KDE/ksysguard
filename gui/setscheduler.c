#include <stdio.h>
#include <stdlib.h>
#include <sched.h>

int main(int argc, char **argv)
{
	if(argc != 4) {
		fprintf(stderr, "Usage: %s PID SCHEDULER PRIORITY\n", argv[0]);
		fprintf(stderr, "Sets the CPU Scheduler and priority for the given PID.\n  See man sched_setscheduler for more information\n\n");
		fprintf(stderr, "SCHEDULER is one of:\n");
		fprintf(stderr, "		0	Other\n");
		fprintf(stderr, "		1	FIFO\n");
		fprintf(stderr, "		2	RoundRobin\n");
#ifdef SCHED_BATCH
		fprintf(stderr, "		3	Batch\n");
#endif
		exit(EXIT_FAILURE);
	}
	int pid = atoi(argv[1]);
	int priorityClass = atoi(argv[2]);
	int priority = atoi(argv[3]);
	printf("Setting scheduler for %d to %d %d\n", pid, priorityClass, priority);

	struct sched_param params;
	params.sched_priority = priority;
	int result;
	switch(priorityClass) {
		case 0:
			result = sched_setscheduler( pid, SCHED_OTHER, &params);
			break;
		case 1:
			result = sched_setscheduler( pid, SCHED_FIFO, &params);
			break;
		case 2:
			result = sched_setscheduler( pid, SCHED_RR, &params);;
			break;
#ifdef SCHED_BATCH
		case 3:
			result = sched_setscheduler( pid, SCHED_BATCH, &params);
			break;
#endif
		default:
			fprintf(stderr, "Invalid Priority class\n");
			exit(EXIT_FAILURE);
	}
	if(result != 0) {
		perror("Failure setting scheduler");
	}

	return result;
}
