/*
$Id: master.c,v 1.9 2017/09/24 23:32:22 o1-hester Exp $
$Date: 2017/09/24 23:32:22 $
$Revision: 1.9 $
$Log: master.c,v $
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
int initsemaphores(int semid, int lines);

int main (int argc, char** argv) {
	// initialize mylist from file
	char** mylist;
	int lines; 
	if (initmylist(INFILE, &lines, &mylist) == -1) {
		perror("Failed to read from file.");
		return 1;
	}

	alarm(60);
	/*************** Set up signal handler ********/
	struct sigaction newact = {0};
	struct sigaction timer = {0};
	timer.sa_handler = handletimer;
	timer.sa_flags = 0;
	newact.sa_handler = catchctrlc;
	newact.sa_flags = 0;
	// set timer handler
	if ((sigemptyset(&timer.sa_mask) == -1) ||
	    (sigaction(SIGALRM, &timer, NULL) == -1)) {
		perror("Failed to set SIGALRM handler.");
		return 1;
	}	
	// set intr handler
	if ((sigemptyset(&newact.sa_mask) == -1) ||
	    (sigaction(SIGINT, &newact, NULL) == -1)) {
		perror("Failed to set SIGINT handler.");
		return 1;
	}	
	
	// get key from file
	key_t mkey, skey;
	mkey = ftok(KEYPATH, PROJ_ID);
	skey = ftok(KEYPATH, SEM_ID); 

	/*************** Set up semaphore *************/
	// semaphore contains 3 sems:
	// 0 = file i/o lock
	// 1 = master knows when done
	// 2 = for limiting to 19 children at one time
	int semid;
	if ((semid = semget(skey, 3, PERM | IPC_CREAT)) == -1) {
		perror("Failed to create semaphore.");
		return 1;
	}	
	struct sembuf waitfordone[1];
	struct sembuf birthcontrol[2];
	setsembuf(waitfordone, 1, 0, 0);
	setsembuf(birthcontrol, 2, -1, 0);
	setsembuf(birthcontrol+1, 2, 1, 0);
	if (initsemaphores(semid, lines) == -1) {
		perror("Failed to init semaphores.");
		return 1;
	}
	/*
	// set up file i/o lock
	if (initelement(semid, 0, 1) == -1) {
		perror("Failed to initialize semaphore.");
		if (semctl(semid, 0, IPC_RMID) == -1)
			perror("Failed to remove semaphore.");
		return 1;
	}
	// set up master knowing when done
	if (initelement(semid, 1, lines) == -1) {
		perror("Failed to initialize semaphore.");
		if (semctl(semid, 0, IPC_RMID) == -1)
			perror("Failed to remove semaphore.");
		return 1;
	}
	// set up child limiter
	if (initelement(semid, 2, 19) == -1) {
		perror("Failed to initialize semaphore.");
		if (semctl(semid, 0, IPC_RMID) == -1)
			perror("Failed to remove semaphore.");
		return 1;
	}
	*/
	/************** Set up message queue *********/
	// Initiate message queue	
	int msgid;
	if ((msgid = getmsgid(mkey)) == -1) {
		perror("Failed to create message queue.");
		return 1;
	}
	// Send mylist to msgqueue
	int j;
	mymsg_t* mymsg;
	for (j = 1; j <= lines; j++) {
		if ((mymsg = (mymsg_t*) malloc(sizeof(mymsg_t))) == NULL) {
			perror("Failed to allocate message.");
			return 1;
		}
		// mType is index of string
		// mText is string
		// child finds string using mType
		mymsg->mType = j;
		memcpy(mymsg->mText, mylist[j-1], LINESIZE);
		if (msgsnd(msgid, mymsg, LINESIZE, 0) == -1) {
			perror("Failed to send message.");
			return 1;
		}	
	}
	free(mylist);
	
	/****************** Spawn Children ***********/
	// make child
	// char[]'s for sending id and index to child process
	int childpid;
	char palinid[16];
	char palinindex[16];
	int semval;
	j = 0;
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
		// removal of next line may result in loss of terminal use!
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

	if (childpid == -1) {
		perror("Failed to create child.");
		//if (removeshmem(msgid, semid) == -1) {
			// failed to remove shared mem segment
		//	return 1;
		//}
		if (semop(semid, birthcontrol+1, 1) == -1) {
			perror("Semaphore birth control.");
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
		fprintf(stderr, "%ld: waiting...", (long)getpid());	
		// Waits for all children to be done
		if (semop(semid, waitfordone, 1) == -1) {
			perror("Failed to wait for children.");
			return 1;
		}
		if (removeshmem(msgid, semid) == -1) {
			// failed to remove shared mem segment
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
}
