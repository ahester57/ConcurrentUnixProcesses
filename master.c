/*
$Id: master.c,v 1.3 2017/09/17 22:50:33 o1-hester Exp o1-hester $
$Date: 2017/09/17 22:50:33 $
$Revision: 1.3 $
$Log: master.c,v $
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
#define PROJ_ID 456234
#define SEM_ID 234456
#define PERM (S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH)

typedef struct {
	long mType;
	char mText[80];
} mymsg_t;

void setArrayFromFile(char** list, FILE* fp);
int countLines(FILE* fp);
int detachAndRemove(int msgid);

int main (int argc, char** argv) {
	const char* filename = "./strings.in";
	const char* keypath = "./yobanana.boy";
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
	key_t key;
	key = ftok(keypath, PROJ_ID);
	
	// Initiate message queue	
	int msgid;
	mymsg_t* mymsg;
	if ((msgid = msgget(key, PERM | IPC_CREAT)) == -1) {
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
	for (j = 1; j < 19; j++) {
		if (childpid = fork()) {
			sprintf(palinid, "%d", j);	
			break;
		}
	}
	if (childpid == -1)
		perror("Failed to create child.");

	if (childpid > 0) {
		execl("./palin", "palin", palinid, palinid, NULL);
		perror("Exec failure.");
		exit(1); // if error
	}
	if (childpid == 0) {
		int v;
		// destroy shmem segment

		while (v = waitpid(-1, NULL, WNOHANG)) {
			if (errno == ECHILD) {
				perror("waiting:");
				break;
			}
		};


		fprintf(stderr, "Killing msgqueue.");	
		if (detachAndRemove(msgid) == -1){
			perror("Failed to destroy message queue.");
			exit(1);
		}
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

// destroy shared memory segment
int detachAndRemove(int msgid) {
	return msgctl(msgid, IPC_RMID, NULL);
}

