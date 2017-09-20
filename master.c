/*
$Id: master.c,v 1.4 2017/09/20 01:59:20 o1-hester Exp o1-hester $
$Date: 2017/09/20 01:59:20 $
$Revision: 1.4 $
$Log: master.c,v $
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

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/stat.h>
#define KEYPATH "./yobanana.boy"
#define PROJ_ID 456234
#define SEM_ID 234456
#define PERM (S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH)

typedef struct {
	long mType;
	char mText[80];
} mymsg_t;

void setArrayFromFile(char** list, FILE* fp);
int countLines(FILE* fp);
int removeMsgQueue(int msgid);
void setsembuf(struct sembuf *s, int n, int op, int flg);
int initelement(int semid, int semnum, int semval);

int main (int argc, char** argv) {
	const char* filename = "./strings.in";
	char** mylist;

	FILE* fp = fopen(filename, "r");
	if (fp == NULL) {
		fprintf(stderr, "File not found.\n");
		exit(1);
	}
	// initialize mylist from file
	int lines = countLines(fp);
	mylist = malloc(lines*sizeof(char*));
	setArrayFromFile(mylist, fp);

	// get key from file
	key_t mkey, skey;
	mkey = ftok(KEYPATH, PROJ_ID);
	skey = ftok(KEYPATH, SEM_ID); 

	/*************** Set up semaphore *************/
	int semid;
	if ((semid = semget(skey, 2, PERM | IPC_CREAT)) == -1) {
		perror("Failed to create semaphore.");
		return 1;
	}	
	struct sembuf wait[1];
	struct sembuf waitfordone[1];
	struct sembuf signal[1];
	setsembuf(wait, 0, -1, 0);
	setsembuf(waitfordone, 1, 0, 0);
	setsembuf(signal, 0, 1, 0);
	if (initelement(semid, 0, 1) == -1) {
		perror("Failed to initialize semaphore.");
		// remove sem
		return 1;
	}

	if (initelement(semid, 1, 19) == -1) {
		perror("Failed to initialize semaphore.");
		// remove sem
		return 1;
	}


	/************** Set up message queue *********/
	// Initiate message queue	
	int msgid;
	mymsg_t* mymsg;
	if ((msgid = msgget(mkey, PERM | IPC_CREAT)) == -1) {
		perror("Failed to create message queue.");
		exit(1);
	}
	
	// Send mylist to msgqueue
	int j;
	for (j = 1; j <= lines; j++) {
		if ((mymsg = (mymsg_t*) malloc(sizeof(mymsg_t))) == NULL) {
			perror("Failed to allocate message.");
			return 1;
		}
		mymsg->mType = j;
		strcpy(mymsg->mText, mylist[j-1]);
		if (msgsnd(msgid, mymsg, 80, 0) == -1) {
			perror("Failed to send message.");
			return 1;
		}	
	}
	free(mylist);
	
	// make child
	int childpid;
	char palinid[3];
	int i = 0;
	while (i < 19) {
		if (childpid = fork()) {
			sprintf(palinid, "%d", i+1);
			break;
		}
		i++;
	}

	/***************** Child ******************/
	if (childpid == -1)
		perror("Failed to create child.");

	if (childpid > 0) {
		execl("./palin", "palin", palinid, palinid, NULL);
		perror("Exec failure.");
		exit(1); // if error
	}
	/***************** Parent *****************/
	if (childpid == 0) {
		
		semop(semid, waitfordone, 1);

		fprintf(stderr, "Killing msgqueue.\n");	
		if (removeMsgQueue(msgid) == -1){
			perror("Failed to destroy message queue.");
			return 1;
		}
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
void setArrayFromFile(char** list, FILE* fp) {
	const size_t LINESIZE = 80;
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
}

// Count how many lines a file has
int countLines(FILE* fp) {
	const size_t LINESIZE = 80;
	int n = 0;
	char* line = malloc(LINESIZE*sizeof(char));
	rewind(fp);
	while (fgets(line, LINESIZE, (FILE*)fp)) {
		n++;
	}
	free(line);
	return n;
}

// returns -1 on failure
int initelement(int semid, int semnum, int semval) {
	union semun {
		int val;
		struct semid_ds *buf;
		unsigned short *array;
	} arg;
	arg.val = semval;
	return semctl(semid, semnum, SETVAL, arg);
}
	
//set up a semaphore operation
void setsembuf(struct sembuf *s, int n, int op, int flg) {
	s->sem_num = (short)n;
	s->sem_op = (short)op;
	s->sem_flg = (short)flg;
	return;
}

// destroy message queue segment
int removeMsgQueue(int msgid) {
	return msgctl(msgid, IPC_RMID, NULL);
}

