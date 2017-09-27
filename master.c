/*
$Id: master.c,v 1.12 2017/09/27 20:32:01 o1-hester Exp $
$Date: 2017/09/27 20:32:01 $
$Revision: 1.12 $
$Log: master.c,v $
Revision 1.12  2017/09/27 20:32:01  o1-hester
alarm

Revision 1.11  2017/09/25 21:37:15  o1-hester
*** empty log message ***

Revision 1.10  2017/09/25 06:35:46  o1-hester
*** empty log message ***

Revision 1.9  2017/09/24 23:32:22  o1-hester
cleanup, modularization

Revision 1.8  2017/09/24 06:37:33  o1-hester
signals better

Revision 1.7  2017/09/23 04:43:25  o1-hester
trying to set up signal handlers

Revision 1.6  2017/09/22 22:55:49  o1-hester
cleanup

Revision 1.5  2017/09/20 06:50:10  o1-hester
child limit

Revision 1.4  2017/09/20 01:59:20  o1-hester
*** empty log message ***

Revision 1.3  2017/09/17 22:50:33  o1-hester
shm to msgqueue

Revision 1.2  2017/09/17 21:24:14  o1-hester
shared memory work

Revision 1.1  2017/09/17 00:31:12  o1-hester
Initial revision

$Author: o1-hester $ 
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ipchelper.h"
#include "sighandler.h"
#include "filehelper.h"
#define INFILE "./strings.in"

int initmylist(const char* filename, int* lines, char*** mylist);
int initsighandlers();
int initsemaphores(int semid, int lines);

int main (int argc, char** argv) {
	// initialize mylist from file
	char** mylist;
	int lines; 
	if (initmylist(INFILE, &lines, &mylist) == -1) {
		perror("Failed to read from file.");
		return 1;
	}

	// get key from file
	key_t mkey, skey;
	if (((mkey = ftok(KEYPATH, PROJ_ID)) == -1) ||
		((skey = ftok(KEYPATH, SEM_ID)) == -1)) {
		perror("Failed to retreive keys.");
		return 1;
	}
	// add cmd line argument for timer
	alarm(60);
	/*************** Set up signal handler ********/
	if (initsighandlers() == -1) {
		perror("Failed to set up signal handlers.");
		return 1;
	}
	/*************** Set up semaphore *************/
	// semaphore contains 3 sems:
	// 0 = file i/o lock
	// 1 = master knows when done
	// 2 = for limiting to 19 children at one time
	int semid;
	if ((semid = getsemid(skey, 3)) == -1) {
		perror("Failed to create semaphore.");
		return 1;
	}	
	struct sembuf waitfordone[1];
	struct sembuf birthcontrol[1];
	setsembuf(waitfordone, 1, 0, 0);
	setsembuf(birthcontrol, 2, -1, 0);
	if (initsemaphores(semid, lines) == -1) {
		perror("Failed to init semaphores.");
		return 1;
	}
	/************** Set up message queue *********/
	// Initiate message queue	
	int msgid;
	if ((msgid = getmsgid(mkey)) == -1) {
		perror("Failed to create message queue.");
		return 1;
	}
	// Send mylist to msgqueue
	if (sendmessages(msgid, mylist, lines) == -1) {
		perror("Failed to send messages to queue.");
		return 1;
	}
	free(mylist);
	/****************** Spawn Children ***********/
	// make child
	// char[]'s for sending id and index to child process
	int childpid;
	char palinid[16];
	char palinindex[16];
	int semval;
	int j = 0;
	while (j < lines) {
		// if there are 19 processes, wait for one to finish
		if (semop(semid, birthcontrol, 1) == -1) {
			perror("Semaphore birth control.");
			return 1;
		}
		if ((semval = semctl(semid, 2, GETVAL)) == -1) {
			perror("Semaphore birth control.");
			return 1;
		}
		/************** Important ************/
		if (semval < 2) wait(NULL);
		if ((childpid = fork()) <= 0) {
			// set id and msg index
			sprintf(palinid, "%d", j+1 % 20);
			sprintf(palinindex, "%d", j);
			break;
		}  
		fprintf(stderr, "... child  w/ index: %d spawned.\n", j);
		if (childpid != -1)	
			j++;
	}
	// If master fails at spawning children
	if (childpid == -1) {
		perror("Failed to create child.");
		if (removeshmem(msgid, semid) == -1) {
			perror("failed to remove shared mem segment");
			return 1;
		}
	}
	/***************** Child ******************/
	if (childpid == 0) {
		// execute palin with id
		execl("./palin", "palin", palinid, palinindex, (char*)NULL);
		perror("Exec failure.");
		return 1; // if error
	}
	/***************** Parent *****************/
	if (childpid > 0) {
		fprintf(stderr, "master: done spawning, waiting for done.\n");	
		// Waits for all children to be done
		if (semop(semid, waitfordone, 1) == -1) {
			perror("Failed to wait for children.");
			return 1;
		}
		if (removeshmem(msgid, semid) == -1) {
			// failed to remove shared mem segment
			perror("Failed to remove shared memory.");
			return 1;
		}
		fprintf(stderr, "Done. %ld\n", (long)getpgid(getpid()));
	}	
	return 0;	

}

// initialize mylist from file, return -1 on error
int initmylist(const char* filename, int* lines, char*** mylist) {
	if ((*lines = countLines(filename)) == -1) {
		return -1;
	}
	*mylist = malloc((*lines)*sizeof(char*));
	if (setArrayFromFile(filename, *mylist) == -1) {
		return -1;
	}
	return 0;
}

// initialize signal handlers, return -1 on error
int initsighandlers() {
	struct sigaction newact = {0};
	struct sigaction timer = {0};
	timer.sa_handler = handletimer;
	timer.sa_flags = 0;
	newact.sa_handler = catchctrlc;
	newact.sa_flags = 0;
	// set timer handler
	if ((sigemptyset(&timer.sa_mask) == -1) ||
	    (sigaction(SIGALRM, &timer, NULL) == -1)) {
		return -1;
	}	
	// set intr handler
	if ((sigemptyset(&newact.sa_mask) == -1) ||
	    (sigaction(SIGINT, &newact, NULL) == -1)) {
		return -1;
	}	
	return 0;
}

// initialize semaphores, return -1 on error
int initsemaphores(int semid, int lines) {
	// set up file i/o lock
	if (initelement(semid, 0, 1) == -1) {
		if (semctl(semid, 0, IPC_RMID) == -1)
			return -1;
		return -1;
	}
	// set up master knowing when done
	if (initelement(semid, 1, lines) == -1) {
		if (semctl(semid, 0, IPC_RMID) == -1)
			return -1;
		return -1;
	}
	// set up child limiter
	if (initelement(semid, 2, 19) == -1) {
		if (semctl(semid, 0, IPC_RMID) == -1)
			return -1;
		return -1;
	}
	return 0;
}
