/*
$Id: master.c,v 1.6 2017/09/22 22:55:49 o1-hester Exp $
$Date: 2017/09/22 22:55:49 $
$Revision: 1.6 $
$Log: master.c,v $
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

#include "ipchelper.h"
#define INFILE "./strings.in"

int setArrayFromFile(char** list, FILE* fp);
int countLines(FILE* fp);

int main (int argc, char** argv) {
	// open input file
	FILE* fp = fopen(INFILE, "r");
	if (fp == NULL) {
		fprintf(stderr, "File not found.\n");
		return 1;
	}

	// initialize mylist from file
	char** mylist;
	int lines; 
	if ((lines = countLines(fp)) == -1) {
		perror("Failed to read from file.");
		return 1;
	}
	mylist = malloc(lines*sizeof(char*));
	if (setArrayFromFile(mylist, fp) == -1) {
		perror("Failed to read from file.");
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
	struct sembuf birthcontrol[1];
	setsembuf(waitfordone, 1, 0, 0);
	setsembuf(birthcontrol, 2, -1, 0);

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

	/************** Set up message queue *********/
	// Initiate message queue	
	int msgid;
	if ((msgid = msgget(mkey, PERM | IPC_CREAT)) == -1) {
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
	
	// make child
	int childpid;
	char palinid[16];
	char palinindex[16];
	j = 0;
	while (j < lines) {
		// if there are 19 processes, wait for one to finish
		if (semop(semid, birthcontrol, 1) == -1) {
			perror("Semaphore birth control.");
			return 1;
		}
		if (childpid = fork()) {
			// set id and msg index
			sprintf(palinid, "%d", j+1 % 20);
			sprintf(palinindex, "%d", j);
			break;
		}
		j++;
	}

	/***************** Child ******************/
	if (childpid == -1)
		perror("Failed to create child.");

	if (childpid > 0) {
		// execute palin with id
		execl("./palin", "palin", palinid, palinindex, NULL);
		perror("Exec failure.");
		return 1; // if error
	}
	/***************** Parent *****************/
	if (childpid == 0) {
		
		// Waits for all children to be done
		if (semop(semid, waitfordone, 1) == -1) {
			perror("Failed to wait for children.");
			return 1;
		}
		// Kill message queue
		fprintf(stderr, "Killing msgqueue.\n");	
		if (removeMsgQueue(msgid) == -1){
			perror("Failed to destroy message queue.");
			return 1;
		}
		// kill semaphore set
		fprintf(stderr, "Killing semaphore set.\n");
		if (semctl(semid, 0, IPC_RMID) == -1) {
			perror("Failed to remove semaphore set.");
			return 1;
		}
		fprintf(stderr, "Done.\n");
	}	
	return 0;	

}

// Set a char** to a list of strings (by line) from a file
int setArrayFromFile(char** list, FILE* fp) {
	int n = 0;
	char* line = malloc(LINESIZE*sizeof(char));
	rewind(fp);
	while (fgets(line, LINESIZE, (FILE*)fp)) {
		list[n] = malloc(LINESIZE*sizeof(char));
		// copy line into array, remove newline
		memcpy(list[n], line, LINESIZE);
		list[n][strcspn(list[n], "\n")] = '\0';
		n++;
	}
	free(line);
	if (errno != 0)
		return -1;
	return 0;
}

// Count how many lines a file has
int countLines(FILE* fp) {
	int n = 0;
	char* line = malloc(LINESIZE*sizeof(char));
	rewind(fp);
	while (fgets(line, LINESIZE, (FILE*)fp)) {
		n++;
	}
	free(line);
	if (errno != 0)
		return -1;
	return n;
}

